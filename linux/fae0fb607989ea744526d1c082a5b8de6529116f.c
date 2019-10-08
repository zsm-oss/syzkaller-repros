// WARNING in kernfs_add_one
// https://syzkaller.appspot.com/bug?id=fae0fb607989ea744526d1c082a5b8de6529116f
// status:fixed
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

uint64_t r[1] = {0xffffffffffffffff};
void execute_one()
{
  long res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0x10);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20b3dfc8 = 0x20000180;
  *(uint16_t*)0x20000180 = 0x10;
  *(uint16_t*)0x20000182 = 0x6c;
  *(uint32_t*)0x20000184 = 0;
  *(uint32_t*)0x20000188 = 0;
  *(uint32_t*)0x20b3dfd0 = 0xc;
  *(uint64_t*)0x20b3dfd8 = 0x200015c0;
  *(uint64_t*)0x200015c0 = 0x200001c0;
  *(uint32_t*)0x200001c0 = 0x18;
  *(uint16_t*)0x200001c4 = 0x22;
  *(uint16_t*)0x200001c6 = 0x109;
  *(uint32_t*)0x200001c8 = 0;
  *(uint32_t*)0x200001cc = 0;
  *(uint8_t*)0x200001d0 = 4;
  *(uint8_t*)0x200001d1 = 0;
  *(uint16_t*)0x200001d2 = 0;
  *(uint16_t*)0x200001d4 = 4;
  *(uint16_t*)0x200001d6 = 0x12;
  *(uint64_t*)0x200015c8 = 0x18;
  *(uint64_t*)0x20b3dfe0 = 1;
  *(uint64_t*)0x20b3dfe8 = 0;
  *(uint64_t*)0x20b3dff0 = 0;
  *(uint32_t*)0x20b3dff8 = 0;
  write_file("/sys/kernel/debug/failslab/ignore-gfp-wait", "N");
  write_file("/sys/kernel/debug/fail_futex/ignore-private", "N");
  inject_fault(44);
  syscall(__NR_sendmsg, r[0], 0x20b3dfc8, 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
