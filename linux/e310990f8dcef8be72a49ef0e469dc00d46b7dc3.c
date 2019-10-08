// WARNING in __mark_chain_precision
// https://syzkaller.appspot.com/bug?id=e310990f8dcef8be72a49ef0e469dc00d46b7dc3
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

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);

  *(uint32_t*)0x20000200 = 4;
  *(uint32_t*)0x20000204 = 0xe;
  *(uint64_t*)0x20000208 = 0x20000780;
  memcpy((void*)0x20000780,
         "\xb7\x02\x00\x00\x00\x00\x00\x00\xbf\xa3\x00\x00\x00\x00\x00\x00\x26"
         "\x03\x00\x00\x00\xfe\xff\xff\x7a\x03\xf0\xff\xf8\xff\xff\xff\x79\xa4"
         "\xf0\xff\x00\x00\x00\x00\xb7\x06\x00\x00\xff\xff\xff\xff\x6d\x64\x05"
         "\x00\x00\x00\x00\x00\x65\x04\x04\x00\x01\x00\x00\x00\x04\x04\x00\x00"
         "\x01\x00\x00\x00\xb7\x05\x00\x00\x00\x00\x00\x00\x6a\x0a\x80\xfe\x00"
         "\x00\x00\x00\x85\x00\x00\x00\x53\x00\x00\x00\xb7\x00\x00\x00\x00\x00"
         "\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00\x4e\x62\x01\x36\x30\x34\xfd"
         "\xb1\x17\x16\x8b\xd0\x7b\xa0\x8a\xf3\x39\xd1\xa1\xee\x35\xfe\x2a\x3a"
         "\x25\x5c\x33\x28\x20\x44\xb3\x24\x95\x3c\x0a\x9f\xa9\xa8\x44\x52\x56"
         "\x99\x57\xc1\x00\x2e\xd7\xd4\xd8\xe1\x7f\x79\x1f\x47\x98\xc8\xeb\x48"
         "\x4d\xe0\x33\x52\xc6\x9b\x3e\xdf\xf5\xbe\x26\x76\x5b\xa5\xf8\xf2\x87"
         "\x90\x21\xc2\xea\x53\xea\x79\xac\xd7\xfb\x38\xfd\xf7\x9f\x2b\xe9\x08"
         "\x7a\x3e\x7b\x7c\x4a\xe7\xdd\x5e\x4d\xee\x88\x51\xd4\x0c\x61\x7b\x58"
         "\xc8\x10\x8d\xdf\x12\xdd\xdd\x4b\xfc\x6a\x4d\xd3\x53\x83\x56\x1c\xbe"
         "\x04\x58\xf1\xf5\xb6\xbe\xba\x51\x0b\x42\x29\xb0\xd4\xb5\x04\x51\x6c"
         "\x4c\x3e\x5d\x1a\xa0\x44\xd8\xd0\x07\x28\x14\x1c\xd6\x7b\xcd\x68\xf2"
         "\x53\x28\x8e\x65\x5c\x6b\x34\xe0\x2e\x90\x63\x7e\xf2\x91\x2b\xa7\xde"
         "\x26\xff\x23\x57\xef\x17\xf9\x5a\x25\x78\x0c\x3a\x05\x78\x44\x29\x26"
         "\xef\x4e\x91\x2f\x01\xa2\x01\xe6\x94\xe3\x80\x6e\x8c\x8f\xe8",
         321);
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
  syscall(__NR_bpf, 5, 0x20000200, 0x48);
  return 0;
}
