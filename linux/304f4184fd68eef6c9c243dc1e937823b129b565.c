// KMSAN: uninit-value in gcmaes_decrypt
// https://syzkaller.appspot.com/bug?id=304f4184fd68eef6c9c243dc1e937823b129b565
// status:invalid
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
  res = syscall(__NR_socket, 0x26, 5, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000080 = 0x26;
  memcpy((void*)0x20000082,
         "\x61\x65\x61\x64\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 14);
  *(uint32_t*)0x20000090 = 0;
  *(uint32_t*)0x20000094 = 0;
  memcpy((void*)0x20000098,
         "\x67\x65\x6e\x65\x72\x69\x63\x2d\x67\x63\x6d\x2d\x61\x65\x73\x6e\x69"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  syscall(__NR_bind, r[0], 0x20000080, 0x58);
  res = syscall(__NR_accept, r[0], 0, 0);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000040, "\xd3\xab\x27\x19\x1a\x01\x00\x23\x56\xba\x60\x2d"
                            "\xff\x05\x00\x22\xfe\xf9\xb9\xaf\xa4\xb2\x00\x79",
         24);
  syscall(__NR_setsockopt, r[0], 0x117, 1, 0x20000040, 0x18);
  *(uint64_t*)0x20002c80 = 0;
  *(uint32_t*)0x20002c88 = 0;
  *(uint64_t*)0x20002c90 = 0x200002c0;
  *(uint64_t*)0x200002c0 = 0x20000200;
  memcpy((void*)0x20000200,
         "\x6d\x7c\x3a\xea\x08\x07\x5f\xc6\x3b\x36\x3a\x30\xc4\x97\xc7\x21",
         16);
  *(uint64_t*)0x200002c8 = 0x10;
  *(uint64_t*)0x20002c98 = 1;
  *(uint64_t*)0x20002ca0 = 0x20000200;
  *(uint64_t*)0x20002ca8 = 0;
  *(uint32_t*)0x20002cb0 = 0;
  syscall(__NR_sendmmsg, r[1], 0x20002c80, 1, 0);
  *(uint64_t*)0x20000d40 = 0x20000180;
  *(uint32_t*)0x20000d48 = 0x80;
  *(uint64_t*)0x20000d50 = 0x20000c40;
  *(uint64_t*)0x20000c40 = 0x20000500;
  *(uint64_t*)0x20000c48 = 0x12;
  *(uint64_t*)0x20000d58 = 1;
  *(uint64_t*)0x20000d60 = 0x20000cc0;
  *(uint64_t*)0x20000d68 = 0x61;
  *(uint32_t*)0x20000d70 = 0;
  syscall(__NR_recvmsg, r[1], 0x20000d40, 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
