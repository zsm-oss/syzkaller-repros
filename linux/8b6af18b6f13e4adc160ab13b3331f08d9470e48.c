// WARNING in set_precision
// https://syzkaller.appspot.com/bug?id=8b6af18b6f13e4adc160ab13b3331f08d9470e48
// status:fixed
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
    strncpy(buf, (char*)a0, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

void loop()
{
  memcpy((void*)0x20002ec0, "/dev/sg#", 9);
  syz_open_dev(0x20002ec0, 2, 0);
  memcpy((void*)0x20000280, "dns_resolver", 13);
  *(uint8_t*)0x200002c0 = 0x73;
  *(uint8_t*)0x200002c1 = 0x79;
  *(uint8_t*)0x200002c2 = 0x7a;
  *(uint8_t*)0x200002c3 = 0;
  *(uint8_t*)0x200002c4 = 0;
  syscall(__NR_add_key, 0x20000280, 0x200002c0, 0x20000300, 0xfffff,
          0xfffffffd);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
