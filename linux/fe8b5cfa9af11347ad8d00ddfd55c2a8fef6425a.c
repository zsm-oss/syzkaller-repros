// WARNING: kmalloc bug in cpu_map_update_elem
// https://syzkaller.appspot.com/bug?id=fe8b5cfa9af11347ad8d00ddfd55c2a8fef6425a
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  *(uint32_t*)0x204f9fe4 = 0x10;
  *(uint32_t*)0x204f9fe8 = 4;
  *(uint32_t*)0x204f9fec = 4;
  *(uint32_t*)0x204f9ff0 = 1;
  *(uint32_t*)0x204f9ff4 = 0;
  *(uint32_t*)0x204f9ff8 = -1;
  *(uint32_t*)0x204f9ffc = 0;
  *(uint8_t*)0x204fa000 = 0;
  *(uint8_t*)0x204fa001 = 0;
  *(uint8_t*)0x204fa002 = 0;
  *(uint8_t*)0x204fa003 = 0;
  *(uint8_t*)0x204fa004 = 0;
  *(uint8_t*)0x204fa005 = 0;
  *(uint8_t*)0x204fa006 = 0;
  *(uint8_t*)0x204fa007 = 0;
  *(uint8_t*)0x204fa008 = 0;
  *(uint8_t*)0x204fa009 = 0;
  *(uint8_t*)0x204fa00a = 0;
  *(uint8_t*)0x204fa00b = 0;
  *(uint8_t*)0x204fa00c = 0;
  *(uint8_t*)0x204fa00d = 0;
  *(uint8_t*)0x204fa00e = 0;
  *(uint8_t*)0x204fa00f = 0;
  r[0] = syscall(__NR_bpf, 0, 0x204f9fe4, 0x2c);
  *(uint32_t*)0x2029d000 = r[0];
  *(uint64_t*)0x2029d008 = 0x20aaa000;
  *(uint64_t*)0x2029d010 = 0x20d6b000;
  *(uint64_t*)0x2029d018 = 0;
  memcpy((void*)0x20d6b000, "\xf5", 1);
  syscall(__NR_bpf, 2, 0x2029d000, 0x20);
}

int main()
{
  loop();
  return 0;
}
