// WARNING: bad unlock balance in __get_user_pages_remote
// https://syzkaller.appspot.com/bug?id=a8c70b7f3579fc0587dc
// status:6 arch:386
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/futex.h>

static unsigned long long procid;

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  int i;
  for (i = 0; i < 100; i++) {
    if (pthread_create(&th, &attr, fn, arg) == 0) {
      pthread_attr_destroy(&attr);
      return;
    }
    if (errno == EAGAIN) {
      usleep(50);
      continue;
    }
    break;
  }
  exit(1);
}

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1000000);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
}

static bool write_file(const char* file, const char* what, ...)
{
  char buf[1024];
  va_list args;
  va_start(args, what);
  vsnprintf(buf, sizeof(buf), what, args);
  va_end(args);
  buf[sizeof(buf) - 1] = 0;
  int len = strlen(buf);
  int fd = open(file, O_WRONLY | O_CLOEXEC);
  if (fd == -1)
    return false;
  if (write(fd, buf, len) != len) {
    int err = errno;
    close(fd);
    errno = err;
    return false;
  }
  close(fd);
  return true;
}

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  int i;
  for (i = 0; i < 100; i++) {
    if (waitpid(-1, status, WNOHANG | __WALL) == pid)
      return;
    usleep(1000);
  }
  DIR* dir = opendir("/sys/fs/fuse/connections");
  if (dir) {
    for (;;) {
      struct dirent* ent = readdir(dir);
      if (!ent)
        break;
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        continue;
      char abort[300];
      snprintf(abort, sizeof(abort), "/sys/fs/fuse/connections/%s/abort",
               ent->d_name);
      int fd = open(abort, O_WRONLY);
      if (fd == -1) {
        continue;
      }
      if (write(fd, abort, 1) < 0) {
      }
      close(fd);
    }
    closedir(dir);
  } else {
  }
  while (waitpid(-1, status, __WALL) != pid) {
  }
}

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
}

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void execute_one(void)
{
  int i, call, thread;
  for (call = 0; call < 5; call++) {
    for (thread = 0; thread < (int)(sizeof(threads) / sizeof(threads[0]));
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      event_timedwait(&th->done, 45);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

#ifndef __NR_gettid
#define __NR_gettid 224
#endif
#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_perf_event_open
#define __NR_perf_event_open 336
#endif
#ifndef __NR_process_vm_writev
#define __NR_process_vm_writev 348
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[1] = {0x0};

void execute_call(int call)
{
  intptr_t res;
  switch (call) {
  case 0:
    *(uint32_t*)0x200000c0 = 1;
    *(uint32_t*)0x200000c4 = 0x70;
    *(uint8_t*)0x200000c8 = 0;
    *(uint8_t*)0x200000c9 = 0;
    *(uint8_t*)0x200000ca = 0;
    *(uint8_t*)0x200000cb = 0;
    *(uint32_t*)0x200000cc = 0;
    *(uint64_t*)0x200000d0 = 0x41c1;
    *(uint64_t*)0x200000d8 = 0;
    *(uint64_t*)0x200000e0 = 0;
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 0, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 1, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 2, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 3, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 4, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 3, 5, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 6, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 7, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 8, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 9, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 10, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 11, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 12, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 13, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 14, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 15, 2);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 17, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 18, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 19, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 20, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 21, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 22, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 23, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 24, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 25, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 26, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 27, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 28, 1);
    STORE_BY_BITMASK(uint64_t, , 0x200000e8, 0, 29, 35);
    *(uint32_t*)0x200000f0 = 0;
    *(uint32_t*)0x200000f4 = 0;
    *(uint64_t*)0x200000f8 = 0;
    *(uint64_t*)0x20000100 = 0;
    *(uint64_t*)0x20000108 = 0;
    *(uint64_t*)0x20000110 = 0;
    *(uint32_t*)0x20000118 = 0;
    *(uint32_t*)0x2000011c = 0;
    *(uint64_t*)0x20000120 = 0;
    *(uint32_t*)0x20000128 = 0;
    *(uint16_t*)0x2000012c = 0;
    *(uint16_t*)0x2000012e = 0;
    syscall(__NR_perf_event_open, 0x200000c0, 0, -1, -1, 0);
    break;
  case 1:
    syscall(__NR_mmap, 0x20000000, 0xb36000, 3, 0x8031, -1, 0);
    break;
  case 2:
    res = syscall(__NR_gettid);
    if (res != -1)
      r[0] = res;
    break;
  case 3:
    *(uint32_t*)0x20c22000 = 0x2034afa4;
    *(uint32_t*)0x20c22004 = 0x1f80;
    *(uint32_t*)0x20c22fa0 = 0x20000080;
    *(uint32_t*)0x20c22fa4 = 0x2034afa5;
    syscall(__NR_process_vm_writev, (intptr_t)r[0], 0x20c22000, 1, 0x20c22fa0,
            1, 0);
    break;
  case 4:
    syscall(__NR_ioctl, -1, 0x5418, -1);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
