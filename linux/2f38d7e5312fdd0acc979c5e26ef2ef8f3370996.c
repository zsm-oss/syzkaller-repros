// possible deadlock in cma_netdev_callback
// https://syzkaller.appspot.com/bug?id=2f38d7e5312fdd0acc979c5e26ef2ef8f3370996
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
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0x14ul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x200031c0 = 0;
  *(uint32_t*)0x200031c8 = 0;
  *(uint64_t*)0x200031d0 = 0x20003180;
  *(uint64_t*)0x20003180 = 0x20000000;
  *(uint32_t*)0x20000000 = 0x38;
  *(uint16_t*)0x20000004 = 0x1403;
  *(uint16_t*)0x20000006 = 1;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint16_t*)0x20000010 = 9;
  *(uint16_t*)0x20000012 = 2;
  memcpy((void*)0x20000014, "syz1\000", 5);
  *(uint16_t*)0x2000001c = 8;
  *(uint16_t*)0x2000001e = 0x41;
  memcpy((void*)0x20000020, "siw\000", 4);
  *(uint16_t*)0x20000024 = 0x14;
  *(uint16_t*)0x20000026 = 0x33;
  memcpy((void*)0x20000028,
         "lo\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 16);
  *(uint64_t*)0x20003188 = 0x38;
  *(uint64_t*)0x200031d8 = 1;
  *(uint64_t*)0x200031e0 = 0;
  *(uint64_t*)0x200031e8 = 0;
  *(uint32_t*)0x200031f0 = 0;
  syscall(__NR_sendmsg, r[0], 0x200031c0ul, 0ul);
  res = syscall(__NR_socket, 0x10ul, 3ul, 0ul);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0ul);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x20000040 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint64_t*)0x20000050 = 0x20000000;
  *(uint64_t*)0x20000000 = 0x20000380;
  memcpy((void*)0x20000380, "\x3c\x00\x00\x00\x10\x00\x85\x06\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         20);
  *(uint32_t*)0x20000394 = -1;
  memcpy((void*)0x20000398, "\x03\x00\x00\x00\x00\x00\x00\x00\x1c\x00\x12\x00"
                            "\x0c\x00\x01\x00\x62\x6f\x6e\x64\x00\x00\x00\x00"
                            "\x0c\x00\x02\x00\x08\x00\x01\x00\x01\x00\x00\x00",
         36);
  *(uint64_t*)0x20000008 = 0x3c;
  *(uint64_t*)0x20000058 = 1;
  *(uint64_t*)0x20000060 = 0;
  *(uint64_t*)0x20000068 = 0;
  *(uint32_t*)0x20000070 = 0;
  syscall(__NR_sendmsg, r[2], 0x20000040ul, 0ul);
  *(uint64_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c8 = 0xfffffe1e;
  *(uint64_t*)0x200000d0 = 0x20000200;
  *(uint64_t*)0x20000200 = 0x20000540;
  memcpy((void*)0x20000540, "\x4c\x00\x00\x00\x10\x00\x1f\xff\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         20);
  *(uint32_t*)0x20000554 = 0;
  memcpy((void*)0x20000558, "\x00\x00\x00\x00\x00\x00\x00\x00\x24\x00\x12\x00"
                            "\x0b\x00\x01\x00\x67\x72\x65\x74\x61\x70\x00\x00"
                            "\x14\x00\x02\x00\x08\x00\x01\x00",
         32);
  *(uint32_t*)0x20000578 = -1;
  memcpy((void*)0x2000057c, "\x08\x00\x07\x00\xff\xff\xff\xff\x08\x00\x0a\x00",
         12);
  *(uint32_t*)0x20000588 = -1;
  memcpy((void*)0x2000058c,
         "\x6e\xc8\x19\x2b\x70\x7b\xdd\xfe\x6d\x3c\x2b\xfb\x6a\x8c\xfe\x70\x1b"
         "\xc6\x4c\x4d\xeb\xad\xf7\x80\x5e\x0f\xa2\x45\x64\xb3\xa2\x3c\xce\x9a"
         "\x2e\x17\xd7\xee\x4b\x83\x57\xb9\x31\x71\xf5\xb5\x65\x6f\x2b\x03\xd9"
         "\x9a\x4c\xe3\xc6\x8d\x38\x32\xde\x56\x01\xbb\x8a\x2d\x2e\xf0\xc8\x18"
         "\xbe\xcd\xb8\x9a\xac\xfc\xc2\x52\xf6\xf2\xb9\x25\xb4\xe5\xc3\x36\x79"
         "\x9c\xf9\x76\x94\x66\x2d\x98\x75\x2b\x54\xa0\x3b\xce\xca\x24\x75\x6f"
         "\xff\xd3\x5b\xa0\xd0\x35\xa8\x35\xbf\x21\xa7\x64\xf0\x2f\x8e\x80\x5d"
         "\x2a\x49\x92\x23\x91\x5d\x9d\x36\x63\xf0\x01\xd9\x25\x12\xc5\x76\x9c"
         "\x51\x77\x7d\x9c\x9d\x0b\x4b\x11\x6f\x0b\x8f\x74\xfe\xba\xfd\x4e\x19"
         "\x78\x1c\x21\xe1\x42\xf2\xe2\x3d\x22\x5b\x2d\xbd\xe2\xd1\x91\xc6\xfb"
         "\xa8\x43\xeb\x07\x65\xba\x6d\x24\x23\x73\x17\xcb\xab\xdd\x2a\x10\xaa"
         "\x71\x55\x9a\xae\x09\xe7\x75\xa0\xf7\xc2\x58\x93\xf3\x5a\x7e\x43\xff"
         "\x67\x3c\xf8\xe1\x73\x3b\x0a\xc5\xf6\xb5\x2b\xed\x00\x86\x2f\x76\xc1"
         "\xd2\x85",
         223);
  *(uint64_t*)0x20000208 = 0x4c;
  *(uint64_t*)0x200000d8 = 1;
  *(uint64_t*)0x200000e0 = 0;
  *(uint64_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000f0 = 0;
  syscall(__NR_sendmsg, r[1], 0x200000c0ul, 0ul);
  return 0;
}
