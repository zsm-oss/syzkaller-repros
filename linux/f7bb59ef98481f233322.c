// UBSAN: undefined-behaviour in snd_pcm_oss_change_params_locked
// https://syzkaller.appspot.com/bug?id=f7bb59ef98481f233322
// status:0
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
  memcpy((void*)0x20000100, "/dev/adsp1\000", 11);
  res =
      syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000100ul, 0x80002ul, 0ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x200000c0 = 0;
  syscall(__NR_ioctl, r[0], 0xc0045002, 0x200000c0ul);
  *(uint32_t*)0x20000000 = 0x27;
  syscall(__NR_ioctl, r[0], 0xc0045006, 0x20000000ul);
  return 0;
}
