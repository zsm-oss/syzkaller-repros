// KASAN: null-ptr-deref Write in choke_reset
// https://syzkaller.appspot.com/bug?id=8805bfe94cd9d0b6565c
// status:6
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

uint64_t r[6] = {0xffffffffffffffff, 0x0, 0xffffffffffffffff,
                 0xffffffffffffffff, 0x0, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x11ul, 0x800000003ul, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000080 = 0x11;
  memcpy((void*)0x20000082,
         "\x00\x00\x01\x00\x00\x00\x00\x00\x08\x00\x44\x94\x4e\xeb\xa7\x1a\x49"
         "\x76\xe2\x52\x92\x2c\xb1\x8f\x6e\x2e\x2a\xba\x00\x00\x00\x01\x2e\x0b"
         "\x38\x36\x00\x54\x04\xb0\xe0\x30\x1a\x4c\xe8\x75\xf2\xe3\xff\x5f\x16"
         "\x3e\xe3\x40\xb7\x67\x95\x00\x80\x00\x00\x00\x00\x00\x00\x01\x01\x01"
         "\x3c\x58\x11\x03\x9e\x15\x77\x50\x27\xec\xce\x66\xfd\x79\x2b\xbf\x0e"
         "\x5b\xf5\xff\x1b\x08\x16\xf3\xf6\xdb\x1c\x00\x01\x00\x00\x00\x00\x00"
         "\x00\x00\x49\x74\x00\x00\x00\x00\x00\x00\x00\x06\xad\x8e\x5e\xcc\x32"
         "\x6d\x3a\x09\xff\xc2\xc6\x54",
         126);
  syscall(__NR_bind, r[0], 0x20000080ul, 0x80ul);
  *(uint32_t*)0x20000000 = 0x2f;
  res = syscall(__NR_getsockname, r[0], 0x20000040ul, 0x20000000ul);
  if (res != -1)
    r[1] = *(uint32_t*)0x20000044;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x200007c0 = 0;
  *(uint32_t*)0x200007c8 = 0;
  *(uint64_t*)0x200007d0 = 0x20000780;
  *(uint64_t*)0x20000780 = 0x20000800;
  *(uint32_t*)0x20000800 = 0x154;
  *(uint16_t*)0x20000804 = 0x24;
  *(uint16_t*)0x20000806 = 0xd29;
  *(uint32_t*)0x20000808 = 0;
  *(uint32_t*)0x2000080c = 0;
  *(uint8_t*)0x20000810 = 0;
  *(uint8_t*)0x20000811 = 0;
  *(uint16_t*)0x20000812 = 0;
  *(uint32_t*)0x20000814 = r[1];
  *(uint16_t*)0x20000818 = 0;
  *(uint16_t*)0x2000081a = 0;
  *(uint16_t*)0x2000081c = -1;
  *(uint16_t*)0x2000081e = -1;
  *(uint16_t*)0x20000820 = 0;
  *(uint16_t*)0x20000822 = 0;
  *(uint16_t*)0x20000824 = 0xa;
  *(uint16_t*)0x20000826 = 1;
  memcpy((void*)0x20000828, "choke\000", 6);
  *(uint16_t*)0x20000830 = 0x124;
  *(uint16_t*)0x20000832 = 2;
  *(uint16_t*)0x20000834 = 8;
  *(uint16_t*)0x20000836 = 3;
  *(uint32_t*)0x20000838 = 0;
  *(uint16_t*)0x2000083c = 0x14;
  *(uint16_t*)0x2000083e = 1;
  *(uint32_t*)0x20000840 = 0;
  *(uint32_t*)0x20000844 = 0;
  *(uint32_t*)0x20000848 = 0;
  *(uint8_t*)0x2000084c = 0;
  *(uint8_t*)0x2000084d = 0;
  *(uint8_t*)0x2000084e = 0;
  *(uint8_t*)0x2000084f = 0;
  *(uint16_t*)0x20000850 = 0x104;
  *(uint16_t*)0x20000852 = 2;
  memcpy((void*)0x20000854,
         "\x45\xeb\xfd\x49\xff\xc3\xe6\x6e\xe8\xab\x66\x97\x3a\xc0\x55\xca\xdb"
         "\x72\x70\xe7\xc2\x96\x86\xc3\xf8\x9e\xbd\xde\xc7\xa9\x9e\xe6\x86\x57"
         "\x65\x86\xb4\x7f\xdd\x9b\xb1\xbc\x75\x9b\x2a\x89\xc6\xf8\xc3\xa8\x9b"
         "\x39\x7c\x8e\x85\x41\xde\x4e\x45\x78\x50\x10\xf2\x45\x9b\xca\x7e\x54"
         "\x43\x28\xc9\x2e\x20\x4d\x15\x29\x32\x2e\xcf\x1f\xde\x00\x7b\xbd\xfd"
         "\x62\xda\x2d\x07\xcd\x6a\x82\xec\x7a\xca\xe0\x4f\xd1\xe2\xd2\x26\x66"
         "\x0f\xfd\xba\xcf\x32\xc0\x66\xd1\x01\x39\x9d\xf5\x09\x4f\xa1\xf6\x79"
         "\xf8\x53\x7a\xd6\xb5\x5c\xa4\x44\x3c\xcd\x4a\x1e\x73\xcd\x50\x87\x12"
         "\x0f\x99\xb4\x94\x41\x50\x8e\x04\xc8\xb1\x4f\xf0\x3f\xd6\x3b\x19\x63"
         "\x05\xcc\x4c\x25\xc3\x3f\x57\xdb\x71\x28\x0b\xd8\x0c\x2b\x75\x3d\xe2"
         "\x48\x00\x36\x11\x77\xf0\xff\x10\xc4\x0e\xbb\x42\x11\xa1\xb8\x0c\xe7"
         "\x62\x46\x78\x83\x0d\xeb\xb9\x37\x10\xc1\xd8\xe9\x64\x1c\x58\x01\xda"
         "\xf2\xaf\xdf\xfa\x00\xca\x1b\x8f\x36\x1f\xc6\xdd\x05\xf1\x0d\x68\x43"
         "\xce\x5e\x55\x0d\xa3\x9f\x9c\x7c\xe8\xac\x40\xda\x55\x53\xee\x8a\xae"
         "\x38\xc8\x2d\xf7\x51\x7a\x11\x83\x75\x75\xd5\xd4\xfb\x5d\xe4\x25\x8f"
         "\x70",
         256);
  *(uint64_t*)0x20000788 = 0x154;
  *(uint64_t*)0x200007d8 = 1;
  *(uint64_t*)0x200007e0 = 0;
  *(uint64_t*)0x200007e8 = 0;
  *(uint32_t*)0x200007f0 = 0;
  syscall(__NR_sendmsg, r[2], 0x200007c0ul, 0ul);
  res = syscall(__NR_socket, 0x11ul, 0x800000003ul, 0);
  if (res != -1)
    r[3] = res;
  *(uint16_t*)0x20000080 = 0x11;
  memcpy((void*)0x20000082,
         "\x00\x00\x01\x00\x00\x00\x00\x00\x08\x00\x44\x94\x4e\xeb\xa7\x1a\x49"
         "\x76\xe2\x52\x92\x2c\xb1\x8f\x6e\x2e\x2a\xba\x00\x00\x00\x01\x2e\x0b"
         "\x38\x36\x00\x54\x04\xb0\xe0\x30\x1a\x4c\xe8\x75\xf2\xe3\xff\x5f\x16"
         "\x3e\xe3\x40\xb7\x67\x95\x00\x80\x00\x00\x00\x00\x00\x00\x01\x01\x01"
         "\x3c\x58\x11\x03\x9e\x15\x77\x50\x27\xec\xce\x66\xfd\x79\x2b\xbf\x0e"
         "\x5b\xf5\xff\x1b\x08\x16\xf3\xf6\xdb\x1c\x00\x01\x00\x00\x00\x00\x00"
         "\x00\x00\x49\x74\x00\x00\x00\x00\x00\x00\x00\x06\xad\x8e\x5e\xcc\x32"
         "\x6d\x3a\x09\xff\xc2\xc6\x54",
         126);
  syscall(__NR_bind, r[3], 0x20000080ul, 0x80ul);
  *(uint32_t*)0x20000000 = 0x14;
  res = syscall(__NR_getsockname, r[3], 0x200003c0ul, 0x20000000ul);
  if (res != -1)
    r[4] = *(uint32_t*)0x200003c4;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[5] = res;
  *(uint64_t*)0x200007c0 = 0;
  *(uint32_t*)0x200007c8 = 0;
  *(uint64_t*)0x200007d0 = 0x20000780;
  *(uint64_t*)0x20000780 = 0x20000400;
  *(uint32_t*)0x20000400 = 0x40;
  *(uint16_t*)0x20000404 = 0x24;
  *(uint16_t*)0x20000406 = 0xf0b;
  *(uint32_t*)0x20000408 = 0;
  *(uint32_t*)0x2000040c = 0;
  *(uint8_t*)0x20000410 = 0;
  *(uint8_t*)0x20000411 = 0;
  *(uint16_t*)0x20000412 = 0;
  *(uint32_t*)0x20000414 = r[4];
  *(uint16_t*)0x20000418 = 0;
  *(uint16_t*)0x2000041a = 0;
  *(uint16_t*)0x2000041c = -1;
  *(uint16_t*)0x2000041e = -1;
  *(uint16_t*)0x20000420 = 0;
  *(uint16_t*)0x20000422 = 0;
  *(uint16_t*)0x20000424 = 7;
  *(uint16_t*)0x20000426 = 1;
  memcpy((void*)0x20000428, "fq\000", 3);
  *(uint16_t*)0x2000042c = 0x14;
  *(uint16_t*)0x2000042e = 2;
  *(uint16_t*)0x20000430 = 8;
  *(uint16_t*)0x20000432 = 1;
  *(uint32_t*)0x20000434 = 0;
  *(uint16_t*)0x20000438 = 8;
  *(uint16_t*)0x2000043a = 9;
  *(uint32_t*)0x2000043c = 0;
  *(uint64_t*)0x20000788 = 0x40;
  *(uint64_t*)0x200007d8 = 1;
  *(uint64_t*)0x200007e0 = 0;
  *(uint64_t*)0x200007e8 = 0;
  *(uint32_t*)0x200007f0 = 0;
  syscall(__NR_sendmsg, r[5], 0x200007c0ul, 0ul);
  return 0;
}
