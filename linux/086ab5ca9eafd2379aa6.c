// KMSAN: uninit-value in xa_load
// https://syzkaller.appspot.com/bug?id=086ab5ca9eafd2379aa6
// status:6
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
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  *(uint32_t*)0x20000080 = 8;
  *(uint32_t*)0x20000084 = 0;
  *(uint32_t*)0x20000088 = 0;
  *(uint32_t*)0x2000008c = 0;
  *(uint32_t*)0x20000090 = 0;
  *(uint8_t*)0x20000094 = 0;
  *(uint8_t*)0x20000095 = 0;
  *(uint8_t*)0x20000096 = 0;
  *(uint8_t*)0x20000097 = 0;
  *(uint8_t*)0x20000098 = 0;
  *(uint8_t*)0x20000099 = 0;
  *(uint8_t*)0x2000009a = 0;
  *(uint8_t*)0x2000009b = 0;
  syscall(__NR_ioctl, -1, 0x401c5820, 0x20000080ul);
  memcpy((void*)0x20000000, "/dev/infiniband/rdma_cm\000", 24);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000000ul, 2ul, 0ul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x200000c0 = 0x20000080;
  *(uint64_t*)0x200000c8 = 0xfffffebe;
  syscall(__NR_writev, r[0], 0x200000c0ul, 1ul);
  return 0;
}
