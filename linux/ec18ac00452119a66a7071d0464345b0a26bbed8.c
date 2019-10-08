// general protection fault in kvm_arch_vcpu_ioctl_run
// https://syzkaller.appspot.com/bug?id=ec18ac00452119a66a7071d0464345b0a26bbed8
// status:fixed
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

uint64_t r[6] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000040, "/dev/kvm\x00", 9);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000040, 0, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_ioctl, r[0], 0xae01, 0);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000240 = 0x79;
  *(uint32_t*)0x20000244 = 0;
  *(uint64_t*)0x20000248 = 0;
  *(uint64_t*)0x20000250 = 0;
  *(uint64_t*)0x20000258 = 0;
  *(uint64_t*)0x20000260 = 0;
  *(uint8_t*)0x20000268 = 0;
  *(uint8_t*)0x20000269 = 0;
  *(uint8_t*)0x2000026a = 0;
  *(uint8_t*)0x2000026b = 0;
  *(uint8_t*)0x2000026c = 0;
  *(uint8_t*)0x2000026d = 0;
  *(uint8_t*)0x2000026e = 0;
  *(uint8_t*)0x2000026f = 0;
  *(uint8_t*)0x20000270 = 0;
  *(uint8_t*)0x20000271 = 0;
  *(uint8_t*)0x20000272 = 0;
  *(uint8_t*)0x20000273 = 0;
  *(uint8_t*)0x20000274 = 0;
  *(uint8_t*)0x20000275 = 0;
  *(uint8_t*)0x20000276 = 0;
  *(uint8_t*)0x20000277 = 0;
  *(uint8_t*)0x20000278 = 0;
  *(uint8_t*)0x20000279 = 0;
  *(uint8_t*)0x2000027a = 0;
  *(uint8_t*)0x2000027b = 0;
  *(uint8_t*)0x2000027c = 0;
  *(uint8_t*)0x2000027d = 0;
  *(uint8_t*)0x2000027e = 0;
  *(uint8_t*)0x2000027f = 0;
  *(uint8_t*)0x20000280 = 0;
  *(uint8_t*)0x20000281 = 0;
  *(uint8_t*)0x20000282 = 0;
  *(uint8_t*)0x20000283 = 0;
  *(uint8_t*)0x20000284 = 0;
  *(uint8_t*)0x20000285 = 0;
  *(uint8_t*)0x20000286 = 0;
  *(uint8_t*)0x20000287 = 0;
  *(uint8_t*)0x20000288 = 0;
  *(uint8_t*)0x20000289 = 0;
  *(uint8_t*)0x2000028a = 0;
  *(uint8_t*)0x2000028b = 0;
  *(uint8_t*)0x2000028c = 0;
  *(uint8_t*)0x2000028d = 0;
  *(uint8_t*)0x2000028e = 0;
  *(uint8_t*)0x2000028f = 0;
  *(uint8_t*)0x20000290 = 0;
  *(uint8_t*)0x20000291 = 0;
  *(uint8_t*)0x20000292 = 0;
  *(uint8_t*)0x20000293 = 0;
  *(uint8_t*)0x20000294 = 0;
  *(uint8_t*)0x20000295 = 0;
  *(uint8_t*)0x20000296 = 0;
  *(uint8_t*)0x20000297 = 0;
  *(uint8_t*)0x20000298 = 0;
  *(uint8_t*)0x20000299 = 0;
  *(uint8_t*)0x2000029a = 0;
  *(uint8_t*)0x2000029b = 0;
  *(uint8_t*)0x2000029c = 0;
  *(uint8_t*)0x2000029d = 0;
  *(uint8_t*)0x2000029e = 0;
  *(uint8_t*)0x2000029f = 0;
  *(uint8_t*)0x200002a0 = 0;
  *(uint8_t*)0x200002a1 = 0;
  *(uint8_t*)0x200002a2 = 0;
  *(uint8_t*)0x200002a3 = 0;
  *(uint8_t*)0x200002a4 = 0;
  *(uint8_t*)0x200002a5 = 0;
  *(uint8_t*)0x200002a6 = 0;
  *(uint8_t*)0x200002a7 = 0;
  syscall(__NR_ioctl, r[1], 0x4068aea3, 0x20000240);
  res = syscall(__NR_ioctl, r[1], 0xae41, 0);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x200002c0 = 0;
  *(uint32_t*)0x200002c8 = 0;
  *(uint16_t*)0x200002cc = 0;
  *(uint8_t*)0x200002ce = 0;
  *(uint8_t*)0x200002cf = 0;
  *(uint8_t*)0x200002d0 = 0;
  *(uint8_t*)0x200002d1 = 0;
  *(uint8_t*)0x200002d2 = 0;
  *(uint8_t*)0x200002d3 = 0;
  *(uint8_t*)0x200002d4 = 0;
  *(uint8_t*)0x200002d5 = 0;
  *(uint8_t*)0x200002d6 = 0;
  *(uint8_t*)0x200002d7 = 0;
  *(uint64_t*)0x200002d8 = 0;
  *(uint32_t*)0x200002e0 = 0;
  *(uint16_t*)0x200002e4 = 0;
  *(uint8_t*)0x200002e6 = 0;
  *(uint8_t*)0x200002e7 = 0;
  *(uint8_t*)0x200002e8 = 0;
  *(uint8_t*)0x200002e9 = 0;
  *(uint8_t*)0x200002ea = 0;
  *(uint8_t*)0x200002eb = 0;
  *(uint8_t*)0x200002ec = 0;
  *(uint8_t*)0x200002ed = 0;
  *(uint8_t*)0x200002ee = 0;
  *(uint8_t*)0x200002ef = 0;
  *(uint64_t*)0x200002f0 = 0;
  *(uint32_t*)0x200002f8 = 0;
  *(uint16_t*)0x200002fc = 0;
  *(uint8_t*)0x200002fe = 0;
  *(uint8_t*)0x200002ff = 0;
  *(uint8_t*)0x20000300 = 0;
  *(uint8_t*)0x20000301 = 0;
  *(uint8_t*)0x20000302 = 0;
  *(uint8_t*)0x20000303 = 0;
  *(uint8_t*)0x20000304 = 0;
  *(uint8_t*)0x20000305 = 0;
  *(uint8_t*)0x20000306 = 0;
  *(uint8_t*)0x20000307 = 0;
  *(uint64_t*)0x20000308 = 0;
  *(uint32_t*)0x20000310 = 0;
  *(uint16_t*)0x20000314 = 0;
  *(uint8_t*)0x20000316 = 0;
  *(uint8_t*)0x20000317 = 0;
  *(uint8_t*)0x20000318 = 0;
  *(uint8_t*)0x20000319 = 0;
  *(uint8_t*)0x2000031a = 0;
  *(uint8_t*)0x2000031b = 0;
  *(uint8_t*)0x2000031c = 0;
  *(uint8_t*)0x2000031d = 0;
  *(uint8_t*)0x2000031e = 0;
  *(uint8_t*)0x2000031f = 0;
  *(uint64_t*)0x20000320 = 0;
  *(uint32_t*)0x20000328 = 0;
  *(uint16_t*)0x2000032c = 0;
  *(uint8_t*)0x2000032e = 0;
  *(uint8_t*)0x2000032f = 0;
  *(uint8_t*)0x20000330 = 0;
  *(uint8_t*)0x20000331 = 0;
  *(uint8_t*)0x20000332 = 0;
  *(uint8_t*)0x20000333 = 0;
  *(uint8_t*)0x20000334 = 0;
  *(uint8_t*)0x20000335 = 0;
  *(uint8_t*)0x20000336 = 0;
  *(uint8_t*)0x20000337 = 0;
  *(uint64_t*)0x20000338 = 0;
  *(uint32_t*)0x20000340 = 0;
  *(uint16_t*)0x20000344 = 0;
  *(uint8_t*)0x20000346 = 0;
  *(uint8_t*)0x20000347 = 0;
  *(uint8_t*)0x20000348 = 0;
  *(uint8_t*)0x20000349 = 0;
  *(uint8_t*)0x2000034a = 0;
  *(uint8_t*)0x2000034b = 0;
  *(uint8_t*)0x2000034c = 0;
  *(uint8_t*)0x2000034d = 0;
  *(uint8_t*)0x2000034e = 0;
  *(uint8_t*)0x2000034f = 0;
  *(uint64_t*)0x20000350 = 0;
  *(uint32_t*)0x20000358 = 0;
  *(uint16_t*)0x2000035c = 0;
  *(uint8_t*)0x2000035e = 0;
  *(uint8_t*)0x2000035f = 0;
  *(uint8_t*)0x20000360 = 0;
  *(uint8_t*)0x20000361 = 0;
  *(uint8_t*)0x20000362 = 0;
  *(uint8_t*)0x20000363 = 0;
  *(uint8_t*)0x20000364 = 0;
  *(uint8_t*)0x20000365 = 0;
  *(uint8_t*)0x20000366 = 0;
  *(uint8_t*)0x20000367 = 0;
  *(uint64_t*)0x20000368 = 0;
  *(uint32_t*)0x20000370 = 0;
  *(uint16_t*)0x20000374 = 0;
  *(uint8_t*)0x20000376 = 0;
  *(uint8_t*)0x20000377 = 0;
  *(uint8_t*)0x20000378 = 0;
  *(uint8_t*)0x20000379 = 0;
  *(uint8_t*)0x2000037a = 0;
  *(uint8_t*)0x2000037b = 0;
  *(uint8_t*)0x2000037c = 0;
  *(uint8_t*)0x2000037d = 0;
  *(uint8_t*)0x2000037e = 0;
  *(uint8_t*)0x2000037f = 0;
  *(uint64_t*)0x20000380 = 0;
  *(uint16_t*)0x20000388 = 0;
  *(uint16_t*)0x2000038a = 0;
  *(uint16_t*)0x2000038c = 0;
  *(uint16_t*)0x2000038e = 0;
  *(uint64_t*)0x20000390 = 0;
  *(uint16_t*)0x20000398 = 0;
  *(uint16_t*)0x2000039a = 0;
  *(uint16_t*)0x2000039c = 0;
  *(uint16_t*)0x2000039e = 0;
  *(uint64_t*)0x200003a0 = 0;
  *(uint64_t*)0x200003a8 = 0;
  *(uint64_t*)0x200003b0 = 0;
  *(uint64_t*)0x200003b8 = 0;
  *(uint64_t*)0x200003c0 = 0;
  *(uint64_t*)0x200003c8 = 0;
  *(uint64_t*)0x200003d0 = 0;
  *(uint64_t*)0x200003d8 = 0;
  *(uint64_t*)0x200003e0 = 0;
  *(uint64_t*)0x200003e8 = 0;
  *(uint64_t*)0x200003f0 = 0;
  syscall(__NR_ioctl, r[2], 0x4138ae84, 0x200002c0);
  memcpy((void*)0x20000040, "/dev/kvm\x00", 9);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000040, 0, 0);
  if (res != -1)
    r[3] = res;
  res = syscall(__NR_ioctl, r[3], 0xae01, 0);
  if (res != -1)
    r[4] = res;
  res = syscall(__NR_ioctl, r[4], 0xae41, 0);
  if (res != -1)
    r[5] = res;
  *(uint32_t*)0x20000340 = 1;
  *(uint32_t*)0x20000344 = 0;
  *(uint32_t*)0x20000348 = 0x4000009e;
  *(uint32_t*)0x2000034c = 0;
  *(uint64_t*)0x20000350 = 0;
  syscall(__NR_ioctl, r[5], 0x4008ae89, 0x20000340);
  syscall(__NR_ioctl, r[5], 0xae80, 0);
  return 0;
}
