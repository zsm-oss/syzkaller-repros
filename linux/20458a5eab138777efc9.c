// KASAN: use-after-free Read in evdev_cleanup
// https://syzkaller.appspot.com/bug?id=20458a5eab138777efc9
// status:2
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000000, "/dev/uinput\000", 12);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000000ul, 0x802ul, 0ul);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000b40,
         "syz1\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000",
         80);
  *(uint16_t*)0x20000b90 = 0;
  *(uint16_t*)0x20000b92 = 0;
  *(uint16_t*)0x20000b94 = 0;
  *(uint16_t*)0x20000b96 = 0;
  *(uint32_t*)0x20000b98 = 0;
  *(uint32_t*)0x20000b9c = 0;
  *(uint32_t*)0x20000ba0 = 0;
  *(uint32_t*)0x20000ba4 = 0;
  *(uint32_t*)0x20000ba8 = 0;
  *(uint32_t*)0x20000bac = 0;
  *(uint32_t*)0x20000bb0 = 0;
  *(uint32_t*)0x20000bb4 = 0;
  *(uint32_t*)0x20000bb8 = 0;
  *(uint32_t*)0x20000bbc = 0;
  *(uint32_t*)0x20000bc0 = 0;
  *(uint32_t*)0x20000bc4 = 0;
  *(uint32_t*)0x20000bc8 = 0;
  *(uint32_t*)0x20000bcc = 0;
  *(uint32_t*)0x20000bd0 = 0;
  *(uint32_t*)0x20000bd4 = 0;
  *(uint32_t*)0x20000bd8 = 0;
  *(uint32_t*)0x20000bdc = 0;
  *(uint32_t*)0x20000be0 = 0;
  *(uint32_t*)0x20000be4 = 0;
  *(uint32_t*)0x20000be8 = 0;
  *(uint32_t*)0x20000bec = 0;
  *(uint32_t*)0x20000bf0 = 0;
  *(uint32_t*)0x20000bf4 = 0;
  *(uint32_t*)0x20000bf8 = 0;
  *(uint32_t*)0x20000bfc = 0;
  *(uint32_t*)0x20000c00 = 0;
  *(uint32_t*)0x20000c04 = 0;
  *(uint32_t*)0x20000c08 = 0;
  *(uint32_t*)0x20000c0c = 0;
  *(uint32_t*)0x20000c10 = 0;
  *(uint32_t*)0x20000c14 = 0;
  *(uint32_t*)0x20000c18 = 0;
  *(uint32_t*)0x20000c1c = 0;
  *(uint32_t*)0x20000c20 = 0;
  *(uint32_t*)0x20000c24 = 0;
  *(uint32_t*)0x20000c28 = 0;
  *(uint32_t*)0x20000c2c = 0;
  *(uint32_t*)0x20000c30 = 0;
  *(uint32_t*)0x20000c34 = 0;
  *(uint32_t*)0x20000c38 = 0;
  *(uint32_t*)0x20000c3c = 0;
  *(uint32_t*)0x20000c40 = 0;
  *(uint32_t*)0x20000c44 = 0;
  *(uint32_t*)0x20000c48 = 0;
  *(uint32_t*)0x20000c4c = 0;
  *(uint32_t*)0x20000c50 = 0;
  *(uint32_t*)0x20000c54 = 0;
  *(uint32_t*)0x20000c58 = 0;
  *(uint32_t*)0x20000c5c = 0;
  *(uint32_t*)0x20000c60 = 0;
  *(uint32_t*)0x20000c64 = 0;
  *(uint32_t*)0x20000c68 = 0;
  *(uint32_t*)0x20000c6c = 0;
  *(uint32_t*)0x20000c70 = 0;
  *(uint32_t*)0x20000c74 = 0;
  *(uint32_t*)0x20000c78 = 0;
  *(uint32_t*)0x20000c7c = 0;
  *(uint32_t*)0x20000c80 = 0;
  *(uint32_t*)0x20000c84 = 0;
  *(uint32_t*)0x20000c88 = 0;
  *(uint32_t*)0x20000c8c = 0;
  *(uint32_t*)0x20000c90 = 0;
  *(uint32_t*)0x20000c94 = 0;
  *(uint32_t*)0x20000c98 = 0;
  *(uint32_t*)0x20000c9c = 0;
  *(uint32_t*)0x20000ca0 = 0;
  *(uint32_t*)0x20000ca4 = 0;
  *(uint32_t*)0x20000ca8 = 0;
  *(uint32_t*)0x20000cac = 0;
  *(uint32_t*)0x20000cb0 = 0;
  *(uint32_t*)0x20000cb4 = 0;
  *(uint32_t*)0x20000cb8 = 0;
  *(uint32_t*)0x20000cbc = 0;
  *(uint32_t*)0x20000cc0 = 0;
  *(uint32_t*)0x20000cc4 = 0;
  *(uint32_t*)0x20000cc8 = 0;
  *(uint32_t*)0x20000ccc = 0;
  *(uint32_t*)0x20000cd0 = 0;
  *(uint32_t*)0x20000cd4 = 0;
  *(uint32_t*)0x20000cd8 = 0;
  *(uint32_t*)0x20000cdc = 0;
  *(uint32_t*)0x20000ce0 = 0;
  *(uint32_t*)0x20000ce4 = 0;
  *(uint32_t*)0x20000ce8 = 0;
  *(uint32_t*)0x20000cec = 0;
  *(uint32_t*)0x20000cf0 = 0;
  *(uint32_t*)0x20000cf4 = 0;
  *(uint32_t*)0x20000cf8 = 0;
  *(uint32_t*)0x20000cfc = 0;
  *(uint32_t*)0x20000d00 = 0;
  *(uint32_t*)0x20000d04 = 0;
  *(uint32_t*)0x20000d08 = 0;
  *(uint32_t*)0x20000d0c = 0;
  *(uint32_t*)0x20000d10 = 0;
  *(uint32_t*)0x20000d14 = 0;
  *(uint32_t*)0x20000d18 = 0;
  *(uint32_t*)0x20000d1c = 0;
  *(uint32_t*)0x20000d20 = 0;
  *(uint32_t*)0x20000d24 = 0;
  *(uint32_t*)0x20000d28 = 0;
  *(uint32_t*)0x20000d2c = 0;
  *(uint32_t*)0x20000d30 = 0;
  *(uint32_t*)0x20000d34 = 0;
  *(uint32_t*)0x20000d38 = 0;
  *(uint32_t*)0x20000d3c = 0;
  *(uint32_t*)0x20000d40 = 0;
  *(uint32_t*)0x20000d44 = 0;
  *(uint32_t*)0x20000d48 = 0;
  *(uint32_t*)0x20000d4c = 0;
  *(uint32_t*)0x20000d50 = 0;
  *(uint32_t*)0x20000d54 = 0;
  *(uint32_t*)0x20000d58 = 0;
  *(uint32_t*)0x20000d5c = 0;
  *(uint32_t*)0x20000d60 = 0;
  *(uint32_t*)0x20000d64 = 0;
  *(uint32_t*)0x20000d68 = 0;
  *(uint32_t*)0x20000d6c = 0;
  *(uint32_t*)0x20000d70 = 0;
  *(uint32_t*)0x20000d74 = 0;
  *(uint32_t*)0x20000d78 = 0;
  *(uint32_t*)0x20000d7c = 0;
  *(uint32_t*)0x20000d80 = 0;
  *(uint32_t*)0x20000d84 = 0;
  *(uint32_t*)0x20000d88 = 0;
  *(uint32_t*)0x20000d8c = 0;
  *(uint32_t*)0x20000d90 = 0;
  *(uint32_t*)0x20000d94 = 0;
  *(uint32_t*)0x20000d98 = 0;
  *(uint32_t*)0x20000d9c = 0;
  *(uint32_t*)0x20000da0 = 0;
  *(uint32_t*)0x20000da4 = 0;
  *(uint32_t*)0x20000da8 = 0;
  *(uint32_t*)0x20000dac = 0;
  *(uint32_t*)0x20000db0 = 0;
  *(uint32_t*)0x20000db4 = 0;
  *(uint32_t*)0x20000db8 = 0;
  *(uint32_t*)0x20000dbc = 0;
  *(uint32_t*)0x20000dc0 = 0;
  *(uint32_t*)0x20000dc4 = 0;
  *(uint32_t*)0x20000dc8 = 0;
  *(uint32_t*)0x20000dcc = 0;
  *(uint32_t*)0x20000dd0 = 0;
  *(uint32_t*)0x20000dd4 = 0;
  *(uint32_t*)0x20000dd8 = 0;
  *(uint32_t*)0x20000ddc = 0;
  *(uint32_t*)0x20000de0 = 0;
  *(uint32_t*)0x20000de4 = 0;
  *(uint32_t*)0x20000de8 = 0;
  *(uint32_t*)0x20000dec = 0;
  *(uint32_t*)0x20000df0 = 0;
  *(uint32_t*)0x20000df4 = 0;
  *(uint32_t*)0x20000df8 = 0;
  *(uint32_t*)0x20000dfc = 0;
  *(uint32_t*)0x20000e00 = 0;
  *(uint32_t*)0x20000e04 = 0;
  *(uint32_t*)0x20000e08 = 0;
  *(uint32_t*)0x20000e0c = 0;
  *(uint32_t*)0x20000e10 = 0;
  *(uint32_t*)0x20000e14 = 0;
  *(uint32_t*)0x20000e18 = 0;
  *(uint32_t*)0x20000e1c = 0;
  *(uint32_t*)0x20000e20 = 0;
  *(uint32_t*)0x20000e24 = 0;
  *(uint32_t*)0x20000e28 = 0;
  *(uint32_t*)0x20000e2c = 0;
  *(uint32_t*)0x20000e30 = 0;
  *(uint32_t*)0x20000e34 = 0;
  *(uint32_t*)0x20000e38 = 0;
  *(uint32_t*)0x20000e3c = 0;
  *(uint32_t*)0x20000e40 = 0;
  *(uint32_t*)0x20000e44 = 0;
  *(uint32_t*)0x20000e48 = 0;
  *(uint32_t*)0x20000e4c = 0;
  *(uint32_t*)0x20000e50 = 0;
  *(uint32_t*)0x20000e54 = 0;
  *(uint32_t*)0x20000e58 = 0;
  *(uint32_t*)0x20000e5c = 0;
  *(uint32_t*)0x20000e60 = 0;
  *(uint32_t*)0x20000e64 = 0;
  *(uint32_t*)0x20000e68 = 0;
  *(uint32_t*)0x20000e6c = 0;
  *(uint32_t*)0x20000e70 = 0;
  *(uint32_t*)0x20000e74 = 0;
  *(uint32_t*)0x20000e78 = 0;
  *(uint32_t*)0x20000e7c = 0;
  *(uint32_t*)0x20000e80 = 0;
  *(uint32_t*)0x20000e84 = 0;
  *(uint32_t*)0x20000e88 = 0;
  *(uint32_t*)0x20000e8c = 0;
  *(uint32_t*)0x20000e90 = 0;
  *(uint32_t*)0x20000e94 = 0;
  *(uint32_t*)0x20000e98 = 0;
  *(uint32_t*)0x20000e9c = 0;
  *(uint32_t*)0x20000ea0 = 0;
  *(uint32_t*)0x20000ea4 = 0;
  *(uint32_t*)0x20000ea8 = 0;
  *(uint32_t*)0x20000eac = 0;
  *(uint32_t*)0x20000eb0 = 0;
  *(uint32_t*)0x20000eb4 = 0;
  *(uint32_t*)0x20000eb8 = 0;
  *(uint32_t*)0x20000ebc = 0;
  *(uint32_t*)0x20000ec0 = 0;
  *(uint32_t*)0x20000ec4 = 0;
  *(uint32_t*)0x20000ec8 = 0;
  *(uint32_t*)0x20000ecc = 0;
  *(uint32_t*)0x20000ed0 = 0;
  *(uint32_t*)0x20000ed4 = 0;
  *(uint32_t*)0x20000ed8 = 0;
  *(uint32_t*)0x20000edc = 0;
  *(uint32_t*)0x20000ee0 = 0;
  *(uint32_t*)0x20000ee4 = 0;
  *(uint32_t*)0x20000ee8 = 0;
  *(uint32_t*)0x20000eec = 0;
  *(uint32_t*)0x20000ef0 = 0;
  *(uint32_t*)0x20000ef4 = 0;
  *(uint32_t*)0x20000ef8 = 0;
  *(uint32_t*)0x20000efc = 0;
  *(uint32_t*)0x20000f00 = 0;
  *(uint32_t*)0x20000f04 = 0;
  *(uint32_t*)0x20000f08 = 0;
  *(uint32_t*)0x20000f0c = 0;
  *(uint32_t*)0x20000f10 = 0;
  *(uint32_t*)0x20000f14 = 0;
  *(uint32_t*)0x20000f18 = 0;
  *(uint32_t*)0x20000f1c = 0;
  *(uint32_t*)0x20000f20 = 0;
  *(uint32_t*)0x20000f24 = 0;
  *(uint32_t*)0x20000f28 = 0;
  *(uint32_t*)0x20000f2c = 0;
  *(uint32_t*)0x20000f30 = 0;
  *(uint32_t*)0x20000f34 = 0;
  *(uint32_t*)0x20000f38 = 0;
  *(uint32_t*)0x20000f3c = 0;
  *(uint32_t*)0x20000f40 = 0;
  *(uint32_t*)0x20000f44 = 0;
  *(uint32_t*)0x20000f48 = 0;
  *(uint32_t*)0x20000f4c = 0;
  *(uint32_t*)0x20000f50 = 0;
  *(uint32_t*)0x20000f54 = 0;
  *(uint32_t*)0x20000f58 = 0;
  *(uint32_t*)0x20000f5c = 0;
  *(uint32_t*)0x20000f60 = 0;
  *(uint32_t*)0x20000f64 = 0;
  *(uint32_t*)0x20000f68 = 0;
  *(uint32_t*)0x20000f6c = 0;
  *(uint32_t*)0x20000f70 = 0;
  *(uint32_t*)0x20000f74 = 0;
  *(uint32_t*)0x20000f78 = 0;
  *(uint32_t*)0x20000f7c = 0;
  *(uint32_t*)0x20000f80 = 0;
  *(uint32_t*)0x20000f84 = 0;
  *(uint32_t*)0x20000f88 = 0;
  *(uint32_t*)0x20000f8c = 0;
  *(uint32_t*)0x20000f90 = 0;
  *(uint32_t*)0x20000f94 = 0;
  *(uint32_t*)0x20000f98 = 0;
  syscall(__NR_write, r[0], 0x20000b40ul, 0x45cul);
  syscall(__NR_ioctl, r[0], 0x5501, 0);
  return 0;
}
