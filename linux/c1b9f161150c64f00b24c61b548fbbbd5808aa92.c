// WARNING in tracepoint_probe_register_prio (2)
// https://syzkaller.appspot.com/bug?id=c1b9f161150c64f00b24c61b548fbbbd5808aa92
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
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

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* uctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  doexit(sig);
}

static void install_segv_handler()
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
    close(fd);
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

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res;
  NONFAILING(*(uint32_t*)0x2025c000 = 2);
  NONFAILING(*(uint32_t*)0x2025c004 = 0x78);
  NONFAILING(*(uint8_t*)0x2025c008 = 0xe3);
  NONFAILING(*(uint8_t*)0x2025c009 = 0);
  NONFAILING(*(uint8_t*)0x2025c00a = 0);
  NONFAILING(*(uint8_t*)0x2025c00b = 0);
  NONFAILING(*(uint32_t*)0x2025c00c = 0);
  NONFAILING(*(uint64_t*)0x2025c010 = 0);
  NONFAILING(*(uint64_t*)0x2025c018 = 0);
  NONFAILING(*(uint64_t*)0x2025c020 = 0);
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 0, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 1, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 2, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 3, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 4, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 5, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 6, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 7, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 8, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 9, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 10, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 11, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 12, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 13, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 14, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 15, 2));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 17, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 18, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 19, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 20, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 21, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 22, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 23, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 24, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 25, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 26, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 27, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 28, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x2025c028, 0, 29, 35));
  NONFAILING(*(uint32_t*)0x2025c030 = 0);
  NONFAILING(*(uint32_t*)0x2025c034 = 0);
  NONFAILING(*(uint64_t*)0x2025c038 = 0x20389000);
  NONFAILING(*(uint64_t*)0x2025c040 = 0);
  NONFAILING(*(uint64_t*)0x2025c048 = 0);
  NONFAILING(*(uint64_t*)0x2025c050 = 0);
  NONFAILING(*(uint32_t*)0x2025c058 = 0);
  NONFAILING(*(uint32_t*)0x2025c05c = 0);
  NONFAILING(*(uint64_t*)0x2025c060 = 0);
  NONFAILING(*(uint32_t*)0x2025c068 = 0);
  NONFAILING(*(uint16_t*)0x2025c06c = 0);
  NONFAILING(*(uint16_t*)0x2025c06e = 0);
  res = syscall(__NR_perf_event_open, 0x2025c000, 0, -1, -1, 0);
  if (res != -1)
    r[0] = res;
  NONFAILING(*(uint32_t*)0x20348f88 = 2);
  NONFAILING(*(uint32_t*)0x20348f8c = 0x78);
  NONFAILING(*(uint8_t*)0x20348f90 = 0xe2);
  NONFAILING(*(uint8_t*)0x20348f91 = 0);
  NONFAILING(*(uint8_t*)0x20348f92 = 0);
  NONFAILING(*(uint8_t*)0x20348f93 = 0);
  NONFAILING(*(uint32_t*)0x20348f94 = 0);
  NONFAILING(*(uint64_t*)0x20348f98 = 0);
  NONFAILING(*(uint64_t*)0x20348fa0 = 0);
  NONFAILING(*(uint64_t*)0x20348fa8 = 0);
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 0, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 1, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 2, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 3, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 4, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 5, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 6, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 7, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 8, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 9, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 10, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 11, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 12, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 13, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 14, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 15, 2));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 17, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 18, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 19, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 20, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 21, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 22, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 23, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 24, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 25, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 26, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 27, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 28, 1));
  NONFAILING(STORE_BY_BITMASK(uint64_t, 0x20348fb0, 0, 29, 35));
  NONFAILING(*(uint32_t*)0x20348fb8 = 0);
  NONFAILING(*(uint32_t*)0x20348fbc = 0);
  NONFAILING(*(uint64_t*)0x20348fc0 = 0x20000000);
  NONFAILING(*(uint64_t*)0x20348fc8 = 0);
  NONFAILING(*(uint64_t*)0x20348fd0 = 0);
  NONFAILING(*(uint64_t*)0x20348fd8 = 0);
  NONFAILING(*(uint32_t*)0x20348fe0 = 0);
  NONFAILING(*(uint32_t*)0x20348fe4 = 0);
  NONFAILING(*(uint64_t*)0x20348fe8 = 0);
  NONFAILING(*(uint32_t*)0x20348ff0 = 0);
  NONFAILING(*(uint16_t*)0x20348ff4 = 0);
  NONFAILING(*(uint16_t*)0x20348ff6 = 0);
  write_file("/sys/kernel/debug/failslab/ignore-gfp-wait", "N");
  write_file("/sys/kernel/debug/fail_futex/ignore-private", "N");
  inject_fault(1);
  syscall(__NR_perf_event_open, 0x20348f88, 0, -1, r[0], 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  install_segv_handler();
  loop();
  return 0;
}
