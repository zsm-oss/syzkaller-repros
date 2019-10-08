// KASAN: use-after-free Write in padata_parallel_worker
// https://syzkaller.appspot.com/bug?id=81def6cbb5fbac9e86cb529131e43694bdaa7316
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
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x26, 5, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x200003c0 = 0x26;
  memcpy((void*)0x200003c2, "aead\000\000\000\000\000\000\000\000\000\000", 14);
  *(uint32_t*)0x200003d0 = 0;
  *(uint32_t*)0x200003d4 = 0;
  memcpy((void*)0x200003d8, "pcrypt(gcm(aes))"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000",
         64);
  syscall(__NR_bind, r[0], 0x200003c0, 0x58);
  res = syscall(__NR_socket, 0xa, 1, 0);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_socket, 0xa, 1, 0);
  if (res != -1)
    r[2] = res;
  *(uint16_t*)0x200000c0 = 0xa;
  *(uint16_t*)0x200000c2 = htobe16(0x4e22);
  *(uint32_t*)0x200000c4 = htobe32(0);
  memcpy((void*)0x200000c8,
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000",
         16);
  *(uint32_t*)0x200000d8 = 0;
  syscall(__NR_bind, r[2], 0x200000c0, 0x1c);
  syscall(__NR_listen, r[2], 0);
  *(uint16_t*)0x20000040 = 0xa;
  *(uint16_t*)0x20000042 = htobe16(0x4e22);
  *(uint32_t*)0x20000044 = htobe32(0);
  *(uint64_t*)0x20000048 = htobe64(0);
  *(uint64_t*)0x20000050 = htobe64(1);
  *(uint32_t*)0x20000058 = 0;
  syscall(__NR_sendto, r[1], 0, 0, 0x20004004, 0x20000040, 0x1c);
  memcpy((void*)0x20000000, "tls\000", 4);
  syscall(__NR_setsockopt, r[1], 6, 0x1f, 0x20000000, 0xc498ead121f97dd6);
  *(uint16_t*)0x20000140 = 0x303;
  *(uint16_t*)0x20000142 = 0x33;
  memcpy((void*)0x20000144, "\xd4\x4e\xb8\xc7\x30\x8e\xc7\xc4", 8);
  memcpy((void*)0x2000014c,
         "\x44\x20\x65\x23\x89\x29\x35\x0a\xde\x91\x90\x0b\x51\xfc\x95\x34",
         16);
  memcpy((void*)0x2000015c, "\x6b\xdd\xa7\x20", 4);
  memcpy((void*)0x20000160, "\x7e\xe5\x14\x30\xda\x3f\x51\xb3", 8);
  syscall(__NR_setsockopt, r[1], 0x11a, 1, 0x20000140, 0x28);
  syscall(__NR_sendto, r[1], 0x200005c0, 0xffffffffffffffc1, 0, 0,
          0x1201000000003618);
  return 0;
}
