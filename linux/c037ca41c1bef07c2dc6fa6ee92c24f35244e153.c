// WARNING in xfrm_policy_insert
// https://syzkaller.appspot.com/bug?id=c037ca41c1bef07c2dc6fa6ee92c24f35244e153
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

static void test();

void loop()
{
  while (1) {
    test();
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

long r[3];
uint64_t procid;
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
    break;
  case 1:
    if (syscall(__NR_socketpair, 0xa, 0x80a, 0xf, 0x20000000) != -1)
      r[0] = *(uint32_t*)0x20000000;
    break;
  case 2:
    *(uint32_t*)0x20705000 = 0x10;
    syscall(__NR_accept, r[0], 0x20000000, 0x20705000);
    break;
  case 3:
    r[1] = syscall(__NR_socket, 0x10, 3, 6);
    break;
  case 4:
    *(uint64_t*)0x2014dfc8 = 0x206a2ff4;
    *(uint32_t*)0x2014dfd0 = 0xc;
    *(uint64_t*)0x2014dfd8 = 0x2054aff0;
    *(uint64_t*)0x2014dfe0 = 1;
    *(uint64_t*)0x2014dfe8 = 0;
    *(uint64_t*)0x2014dff0 = 0;
    *(uint32_t*)0x2014dff8 = 0;
    *(uint16_t*)0x206a2ff4 = 0x10;
    *(uint16_t*)0x206a2ff6 = 0;
    *(uint32_t*)0x206a2ff8 = 0;
    *(uint32_t*)0x206a2ffc = 0;
    *(uint64_t*)0x2054aff0 = 0x207c7000;
    *(uint64_t*)0x2054aff8 = 0xb8;
    *(uint32_t*)0x207c7000 = 0xb8;
    *(uint16_t*)0x207c7004 = 0x19;
    *(uint16_t*)0x207c7006 = 0x401;
    *(uint32_t*)0x207c7008 = 0;
    *(uint32_t*)0x207c700c = 0;
    *(uint32_t*)0x207c7010 = htobe32(0xe0000001);
    *(uint8_t*)0x207c7020 = 0xac;
    *(uint8_t*)0x207c7021 = 0x14;
    *(uint8_t*)0x207c7022 = 0;
    *(uint8_t*)0x207c7023 = 0;
    *(uint16_t*)0x207c7030 = 0;
    *(uint16_t*)0x207c7032 = htobe16(0);
    *(uint16_t*)0x207c7034 = 0;
    *(uint16_t*)0x207c7036 = htobe16(0);
    *(uint16_t*)0x207c7038 = 2;
    *(uint8_t*)0x207c703a = 0;
    *(uint8_t*)0x207c703b = 0;
    *(uint8_t*)0x207c703c = 0;
    *(uint32_t*)0x207c7040 = 0;
    *(uint32_t*)0x207c7044 = 0;
    *(uint64_t*)0x207c7048 = 0;
    *(uint64_t*)0x207c7050 = 0;
    *(uint64_t*)0x207c7058 = 0;
    *(uint64_t*)0x207c7060 = 0;
    *(uint64_t*)0x207c7068 = 0;
    *(uint64_t*)0x207c7070 = 0;
    *(uint64_t*)0x207c7078 = 0;
    *(uint64_t*)0x207c7080 = 0;
    *(uint64_t*)0x207c7088 = 0;
    *(uint64_t*)0x207c7090 = 0;
    *(uint64_t*)0x207c7098 = 0;
    *(uint64_t*)0x207c70a0 = 0;
    *(uint32_t*)0x207c70a8 = 0x7f;
    *(uint32_t*)0x207c70ac = 0;
    *(uint8_t*)0x207c70b0 = 0;
    *(uint8_t*)0x207c70b1 = 0;
    *(uint8_t*)0x207c70b2 = 0;
    *(uint8_t*)0x207c70b3 = 0;
    syscall(__NR_sendmsg, r[1], 0x2014dfc8, 0);
    break;
  case 5:
    *(uint64_t*)0x2014dfc8 = 0x206a2ff4;
    *(uint32_t*)0x2014dfd0 = 0xc;
    *(uint64_t*)0x2014dfd8 = 0x2054aff0;
    *(uint64_t*)0x2014dfe0 = 1;
    *(uint64_t*)0x2014dfe8 = 0;
    *(uint64_t*)0x2014dff0 = 0;
    *(uint32_t*)0x2014dff8 = 0;
    *(uint16_t*)0x206a2ff4 = 0x10;
    *(uint16_t*)0x206a2ff6 = 0;
    *(uint32_t*)0x206a2ff8 = 0;
    *(uint32_t*)0x206a2ffc = 0;
    *(uint64_t*)0x2054aff0 = 0x207c7000;
    *(uint64_t*)0x2054aff8 = 0xb8;
    *(uint32_t*)0x207c7000 = 0xb8;
    *(uint16_t*)0x207c7004 = 0x19;
    *(uint16_t*)0x207c7006 = 0x401;
    *(uint32_t*)0x207c7008 = 0;
    *(uint32_t*)0x207c700c = 0;
    *(uint32_t*)0x207c7010 = htobe32(0xe0000001);
    *(uint8_t*)0x207c7020 = 0xac;
    *(uint8_t*)0x207c7021 = 0x14;
    *(uint8_t*)0x207c7022 = 0;
    *(uint8_t*)0x207c7023 = 0;
    *(uint16_t*)0x207c7030 = 0;
    *(uint16_t*)0x207c7032 = htobe16(0);
    *(uint16_t*)0x207c7034 = 0;
    *(uint16_t*)0x207c7036 = htobe16(0);
    *(uint16_t*)0x207c7038 = 2;
    *(uint8_t*)0x207c703a = 0;
    *(uint8_t*)0x207c703b = 0;
    *(uint8_t*)0x207c703c = 0;
    *(uint32_t*)0x207c7040 = 0;
    *(uint32_t*)0x207c7044 = 0;
    *(uint64_t*)0x207c7048 = 0;
    *(uint64_t*)0x207c7050 = 0;
    *(uint64_t*)0x207c7058 = 0;
    *(uint64_t*)0x207c7060 = 0;
    *(uint64_t*)0x207c7068 = 0;
    *(uint64_t*)0x207c7070 = 0;
    *(uint64_t*)0x207c7078 = 0;
    *(uint64_t*)0x207c7080 = 0;
    *(uint64_t*)0x207c7088 = 0;
    *(uint64_t*)0x207c7090 = 0;
    *(uint64_t*)0x207c7098 = 0;
    *(uint64_t*)0x207c70a0 = 0;
    *(uint32_t*)0x207c70a8 = 0;
    *(uint32_t*)0x207c70ac = 0;
    *(uint8_t*)0x207c70b0 = 0;
    *(uint8_t*)0x207c70b1 = 0;
    *(uint8_t*)0x207c70b2 = 0;
    *(uint8_t*)0x207c70b3 = 0;
    syscall(__NR_sendmsg, -1, 0x2014dfc8, 0);
    break;
  case 6:
    r[2] = syscall(__NR_socket, 0x10, 3, 6);
    break;
  case 7:
    *(uint64_t*)0x2014dfc8 = 0x206a2ff4;
    *(uint32_t*)0x2014dfd0 = 0xc;
    *(uint64_t*)0x2014dfd8 = 0x2054aff0;
    *(uint64_t*)0x2014dfe0 = 1;
    *(uint64_t*)0x2014dfe8 = 0;
    *(uint64_t*)0x2014dff0 = 0;
    *(uint32_t*)0x2014dff8 = 0;
    *(uint16_t*)0x206a2ff4 = 0x10;
    *(uint16_t*)0x206a2ff6 = 0;
    *(uint32_t*)0x206a2ff8 = 0;
    *(uint32_t*)0x206a2ffc = 0;
    *(uint64_t*)0x2054aff0 = 0x207c7000;
    *(uint64_t*)0x2054aff8 = 0xc4;
    *(uint32_t*)0x207c7000 = 0xc4;
    *(uint16_t*)0x207c7004 = 0x19;
    *(uint16_t*)0x207c7006 = 0x401;
    *(uint32_t*)0x207c7008 = 0;
    *(uint32_t*)0x207c700c = 0;
    *(uint32_t*)0x207c7010 = htobe32(0xe0000001);
    *(uint8_t*)0x207c7020 = 0xac;
    *(uint8_t*)0x207c7021 = 0x14;
    *(uint8_t*)0x207c7022 = 0;
    *(uint8_t*)0x207c7023 = 0;
    *(uint16_t*)0x207c7030 = 0;
    *(uint16_t*)0x207c7032 = htobe16(0);
    *(uint16_t*)0x207c7034 = 0;
    *(uint16_t*)0x207c7036 = htobe16(0);
    *(uint16_t*)0x207c7038 = 2;
    *(uint8_t*)0x207c703a = 0;
    *(uint8_t*)0x207c703b = 0;
    *(uint8_t*)0x207c703c = 0;
    *(uint32_t*)0x207c7040 = 0;
    *(uint32_t*)0x207c7044 = 0;
    *(uint64_t*)0x207c7048 = 0;
    *(uint64_t*)0x207c7050 = 0;
    *(uint64_t*)0x207c7058 = 0;
    *(uint64_t*)0x207c7060 = 0;
    *(uint64_t*)0x207c7068 = 0;
    *(uint64_t*)0x207c7070 = 0;
    *(uint64_t*)0x207c7078 = 0;
    *(uint64_t*)0x207c7080 = 0;
    *(uint64_t*)0x207c7088 = 0;
    *(uint64_t*)0x207c7090 = 0;
    *(uint64_t*)0x207c7098 = 0;
    *(uint64_t*)0x207c70a0 = 0;
    *(uint32_t*)0x207c70a8 = 0;
    *(uint32_t*)0x207c70ac = 0;
    *(uint8_t*)0x207c70b0 = 0;
    *(uint8_t*)0x207c70b1 = 0;
    *(uint8_t*)0x207c70b2 = 0;
    *(uint8_t*)0x207c70b3 = 0;
    *(uint16_t*)0x207c70b8 = 0xc;
    *(uint16_t*)0x207c70ba = 0x15;
    *(uint32_t*)0x207c70bc = 0;
    *(uint32_t*)0x207c70c0 = 3;
    syscall(__NR_sendmsg, r[2], 0x2014dfc8, 0);
    break;
  }
}

void test()
{
  memset(r, -1, sizeof(r));
  execute(8);
  collide = 1;
  execute(8);
}

int main()
{
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
