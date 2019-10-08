// KASAN: slab-out-of-bounds Read in sock_hash_ctx_update_elem
// https://syzkaller.appspot.com/bug?id=4387b587226bb5f873bcf7dc8febc50c2dd3c540
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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  syscall(__NR_socketpair, 0, 0, 0, 0x20000140);
  res = syscall(__NR_socket, 2, 1, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20e5b000 = 2;
  *(uint16_t*)0x20e5b002 = htobe16(0x4e20);
  *(uint32_t*)0x20e5b004 = htobe32(0xe0000001);
  *(uint8_t*)0x20e5b008 = 0;
  *(uint8_t*)0x20e5b009 = 0;
  *(uint8_t*)0x20e5b00a = 0;
  *(uint8_t*)0x20e5b00b = 0;
  *(uint8_t*)0x20e5b00c = 0;
  *(uint8_t*)0x20e5b00d = 0;
  *(uint8_t*)0x20e5b00e = 0;
  *(uint8_t*)0x20e5b00f = 0;
  syscall(__NR_bind, r[0], 0x20e5b000, 0x10);
  *(uint16_t*)0x20ccb000 = 2;
  *(uint16_t*)0x20ccb002 = htobe16(0x4e20);
  *(uint32_t*)0x20ccb004 = htobe32(0);
  *(uint8_t*)0x20ccb008 = 0;
  *(uint8_t*)0x20ccb009 = 0;
  *(uint8_t*)0x20ccb00a = 0;
  *(uint8_t*)0x20ccb00b = 0;
  *(uint8_t*)0x20ccb00c = 0;
  *(uint8_t*)0x20ccb00d = 0;
  *(uint8_t*)0x20ccb00e = 0;
  *(uint8_t*)0x20ccb00f = 0;
  syscall(__NR_connect, r[0], 0x20ccb000, 0x10);
  *(uint32_t*)0x20000340 = 1;
  *(uint32_t*)0x20000344 = 3;
  *(uint64_t*)0x20000348 = 0x20000000;
  memcpy((void*)0x20000000, "\x18\x00\x00\x00\x00\x00\x20\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00",
         24);
  *(uint64_t*)0x20000350 = 0x202bf000;
  memcpy((void*)0x202bf000, "syzkaller", 10);
  *(uint32_t*)0x20000358 = 1;
  *(uint32_t*)0x2000035c = 0xb7;
  *(uint64_t*)0x20000360 = 0x20000440;
  *(uint32_t*)0x20000368 = 0;
  *(uint32_t*)0x2000036c = 0;
  *(uint8_t*)0x20000370 = 0;
  *(uint8_t*)0x20000371 = 0;
  *(uint8_t*)0x20000372 = 0;
  *(uint8_t*)0x20000373 = 0;
  *(uint8_t*)0x20000374 = 0;
  *(uint8_t*)0x20000375 = 0;
  *(uint8_t*)0x20000376 = 0;
  *(uint8_t*)0x20000377 = 0;
  *(uint8_t*)0x20000378 = 0;
  *(uint8_t*)0x20000379 = 0;
  *(uint8_t*)0x2000037a = 0;
  *(uint8_t*)0x2000037b = 0;
  *(uint8_t*)0x2000037c = 0;
  *(uint8_t*)0x2000037d = 0;
  *(uint8_t*)0x2000037e = 0;
  *(uint8_t*)0x2000037f = 0;
  *(uint32_t*)0x20000380 = 0;
  *(uint32_t*)0x20000384 = 0;
  res = syscall(__NR_bpf, 5, 0x20000340, 0x48);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_socket, 0x29, 0x1000000000002, 0);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x200000c0 = r[0];
  *(uint32_t*)0x200000c4 = r[1];
  syscall(__NR_ioctl, r[2], 0x89e0, 0x200000c0);
  *(uint32_t*)0x20000280 = 0x12;
  *(uint32_t*)0x20000284 = 0;
  *(uint32_t*)0x20000288 = 4;
  *(uint32_t*)0x2000028c = 1;
  *(uint32_t*)0x20000290 = 0;
  *(uint32_t*)0x20000294 = -1;
  *(uint32_t*)0x20000298 = 0;
  *(uint8_t*)0x2000029c = 0;
  *(uint8_t*)0x2000029d = 0;
  *(uint8_t*)0x2000029e = 0;
  *(uint8_t*)0x2000029f = 0;
  *(uint8_t*)0x200002a0 = 0;
  *(uint8_t*)0x200002a1 = 0;
  *(uint8_t*)0x200002a2 = 0;
  *(uint8_t*)0x200002a3 = 0;
  *(uint8_t*)0x200002a4 = 0;
  *(uint8_t*)0x200002a5 = 0;
  *(uint8_t*)0x200002a6 = 0;
  *(uint8_t*)0x200002a7 = 0;
  *(uint8_t*)0x200002a8 = 0;
  *(uint8_t*)0x200002a9 = 0;
  *(uint8_t*)0x200002aa = 0;
  *(uint8_t*)0x200002ab = 0;
  res = syscall(__NR_bpf, 0, 0x20000280, 0x2c);
  if (res != -1)
    r[3] = res;
  memcpy((void*)0x200001c0,
         "\x7f\x45\x4c\x46\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x30\xaa\x8c\x18\x2e\x7a\xe2\x04\x44\xd9"
         "\x90\x86\x00\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x38\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         132);
  syscall(__NR_write, r[2], 0x200001c0, 0x84);
  *(uint32_t*)0x20000180 = r[3];
  *(uint64_t*)0x20000188 = 0x20000000;
  *(uint64_t*)0x20000190 = 0x20000140;
  *(uint64_t*)0x20000198 = 0;
  syscall(__NR_bpf, 2, 0x20000180, 0x20);
  return 0;
}
