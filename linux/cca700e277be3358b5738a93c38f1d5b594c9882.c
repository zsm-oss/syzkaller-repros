// WARNING in kmem_cache_create_usercopy
// https://syzkaller.appspot.com/bug?id=cca700e277be3358b5738a93c38f1d5b594c9882
// status:fixed
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
  res = syscall(__NR_pipe2, 0x20000100, 0);
  if (res != -1) {
    r[0] = *(uint32_t*)0x20000100;
    r[1] = *(uint32_t*)0x20000104;
  }
  memcpy((void*)0x20000140, "./file0", 8);
  syscall(__NR_mkdir, 0x20000140, 0);
  *(uint32_t*)0x20000040 = 0x15;
  *(uint8_t*)0x20000044 = 0x65;
  *(uint16_t*)0x20000045 = -1;
  *(uint32_t*)0x20000047 = 8;
  *(uint16_t*)0x2000004b = 8;
  memcpy((void*)0x2000004d, "9P2000.u", 8);
  syscall(__NR_write, r[1], 0x20000040, 0x15);
  memcpy((void*)0x20000000, "./file0", 8);
  memcpy((void*)0x200008c0, "9p", 3);
  memcpy((void*)0x20000a80, "trans=fd,", 9);
  memcpy((void*)0x20000a89, "rfdno", 5);
  *(uint8_t*)0x20000a8e = 0x3d;
  sprintf((char*)0x20000a8f, "0x%016llx", (long long)r[0]);
  *(uint8_t*)0x20000aa1 = 0x2c;
  memcpy((void*)0x20000aa2, "wfdno", 5);
  *(uint8_t*)0x20000aa7 = 0x3d;
  sprintf((char*)0x20000aa8, "0x%016llx", (long long)r[1]);
  *(uint8_t*)0x20000aba = 0x2c;
  memcpy((void*)0x20000abb, "\x63\x61\x63\x68\x65\x3d\xc0\x6d\x61\x70", 10);
  *(uint8_t*)0x20000ac5 = 0x2c;
  *(uint8_t*)0x20000ac6 = 0;
  syscall(__NR_mount, 0, 0x20000000, 0x200008c0, 0, 0x20000a80);
  return 0;
}
