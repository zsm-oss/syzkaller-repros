// general protection fault in getname_kernel
// https://syzkaller.appspot.com/bug?id=b5d799a2cc619607ed6e322f97f78eaf534451e1
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

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);

  memcpy((void*)0x20000000, "./file0", 8);
  syscall(__NR_mkdir, 0x20000000, 0);
  memcpy((void*)0x20000080, "./file0", 8);
  memcpy((void*)0x20000040, "gfs2meta", 9);
  memcpy((void*)0x200000c0, "\x45\x96\xd4\x66\x5d\x55\x62\x5f\x77\x0d\xac\x36"
                            "\xf4\x4d\x74\xe6\xce\xdd\x7c\x7b\x69\x02\x5d\x11"
                            "\x82\x15\x2d\xc7",
         28);
  syscall(__NR_mount, 0, 0x20000080, 0x20000040, 0, 0x200000c0);
  return 0;
}
