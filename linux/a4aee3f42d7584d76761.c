// WARNING in hwsim_new_radio_nl
// https://syzkaller.appspot.com/bug?id=a4aee3f42d7584d76761
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
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 2ul, 0x10);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000000 = 0;
  *(uint32_t*)0x20000008 = 0xfffffed3;
  *(uint64_t*)0x20000010 = 0x20000080;
  *(uint64_t*)0x20000080 = 0x20000040;
  memcpy((void*)0x20000040, "\x2e\x02\x02\x00\x25\x00\x05\x35\xd2\x5a\x80\x64"
                            "\x8c\x63\x94\x0d\x04\x24\xfc\x60\x00\x81\x11\x40"
                            "\x0c\x00\x00\x00\x05\x1a\x82\xc1\x37\x15\x3e\x67"
                            "\x09\x02\x01\x80\x03\x00\x17\x00\xd1\xbd",
         46);
  *(uint64_t*)0x20000088 = 0x33fe0;
  *(uint64_t*)0x20000018 = 1;
  *(uint64_t*)0x20000020 = 0;
  *(uint64_t*)0x20000028 = 0;
  *(uint32_t*)0x20000030 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000000ul, 0ul);
  return 0;
}
