// BUG: unable to handle kernel paging request in smc_ib_remember_port_attr
// https://syzkaller.appspot.com/bug?id=4fe2536d0e707f5117e07ccc49befa896be2dde7
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
  long res;
  res = syscall(__NR_socket, 0x10, 3, 0x10);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20a87fc8 = 0x20fc6000;
  *(uint16_t*)0x20fc6000 = 0x10;
  *(uint16_t*)0x20fc6002 = 0x4000;
  *(uint32_t*)0x20fc6004 = 0;
  *(uint32_t*)0x20fc6008 = 0;
  *(uint32_t*)0x20a87fd0 = 0xc;
  *(uint64_t*)0x20a87fd8 = 0x20e1eff8;
  *(uint64_t*)0x20e1eff8 = 0x208ba000;
  *(uint32_t*)0x208ba000 = 0x1c;
  *(uint16_t*)0x208ba004 = 0x25;
  *(uint16_t*)0x208ba006 = 0x801;
  *(uint32_t*)0x208ba008 = 0;
  *(uint32_t*)0x208ba00c = 0;
  *(uint8_t*)0x208ba010 = 2;
  *(uint8_t*)0x208ba011 = 0;
  *(uint16_t*)0x208ba012 = 0;
  *(uint16_t*)0x208ba014 = 8;
  *(uint16_t*)0x208ba016 = 1;
  memcpy((void*)0x208ba018, "K", 1);
  *(uint64_t*)0x20e1f000 = 0x1c;
  *(uint64_t*)0x20a87fe0 = 1;
  *(uint64_t*)0x20a87fe8 = 0;
  *(uint64_t*)0x20a87ff0 = 0;
  *(uint32_t*)0x20a87ff8 = 0;
  syscall(__NR_sendmsg, r[0], 0x20a87fc8, 0x40004);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
