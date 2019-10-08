// KASAN: slab-out-of-bounds Read in __sctp_v6_cmp_addr
// https://syzkaller.appspot.com/bug?id=40eccf33adab0333176aac1200041f8e29dce3e9
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

struct csum_inet {
  uint32_t acc;
};

static void csum_inet_init(struct csum_inet* csum)
{
  csum->acc = 0;
}

static void csum_inet_update(struct csum_inet* csum, const uint8_t* data,
                             size_t length)
{
  if (length == 0)
    return;

  size_t i;
  for (i = 0; i < length - 1; i += 2)
    csum->acc += *(uint16_t*)&data[i];

  if (length & 1)
    csum->acc += (uint16_t)data[length - 1];

  while (csum->acc > 0xffff)
    csum->acc = (csum->acc & 0xffff) + (csum->acc >> 16);
}

static uint16_t csum_inet_digest(struct csum_inet* csum)
{
  return ~csum->acc;
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

uint64_t r[1] = {0xffffffffffffffff};
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 0xa, 1, 0x84);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint16_t*)0x205ba000 = 2;
    *(uint16_t*)0x205ba002 = htobe16(0x4e20);
    *(uint32_t*)0x205ba004 = htobe32(0x7f000001);
    *(uint8_t*)0x205ba008 = 0;
    *(uint8_t*)0x205ba009 = 0;
    *(uint8_t*)0x205ba00a = 0;
    *(uint8_t*)0x205ba00b = 0;
    *(uint8_t*)0x205ba00c = 0;
    *(uint8_t*)0x205ba00d = 0;
    *(uint8_t*)0x205ba00e = 0;
    *(uint8_t*)0x205ba00f = 0;
    syscall(__NR_setsockopt, r[0], 0x84, 0x64, 0x205ba000, 0x10);
    break;
  case 2:
    *(uint8_t*)0x20694ffe = -1;
    *(uint8_t*)0x20694fff = -1;
    *(uint8_t*)0x20695000 = -1;
    *(uint8_t*)0x20695001 = -1;
    *(uint8_t*)0x20695002 = -1;
    *(uint8_t*)0x20695003 = -1;
    *(uint8_t*)0x20695004 = 1;
    *(uint8_t*)0x20695005 = 0x80;
    *(uint8_t*)0x20695006 = 0xc2;
    *(uint8_t*)0x20695007 = 0;
    *(uint8_t*)0x20695008 = 0;
    *(uint8_t*)0x20695009 = 0;
    *(uint16_t*)0x2069500a = htobe16(0x86dd);
    STORE_BY_BITMASK(uint8_t, 0x2069500c, 0, 0, 4);
    STORE_BY_BITMASK(uint8_t, 0x2069500c, 6, 4, 4);
    memcpy((void*)0x2069500d, "\x06\xf5\x26", 3);
    *(uint16_t*)0x20695010 = htobe16(8);
    *(uint8_t*)0x20695012 = 0x11;
    *(uint8_t*)0x20695013 = 0;
    *(uint8_t*)0x20695014 = 0xfe;
    *(uint8_t*)0x20695015 = 0x80;
    *(uint8_t*)0x20695016 = 0;
    *(uint8_t*)0x20695017 = 0;
    *(uint8_t*)0x20695018 = 0;
    *(uint8_t*)0x20695019 = 0;
    *(uint8_t*)0x2069501a = 0;
    *(uint8_t*)0x2069501b = 0;
    *(uint8_t*)0x2069501c = 0;
    *(uint8_t*)0x2069501d = 0;
    *(uint8_t*)0x2069501e = 0;
    *(uint8_t*)0x2069501f = 0;
    *(uint8_t*)0x20695020 = 0;
    *(uint8_t*)0x20695021 = 0;
    *(uint8_t*)0x20695022 = 0;
    *(uint8_t*)0x20695023 = -1;
    *(uint8_t*)0x20695024 = -1;
    *(uint8_t*)0x20695025 = 2;
    *(uint8_t*)0x20695026 = 0;
    *(uint8_t*)0x20695027 = 0;
    *(uint8_t*)0x20695028 = 0;
    *(uint8_t*)0x20695029 = 0;
    *(uint8_t*)0x2069502a = 0;
    *(uint8_t*)0x2069502b = 0;
    *(uint8_t*)0x2069502c = 0;
    *(uint8_t*)0x2069502d = 0;
    *(uint8_t*)0x2069502e = 0;
    *(uint8_t*)0x2069502f = 0;
    *(uint8_t*)0x20695030 = 0;
    *(uint8_t*)0x20695031 = 0;
    *(uint8_t*)0x20695032 = 0;
    *(uint8_t*)0x20695033 = 1;
    *(uint16_t*)0x20695034 = htobe16(0);
    *(uint16_t*)0x20695036 = htobe16(2);
    *(uint16_t*)0x20695038 = htobe16(8);
    *(uint16_t*)0x2069503a = 0;
    *(uint32_t*)0x20775000 = 0;
    *(uint32_t*)0x20775004 = 0;
    *(uint32_t*)0x20775008 = 0;
    *(uint32_t*)0x2077500c = 0;
    *(uint32_t*)0x20775010 = 0;
    *(uint32_t*)0x20775014 = 0;
    struct csum_inet csum_1;
    csum_inet_init(&csum_1);
    csum_inet_update(&csum_1, (const uint8_t*)0x20695014, 16);
    csum_inet_update(&csum_1, (const uint8_t*)0x20695024, 16);
    uint32_t csum_1_chunk_2 = 0x8000000;
    csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_2, 4);
    uint32_t csum_1_chunk_3 = 0x11000000;
    csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_3, 4);
    csum_inet_update(&csum_1, (const uint8_t*)0x20695034, 8);
    *(uint16_t*)0x2069503a = csum_inet_digest(&csum_1);
    break;
  }
}

void loop()
{
  execute(3);
  collide = 1;
  execute(3);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
