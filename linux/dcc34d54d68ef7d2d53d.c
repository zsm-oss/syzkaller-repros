// KASAN: invalid-free in tcf_exts_destroy
// https://syzkaller.appspot.com/bug?id=dcc34d54d68ef7d2d53d
// status:6 arch:386
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
  if (unshare(CLONE_NEWPID)) {
  }
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

#ifndef __NR_getsockname
#define __NR_getsockname 367
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[5] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff, 0x0};

void loop(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[2] = res;
  res = syscall(__NR_socket, 0x10, 0x803, 0);
  if (res != -1)
    r[3] = res;
  *(uint32_t*)0x20000280 = 0;
  *(uint32_t*)0x20000284 = 0;
  *(uint32_t*)0x20000288 = 0x20000180;
  *(uint32_t*)0x20000180 = 0;
  *(uint32_t*)0x20000184 = 0;
  *(uint32_t*)0x2000028c = 1;
  *(uint32_t*)0x20000290 = 0;
  *(uint32_t*)0x20000294 = 0;
  *(uint32_t*)0x20000298 = 0;
  syscall(__NR_sendmsg, (intptr_t)r[3], 0x20000280, 0);
  *(uint32_t*)0x200002c0 = 0x14;
  res = syscall(__NR_getsockname, (intptr_t)r[3], 0x20000100, 0x200002c0);
  if (res != -1)
    r[4] = *(uint32_t*)0x20000104;
  *(uint32_t*)0x20000240 = 0;
  *(uint32_t*)0x20000244 = 0;
  *(uint32_t*)0x20000248 = 0x20000140;
  *(uint32_t*)0x20000140 = 0x200003c0;
  memcpy((void*)0x200003c0, "\x38\x00\x00\x00\x24\x00\x07\x05\x00\x00\x00\x40"
                            "\x07\xa2\xa3\x00\x05\x00\x00\x00",
         20);
  *(uint32_t*)0x200003d4 = r[4];
  memcpy((void*)0x200003d8, "\x00\x00\x00\x00\xff\xff\xff\xff\x00\x00\x00\x00"
                            "\x09\x00\x01\x00\x68\x66\x73\x63\x00\x00\x00\x00"
                            "\x08\x00\x02\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20000144 = 0x38;
  *(uint32_t*)0x2000024c = 1;
  *(uint32_t*)0x20000250 = 0;
  *(uint32_t*)0x20000254 = 0;
  *(uint32_t*)0x20000258 = 0;
  syscall(__NR_sendmsg, (intptr_t)r[2], 0x20000240, 0);
  *(uint32_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c4 = 0;
  *(uint32_t*)0x200001c8 = 0x20000180;
  *(uint32_t*)0x20000180 = 0x20000580;
  *(uint32_t*)0x20000580 = 0x3c;
  *(uint16_t*)0x20000584 = 0x2c;
  *(uint16_t*)0x20000586 = 0xd27;
  *(uint32_t*)0x20000588 = 0;
  *(uint32_t*)0x2000058c = 0;
  *(uint8_t*)0x20000590 = 0;
  *(uint8_t*)0x20000591 = 0;
  *(uint16_t*)0x20000592 = 0;
  *(uint32_t*)0x20000594 = r[4];
  *(uint16_t*)0x20000598 = 0;
  *(uint16_t*)0x2000059a = 0;
  *(uint16_t*)0x2000059c = 0;
  *(uint16_t*)0x2000059e = 0;
  *(uint16_t*)0x200005a0 = 4;
  *(uint16_t*)0x200005a2 = 0xfff1;
  *(uint16_t*)0x200005a4 = 0xc;
  *(uint16_t*)0x200005a6 = 1;
  memcpy((void*)0x200005a8, "tcindex\000", 8);
  *(uint16_t*)0x200005b0 = 0xc;
  *(uint16_t*)0x200005b2 = 2;
  *(uint16_t*)0x200005b4 = 6;
  *(uint16_t*)0x200005b6 = 2;
  *(uint16_t*)0x200005b8 = 0;
  *(uint32_t*)0x20000184 = 0x3c;
  *(uint32_t*)0x200001cc = 1;
  *(uint32_t*)0x200001d0 = 0;
  *(uint32_t*)0x200001d4 = 0;
  *(uint32_t*)0x200001d8 = 0;
  syscall(__NR_sendmsg, (intptr_t)r[1], 0x200001c0, 0);
  *(uint32_t*)0x200001c0 = 0;
  *(uint32_t*)0x200001c4 = 0;
  *(uint32_t*)0x200001c8 = 0x20000180;
  *(uint32_t*)0x20000180 = 0x20000400;
  *(uint32_t*)0x20000400 = 0x3c;
  *(uint16_t*)0x20000404 = 0x2c;
  *(uint16_t*)0x20000406 = 0xd27;
  *(uint32_t*)0x20000408 = 0;
  *(uint32_t*)0x2000040c = 0;
  *(uint8_t*)0x20000410 = 0;
  *(uint8_t*)0x20000411 = 0;
  *(uint16_t*)0x20000412 = 0xf0;
  *(uint32_t*)0x20000414 = r[4];
  *(uint16_t*)0x20000418 = 3;
  *(uint16_t*)0x2000041a = 0;
  *(uint16_t*)0x2000041c = 0;
  *(uint16_t*)0x2000041e = 0;
  *(uint16_t*)0x20000420 = 0;
  *(uint16_t*)0x20000422 = 0xfff1;
  *(uint16_t*)0x20000424 = 0xc;
  *(uint16_t*)0x20000426 = 1;
  memcpy((void*)0x20000428, "tcindex\000", 8);
  *(uint16_t*)0x20000430 = 0xc;
  *(uint16_t*)0x20000432 = 2;
  *(uint16_t*)0x20000434 = 8;
  *(uint16_t*)0x20000436 = 1;
  *(uint32_t*)0x20000438 = 0;
  *(uint32_t*)0x20000184 = 0x3c;
  *(uint32_t*)0x200001cc = 1;
  *(uint32_t*)0x200001d0 = 0;
  *(uint32_t*)0x200001d4 = 0;
  *(uint32_t*)0x200001d8 = 0;
  syscall(__NR_sendmsg, (intptr_t)r[0], 0x200001c0, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  do_sandbox_none();
  return 0;
}
