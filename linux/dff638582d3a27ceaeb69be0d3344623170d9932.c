// KASAN: use-after-free Read in finish_task_switch
// https://syzkaller.appspot.com/bug?id=dff638582d3a27ceaeb69be0d3344623170d9932
// status:fixed
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
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

unsigned long long procid;

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
  if (pthread_create(&th, &attr, fn, arg))
    exit(1);
  pthread_attr_destroy(&attr);
}

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
  }

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
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG);
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
    if (__atomic_load_n(&ev->state, __ATOMIC_RELAXED))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
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

#define SYZ_HAVE_SETUP_TEST 1
static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
}

#define SYZ_HAVE_RESET_TEST 1
static void reset_test()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
    close(fd);
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
  int collide = 0;
again:
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
      if (collide && (call % 2) == 0)
        break;
      event_timedwait(&th->done, 45);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
  if (!collide) {
    collide = 1;
    goto again;
  }
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
      reset_test();
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    *(uint32_t*)0x2001d000 = 1;
    *(uint32_t*)0x2001d004 = 0x70;
    *(uint8_t*)0x2001d008 = 0;
    *(uint8_t*)0x2001d009 = 0;
    *(uint8_t*)0x2001d00a = 0;
    *(uint8_t*)0x2001d00b = 0;
    *(uint32_t*)0x2001d00c = 0;
    *(uint64_t*)0x2001d010 = 0x7f;
    *(uint64_t*)0x2001d018 = 0;
    *(uint64_t*)0x2001d020 = 0;
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 0, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 1, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 2, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 3, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 4, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0x81, 5, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 6, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 7, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 8, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 9, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 10, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 11, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 12, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 13, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 14, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 15, 2);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 17, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 18, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 19, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 20, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 21, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 22, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 23, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 24, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 25, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 26, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 27, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 28, 1);
    STORE_BY_BITMASK(uint64_t, 0x2001d028, 0, 29, 35);
    *(uint32_t*)0x2001d030 = 0;
    *(uint32_t*)0x2001d034 = 0;
    *(uint64_t*)0x2001d038 = 0x20abe000;
    *(uint64_t*)0x2001d040 = 0;
    *(uint64_t*)0x2001d048 = 0;
    *(uint64_t*)0x2001d050 = 0;
    *(uint32_t*)0x2001d058 = 0;
    *(uint32_t*)0x2001d05c = 0;
    *(uint64_t*)0x2001d060 = 0;
    *(uint32_t*)0x2001d068 = 0;
    *(uint16_t*)0x2001d06c = 0;
    *(uint16_t*)0x2001d06e = 0;
    syscall(__NR_perf_event_open, 0x2001d000, 0, -1, -1, 0);
    break;
  case 1:
    memcpy((void*)0x20000200, "/dev/kvm", 9);
    res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000200, 0, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 2:
    res = syscall(__NR_ioctl, r[0], 0xae01, 0);
    if (res != -1)
      r[1] = res;
    break;
  case 3:
    res = syscall(__NR_ioctl, r[1], 0xae41, 0);
    if (res != -1)
      r[2] = res;
    break;
  case 4:
    syscall(__NR_ioctl, r[2], 0x4080aebf, 0x200000c0);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
