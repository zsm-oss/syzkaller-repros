// general protection fault in ucma_set_ib_path
// https://syzkaller.appspot.com/bug?id=36e9a4af8d7c0e2a49f9c4edf51f8833cd9f683c
// status:dup
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[2] = {0xffffffffffffffff, 0xffffffff};
void loop()
{
  long res;
  memcpy((void*)0x200001c0, "/dev/infiniband/rdma_cm", 24);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x200001c0, 2, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20001300 = 0;
  *(uint16_t*)0x20001304 = 0x18;
  *(uint16_t*)0x20001306 = 0xfa00;
  *(uint64_t*)0x20001308 = 0;
  *(uint64_t*)0x20001310 = 0x200012c0;
  *(uint16_t*)0x20001318 = 0x106;
  *(uint8_t*)0x2000131a = 0;
  *(uint8_t*)0x2000131b = 0;
  *(uint8_t*)0x2000131c = 0;
  *(uint8_t*)0x2000131d = 0;
  *(uint8_t*)0x2000131e = 0;
  *(uint8_t*)0x2000131f = 0;
  res = syscall(__NR_write, r[0], 0x20001300, 0x20);
  if (res != -1)
    r[1] = *(uint32_t*)0x200012c0;
  *(uint32_t*)0x20001340 = 0xe;
  *(uint16_t*)0x20001344 = 0x18;
  *(uint16_t*)0x20001346 = 0xfa00;
  *(uint64_t*)0x20001348 = 0x20000b00;
  *(uint32_t*)0x20000b00 = 0x2b;
  *(uint32_t*)0x20000b04 = 0;
  *(uint32_t*)0x20000b08 = 0;
  *(uint32_t*)0x20000b0c = 0;
  *(uint32_t*)0x20000b10 = 0;
  *(uint32_t*)0x20000b14 = 0;
  *(uint32_t*)0x20000b18 = 0;
  *(uint32_t*)0x20000b1c = 0;
  *(uint32_t*)0x20000b20 = 0;
  *(uint32_t*)0x20000b24 = 0;
  *(uint32_t*)0x20000b28 = 0;
  *(uint32_t*)0x20000b2c = 0;
  *(uint32_t*)0x20000b30 = 0;
  *(uint32_t*)0x20000b34 = 0;
  *(uint32_t*)0x20000b38 = 0;
  *(uint32_t*)0x20000b3c = 0;
  *(uint32_t*)0x20000b40 = 0;
  *(uint32_t*)0x20000b44 = 0;
  *(uint32_t*)0x20001350 = r[1];
  *(uint32_t*)0x20001354 = 1;
  *(uint32_t*)0x20001358 = 1;
  *(uint32_t*)0x2000135c = 0x48;
  syscall(__NR_write, r[0], 0x20001340, 0x34d);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
