// INFO: task hung in bt_get
// https://syzkaller.appspot.com/bug?id=e9046226143f77515ca2b7d680fdd763e17f0521
// status:open
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};
void execute_one()
{
  long res = 0;
  memcpy((void*)0x2000fffa, "./bus", 6);
  res = syscall(__NR_open, 0x2000fffa, 0x141042, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_close, r[0]);
  memcpy((void*)0x20000000, "/dev/loop#", 11);
  res = syz_open_dev(0x20000000, 0, 0x105080);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000140,
         "\x00\x00\x00\x00\x8c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 15);
  res = syscall(__NR_memfd_create, 0x20000140, 0);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x20000080 = 0x200000c0;
  memcpy((void*)0x200000c0, "\xa8", 1);
  *(uint64_t*)0x20000088 = 1;
  syscall(__NR_pwritev, r[2], 0x20000080, 1, 0x81003);
  syscall(__NR_ioctl, r[1], 0x4c00, r[2]);
  syscall(__NR_ioctl, r[0], 0x4c06, r[0]);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (;;) {
    loop();
  }
}
