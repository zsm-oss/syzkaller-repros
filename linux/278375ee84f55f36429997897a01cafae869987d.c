// INFO: task hung in lock_mount
// https://syzkaller.appspot.com/bug?id=278375ee84f55f36429997897a01cafae869987d
// status:open
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

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
        if (running)
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

unsigned long long procid;
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_fcntl, -1, 0, -1);
    break;
  case 1:
    memcpy((void*)0x20000000, "./file0", 8);
    syscall(__NR_mkdir, 0x20000000, 0);
    break;
  case 2:
    memcpy((void*)0x20000380, "./file0", 8);
    memcpy((void*)0x20026ff8, "./file0", 8);
    memcpy((void*)0x20000300, "ramfs", 6);
    syscall(__NR_mount, 0x20000380, 0x20026ff8, 0x20000300, 0, 0x200007c0);
    break;
  case 3:
    *(uint16_t*)0x200001c0 = 4;
    *(uint16_t*)0x200001c2 = htobe16(0);
    *(uint32_t*)0x200001c4 = htobe32(0);
    memcpy((void*)0x200001c8, "\x0a\x04\xdf\xa8\x09\xba", 6);
    *(uint8_t*)0x200001ce = 0;
    *(uint8_t*)0x200001cf = 0;
    syscall(__NR_bind, -1, 0x200001c0, 0x80);
    break;
  case 4:
    memcpy((void*)0x20d04000, "./file0", 8);
    memcpy((void*)0x20903000, "./file0", 8);
    memcpy((void*)0x20000340, "bdev", 5);
    syscall(__NR_mount, 0x20d04000, 0x20903000, 0x20000340, 0x100000,
            0x200002c0);
    break;
  case 5:
    memcpy((void*)0x20000240, "./file0", 8);
    memcpy((void*)0x20000280, ".", 1);
    memcpy((void*)0x20000040, "\x04\x5b\x89\x8f\x73", 5);
    syscall(__NR_mount, 0x20000240, 0x20000280, 0x20000040, 0x1004, 0);
    break;
  case 6:
    memcpy((void*)0x20000000, "./file0", 8);
    memcpy((void*)0x200000c0, ".", 1);
    memcpy((void*)0x20000140, "vxfs", 5);
    syscall(__NR_mount, 0x20000000, 0x200000c0, 0x20000140, 0x3080, 0x20000200);
    break;
  case 7:
    memcpy((void*)0x200008c0, ".", 1);
    memcpy((void*)0x20000080, ".", 1);
    memcpy((void*)0x20753000, "mslos", 6);
    syscall(__NR_mount, 0x200008c0, 0x20000080, 0x20753000, 0x5010, 0x20000ac0);
    break;
  case 8:
    memcpy((void*)0x20fb6000, "./file0", 8);
    memcpy((void*)0x20d78000, ".", 1);
    memcpy((void*)0x20fdb000, "ubifs", 6);
    syscall(__NR_mount, 0x20fb6000, 0x20d78000, 0x20fdb000, 0x1003, 0);
    break;
  case 9:
    memcpy((void*)0x202b9ff8, "./file0", 8);
    memcpy((void*)0x20cbeff8, "./file0", 8);
    memcpy((void*)0x205f7ffa, "ramfs", 6);
    syscall(__NR_mount, 0x202b9ff8, 0x20cbeff8, 0x205f7ffa, 0, 0x2002f000);
    break;
  case 10:
    memcpy((void*)0x20000100, "./file0/file0", 14);
    syscall(__NR_mkdir, 0x20000100, 0);
    break;
  case 11:
    memcpy((void*)0x20000400, "./file0/file1", 14);
    memcpy((void*)0x200003c0, "./file0", 8);
    memcpy((void*)0x20753000, "qnx6", 5);
    syscall(__NR_mount, 0x20000400, 0x200003c0, 0x20753000, 0x5410, 0x200002c0);
    break;
  case 12:
    memcpy((void*)0x20000040, "./file0/file0", 14);
    memcpy((void*)0x20000140, "./file0/file1", 14);
    syscall(__NR_rename, 0x20000040, 0x20000140);
    break;
  }
}

void execute_one()
{
  execute(13);
  collide = 1;
  execute(13);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}
