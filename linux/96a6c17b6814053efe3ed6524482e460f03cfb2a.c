// general protection fault in tcf_ife_init
// https://syzkaller.appspot.com/bug?id=96a6c17b6814053efe3ed6524482e460f03cfb2a
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
  res = syscall(__NR_socket, 0x10, 3, 0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000180 = 0;
  *(uint32_t*)0x20000188 = 0;
  *(uint64_t*)0x20000190 = 0x20000140;
  *(uint64_t*)0x20000140 = 0x20000200;
  memcpy((void*)0x20000200, "\x28\x00\x00\x00\x30\x00\x01\x00\x00\x00\x00\x00"
                            "\x66\xf3\x06\x9a\x08\xf9\xe3\xab\x14\x00\x01\x00"
                            "\x10\x00\x01\x00\x08\x00\x01\x00\x69\x66\x65\x00"
                            "\xc3\x00\x00\x00",
         40);
  *(uint64_t*)0x20000148 = 0x28;
  *(uint64_t*)0x20000198 = 1;
  *(uint64_t*)0x200001a0 = 0;
  *(uint64_t*)0x200001a8 = 0;
  *(uint32_t*)0x200001b0 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000180, 0);
  return 0;
}
