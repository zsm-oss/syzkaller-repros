// KMSAN: uninit-value in ip6_parse_tlv
// https://syzkaller.appspot.com/bug?id=a446d3718ee6322911a0c6d34db57909e1838fe7
// status:open
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 0xa, 0x80003, 0xef);
  if (res != -1)
    r[0] = res;
  *(uint8_t*)0x20000000 = 0x2b;
  *(uint8_t*)0x20000001 = 1;
  *(uint8_t*)0x20000002 = 0;
  *(uint8_t*)0x20000003 = 0;
  *(uint8_t*)0x20000004 = 0;
  *(uint8_t*)0x20000005 = 0;
  *(uint8_t*)0x20000006 = 0;
  *(uint8_t*)0x20000007 = 0;
  *(uint8_t*)0x20000008 = 5;
  *(uint8_t*)0x20000009 = 2;
  *(uint16_t*)0x2000000a = htobe16(7);
  *(uint8_t*)0x2000000c = 0;
  *(uint8_t*)0x2000000d = 1;
  *(uint8_t*)0x2000000e = 0;
  *(uint8_t*)0x2000000f = 5;
  *(uint8_t*)0x20000010 = 2;
  *(uint16_t*)0x20000011 = htobe16(0x40);
  *(uint8_t*)0x20000013 = 5;
  *(uint8_t*)0x20000014 = 2;
  *(uint16_t*)0x20000015 = htobe16(0x400);
  syscall(__NR_setsockopt, r[0], 0x29, 0x3b, 0x20000000, 0x18);
  *(uint16_t*)0x20003000 = 0xa;
  *(uint16_t*)0x20003002 = htobe16(0);
  *(uint32_t*)0x20003004 = 0;
  *(uint8_t*)0x20003008 = -1;
  *(uint8_t*)0x20003009 = 2;
  *(uint8_t*)0x2000300a = 0;
  *(uint8_t*)0x2000300b = 0;
  *(uint8_t*)0x2000300c = 0;
  *(uint8_t*)0x2000300d = 0;
  *(uint8_t*)0x2000300e = 0;
  *(uint8_t*)0x2000300f = 0;
  *(uint8_t*)0x20003010 = 0;
  *(uint8_t*)0x20003011 = 0;
  *(uint8_t*)0x20003012 = 0;
  *(uint8_t*)0x20003013 = 0;
  *(uint8_t*)0x20003014 = 0;
  *(uint8_t*)0x20003015 = 0;
  *(uint8_t*)0x20003016 = 0;
  *(uint8_t*)0x20003017 = 1;
  *(uint32_t*)0x20003018 = 0;
  syscall(__NR_sendto, r[0], 0x20001ffe, 0, 0, 0x20003000, 0x1c);
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint32_t*)0x20000010 = 0;
  *(uint32_t*)0x20000014 = 0;
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
