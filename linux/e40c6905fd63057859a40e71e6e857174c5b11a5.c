// KASAN: use-after-free Read in tls_sk_proto_close (2)
// https://syzkaller.appspot.com/bug?id=e40c6905fd63057859a40e71e6e857174c5b11a5
// status:fixed
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

uint64_t r[1] = {0xffffffffffffffff};
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 0xa, 1, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint32_t*)0x200000c0 = 1;
    syscall(__NR_setsockopt, r[0], 6, 0x13, 0x200000c0, 0x1d4);
    break;
  case 2:
    *(uint16_t*)0x20000080 = 0xa;
    *(uint16_t*)0x20000082 = htobe16(0);
    *(uint32_t*)0x20000084 = 0;
    *(uint8_t*)0x20000088 = 0;
    *(uint8_t*)0x20000089 = 0;
    *(uint8_t*)0x2000008a = 0;
    *(uint8_t*)0x2000008b = 0;
    *(uint8_t*)0x2000008c = 0;
    *(uint8_t*)0x2000008d = 0;
    *(uint8_t*)0x2000008e = 0;
    *(uint8_t*)0x2000008f = 0;
    *(uint8_t*)0x20000090 = 0;
    *(uint8_t*)0x20000091 = 0;
    *(uint8_t*)0x20000092 = 0;
    *(uint8_t*)0x20000093 = 0;
    *(uint8_t*)0x20000094 = 0;
    *(uint8_t*)0x20000095 = 0;
    *(uint8_t*)0x20000096 = 0;
    *(uint8_t*)0x20000097 = 0;
    *(uint32_t*)0x20000098 = 0;
    syscall(__NR_connect, r[0], 0x20000080, 0x1c);
    break;
  case 3:
    memcpy((void*)0x20000040, "tls", 4);
    syscall(__NR_setsockopt, r[0], 6, 0x1f, 0x20000040, 0x13a);
    break;
  case 4:
    *(uint16_t*)0x20000100 = 0x303;
    *(uint16_t*)0x20000102 = 0x33;
    syscall(__NR_setsockopt, r[0], 0x11a, 1, 0x20000100, 0x28);
    break;
  case 5:
    *(uint8_t*)0x20000040 = -1;
    *(uint8_t*)0x20000041 = -1;
    *(uint8_t*)0x20000042 = -1;
    *(uint8_t*)0x20000043 = -1;
    *(uint8_t*)0x20000044 = -1;
    *(uint8_t*)0x20000045 = -1;
    *(uint8_t*)0x20000046 = 0;
    *(uint8_t*)0x20000047 = 0;
    *(uint8_t*)0x20000048 = 0;
    *(uint8_t*)0x20000049 = 0;
    *(uint8_t*)0x2000004a = 0;
    *(uint8_t*)0x2000004b = 0;
    *(uint16_t*)0x2000004c = htobe16(0x806);
    *(uint16_t*)0x2000004e = htobe16(0);
    *(uint16_t*)0x20000050 = htobe16(0);
    *(uint8_t*)0x20000052 = 6;
    *(uint8_t*)0x20000053 = 0;
    *(uint16_t*)0x20000054 = htobe16(0);
    *(uint8_t*)0x20000056 = 0xaa;
    *(uint8_t*)0x20000057 = 0xaa;
    *(uint8_t*)0x20000058 = 0xaa;
    *(uint8_t*)0x20000059 = 0xaa;
    *(uint8_t*)0x2000005a = 0xaa;
    *(uint8_t*)0x2000005b = 0xbb;
    *(uint8_t*)0x2000005c = 0xaa;
    *(uint8_t*)0x2000005d = 0xaa;
    *(uint8_t*)0x2000005e = 0xaa;
    *(uint8_t*)0x2000005f = 0xaa;
    *(uint8_t*)0x20000060 = 0xaa;
    *(uint8_t*)0x20000061 = 0xaa;
    memcpy((void*)0x20000062,
           "\x1d\x4c\x3c\x44\x39\x2c\xf9\x6b\x83\x72\x13\x01\x88\x73\x92\x36",
           16);
    *(uint32_t*)0x20000000 = 0;
    *(uint32_t*)0x20000004 = 0;
    *(uint32_t*)0x20000008 = 0;
    *(uint32_t*)0x2000000c = 0;
    *(uint32_t*)0x20000010 = 0;
    *(uint32_t*)0x20000014 = 0;
    break;
  }
}

void loop()
{
  execute(6);
  collide = 1;
  execute(6);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
