// KASAN: slab-out-of-bounds Read in bpf_skb_vlan_push
// https://syzkaller.appspot.com/bug?id=8c2bb3cbc161a43ed2e5f435263ef5d03bb9e36e
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
  long res = 0;
  *(uint32_t*)0x20000200 = 4;
  *(uint32_t*)0x20000204 = 0xe;
  *(uint64_t*)0x20000208 = 0x20000280;
  memcpy((void*)0x20000280,
         "\xb7\x02\x00\x00\x00\x00\x00\x00\xbf\xa3\x00\x00\x00\x00\x00\x00\x07"
         "\x01\x00\x00\x00\xfe\xff\xff\x7a\x0a\xf0\xff\xf8\xff\xff\xff\x79\xa4"
         "\xf0\xff\x00\x00\x00\x00\xb7\x06\x00\x00\xff\xff\xff\xff\x2d\x64\x05"
         "\x00\x00\x00\x00\x00\x65\x04\x04\x00\x01\x00\x00\x00\x04\x04\x00\x00"
         "\x01\x00\x00\x00\xb7\x05\x00\x00\x00\x00\x00\x00\x6a\x0a\x00\xfe\x00"
         "\x00\x00\x00\x85\x00\x00\x00\x12\x00\x00\x00\xb7\x00\x00\x00\x00\x00"
         "\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00",
         112);
  *(uint64_t*)0x20000210 = 0x20000000;
  memcpy((void*)0x20000000, "syzkaller", 10);
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
  res = syscall(__NR_bpf, 5, 0x20000200, 0x48);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000140 = r[0];
  *(uint32_t*)0x20000144 = 0;
  *(uint32_t*)0x20000148 = 0xa9;
  *(uint32_t*)0x2000014c = 0xfd;
  *(uint64_t*)0x20000150 = 0x20000600;
  memcpy((void*)0x20000600,
         "\x12\x1d\x9b\x15\x11\xa3\x4b\xc4\xfd\xe6\xa9\xc9\xce\x6e\x5a\x03\x8a"
         "\x2d\x81\x10\x5d\x79\xbc\xbd\x08\xc5\xeb\x7e\xc4\x69\x9b\xe7\x06\x4f"
         "\x0c\xd1\x48\x80\xf1\xaa\x5c\x11\x54\x59\x76\x07\xbe\xeb\x2c\xb8\xc3"
         "\xb8\xb8\xdb\x78\xaf\x9f\x34\x18\x9b\x76\x1a\xdc\xb5\xc1\x61\x36\x93"
         "\xd3\x30\x2e\xb1\x90\xe2\xe6\x5e\x14\x77\xe7\x13\x74\x54\xc1\xe5\x2d"
         "\xbf\xf0\x81\x0d\x6e\x7a\x1a\x05\x34\x51\xd0\xc4\xd8\x77\x6e\xa7\xa6"
         "\x9d\xc7\x15\xb2\x90\x26\x7d\x4c\xfc\xe5\x3c\xb2\x60\x9d\xe0\xac\x39"
         "\xc7\xb7\x71\xc0\x85\x0a\xb9\x70\xde\x91\x9f\x2d\x22\x3a\x91\xe6\x22"
         "\xe5\x85\x6d\xa9\xea\x30\x6e\x35\x3c\xbb\xd5\x28\x08\xfb\x12\x2a\xb7"
         "\x38\xa0\x8a\x59\x88\xa9\x81\x33\xbf\x27\xce\xf7\x9a\x42\xc1\x66",
         169);
  *(uint64_t*)0x20000158 = 0x20000480;
  *(uint32_t*)0x20000160 = 0x6db9;
  *(uint32_t*)0x20000164 = 0;
  syscall(__NR_bpf, 0xa, 0x20000140, 0x28);
  return 0;
}
