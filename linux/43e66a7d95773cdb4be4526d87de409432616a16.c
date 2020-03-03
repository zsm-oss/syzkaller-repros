// general protection fault in sco_sock_getsockopt
// https://syzkaller.appspot.com/bug?id=43e66a7d95773cdb4be4526d87de409432616a16
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

const int kInitNetNsFd = 239;

static long syz_init_net_socket(volatile long domain, volatile long type,
                                volatile long proto)
{
  return syscall(__NR_socket, domain, type, proto);
}

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  intptr_t res = 0;
  res = syz_init_net_socket(0x1f, 5, 2);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0x26ul, 5ul, 0ul);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_fcntl, r[0], 0ul, r[1]);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x20000080 = 0;
  syscall(__NR_getsockopt, r[2], 0x84ul, 0xeul, 0ul, 0x20000080ul);
  return 0;
}
