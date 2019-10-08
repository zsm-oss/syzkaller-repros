// kernel BUG at fs/ext4/extents.c:LINE!
// https://syzkaller.appspot.com/bug?id=7e7470f0058e0950706a4fd684c2d86c7b43df31
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
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
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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

static void use_temporary_dir()
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    fail("failed to mkdtemp");
  if (chmod(tmpdir, 0777))
    fail("failed to chmod");
  if (chdir(tmpdir))
    fail("failed to chdir");
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
  memcpy((void*)0x20002000, "./bus", 6);
  res = syscall(__NR_open, 0x20002000, 0x141042, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000280, "./bus", 6);
  res = syscall(__NR_open, 0x20000280, 0x141042, 0x88);
  if (res != -1)
    r[1] = res;
  syscall(__NR_fallocate, r[1], 0, 0, 4);
  syscall(__NR_fallocate, r[1], 1, 0x200002, 0x10000101);
  syscall(__NR_fallocate, r[0], 0x20, 0, 0xfffffeff000);
  *(uint64_t*)0x20000000 = 0;
  syscall(__NR_sendfile, r[0], r[1], 0x20000000, 0x100000001);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  char* cwd = get_current_dir_name();
  for (;;) {
    if (chdir(cwd))
      fail("failed to chdir");
    use_temporary_dir();
    loop();
  }
}
