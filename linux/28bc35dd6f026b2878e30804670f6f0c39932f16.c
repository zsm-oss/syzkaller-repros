// no output from test machine
// https://syzkaller.appspot.com/bug?id=28bc35dd6f026b2878e30804670f6f0c39932f16
// status:open
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

static uintptr_t syz_open_procfs(uintptr_t a0, uintptr_t a1)
{

  char buf[128];
  memset(buf, 0, sizeof(buf));
  if (a0 == 0) {
    snprintf(buf, sizeof(buf), "/proc/self/%s", (char*)a1);
  } else if (a0 == (uintptr_t)-1) {
    snprintf(buf, sizeof(buf), "/proc/thread-self/%s", (char*)a1);
  } else {
    snprintf(buf, sizeof(buf), "/proc/self/task/%d/%s", (int)a0, (char*)a1);
  }
  int fd = open(buf, O_RDWR);
  if (fd == -1)
    fd = open(buf, O_RDONLY);
  return fd;
}

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};
void loop()
{
  long res = 0;
  memcpy((void*)0x200000c0, "/dev/loop#", 11);
  res = syz_open_dev(0x200000c0, 4, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000100, "/dev/loop#", 11);
  res = syz_open_dev(0x20000100, 0, 0);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000140, "/dev/loop#", 11);
  res = syz_open_dev(0x20000140, 4, 2);
  if (res != -1)
    r[2] = res;
  memcpy((void*)0x20000280, "net/raw", 8);
  res = syz_open_procfs(0, 0x20000280);
  if (res != -1)
    r[3] = res;
  syscall(__NR_ioctl, r[0], 0x4c00, r[3]);
  syscall(__NR_ioctl, r[0], 0x4c06, r[2]);
  syscall(__NR_ioctl, r[1], 0x4c00, r[0]);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
