// KASAN: stack-out-of-bounds Read in xfrm_state_find (5)
// https://syzkaller.appspot.com/bug?id=44fa54548362cb84e26da7c1bbd356c86c54f36d
// status:open
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
  res = syscall(__NR_socket, 0xa, 0x2100000000000002, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000000 = 0xa;
  *(uint16_t*)0x20000002 = htobe16(0x4e23);
  *(uint32_t*)0x20000004 = 0;
  *(uint8_t*)0x20000008 = 0;
  *(uint8_t*)0x20000009 = 0;
  *(uint8_t*)0x2000000a = 0;
  *(uint8_t*)0x2000000b = 0;
  *(uint8_t*)0x2000000c = 0;
  *(uint8_t*)0x2000000d = 0;
  *(uint8_t*)0x2000000e = 0;
  *(uint8_t*)0x2000000f = 0;
  *(uint8_t*)0x20000010 = 0;
  *(uint8_t*)0x20000011 = 0;
  *(uint8_t*)0x20000012 = -1;
  *(uint8_t*)0x20000013 = -1;
  *(uint32_t*)0x20000014 = htobe32(0x7f000001);
  *(uint32_t*)0x20000018 = 0;
  syscall(__NR_connect, r[0], 0x20000000, 0x1c);
  *(uint8_t*)0x20000080 = 0xfe;
  *(uint8_t*)0x20000081 = 0x80;
  *(uint8_t*)0x20000082 = 0;
  *(uint8_t*)0x20000083 = 0;
  *(uint8_t*)0x20000084 = 0;
  *(uint8_t*)0x20000085 = 0;
  *(uint8_t*)0x20000086 = 0;
  *(uint8_t*)0x20000087 = 0;
  *(uint8_t*)0x20000088 = 0;
  *(uint8_t*)0x20000089 = 0;
  *(uint8_t*)0x2000008a = 0;
  *(uint8_t*)0x2000008b = 0;
  *(uint8_t*)0x2000008c = 0;
  *(uint8_t*)0x2000008d = 0;
  *(uint8_t*)0x2000008e = 0;
  *(uint8_t*)0x2000008f = 0xbb;
  *(uint64_t*)0x20000090 = htobe64(0);
  *(uint64_t*)0x20000098 = htobe64(1);
  *(uint16_t*)0x200000a0 = htobe16(0);
  *(uint16_t*)0x200000a2 = htobe16(0);
  *(uint16_t*)0x200000a4 = htobe16(0);
  *(uint16_t*)0x200000a6 = htobe16(0);
  *(uint16_t*)0x200000a8 = 2;
  *(uint8_t*)0x200000aa = 0;
  *(uint8_t*)0x200000ab = 0;
  *(uint8_t*)0x200000ac = 0;
  *(uint32_t*)0x200000b0 = 0;
  *(uint32_t*)0x200000b4 = 0;
  *(uint64_t*)0x200000b8 = 0;
  *(uint64_t*)0x200000c0 = 0;
  *(uint64_t*)0x200000c8 = 0;
  *(uint64_t*)0x200000d0 = 0;
  *(uint64_t*)0x200000d8 = 0;
  *(uint64_t*)0x200000e0 = 0;
  *(uint64_t*)0x200000e8 = 0;
  *(uint64_t*)0x200000f0 = 0;
  *(uint64_t*)0x200000f8 = 0;
  *(uint64_t*)0x20000100 = 0;
  *(uint64_t*)0x20000108 = 0;
  *(uint64_t*)0x20000110 = 0;
  *(uint32_t*)0x20000118 = 0;
  *(uint32_t*)0x2000011c = 0xfffffffd;
  *(uint8_t*)0x20000120 = 1;
  *(uint8_t*)0x20000121 = 0;
  *(uint8_t*)0x20000122 = 0;
  *(uint8_t*)0x20000123 = 0;
  *(uint8_t*)0x20000128 = 0xac;
  *(uint8_t*)0x20000129 = 0x14;
  *(uint8_t*)0x2000012a = 0x14;
  *(uint8_t*)0x2000012b = 0;
  *(uint32_t*)0x20000138 = htobe32(0);
  *(uint8_t*)0x2000013c = 0x2b;
  *(uint16_t*)0x20000140 = 0xa;
  *(uint8_t*)0x20000144 = 0;
  *(uint8_t*)0x20000145 = 0;
  *(uint8_t*)0x20000146 = 0;
  *(uint8_t*)0x20000147 = 0;
  *(uint8_t*)0x20000148 = 0;
  *(uint8_t*)0x20000149 = 0;
  *(uint8_t*)0x2000014a = 0;
  *(uint8_t*)0x2000014b = 0;
  *(uint8_t*)0x2000014c = 0;
  *(uint8_t*)0x2000014d = 0;
  *(uint8_t*)0x2000014e = -1;
  *(uint8_t*)0x2000014f = -1;
  *(uint32_t*)0x20000150 = htobe32(-1);
  *(uint32_t*)0x20000154 = 0;
  *(uint8_t*)0x20000158 = 7;
  *(uint8_t*)0x20000159 = 0;
  *(uint8_t*)0x2000015a = 2;
  *(uint32_t*)0x2000015c = 0;
  *(uint32_t*)0x20000160 = 0;
  *(uint32_t*)0x20000164 = 0;
  syscall(__NR_setsockopt, r[0], 0x29, 0x23, 0x20000080, 0xe8);
  *(uint64_t*)0x20000580 = 0;
  *(uint32_t*)0x20000588 = 0;
  *(uint64_t*)0x20000590 = 0x200026c0;
  *(uint64_t*)0x20000598 = 0;
  *(uint64_t*)0x200005a0 = 0x20000040;
  *(uint64_t*)0x200005a8 = 0;
  *(uint32_t*)0x200005b0 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000580, 0);
  *(uint8_t*)0x200c5fe8 = 0xaa;
  *(uint8_t*)0x200c5fe9 = 0xaa;
  *(uint8_t*)0x200c5fea = 0xaa;
  *(uint8_t*)0x200c5feb = 0xaa;
  *(uint8_t*)0x200c5fec = 0;
  *(uint8_t*)0x200c5fed = 0;
  *(uint8_t*)0x200c5fee = -1;
  *(uint8_t*)0x200c5fef = -1;
  *(uint8_t*)0x200c5ff0 = -1;
  *(uint8_t*)0x200c5ff1 = -1;
  *(uint8_t*)0x200c5ff2 = -1;
  *(uint8_t*)0x200c5ff3 = -1;
  *(uint16_t*)0x200c5ff4 = htobe16(0x8863);
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0xfffffffc;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0x4000;
  *(uint32_t*)0x20000010 = 0;
  *(uint32_t*)0x20000014 = 0;
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
