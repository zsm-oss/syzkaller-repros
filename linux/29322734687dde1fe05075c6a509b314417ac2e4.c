// KMSAN: uninit-value in nh_valid_get_del_req
// https://syzkaller.appspot.com/bug?id=29322734687dde1fe05075c6a509b314417ac2e4
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

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  intptr_t res = 0;
  *(uint64_t*)0x20000200 = 0;
  *(uint32_t*)0x20000208 = 0;
  *(uint64_t*)0x20000210 = 0x20000680;
  *(uint64_t*)0x20000680 = 0x20001e40;
  *(uint32_t*)0x20001e40 = 0x14;
  *(uint16_t*)0x20001e44 = 0x69;
  *(uint16_t*)0x20001e46 = 0x109;
  *(uint32_t*)0x20001e48 = 0;
  *(uint32_t*)0x20001e4c = 0;
  *(uint8_t*)0x20001e50 = 0;
  *(uint8_t*)0x20001e51 = 0;
  *(uint16_t*)0x20001e52 = 0;
  *(uint64_t*)0x20000688 = 0x14;
  *(uint64_t*)0x20000218 = 1;
  *(uint64_t*)0x20000220 = 0;
  *(uint64_t*)0x20000228 = 0;
  *(uint32_t*)0x20000230 = 0;
  syscall(__NR_sendmsg, -1, 0x20000200, 0);
  res = syscall(__NR_socket, 0x10, 0x80002, 0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000140 = 0;
  *(uint32_t*)0x20000148 = 0;
  *(uint64_t*)0x20000150 = 0x20000100;
  *(uint64_t*)0x20000158 = 0;
  *(uint64_t*)0x20000160 = 0x20000100;
  *(uint64_t*)0x20000168 = 0;
  *(uint32_t*)0x20000170 = 0;
  syscall(__NR_sendmmsg, r[0], 0x20000140, 0x492492492492805, 0);
  return 0;
}
