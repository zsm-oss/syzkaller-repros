// kernel BUG at include/linux/mm.h:LINE! (3)
// https://syzkaller.appspot.com/bug?id=23f44ea21ad9e4529655574ecc486602304ee765
// status:invalid
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

static long syz_open_dev(long a0, long a1, long a2)
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

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000680, "/dev/loop#\000", 11);
  res = syz_open_dev(0x20000680, 0, 0x800000000105082);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x200001c0, "eth0-(eth0:\000", 12);
  res = syscall(__NR_memfd_create, 0x200001c0, 0);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x20f50f90 = 0x20000100;
  memcpy((void*)0x20000100, "\xa8", 1);
  *(uint64_t*)0x20f50f98 = 1;
  syscall(__NR_pwritev, r[1], 0x20f50f90, 1, 0x81003);
  syscall(__NR_ioctl, r[0], 0x4c00, r[1]);
  syscall(__NR_sendfile, r[0], r[0], 0, 0x102000300);
  return 0;
}
