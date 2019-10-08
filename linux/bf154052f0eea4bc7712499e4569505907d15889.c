// possible deadlock in blkdev_reread_part
// https://syzkaller.appspot.com/bug?id=bf154052f0eea4bc7712499e4569505907d15889
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block",
            (uint8_t)a1, (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

#ifndef __NR_mmap
#define __NR_mmap 90
#endif
#ifndef __NR_memfd_create
#define __NR_memfd_create 356
#endif
#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[20];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  memcpy((void*)0x2040aff5,
         "\x2f\x64\x65\x76\x2f\x6c\x6f\x6f\x70\x23\x00", 11);
  r[2] = syz_open_dev(0x2040aff5ul, 0xfffffffffffffffeul, 0x1ul);
  memcpy((void*)0x20614000, "\x74\x75\x6e\x08\x00\x00\x00\x00\x00\x00"
                            "\x00\x80\x00\x00\x00\x00",
         16);
  r[4] = syscall(__NR_memfd_create, 0x20614000ul, 0x0ul);
  r[5] = syscall(__NR_ioctl, r[2], 0x4c00ul, r[4]);
  *(uint32_t*)0x20480f74 = (uint32_t)0x0;
  *(uint32_t*)0x20480f78 = (uint32_t)0x0;
  *(uint32_t*)0x20480f7c = (uint32_t)0x0;
  *(uint32_t*)0x20480f80 = (uint32_t)0x0;
  *(uint32_t*)0x20480f84 = (uint32_t)0x4d0c;
  *(uint32_t*)0x20480f88 = (uint32_t)0x0;
  *(uint32_t*)0x20480f8c = (uint32_t)0x19;
  *(uint32_t*)0x20480f90 = (uint32_t)0x19;
  memcpy((void*)0x20480f94,
         "\x6a\xd1\xac\x55\x2a\x20\x53\x84\x37\x03\x55\xd0\xe6\x76\xa5"
         "\x36\xa6\xa0\xd6\xcf\x25\x88\x66\xd7\x36\xe7\xe3\xae\x08\x19"
         "\x10\x31\xb2\x6f\xa1\x1f\xda\xd8\xfc\x15\x97\x13\x72\xc9\xc3"
         "\x9c\x37\xfa\xff\x14\xfe\x28\x3f\x54\x3c\xbd\x00\x6d\x78\x7e"
         "\x29\x7e\xe0\x88",
         64);
  memcpy((void*)0x20480fd4, "\xa5\x22\xdd\x05\x3d\xb5\xd3\x02\xd1\x2f"
                            "\xcf\x49\x00\x33\xac\xea\x72\xb8\xaf\xc4"
                            "\xb4\x6b\x86\xf1\xc7\x41\xe0\x92\xda\xf3"
                            "\x0f\x66",
         32);
  *(uint32_t*)0x20480ff4 = (uint32_t)0x100000001;
  *(uint32_t*)0x20480ff8 = (uint32_t)0x1;
  *(uint32_t*)0x20480ffc = (uint32_t)0x0;
  r[19] = syscall(__NR_ioctl, r[2], 0x4c02ul, 0x20480f74ul);
}

int main()
{
  loop();
  return 0;
}
