// WARNING in pcpu_alloc
// https://syzkaller.appspot.com/bug?id=1237884c1c84e9684d7711733f733db26ae5700c
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 160 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 256;
  setrlimit(RLIMIT_NOFILE, &rlim);
  if (unshare(CLONE_NEWNS)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(0x02000000)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
}

int wait_for_loop(int pid)
{
  if (pid < 0)
    exit(1);
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);
  setup_common();
  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }
  loop();
  exit(1);
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

void loop(void)
{
  *(uint32_t*)0x20000000 = 1;
  *(uint32_t*)0x20000004 = 8;
  *(uint32_t*)0x20000008 = 0x7fff;
  *(uint32_t*)0x2000000c = 1;
  *(uint32_t*)0x20000010 = 0;
  *(uint32_t*)0x20000014 = 0;
  *(uint32_t*)0x20000018 = 0;
  *(uint8_t*)0x2000001c = 0;
  *(uint8_t*)0x2000001d = 0;
  *(uint8_t*)0x2000001e = 0;
  *(uint8_t*)0x2000001f = 0;
  *(uint8_t*)0x20000020 = 0;
  *(uint8_t*)0x20000021 = 0;
  *(uint8_t*)0x20000022 = 0;
  *(uint8_t*)0x20000023 = 0;
  *(uint8_t*)0x20000024 = 0;
  *(uint8_t*)0x20000025 = 0;
  *(uint8_t*)0x20000026 = 0;
  *(uint8_t*)0x20000027 = 0;
  *(uint8_t*)0x20000028 = 0;
  *(uint8_t*)0x20000029 = 0;
  *(uint8_t*)0x2000002a = 0;
  *(uint8_t*)0x2000002b = 0;
  syscall(__NR_bpf, 0, 0x20000000, 0x2c);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  do_sandbox_none();
  return 0;
}
