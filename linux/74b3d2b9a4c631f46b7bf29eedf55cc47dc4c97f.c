// INFO: task hung in ext4_map_blocks
// https://syzkaller.appspot.com/bug?id=74b3d2b9a4c631f46b7bf29eedf55cc47dc4c97f
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000080,
         "\x73\x74\x61\x74\x09\xc0\xd2\xfe\xbc\xf9\xdf\x2d\xea\xc8\xc1\x77\xff"
         "\x17\x12\x48\xe9\x11\x93\x51\x30\x49\xf8\x31\x55\x0d\x6f\x7d\xe6\x6c"
         "\xf6\x37\xbd\xbf\x13\x11\x92\x0c\x8a\x26\xed\xa4\xdc\xc3\x78\x3f\x9d"
         "\xb5\x11\x6b\x34\xd3\x1b\x05\x12\xa5\x60\x8a\xaf\xf0\x1e\x79\x52\x34"
         "\x0c\xd6\xfd\x00\x00\x00\x00",
         75);
  res = syscall(__NR_openat, 0xffffff9c, 0x20000080, 0x275a, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000140, "./file0", 8);
  res = syscall(__NR_creat, 0x20000140, 0);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000180, "\x67\x54\xe2\xe7\x5a\x76\xa4\x38\x44\x64\x64\x03"
                            "\xdd\x7e\x18\x2f\xfd\xff\xc7\x36\xb4\x85\x05\x00"
                            "\x00\x00\x00\x00\x00\x00\x00",
         31);
  syscall(__NR_write, r[1], 0x20000180, 0x1f);
  syscall(__NR_fallocate, r[1], 1, 0x100000000, 0xffff);
  syscall(__NR_fallocate, r[0], 0, 0, 0x10001);
  *(uint32_t*)0x20000040 = 0;
  *(uint32_t*)0x20000044 = r[1];
  *(uint64_t*)0x20000048 = 1;
  *(uint64_t*)0x20000050 = 0xfffffff0;
  *(uint64_t*)0x20000058 = 0;
  *(uint64_t*)0x20000060 = 0;
  syscall(__NR_ioctl, r[0], 0xc028660f, 0x20000040);
  return 0;
}
