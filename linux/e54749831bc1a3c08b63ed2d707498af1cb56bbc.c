// kernel BUG at drivers/android/binder_alloc.c:LINE!
// https://syzkaller.appspot.com/bug?id=e54749831bc1a3c08b63ed2d707498af1cb56bbc
// status:fixed
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  exit(sig);
}

static void install_segv_handler(void)
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

static long syz_open_dev(volatile long a0, volatile long a1, volatile long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    NONFAILING(strncpy(buf, (char*)a0, sizeof(buf) - 1));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  install_segv_handler();
  long res = 0;
  NONFAILING(memcpy((void*)0x20000400, "/dev/binder#\000", 13));
  res = syz_open_dev(0x20000400, 0, 0);
  if (res != -1)
    r[0] = res;
  res = syz_open_dev(0, 0, 0x800);
  if (res != -1)
    r[1] = res;
  syscall(__NR_mmap, 0x20001000, 0x3000, 0, 0x20011, r[1], 0);
  syscall(__NR_ioctl, r[1], 0x40046207, 0);
  NONFAILING(*(uint64_t*)0x20000440 = 0x44);
  NONFAILING(*(uint64_t*)0x20000448 = 0);
  NONFAILING(*(uint64_t*)0x20000450 = 0x20000300);
  NONFAILING(memcpy((void*)0x20000300,
                    "\000c@@"
                    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
                    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
                    "\000\000\000\000\000\000\000\000\000\000\000\000\b\000\000"
                    "\000\000\000\000\000\000\000\000\000\000\000\000\000",
                    60));
  NONFAILING(*(uint64_t*)0x2000033c = 0x20000000);
  NONFAILING(memcpy((void*)0x20000000,
                    "\x00\x00\x00\x00\x02\x00\x00\x00\xcd\x24\x6e\x47\x20\xbf"
                    "\xd7\xaa\xc8\x09\x90\xb0\x12\xbd\xbf\x70\xc4\xb8\x67\x30"
                    "\xd2\xc4\x31\x1f\xb2\x45\xb0\x0d\x8b\xd4\xd3\x4d\xa8\x67"
                    "\xc6\x09\xe4\x80\x33\xc9\x84\xb9\x0a\xac\x75\x77\x63\x1f"
                    "\xd5\xf8\x30\x06\x43\x8f\xb4\x83\x24\xe0\x6c\xb4",
                    68));
  NONFAILING(*(uint64_t*)0x20000458 = 0);
  NONFAILING(*(uint64_t*)0x20000460 = 0);
  NONFAILING(*(uint64_t*)0x20000468 = 0);
  syscall(__NR_ioctl, r[0], 0xc0306201, 0x20000440);
  return 0;
}
