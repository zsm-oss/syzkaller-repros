// WARNING: suspicious RCU usage in xfrm_get_sadinfo
// https://syzkaller.appspot.com/bug?id=106319f5d94ac049166744eee79e455ce4d0435c
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
  long res = 0;
  res = syscall(__NR_socket, 0x10, 3, 6);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000000 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint64_t*)0x20000010 = 0x20001400;
  *(uint64_t*)0x20001400 = 0x20001440;
  *(uint32_t*)0x20001440 = 0x14;
  *(uint16_t*)0x20001444 = 0x23;
  *(uint16_t*)0x20001446 = 0xbc51;
  *(uint32_t*)0x20001448 = 0;
  *(uint32_t*)0x2000144c = 0;
  *(uint32_t*)0x20001450 = 0;
  *(uint64_t*)0x20001408 = 0x14;
  *(uint64_t*)0x20000018 = 1;
  *(uint64_t*)0x20000020 = 0;
  *(uint64_t*)0x20000028 = 0;
  *(uint32_t*)0x20000030 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000000, 0);
  return 0;
}
