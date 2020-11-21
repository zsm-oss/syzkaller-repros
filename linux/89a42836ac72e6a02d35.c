// KASAN: slab-out-of-bounds Read in kvm_vcpu_gfn_to_memslot
// https://syzkaller.appspot.com/bug?id=89a42836ac72e6a02d35
// status:3
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
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  memcpy((void*)0x200000c0, "/dev/kvm\000", 9);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x200000c0ul, 0x100ul, 0ul);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_ioctl, r[0], 0xae01, 0ul);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000740 = 3;
  *(uint32_t*)0x20000744 = 2;
  *(uint64_t*)0x20000748 = 0xf000;
  *(uint64_t*)0x20000750 = 0x1000;
  *(uint64_t*)0x20000758 = 0x20000000;
  syscall(__NR_ioctl, r[1], 0x4020ae46, 0x20000740ul);
  res = syscall(__NR_ioctl, r[1], 0xae41, 0ul);
  if (res != -1)
    r[2] = res;
  syscall(__NR_ioctl, r[2], 0xae80, 0ul);
  syscall(__NR_ioctl, r[2], 0xae80, 0ul);
  return 0;
}
