// BUG: unable to handle kernel paging request in xfrm_hash_rebuild
// https://syzkaller.appspot.com/bug?id=8c349ca624bc0e1ef053268fbc8e1c9e48f1d6d8
// status:open
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
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

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();

  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 128 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);

#define CLONE_NEWCGROUP 0x02000000

  if (unshare(CLONE_NEWNS)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(CLONE_NEWCGROUP)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid < 0)
    fail("sandbox fork failed");
  if (pid)
    return pid;

  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }

  loop();
  doexit(1);
}

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res;
  res = syscall(__NR_socket, 0x10, 3, 6);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000340 = 0x20000000;
  *(uint16_t*)0x20000000 = 0x10;
  *(uint16_t*)0x20000002 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0x40000;
  *(uint32_t*)0x20000348 = 0xc;
  *(uint64_t*)0x20000350 = 0x20000300;
  *(uint64_t*)0x20000300 = 0x20000200;
  *(uint32_t*)0x20000200 = 0xcc;
  *(uint16_t*)0x20000204 = 0x19;
  *(uint16_t*)0x20000206 = 0x101;
  *(uint32_t*)0x20000208 = 0x70bd2a;
  *(uint32_t*)0x2000020c = 0x25dfdbfc;
  *(uint8_t*)0x20000210 = 0;
  *(uint8_t*)0x20000211 = 0;
  *(uint8_t*)0x20000212 = 0;
  *(uint8_t*)0x20000213 = 0;
  *(uint8_t*)0x20000214 = 0;
  *(uint8_t*)0x20000215 = 0;
  *(uint8_t*)0x20000216 = 0;
  *(uint8_t*)0x20000217 = 0;
  *(uint8_t*)0x20000218 = 0;
  *(uint8_t*)0x20000219 = 0;
  *(uint8_t*)0x2000021a = 0;
  *(uint8_t*)0x2000021b = 0;
  *(uint8_t*)0x2000021c = 0;
  *(uint8_t*)0x2000021d = 0;
  *(uint8_t*)0x2000021e = 0;
  *(uint8_t*)0x2000021f = 0;
  *(uint32_t*)0x20000220 = htobe32(0xe0000001);
  *(uint16_t*)0x20000230 = htobe16(0x4e24);
  *(uint16_t*)0x20000232 = htobe16(1);
  *(uint16_t*)0x20000234 = htobe16(0x4e22);
  *(uint16_t*)0x20000236 = htobe16(0);
  *(uint16_t*)0x20000238 = 2;
  *(uint8_t*)0x2000023a = 0x20;
  *(uint8_t*)0x2000023b = 0xa0;
  *(uint8_t*)0x2000023c = 0x7f;
  *(uint32_t*)0x20000240 = 0;
  *(uint32_t*)0x20000244 = 0;
  *(uint64_t*)0x20000248 = 2;
  *(uint64_t*)0x20000250 = 5;
  *(uint64_t*)0x20000258 = 5;
  *(uint64_t*)0x20000260 = 0x1f;
  *(uint64_t*)0x20000268 = 2;
  *(uint64_t*)0x20000270 = 0x2715;
  *(uint64_t*)0x20000278 = 5;
  *(uint64_t*)0x20000280 = 0x81;
  *(uint64_t*)0x20000288 = 8;
  *(uint64_t*)0x20000290 = 8;
  *(uint64_t*)0x20000298 = 7;
  *(uint64_t*)0x200002a0 = 0x59;
  *(uint32_t*)0x200002a8 = 0x87;
  *(uint32_t*)0x200002ac = 0x6e6bbd;
  *(uint8_t*)0x200002b0 = 1;
  *(uint8_t*)0x200002b1 = 1;
  *(uint8_t*)0x200002b2 = 2;
  *(uint8_t*)0x200002b3 = 3;
  *(uint16_t*)0x200002b8 = 0x14;
  *(uint16_t*)0x200002ba = 0xe;
  *(uint8_t*)0x200002bc = 0xfe;
  *(uint8_t*)0x200002bd = 0x80;
  *(uint8_t*)0x200002be = 0;
  *(uint8_t*)0x200002bf = 0;
  *(uint8_t*)0x200002c0 = 0;
  *(uint8_t*)0x200002c1 = 0;
  *(uint8_t*)0x200002c2 = 0;
  *(uint8_t*)0x200002c3 = 0;
  *(uint8_t*)0x200002c4 = 0;
  *(uint8_t*)0x200002c5 = 0;
  *(uint8_t*)0x200002c6 = 0;
  *(uint8_t*)0x200002c7 = 0;
  *(uint8_t*)0x200002c8 = 0;
  *(uint8_t*)0x200002c9 = 0;
  *(uint8_t*)0x200002ca = 0;
  *(uint8_t*)0x200002cb = 0xa;
  *(uint64_t*)0x20000308 = 0xcc;
  *(uint64_t*)0x20000358 = 1;
  *(uint64_t*)0x20000360 = 0;
  *(uint64_t*)0x20000368 = 0;
  *(uint32_t*)0x20000370 = 1;
  syscall(__NR_sendmsg, r[0], 0x20000340, 0x8000);
  *(uint64_t*)0x2016afc8 = 0x20ac0000;
  *(uint16_t*)0x20ac0000 = 0x10;
  *(uint16_t*)0x20ac0002 = 0;
  *(uint32_t*)0x20ac0004 = 0;
  *(uint32_t*)0x20ac0008 = 0;
  *(uint32_t*)0x2016afd0 = 0xc;
  *(uint64_t*)0x2016afd8 = 0x206fb000;
  *(uint64_t*)0x206fb000 = 0x202ebe88;
  *(uint32_t*)0x202ebe88 = 0x1c;
  *(uint16_t*)0x202ebe8c = 0x24;
  *(uint16_t*)0x202ebe8e = 1;
  *(uint32_t*)0x202ebe90 = 0;
  *(uint32_t*)0x202ebe94 = 0;
  *(uint32_t*)0x202ebe98 = 0;
  *(uint16_t*)0x202ebe9c = 8;
  *(uint16_t*)0x202ebe9e = 3;
  *(uint8_t*)0x202ebea0 = 0;
  *(uint8_t*)0x202ebea1 = 0;
  *(uint64_t*)0x206fb008 = 0x1c;
  *(uint64_t*)0x2016afe0 = 1;
  *(uint64_t*)0x2016afe8 = 0;
  *(uint64_t*)0x2016aff0 = 0;
  *(uint32_t*)0x2016aff8 = 0;
  syscall(__NR_sendmsg, r[0], 0x2016afc8, 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  int pid = do_sandbox_none();
  int status = 0;
  while (waitpid(pid, &status, __WALL) != pid) {
  }
  return 0;
}
