// WARNING: possible circular locking dependency detected (4)
// https://syzkaller.appspot.com/bug?id=77d9a62c6781d69dd833adc06ead030abc367218
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 0xa, 1, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_mmap, 0x20ab7000, 0x4000, 8, 0x12, r[0], 0);
  res = syscall(__NR_socket, 0xa, 0x80000000000001, 0x84);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20444ff8 = 0;
  *(uint32_t*)0x20444ffc = 7;
  syscall(__NR_setsockopt, r[1], 0x84, 0x76, 0x20444ff8, 8);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
