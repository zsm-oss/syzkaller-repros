// kernel BUG at net/core/dev.c:LINE!
// https://syzkaller.appspot.com/bug?id=7010af67ced6105e5ab6
// status:0
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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000080, "/dev/net/tun\000", 13);
  res =
      syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000080ul, 0x88002ul, 0ul);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0xaul, 1ul, 0);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_dup2, r[1], r[1]);
  if (res != -1)
    r[2] = res;
  memcpy((void*)0x200009c0, "mangle\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000",
         32);
  *(uint32_t*)0x200009e0 = 0x1f;
  *(uint32_t*)0x200009e4 = 6;
  *(uint32_t*)0x200009e8 = 0x3b0;
  *(uint32_t*)0x200009ec = 0;
  *(uint32_t*)0x200009f0 = 0;
  *(uint32_t*)0x200009f4 = 0x280;
  *(uint32_t*)0x200009f8 = 0x280;
  *(uint32_t*)0x200009fc = 0;
  *(uint32_t*)0x20000a00 = 0x318;
  *(uint32_t*)0x20000a04 = 0x318;
  *(uint32_t*)0x20000a08 = 0x318;
  *(uint32_t*)0x20000a0c = 0x318;
  *(uint32_t*)0x20000a10 = 0x318;
  *(uint32_t*)0x20000a14 = 6;
  *(uint64_t*)0x20000a18 = 0;
  *(uint8_t*)0x20000a20 = 0;
  *(uint8_t*)0x20000a21 = 0;
  *(uint8_t*)0x20000a22 = 0;
  *(uint8_t*)0x20000a23 = 0;
  *(uint8_t*)0x20000a24 = 0;
  *(uint8_t*)0x20000a25 = 0;
  *(uint8_t*)0x20000a26 = 0;
  *(uint8_t*)0x20000a27 = 0;
  *(uint8_t*)0x20000a28 = 0;
  *(uint8_t*)0x20000a29 = 0;
  *(uint8_t*)0x20000a2a = 0;
  *(uint8_t*)0x20000a2b = 0;
  *(uint8_t*)0x20000a2c = 0;
  *(uint8_t*)0x20000a2d = 0;
  *(uint8_t*)0x20000a2e = 0;
  *(uint8_t*)0x20000a2f = 0;
  *(uint8_t*)0x20000a30 = 0;
  *(uint8_t*)0x20000a31 = 0;
  *(uint8_t*)0x20000a32 = 0;
  *(uint8_t*)0x20000a33 = 0;
  *(uint8_t*)0x20000a34 = 0;
  *(uint8_t*)0x20000a35 = 0;
  *(uint8_t*)0x20000a36 = 0;
  *(uint8_t*)0x20000a37 = 0;
  *(uint8_t*)0x20000a38 = 0;
  *(uint8_t*)0x20000a39 = 0;
  *(uint8_t*)0x20000a3a = 0;
  *(uint8_t*)0x20000a3b = 0;
  *(uint8_t*)0x20000a3c = 0;
  *(uint8_t*)0x20000a3d = 0;
  *(uint8_t*)0x20000a3e = 0;
  *(uint8_t*)0x20000a3f = 0;
  *(uint8_t*)0x20000a40 = 0;
  *(uint8_t*)0x20000a41 = 0;
  *(uint8_t*)0x20000a42 = 0;
  *(uint8_t*)0x20000a43 = 0;
  *(uint8_t*)0x20000a44 = 0;
  *(uint8_t*)0x20000a45 = 0;
  *(uint8_t*)0x20000a46 = 0;
  *(uint8_t*)0x20000a47 = 0;
  *(uint8_t*)0x20000a48 = 0;
  *(uint8_t*)0x20000a49 = 0;
  *(uint8_t*)0x20000a4a = 0;
  *(uint8_t*)0x20000a4b = 0;
  *(uint8_t*)0x20000a4c = 0;
  *(uint8_t*)0x20000a4d = 0;
  *(uint8_t*)0x20000a4e = 0;
  *(uint8_t*)0x20000a4f = 0;
  *(uint8_t*)0x20000a50 = 0;
  *(uint8_t*)0x20000a51 = 0;
  *(uint8_t*)0x20000a52 = 0;
  *(uint8_t*)0x20000a53 = 0;
  *(uint8_t*)0x20000a54 = 0;
  *(uint8_t*)0x20000a55 = 0;
  *(uint8_t*)0x20000a56 = 0;
  *(uint8_t*)0x20000a57 = 0;
  *(uint8_t*)0x20000a58 = 0;
  *(uint8_t*)0x20000a59 = 0;
  *(uint8_t*)0x20000a5a = 0;
  *(uint8_t*)0x20000a5b = 0;
  *(uint8_t*)0x20000a5c = 0;
  *(uint8_t*)0x20000a5d = 0;
  *(uint8_t*)0x20000a5e = 0;
  *(uint8_t*)0x20000a5f = 0;
  *(uint8_t*)0x20000a60 = 0;
  *(uint8_t*)0x20000a61 = 0;
  *(uint8_t*)0x20000a62 = 0;
  *(uint8_t*)0x20000a63 = 0;
  *(uint8_t*)0x20000a64 = 0;
  *(uint8_t*)0x20000a65 = 0;
  *(uint8_t*)0x20000a66 = 0;
  *(uint8_t*)0x20000a67 = 0;
  *(uint8_t*)0x20000a68 = 0;
  *(uint8_t*)0x20000a69 = 0;
  *(uint8_t*)0x20000a6a = 0;
  *(uint8_t*)0x20000a6b = 0;
  *(uint8_t*)0x20000a6c = 0;
  *(uint8_t*)0x20000a6d = 0;
  *(uint8_t*)0x20000a6e = 0;
  *(uint8_t*)0x20000a6f = 0;
  *(uint8_t*)0x20000a70 = 0;
  *(uint8_t*)0x20000a71 = 0;
  *(uint8_t*)0x20000a72 = 0;
  *(uint8_t*)0x20000a73 = 0;
  *(uint32_t*)0x20000a74 = 0;
  *(uint16_t*)0x20000a78 = 0x70;
  *(uint16_t*)0x20000a7a = 0x98;
  *(uint32_t*)0x20000a7c = 0;
  *(uint64_t*)0x20000a80 = 0;
  *(uint64_t*)0x20000a88 = 0;
  *(uint16_t*)0x20000a90 = 0x28;
  memcpy((void*)0x20000a92, "TTL\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000",
         29);
  *(uint8_t*)0x20000aaf = 0;
  *(uint8_t*)0x20000ab0 = 0;
  *(uint8_t*)0x20000ab1 = 0;
  *(uint8_t*)0x20000ab8 = 0;
  *(uint8_t*)0x20000ab9 = 0;
  *(uint8_t*)0x20000aba = 0;
  *(uint8_t*)0x20000abb = 0;
  *(uint8_t*)0x20000abc = 0;
  *(uint8_t*)0x20000abd = 0;
  *(uint8_t*)0x20000abe = 0;
  *(uint8_t*)0x20000abf = 0;
  *(uint8_t*)0x20000ac0 = 0;
  *(uint8_t*)0x20000ac1 = 0;
  *(uint8_t*)0x20000ac2 = 0;
  *(uint8_t*)0x20000ac3 = 0;
  *(uint8_t*)0x20000ac4 = 0;
  *(uint8_t*)0x20000ac5 = 0;
  *(uint8_t*)0x20000ac6 = 0;
  *(uint8_t*)0x20000ac7 = 0;
  *(uint8_t*)0x20000ac8 = 0;
  *(uint8_t*)0x20000ac9 = 0;
  *(uint8_t*)0x20000aca = 0;
  *(uint8_t*)0x20000acb = 0;
  *(uint8_t*)0x20000acc = 0;
  *(uint8_t*)0x20000acd = 0;
  *(uint8_t*)0x20000ace = 0;
  *(uint8_t*)0x20000acf = 0;
  *(uint8_t*)0x20000ad0 = 0;
  *(uint8_t*)0x20000ad1 = 0;
  *(uint8_t*)0x20000ad2 = 0;
  *(uint8_t*)0x20000ad3 = 0;
  *(uint8_t*)0x20000ad4 = 0;
  *(uint8_t*)0x20000ad5 = 0;
  *(uint8_t*)0x20000ad6 = 0;
  *(uint8_t*)0x20000ad7 = 0;
  *(uint8_t*)0x20000ad8 = 0;
  *(uint8_t*)0x20000ad9 = 0;
  *(uint8_t*)0x20000ada = 0;
  *(uint8_t*)0x20000adb = 0;
  *(uint8_t*)0x20000adc = 0;
  *(uint8_t*)0x20000add = 0;
  *(uint8_t*)0x20000ade = 0;
  *(uint8_t*)0x20000adf = 0;
  *(uint8_t*)0x20000ae0 = 0;
  *(uint8_t*)0x20000ae1 = 0;
  *(uint8_t*)0x20000ae2 = 0;
  *(uint8_t*)0x20000ae3 = 0;
  *(uint8_t*)0x20000ae4 = 0;
  *(uint8_t*)0x20000ae5 = 0;
  *(uint8_t*)0x20000ae6 = 0;
  *(uint8_t*)0x20000ae7 = 0;
  *(uint8_t*)0x20000ae8 = 0;
  *(uint8_t*)0x20000ae9 = 0;
  *(uint8_t*)0x20000aea = 0;
  *(uint8_t*)0x20000aeb = 0;
  *(uint8_t*)0x20000aec = 0;
  *(uint8_t*)0x20000aed = 0;
  *(uint8_t*)0x20000aee = 0;
  *(uint8_t*)0x20000aef = 0;
  *(uint8_t*)0x20000af0 = 0;
  *(uint8_t*)0x20000af1 = 0;
  *(uint8_t*)0x20000af2 = 0;
  *(uint8_t*)0x20000af3 = 0;
  *(uint8_t*)0x20000af4 = 0;
  *(uint8_t*)0x20000af5 = 0;
  *(uint8_t*)0x20000af6 = 0;
  *(uint8_t*)0x20000af7 = 0;
  *(uint8_t*)0x20000af8 = 0;
  *(uint8_t*)0x20000af9 = 0;
  *(uint8_t*)0x20000afa = 0;
  *(uint8_t*)0x20000afb = 0;
  *(uint8_t*)0x20000afc = 0;
  *(uint8_t*)0x20000afd = 0;
  *(uint8_t*)0x20000afe = 0;
  *(uint8_t*)0x20000aff = 0;
  *(uint8_t*)0x20000b00 = 0;
  *(uint8_t*)0x20000b01 = 0;
  *(uint8_t*)0x20000b02 = 0;
  *(uint8_t*)0x20000b03 = 0;
  *(uint8_t*)0x20000b04 = 0;
  *(uint8_t*)0x20000b05 = 0;
  *(uint8_t*)0x20000b06 = 0;
  *(uint8_t*)0x20000b07 = 0;
  *(uint8_t*)0x20000b08 = 0;
  *(uint8_t*)0x20000b09 = 0;
  *(uint8_t*)0x20000b0a = 0;
  *(uint8_t*)0x20000b0b = 0;
  *(uint32_t*)0x20000b0c = 0;
  *(uint16_t*)0x20000b10 = 0x70;
  *(uint16_t*)0x20000b12 = 0x98;
  *(uint32_t*)0x20000b14 = 0;
  *(uint64_t*)0x20000b18 = 0;
  *(uint64_t*)0x20000b20 = 0;
  *(uint16_t*)0x20000b28 = 0x28;
  memcpy((void*)0x20000b2a, "CHECKSUM\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000",
         29);
  *(uint8_t*)0x20000b47 = 0;
  *(uint8_t*)0x20000b48 = 1;
  *(uint32_t*)0x20000b50 = htobe32(0xe0000002);
  *(uint8_t*)0x20000b54 = 0xac;
  *(uint8_t*)0x20000b55 = 0x14;
  *(uint8_t*)0x20000b56 = 0x14;
  *(uint8_t*)0x20000b57 = 0xaa;
  *(uint32_t*)0x20000b58 = htobe32(0);
  *(uint32_t*)0x20000b5c = htobe32(0);
  memcpy((void*)0x20000b60,
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000",
         16);
  memcpy((void*)0x20000b70, "erspan0\000\000\000\000\000\000\000\000\000", 16);
  *(uint8_t*)0x20000b80 = 0;
  *(uint8_t*)0x20000b90 = 0;
  *(uint16_t*)0x20000ba0 = 0;
  *(uint8_t*)0x20000ba2 = 0;
  *(uint8_t*)0x20000ba3 = 0;
  *(uint32_t*)0x20000ba4 = 0;
  *(uint16_t*)0x20000ba8 = 0x70;
  *(uint16_t*)0x20000baa = 0xb8;
  *(uint32_t*)0x20000bac = 0;
  *(uint64_t*)0x20000bb0 = 0;
  *(uint64_t*)0x20000bb8 = 0;
  *(uint16_t*)0x20000bc0 = 0x48;
  memcpy((void*)0x20000bc2, "TEE\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000",
         29);
  *(uint8_t*)0x20000bdf = 1;
  *(uint8_t*)0x20000be0 = -1;
  *(uint8_t*)0x20000be1 = 2;
  *(uint8_t*)0x20000be2 = 0;
  *(uint8_t*)0x20000be3 = 0;
  *(uint8_t*)0x20000be4 = 0;
  *(uint8_t*)0x20000be5 = 0;
  *(uint8_t*)0x20000be6 = 0;
  *(uint8_t*)0x20000be7 = 0;
  *(uint8_t*)0x20000be8 = 0;
  *(uint8_t*)0x20000be9 = 0;
  *(uint8_t*)0x20000bea = 0;
  *(uint8_t*)0x20000beb = 0;
  *(uint8_t*)0x20000bec = 0;
  *(uint8_t*)0x20000bed = 0;
  *(uint8_t*)0x20000bee = 0;
  *(uint8_t*)0x20000bef = 1;
  memcpy((void*)0x20000bf0,
         "gre0\000\000\000\000\000\000\000\000\000\000\000\000", 16);
  *(uint64_t*)0x20000c00 = 0;
  *(uint8_t*)0x20000c08 = 0;
  *(uint8_t*)0x20000c09 = 0;
  *(uint8_t*)0x20000c0a = 0;
  *(uint8_t*)0x20000c0b = 0;
  *(uint8_t*)0x20000c0c = 0;
  *(uint8_t*)0x20000c0d = 0;
  *(uint8_t*)0x20000c0e = 0;
  *(uint8_t*)0x20000c0f = 0;
  *(uint8_t*)0x20000c10 = 0;
  *(uint8_t*)0x20000c11 = 0;
  *(uint8_t*)0x20000c12 = 0;
  *(uint8_t*)0x20000c13 = 0;
  *(uint8_t*)0x20000c14 = 0;
  *(uint8_t*)0x20000c15 = 0;
  *(uint8_t*)0x20000c16 = 0;
  *(uint8_t*)0x20000c17 = 0;
  *(uint8_t*)0x20000c18 = 0;
  *(uint8_t*)0x20000c19 = 0;
  *(uint8_t*)0x20000c1a = 0;
  *(uint8_t*)0x20000c1b = 0;
  *(uint8_t*)0x20000c1c = 0;
  *(uint8_t*)0x20000c1d = 0;
  *(uint8_t*)0x20000c1e = 0;
  *(uint8_t*)0x20000c1f = 0;
  *(uint8_t*)0x20000c20 = 0;
  *(uint8_t*)0x20000c21 = 0;
  *(uint8_t*)0x20000c22 = 0;
  *(uint8_t*)0x20000c23 = 0;
  *(uint8_t*)0x20000c24 = 0;
  *(uint8_t*)0x20000c25 = 0;
  *(uint8_t*)0x20000c26 = 0;
  *(uint8_t*)0x20000c27 = 0;
  *(uint8_t*)0x20000c28 = 0;
  *(uint8_t*)0x20000c29 = 0;
  *(uint8_t*)0x20000c2a = 0;
  *(uint8_t*)0x20000c2b = 0;
  *(uint8_t*)0x20000c2c = 0;
  *(uint8_t*)0x20000c2d = 0;
  *(uint8_t*)0x20000c2e = 0;
  *(uint8_t*)0x20000c2f = 0;
  *(uint8_t*)0x20000c30 = 0;
  *(uint8_t*)0x20000c31 = 0;
  *(uint8_t*)0x20000c32 = 0;
  *(uint8_t*)0x20000c33 = 0;
  *(uint8_t*)0x20000c34 = 0;
  *(uint8_t*)0x20000c35 = 0;
  *(uint8_t*)0x20000c36 = 0;
  *(uint8_t*)0x20000c37 = 0;
  *(uint8_t*)0x20000c38 = 0;
  *(uint8_t*)0x20000c39 = 0;
  *(uint8_t*)0x20000c3a = 0;
  *(uint8_t*)0x20000c3b = 0;
  *(uint8_t*)0x20000c3c = 0;
  *(uint8_t*)0x20000c3d = 0;
  *(uint8_t*)0x20000c3e = 0;
  *(uint8_t*)0x20000c3f = 0;
  *(uint8_t*)0x20000c40 = 0;
  *(uint8_t*)0x20000c41 = 0;
  *(uint8_t*)0x20000c42 = 0;
  *(uint8_t*)0x20000c43 = 0;
  *(uint8_t*)0x20000c44 = 0;
  *(uint8_t*)0x20000c45 = 0;
  *(uint8_t*)0x20000c46 = 0;
  *(uint8_t*)0x20000c47 = 0;
  *(uint8_t*)0x20000c48 = 0;
  *(uint8_t*)0x20000c49 = 0;
  *(uint8_t*)0x20000c4a = 0;
  *(uint8_t*)0x20000c4b = 0;
  *(uint8_t*)0x20000c4c = 0;
  *(uint8_t*)0x20000c4d = 0;
  *(uint8_t*)0x20000c4e = 0;
  *(uint8_t*)0x20000c4f = 0;
  *(uint8_t*)0x20000c50 = 0;
  *(uint8_t*)0x20000c51 = 0;
  *(uint8_t*)0x20000c52 = 0;
  *(uint8_t*)0x20000c53 = 0;
  *(uint8_t*)0x20000c54 = 0;
  *(uint8_t*)0x20000c55 = 0;
  *(uint8_t*)0x20000c56 = 0;
  *(uint8_t*)0x20000c57 = 0;
  *(uint8_t*)0x20000c58 = 0;
  *(uint8_t*)0x20000c59 = 0;
  *(uint8_t*)0x20000c5a = 0;
  *(uint8_t*)0x20000c5b = 0;
  *(uint32_t*)0x20000c5c = 0;
  *(uint16_t*)0x20000c60 = 0x70;
  *(uint16_t*)0x20000c62 = 0x98;
  *(uint32_t*)0x20000c64 = 0;
  *(uint64_t*)0x20000c68 = 0;
  *(uint64_t*)0x20000c70 = 0;
  *(uint16_t*)0x20000c78 = 0x28;
  memcpy((void*)0x20000c7a, "DSCP\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000",
         29);
  *(uint8_t*)0x20000c97 = 0;
  *(uint8_t*)0x20000c98 = 0;
  *(uint8_t*)0x20000ca0 = 0;
  *(uint8_t*)0x20000ca1 = 0;
  *(uint8_t*)0x20000ca2 = 0;
  *(uint8_t*)0x20000ca3 = 0;
  *(uint8_t*)0x20000ca4 = 0;
  *(uint8_t*)0x20000ca5 = 0;
  *(uint8_t*)0x20000ca6 = 0;
  *(uint8_t*)0x20000ca7 = 0;
  *(uint8_t*)0x20000ca8 = 0;
  *(uint8_t*)0x20000ca9 = 0;
  *(uint8_t*)0x20000caa = 0;
  *(uint8_t*)0x20000cab = 0;
  *(uint8_t*)0x20000cac = 0;
  *(uint8_t*)0x20000cad = 0;
  *(uint8_t*)0x20000cae = 0;
  *(uint8_t*)0x20000caf = 0;
  *(uint8_t*)0x20000cb0 = 0;
  *(uint8_t*)0x20000cb1 = 0;
  *(uint8_t*)0x20000cb2 = 0;
  *(uint8_t*)0x20000cb3 = 0;
  *(uint8_t*)0x20000cb4 = 0;
  *(uint8_t*)0x20000cb5 = 0;
  *(uint8_t*)0x20000cb6 = 0;
  *(uint8_t*)0x20000cb7 = 0;
  *(uint8_t*)0x20000cb8 = 0;
  *(uint8_t*)0x20000cb9 = 0;
  *(uint8_t*)0x20000cba = 0;
  *(uint8_t*)0x20000cbb = 0;
  *(uint8_t*)0x20000cbc = 0;
  *(uint8_t*)0x20000cbd = 0;
  *(uint8_t*)0x20000cbe = 0;
  *(uint8_t*)0x20000cbf = 0;
  *(uint8_t*)0x20000cc0 = 0;
  *(uint8_t*)0x20000cc1 = 0;
  *(uint8_t*)0x20000cc2 = 0;
  *(uint8_t*)0x20000cc3 = 0;
  *(uint8_t*)0x20000cc4 = 0;
  *(uint8_t*)0x20000cc5 = 0;
  *(uint8_t*)0x20000cc6 = 0;
  *(uint8_t*)0x20000cc7 = 0;
  *(uint8_t*)0x20000cc8 = 0;
  *(uint8_t*)0x20000cc9 = 0;
  *(uint8_t*)0x20000cca = 0;
  *(uint8_t*)0x20000ccb = 0;
  *(uint8_t*)0x20000ccc = 0;
  *(uint8_t*)0x20000ccd = 0;
  *(uint8_t*)0x20000cce = 0;
  *(uint8_t*)0x20000ccf = 0;
  *(uint8_t*)0x20000cd0 = 0;
  *(uint8_t*)0x20000cd1 = 0;
  *(uint8_t*)0x20000cd2 = 0;
  *(uint8_t*)0x20000cd3 = 0;
  *(uint8_t*)0x20000cd4 = 0;
  *(uint8_t*)0x20000cd5 = 0;
  *(uint8_t*)0x20000cd6 = 0;
  *(uint8_t*)0x20000cd7 = 0;
  *(uint8_t*)0x20000cd8 = 0;
  *(uint8_t*)0x20000cd9 = 0;
  *(uint8_t*)0x20000cda = 0;
  *(uint8_t*)0x20000cdb = 0;
  *(uint8_t*)0x20000cdc = 0;
  *(uint8_t*)0x20000cdd = 0;
  *(uint8_t*)0x20000cde = 0;
  *(uint8_t*)0x20000cdf = 0;
  *(uint8_t*)0x20000ce0 = 0;
  *(uint8_t*)0x20000ce1 = 0;
  *(uint8_t*)0x20000ce2 = 0;
  *(uint8_t*)0x20000ce3 = 0;
  *(uint8_t*)0x20000ce4 = 0;
  *(uint8_t*)0x20000ce5 = 0;
  *(uint8_t*)0x20000ce6 = 0;
  *(uint8_t*)0x20000ce7 = 0;
  *(uint8_t*)0x20000ce8 = 0;
  *(uint8_t*)0x20000ce9 = 0;
  *(uint8_t*)0x20000cea = 0;
  *(uint8_t*)0x20000ceb = 0;
  *(uint8_t*)0x20000cec = 0;
  *(uint8_t*)0x20000ced = 0;
  *(uint8_t*)0x20000cee = 0;
  *(uint8_t*)0x20000cef = 0;
  *(uint8_t*)0x20000cf0 = 0;
  *(uint8_t*)0x20000cf1 = 0;
  *(uint8_t*)0x20000cf2 = 0;
  *(uint8_t*)0x20000cf3 = 0;
  *(uint32_t*)0x20000cf4 = 0;
  *(uint16_t*)0x20000cf8 = 0x70;
  *(uint16_t*)0x20000cfa = 0x98;
  *(uint32_t*)0x20000cfc = 0;
  *(uint64_t*)0x20000d00 = 0;
  *(uint64_t*)0x20000d08 = 0;
  *(uint16_t*)0x20000d10 = 0x28;
  memcpy((void*)0x20000d12, "TTL\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000",
         29);
  *(uint8_t*)0x20000d2f = 0;
  *(uint8_t*)0x20000d30 = 2;
  *(uint8_t*)0x20000d31 = 2;
  *(uint8_t*)0x20000d38 = 0;
  *(uint8_t*)0x20000d39 = 0;
  *(uint8_t*)0x20000d3a = 0;
  *(uint8_t*)0x20000d3b = 0;
  *(uint8_t*)0x20000d3c = 0;
  *(uint8_t*)0x20000d3d = 0;
  *(uint8_t*)0x20000d3e = 0;
  *(uint8_t*)0x20000d3f = 0;
  *(uint8_t*)0x20000d40 = 0;
  *(uint8_t*)0x20000d41 = 0;
  *(uint8_t*)0x20000d42 = 0;
  *(uint8_t*)0x20000d43 = 0;
  *(uint8_t*)0x20000d44 = 0;
  *(uint8_t*)0x20000d45 = 0;
  *(uint8_t*)0x20000d46 = 0;
  *(uint8_t*)0x20000d47 = 0;
  *(uint8_t*)0x20000d48 = 0;
  *(uint8_t*)0x20000d49 = 0;
  *(uint8_t*)0x20000d4a = 0;
  *(uint8_t*)0x20000d4b = 0;
  *(uint8_t*)0x20000d4c = 0;
  *(uint8_t*)0x20000d4d = 0;
  *(uint8_t*)0x20000d4e = 0;
  *(uint8_t*)0x20000d4f = 0;
  *(uint8_t*)0x20000d50 = 0;
  *(uint8_t*)0x20000d51 = 0;
  *(uint8_t*)0x20000d52 = 0;
  *(uint8_t*)0x20000d53 = 0;
  *(uint8_t*)0x20000d54 = 0;
  *(uint8_t*)0x20000d55 = 0;
  *(uint8_t*)0x20000d56 = 0;
  *(uint8_t*)0x20000d57 = 0;
  *(uint8_t*)0x20000d58 = 0;
  *(uint8_t*)0x20000d59 = 0;
  *(uint8_t*)0x20000d5a = 0;
  *(uint8_t*)0x20000d5b = 0;
  *(uint8_t*)0x20000d5c = 0;
  *(uint8_t*)0x20000d5d = 0;
  *(uint8_t*)0x20000d5e = 0;
  *(uint8_t*)0x20000d5f = 0;
  *(uint8_t*)0x20000d60 = 0;
  *(uint8_t*)0x20000d61 = 0;
  *(uint8_t*)0x20000d62 = 0;
  *(uint8_t*)0x20000d63 = 0;
  *(uint8_t*)0x20000d64 = 0;
  *(uint8_t*)0x20000d65 = 0;
  *(uint8_t*)0x20000d66 = 0;
  *(uint8_t*)0x20000d67 = 0;
  *(uint8_t*)0x20000d68 = 0;
  *(uint8_t*)0x20000d69 = 0;
  *(uint8_t*)0x20000d6a = 0;
  *(uint8_t*)0x20000d6b = 0;
  *(uint8_t*)0x20000d6c = 0;
  *(uint8_t*)0x20000d6d = 0;
  *(uint8_t*)0x20000d6e = 0;
  *(uint8_t*)0x20000d6f = 0;
  *(uint8_t*)0x20000d70 = 0;
  *(uint8_t*)0x20000d71 = 0;
  *(uint8_t*)0x20000d72 = 0;
  *(uint8_t*)0x20000d73 = 0;
  *(uint8_t*)0x20000d74 = 0;
  *(uint8_t*)0x20000d75 = 0;
  *(uint8_t*)0x20000d76 = 0;
  *(uint8_t*)0x20000d77 = 0;
  *(uint8_t*)0x20000d78 = 0;
  *(uint8_t*)0x20000d79 = 0;
  *(uint8_t*)0x20000d7a = 0;
  *(uint8_t*)0x20000d7b = 0;
  *(uint8_t*)0x20000d7c = 0;
  *(uint8_t*)0x20000d7d = 0;
  *(uint8_t*)0x20000d7e = 0;
  *(uint8_t*)0x20000d7f = 0;
  *(uint8_t*)0x20000d80 = 0;
  *(uint8_t*)0x20000d81 = 0;
  *(uint8_t*)0x20000d82 = 0;
  *(uint8_t*)0x20000d83 = 0;
  *(uint8_t*)0x20000d84 = 0;
  *(uint8_t*)0x20000d85 = 0;
  *(uint8_t*)0x20000d86 = 0;
  *(uint8_t*)0x20000d87 = 0;
  *(uint8_t*)0x20000d88 = 0;
  *(uint8_t*)0x20000d89 = 0;
  *(uint8_t*)0x20000d8a = 0;
  *(uint8_t*)0x20000d8b = 0;
  *(uint32_t*)0x20000d8c = 0;
  *(uint16_t*)0x20000d90 = 0x70;
  *(uint16_t*)0x20000d92 = 0x98;
  *(uint32_t*)0x20000d94 = 0;
  *(uint64_t*)0x20000d98 = 0;
  *(uint64_t*)0x20000da0 = 0;
  *(uint16_t*)0x20000da8 = 0x28;
  memcpy((void*)0x20000daa, "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000",
         29);
  *(uint8_t*)0x20000dc7 = 0;
  *(uint32_t*)0x20000dc8 = 0xfffffffe;
  syscall(__NR_setsockopt, r[2], 0, 0x40, 0x200009c0ul, 0x410ul);
  memcpy((void*)0x20000040, "syzkaller1\000\000\000\000\000\000", 16);
  *(uint16_t*)0x20000050 = 0x5001;
  syscall(__NR_ioctl, r[0], 0x400454ca, 0x20000040ul);
  res = syscall(__NR_socket, 0x10ul, 3ul, 0);
  if (res != -1)
    r[3] = res;
  memcpy((void*)0x20000140, "syzkaller1\000\000\000\000\000\000", 16);
  *(uint16_t*)0x20000150 = 7;
  *(uint16_t*)0x20000152 = htobe16(0);
  *(uint32_t*)0x20000154 = htobe32(0x7f000001);
  syscall(__NR_ioctl, r[3], 0x8914, 0x20000140ul);
  *(uint8_t*)0x200000c0 = 3;
  *(uint8_t*)0x200000c1 = 0;
  *(uint16_t*)0x200000c2 = 0;
  *(uint16_t*)0x200000c4 = 0;
  *(uint16_t*)0x200000c6 = 0xe0;
  *(uint16_t*)0x200000c8 = 0;
  STORE_BY_BITMASK(uint8_t, , 0x200000ca, 5, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x200000ca, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x200000cb, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, , 0x200000cb, 0, 2, 6);
  *(uint16_t*)0x200000cc = htobe16(0x1c);
  *(uint16_t*)0x200000ce = htobe16(0);
  *(uint16_t*)0x200000d0 = htobe16(0);
  *(uint8_t*)0x200000d2 = 0;
  *(uint8_t*)0x200000d3 = 2;
  *(uint16_t*)0x200000d4 = htobe16(0);
  *(uint32_t*)0x200000d6 = htobe32(0);
  *(uint32_t*)0x200000da = htobe32(-1);
  *(uint8_t*)0x200000de = 0x16;
  *(uint8_t*)0x200000df = 0;
  *(uint16_t*)0x200000e0 = htobe16(0);
  *(uint32_t*)0x200000e2 = htobe32(0);
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x200000de, 8);
  *(uint16_t*)0x200000e0 = csum_inet_digest(&csum_1);
  struct csum_inet csum_2;
  csum_inet_init(&csum_2);
  csum_inet_update(&csum_2, (const uint8_t*)0x200000ca, 20);
  *(uint16_t*)0x200000d4 = csum_inet_digest(&csum_2);
  syscall(__NR_write, r[0], 0x200000c0ul, 0xfdeful);
  return 0;
}
