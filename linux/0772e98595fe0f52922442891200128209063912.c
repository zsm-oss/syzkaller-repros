// KASAN: stack-out-of-bounds Write in __ip_tunnel_create
// https://syzkaller.appspot.com/bug?id=0772e98595fe0f52922442891200128209063912
// status:fixed
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
  memcpy((void*)0x20000000,
         "\x5f\xb8\x8c\x75\x01\x96\x07\x7a\xa3\x21\x4b\xeb\xc1\x3f\x7d\x74\xad"
         "\x4d\x93\x20\x61\x1c\xf5\xa3\x3f\x2a\x3f\x09\xe1\xc9\x26\x35\x9d\xcf"
         "\xe2\x00\x01\xc7\x81\xcb\x67\xf7\x9c\xb0\xc6\xf5\xf4\xbf\x8e\xd6\x0d"
         "\xce\x21\x76\xc2\x2a\x20\xb2\x2b\x9b\x24\x17\x2b\x55\x69\xec\x67\x90"
         "\x6e\x7d\xf9\x3b\x05\xb1\x9b\xa5\x36\x0f\x2e\xab\x62\x81\x20\x70\xfc"
         "\x18\x1a\x24\x83\x36\x65\x95\x11\x3b\x24\x08\x04\xd8\x82\xa7\xdc\xd7"
         "\xac\x4b\xa5\x3e\x06\xd0\x25\x58\x96\x8b\x97\x81\x22\x71\x6d\x18\x23"
         "\xdc\xc7\xd4\x01\x55\xc1\xbc\x1e\x84\x33\xf5\xba\x15\xf5\xee\x48\xc0"
         "\xca\x94\x1f\xce\x87\x2e\x49\x7d\x13\x77\xe2\xa5\xbd\xe9\x56\xf0\x03"
         "\xca\xaf\x58\xc7\x52\x0f\x82\xd7\x34\x6c\x26\x6e\x8e\xa7\x0d\xab\x3b"
         "\xb7\xaf\x6d\x78\x60\x2a\x31\xab\x8d\x23\x2b\x07\xf6\xe3\xef\x52\x4b"
         "\x55\x2e\xd8\x8a\x1b\x1c\x02\xbc\x89\xf4\xf6\x71\x85\x5d\x40\xa2\xc1"
         "\xc1\x73\xd4\xbb\x12\x1b\xc8\x62\x70\xc3\x2d\x39\xc4\xc0\xd0\x9a\x29"
         "\xb8\x98\x31\x69\x91\x4d\x8d\xf8\x64\x17\xa8\x02\xb7\x72\xbb\x63\x8f"
         "\x72\xcc\xdc\xbb\x46\x26\x7b\x57\x51\x81\x6c\x77\xb6\x73\x9f\x48\x42"
         "\x92\xd5\xca\x5e\x1b\xc4\xef\xee\x06\xf4\xe0\x7e\x6a\x79\x5f\xae\xe2"
         "\xaf\x27\xa8\xf2\x12\x98\x05\x58\x06\x49\x4e\xe0\xe9\x61\xcd\x5b\xd0"
         "\x77\xbd\x77\x56\xf9\xe3\x9d\xd3\xab\xdb\xe5\xc5\x98\xcc\x5c\xbd\x44"
         "\xc5\xc3\x19\xaf\x92\x3f\xbc\x47\x31\x54\x5f\xf1\x34\x74\x9c\x2f\x42"
         "\xee\x84\x54\xbf\x33\xef\x30\x9a\x63\x23\xdc\x84\x76\x76\xd5\xbe\x2f"
         "\xcb\x9a\xbc\x71\x7f\xc1\xe2\x93\x96\x94\x3e\xc0\xe2\x64\xa8\x0d\x64"
         "\xca\x66\xef\x2c\x42\xa5\x71\x12\xa8\xa1\x46\x09\x7d\x9f\x42\x56\xef"
         "\x6e\x6f\xd7\x3b\xe2\x6e\x9a\x5f\x79\xb2\x1d\xae\x42\x06\x6d\x9c\xf4"
         "\x44\x10\x21\xd6\x4a\x2d\x6f\xd6\x1c\x7e\xa7\x09\x94\x8d\x4b\xe3\x80"
         "\xe0\xfa\x4b\x76\xdf\xf6\xae\xef\x45",
         417);
  *(uint16_t*)0x20001000 = 0xa;
  *(uint16_t*)0x20001002 = htobe16(0);
  *(uint32_t*)0x20001004 = 0;
  *(uint8_t*)0x20001008 = -1;
  *(uint8_t*)0x20001009 = 2;
  *(uint8_t*)0x2000100a = 0;
  *(uint8_t*)0x2000100b = 0;
  *(uint8_t*)0x2000100c = 0;
  *(uint8_t*)0x2000100d = 0;
  *(uint8_t*)0x2000100e = 0;
  *(uint8_t*)0x2000100f = 0;
  *(uint8_t*)0x20001010 = 0;
  *(uint8_t*)0x20001011 = 0;
  *(uint8_t*)0x20001012 = 0;
  *(uint8_t*)0x20001013 = 0;
  *(uint8_t*)0x20001014 = 0;
  *(uint8_t*)0x20001015 = 0;
  *(uint8_t*)0x20001016 = 0;
  *(uint8_t*)0x20001017 = 1;
  *(uint32_t*)0x20001018 = 0;
  syscall(__NR_sendto, -1, 0x20000000, 0x1a1, 0, 0x20001000, 0x1c);
  res = syscall(__NR_socket, 0xa, 0x20000000000005, 0x84);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000240,
         "\x74\x75\x6e\x6c\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint64_t*)0x20000250 = 0x20000180;
  memcpy((void*)0x20000180, "\xd6\x1a\x07\x2a\xfa\x6a\x2e\x6e\x91\x05\xa6\x5c"
                            "\xf0\xb0\x8b\xe4\xd5\x75\x65\xe1\xa8\x5e\xff\x28"
                            "\x3e\xe2\x3d\x95\x47\x43\xa2\xe3",
         32);
  syscall(__NR_ioctl, r[0], 0x89f1, 0x20000240);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
