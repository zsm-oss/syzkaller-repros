// general protection fault in smc_getsockopt
// https://syzkaller.appspot.com/bug?id=6e5e224656a49e9167742ed797391bad751c812a
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 0x2b, 1, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_listen, r[0], 2);
  syscall(__NR_shutdown, r[0], 2);
  *(uint32_t*)0x20000040 = 0xc;
  syscall(__NR_getsockopt, r[0], 0, 8, 0x20000000, 0x20000040);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
