// KASAN: slab-out-of-bounds Read in clusterip_tg_check
// https://syzkaller.appspot.com/bug?id=a63c0e4c1b0b82c8a768cd3ca9f7c459faa11787
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0x29000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 2, 0x80002, 0);
  memcpy((void*)0x20027000, "\x66\x69\x6c\x74\x65\x72\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20027020 = 0xe;
  *(uint32_t*)0x20027024 = 4;
  *(uint32_t*)0x20027028 = 0x2c0;
  *(uint32_t*)0x2002702c = -1;
  *(uint32_t*)0x20027030 = 0;
  *(uint32_t*)0x20027034 = 0;
  *(uint32_t*)0x20027038 = 0;
  *(uint32_t*)0x2002703c = -1;
  *(uint32_t*)0x20027040 = -1;
  *(uint32_t*)0x20027044 = 0;
  *(uint32_t*)0x20027048 = 0;
  *(uint32_t*)0x2002704c = 0;
  *(uint32_t*)0x20027050 = -1;
  *(uint32_t*)0x20027054 = 4;
  *(uint64_t*)0x20027058 = 0x20001000;
  *(uint8_t*)0x20027060 = 0;
  *(uint8_t*)0x20027061 = 0;
  *(uint8_t*)0x20027062 = 0;
  *(uint8_t*)0x20027063 = 0;
  *(uint8_t*)0x20027064 = 0;
  *(uint8_t*)0x20027065 = 0;
  *(uint8_t*)0x20027066 = 0;
  *(uint8_t*)0x20027067 = 0;
  *(uint8_t*)0x20027068 = 0;
  *(uint8_t*)0x20027069 = 0;
  *(uint8_t*)0x2002706a = 0;
  *(uint8_t*)0x2002706b = 0;
  *(uint8_t*)0x2002706c = 0;
  *(uint8_t*)0x2002706d = 0;
  *(uint8_t*)0x2002706e = 0;
  *(uint8_t*)0x2002706f = 0;
  *(uint8_t*)0x20027070 = 0;
  *(uint8_t*)0x20027071 = 0;
  *(uint8_t*)0x20027072 = 0;
  *(uint8_t*)0x20027073 = 0;
  *(uint8_t*)0x20027074 = 0;
  *(uint8_t*)0x20027075 = 0;
  *(uint8_t*)0x20027076 = 0;
  *(uint8_t*)0x20027077 = 0;
  *(uint8_t*)0x20027078 = 0;
  *(uint8_t*)0x20027079 = 0;
  *(uint8_t*)0x2002707a = 0;
  *(uint8_t*)0x2002707b = 0;
  *(uint8_t*)0x2002707c = 0;
  *(uint8_t*)0x2002707d = 0;
  *(uint8_t*)0x2002707e = 0;
  *(uint8_t*)0x2002707f = 0;
  *(uint8_t*)0x20027080 = 0;
  *(uint8_t*)0x20027081 = 0;
  *(uint8_t*)0x20027082 = 0;
  *(uint8_t*)0x20027083 = 0;
  *(uint8_t*)0x20027084 = 0;
  *(uint8_t*)0x20027085 = 0;
  *(uint8_t*)0x20027086 = 0;
  *(uint8_t*)0x20027087 = 0;
  *(uint8_t*)0x20027088 = 0;
  *(uint8_t*)0x20027089 = 0;
  *(uint8_t*)0x2002708a = 0;
  *(uint8_t*)0x2002708b = 0;
  *(uint8_t*)0x2002708c = 0;
  *(uint8_t*)0x2002708d = 0;
  *(uint8_t*)0x2002708e = 0;
  *(uint8_t*)0x2002708f = 0;
  *(uint8_t*)0x20027090 = 0;
  *(uint8_t*)0x20027091 = 0;
  *(uint8_t*)0x20027092 = 0;
  *(uint8_t*)0x20027093 = 0;
  *(uint8_t*)0x20027094 = 0;
  *(uint8_t*)0x20027095 = 0;
  *(uint8_t*)0x20027096 = 0;
  *(uint8_t*)0x20027097 = 0;
  *(uint8_t*)0x20027098 = 0;
  *(uint8_t*)0x20027099 = 0;
  *(uint8_t*)0x2002709a = 0;
  *(uint8_t*)0x2002709b = 0;
  *(uint8_t*)0x2002709c = 0;
  *(uint8_t*)0x2002709d = 0;
  *(uint8_t*)0x2002709e = 0;
  *(uint8_t*)0x2002709f = 0;
  *(uint8_t*)0x200270a0 = 0;
  *(uint8_t*)0x200270a1 = 0;
  *(uint8_t*)0x200270a2 = 0;
  *(uint8_t*)0x200270a3 = 0;
  *(uint8_t*)0x200270a4 = 0;
  *(uint8_t*)0x200270a5 = 0;
  *(uint8_t*)0x200270a6 = 0;
  *(uint8_t*)0x200270a7 = 0;
  *(uint8_t*)0x200270a8 = 0;
  *(uint8_t*)0x200270a9 = 0;
  *(uint8_t*)0x200270aa = 0;
  *(uint8_t*)0x200270ab = 0;
  *(uint8_t*)0x200270ac = 0;
  *(uint8_t*)0x200270ad = 0;
  *(uint8_t*)0x200270ae = 0;
  *(uint8_t*)0x200270af = 0;
  *(uint8_t*)0x200270b0 = 0;
  *(uint8_t*)0x200270b1 = 0;
  *(uint8_t*)0x200270b2 = 0;
  *(uint8_t*)0x200270b3 = 0;
  *(uint32_t*)0x200270b4 = 0;
  *(uint16_t*)0x200270b8 = 0x70;
  *(uint16_t*)0x200270ba = 0x98;
  *(uint32_t*)0x200270bc = 0;
  *(uint64_t*)0x200270c0 = 0;
  *(uint64_t*)0x200270c8 = 0;
  *(uint16_t*)0x200270d0 = 0x28;
  memcpy((void*)0x200270d2, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x200270ef = 0;
  *(uint32_t*)0x200270f0 = 0xfffffffe;
  *(uint8_t*)0x200270f8 = 0;
  *(uint8_t*)0x200270f9 = 0;
  *(uint8_t*)0x200270fa = 0;
  *(uint8_t*)0x200270fb = 0;
  *(uint8_t*)0x200270fc = 0;
  *(uint8_t*)0x200270fd = 0;
  *(uint8_t*)0x200270fe = 0;
  *(uint8_t*)0x200270ff = 0;
  *(uint8_t*)0x20027100 = 0;
  *(uint8_t*)0x20027101 = 0;
  *(uint8_t*)0x20027102 = 0;
  *(uint8_t*)0x20027103 = 0;
  *(uint8_t*)0x20027104 = 0;
  *(uint8_t*)0x20027105 = 0;
  *(uint8_t*)0x20027106 = 0;
  *(uint8_t*)0x20027107 = 0;
  *(uint8_t*)0x20027108 = 0;
  *(uint8_t*)0x20027109 = 0;
  *(uint8_t*)0x2002710a = 0;
  *(uint8_t*)0x2002710b = 0;
  *(uint8_t*)0x2002710c = 0;
  *(uint8_t*)0x2002710d = 0;
  *(uint8_t*)0x2002710e = 0;
  *(uint8_t*)0x2002710f = 0;
  *(uint8_t*)0x20027110 = 0;
  *(uint8_t*)0x20027111 = 0;
  *(uint8_t*)0x20027112 = 0;
  *(uint8_t*)0x20027113 = 0;
  *(uint8_t*)0x20027114 = 0;
  *(uint8_t*)0x20027115 = 0;
  *(uint8_t*)0x20027116 = 0;
  *(uint8_t*)0x20027117 = 0;
  *(uint8_t*)0x20027118 = 0;
  *(uint8_t*)0x20027119 = 0;
  *(uint8_t*)0x2002711a = 0;
  *(uint8_t*)0x2002711b = 0;
  *(uint8_t*)0x2002711c = 0;
  *(uint8_t*)0x2002711d = 0;
  *(uint8_t*)0x2002711e = 0;
  *(uint8_t*)0x2002711f = 0;
  *(uint8_t*)0x20027120 = 0;
  *(uint8_t*)0x20027121 = 0;
  *(uint8_t*)0x20027122 = 0;
  *(uint8_t*)0x20027123 = 0;
  *(uint8_t*)0x20027124 = 0;
  *(uint8_t*)0x20027125 = 0;
  *(uint8_t*)0x20027126 = 0;
  *(uint8_t*)0x20027127 = 0;
  *(uint8_t*)0x20027128 = 0;
  *(uint8_t*)0x20027129 = 0;
  *(uint8_t*)0x2002712a = 0;
  *(uint8_t*)0x2002712b = 0;
  *(uint8_t*)0x2002712c = 0;
  *(uint8_t*)0x2002712d = 0;
  *(uint8_t*)0x2002712e = 0;
  *(uint8_t*)0x2002712f = 0;
  *(uint8_t*)0x20027130 = 0;
  *(uint8_t*)0x20027131 = 0;
  *(uint8_t*)0x20027132 = 0;
  *(uint8_t*)0x20027133 = 0;
  *(uint8_t*)0x20027134 = 0;
  *(uint8_t*)0x20027135 = 0;
  *(uint8_t*)0x20027136 = 0;
  *(uint8_t*)0x20027137 = 0;
  *(uint8_t*)0x20027138 = 0;
  *(uint8_t*)0x20027139 = 0;
  *(uint8_t*)0x2002713a = 0;
  *(uint8_t*)0x2002713b = 0;
  *(uint8_t*)0x2002713c = 0;
  *(uint8_t*)0x2002713d = 0;
  *(uint8_t*)0x2002713e = 0;
  *(uint8_t*)0x2002713f = 0;
  *(uint8_t*)0x20027140 = 0;
  *(uint8_t*)0x20027141 = 0;
  *(uint8_t*)0x20027142 = 0;
  *(uint8_t*)0x20027143 = 0;
  *(uint8_t*)0x20027144 = 0;
  *(uint8_t*)0x20027145 = 0;
  *(uint8_t*)0x20027146 = 0;
  *(uint8_t*)0x20027147 = 0;
  *(uint8_t*)0x20027148 = 0;
  *(uint8_t*)0x20027149 = 0;
  *(uint8_t*)0x2002714a = 0;
  *(uint8_t*)0x2002714b = 0;
  *(uint32_t*)0x2002714c = 0;
  *(uint16_t*)0x20027150 = 0x70;
  *(uint16_t*)0x20027152 = 0x98;
  *(uint32_t*)0x20027154 = 0;
  *(uint64_t*)0x20027158 = 0;
  *(uint64_t*)0x20027160 = 0;
  *(uint16_t*)0x20027168 = 0x28;
  memcpy((void*)0x2002716a, "\x52\x45\x4a\x45\x43\x54\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20027187 = 0;
  *(uint32_t*)0x20027188 = 0;
  *(uint32_t*)0x20027190 = htobe32(6);
  *(uint32_t*)0x20027194 = htobe32(4);
  *(uint32_t*)0x20027198 = htobe32(0xff);
  *(uint32_t*)0x2002719c = htobe32(-1);
  memcpy((void*)0x200271a0,
         "\x73\x69\x74\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint8_t*)0x200271b0 = 0x73;
  *(uint8_t*)0x200271b1 = 0x79;
  *(uint8_t*)0x200271b2 = 0x7a;
  *(uint8_t*)0x200271b3 = 0x30;
  *(uint8_t*)0x200271b4 = 0;
  *(uint8_t*)0x200271c0 = -1;
  *(uint8_t*)0x200271c1 = 0;
  *(uint8_t*)0x200271c2 = 0;
  *(uint8_t*)0x200271c3 = 0;
  *(uint8_t*)0x200271c4 = 0;
  *(uint8_t*)0x200271c5 = 0;
  *(uint8_t*)0x200271c6 = 0;
  *(uint8_t*)0x200271c7 = 0;
  *(uint8_t*)0x200271c8 = 0;
  *(uint8_t*)0x200271c9 = 0;
  *(uint8_t*)0x200271ca = 0;
  *(uint8_t*)0x200271cb = 0;
  *(uint8_t*)0x200271cc = 0;
  *(uint8_t*)0x200271cd = 0;
  *(uint8_t*)0x200271ce = 0;
  *(uint8_t*)0x200271cf = 0;
  *(uint8_t*)0x200271d0 = -1;
  *(uint8_t*)0x200271d1 = 0;
  *(uint8_t*)0x200271d2 = 0;
  *(uint8_t*)0x200271d3 = 0;
  *(uint8_t*)0x200271d4 = 0;
  *(uint8_t*)0x200271d5 = 0;
  *(uint8_t*)0x200271d6 = 0;
  *(uint8_t*)0x200271d7 = 0;
  *(uint8_t*)0x200271d8 = 0;
  *(uint8_t*)0x200271d9 = 0;
  *(uint8_t*)0x200271da = 0;
  *(uint8_t*)0x200271db = 0;
  *(uint8_t*)0x200271dc = 0;
  *(uint8_t*)0x200271dd = 0;
  *(uint8_t*)0x200271de = 0;
  *(uint8_t*)0x200271df = 0;
  *(uint16_t*)0x200271e0 = 0xfb;
  *(uint8_t*)0x200271e2 = 1;
  *(uint8_t*)0x200271e3 = 4;
  *(uint32_t*)0x200271e4 = 0;
  *(uint16_t*)0x200271e8 = 0x98;
  *(uint16_t*)0x200271ea = 0xf8;
  *(uint32_t*)0x200271ec = 0;
  *(uint64_t*)0x200271f0 = 0;
  *(uint64_t*)0x200271f8 = 0;
  *(uint16_t*)0x20027200 = 0x28;
  memcpy((void*)0x20027202, "\x63\x6f\x6e\x6e\x6c\x61\x62\x65\x6c\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x2002721f = 0;
  *(uint16_t*)0x20027220 = 0;
  *(uint16_t*)0x20027222 = 0;
  *(uint16_t*)0x20027228 = 0x60;
  memcpy((void*)0x2002722a, "\x43\x4c\x55\x53\x54\x45\x52\x49\x50\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20027247 = 0;
  *(uint32_t*)0x20027248 = 1;
  *(uint8_t*)0x2002724c = 0xaa;
  *(uint8_t*)0x2002724d = 0xaa;
  *(uint8_t*)0x2002724e = 0xaa;
  *(uint8_t*)0x2002724f = 0xaa;
  *(uint8_t*)0x20027250 = 0;
  *(uint8_t*)0x20027251 = 0xaa;
  *(uint16_t*)0x20027252 = 6;
  *(uint16_t*)0x20027254 = 0x3ff;
  *(uint16_t*)0x20027256 = 0x50;
  *(uint16_t*)0x20027258 = 0x56b;
  *(uint16_t*)0x2002725a = 0x1f;
  *(uint16_t*)0x2002725c = 2;
  *(uint16_t*)0x2002725e = 0x35c;
  *(uint16_t*)0x20027260 = 0x101;
  *(uint16_t*)0x20027262 = 7;
  *(uint16_t*)0x20027264 = 0x40;
  *(uint16_t*)0x20027266 = 1;
  *(uint16_t*)0x20027268 = 0x200;
  *(uint16_t*)0x2002726a = 0;
  *(uint16_t*)0x2002726c = 0xff;
  *(uint16_t*)0x2002726e = 0x22d;
  *(uint16_t*)0x20027270 = 6;
  *(uint16_t*)0x20027272 = 8;
  *(uint16_t*)0x20027274 = 8;
  *(uint32_t*)0x20027278 = 1;
  *(uint32_t*)0x2002727c = 9;
  *(uint64_t*)0x20027280 = 7;
  *(uint8_t*)0x20027288 = 0;
  *(uint8_t*)0x20027289 = 0;
  *(uint8_t*)0x2002728a = 0;
  *(uint8_t*)0x2002728b = 0;
  *(uint8_t*)0x2002728c = 0;
  *(uint8_t*)0x2002728d = 0;
  *(uint8_t*)0x2002728e = 0;
  *(uint8_t*)0x2002728f = 0;
  *(uint8_t*)0x20027290 = 0;
  *(uint8_t*)0x20027291 = 0;
  *(uint8_t*)0x20027292 = 0;
  *(uint8_t*)0x20027293 = 0;
  *(uint8_t*)0x20027294 = 0;
  *(uint8_t*)0x20027295 = 0;
  *(uint8_t*)0x20027296 = 0;
  *(uint8_t*)0x20027297 = 0;
  *(uint8_t*)0x20027298 = 0;
  *(uint8_t*)0x20027299 = 0;
  *(uint8_t*)0x2002729a = 0;
  *(uint8_t*)0x2002729b = 0;
  *(uint8_t*)0x2002729c = 0;
  *(uint8_t*)0x2002729d = 0;
  *(uint8_t*)0x2002729e = 0;
  *(uint8_t*)0x2002729f = 0;
  *(uint8_t*)0x200272a0 = 0;
  *(uint8_t*)0x200272a1 = 0;
  *(uint8_t*)0x200272a2 = 0;
  *(uint8_t*)0x200272a3 = 0;
  *(uint8_t*)0x200272a4 = 0;
  *(uint8_t*)0x200272a5 = 0;
  *(uint8_t*)0x200272a6 = 0;
  *(uint8_t*)0x200272a7 = 0;
  *(uint8_t*)0x200272a8 = 0;
  *(uint8_t*)0x200272a9 = 0;
  *(uint8_t*)0x200272aa = 0;
  *(uint8_t*)0x200272ab = 0;
  *(uint8_t*)0x200272ac = 0;
  *(uint8_t*)0x200272ad = 0;
  *(uint8_t*)0x200272ae = 0;
  *(uint8_t*)0x200272af = 0;
  *(uint8_t*)0x200272b0 = 0;
  *(uint8_t*)0x200272b1 = 0;
  *(uint8_t*)0x200272b2 = 0;
  *(uint8_t*)0x200272b3 = 0;
  *(uint8_t*)0x200272b4 = 0;
  *(uint8_t*)0x200272b5 = 0;
  *(uint8_t*)0x200272b6 = 0;
  *(uint8_t*)0x200272b7 = 0;
  *(uint8_t*)0x200272b8 = 0;
  *(uint8_t*)0x200272b9 = 0;
  *(uint8_t*)0x200272ba = 0;
  *(uint8_t*)0x200272bb = 0;
  *(uint8_t*)0x200272bc = 0;
  *(uint8_t*)0x200272bd = 0;
  *(uint8_t*)0x200272be = 0;
  *(uint8_t*)0x200272bf = 0;
  *(uint8_t*)0x200272c0 = 0;
  *(uint8_t*)0x200272c1 = 0;
  *(uint8_t*)0x200272c2 = 0;
  *(uint8_t*)0x200272c3 = 0;
  *(uint8_t*)0x200272c4 = 0;
  *(uint8_t*)0x200272c5 = 0;
  *(uint8_t*)0x200272c6 = 0;
  *(uint8_t*)0x200272c7 = 0;
  *(uint8_t*)0x200272c8 = 0;
  *(uint8_t*)0x200272c9 = 0;
  *(uint8_t*)0x200272ca = 0;
  *(uint8_t*)0x200272cb = 0;
  *(uint8_t*)0x200272cc = 0;
  *(uint8_t*)0x200272cd = 0;
  *(uint8_t*)0x200272ce = 0;
  *(uint8_t*)0x200272cf = 0;
  *(uint8_t*)0x200272d0 = 0;
  *(uint8_t*)0x200272d1 = 0;
  *(uint8_t*)0x200272d2 = 0;
  *(uint8_t*)0x200272d3 = 0;
  *(uint8_t*)0x200272d4 = 0;
  *(uint8_t*)0x200272d5 = 0;
  *(uint8_t*)0x200272d6 = 0;
  *(uint8_t*)0x200272d7 = 0;
  *(uint8_t*)0x200272d8 = 0;
  *(uint8_t*)0x200272d9 = 0;
  *(uint8_t*)0x200272da = 0;
  *(uint8_t*)0x200272db = 0;
  *(uint32_t*)0x200272dc = 0;
  *(uint16_t*)0x200272e0 = 0x70;
  *(uint16_t*)0x200272e2 = 0x98;
  *(uint32_t*)0x200272e4 = 0;
  *(uint64_t*)0x200272e8 = 0;
  *(uint64_t*)0x200272f0 = 0;
  *(uint16_t*)0x200272f8 = 0x28;
  memcpy((void*)0x200272fa, "\x52\x45\x4a\x45\x43\x54\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20027317 = 0;
  *(uint32_t*)0x20027318 = 0;
  *(uint64_t*)0x20001000 = 0;
  *(uint64_t*)0x20001008 = 0;
  *(uint64_t*)0x20001010 = 0;
  *(uint64_t*)0x20001018 = 0;
  *(uint64_t*)0x20001020 = 0;
  *(uint64_t*)0x20001028 = 0;
  *(uint64_t*)0x20001030 = 0;
  *(uint64_t*)0x20001038 = 0;
  syscall(__NR_setsockopt, r[0], 0, 0x40, 0x20027000, 0x320);
}

int main()
{
  loop();
  return 0;
}
