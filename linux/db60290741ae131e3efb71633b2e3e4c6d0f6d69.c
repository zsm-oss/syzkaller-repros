// INFO: trying to register non-static key. (2)
// https://syzkaller.appspot.com/bug?id=db60290741ae131e3efb71633b2e3e4c6d0f6d69
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

long r[4];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socket, 0xaul, 0x1ul, 0x0ul);
  memcpy((void*)0x20f2b000, "\x2b\xe6\x00\xe5\xe0\xff\xff\x01\x00\x00"
                            "\x92\xc5\x00\x1d\x8d\x9a",
         16);
  r[3] = syscall(__NR_setsockopt, r[1], 0x6ul, 0x21ul, 0x20f2b000ul,
                 0x10ul);
}

int main()
{
  loop();
  return 0;
}
