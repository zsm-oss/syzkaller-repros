// general protection fault in icmp_timeout_obj_to_nlattr
// https://syzkaller.appspot.com/bug?id=abad7e09f759ea5326f8941f38e2fea72b395187
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
  res = syscall(__NR_socket, 0x10, 3, 0xc);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20dddfc8 = 0;
  *(uint32_t*)0x20dddfd0 = 0;
  *(uint64_t*)0x20dddfd8 = 0x208a7000;
  *(uint64_t*)0x208a7000 = 0x20000280;
  memcpy((void*)0x20000280, "\x2c\x00\x00\x00\x04\x08\x05\x01\xff\x00\x80\xff"
                            "\xfd\xff\xff\x01\x0a\x00\x00\x00\x0c\x00\x03\x00"
                            "\x01\x00\x00\x00\x7d\x0a\x00\x01\x0c\x00\x02\x00"
                            "\x00\x02\xfa\x17\x71\x11\x04\xa6",
         44);
  *(uint64_t*)0x208a7008 = 0x2c;
  *(uint64_t*)0x20dddfe0 = 1;
  *(uint64_t*)0x20dddfe8 = 0;
  *(uint64_t*)0x20dddff0 = 0;
  *(uint32_t*)0x20dddff8 = 0;
  syscall(__NR_sendmsg, r[0], 0x20dddfc8, 0);
  return 0;
}
