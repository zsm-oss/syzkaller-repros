// KASAN: slab-out-of-bounds Read in crypto_morus640_decrypt_chunk
// https://syzkaller.appspot.com/bug?id=b2c701d1581d38ec195c40db364d8d6c9ff9e645
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 0x26, 5, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000140 = 0x26;
  memcpy((void*)0x20000142,
         "\x61\x65\x61\x64\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 14);
  *(uint32_t*)0x20000150 = 0;
  *(uint32_t*)0x20000154 = 0;
  memcpy((void*)0x20000158,
         "\x6d\x6f\x72\x75\x73\x36\x34\x30\x2d\x67\x65\x6e\x65\x72\x69\x63\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  syscall(__NR_bind, r[0], 0x20000140, 0x58);
  memcpy((void*)0x20000080,
         "\xb7\xf2\x28\x8a\x91\x19\x93\xf0\x26\x5d\xf5\xcf\x1c\xdd\x8b\x55",
         16);
  syscall(__NR_setsockopt, r[0], 0x117, 1, 0x20000080, 0x10);
  res = syscall(__NR_accept, r[0], 0, 0);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x200003c0 = -1;
  *(uint64_t*)0x200003c2 = 0;
  *(uint32_t*)0x200003ca = -1;
  *(uint16_t*)0x200003ce = -1;
  *(uint64_t*)0x200003d0 = 0x20000340;
  *(uint16_t*)0x20000340 = -1;
  *(uint32_t*)0x20000342 = -1;
  syscall(__NR_write, r[1], 0x200003c0, 0x18);
  *(uint64_t*)0x20002840 = 0x200000c0;
  *(uint32_t*)0x20002848 = 0x80;
  *(uint64_t*)0x20002850 = 0x20002700;
  *(uint64_t*)0x20002700 = 0x20001380;
  *(uint64_t*)0x20002708 = 5;
  *(uint64_t*)0x20002710 = 0x20002600;
  *(uint64_t*)0x20002718 = 0xf8;
  *(uint64_t*)0x20002858 = 2;
  *(uint64_t*)0x20002860 = 0x20002780;
  *(uint64_t*)0x20002868 = 0xae;
  *(uint32_t*)0x20002870 = 0;
  syscall(__NR_recvmsg, r[1], 0x20002840, 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
