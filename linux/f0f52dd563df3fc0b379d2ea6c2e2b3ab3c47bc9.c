// WARNING in skb_try_coalesce
// https://syzkaller.appspot.com/bug?id=f0f52dd563df3fc0b379d2ea6c2e2b3ab3c47bc9
// status:open
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
  }

struct csum_inet {
  uint32_t acc;
};

static void csum_inet_init(struct csum_inet* csum)
{
  csum->acc = 0;
}

static void csum_inet_update(struct csum_inet* csum, const uint8_t* data,
                             size_t length)
{
  if (length == 0)
    return;

  size_t i;
  for (i = 0; i < length - 1; i += 2)
    csum->acc += *(uint16_t*)&data[i];

  if (length & 1)
    csum->acc += (uint16_t)data[length - 1];

  while (csum->acc > 0xffff)
    csum->acc = (csum->acc & 0xffff) + (csum->acc >> 16);
}

static uint16_t csum_inet_digest(struct csum_inet* csum)
{
  return ~csum->acc;
}

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};
void loop()
{
  long res;
  res = syscall(__NR_socket, 0xa, 0x400000000001, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_dup, r[0]);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x20fa0fe4 = 0xa;
  *(uint16_t*)0x20fa0fe6 = htobe16(0x4e20);
  *(uint32_t*)0x20fa0fe8 = 0;
  *(uint64_t*)0x20fa0fec = htobe64(0);
  *(uint64_t*)0x20fa0ff4 = htobe64(1);
  *(uint32_t*)0x20fa0ffc = 0;
  syscall(__NR_bind, r[0], 0x20fa0fe4, 0x1c);
  *(uint16_t*)0x204a5fe4 = 0xa;
  *(uint16_t*)0x204a5fe6 = htobe16(0x4e20);
  *(uint32_t*)0x204a5fe8 = 0;
  *(uint64_t*)0x204a5fec = htobe64(0);
  *(uint64_t*)0x204a5ff4 = htobe64(1);
  *(uint32_t*)0x204a5ffc = 0;
  syscall(__NR_sendto, r[0], 0x20f98000, 0xfffffffffffffff3, 0x2000000c,
          0x204a5fe4, 0x1c);
  memcpy((void*)0x20002000, "./bus", 6);
  res = syscall(__NR_open, 0x20002000, 0x141042, 0);
  if (res != -1)
    r[2] = res;
  syscall(__NR_ftruncate, r[2], 0x27ffe);
  *(uint8_t*)0x20000840 = 0x3a;
  *(uint8_t*)0x20000841 = 0x31;
  *(uint8_t*)0x20000842 = 0;
  *(uint8_t*)0x20000843 = 0;
  *(uint8_t*)0x20000844 = 0;
  *(uint8_t*)0x20000845 = 0;
  *(uint8_t*)0x20000846 = 0;
  *(uint8_t*)0x20000847 = 0;
  *(uint8_t*)0x20000848 = 2;
  *(uint8_t*)0x20000849 = 0x86;
  memcpy((void*)0x2000084a,
         "\x85\x1f\x16\xcf\x7a\x8b\x8c\x2a\xac\xe1\x0b\xe7\x3b\x15\x60\xf2\x08"
         "\x4c\x7c\xd9\x02\xb4\x26\xa6\x88\x65\x14\xae\x0b\x20\x84\x37\x8a\x4f"
         "\xb5\xa3\x89\x5c\xea\xfe\x76\xb5\xe8\x34\xbc\x5d\x61\xe3\xe7\xf0\xdf"
         "\x4f\x56\x72\x55\x94\x93\x29\xf4\x25\xf8\x59\x2a\x9d\x21\x1a\xe4\xe5"
         "\x22\x1e\x0b\x44\x0e\xf1\x7b\x07\x7e\x87\x4f\xa0\xe1\x3e\x04\xef\xdd"
         "\x0b\x9d\x0b\x25\x2b\x8b\xa6\xb8\xe0\xb7\xfd\x8f\x69\xed\xe4\xb1\xb0"
         "\xd1\x5a\x29\xe7\x13\x5d\xb9\x82\xa5\xa5\x39\xaf\x7d\x47\xb0\x3a\xe6"
         "\x26\x5e\x87\xa9\x85\x92\x1f\x85\xbf\xc6\xd5\xbd\x6b\xbe\xd5",
         134);
  *(uint8_t*)0x200008d0 = 7;
  *(uint8_t*)0x200008d1 = 0xc6;
  memcpy((void*)0x200008d2,
         "\xa2\x18\x55\x5d\x4a\x24\x2d\x16\xcf\x80\xcb\xfb\xa6\xeb\x86\x5d\xf5"
         "\xf3\x34\xa4\xff\x14\xd5\xc9\x6a\xc2\x9d\x85\x1d\x5c\x2b\xda\x51\xca"
         "\x35\xf8\x9a\x7c\xef\xbc\x3c\x19\x99\x79\xa3\x1c\xeb\x26\x8a\x12\x8a"
         "\x5e\xf1\x2e\x37\xd0\x0b\xf7\x1f\x81\xc0\x61\xbd\x0d\xef\x3e\x96\xd5"
         "\x8d\xa5\xee\x95\xbf\xc7\xaf\xa5\xa7\x81\x0a\xf3\xc3\x27\x73\x3a\x69"
         "\xa1\x80\x3f\x06\xa4\xb2\x88\xc2\x84\x75\x91\x7b\x70\xd4\x59\x2f\xa6"
         "\x4c\xdf\x77\xe2\xf3\xf2\xc2\x15\x7e\x05\x21\x5c\x3b\xb5\xf7\x1e\x5e"
         "\x79\x4b\x73\xb7\x1a\x43\xed\x66\xeb\xb6\xe8\xea\x09\xf7\x8e\xea\x77"
         "\x26\xbb\x1d\x78\x50\x40\x0f\x65\xbb\x56\xd2\x00\x2f\x98\xda\xd7\x97"
         "\xee\xee\xa4\x16\x7c\x59\x34\xa3\xcd\x80\xfa\xbc\x62\xf1\x25\xbc\x27"
         "\x92\x22\x9b\x0a\x29\x1c\x58\xf3\xa9\x0e\x79\x3f\xc3\x27\x3b\xec\x5a"
         "\x38\x3c\x78\xd4\xf4\x62\x92\x5e\x25\xbc\x0f",
         198);
  *(uint8_t*)0x20000998 = 1;
  *(uint8_t*)0x20000999 = 4;
  *(uint8_t*)0x2000099a = 0;
  *(uint8_t*)0x2000099b = 0;
  *(uint8_t*)0x2000099c = 0;
  *(uint8_t*)0x2000099d = 0;
  *(uint8_t*)0x2000099e = 7;
  *(uint8_t*)0x2000099f = 0x30;
  *(uint32_t*)0x200009a0 = htobe32(7);
  *(uint8_t*)0x200009a4 = 0xa;
  *(uint8_t*)0x200009a5 = -1;
  *(uint16_t*)0x200009a6 = 9;
  *(uint64_t*)0x200009a8 = 3;
  *(uint64_t*)0x200009b0 = 0x400;
  *(uint64_t*)0x200009b8 = 0xfffffffffffffe01;
  *(uint64_t*)0x200009c0 = 0xfffffffffffffff9;
  *(uint64_t*)0x200009c8 = 0x3ff;
  syscall(__NR_setsockopt, r[0], 0x29, 0x36, 0x20000840, 0x190);
  *(uint64_t*)0x2053cffc = 0;
  syscall(__NR_sendfile, r[1], r[2], 0x2053cffc, 0x8080000001);
  *(uint8_t*)0x20d51eed = 0xaa;
  *(uint8_t*)0x20d51eee = 0xaa;
  *(uint8_t*)0x20d51eef = 0xaa;
  *(uint8_t*)0x20d51ef0 = 0xaa;
  *(uint8_t*)0x20d51ef1 = 0xaa;
  *(uint8_t*)0x20d51ef2 = 0xaa;
  *(uint8_t*)0x20d51ef3 = 1;
  *(uint8_t*)0x20d51ef4 = 0x80;
  *(uint8_t*)0x20d51ef5 = 0xc2;
  *(uint8_t*)0x20d51ef6 = 0;
  *(uint8_t*)0x20d51ef7 = 0;
  *(uint8_t*)0x20d51ef8 = 0;
  *(uint16_t*)0x20d51ef9 = htobe16(0x800);
  STORE_BY_BITMASK(uint8_t, 0x20d51efb, 5, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20d51efb, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x20d51efc, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x20d51efc, 0, 2, 6);
  *(uint16_t*)0x20d51efd = htobe16(0x58);
  *(uint16_t*)0x20d51eff = htobe16(0);
  *(uint16_t*)0x20d51f01 = htobe16(0);
  *(uint8_t*)0x20d51f03 = 0;
  *(uint8_t*)0x20d51f04 = 0x2f;
  *(uint16_t*)0x20d51f05 = 0;
  *(uint8_t*)0x20d51f07 = 0xac;
  *(uint8_t*)0x20d51f08 = 0x14;
  *(uint8_t*)0x20d51f09 = -1;
  *(uint8_t*)0x20d51f0a = 0xaa;
  *(uint32_t*)0x20d51f0b = htobe32(-1);
  STORE_BY_BITMASK(uint16_t, 0x20d51f0f, 0, 0, 1);
  STORE_BY_BITMASK(uint16_t, 0x20d51f0f, 0, 1, 1);
  STORE_BY_BITMASK(uint16_t, 0x20d51f0f, 1, 2, 1);
  STORE_BY_BITMASK(uint16_t, 0x20d51f0f, 0, 3, 1);
  STORE_BY_BITMASK(uint16_t, 0x20d51f0f, 0xb, 4, 4);
  STORE_BY_BITMASK(uint16_t, 0x20d51f0f, 0, 8, 1);
  STORE_BY_BITMASK(uint16_t, 0x20d51f0f, 0, 9, 4);
  STORE_BY_BITMASK(uint16_t, 0x20d51f0f, 8, 13, 3);
  *(uint16_t*)0x20d51f11 = htobe16(0x880b);
  *(uint16_t*)0x20d51f13 = htobe16(0);
  *(uint16_t*)0x20d51f15 = htobe16(0);
  STORE_BY_BITMASK(uint16_t, 0x20d51f17, 0, 0, 1);
  STORE_BY_BITMASK(uint16_t, 0x20d51f17, 0, 1, 1);
  STORE_BY_BITMASK(uint16_t, 0x20d51f17, 0, 2, 1);
  STORE_BY_BITMASK(uint16_t, 0x20d51f17, 0, 3, 1);
  STORE_BY_BITMASK(uint16_t, 0x20d51f17, 0, 4, 9);
  STORE_BY_BITMASK(uint16_t, 0x20d51f17, 0, 13, 3);
  *(uint16_t*)0x20d51f19 = htobe16(0x800);
  STORE_BY_BITMASK(uint16_t, 0x20d51f1b, 0, 0, 1);
  STORE_BY_BITMASK(uint16_t, 0x20d51f1b, 0, 1, 1);
  STORE_BY_BITMASK(uint16_t, 0x20d51f1b, 0, 2, 1);
  STORE_BY_BITMASK(uint16_t, 0x20d51f1b, 0, 3, 1);
  STORE_BY_BITMASK(uint16_t, 0x20d51f1b, 0, 4, 9);
  STORE_BY_BITMASK(uint16_t, 0x20d51f1b, 0, 13, 3);
  *(uint16_t*)0x20d51f1d = htobe16(0x86dd);
  *(uint16_t*)0x20d51f1f = 0;
  *(uint16_t*)0x20d51f21 = htobe16(0);
  *(uint32_t*)0x20d51f23 = htobe32(0);
  STORE_BY_BITMASK(uint8_t, 0x20d51f27, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20d51f27, 0, 4, 4);
  *(uint8_t*)0x20d51f28 = 0;
  STORE_BY_BITMASK(uint8_t, 0x20d51f29, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x20d51f29, 0, 2, 1);
  STORE_BY_BITMASK(uint8_t, 0x20d51f29, 0, 3, 2);
  STORE_BY_BITMASK(uint8_t, 0x20d51f29, 0, 5, 3);
  *(uint8_t*)0x20d51f2a = 0;
  *(uint32_t*)0x20d51f2b = 0;
  *(uint32_t*)0x20d51f2f = htobe32(0);
  *(uint16_t*)0x20d51f33 = 0;
  *(uint16_t*)0x20d51f35 = htobe16(0);
  *(uint32_t*)0x20d51f37 = htobe32(0);
  STORE_BY_BITMASK(uint8_t, 0x20d51f3b, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20d51f3b, 0, 4, 4);
  *(uint8_t*)0x20d51f3c = 0;
  STORE_BY_BITMASK(uint8_t, 0x20d51f3d, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x20d51f3d, 0, 2, 1);
  STORE_BY_BITMASK(uint8_t, 0x20d51f3d, 0, 3, 2);
  STORE_BY_BITMASK(uint8_t, 0x20d51f3d, 0, 5, 3);
  *(uint8_t*)0x20d51f3e = 0;
  *(uint32_t*)0x20d51f3f = 0;
  *(uint32_t*)0x20d51f43 = htobe32(0);
  *(uint16_t*)0x20d51f47 = htobe16(0);
  STORE_BY_BITMASK(uint8_t, 0x20d51f49, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x20d51f49, 0, 2, 5);
  STORE_BY_BITMASK(uint8_t, 0x20d51f49, 0, 7, 1);
  STORE_BY_BITMASK(uint8_t, 0x20d51f4a, 0, 0, 1);
  STORE_BY_BITMASK(uint8_t, 0x20d51f4a, 0, 1, 2);
  STORE_BY_BITMASK(uint8_t, 0x20d51f4a, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, 0x20d51f4a, 0, 4, 1);
  *(uint16_t*)0x20d51f4b = 0;
  *(uint16_t*)0x20d51f4d = htobe16(0);
  *(uint32_t*)0x20d51f4f = htobe32(0);
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x20d51efb, 20);
  *(uint16_t*)0x20d51f05 = csum_inet_digest(&csum_1);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
