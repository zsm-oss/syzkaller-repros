// kernel BUG at include/linux/mm.h:LINE!
// https://syzkaller.appspot.com/bug?id=3288f5ad69550f38978ffb30994d3cdf6752cb2f
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

#ifndef __NR_pkey_mprotect
#define __NR_pkey_mprotect 329
#endif

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  res = syscall(__NR_socket, 0x2b, 1, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000000 = 2;
  *(uint16_t*)0x20000002 = htobe16(0x4e23);
  *(uint32_t*)0x20000004 = htobe32(-1);
  syscall(__NR_bind, r[0], 0x20000000, 0x10);
  syscall(__NR_pkey_mprotect, 0x20014000, 0x3000, 0, -1);
  *(uint16_t*)0x20000300 = 2;
  *(uint16_t*)0x20000302 = htobe16(0x4e23);
  *(uint32_t*)0x20000304 = htobe32(0x7f000001);
  syscall(__NR_connect, r[0], 0x20000300, 0x10);
  memcpy((void*)0x20000340, "tls\000", 4);
  syscall(__NR_setsockopt, r[0], 6, 0x1f, 0x20000340, 4);
  *(uint32_t*)0x20000140 = htobe32(0x3033300);
  *(uint32_t*)0x20000144 = htobe32(0xe000000a);
  *(uint32_t*)0x20000148 = htobe32(0x7f000001);
  syscall(__NR_setsockopt, r[0], 0x11a, 1, 0x20000140, 0x28);
  *(uint8_t*)0x20000080 = 4;
  syscall(__NR_write, r[0], 0x20000080, 0x20000081);
  return 0;
}
