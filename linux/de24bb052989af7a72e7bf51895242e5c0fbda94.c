// WARNING in __device_add_disk
// https://syzkaller.appspot.com/bug?id=de24bb052989af7a72e7bf51895242e5c0fbda94
// status:open
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <fcntl.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/sysmacros.h>
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

static uintptr_t syz_fuseblk_mount(uintptr_t a0, uintptr_t a1, uintptr_t a2,
                                   uintptr_t a3, uintptr_t a4, uintptr_t a5,
                                   uintptr_t a6, uintptr_t a7)
{
  uint64_t target = a0;
  uint64_t blkdev = a1;
  uint64_t mode = a2;
  uint64_t uid = a3;
  uint64_t gid = a4;
  uint64_t maxread = a5;
  uint64_t blksize = a6;
  uint64_t flags = a7;

  int fd = open("/dev/fuse", O_RDWR);
  if (fd == -1)
    return fd;
  if (syscall(SYS_mknodat, AT_FDCWD, blkdev, S_IFBLK, makedev(7, 199)))
    return fd;
  char buf[256];
  sprintf(buf, "fd=%d,user_id=%ld,group_id=%ld,rootmode=0%o", fd, (long)uid,
          (long)gid, (unsigned)mode & ~3u);
  if (maxread != 0)
    sprintf(buf + strlen(buf), ",max_read=%ld", (long)maxread);
  if (blksize != 0)
    sprintf(buf + strlen(buf), ",blksize=%ld", (long)blksize);
  if (mode & 1)
    strcat(buf, ",default_permissions");
  if (mode & 2)
    strcat(buf, ",allow_other");
  syscall(SYS_mount, blkdev, target, "fuseblk", flags, buf);
  return fd;
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

void execute_one()
{
  memcpy((void*)0x20000080, "./file0", 8);
  memcpy((void*)0x200000c0, "./file0", 8);
  write_file("/sys/kernel/debug/failslab/ignore-gfp-wait", "N");
  write_file("/sys/kernel/debug/fail_futex/ignore-private", "N");
  inject_fault(83);
  syz_fuseblk_mount(0x20000080, 0x200000c0, 0xa000, 0, 0, 0, 0, 0);
}

int main()
{
  write_file("/sys/kernel/debug/failslab/ignore-gfp-wait", "N");
  write_file("/sys/kernel/debug/fail_futex/ignore-private", "N");
  inject_fault(83);
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
