// suspicious RCU usage at ./include/trace/events/kmem.h:LINE
// https://syzkaller.appspot.com/bug?id=e886f5c1f6419c3ab8d46abb6257f016982294cd
// status:dup
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

long r[27];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xf78000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socket, 0x26ul, 0x5ul, 0x0ul);
  *(uint16_t*)0x20f72fa8 = (uint16_t)0x26;
  memcpy((void*)0x20f72faa,
         "\x73\x6b\x63\x69\x70\x68\x65\x72\x00\x00\x00\x00\x00\x00",
         14);
  *(uint32_t*)0x20f72fb8 = (uint32_t)0x0;
  *(uint32_t*)0x20f72fbc = (uint32_t)0x0;
  memcpy((void*)0x20f72fc0,
         "\x73\x61\x6c\x73\x61\x32\x30\x2d\x67\x65\x6e\x65\x72\x69\x63"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00",
         64);
  r[7] = syscall(__NR_bind, r[1], 0x20f72fa8ul, 0x58ul);
  memcpy((void*)0x20f76fef, "\x0a\x47\x75\xb1\xd5\xca\x84\xe5\xb3\xb6"
                            "\x0c\xed\x5c\x54\xdb\xb7\x51",
         17);
  r[9] = syscall(__NR_setsockopt, r[1], 0x117ul, 0x1ul, 0x20f76feful,
                 0x11ul);
  r[10] = syscall(__NR_accept, r[1], 0x0ul, 0x0ul);
  *(uint64_t*)0x20d63fc8 = (uint64_t)0x20f75000;
  *(uint32_t*)0x20d63fd0 = (uint32_t)0x10;
  *(uint64_t*)0x20d63fd8 = (uint64_t)0x208b4fb0;
  *(uint64_t*)0x20d63fe0 = (uint64_t)0x4;
  *(uint64_t*)0x20d63fe8 = (uint64_t)0x204f4000;
  *(uint64_t*)0x20d63ff0 = (uint64_t)0x0;
  *(uint32_t*)0x20d63ff8 = (uint32_t)0x10200;
  *(uint64_t*)0x208b4fb0 = (uint64_t)0x20f77fb2;
  *(uint64_t*)0x208b4fb8 = (uint64_t)0x0;
  *(uint64_t*)0x208b4fc0 = (uint64_t)0x206c5000;
  *(uint64_t*)0x208b4fc8 = (uint64_t)0x0;
  *(uint64_t*)0x208b4fd0 = (uint64_t)0x20f75fc9;
  *(uint64_t*)0x208b4fd8 = (uint64_t)0x0;
  *(uint64_t*)0x208b4fe0 = (uint64_t)0x20f75000;
  *(uint64_t*)0x208b4fe8 = (uint64_t)0x19;
  r[26] = syscall(__NR_recvmsg, r[10], 0x20d63fc8ul, 0x0ul);
}

int main()
{
  loop();
  return 0;
}
