// BUG: using __this_cpu_read() in preemptible [ADDR] code: syzkaller6NUM/3352
// https://syzkaller.appspot.com/bug?id=af45eb40590a1b6c275960d1b20f9ddf0d43d6f6
// status:invalid
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

static void test();

void loop()
{
  while (1) {
    test();
  }
}

#ifndef __NR_socket
#define __NR_socket 359
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[67];
void test()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socket, 0xful, 0x3ul, 0x2ul);
  *(uint32_t*)0x208befc8 = (uint32_t)0x0;
  *(uint32_t*)0x208befcc = (uint32_t)0x0;
  *(uint32_t*)0x208befd0 = (uint32_t)0x208feff0;
  *(uint32_t*)0x208befd4 = (uint32_t)0x1;
  *(uint32_t*)0x208befd8 = (uint32_t)0x0;
  *(uint32_t*)0x208befdc = (uint32_t)0x0;
  *(uint32_t*)0x208befe0 = (uint32_t)0x0;
  *(uint32_t*)0x208feff0 = (uint32_t)0x20333f88;
  *(uint32_t*)0x208feff4 = (uint32_t)0x60;
  *(uint8_t*)0x20333f88 = (uint8_t)0x2;
  *(uint8_t*)0x20333f89 = (uint8_t)0x3;
  *(uint8_t*)0x20333f8a = (uint8_t)0x0;
  *(uint8_t*)0x20333f8b = (uint8_t)0x9;
  *(uint16_t*)0x20333f8c = (uint16_t)0xc;
  *(uint16_t*)0x20333f8e = (uint16_t)0x0;
  *(uint32_t*)0x20333f90 = (uint32_t)0x0;
  *(uint32_t*)0x20333f94 = (uint32_t)0x0;
  *(uint16_t*)0x20333f98 = (uint16_t)0x2;
  *(uint16_t*)0x20333f9a = (uint16_t)0x13;
  *(uint8_t*)0x20333f9c = (uint8_t)0x2;
  *(uint8_t*)0x20333f9d = (uint8_t)0x0;
  *(uint16_t*)0x20333f9e = (uint16_t)0x0;
  *(uint32_t*)0x20333fa0 = (uint32_t)0x0;
  *(uint32_t*)0x20333fa4 = (uint32_t)0x0;
  *(uint16_t*)0x20333fa8 = (uint16_t)0x3;
  *(uint16_t*)0x20333faa = (uint16_t)0x6;
  *(uint8_t*)0x20333fac = (uint8_t)0x0;
  *(uint8_t*)0x20333fad = (uint8_t)0x0;
  *(uint16_t*)0x20333fae = (uint16_t)0x0;
  *(uint16_t*)0x20333fb0 = (uint16_t)0x2;
  *(uint16_t*)0x20333fb2 = (uint16_t)0x204e;
  *(uint32_t*)0x20333fb4 = (uint32_t)0x10000e0;
  *(uint8_t*)0x20333fb8 = (uint8_t)0x0;
  *(uint8_t*)0x20333fb9 = (uint8_t)0x0;
  *(uint8_t*)0x20333fba = (uint8_t)0x0;
  *(uint8_t*)0x20333fbb = (uint8_t)0x0;
  *(uint8_t*)0x20333fbc = (uint8_t)0x0;
  *(uint8_t*)0x20333fbd = (uint8_t)0x0;
  *(uint8_t*)0x20333fbe = (uint8_t)0x0;
  *(uint8_t*)0x20333fbf = (uint8_t)0x0;
  *(uint16_t*)0x20333fc0 = (uint16_t)0x2;
  *(uint16_t*)0x20333fc2 = (uint16_t)0x1;
  *(uint32_t*)0x20333fc4 = (uint32_t)0x0;
  *(uint8_t*)0x20333fc8 = (uint8_t)0x0;
  *(uint8_t*)0x20333fc9 = (uint8_t)0x0;
  *(uint8_t*)0x20333fca = (uint8_t)0x0;
  *(uint8_t*)0x20333fcb = (uint8_t)0x2;
  *(uint32_t*)0x20333fcc = (uint32_t)0x0;
  *(uint16_t*)0x20333fd0 = (uint16_t)0x3;
  *(uint16_t*)0x20333fd2 = (uint16_t)0x5;
  *(uint8_t*)0x20333fd4 = (uint8_t)0x0;
  *(uint8_t*)0x20333fd5 = (uint8_t)0x0;
  *(uint16_t*)0x20333fd6 = (uint16_t)0x0;
  *(uint16_t*)0x20333fd8 = (uint16_t)0x2;
  *(uint16_t*)0x20333fda = (uint16_t)0x204e;
  *(uint32_t*)0x20333fdc = (uint32_t)0x0;
  *(uint8_t*)0x20333fe0 = (uint8_t)0x0;
  *(uint8_t*)0x20333fe1 = (uint8_t)0x0;
  *(uint8_t*)0x20333fe2 = (uint8_t)0x0;
  *(uint8_t*)0x20333fe3 = (uint8_t)0x0;
  *(uint8_t*)0x20333fe4 = (uint8_t)0x0;
  *(uint8_t*)0x20333fe5 = (uint8_t)0x0;
  *(uint8_t*)0x20333fe6 = (uint8_t)0x0;
  *(uint8_t*)0x20333fe7 = (uint8_t)0x0;
  r[66] = syscall(__NR_sendmsg, r[1], 0x208befc8ul, 0x0ul);
}

int main()
{
  loop();
  return 0;
}
