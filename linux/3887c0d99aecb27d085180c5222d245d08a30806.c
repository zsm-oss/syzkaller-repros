// KMSAN: uninit-value in memcmp
// https://syzkaller.appspot.com/bug?id=3887c0d99aecb27d085180c5222d245d08a30806
// status:fixed
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
    strncpy(buf, (char*)a0, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 0x11, 0x100000802, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000240, "/dev/net/tun", 13);
  res = syz_open_dev(0x20000240, 0, 0);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x200000c0,
         "\x69\x66\x62\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint16_t*)0x200000d0 = 0x4012;
  syscall(__NR_ioctl, r[1], 0x400454ca, 0x200000c0);
  syscall(__NR_ioctl, r[1], 0x400454cd, 0x30a);
  memcpy((void*)0x20000040,
         "\x69\x66\x62\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint16_t*)0x20000050 = 0xa201;
  syscall(__NR_ioctl, r[0], 0x8914, 0x20000040);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
