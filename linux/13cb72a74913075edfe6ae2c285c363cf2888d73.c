// KMSAN: uninit-value in nfqnl_recv_config
// https://syzkaller.appspot.com/bug?id=13cb72a74913075edfe6ae2c285c363cf2888d73
// status:invalid
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0xc);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000080 = 0;
  *(uint32_t*)0x20000088 = 0;
  *(uint64_t*)0x20000090 = 0x20009ff0;
  *(uint64_t*)0x20009ff0 = 0x20000000;
  memcpy((void*)0x20000000, "\x24\x00\x00\x00\x02\x03\x07\x03\x1d\xff\xfd\x94"
                            "\x6f\xa2\x83\x00\x20\x20\x0a\x00\x09\x00\x05\x00"
                            "\x00\x1d\x85\x68\x0c\x1b\xa3\xa2\x04\x00\x04\x00",
         36);
  *(uint64_t*)0x20009ff8 = 0x24;
  *(uint64_t*)0x20000098 = 1;
  *(uint64_t*)0x200000a0 = 0;
  *(uint64_t*)0x200000a8 = 0;
  *(uint32_t*)0x200000b0 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000080, 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
