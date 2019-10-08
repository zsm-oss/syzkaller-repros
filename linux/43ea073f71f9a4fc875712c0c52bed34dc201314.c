// WARNING: kmalloc bug in input_mt_init_slots
// https://syzkaller.appspot.com/bug?id=43ea073f71f9a4fc875712c0c52bed34dc201314
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    memcpy((void*)0x20000000, "/dev/uinput", 12);
    res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000000, 0x805, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint16_t*)0x20000040 = 0x2f;
    *(uint32_t*)0x20000044 = 0;
    *(uint32_t*)0x20000048 = 0;
    *(uint32_t*)0x2000004c = 0;
    *(uint32_t*)0x20000050 = 0;
    *(uint32_t*)0x20000054 = 0;
    *(uint32_t*)0x20000058 = 0;
    syscall(__NR_ioctl, r[0], 0x401c5504, 0x20000040);
    break;
  case 2:
    syscall(__NR_ioctl, r[0], 0x40045564, 3);
    break;
  case 3:
    memcpy((void*)0x20000540,
           "\x73\x79\x7a\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           80);
    *(uint16_t*)0x20000590 = 0;
    *(uint16_t*)0x20000592 = 0xe4b9;
    *(uint16_t*)0x20000594 = 6;
    *(uint16_t*)0x20000596 = 8;
    *(uint32_t*)0x20000598 = 0x54;
    *(uint32_t*)0x2000059c = 2;
    *(uint32_t*)0x200005a0 = 0x107;
    *(uint32_t*)0x200005a4 = 5;
    *(uint32_t*)0x200005a8 = 0x884;
    *(uint32_t*)0x200005ac = 5;
    *(uint32_t*)0x200005b0 = 0xd5;
    *(uint32_t*)0x200005b4 = 0xfff;
    *(uint32_t*)0x200005b8 = 0xfffffff9;
    *(uint32_t*)0x200005bc = 0x200;
    *(uint32_t*)0x200005c0 = 0;
    *(uint32_t*)0x200005c4 = 3;
    *(uint32_t*)0x200005c8 = 7;
    *(uint32_t*)0x200005cc = 8;
    *(uint32_t*)0x200005d0 = 7;
    *(uint32_t*)0x200005d4 = 5;
    *(uint32_t*)0x200005d8 = 6;
    *(uint32_t*)0x200005dc = 5;
    *(uint32_t*)0x200005e0 = 6;
    *(uint32_t*)0x200005e4 = 0x200;
    *(uint32_t*)0x200005e8 = 0x97;
    *(uint32_t*)0x200005ec = 3;
    *(uint32_t*)0x200005f0 = 0x1cb;
    *(uint32_t*)0x200005f4 = 7;
    *(uint32_t*)0x200005f8 = 0x80000000;
    *(uint32_t*)0x200005fc = 0x3bc;
    *(uint32_t*)0x20000600 = 0x7b5970f7;
    *(uint32_t*)0x20000604 = 7;
    *(uint32_t*)0x20000608 = 0x1ff;
    *(uint32_t*)0x2000060c = 0xca7;
    *(uint32_t*)0x20000610 = 0x80000000;
    *(uint32_t*)0x20000614 = 4;
    *(uint32_t*)0x20000618 = 0x81;
    *(uint32_t*)0x2000061c = 0x100;
    *(uint32_t*)0x20000620 = 1;
    *(uint32_t*)0x20000624 = 0xfffffeff;
    *(uint32_t*)0x20000628 = 6;
    *(uint32_t*)0x2000062c = 2;
    *(uint32_t*)0x20000630 = 0xffff;
    *(uint32_t*)0x20000634 = 8;
    *(uint32_t*)0x20000638 = 4;
    *(uint32_t*)0x2000063c = 5;
    *(uint32_t*)0x20000640 = 0x81;
    *(uint32_t*)0x20000644 = 6;
    *(uint32_t*)0x20000648 = 7;
    *(uint32_t*)0x2000064c = 0x7ff;
    *(uint32_t*)0x20000650 = 5;
    *(uint32_t*)0x20000654 = 7;
    *(uint32_t*)0x20000658 = 0xfffffffc;
    *(uint32_t*)0x2000065c = 0x40;
    *(uint32_t*)0x20000660 = 0;
    *(uint32_t*)0x20000664 = 0x20;
    *(uint32_t*)0x20000668 = 0x14e;
    *(uint32_t*)0x2000066c = 1;
    *(uint32_t*)0x20000670 = 5;
    *(uint32_t*)0x20000674 = 1;
    *(uint32_t*)0x20000678 = 6;
    *(uint32_t*)0x2000067c = 0x191;
    *(uint32_t*)0x20000680 = 0x95c;
    *(uint32_t*)0x20000684 = 3;
    *(uint32_t*)0x20000688 = 9;
    *(uint32_t*)0x2000068c = 0xff;
    *(uint32_t*)0x20000690 = 0x400;
    *(uint32_t*)0x20000694 = 4;
    *(uint32_t*)0x20000698 = 0;
    *(uint32_t*)0x2000069c = 0xf0c;
    *(uint32_t*)0x200006a0 = 0;
    *(uint32_t*)0x200006a4 = 0x8000;
    *(uint32_t*)0x200006a8 = 5;
    *(uint32_t*)0x200006ac = 7;
    *(uint32_t*)0x200006b0 = 5;
    *(uint32_t*)0x200006b4 = 0;
    *(uint32_t*)0x200006b8 = 5;
    *(uint32_t*)0x200006bc = 0x207;
    *(uint32_t*)0x200006c0 = 0x8000;
    *(uint32_t*)0x200006c4 = 0;
    *(uint32_t*)0x200006c8 = 0xfff;
    *(uint32_t*)0x200006cc = 7;
    *(uint32_t*)0x200006d0 = 3;
    *(uint32_t*)0x200006d4 = 0;
    *(uint32_t*)0x200006d8 = 0xa79;
    *(uint32_t*)0x200006dc = 0x61e1;
    *(uint32_t*)0x200006e0 = 0x101;
    *(uint32_t*)0x200006e4 = 0x200;
    *(uint32_t*)0x200006e8 = 4;
    *(uint32_t*)0x200006ec = 0x200;
    *(uint32_t*)0x200006f0 = 1;
    *(uint32_t*)0x200006f4 = 0xe57;
    *(uint32_t*)0x200006f8 = 5;
    *(uint32_t*)0x200006fc = 0;
    *(uint32_t*)0x20000700 = 0;
    *(uint32_t*)0x20000704 = 1;
    *(uint32_t*)0x20000708 = 1;
    *(uint32_t*)0x2000070c = 0;
    *(uint32_t*)0x20000710 = 7;
    *(uint32_t*)0x20000714 = 7;
    *(uint32_t*)0x20000718 = 0;
    *(uint32_t*)0x2000071c = 6;
    *(uint32_t*)0x20000720 = 1;
    *(uint32_t*)0x20000724 = 2;
    *(uint32_t*)0x20000728 = 1;
    *(uint32_t*)0x2000072c = 9;
    *(uint32_t*)0x20000730 = 0;
    *(uint32_t*)0x20000734 = 5;
    *(uint32_t*)0x20000738 = 5;
    *(uint32_t*)0x2000073c = 6;
    *(uint32_t*)0x20000740 = 6;
    *(uint32_t*)0x20000744 = 4;
    *(uint32_t*)0x20000748 = 1;
    *(uint32_t*)0x2000074c = 1;
    *(uint32_t*)0x20000750 = 0x400;
    *(uint32_t*)0x20000754 = 0x1000;
    *(uint32_t*)0x20000758 = 0x9c8;
    *(uint32_t*)0x2000075c = 7;
    *(uint32_t*)0x20000760 = 0xc86;
    *(uint32_t*)0x20000764 = 0;
    *(uint32_t*)0x20000768 = 0x8000;
    *(uint32_t*)0x2000076c = 9;
    *(uint32_t*)0x20000770 = 4;
    *(uint32_t*)0x20000774 = 2;
    *(uint32_t*)0x20000778 = 0x3798;
    *(uint32_t*)0x2000077c = 0x8af;
    *(uint32_t*)0x20000780 = 4;
    *(uint32_t*)0x20000784 = 0x3ff;
    *(uint32_t*)0x20000788 = 0x46e;
    *(uint32_t*)0x2000078c = 2;
    *(uint32_t*)0x20000790 = 5;
    *(uint32_t*)0x20000794 = 8;
    *(uint32_t*)0x20000798 = 2;
    *(uint32_t*)0x2000079c = 0;
    *(uint32_t*)0x200007a0 = 0x80000000;
    *(uint32_t*)0x200007a4 = 2;
    *(uint32_t*)0x200007a8 = 0x207;
    *(uint32_t*)0x200007ac = 8;
    *(uint32_t*)0x200007b0 = 0x7fff;
    *(uint32_t*)0x200007b4 = 0x1000;
    *(uint32_t*)0x200007b8 = 0xfffffff9;
    *(uint32_t*)0x200007bc = 0x80e;
    *(uint32_t*)0x200007c0 = 4;
    *(uint32_t*)0x200007c4 = 5;
    *(uint32_t*)0x200007c8 = 0;
    *(uint32_t*)0x200007cc = 8;
    *(uint32_t*)0x200007d0 = 9;
    *(uint32_t*)0x200007d4 = 5;
    *(uint32_t*)0x200007d8 = 9;
    *(uint32_t*)0x200007dc = 0xff;
    *(uint32_t*)0x200007e0 = 5;
    *(uint32_t*)0x200007e4 = 0xc23f;
    *(uint32_t*)0x200007e8 = 8;
    *(uint32_t*)0x200007ec = 1;
    *(uint32_t*)0x200007f0 = 4;
    *(uint32_t*)0x200007f4 = 0xd4;
    *(uint32_t*)0x200007f8 = 0xffff104f;
    *(uint32_t*)0x200007fc = 1;
    *(uint32_t*)0x20000800 = 0xa9d;
    *(uint32_t*)0x20000804 = 6;
    *(uint32_t*)0x20000808 = 0x800;
    *(uint32_t*)0x2000080c = 0x25;
    *(uint32_t*)0x20000810 = 6;
    *(uint32_t*)0x20000814 = 0x8000;
    *(uint32_t*)0x20000818 = 7;
    *(uint32_t*)0x2000081c = 0x200001;
    *(uint32_t*)0x20000820 = 0x3f;
    *(uint32_t*)0x20000824 = 0x8001;
    *(uint32_t*)0x20000828 = 0;
    *(uint32_t*)0x2000082c = 7;
    *(uint32_t*)0x20000830 = 7;
    *(uint32_t*)0x20000834 = 0xab8;
    *(uint32_t*)0x20000838 = 0xe078;
    *(uint32_t*)0x2000083c = 0xe2;
    *(uint32_t*)0x20000840 = 0x8000;
    *(uint32_t*)0x20000844 = 2;
    *(uint32_t*)0x20000848 = 0x8000;
    *(uint32_t*)0x2000084c = 0;
    *(uint32_t*)0x20000850 = 1;
    *(uint32_t*)0x20000854 = 1;
    *(uint32_t*)0x20000858 = 0x80;
    *(uint32_t*)0x2000085c = 0x1f;
    *(uint32_t*)0x20000860 = 0x101;
    *(uint32_t*)0x20000864 = 2;
    *(uint32_t*)0x20000868 = 0x501;
    *(uint32_t*)0x2000086c = 0xbdc2;
    *(uint32_t*)0x20000870 = 0xfffffffe;
    *(uint32_t*)0x20000874 = 0x7fff;
    *(uint32_t*)0x20000878 = 0x800;
    *(uint32_t*)0x2000087c = 9;
    *(uint32_t*)0x20000880 = 6;
    *(uint32_t*)0x20000884 = 9;
    *(uint32_t*)0x20000888 = 0x47c51446;
    *(uint32_t*)0x2000088c = 0x40000;
    *(uint32_t*)0x20000890 = 3;
    *(uint32_t*)0x20000894 = 2;
    *(uint32_t*)0x20000898 = 9;
    *(uint32_t*)0x2000089c = 9;
    *(uint32_t*)0x200008a0 = 1;
    *(uint32_t*)0x200008a4 = 0x3f;
    *(uint32_t*)0x200008a8 = 0;
    *(uint32_t*)0x200008ac = 0xffff;
    *(uint32_t*)0x200008b0 = 0x20;
    *(uint32_t*)0x200008b4 = 0x7f;
    *(uint32_t*)0x200008b8 = 3;
    *(uint32_t*)0x200008bc = 4;
    *(uint32_t*)0x200008c0 = 0x6c4;
    *(uint32_t*)0x200008c4 = 0;
    *(uint32_t*)0x200008c8 = 6;
    *(uint32_t*)0x200008cc = 0;
    *(uint32_t*)0x200008d0 = 0;
    *(uint32_t*)0x200008d4 = 0x5c5;
    *(uint32_t*)0x200008d8 = 1;
    *(uint32_t*)0x200008dc = 4;
    *(uint32_t*)0x200008e0 = 0;
    *(uint32_t*)0x200008e4 = 0xfffffffb;
    *(uint32_t*)0x200008e8 = 1;
    *(uint32_t*)0x200008ec = 0xfffffff9;
    *(uint32_t*)0x200008f0 = 5;
    *(uint32_t*)0x200008f4 = 0x810;
    *(uint32_t*)0x200008f8 = 1;
    *(uint32_t*)0x200008fc = 0xfffffffd;
    *(uint32_t*)0x20000900 = 7;
    *(uint32_t*)0x20000904 = 0xb457;
    *(uint32_t*)0x20000908 = 9;
    *(uint32_t*)0x2000090c = 0x7f;
    *(uint32_t*)0x20000910 = 0x89;
    *(uint32_t*)0x20000914 = 5;
    *(uint32_t*)0x20000918 = 0x80;
    *(uint32_t*)0x2000091c = 5;
    *(uint32_t*)0x20000920 = 8;
    *(uint32_t*)0x20000924 = 4;
    *(uint32_t*)0x20000928 = 0xfff;
    *(uint32_t*)0x2000092c = 0x75;
    *(uint32_t*)0x20000930 = 0x81;
    *(uint32_t*)0x20000934 = 0x7fffffff;
    *(uint32_t*)0x20000938 = 0xc5f0;
    *(uint32_t*)0x2000093c = 3;
    *(uint32_t*)0x20000940 = 0x80000000;
    *(uint32_t*)0x20000944 = 0xc4;
    *(uint32_t*)0x20000948 = 0xf47c;
    *(uint32_t*)0x2000094c = 0x547;
    *(uint32_t*)0x20000950 = 0x7d4;
    *(uint32_t*)0x20000954 = 0x10001;
    *(uint32_t*)0x20000958 = 0x1ed;
    *(uint32_t*)0x2000095c = 0;
    *(uint32_t*)0x20000960 = 0x800;
    *(uint32_t*)0x20000964 = 3;
    *(uint32_t*)0x20000968 = 0;
    *(uint32_t*)0x2000096c = 0xa40;
    *(uint32_t*)0x20000970 = 9;
    *(uint32_t*)0x20000974 = -1;
    *(uint32_t*)0x20000978 = 2;
    *(uint32_t*)0x2000097c = 0x8001;
    *(uint32_t*)0x20000980 = 0x6b;
    *(uint32_t*)0x20000984 = 1;
    *(uint32_t*)0x20000988 = 0x3ff;
    *(uint32_t*)0x2000098c = 4;
    *(uint32_t*)0x20000990 = 5;
    *(uint32_t*)0x20000994 = 0xadf;
    *(uint32_t*)0x20000998 = 2;
    syscall(__NR_write, r[0], 0x20000540, 0x45c);
    break;
  case 4:
    syscall(__NR_ioctl, r[0], 0x5501);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
