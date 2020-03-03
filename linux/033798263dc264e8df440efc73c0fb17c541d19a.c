// possible deadlock in bpf_lru_push_free
// https://syzkaller.appspot.com/bug?id=033798263dc264e8df440efc73c0fb17c541d19a
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <linux/capability.h>

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
  rlim.rlim_cur = rlim.rlim_max = (200 << 20);
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 32 << 20;
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
  typedef struct {
    const char* name;
    const char* value;
  } sysctl_t;
  static const sysctl_t sysctls[] = {
      {"/proc/sys/kernel/shmmax", "16777216"},
      {"/proc/sys/kernel/shmall", "536870912"},
      {"/proc/sys/kernel/shmmni", "1024"},
      {"/proc/sys/kernel/msgmax", "8192"},
      {"/proc/sys/kernel/msgmni", "1024"},
      {"/proc/sys/kernel/msgmnb", "1024"},
      {"/proc/sys/kernel/sem", "1024 1048576 500 1024"},
  };
  unsigned i;
  for (i = 0; i < sizeof(sysctls) / sizeof(sysctls[0]); i++)
    write_file(sysctls[i].name, sysctls[i].value);
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

static void drop_caps(void)
{
  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    exit(1);
  const int drop = (1 << CAP_SYS_PTRACE) | (1 << CAP_SYS_NICE);
  cap_data[0].effective &= ~drop;
  cap_data[0].permitted &= ~drop;
  cap_data[0].inheritable &= ~drop;
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    exit(1);
}

static int do_sandbox_none(void)
{
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);
  setup_common();
  sandbox_common();
  drop_caps();
  if (unshare(CLONE_NEWNET)) {
  }
  loop();
  exit(1);
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[1] = {0xffffffffffffffff};

void loop(void)
{
  intptr_t res = 0;
  *(uint32_t*)0x20000140 = 0xa;
  *(uint32_t*)0x20000144 = 3;
  *(uint32_t*)0x20000148 = 0x6c0d;
  *(uint32_t*)0x2000014c = 1;
  *(uint32_t*)0x20000150 = 2;
  *(uint32_t*)0x20000154 = -1;
  *(uint32_t*)0x20000158 = 0;
  *(uint8_t*)0x2000015c = 0;
  *(uint8_t*)0x2000015d = 0;
  *(uint8_t*)0x2000015e = 0;
  *(uint8_t*)0x2000015f = 0;
  *(uint8_t*)0x20000160 = 0;
  *(uint8_t*)0x20000161 = 0;
  *(uint8_t*)0x20000162 = 0;
  *(uint8_t*)0x20000163 = 0;
  *(uint8_t*)0x20000164 = 0;
  *(uint8_t*)0x20000165 = 0;
  *(uint8_t*)0x20000166 = 0;
  *(uint8_t*)0x20000167 = 0;
  *(uint8_t*)0x20000168 = 0;
  *(uint8_t*)0x20000169 = 0;
  *(uint8_t*)0x2000016a = 0;
  *(uint8_t*)0x2000016b = 0;
  *(uint32_t*)0x2000016c = 0;
  *(uint32_t*)0x20000170 = -1;
  *(uint32_t*)0x20000174 = 0;
  *(uint32_t*)0x20000178 = 0;
  *(uint32_t*)0x2000017c = 0;
  res = syscall(__NR_bpf, 0ul, 0x20000140ul, 0x3cul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000180 = r[0];
  *(uint64_t*)0x20000188 = 0x20000000;
  *(uint64_t*)0x20000190 = 0x20000080;
  *(uint64_t*)0x20000198 = 0;
  syscall(__NR_bpf, 2ul, 0x20000180ul, 0x20ul);
  *(uint32_t*)0x20000340 = r[0];
  *(uint64_t*)0x20000348 = 0x20000300;
  *(uint64_t*)0x20000350 = 0x20000340;
  *(uint64_t*)0x20000358 = 0;
  syscall(__NR_bpf, 2ul, 0x20000340ul, 0x20ul);
  *(uint64_t*)0x20000100 = 0;
  *(uint64_t*)0x20000108 = 0;
  *(uint64_t*)0x20000110 = 0;
  *(uint64_t*)0x20000118 = 0;
  *(uint32_t*)0x20000120 = 0x101;
  *(uint32_t*)0x20000124 = r[0];
  *(uint64_t*)0x20000128 = 0;
  *(uint64_t*)0x20000130 = 0;
  syscall(__NR_bpf, 0x19ul, 0x20000100ul, 0x38ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  do_sandbox_none();
  return 0;
}
