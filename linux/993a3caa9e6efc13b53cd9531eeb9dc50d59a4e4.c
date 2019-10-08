// general protection fault in p9_client_prepare_req
// https://syzkaller.appspot.com/bug?id=993a3caa9e6efc13b53cd9531eeb9dc50d59a4e4
// status:open
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
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

static void exitf(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit(kRetryStatus);
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

static int inject_fault(int nth)
{
  int fd;
  char buf[16];

  fd = open("/proc/thread-self/fail-nth", O_RDWR);
  if (fd == -1)
    exitf("failed to open /proc/thread-self/fail-nth");
  sprintf(buf, "%d", nth + 1);
  if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
    exitf("failed to write /proc/thread-self/fail-nth");
  return fd;
}

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void execute_one()
{
  long res = 0;
  *(uint32_t*)0x20000200 = 2;
  *(uint32_t*)0x20000204 = 0x70;
  *(uint8_t*)0x20000208 = 0xe6;
  *(uint8_t*)0x20000209 = 0;
  *(uint8_t*)0x2000020a = 0;
  *(uint8_t*)0x2000020b = 0;
  *(uint32_t*)0x2000020c = 0;
  *(uint64_t*)0x20000210 = 0;
  *(uint64_t*)0x20000218 = 0;
  *(uint64_t*)0x20000220 = 0;
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 5, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, 0x20000228, 0, 29, 35);
  *(uint32_t*)0x20000230 = 0;
  *(uint32_t*)0x20000234 = 0;
  *(uint64_t*)0x20000238 = 0;
  *(uint64_t*)0x20000240 = 0;
  *(uint64_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0;
  *(uint32_t*)0x20000258 = 0;
  *(uint32_t*)0x2000025c = 0;
  *(uint64_t*)0x20000260 = 0;
  *(uint32_t*)0x20000268 = 0;
  *(uint16_t*)0x2000026c = 0;
  *(uint16_t*)0x2000026e = 0;
  syscall(__NR_perf_event_open, 0x20000200, 0, 0, -1, 0);
  res = syscall(__NR_pipe2, 0x20000100, 0);
  if (res != -1) {
    r[0] = *(uint32_t*)0x20000100;
    r[1] = *(uint32_t*)0x20000104;
  }
  memcpy((void*)0x20000300, "./file0", 8);
  syscall(__NR_mkdir, 0x20000300, 0);
  memcpy((void*)0x200000c0, "./file0", 8);
  memcpy((void*)0x20000340, "9p", 3);
  memcpy((void*)0x200001c0, "trans=fd,", 9);
  memcpy((void*)0x200001c9, "rfdno", 5);
  *(uint8_t*)0x200001ce = 0x3d;
  sprintf((char*)0x200001cf, "0x%016llx", (long long)-1);
  *(uint8_t*)0x200001e1 = 0x2c;
  memcpy((void*)0x200001e2, "wfdno", 5);
  *(uint8_t*)0x200001e7 = 0x3d;
  sprintf((char*)0x200001e8, "0x%016llx", (long long)-1);
  *(uint8_t*)0x200001fa = 0x2c;
  *(uint8_t*)0x200001fb = 0;
  syscall(__NR_mount, 0, 0x200000c0, 0x20000340, 0, 0x200001c0);
  memcpy((void*)0x20000000, "./file0", 8);
  memcpy((void*)0x20000900, "9p", 3);
  memcpy((void*)0x20000840, "trans=fd,", 9);
  memcpy((void*)0x20000849, "rfdno", 5);
  *(uint8_t*)0x2000084e = 0x3d;
  sprintf((char*)0x2000084f, "0x%016llx", (long long)r[0]);
  *(uint8_t*)0x20000861 = 0x2c;
  memcpy((void*)0x20000862, "wfdno", 5);
  *(uint8_t*)0x20000867 = 0x3d;
  sprintf((char*)0x20000868, "0x%016llx", (long long)r[1]);
  *(uint8_t*)0x2000087a = 0x2c;
  memcpy((void*)0x2000087b, "access", 6);
  *(uint8_t*)0x20000881 = 0x3d;
  sprintf((char*)0x20000882, "%020llu", (long long)0);
  *(uint8_t*)0x20000896 = 0x2c;
  *(uint8_t*)0x20000897 = 0;
  write_file("/sys/kernel/debug/failslab/ignore-gfp-wait", "N");
  write_file("/sys/kernel/debug/fail_futex/ignore-private", "N");
  inject_fault(17);
  syscall(__NR_mount, 0, 0x20000000, 0x20000900, 0, 0x20000840);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
