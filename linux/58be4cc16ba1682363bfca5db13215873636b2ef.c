// BUG: bad usercopy in bpf_test_finish
// https://syzkaller.appspot.com/bug?id=58be4cc16ba1682363bfca5db13215873636b2ef
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  install_segv_handler();
  long res = 0;
  NONFAILING(*(uint32_t*)0x20000200 = 0xc);
  NONFAILING(*(uint32_t*)0x20000204 = 0xe);
  NONFAILING(*(uint64_t*)0x20000208 = 0x20000000);
  NONFAILING(memcpy((void*)0x20000000,
                    "\xb7\x02\x00\x00\x1a\x4b\x00\x00\xbf\xa3\x00\x00\x00\x00"
                    "\x00\x00\x07\x03\x00\x00\x00\xfe\xff\xff\x7a\x0a\xf0\xff"
                    "\xf8\xff\xff\xff\x79\xa4\xf0\xff\x00\x00\x00\x00\xb7\x06"
                    "\x00\x00\xff\xff\xff\xff\x2d\x64\x05\x00\x00\x00\x00\x00"
                    "\x65\x04\x04\x00\x01\x00\x00\x00\x04\x04\x00\x00\x01\x03"
                    "\x00\x00\xb7\x03\x00\x00\x00\x00\x00\x00\x6a\x0a\x00\xfe"
                    "\x00\x00\x00\x00\x85\x00\x00\x00\x2b\x00\x00\x00\xb7\x00"
                    "\x00\x00\x00\x00\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00",
                    112));
  NONFAILING(*(uint64_t*)0x20000210 = 0x20000340);
  NONFAILING(memcpy((void*)0x20000340, "syzkaller", 10));
  NONFAILING(*(uint32_t*)0x20000218 = 0);
  NONFAILING(*(uint32_t*)0x2000021c = 0);
  NONFAILING(*(uint64_t*)0x20000220 = 0);
  NONFAILING(*(uint32_t*)0x20000228 = 0);
  NONFAILING(*(uint32_t*)0x2000022c = 0);
  NONFAILING(*(uint8_t*)0x20000230 = 0);
  NONFAILING(*(uint8_t*)0x20000231 = 0);
  NONFAILING(*(uint8_t*)0x20000232 = 0);
  NONFAILING(*(uint8_t*)0x20000233 = 0);
  NONFAILING(*(uint8_t*)0x20000234 = 0);
  NONFAILING(*(uint8_t*)0x20000235 = 0);
  NONFAILING(*(uint8_t*)0x20000236 = 0);
  NONFAILING(*(uint8_t*)0x20000237 = 0);
  NONFAILING(*(uint8_t*)0x20000238 = 0);
  NONFAILING(*(uint8_t*)0x20000239 = 0);
  NONFAILING(*(uint8_t*)0x2000023a = 0);
  NONFAILING(*(uint8_t*)0x2000023b = 0);
  NONFAILING(*(uint8_t*)0x2000023c = 0);
  NONFAILING(*(uint8_t*)0x2000023d = 0);
  NONFAILING(*(uint8_t*)0x2000023e = 0);
  NONFAILING(*(uint8_t*)0x2000023f = 0);
  NONFAILING(*(uint32_t*)0x20000240 = 0);
  NONFAILING(*(uint32_t*)0x20000244 = 0);
  res = syscall(__NR_bpf, 5, 0x20000200, 0x48);
  if (res != -1)
    r[0] = res;
  NONFAILING(*(uint32_t*)0x20000180 = r[0]);
  NONFAILING(*(uint32_t*)0x20000184 = 0);
  NONFAILING(*(uint32_t*)0x20000188 = 0xe);
  NONFAILING(*(uint32_t*)0x2000018c = 0xf8);
  NONFAILING(*(uint64_t*)0x20000190 = 0x20000080);
  NONFAILING(memcpy((void*)0x20000080,
                    "\x7a\x64\xf6\xbf\xff\x9c\x6c\xa2\x8b\xe4\x03\x39\xb5\xff",
                    14));
  NONFAILING(*(uint64_t*)0x20000198 = 0x20000500);
  NONFAILING(*(uint32_t*)0x200001a0 = 0x101);
  NONFAILING(*(uint32_t*)0x200001a4 = 0);
  syscall(__NR_bpf, 0xa, 0x20000180, 0x28);
  return 0;
}
