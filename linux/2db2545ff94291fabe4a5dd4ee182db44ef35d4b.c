// BUG: GPF in non-whitelisted uaccess (non-canonical address?)
// https://syzkaller.appspot.com/bug?id=2db2545ff94291fabe4a5dd4ee182db44ef35d4b
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  res = syscall(__NR_socket, 0xa, 0x400000000001, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_close, r[0]);
  memcpy((void*)0x20000940, "/dev/uhid", 10);
  syscall(__NR_openat, 0xffffffffffffff9c, 0x20000940, 0x802, 0);
  memcpy((void*)0x20000040, "./bus", 6);
  res = syscall(__NR_open, 0x20000040, 0x141042, 0);
  if (res != -1)
    r[1] = res;
  *(uint32_t*)0x20000580 = 0;
  memcpy((void*)0x20000584,
         "\x73\x79\x7a\x31\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         128);
  memcpy((void*)0x20000604,
         "\x73\x79\x7a\x31\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  memcpy((void*)0x20000644,
         "\x73\x79\x7a\x31\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  *(uint64_t*)0x20000684 = 0x20000440;
  *(uint16_t*)0x2000068c = 0x80;
  *(uint16_t*)0x2000068e = 0;
  *(uint32_t*)0x20000690 = 0;
  *(uint32_t*)0x20000694 = 0;
  *(uint32_t*)0x20000698 = 0;
  *(uint32_t*)0x2000069c = 0;
  syscall(__NR_write, r[1], 0x20000580, 0x120);
  *(uint64_t*)0x20000000 = 0x20000980;
  memcpy(
      (void*)0x20000980,
      "\x4a\xcc\x58\x2d\x79\xa6\x64\xb8\x45\xdf\xb8\xee\x62\x6a\x97\xea\xfc\x2a"
      "\x28\xe1\x5d\x98\x3c\xb1\x10\xe8\x23\x41\x1f\x63\x2d\x70\x41\xff\x47\x07"
      "\x38\xa9\xb7\xa5\xea\x70\xf6\x02\x78\x24\x5a\x34\x64\xdd\x84\x71\x46\xc0"
      "\x5a\xad\x7d\x4e\xc6\x8f\x49\x56\x39\x0f\xe8\x97\x03\x78\x48\x98\xe6\x41"
      "\xef\xc2\xa8\x68\x54\x55\x4b\x75\x7c\xa0\xef\xe4\x03\xf1\x10\xe4\x64\x81"
      "\x99\x4a\xb0\x9d\xbf\xad\xaa\x4c\xbe\x00\x79\xbc\x83\xee\xf7\xe4\x7f\x40"
      "\xfd\x0f\x4f\x5a\xf7\xd3\x04\x96\x8c\x96\x3a\x19\xe2\x08\x03\xb9\x2b\x9c"
      "\xff\x7d\xed\x7f\x9c\xbc\x46\x3e\x43\xc9\xa2\x6b\x3d\x59\xe4\x0c\xa2\x66"
      "\x93\xf3\x31\x0d\xe1\x3a\xc8\x14\x27\x6b\x90\xb9\xc7\xa4\xcb\x76\x13\xbd"
      "\xfa\xfe\xdc\xab\xc1\xa9\x59\x5d\x4f\xe0\x92\x8f\xde\xe8\x56\x21\x27\x5b"
      "\xc3\x5f\xd9\xd2\xc2\xa4\xe6\xe6\x3e\x63\xd4\xea\xd6\xec\x19\x8c\x7a\x68"
      "\xa5\x9d\x76\xe5\x28\xf8\x37\x10\x24\x09\x01\x86\xe5\x40\x95\x63\x71\xc7"
      "\xa4\x29\x79\x7c\x32\x40\x68\x13\x4d\x22\x30\xc9\x8d\x93\x17\x3a\xe6\x80"
      "\xab\x3b\x64\x16\xe2\xdc\x51\x1c\x58\xc8\x50\xd4\x2c\xf2\xe1\x15\xb4\x4c"
      "\x9b\xa4\xb1\x31\x06\x7f\xe2\x8e\x5e\xa9\x95\x7c\xe4\xe5\x8b\xe9\x3d\xc5"
      "\x03\x8f\x26\x4c\xbe\x37\xa7\x39\xc6\xdb\x8b\x5d\xd5\x81\x2e\x31\xe9\x5f"
      "\x95\xf2\x10\xd1\x0a\x56\x74\x1f\xc9\x1c\x6e\x65\x52\xd0\x2e\x07\x0d\x09"
      "\xe2\x20\xcd\x89\xe6\x72\x53\xa3\xa7\x64\x70\xd0\xf6\xe0\xd4\x32\x17\xb8"
      "\x24\xf3\x86\xb8\xa6\xd1\x31\x98\xf5\x5a\xfc\xc5\x1c\x8b\x23\xe4\xac\x89"
      "\x1f\x99\xae\x7f\xdb\x34\x1c\x08\x93\x8f\x87\x34\x6f\x86\xcd\x6b\x66\xef"
      "\x82\x63\x8b\x1e\x1f\xf3\x61\x56\x2b\x43\x0d\x74\x43\x59\xdd\x5d\xd6\xd5"
      "\xee\x30\xc8\xb2\xd1\x35\x7c\xd6\x7f\x12\x6b\x9e\xaf\x36\x98\x77\xc0\x00"
      "\x6c\x4c\x5e\x81\xa5\xc9\x14\xf0\xcf\x02\x8b\x13\x43\xd4\xb3\x25\xb4\x9c"
      "\x35\xe6\xc8\x7f\x8e\xa7\xc9\xb8\xa1\xaa\xd9\x6e\xf2\xa7\xda\x87\x14\x10"
      "\x82\x4c\x82\x10\x63\x7d\x75\xec\xa7\x15\x6d\x4e\x95\x5d\x19\xd3\xe4\x56"
      "\x31\x41\x4a\xfc\xd6\x26\x68\xb9\x40\xe4\x33\xff\xa0\x2b\x02\x69\xd4\x2e"
      "\xbc\x3d\xdb\x80\x35\x45\xa7\x97\xbe\x54\x71\xe0\x19\x2b\x43\xc0\xb9\x98"
      "\x99\xfa\x5d\x07\x0f\x1c\x99\x57\x21\xa0\x0e\xcf\xc4\x50\x7e\x0b\x8d\x51"
      "\x8d\xb8\x88\x33\x7e\xec\x34\x38\xa6\x8d\xc4\xc4\x44\x26\xf0\x8d\x79\x11"
      "\x5d\x5c\x63\x17\x6b\xd8\x94\x95\x69\x65\x39\x13\x45\x0f\x9e\x4d\x3b\x00"
      "\xce\x4c\xf4\xa9\xd5\xbf\x8b\xfb\x94\x48\x72\x25\xae\x83\x69\xef\x84\x00"
      "\x1b\xcc\x38\xb7\xc4\x55\x75\x3f\x57\x90\xec\x6a\xa5\xf8\xd3\x83\x7a\x11"
      "\xb2\xc9\x8e\x57\xe1\xb2\xcf\xd6\xcd\xee\xb1\x7d\xe0\x93\xbc\x41\xd8\x67"
      "\x1c\x1a\xf6\x75\x25\xf1\x44\x5e\xff\xb5\x4e\xf9\x97\x46\x91\x74\xb9\x9c"
      "\x8b\x10\x41\x65\x79\x66\x6d\x3e\x9e\xb4\xf3\x90\xba\xaf\x15\xcc\x1f\x0a"
      "\x1c\x77\xd7\xa5\xa5\xaa\x82\x4c\x63\xa2\x56\x72\x4b\x45\x1d\xfb\xff\xc7"
      "\xce\xcc\xd8\x66\x54\x23\xe6\x35\x13\x01\x47\xc5\xd3\xc6\x4b\x2c\x85\x5f"
      "\x06\x0a\x56\xa5\x5b\x46\x19\xa8\x41\x5d\x88\xb5\x44\x7a\xd6\xec\x3d\x07"
      "\x7b\xff\x12\xb1\x86\xb8\x63\x91\x98\xd9\x9d\x05\x2b\x73\xb8\xa2\x44\x99"
      "\x34\xbe\x5c\x40\x4c\xfb\xcc\xe7\x6b\x6c\xd1\x27\x55\x6a\x5d\xe5\xb8\x10"
      "\xa6\xe9\xed\xbd\xd7\x84\x15\xb9\x2f\x6c\x72\x6b\x81\x33\x7c\x8e\xab\x9e"
      "\x26\x1a\x6c\xff\x07\x7e\xec\x72\x9f\x8b\xa4\x60\xbb\x05\xd4\x4b\x38\x25"
      "\xfa\x81\xad\xc0\x66\xee\xc0\xd7\xc1\x65\xce\x34\x5e\x09\xce\xa1\xfc\x40"
      "\xb5\xe7\xae\xdd\x99\x1a\xc2\x5d\xd5\x8a\x59\xce\xe8\x88\x23\x1e\xf9\xb3"
      "\x32\x5f\xf8\x9b\xc0\x37\x01\x42\xc4\xae\x22\xe3\xcd\xde\xb0\xbc\xf5\xc7"
      "\x45\x73\x11\xf2\x55\xf6\x2f\xbb\xff\x20\x5e\x05\x10\x16\x4f\x27\xff\x9c"
      "\x30\xec\x13\x69\xef\x69\x81\x52\xe8\x8d\x7a\x3d\x10\x98\xa6\x09\x0f\xb5"
      "\x60\xcb\x2e\x0b\x92\xdf\x2b\x0e\xa5\xb0\xec\x81\x3b\x4d\x22\x91\xc4\xc3"
      "\xc1\xba\xf5\x7b\xc3\x8c\xf2\x0a\xd7\xcf\x47\x3d\x7b\x51\x88\xbe\x93\x50"
      "\xac\x61\xd6\x51\xec\xcd\xc3\x4f\xe8\x9b\x70\x88\x13\x89\x82\xca\x80\x20"
      "\x22\x93\x2a\xe5\x94\x46\x0a\xf7\x7b\xb0\x54\x71\xac\x38\x93\x18\xcf\x04"
      "\x6d\x53\x6a\x75\x30\x48\x36\x4b\x03\xbb\x68\x2a\xba\x6c\xb9\x59\x7f\xf2"
      "\xfd\xb7\xf7\xfc\x64\x52\x6a\x77\xd2\x3c\x8d\xee\xe6\xa8\x4f\xf2\xf9\xb2"
      "\x21\x5a\x10\x0f\x8f\x99\xe5\xf5\x3e\x31\xfc\xff\xc8\x1f\x69\xb7\x91\xd8"
      "\x96\xa5\xf0\x60\x02\x69\xb4\x8f\x55\xaa\x45\x42\x42\xbf\x58\x1e\x1d\x2e"
      "\x1b\x35\xe7\x25\xfb\x59\x5c\x86\xbb\xae\x30\xef\x6c\xde\xb7\xdf\xb4\x22"
      "\x57\xbf\x8e\xf5\x1c\xf7\x13\x4b\x53\xa0\x98\x53\x28\xda\x46\x83\x85\x62"
      "\xf2\x02\xdd\x9b\x2c\x86\x2e\x59\x95\x9a\x5f\xd5\x75\x50\x80\xe0\x19\x32"
      "\x1e\x71\x2c\x5d\xfc\x1b\x2b\x2f\x78\xdb\x30\xba\xe0\x8a\x0f\x33\x84\xfa"
      "\x44\x61\x6c\x72\x44\x2b\x9a\x60\x63\x03\xab\x98\xdc\xde\x21\xa9\x5f\xe5"
      "\x02\x9d\x5f\x04\xc7\x83\xd2\x6f\x21\x73\x7b\xb9\xb9\xf8\x00\x6e\x1e\x5f"
      "\x44\x98\xf3\x51\x8d\x22\x38\xb3\xba\xff\x5e\x76\xde\x08\xf6\x99\xcc\x25"
      "\xf6\x43\xed\x07\x59\xf2\xcf\xd7\x06\x1c\x9c\x1f\xc2\x49\xde\x9f\x64\x8e"
      "\x69\x59\x59\x2a\x20\x2f\x8e\x09\x9b\x91\x57\x13\xd0\x15\x89\x6f\xfa\x5d"
      "\x7d\x1e\x4c\x40\xa9\x48\x77\x9a\x50\x2d\x3e\x6c\x9c\x8c\x54\x4c\x40\x47"
      "\x20\xf6\x60\x7a\x24\x16\x50\x39\x3f\xde\x7b\xf5\xc2\x06\x3a\x3a\x90\x75"
      "\xcb\x9f\xdb\x3a\x42\x1c\xf3\x10\xdc\xf3\x8d\xdc\x22\x84\x44\xb0\xe2\xde"
      "\x4b\xc3\x50\xfd\x6c\x45\xf6\xfc\x99\xfc\xec\x8b\xd9\x19\xfe\x28\x0e\xc8"
      "\x58\x67\xf0\xbf\x0f\x04\x9f\x4a\x49\xfb\xc2\x5e\x3c\x19\x77\xa0\xc4\x0e"
      "\x62\x14\x24\xfa\x48\x46\xd2\x4d\xaf\xd2\x8a\x54\x71\x13\x0e\x1a\x25\xad"
      "\x41\x03\x64\x2e\x36\x2f\xeb\xce\x1f\xa7\xb3\x41\x0d\x36\xfa\xc3\x84\x13"
      "\x26\xce\xca\x98\x1d\x17\xab\x60\x72\xe9\x38\x90\xa8\x13\x53\x3a\x8b\xcd"
      "\x3c\xe9\xf3\x8a\xdb\x7a\xa1\x57\xf9\xe7\xfc\xda\x34\x94\x93\xd2\x91\xcd"
      "\xdc\xe6\x19\x8a\xde\xfe\x84\x51\x86\x89\x14\x31\xec\xd7\xa3\xcd\x48\xe6"
      "\xb8\x9a\xb3\x73\xce\xdf\xdd\xc5\x5e\x17\xee\x0e\xa2\xd2\xb3\x49\xde\x29"
      "\x27\x4b\x58\xa1\x28\x03\x64\x82\x88\xa7\xba\x5a\x98\x76\x3a\x66\xb8\x67"
      "\x5d\xdf\x36\x05\xfd\xc2\x86\xec\x2b\x07\x54\xb7\x3f\x13\x5d\x8c\xe5\x90"
      "\xb4\x8e\x82\xcb\xce\x47\xd5\x42\x7b\xf3\xf5\xff\xd0\xce\x7f\xbb\xe4\x9e"
      "\xd7\x66\xd0\x2e\x07\x7b\x0e\xed\x0e\x3f\xc6\x91\x58\x9d\x86\xe2\x08\xdf"
      "\x65\x29\x18\x7d\x3f\xaa\x46\xba\x5f\xc6\xc2\x4f\xc9\x6f\x58\xaf\xf5\x44"
      "\xb6\xc2\x99\x09\x83\xda\xb8\x35\xaf\xf7\x43\x4f\x94\xdb\xc1\xf0\x12\xcd"
      "\x19\xbb\x84\x70\x50\xc7\xbb\x7d\x69\x16\xa2\xa0\xb6\x4e\x97\x17\x00\x08"
      "\x3c\xd5\xe8\x23\x92\x98\x3a\xe8\x97\x7f\xba\x2c\x4d\x98\xec\xaf\x6a\xeb"
      "\xf6\xd1\x18\x40\x03\x61\x48\x23\x8d\x22\x14\xc9\x1b\xe4\xf1\x67\xff\x0b"
      "\xe7\xcf\x06\xdd\xc5\x78\x6b\xf5\x7e\x66\x66\x7e\xd6\xf7\xa8\xdc\x88\x3f"
      "\x63\x5c\xf8\x45\xbe\xcc\x59\xf0\x9b\xa4\x28\x9e\x99\xb1\xf8\x05\x33\x5f"
      "\x15\xef\x8b\xf9\x0b\x58\x87\x04\xbf\xfa\x53\x6c\x9b\x59\xeb\xb8\x22\xe3"
      "\xd4\x71\xc3\x7b\x1b\xf0\x89\xa4\x9a\x0a\x0f\x8b\xbf\x92\x80\x24\xf4\x95"
      "\x3c\x45\x5c\x31\x90\x5f\xcc\x20\xbd\xdd\x70\xa4\xb8\x30\x66\xd3\xfd\x9e"
      "\x27\xa5\x6d\x73\x48\xe3\x63\x3a\xbe\x95\xa5\x13\x39\x45\xce\x31\xc7\xd0"
      "\x41\xf1\x74\x7a\xeb\xf0\xf5\x51\xde\xab\x06\x0e\x09\xad\x5c\x76\x12\x66"
      "\xed\xe6\xc0\xf2\x7a\x96\xda\x73\x12\x06\xcc\x44\x39\x73\x29\x6c\x71\x5a"
      "\xa8\x11\xbf\x2c\x6a\xb3\x85\xaf\xd2\xe5\x75\xff\xed\x39\x61\x3c\x40\xc3"
      "\xe6\x7f\xfe\x3a\x6a\x7c\xc7\xe3\x5a\x2f\x96\x6d\x24\xc8\x2e\xb7\xa2\x4e"
      "\x65\xda\x4e\x87\x9c\x94\x36\xf8\xa2\x47\x28\x7d\x25\x06\xe8\xbd\xc0\xe6"
      "\xd1\x2e\xf7\x71\xc1\xf8\x7f\x61\xe2\xaf\xce\x7b\x1c\x3a\x93\x82\xe2\x1a"
      "\xe3\xe2\xa4\x20\xd8\x20\x90\x12\x10\x99\x1d\xce\xa4\x0c\xf9\x10\x78\xb7"
      "\x46\x8e\x12\xd8\x66\x40\x36\x63\xad\xc3\x58\x91\xdd\x37\x4b\x99\xd9\x96"
      "\xf7\xe2\x85\x46\xff\x56\xad\x9f\xed\x9d\x6f\xce\x69\x34\xbf\x7b\x5f\x16"
      "\x7d\x4f\x45\x26\xae\x81\xd3\x87\x7f\xa5\x2f\x0e\xdb\x31\x1d\xb2\x32\xea"
      "\xf9\x7c\x50\x3f\x4f\x86\x22\xe1\x51\x9e\xa1\x7e\x82\x73\x62\xea\xae\x65"
      "\x4b\x29\x5b\xaf\xec\x77\x84\xe4\x71\x85\x2d\xcd\xc3\xab\xd3\x8c\x19\xfa"
      "\xbc\xc1\x2d\x1d\x6c\x85\xd9\x58\x0a\x00\x44\x5e\x18\x71\xa0\xe4\x8c\xc8"
      "\x6a\xc6\xe3\xeb\x18\xe7\x86\x02\x72\xdb\x3b\xe3\x8a\xb2\xa7\x1c\x5e\x9d"
      "\x02\xe8\x6d\x46\x4c\x20\xb4\x56\x1a\x5d\xfd\x42\xa8\xcc\xe1\x04\xde\xeb"
      "\x48\x73\x30\xb3\xdb\xad\x0b\x93\x04\x48\x8b\x69\x78\x43\xf7\xfd\x37\x37"
      "\xf0\xb3\xbe\xe7\xcd\xbc\xcd\xae\x2a\x6e\x45\xa4\xd3\xa9\x2b\x53\xfe\x07"
      "\x6a\xf5\xab\x2e\x9a\xd4\x5d\x57\x8a\x29\x7d\x1b\xcb\x2d\x9a\xa1\xad\x2f"
      "\x1c\x1e\x89\x45\x97\xed\x49\x0c\x4e\x7a\x2d\x38\x59\xbe\xb9\x7d\x6a\x57"
      "\xbb\x06\xd1\xee\x81\x09\xe2\x35\x0a\x70\x5f\x5b\xaf\x85\x5c\x5c\x0b\xe5"
      "\x7f\x47\xd2\xfc\x35\x4c\x28\x01\x0b\xdd\xed\xc8\x3f\x82\x0a\x9f\x06\x61"
      "\xc9\xb8\x5c\x5f\xd5\x99\x1f\xa8\xee\x0f\xcd\xf5\x37\xf4\x34\x74\x3a\x00"
      "\x8b\x87\xa5\x55\xcd\xd8\x6a\xbf\xde\xbd\x45\xb5\xa2\xbe\xbb\xf6\xb4\x8a"
      "\x44\x20\x50\x57\xa8\x47\x96\xc8\xc6\x05\x4f\xe3\xe1\x26\x90\x03\x3d\x43"
      "\x56\x17\x4b\x2d\x47\x89\x75\x42\x5d\x94\x78\x91\x73\x49\xca\x60\xa2\x6d"
      "\x79\xed\x68\x62\x92\xf2\xe0\x06\xd4\xba\xf9\x51\xa7\xde\x53\x6b\x96\xf5"
      "\x82\xe9\x4d\xe2\xa8\x15\xdc\x39\x10\x71\x94\xdb\x71\x23\x9f\xdb\xec\xd7"
      "\xee\x61\x1d\x4a\xe4\x41\xe6\x88\x6a\xca\xbf\x09\x0c\x2c\x2e\x1f\x4f\x65"
      "\x5d\xf7\xbd\xe9\x2d\x49\x69\xef\xdf\x2e\xde\xfa\x1b\x9e\x3f\x83\xf7\x66"
      "\xe9\xb8\x8d\x9e\x8c\x95\xe5\xe6\xd2\xac\x5c\x4d\xbd\x13\x32\x34\xc5\xec"
      "\x5c\x65\xf0\x08\x6e\x84\x20\x77\x06\xf7\x4c\x26\x2b\x5f\xdb\x3c\xbe\x13"
      "\xee\xc1\xdb\x8b\xbd\xce\x73\x8d\x4f\x4d\xd2\x1e\x03\x81\xe9\x30\xdb\xbb"
      "\xb2\xcb\x27\x28\xd2\x1c\xd2\x53\x04\xc4\x2b\x87\xc7\xa4\xc0\x39\xa1\xe2"
      "\x84\x10\x69\xda\xb3\x2e\x58\xd1\x33\x17\x3d\x16\x4a\x6f\x2a\xcd\x38\xa5"
      "\x6e\x5f\xa8\x99\xb2\xd8\xfc\x8b\xb6\x3e\xab\x1c\xd7\x4d\x41\x6f\x78\xd6"
      "\xf3\xe6\x58\x92\x4d\x9f\x41\x6f\xbf\x52\x6e\x7c\x37\x95\xd9\xff\xaf\x75"
      "\x52\x8c\x10\x9b\x9d\xd2\x5e\xc3\xd3\x80\xe5\x2f\xd2\xb4\x75\x36\x2f\x9f"
      "\xf0\x01\xe4\xe5\xb7\x3b\xc2\xf2\xa1\x98\x76\x80\x0c\xaa\x90\xf1\x45\x58"
      "\x8a\xeb\x83\xe7\x68\x78\xd4\xce\x4d\x7e\x86\x87\xec\x20\x12\xd8\x97\x52"
      "\xfb\xbd\xa4\x38\xc6\x1a\xa8\x63\xc3\x08\xc5\x40\xae\x5c\x74\x17\x69\xa3"
      "\x80\x32\x80\x8a\x87\x85\x31\x36\xfe\xd9\x16\x4d\x72\x40\xdf\xb9\xb7\xba"
      "\xab\xcd\x58\x30\xac\xa5\x74\x85\x4d\x27\xd2\xea\x07\x8c\xbd\xf6\x6a\x9e"
      "\x32\x99\x77\xdc\xfe\x8d\x18\x4e\x76\xa6\x03\xc9\xe1\x3c\x59\x51\x52\xdc"
      "\xa1\x22\xec\xdc\x9d\xed\xf8\x09\x7b\x86\xa1\x11\x19\xd8\x77\x6b\xdd\x8f"
      "\x09\x1a\x47\x3a\x56\x2f\x1d\x0f\x08\x47\xd0\x80\x12\xa7\x76\x18\xc2\x1e"
      "\xac\x49\xd1\xce\xeb\x1d\xd6\x31\x2f\xf4\x04\x20\xf1\x09\xe8\x59\x7c\x47"
      "\xc1\x97\x6b\xf8\xc2\x20\xea\xd6\x40\x13\x71\xdb\x16\x3d\xe5\xe7\x6c\x02"
      "\x18\x62\xbe\x25\xa7\x62\x0b\x5d\xce\x7a\x70\x63\x61\x2d\x6f\x35\xea\x0c"
      "\x5a\x56\x39\x9f\x6e\x67\x1d\x5c\xfb\x39\xd5\x11\x03\x86\x89\x7b\x23\x05"
      "\x7b\x44\x39\x72\xa8\x34\x03\x2b\x1b\xf4\x88\xee\x76\x9c\xcf\x96\x34\x0f"
      "\x2c\x46\x84\x1d\x54\x9e\x36\x21\x48\x6d\xc3\xbe\x59\xd9\x72\xe3\xc4\x74"
      "\xd4\x0d\xcc\xdd\xb5\x8d\xb6\x69\x00\xf8\x64\x80\xaf\x02\x4d\x5c\xf9\x0c"
      "\x23\x78\xab\x3f\x7c\x12\x1e\x10\xf9\x7c\x70\x0c\xf0\x32\x86\xdd\x99\x12"
      "\xfa\x27\x8d\x95\x5b\xe3\x42\x17\xfe\x30\x2b\x59\x64\x05\xce\xbe\x1d\x48"
      "\x25\xc0\x53\x07\x9b\x92\x01\x06\x9d\xb2\x03\xc9\x6a\x8f\xf8\x0b\xbb\x49"
      "\xb0\x89\x4c\x6f\x42\x4d\x7f\xbf\x79\x65\xaa\x39\xda\xa2\xd4\x17\x95\xad"
      "\x92\x21\x79\x89\x8a\x9d\xe6\x4f\x64\x2f\x7b\x02\xb1\x17\x0a\x57\xe3\x66"
      "\x3d\x00\x67\xf9\x9f\xc6\xc0\xe0\xb4\xa8\x51\xb4\xea\x5a\xe1\x2c\x6a\xa0"
      "\xdb\xc8\x1f\xfe\xb6\x65\x6e\x10\x37\xc2\x69\x1d\xf4\x1b\xff\x81\x3a\x09"
      "\x5e\xbd\x95\xde\xd4\x41\x8c\xfd\x0e\x71\x7a\x9f\x3c\x52\x69\x5f\x8a\x1f"
      "\x2e\x9e\x57\xbc\xbb\x65\xaa\x5c\x40\x76\x0d\x1c\xcb\xf2\x81\x0f\x53\xcc"
      "\x9a\x63\xcf\x2b\x62\x2a\x01\xcf\x4d\x18\xf5\x8f\x78\xce\xd2\xd4\x26\x12"
      "\x30\xd6\xf1\xf7\x90\x9d\x20\xc2\x01\x4b\xe0\xc5\xe1\x59\x1c\xef\xcc\xfc"
      "\xca\xd0\xf3\x2c\x03\x95\xec\x2b\x7a\x4f\x90\xc0\x26\x32\x77\xf3\x8f\xf7"
      "\xff\x43\x7c\x81\x3b\x86\x99\xd4\x93\xde\x67\xd9\x07\x76\xb0\x52\x27\x8d"
      "\x64\xa7\x9c\x7b\x94\x57\x3b\x58\x1f\x20\x52\x23\x29\x76\x89\x81\x01\x45"
      "\xff\x97\xc7\xd9\xd1\xf7\x64\x7d\x31\x77\xa2\xa1\xdb\xe9\x91\x19\xdc\x00"
      "\x8b\x51\xde\x56\x8f\xe9\x55\xc7\x81\x48\xde\x32\x21\xde\x6c\x98\xcc\x39"
      "\xb6\x96\x9e\xb1\x66\x38\xb2\x33\x30\x83\x86\x76\x06\x1e\x3f\xbc\x97\x1c"
      "\x5d\x5f\xb7\xe4\x12\xc2\x84\xd2\x28\xda\x05\x4c\x28\x55\x56\x7a\x2a\x96"
      "\xdd\x1a\x0d\x83\x09\x31\x9e\x14\x02\xf5\xf5\xa9\x04\x9b\x3b\xbf\x88\x8c"
      "\x53\x58\x9f\x34\xba\xf0\x09\x96\x48\x77\x41\xaa\x87\x06\xed\x35\xfe\x72"
      "\xd0\xb7\x35\x8b\x7f\xed\x80\x8c\xe7\xd2\xb9\x4c\xfa\x4d\x71\x8e\x93\xa8"
      "\x47\x12\x04\x9e\xd1\xe5\x3e\x65\xc2\xe2\x27\xa1\x66\xe9\x50\x74\xa5\x87"
      "\x06\x6c\x3f\x5d\x97\xab\x2b\x47\x81\x6c\xb7\xc5\x2b\x26\x6c\x94\x83\x16"
      "\x05\xfe\xa4\x7b\xc6\x2a\xa7\xc2\xf1\x68\x52\xaa\x14\x40\xb6\x42\xcf\x80"
      "\x9d\x6c\x1a\x11\x0a\xd3\xf4\x43\x67\x5a\x21\x9f\xf5\x96\xb2\x65\xab\x70"
      "\xc2\x1e\x1b\xab\x5d\x92\x09\xa9\x75\x97\x2d\xde\xda\xe9\xe0\x29\x94\x72"
      "\x1e\xea\x0b\xd3\x18\xc3\xac\x15\x3d\x63\x44\x3d\x6e\x3c\x0a\xf7\x00\x63"
      "\x10\x66\x29\x2f\x37\x81\xa1\x2d\x8f\x81\x2d\x10\x26\x28\x71\x72\x6e\x73"
      "\x73\x11\x63\xa5\xba\x0e\x56\xaa\x94\x21\x3f\x8c\x22\x6a\x86\xe7\x9b\x30"
      "\xe4\x0d\xa3\xcd\x12\x37\x3c\x33\x25\xd4\xb2\x1c\x83\xfc\xf8\x15\x89\x00"
      "\x80\xfa\x5c\x86\x8b\xb4\x68\x92\x70\xde\x43\xa5\x4d\x59\x77\x5b\x51\x14"
      "\xcc\xcf\x64\xd1\x8a\x26\xa0\xdc\xd4\x82\xca\x64\x92\xb3\xbc\x8b\x3c\x94"
      "\x5a\xce\x3e\xa6\xcc\x7b\xa3\x6c\x9b\xf7\xb6\xec\x4f\xda\x5a\xf4\x33\x0b"
      "\x08\xe1\x00\xde\x12\x72\x04\xb9\x33\xeb\x71\xfb\x02\x19\x21\xb0\xdb\xb7"
      "\xd1\xa6\x9c\x4c\xd4\x10\x40\xaa\x23\x90\x74\xe1\x89\x3c\xd2\x15\x9b\x49"
      "\x18\x40\x89\xce\xe8\x61\xe4\xe3\xf3\x4f\x4a\x05\x1a\xde\x0c\x4e\x55\x0c"
      "\x21\x94\xae\xe4\xc5\x07\x68\x3b\x74\x37\x70\x7d\x2c\xd8\x2a\xb2\x58\x92"
      "\xc2\x25\x46\x80\xd2\xea\x96\x72\x4e\x00\x5e\x16\x19\x49\xe6\xa5\xb0\xf7"
      "\x32\x69\x66\x69\x2e\x20\x7e\x77\x41\xb1\x19\xd4\xce\x90\xab\x7c\x3a\x56"
      "\x09\x8f\x61\x51\x14\x6b\xc5\xf4\x2d\xd6\x70\xd1\xfa\xfc\x68\x15\x95\x68"
      "\xcb\x1a\x81\xf9\xe1\x84\xc9\x51\xad\x09\x2b\xb0\xa0\x8c\x9c\x6f\xc7\xb5"
      "\x6a\x20\x70\x2c\x5b\x86\x72\x54\x16\x16\x74\xc8\x83\x72\xdc\xc6\xf9\xc5"
      "\x5d\xab\xbf\x50\xdc\x37\x4c\x03\x73\x23\x3c\xe3\x4d\xc8\x74\xf6\x53\xd4"
      "\x9c\x6f\x42\xc0\x2a\x59\x34\x33\xdc\x29\x7f\xba\x33\x70\x62\x13\x62\x16"
      "\x84\xbb\xf2\xe5\x58\x83\x93\xf3\x91\x59\x22\xf2\xd0\x7c\xec\x77\x28\xfb"
      "\xaa\x75\x2c\xfa\x49\xed\x46\xb2\xe6\xe0\x86\x00\xd6\x42\xe8\xca\xdb\xb2"
      "\xe1\xea\xe9\x60\xae\xcb\xab\x92\x9d\x50\x2f\xfc\x0f\x2f\x0c\x2f\x2e\x50"
      "\xe2\x78\xec\x7a\xee\x90\xca\xf5\xf1\x2c\xed\x9c\x3d\xb6\x84\x16\xb3\x2c"
      "\xec\x97\x88\x39\x58\xa2\x80\xee\x5c\x95\xd2\x10\xeb\xfc\x4d\xe0\x70\xb0"
      "\x9a\x30\x7f\x53\xee\x7a\x7f\xf6\xee\xac\x43\xef\xc9\x2a\x77\x27\xee\x38"
      "\x4e\x4d\x7d\x48\xc3\x10\x45\xef\x7f\x24\xa0\x8b\x6b\xa8\x64\xd2\xc3\x1f"
      "\x10\xd9\xdf\x49\x6e\xb4\x31\xed\x5d\xbe\xb5\x9e\xce\x4e\x97\x3a\x5c\x34"
      "\xe4\xf7\xa7\xc4\x1a\x3c\x29\xd7\x86\xd1\x31\xd6\x8c\xb2\x6e\xb9\x7d\xa1"
      "\x95\xb9\x8e\x7a\xe0\xa8\x14\xb6\x26\x6c\xac\x3f\x24\xea\x8c\x04\x1b\x2a"
      "\x69\xba\xe2\xf0\x61\x1a\x9d\x0f\xb9\x36\x83\xc0\x7e\xb8\xe3\xfe\x9b\x39"
      "\xfa\xd4\xce\x8d\xfd\xa9\x9b\x47\x6e\x19\xc5\x53\x90\x31\x1c\xdf\xe9\x33"
      "\x1b\xf3\xfc\xeb\x8d\x27\xf2\x87\xac\xdf\xe9\x99\x2d\x7a\xc7\x28\xee\xa2"
      "\xf1\xa0\x88\x4e\x13\x12\xfb\xf1\x77\x03\x46\x43\xa4\xda\x4f\x67\xd9\x7f"
      "\xca\x48\x18\x03\xca\x63\x60\x20\xf0\xe6\x99\x88\x9a\x42\xd8\x4f\x1b\xbf"
      "\xa6\x6b\xc0\xf9\x52\x20\xc4\x32\x6d\x74\x1c\x26\xcb\x41\xdc\xdc\x4f\x04"
      "\x3a\xd5\x10\xe8\xd9\x70\x37\xb8\x2a\x51\xaa\xe7\x43\x96\x66\xff\x72\x20"
      "\x4b\x7d\xdf\x70\x6b\x16\x71\xd4\xf4\x1c\x04\x0b\xc4\xa2\x0d\x73\xd4\xc7"
      "\x3d\x85\xfb\x77\x99\x2c\x06\x10\xfb\x50\xdc\x8d\x5d\xd1\xa9\xe0\x84\x96"
      "\x4e\xf0\x5f\x12\x6c\x3a\x56\xdc\x9f\x78\x03\xe5\x95\x52\x0a\x48\x82\xd9"
      "\x94\xd9\x43\x0b\x0f\xc2\xf8\x3c\xd4\xf6\x51\x40\xc1\x10\xbe\x2e\xfb\x02"
      "\x4e\x8e\x15\x23\xcf\x47\xad\x56\xc3\x02\x7a\xbc\xb8\xfb\x59\xa9\x4f\x31"
      "\x4d\x66\x8b\xab\xd1\x96\x35\xbe\x8e\x8c\xe2\x90\xca\xbd\x80\xce\x2d\xaa"
      "\xad\x60\xed\x17\xe2\x18\xc0\x1c\x55\x4a\x87\x61\x89\x03\x7c\x29\xff\x42"
      "\xc9\xbe\x4b\xb8\x4a\x02\x5c\x6a\xe4\xe6\xcc\xc1\xb5\xc5\x7f\xae\xb8\x74"
      "\x65\xff\x0e\x5d\x09\x2d\xf6\x7b\xf8\xaa\x5d\x0e\x43\xa4",
      3812);
  *(uint64_t*)0x20000008 = 0xee4;
  syscall(__NR_writev, r[1], 0x20000000, 1);
  *(uint64_t*)0x20d83ff8 = 0xe3;
  syscall(__NR_sendfile, r[0], r[1], 0x20d83ff8, 0x8000fffffffe);
  return 0;
}
