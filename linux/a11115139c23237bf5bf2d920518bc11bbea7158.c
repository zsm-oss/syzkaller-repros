// WARNING: kmalloc bug in krealloc
// https://syzkaller.appspot.com/bug?id=a11115139c23237bf5bf2d920518bc11bbea7158
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_pipe2, 0x20000100, 0);
  if (res != -1) {
    r[0] = *(uint32_t*)0x20000100;
    r[1] = *(uint32_t*)0x20000104;
  }
  *(uint32_t*)0x20000180 = 0x62e9c926;
  *(uint8_t*)0x20000184 = 0x4d;
  *(uint16_t*)0x20000185 = 1;
  syscall(__NR_write, r[1], 0x20000180, 7);
  memcpy((void*)0x20000140, "./file0", 8);
  syscall(__NR_mkdir, 0x20000140, 0);
  memcpy((void*)0x200000c0, "./file0", 8);
  memcpy((void*)0x20000340, "9p", 3);
  memcpy((void*)0x200001c0, "trans=fd,", 9);
  memcpy((void*)0x200001c9, "rfdno", 5);
  *(uint8_t*)0x200001ce = 0x3d;
  sprintf((char*)0x200001cf, "0x%016llx", (long long)r[0]);
  *(uint8_t*)0x200001e1 = 0x2c;
  memcpy((void*)0x200001e2, "wfdno", 5);
  *(uint8_t*)0x200001e7 = 0x3d;
  sprintf((char*)0x200001e8, "0x%016llx", (long long)r[1]);
  *(uint8_t*)0x200001fa = 0x2c;
  *(uint8_t*)0x200001fb = 0;
  syscall(__NR_mount, 0, 0x200000c0, 0x20000340, 0, 0x200001c0);
  memcpy((void*)0x20000480, "\x2a\x00\x00\x00\x29\x01\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x07\x00\x2e\x2f\x66\x69\x6c\x65\x30"
                            "\x00\x00\x00\x00\x00\x00",
         42);
  syscall(__NR_write, r[1], 0x20000480, 0x2a);
  *(uint32_t*)0x20000200 = 0xa0;
  *(uint8_t*)0x20000204 = 0x19;
  *(uint16_t*)0x20000205 = 1;
  *(uint64_t*)0x20000207 = 0;
  *(uint8_t*)0x2000020f = 0;
  *(uint32_t*)0x20000210 = 0;
  *(uint64_t*)0x20000214 = 0;
  *(uint32_t*)0x2000021c = 0;
  *(uint32_t*)0x20000220 = 0;
  *(uint32_t*)0x20000224 = 0;
  *(uint64_t*)0x20000228 = 0xd4;
  *(uint64_t*)0x20000230 = 0;
  *(uint64_t*)0x20000238 = 0;
  *(uint64_t*)0x20000240 = 0x80;
  *(uint64_t*)0x20000248 = 2;
  *(uint64_t*)0x20000250 = 0x20;
  *(uint64_t*)0x20000258 = 0x10001;
  *(uint64_t*)0x20000260 = 0x10001;
  *(uint64_t*)0x20000268 = 0xfffffffffffffffe;
  *(uint64_t*)0x20000270 = 0;
  *(uint64_t*)0x20000278 = 0;
  *(uint64_t*)0x20000280 = 0;
  *(uint64_t*)0x20000288 = 0;
  *(uint64_t*)0x20000290 = 0;
  *(uint64_t*)0x20000298 = 0;
  syscall(__NR_write, r[1], 0x20000200, 0xa0);
  *(uint32_t*)0x200003c0 = 0x2a;
  *(uint8_t*)0x200003c4 = 0x29;
  *(uint16_t*)0x200003c5 = 1;
  *(uint32_t*)0x200003c7 = 0;
  *(uint8_t*)0x200003cb = 0;
  *(uint32_t*)0x200003cc = 0;
  *(uint64_t*)0x200003d0 = 0;
  *(uint64_t*)0x200003d8 = 0;
  *(uint8_t*)0x200003e0 = 0;
  *(uint16_t*)0x200003e1 = 7;
  memcpy((void*)0x200003e3, "./file0", 7);
  syscall(__NR_write, r[1], 0x200003c0, 0x2a);
  *(uint32_t*)0x20000040 = 0x5c;
  *(uint8_t*)0x20000044 = 0x7d;
  *(uint16_t*)0x20000045 = 1;
  *(uint16_t*)0x20000047 = 0;
  *(uint16_t*)0x20000049 = 0x55;
  *(uint16_t*)0x2000004b = 0x8001;
  *(uint32_t*)0x2000004d = 0xfffff20b;
  *(uint8_t*)0x20000051 = 0x80;
  *(uint32_t*)0x20000052 = 0;
  *(uint64_t*)0x20000056 = 6;
  *(uint32_t*)0x2000005e = 0x85a40000;
  *(uint32_t*)0x20000062 = 0;
  *(uint32_t*)0x20000066 = 1;
  *(uint64_t*)0x2000006a = 5;
  *(uint16_t*)0x20000072 = 0;
  *(uint16_t*)0x20000074 = 0;
  *(uint16_t*)0x20000076 = 3;
  memcpy((void*)0x20000078, "9p", 3);
  *(uint16_t*)0x2000007b = 0x1f;
  memcpy((void*)0x2000007d, "usermd5sum{posix_acl_accesslo#&", 31);
  syscall(__NR_write, r[1], 0x20000040, 0x5c);
  memcpy((void*)0x20000000, "./file0", 8);
  memcpy((void*)0x20000900, "9p", 3);
  memcpy((void*)0x20000400, "trans=fd,", 9);
  memcpy((void*)0x20000409, "rfdno", 5);
  *(uint8_t*)0x2000040e = 0x3d;
  sprintf((char*)0x2000040f, "0x%016llx", (long long)r[0]);
  *(uint8_t*)0x20000421 = 0x2c;
  memcpy((void*)0x20000422, "wfdno", 5);
  *(uint8_t*)0x20000427 = 0x3d;
  sprintf((char*)0x20000428, "0x%016llx", (long long)r[1]);
  *(uint8_t*)0x2000043a = 0x2c;
  *(uint8_t*)0x2000043b = 0;
  syscall(__NR_mount, 0, 0x20000000, 0x20000900, 0, 0x20000400);
  memcpy((void*)0x200004c0, "./file0", 8);
  memcpy((void*)0x20000500, "security.capability", 20);
  syscall(__NR_getxattr, 0x200004c0, 0x20000500, 0x200002c0, 0x143);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
