// BUG: please report to dccp@vger.kernel.org => prev = 0, last = 0 at net/dccp/ccids/lib/packet_history.c:LINE/tfrc_rx_hist_sample_rtt()
// https://syzkaller.appspot.com/bug?id=0881c535c265ca965edc49c0ac3d0a9850d26eb1
// status:open
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <linux/futex.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/time.h>
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
#include <stdint.h>
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

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void test();

void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      fail("loop fork failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      test();
      doexit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      int res = waitpid(-1, &status, __WALL | WNOHANG);
      if (res == pid)
        break;
      usleep(1000);
      if (current_time_ms() - start > 5 * 1000) {
        kill(-pid, SIGKILL);
        kill(pid, SIGKILL);
        while (waitpid(-1, &status, __WALL) != pid) {
        }
        break;
      }
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
        if (running)
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

long r[3];
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
    break;
  case 1:
    r[0] = syscall(__NR_socket, 0xa, 6, 0);
    break;
  case 2:
    *(uint16_t*)0x20000000 = 0xa;
    *(uint16_t*)0x20000002 = htobe16(0x4e20);
    *(uint32_t*)0x20000004 = 0;
    *(uint8_t*)0x20000008 = 0;
    *(uint8_t*)0x20000009 = 0;
    *(uint8_t*)0x2000000a = 0;
    *(uint8_t*)0x2000000b = 0;
    *(uint8_t*)0x2000000c = 0;
    *(uint8_t*)0x2000000d = 0;
    *(uint8_t*)0x2000000e = 0;
    *(uint8_t*)0x2000000f = 0;
    *(uint8_t*)0x20000010 = 0;
    *(uint8_t*)0x20000011 = 0;
    *(uint8_t*)0x20000012 = 0;
    *(uint8_t*)0x20000013 = 0;
    *(uint8_t*)0x20000014 = 0;
    *(uint8_t*)0x20000015 = 0;
    *(uint8_t*)0x20000016 = 0;
    *(uint8_t*)0x20000017 = 0;
    *(uint32_t*)0x20000018 = 0;
    syscall(__NR_bind, r[0], 0x20000000, 0x1c);
    break;
  case 3:
    r[1] = syscall(__NR_socket, 2, 6, 0);
    break;
  case 4:
    syscall(__NR_listen, r[0], 6);
    break;
  case 5:
    memcpy((void*)0x201c9fff, "\x03", 1);
    syscall(__NR_setsockopt, r[1], 0x10d, 0xd, 0x201c9fff, 1);
    break;
  case 6:
    *(uint16_t*)0x20e5c000 = 2;
    *(uint16_t*)0x20e5c002 = htobe16(0x4e20);
    *(uint32_t*)0x20e5c004 = htobe32(0x7f000001);
    *(uint8_t*)0x20e5c008 = 0;
    *(uint8_t*)0x20e5c009 = 0;
    *(uint8_t*)0x20e5c00a = 0;
    *(uint8_t*)0x20e5c00b = 0;
    *(uint8_t*)0x20e5c00c = 0;
    *(uint8_t*)0x20e5c00d = 0;
    *(uint8_t*)0x20e5c00e = 0;
    *(uint8_t*)0x20e5c00f = 0;
    syscall(__NR_connect, r[1], 0x20e5c000, 0x10);
    break;
  case 7:
    *(uint32_t*)0x2033f000 = 0xc;
    r[2] = syscall(__NR_accept, r[0], 0x204c9000, 0x2033f000);
    break;
  case 8:
    *(uint64_t*)0x206c8000 = 0;
    *(uint32_t*)0x206c8008 = 0;
    *(uint64_t*)0x206c8010 = 0x206b3ff0;
    *(uint64_t*)0x206c8018 = 1;
    *(uint64_t*)0x206c8020 = 0;
    *(uint64_t*)0x206c8028 = 0;
    *(uint32_t*)0x206c8030 = 0;
    *(uint64_t*)0x206b3ff0 = 0x20167000;
    *(uint64_t*)0x206b3ff8 = 0x68;
    *(uint8_t*)0x20167000 = 2;
    *(uint8_t*)0x20167001 = 0xc;
    *(uint8_t*)0x20167002 = 0x86;
    *(uint8_t*)0x20167003 = 8;
    *(uint16_t*)0x20167004 = 0xd;
    *(uint16_t*)0x20167006 = 0;
    *(uint32_t*)0x20167008 = 0x70bd2b;
    *(uint32_t*)0x2016700c = 0x25dfdbfd;
    *(uint16_t*)0x20167010 = 2;
    *(uint16_t*)0x20167012 = 0x13;
    *(uint8_t*)0x20167014 = 4;
    *(uint8_t*)0x20167015 = 0;
    *(uint16_t*)0x20167016 = 0;
    *(uint32_t*)0x20167018 = 0x70bd27;
    *(uint32_t*)0x2016701c = 0x3500;
    *(uint16_t*)0x20167020 = 1;
    *(uint16_t*)0x20167022 = 0x14;
    *(uint8_t*)0x20167024 = 1;
    *(uint8_t*)0x20167025 = 0;
    *(uint8_t*)0x20167026 = 0;
    *(uint8_t*)0x20167027 = 0;
    *(uint16_t*)0x20167028 = 8;
    *(uint16_t*)0x2016702a = 0x12;
    *(uint16_t*)0x2016702c = 3;
    *(uint8_t*)0x2016702e = 2;
    *(uint8_t*)0x2016702f = 0;
    *(uint32_t*)0x20167030 = 0x6e6bb4;
    *(uint32_t*)0x20167034 = 0x7dd7;
    *(uint16_t*)0x20167038 = 6;
    *(uint16_t*)0x2016703a = 0xff;
    *(uint8_t*)0x2016703c = 0xcc;
    *(uint8_t*)0x2016703d = 2;
    *(uint16_t*)0x2016703e = 0;
    *(uint32_t*)0x20167040 = 0xffff;
    *(uint32_t*)0x20167044 = 0;
    *(uint8_t*)0x20167048 = 0xac;
    *(uint8_t*)0x20167049 = 0x14;
    *(uint8_t*)0x2016704a = 0;
    *(uint8_t*)0x2016704b = 0x16;
    *(uint8_t*)0x20167058 = 0;
    *(uint8_t*)0x20167059 = 0;
    *(uint8_t*)0x2016705a = 0;
    *(uint8_t*)0x2016705b = 0;
    *(uint8_t*)0x2016705c = 0;
    *(uint8_t*)0x2016705d = 0;
    *(uint8_t*)0x2016705e = 0;
    *(uint8_t*)0x2016705f = 0;
    *(uint8_t*)0x20167060 = 0;
    *(uint8_t*)0x20167061 = 0;
    *(uint8_t*)0x20167062 = -1;
    *(uint8_t*)0x20167063 = -1;
    *(uint32_t*)0x20167064 = htobe32(0xe0000001);
    syscall(__NR_sendmsg, r[2], 0x206c8000, 0x80);
    break;
  case 9:
    memcpy((void*)0x206c2f47,
           "\x1a\x36\xe3\xa1\x3a\xb3\xf4\xaf\x2b\xb7\xfb\x0b\x3f\xd2\xad\xe8"
           "\xf0\x54\xb7\xef\xee\x2a\xb3\xa1\x23\xa3\x79\x60\x3e\x0f\xe6\xa1"
           "\x10\xe0\x8e\x6d\xd6\x9a\x75\x5c\x96\x95\x62\x61\x69\xa8\x1f\xe3"
           "\x4f\xd8\x0f\xc9\xc3\x70\x79\xd4\x5e\x7b\xf3\x44\xf1\xb3\xbd\x93"
           "\xb1\xb2\x5b\xe9\xb0\x67\xb1\xf4\xe2\x16\xdf\x61\x49\x44\xa0\xc7"
           "\x01\x0b\x7e\x0a\x60\xac\xb9\xf5\xf2\xe7\xda\x57\xb7\x72\x4b\x5f"
           "\x07\x56\x76\x53\xeb\x52\xcf\xf6\x0d\x05\xd2\x32\x33\xd2\x64\x8f"
           "\xe2\x88\xa4\x3a\xe6\x3b\xf7\x88\x31\x69\x01\x42\x3e\x08\x88\xa3"
           "\x92\xe5\xff\x4b\x46\x37\x43\xc5\x6a\x32\x96\x50\x14\xa4\x76\xcc"
           "\xbe\xbb\x81\xa3\xdd\x34\xa0\x8b\x1d\xf2\x6d\xfb\x00\xe0\xc8\xde"
           "\x4e\xcd\xad\xe1\x1e\x76\x11\xb9\xca\x10\x32\xc6\xe6\xee\x2e\x87"
           "\x19\x47\x7d\xbd\xe8\xe7\x52\x52\x05",
           185);
    *(uint16_t*)0x20c69ff0 = 4;
    *(uint16_t*)0x20c69ff2 = htobe16(1);
    *(uint32_t*)0x20c69ff4 = htobe32(9);
    memcpy((void*)0x20c69ff8, "\x95\xa7\x0b\x27\x86\x63", 6);
    *(uint8_t*)0x20c69ffe = -1;
    *(uint8_t*)0x20c69fff = 0;
    syscall(__NR_sendto, r[2], 0x206c2f47, 0xb9, 0x20000000, 0x20c69ff0, 0x10);
    break;
  case 10:
    *(uint64_t*)0x20ddd000 = 0x204d2000;
    *(uint64_t*)0x20ddd008 = 0;
    *(uint64_t*)0x20ddd010 = 0x20b33fc5;
    *(uint64_t*)0x20ddd018 = 0;
    *(uint64_t*)0x20ddd020 = 0x20b75ff0;
    *(uint64_t*)0x20ddd028 = 0;
    *(uint64_t*)0x20ddd030 = 0x20f99000;
    *(uint64_t*)0x20ddd038 = 0x18;
    memcpy((void*)0x20f99000, "\x29\x8c\xb1\xbe\x4d\xa2\xea\xa6\xdf\xa1\xab\x98"
                              "\x5d\xb8\xc1\x54\x45\x2d\x57\x9e\xe4\xcb\xe4"
                              "\xf8",
           24);
    syscall(__NR_writev, r[1], 0x20ddd000, 4);
    break;
  case 11:
    *(uint8_t*)0x20c12000 = 1;
    *(uint8_t*)0x20c12001 = 0x80;
    *(uint8_t*)0x20c12002 = 0xc2;
    *(uint8_t*)0x20c12003 = 0;
    *(uint8_t*)0x20c12004 = 0;
    *(uint8_t*)0x20c12005 = 0;
    *(uint8_t*)0x20c12006 = 0xaa;
    *(uint8_t*)0x20c12007 = 0xaa;
    *(uint8_t*)0x20c12008 = 0xaa;
    *(uint8_t*)0x20c12009 = 0xaa;
    *(uint8_t*)0x20c1200a = 0;
    *(uint8_t*)0x20c1200b = 0xaa;
    *(uint16_t*)0x20c1200c = htobe16(0x800);
    STORE_BY_BITMASK(uint8_t, 0x20c1200e, 5, 0, 4);
    STORE_BY_BITMASK(uint8_t, 0x20c1200e, 4, 4, 4);
    STORE_BY_BITMASK(uint8_t, 0x20c1200f, 0, 0, 2);
    STORE_BY_BITMASK(uint8_t, 0x20c1200f, 0, 2, 6);
    *(uint16_t*)0x20c12010 = htobe16(0x70);
    *(uint16_t*)0x20c12012 = 0;
    *(uint16_t*)0x20c12014 = htobe16(0);
    *(uint8_t*)0x20c12016 = 0;
    *(uint8_t*)0x20c12017 = 1;
    *(uint16_t*)0x20c12018 = 0;
    *(uint32_t*)0x20c1201a = htobe32(0xfffffe01);
    *(uint8_t*)0x20c1201e = 0xac;
    *(uint8_t*)0x20c1201f = 0x14;
    *(uint8_t*)0x20c12020 = 0;
    *(uint8_t*)0x20c12021 = 0xa;
    *(uint8_t*)0x20c12022 = 0xb;
    *(uint8_t*)0x20c12023 = 0;
    *(uint16_t*)0x20c12024 = 0;
    *(uint8_t*)0x20c12026 = 0;
    *(uint8_t*)0x20c12027 = 0;
    *(uint16_t*)0x20c12028 = 0;
    STORE_BY_BITMASK(uint8_t, 0x20c1202a, 0x15, 0, 4);
    STORE_BY_BITMASK(uint8_t, 0x20c1202a, 4, 4, 4);
    STORE_BY_BITMASK(uint8_t, 0x20c1202b, 0, 0, 2);
    STORE_BY_BITMASK(uint8_t, 0x20c1202b, 0, 2, 6);
    *(uint16_t*)0x20c1202c = htobe16(0);
    *(uint16_t*)0x20c1202e = 0;
    *(uint16_t*)0x20c12030 = htobe16(0);
    *(uint8_t*)0x20c12032 = 0;
    *(uint8_t*)0x20c12033 = 4;
    *(uint16_t*)0x20c12034 = htobe16(0);
    *(uint32_t*)0x20c12036 = htobe32(0x7f000001);
    *(uint32_t*)0x20c1203a = htobe32(0xe0000002);
    *(uint8_t*)0x20c1203e = 0x44;
    *(uint8_t*)0x20c1203f = 0x24;
    *(uint8_t*)0x20c12040 = 0;
    STORE_BY_BITMASK(uint8_t, 0x20c12041, 0, 0, 4);
    STORE_BY_BITMASK(uint8_t, 0x20c12041, 0, 4, 4);
    *(uint32_t*)0x20c12042 = htobe32(0);
    *(uint8_t*)0x20c12046 = 0xac;
    *(uint8_t*)0x20c12047 = 0x14;
    *(uint8_t*)0x20c12048 = 0;
    *(uint8_t*)0x20c12049 = 0;
    *(uint32_t*)0x20c1204a = htobe32(0);
    *(uint32_t*)0x20c1204e = htobe32(0);
    *(uint32_t*)0x20c12052 = htobe32(0);
    *(uint32_t*)0x20c12056 = htobe32(0);
    *(uint32_t*)0x20c1205a = htobe32(0);
    *(uint32_t*)0x20c1205e = htobe32(0);
    *(uint8_t*)0x20c12062 = 0x44;
    *(uint8_t*)0x20c12063 = 0xc;
    *(uint8_t*)0x20c12064 = 0;
    STORE_BY_BITMASK(uint8_t, 0x20c12065, 0, 0, 4);
    STORE_BY_BITMASK(uint8_t, 0x20c12065, 0, 4, 4);
    *(uint32_t*)0x20c12066 = htobe32(0xe0000001);
    *(uint32_t*)0x20c1206a = htobe32(0);
    *(uint8_t*)0x20c1206e = 0x94;
    *(uint8_t*)0x20c1206f = 6;
    *(uint32_t*)0x20c12070 = htobe32(0);
    *(uint8_t*)0x20c12074 = 0x94;
    *(uint8_t*)0x20c12075 = 6;
    *(uint32_t*)0x20c12076 = htobe32(0);
    *(uint8_t*)0x20c1207a = 0;
    *(uint32_t*)0x20ea3000 = 0;
    *(uint32_t*)0x20ea3004 = 0;
    struct csum_inet csum_1;
    csum_inet_init(&csum_1);
    csum_inet_update(&csum_1, (const uint8_t*)0x20c12022, 92);
    *(uint16_t*)0x20c12024 = csum_inet_digest(&csum_1);
    struct csum_inet csum_2;
    csum_inet_init(&csum_2);
    csum_inet_update(&csum_2, (const uint8_t*)0x20c1200e, 20);
    *(uint16_t*)0x20c12018 = csum_inet_digest(&csum_2);
    break;
  }
}

void test()
{
  memset(r, -1, sizeof(r));
  execute(12);
  collide = 1;
  execute(12);
}

int main()
{
  for (;;) {
    loop();
  }
}
