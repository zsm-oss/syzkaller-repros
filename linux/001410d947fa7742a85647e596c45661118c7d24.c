// KASAN: use-after-free Read in p9_poll_workfn
// https://syzkaller.appspot.com/bug?id=001410d947fa7742a85647e596c45661118c7d24
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <errno.h>
#include <linux/futex.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

static void fail(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
}

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();

  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 160 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);

  if (unshare(CLONE_NEWNS)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(0x02000000)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
}

int wait_for_loop(int pid)
{
  if (pid < 0)
    fail("sandbox fork failed");
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);

  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }
  loop();
  doexit(1);
}

static void execute_one();
extern unsigned long long procid;

static void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      fail("clone failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      execute_one();
      int fd;
      for (fd = 3; fd < 30; fd++)
        close(fd);
      doexit(0);
    }

    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      int res = waitpid(-1, &status, __WALL | WNOHANG);
      if (res == pid) {
        break;
      }
      usleep(1000);
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill(-pid, SIGKILL);
      kill(pid, SIGKILL);
      while (waitpid(-1, &status, __WALL) != pid) {
      }
      break;
    }
  }
}

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;
static int collide;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    while (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE))
      syscall(SYS_futex, &th->running, FUTEX_WAIT, 0, 0);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    __atomic_store_n(&th->running, 0, __ATOMIC_RELEASE);
    syscall(SYS_futex, &th->running, FUTEX_WAKE);
  }
  return 0;
}

static void execute(int num_calls)
{
  int call, thread;
  running = 0;
  for (call = 0; call < num_calls; call++) {
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 128 << 10);
        pthread_create(&th->th, &attr, thr, th);
      }
      if (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE)) {
        th->call = call;
        __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
        __atomic_store_n(&th->running, 1, __ATOMIC_RELEASE);
        syscall(SYS_futex, &th->running, FUTEX_WAKE);
        if (collide && call % 2)
          break;
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (__atomic_load_n(&running, __ATOMIC_RELAXED))
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    *(uint64_t*)0x20000300 = 0x200000c0;
    *(uint16_t*)0x200000c0 = 0xa;
    *(uint16_t*)0x200000c2 = htobe16(0x4e22);
    *(uint32_t*)0x200000c4 = 0;
    *(uint8_t*)0x200000c8 = -1;
    *(uint8_t*)0x200000c9 = 2;
    *(uint8_t*)0x200000ca = 0;
    *(uint8_t*)0x200000cb = 0;
    *(uint8_t*)0x200000cc = 0;
    *(uint8_t*)0x200000cd = 0;
    *(uint8_t*)0x200000ce = 0;
    *(uint8_t*)0x200000cf = 0;
    *(uint8_t*)0x200000d0 = 0;
    *(uint8_t*)0x200000d1 = 0;
    *(uint8_t*)0x200000d2 = 0;
    *(uint8_t*)0x200000d3 = 0;
    *(uint8_t*)0x200000d4 = 0;
    *(uint8_t*)0x200000d5 = 0;
    *(uint8_t*)0x200000d6 = 0;
    *(uint8_t*)0x200000d7 = 1;
    *(uint32_t*)0x200000d8 = 7;
    *(uint32_t*)0x20000308 = 0x80;
    *(uint64_t*)0x20000310 = 0x20000940;
    *(uint64_t*)0x20000318 = 0;
    *(uint64_t*)0x20000320 = 0x20000a00;
    *(uint64_t*)0x20000328 = 0;
    *(uint32_t*)0x20000330 = 0;
    syscall(__NR_sendmsg, -1, 0x20000300, 0xe803);
    break;
  case 1:
    *(uint16_t*)0x20ccb000 = 2;
    *(uint16_t*)0x20ccb002 = htobe16(0x4e20);
    *(uint32_t*)0x20ccb004 = htobe32(0);
    *(uint8_t*)0x20ccb008 = 0;
    *(uint8_t*)0x20ccb009 = 0;
    *(uint8_t*)0x20ccb00a = 0;
    *(uint8_t*)0x20ccb00b = 0;
    *(uint8_t*)0x20ccb00c = 0;
    *(uint8_t*)0x20ccb00d = 0;
    *(uint8_t*)0x20ccb00e = 0;
    *(uint8_t*)0x20ccb00f = 0;
    syscall(__NR_connect, -1, 0x20ccb000, 0x10);
    break;
  case 2:
    res = syscall(__NR_socket, 2, 6, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 3:
    *(uint16_t*)0x20000140 = 2;
    *(uint16_t*)0x20000142 = htobe16(0);
    *(uint8_t*)0x20000144 = 0xac;
    *(uint8_t*)0x20000145 = 0x14;
    *(uint8_t*)0x20000146 = 0x14;
    *(uint8_t*)0x20000147 = 0xaa;
    *(uint8_t*)0x20000148 = 0;
    *(uint8_t*)0x20000149 = 0;
    *(uint8_t*)0x2000014a = 0;
    *(uint8_t*)0x2000014b = 0;
    *(uint8_t*)0x2000014c = 0;
    *(uint8_t*)0x2000014d = 0;
    *(uint8_t*)0x2000014e = 0;
    *(uint8_t*)0x2000014f = 0;
    syscall(__NR_connect, r[0], 0x20000140, 0x10);
    break;
  case 4:
    memcpy((void*)0x20000f80, "./file0", 8);
    res = syscall(__NR_open, 0x20000f80, 0x40, 0);
    if (res != -1)
      r[1] = res;
    break;
  case 5:
    memcpy((void*)0x20000f00, "./file0", 8);
    memcpy((void*)0x20000f40, "9p", 3);
    memcpy((void*)0x20000fc0, "trans=fd,", 9);
    memcpy((void*)0x20000fc9, "rfdno", 5);
    *(uint8_t*)0x20000fce = 0x3d;
    sprintf((char*)0x20000fcf, "0x%016llx", (long long)r[1]);
    *(uint8_t*)0x20000fe1 = 0x2c;
    memcpy((void*)0x20000fe2, "wfdno", 5);
    *(uint8_t*)0x20000fe7 = 0x3d;
    sprintf((char*)0x20000fe8, "0x%016llx", (long long)r[0]);
    *(uint8_t*)0x20000ffa = 0x2c;
    *(uint8_t*)0x20000ffb = 0;
    syscall(__NR_mount, 0, 0x20000f00, 0x20000f40, 0, 0x20000fc0);
    break;
  case 6:
    *(uint16_t*)0x20000080 = 0;
    memcpy((void*)0x20000082,
           "\x2e\x2f\x66\x69\x6c\x65\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           108);
    syscall(__NR_connect, r[0], 0x20000080, 0x6e);
    break;
  }
}

void execute_one()
{
  execute(7);
  collide = 1;
  execute(7);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    do_sandbox_none();
  }
}
