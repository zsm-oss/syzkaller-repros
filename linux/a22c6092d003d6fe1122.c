// KASAN: slab-out-of-bounds Read in vsscanf (2)
// https://syzkaller.appspot.com/bug?id=a22c6092d003d6fe1122
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
  memcpy((void*)0x20000180, "/sys/fs/smackfs/cipso2\000", 23);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000180ul, 2ul, 0ul);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000040, "mountstats!", 11);
  *(uint8_t*)0x2000004b = 0x20;
  sprintf((char*)0x2000004c, "%020llu", (long long)0x7c);
  *(uint8_t*)0x20000060 = 0x20;
  sprintf((char*)0x20000061, "%020llu", (long long)0);
  *(uint8_t*)0x20000075 = 0x20;
  *(uint8_t*)0x20000076 = 0;
  syscall(__NR_write, r[0], 0x20000040ul, 0x37ul);
  return 0;
}
