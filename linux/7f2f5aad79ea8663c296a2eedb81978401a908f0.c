// WARNING: kmalloc bug in str_read
// https://syzkaller.appspot.com/bug?id=7f2f5aad79ea8663c296a2eedb81978401a908f0
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

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20000000, "/selinux/load", 14);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000000, 2, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000380,
         "\x8c\xff\x7c\xf9\x08\x00\x00\x00\x53\x45\x20\x4c\x69\x6e\x75\x78\x16"
         "\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00\x07\x00\x00\x00\x40\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x99\xa3\x00\x61\xfa\x07\x11\x23"
         "\x13\xd6\xff\x7f\x0d\xbb\x63\xde\x43\x02\x9c\xde\x1d\x6d\xce\x0e\xa0"
         "\x86\x54\x58\xff\xa5\xda\x33\xf1\x09\x9a\x86\x77\x45\xaa\x29\x40\x2a"
         "\x20\x93\x63\xa0\xe0\xe4\xdf\x04\x28\x67\x68\xc1\x01\x30\x95\xd6\x24"
         "\xdc\x32\x00\xdd\xf4\xa9\xdc\x25\x5b\x9d\x27\xcd\x90\x9d\x1a\xe1\x3d"
         "\xa4\xb7\x83\xe8\xc4\x49\x24\x2e\xeb\x9f\x68\xa3\x0b\xdd\xca\x94\xeb"
         "\x1b\x43\x56\xd8\x58\xf0\x26\xce\x57\x69\xe0\xb6\xe8\x3e\x10\x66\xa6"
         "\xfa\x20\x09\x4f\x6f\xa7\x82\x7a\x3a\x35\x6a\x4b\xc8\xe0\x0e\xbd\xe3"
         "\xd0\xaf\xc4\x88\x60\x2a\x35\xc7\x41\xb5\xf0\x68\xb0\xdf\x84\x4a\xf7"
         "\x53\x04\xbb\x23\x07\x26\x29\xcb\x0c\xbe\x6f\xdf\xe6\xff\xa0\x69\xe1"
         "\x08\x80\xe9\xf6\x95\xe1\xaa\x3c\x5f\x4c\x9a\x18\x8c\xd2\xa5\x73\x2d"
         "\x61\x6c\x6d\x95\xb2\x0c\x5d\x68\xa4\x90\x9f\x78\x1f\x42\xc3\x49\x14"
         "\xf2\x11\x90\xa4\x1e\x7b\xc2\xb4\xe8\x64\xf4\x34\x89\x68\x5c\x5a\xcd"
         "\x1a\x74\xf4\xa3\x96\x82\x67\x66\xfb\x22\xe1\xf7\xe4\xa0\x88\x94\xbf"
         "\x8a\x77\x3c\xf0\xb5\x4a\x75\x36\xc1\x14\x24\x6e\xc7\x0b\xd0\xec\x33"
         "\x99\x5d\xb4\x15\xce\x21\xb1\x52\x54\xd2\xd2\x73\x02\xf3\x84\xcf\xec"
         "\xcd\x24\x83\x7c\x2d\xfe\xc7\x90\x53\x29\x91\xe4\x0d\x49\x79\x91\x69"
         "\x5a\xa9\x00\xa0\x3d\x07\x9a\xee\x32\xff\xc9\x2a\x6f\xa4\x6b\x94\xfd"
         "\xd0\xa4\x7c\xf7\x9b\x0b\xa6\xf7\x04\x94\xbe\xa5\x51\x92\x73",
         355);
  syscall(__NR_write, r[0], 0x20000380, 0x163);
  return 0;
}
