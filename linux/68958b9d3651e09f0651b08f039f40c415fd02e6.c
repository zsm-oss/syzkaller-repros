// KASAN: use-after-free Read in debugfs_remove
// https://syzkaller.appspot.com/bug?id=68958b9d3651e09f0651b08f039f40c415fd02e6
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <setjmp.h>
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

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  exit(sig);
}

static void install_segv_handler(void)
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                        \
  {                                                                            \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    }                                                                          \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
  }

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

static long syz_open_dev(volatile long a0, volatile long a1, volatile long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    NONFAILING(strncpy(buf, (char*)a0, sizeof(buf) - 1));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
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
  for (call = 0; call < 38; call++) {
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

uint64_t r[5] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0x0, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res;
  switch (call) {
  case 0:
    syscall(__NR_openat, 0xffffffffffffff9c, 0, 0, 0);
    break;
  case 1:
    syscall(__NR_connect, -1, 0, 0);
    break;
  case 2:
    syscall(__NR_init_module, 0, 0, 0);
    break;
  case 3:
    syscall(__NR_ioctl, -1, 0x4c81, 0);
    break;
  case 4:
    syscall(__NR_init_module, 0, 0, 0);
    break;
  case 5:
    syscall(__NR_perf_event_open, 0, 0, -1, -1, 0);
    break;
  case 6:
    NONFAILING(memcpy((void*)0x20000200, "/dev/loop-control\000", 18));
    syscall(__NR_openat, 0xffffffffffffff9c, 0x20000200, 0, 0);
    break;
  case 7:
    syscall(__NR_connect, -1, 0, 0);
    break;
  case 8:
    NONFAILING(*(uint32_t*)0x200003c0 = 3);
    syscall(__NR_setsockopt, -1, 0x29, 0x3e, 0x200003c0, 4);
    break;
  case 9:
    res = syscall(__NR_openat, 0xffffffffffffff9c, 0, 0, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 10:
    NONFAILING(*(uint32_t*)0x20000240 = 0);
    syscall(__NR_ioctl, r[0], 0x400454dc, 0x20000240);
    break;
  case 11:
    syscall(__NR_openat, 0xffffffffffffff9c, 0, 0, 0);
    break;
  case 12:
    syscall(__NR_socket, 2, 1, 0);
    break;
  case 13:
    syscall(__NR_perf_event_open, 0, 0, -1, -1, 0);
    break;
  case 14:
    NONFAILING(memcpy((void*)0x20000200, "/dev/loop-control\000", 18));
    res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000200, 0, 0);
    if (res != -1)
      r[1] = res;
    break;
  case 15:
    syscall(__NR_ioctl, r[1], 0x4c80, 0);
    break;
  case 16:
    syscall(__NR_ioctl, r[1], 0x4c81, 0);
    break;
  case 17:
    res = syscall(__NR_socket, 0xa, 2, 0);
    if (res != -1)
      r[2] = res;
    break;
  case 18:
    NONFAILING(*(uint16_t*)0x20000000 = 0xa);
    NONFAILING(*(uint16_t*)0x20000002 = htobe16(0));
    NONFAILING(*(uint32_t*)0x20000004 = htobe32(0));
    NONFAILING(*(uint8_t*)0x20000008 = -1);
    NONFAILING(*(uint8_t*)0x20000009 = 2);
    NONFAILING(*(uint8_t*)0x2000000a = 0);
    NONFAILING(*(uint8_t*)0x2000000b = 0);
    NONFAILING(*(uint8_t*)0x2000000c = 0);
    NONFAILING(*(uint8_t*)0x2000000d = 0);
    NONFAILING(*(uint8_t*)0x2000000e = 0);
    NONFAILING(*(uint8_t*)0x2000000f = 0);
    NONFAILING(*(uint8_t*)0x20000010 = 0);
    NONFAILING(*(uint8_t*)0x20000011 = 0);
    NONFAILING(*(uint8_t*)0x20000012 = 0);
    NONFAILING(*(uint8_t*)0x20000013 = 0);
    NONFAILING(*(uint8_t*)0x20000014 = 0);
    NONFAILING(*(uint8_t*)0x20000015 = 0);
    NONFAILING(*(uint8_t*)0x20000016 = 0);
    NONFAILING(*(uint8_t*)0x20000017 = 1);
    NONFAILING(*(uint32_t*)0x20000018 = 0);
    syscall(__NR_connect, r[2], 0x20000000, 0x1c);
    break;
  case 19:
    syscall(__NR_setsockopt, r[2], 0x29, 0x3c, 0, 0);
    break;
  case 20:
    syscall(__NR_setsockopt, r[2], 0x29, 0x3e, 0, 0);
    break;
  case 21:
    syscall(__NR_sendmmsg, r[2], 0, 0, 0);
    break;
  case 22:
    syscall(__NR_openat, 0xffffff9c, 0, 0, 0);
    break;
  case 23:
    syscall(__NR_getsockopt, -1, 0x84, 2, 0, 0);
    break;
  case 24:
    syscall(__NR_perf_event_open, 0, 0, 0, -1, 0);
    break;
  case 25:
    syscall(__NR_stat, 0, 0);
    break;
  case 26:
    syscall(__NR_pipe, 0);
    break;
  case 27:
    res = syscall(__NR_getpid);
    if (res != -1)
      r[3] = res;
    break;
  case 28:
    syscall(__NR_getpriority, 3, r[3]);
    break;
  case 29:
    syscall(__NR_lstat, 0, 0);
    break;
  case 30:
    syscall(__NR_ioctl, -1, 0x8904, 0);
    break;
  case 31:
    syscall(__NR_write, -1, 0, 0);
    break;
  case 32:
    syscall(__NR_keyctl, 0x16, 0, 0, 0, 0);
    break;
  case 33:
    syscall(__NR_ioctl, -1, 0x80404509, 0);
    break;
  case 34:
    NONFAILING(memcpy((void*)0x200001c0, "/dev/loop#\000", 11));
    res = syz_open_dev(0x200001c0, 0, 0);
    if (res != -1)
      r[4] = res;
    break;
  case 35:
    syscall(__NR_ioctl, r[4], 0x1276, 0);
    break;
  case 36:
    syscall(__NR_ioctl, -1, 0x400c55cb, 0);
    break;
  case 37:
    NONFAILING(*(uint8_t*)0x20000080 = 0);
    NONFAILING(*(uint8_t*)0x20000081 = 0);
    NONFAILING(*(uint8_t*)0x20000082 = 0);
    NONFAILING(*(uint8_t*)0x20000083 = 0);
    NONFAILING(*(uint8_t*)0x20000084 = 0);
    NONFAILING(*(uint8_t*)0x20000085 = 0);
    NONFAILING(*(uint8_t*)0x20000086 = 0);
    NONFAILING(*(uint8_t*)0x20000087 = 0);
    NONFAILING(*(uint8_t*)0x20000088 = 0);
    NONFAILING(*(uint8_t*)0x20000089 = 0);
    NONFAILING(*(uint8_t*)0x2000008a = 0);
    NONFAILING(*(uint8_t*)0x2000008b = 0);
    NONFAILING(*(uint8_t*)0x2000008c = 0);
    NONFAILING(*(uint8_t*)0x2000008d = 0);
    NONFAILING(*(uint8_t*)0x2000008e = 0);
    NONFAILING(*(uint8_t*)0x2000008f = 0);
    NONFAILING(*(uint8_t*)0x20000090 = 0);
    NONFAILING(*(uint8_t*)0x20000091 = 0);
    NONFAILING(*(uint8_t*)0x20000092 = 0);
    NONFAILING(*(uint8_t*)0x20000093 = 0);
    NONFAILING(*(uint8_t*)0x20000094 = 0);
    NONFAILING(*(uint8_t*)0x20000095 = 0);
    NONFAILING(*(uint8_t*)0x20000096 = 0);
    NONFAILING(*(uint8_t*)0x20000097 = 0);
    NONFAILING(*(uint8_t*)0x20000098 = 0);
    NONFAILING(*(uint8_t*)0x20000099 = 0);
    NONFAILING(*(uint8_t*)0x2000009a = 0);
    NONFAILING(*(uint8_t*)0x2000009b = 0);
    NONFAILING(*(uint8_t*)0x2000009c = 0);
    NONFAILING(*(uint8_t*)0x2000009d = 0);
    NONFAILING(*(uint8_t*)0x2000009e = 0);
    NONFAILING(*(uint8_t*)0x2000009f = 0);
    NONFAILING(*(uint16_t*)0x200000a0 = 0);
    NONFAILING(*(uint32_t*)0x200000a4 = 0x400);
    NONFAILING(*(uint32_t*)0x200000a8 = 0x3fa);
    NONFAILING(*(uint64_t*)0x200000b0 = 0x10000);
    NONFAILING(*(uint64_t*)0x200000b8 = 0);
    NONFAILING(*(uint32_t*)0x200000c0 = 0);
    syscall(__NR_ioctl, r[4], 0xc0481273, 0x20000080);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  install_segv_handler();
  loop();
  return 0;
}
