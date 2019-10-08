// general protection fault in __free_pages
// https://syzkaller.appspot.com/bug?id=6dcd14a729df98f989c7b76d254226ae67084efd
// status:invalid
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

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  memcpy((void*)0x20265ff7, "/dev/sg#", 9);
  res = syz_open_dev(0x20265ff7, 0, 2);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x2012f000 = 0x50;
  *(uint32_t*)0x2012f004 = 0;
  *(uint64_t*)0x2012f008 = 0;
  *(uint32_t*)0x2012f010 = 7;
  *(uint32_t*)0x2012f014 = 0x1a;
  *(uint32_t*)0x2012f018 = 0;
  *(uint32_t*)0x2012f01c = 0;
  *(uint16_t*)0x2012f020 = 0;
  *(uint16_t*)0x2012f022 = 0;
  *(uint32_t*)0x2012f024 = 0;
  *(uint32_t*)0x2012f028 = 0;
  *(uint32_t*)0x2012f02c = 0;
  *(uint32_t*)0x2012f030 = 0;
  *(uint32_t*)0x2012f034 = 0;
  *(uint32_t*)0x2012f038 = 0;
  *(uint32_t*)0x2012f03c = 0;
  *(uint32_t*)0x2012f040 = 0;
  *(uint32_t*)0x2012f044 = 0;
  *(uint32_t*)0x2012f048 = 0;
  *(uint32_t*)0x2012f04c = 0;
  syscall(__NR_write, r[0], 0x2012f000, 0x50);
  syscall(__NR_read, r[0], 0x20a8bfff, 1);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
