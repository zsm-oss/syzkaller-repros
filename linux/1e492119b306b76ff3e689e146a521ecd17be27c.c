// KASAN: invalid-free in p9stat_free
// https://syzkaller.appspot.com/bug?id=1e492119b306b76ff3e689e146a521ecd17be27c
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

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  res = syscall(__NR_pipe2, 0x20000100, 0);
  if (res != -1) {
    r[0] = *(uint32_t*)0x20000100;
    r[1] = *(uint32_t*)0x20000104;
  }
  *(uint32_t*)0x20000040 = 0xffffffca;
  *(uint8_t*)0x20000044 = 0x4d;
  *(uint16_t*)0x20000045 = 0;
  syscall(__NR_write, r[1], 0x20000040, 7);
  memcpy((void*)0x20000300, "./file0", 8);
  syscall(__NR_mkdir, 0x20000300, 0);
  memcpy((void*)0x200000c0, "./file0", 8);
  memcpy((void*)0x20000340, "9p", 3);
  memcpy((void*)0x20000640, "trans=fd,", 9);
  memcpy((void*)0x20000649, "rfdno", 5);
  *(uint8_t*)0x2000064e = 0x3d;
  sprintf((char*)0x2000064f, "0x%016llx", (long long)r[0]);
  *(uint8_t*)0x20000661 = 0x2c;
  memcpy((void*)0x20000662, "wfdno", 5);
  *(uint8_t*)0x20000667 = 0x3d;
  sprintf((char*)0x20000668, "0x%016llx", (long long)r[1]);
  *(uint8_t*)0x2000067a = 0x2c;
  memcpy((void*)0x2000067b, "noextend", 8);
  *(uint8_t*)0x20000683 = 0x2c;
  *(uint8_t*)0x20000684 = 0;
  syscall(__NR_mount, 0, 0x200000c0, 0x20000340, 0, 0x20000640);
  memcpy((void*)0x20000480, "\x2a\x00\x00\x00\x29\x01\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\xb5\xe6\xbd\xbe\xd0\x77\x0c\xf0"
                            "\x84\xfe\x43\xb7\xef\x98\x4c\x3e\x00\x00\xa7\x3d"
                            "\x11\x10\x8f\x87\x24\x00",
         42);
  syscall(__NR_write, r[1], 0x20000480, 0x2a);
  *(uint32_t*)0x20000200 = 0xa0;
  *(uint8_t*)0x20000204 = 0x19;
  *(uint16_t*)0x20000205 = 1;
  *(uint64_t*)0x20000207 = 0;
  *(uint8_t*)0x2000020f = 0;
  *(uint32_t*)0x20000210 = 0;
  *(uint64_t*)0x20000214 = 0;
  *(uint32_t*)0x2000021c = 0;
  *(uint32_t*)0x20000220 = 0;
  *(uint32_t*)0x20000224 = 0;
  *(uint64_t*)0x20000228 = 0;
  *(uint64_t*)0x20000230 = 0;
  *(uint64_t*)0x20000238 = 0;
  *(uint64_t*)0x20000240 = 0;
  *(uint64_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0;
  *(uint64_t*)0x20000258 = 0;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint64_t*)0x20000270 = 0;
  *(uint64_t*)0x20000278 = 0;
  *(uint64_t*)0x20000280 = 0;
  *(uint64_t*)0x20000288 = 0;
  *(uint64_t*)0x20000290 = 0;
  *(uint64_t*)0x20000298 = 0;
  syscall(__NR_write, r[1], 0x20000200, 0xa0);
  *(uint32_t*)0x20000500 = 9;
  *(uint8_t*)0x20000504 = 0x6f;
  *(uint16_t*)0x20000505 = 1;
  *(uint16_t*)0x20000507 = 0;
  syscall(__NR_write, r[1], 0x20000500, 9);
  *(uint32_t*)0x20000080 = 0x18;
  *(uint8_t*)0x20000084 = 0x73;
  *(uint16_t*)0x20000085 = 1;
  *(uint8_t*)0x20000087 = 0;
  *(uint32_t*)0x20000088 = 0;
  *(uint64_t*)0x2000008c = 0;
  *(uint32_t*)0x20000094 = 0x81;
  syscall(__NR_write, r[1], 0x20000080, 0x18);
  *(uint32_t*)0x20000380 = 0x68;
  *(uint8_t*)0x20000384 = 0x29;
  *(uint16_t*)0x20000385 = 1;
  *(uint32_t*)0x20000387 = 0x40;
  *(uint8_t*)0x2000038b = 0x80;
  *(uint32_t*)0x2000038c = 2;
  *(uint64_t*)0x20000390 = 4;
  *(uint64_t*)0x20000398 = 2;
  *(uint8_t*)0x200003a0 = 7;
  *(uint16_t*)0x200003a1 = 7;
  memcpy((void*)0x200003a3, "./file0", 7);
  *(uint8_t*)0x200003aa = 2;
  *(uint32_t*)0x200003ab = 4;
  *(uint64_t*)0x200003af = 8;
  *(uint64_t*)0x200003b7 = 1;
  *(uint8_t*)0x200003bf = 0x62;
  *(uint16_t*)0x200003c0 = 7;
  memcpy((void*)0x200003c2, "./file0", 7);
  *(uint8_t*)0x200003c9 = 0x21;
  *(uint32_t*)0x200003ca = 1;
  *(uint64_t*)0x200003ce = 8;
  *(uint64_t*)0x200003d6 = 1;
  *(uint8_t*)0x200003de = 0x77;
  *(uint16_t*)0x200003df = 7;
  memcpy((void*)0x200003e1, "./file0", 7);
  syscall(__NR_write, r[1], 0x20000380, 0x68);
  *(uint32_t*)0x20000600 = 0xb;
  *(uint8_t*)0x20000604 = 0x29;
  *(uint16_t*)0x20000605 = 1;
  *(uint32_t*)0x20000607 = 0;
  syscall(__NR_write, r[1], 0x20000600, 0xb);
  memcpy((void*)0x20000140, "./file0", 8);
  memcpy((void*)0x20000900, "9p", 3);
  memcpy((void*)0x20000840, "trans=fd,", 9);
  memcpy((void*)0x20000849, "rfdno", 5);
  *(uint8_t*)0x2000084e = 0x3d;
  sprintf((char*)0x2000084f, "0x%016llx", (long long)r[0]);
  *(uint8_t*)0x20000861 = 0x2c;
  memcpy((void*)0x20000862, "wfdno", 5);
  *(uint8_t*)0x20000867 = 0x3d;
  sprintf((char*)0x20000868, "0x%016llx", (long long)r[1]);
  *(uint8_t*)0x2000087a = 0x2c;
  *(uint8_t*)0x2000087b = 0;
  syscall(__NR_mount, 0, 0x20000140, 0x20000900, 0, 0x20000840);
  memcpy((void*)0x200013c0, "./file0", 8);
  res = syscall(__NR_open, 0x200013c0, 0, 0);
  if (res != -1)
    r[2] = res;
  syscall(__NR_getdents, r[2], 0x20000180, 8);
  return 0;
}
