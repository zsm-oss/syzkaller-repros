// kernel BUG at mm/slab.c:LINE!
// https://syzkaller.appspot.com/bug?id=af6325c391abd41d0d5e1d0fd6cac6c1b2860fa1
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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  intptr_t res = 0;
  *(uint32_t*)0x20000200 = 0xc;
  *(uint32_t*)0x20000204 = 0xe;
  *(uint64_t*)0x20000208 = 0x200007c0;
  memcpy((void*)0x200007c0,
         "\xb7\x02\x00\x00\x03\x00\x00\x00\xbf\xa3\x00\x00\x00\x00\x00\x00\x07"
         "\x03\x00\x00\x00\xfe\xff\xff\x7a\x0a\xf0\xff\xf8\xff\xff\xff\x79\xa4"
         "\xf0\xff\x00\x00\x00\x00\xb7\x06\x00\x00\xff\xff\xff\xff\x2d\x64\x05"
         "\x00\x00\x00\x00\x00\x65\x04\x04\x00\x01\x00\x00\x00\x04\x04\x00\x00"
         "\x01\x07\x7d\x60\xb7\x03\x00\x00\x00\x00\x00\x00\x6a\x0a\x00\xfe\x18"
         "\x00\x00\x00\x85\x00\x00\x00\x2b\x00\x00\x00\xb7\x00\x00\x00\x00\x00"
         "\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00\x0d\x7a\x28\x68\x3a\x41\x02"
         "\xe4\xae\xb5\x0f\x36\x63\x3e\x27\xc2\x79\x34\x1b\xf4\x89\x90\x3c\xfd"
         "\xb4\xc0\x5e\x96\xe3\x04\x6f\x04\xe7\x79\x69\xbe\x06\x34\x67\x4d\x09"
         "\x42\xb6\x6b\x24\x9b\x35\x18\x59\xbb\x7e\x29\x0b\x39\xa6\xf2\xfc\x2a"
         "\x5e\x01\x9b\xc6\xb4\x56\x84\xf0\x02\xcf\x57\xbf\x88\x7e\x83\xfb\xb2"
         "\x21\x5b\x8a\x34\xe6\xbd\xc4\xdc\x1a\xf6\xd3\xc6\x95\x8d\xa4\xbd\xda"
         "\xc6\x02\xe0\x04\x8b\xec\x11\xe8\x74\x60\x2f\x92\x5b\x6d\xbd\x2a\xf2"
         "\x1b\x75\xfe\x26\xaa\x5e\x85\xae\x28\x19\x93\xbf\xa2\x13\x92\x3b\xb7"
         "\x55\xc1\xf1\xab\xd1\x96\x40\x7b\x4c\x8f\xab\xe2\x7b\x21\x21\xa5\xf0"
         "\x3d\xff\x2c",
         258);
  *(uint64_t*)0x20000210 = 0x20000340;
  memcpy((void*)0x20000340, "syzkaller\000", 10);
  *(uint32_t*)0x20000218 = 0;
  *(uint32_t*)0x2000021c = 0;
  *(uint64_t*)0x20000220 = 0;
  *(uint32_t*)0x20000228 = 0;
  *(uint32_t*)0x2000022c = 0;
  *(uint8_t*)0x20000230 = 0;
  *(uint8_t*)0x20000231 = 0;
  *(uint8_t*)0x20000232 = 0;
  *(uint8_t*)0x20000233 = 0;
  *(uint8_t*)0x20000234 = 0;
  *(uint8_t*)0x20000235 = 0;
  *(uint8_t*)0x20000236 = 0;
  *(uint8_t*)0x20000237 = 0;
  *(uint8_t*)0x20000238 = 0;
  *(uint8_t*)0x20000239 = 0;
  *(uint8_t*)0x2000023a = 0;
  *(uint8_t*)0x2000023b = 0;
  *(uint8_t*)0x2000023c = 0;
  *(uint8_t*)0x2000023d = 0;
  *(uint8_t*)0x2000023e = 0;
  *(uint8_t*)0x2000023f = 0;
  *(uint32_t*)0x20000240 = 0;
  *(uint32_t*)0x20000244 = 0;
  *(uint32_t*)0x20000248 = -1;
  *(uint32_t*)0x2000024c = 8;
  *(uint64_t*)0x20000250 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000258 = 0;
  *(uint32_t*)0x2000025c = 0x10;
  *(uint64_t*)0x20000260 = 0x20000000;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint32_t*)0x20000268 = 0;
  res = syscall(__NR_bpf, 5, 0x20000200, 0x48);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000140 = r[0];
  *(uint32_t*)0x20000144 = 0;
  *(uint32_t*)0x20000148 = 0x16;
  *(uint32_t*)0x2000014c = 0xb9;
  *(uint64_t*)0x20000150 = 0x20000040;
  memcpy((void*)0x20000040, "\xb5\xfd\xd6\x89\x29\x84\x64\x12\xb0\x31\xe0\x4f"
                            "\x70\x90\xf7\x82\x72\x8d\x39\x42\x44\xf4",
         22);
  *(uint64_t*)0x20000158 = 0x20000080;
  *(uint32_t*)0x20000160 = 0x48b;
  *(uint32_t*)0x20000164 = 0;
  syscall(__NR_bpf, 0xa, 0x20000140, 0x28);
  return 0;
}
