// BUG: soft lockup in __process_echoes
// https://syzkaller.appspot.com/bug?id=394dbeed8fefb909a5911101a36fff740bd0f5dc
// status:dup
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

static uintptr_t syz_open_pts(uintptr_t a0, uintptr_t a1)
{
  int ptyno = 0;
  if (ioctl(a0, TIOCGPTN, &ptyno))
    return -1;
  char buf[128];
  sprintf(buf, "/dev/pts/%d", ptyno);
  return open(buf, a1, 0);
}

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

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
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (running)
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
    memcpy((void*)0x20000100, "/dev/ptmx", 10);
    res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000100, 1, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint32_t*)0x203b3fdc = 0;
    *(uint32_t*)0x203b3fe0 = 0;
    *(uint32_t*)0x203b3fe4 = 0;
    *(uint32_t*)0x203b3fe8 = 0;
    *(uint8_t*)0x203b3fec = 0;
    *(uint8_t*)0x203b3fed = 0;
    *(uint8_t*)0x203b3fee = 0;
    *(uint8_t*)0x203b3fef = 0;
    *(uint32_t*)0x203b3ff0 = 0;
    *(uint32_t*)0x203b3ff4 = 0;
    *(uint32_t*)0x203b3ff8 = 0;
    *(uint32_t*)0x203b3ffc = 0;
    syscall(__NR_ioctl, r[0], 0x40045431, 0x203b3fdc);
    break;
  case 2:
    syscall(__NR_write, r[0], 0x20000080, 0xfffffe99);
    break;
  case 3:
    res = syz_open_pts(r[0], 0);
    if (res != -1)
      r[1] = res;
    break;
  case 4:
    syscall(__NR_ioctl, r[0], 0x5409, 8);
    break;
  case 5:
    *(uint32_t*)0x200000c0 = 0;
    *(uint32_t*)0x200000c4 = 0;
    *(uint32_t*)0x200000c8 = 0;
    *(uint32_t*)0x200000cc = 0;
    *(uint8_t*)0x200000d0 = 0;
    *(uint8_t*)0x200000d1 = 0;
    *(uint8_t*)0x200000d2 = 0;
    *(uint8_t*)0x200000d3 = 0;
    *(uint32_t*)0x200000d4 = 0;
    *(uint32_t*)0x200000d8 = 0;
    *(uint32_t*)0x200000dc = 0;
    *(uint32_t*)0x200000e0 = 0;
    syscall(__NR_ioctl, r[1], 0x5412, 0x200000c0);
    break;
  }
}

void execute_one()
{
  execute(6);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
