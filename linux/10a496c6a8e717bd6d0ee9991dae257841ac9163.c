// KMSAN: kernel-infoleak in snd_pcm_oss_read
// https://syzkaller.appspot.com/bug?id=10a496c6a8e717bd6d0ee9991dae257841ac9163
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
  res = syscall(__NR_epoll_create1, 0);
  if (res != -1)
    r[0] = res;
  syscall(__NR_close, r[0]);
  memcpy((void*)0x20000000, "/dev/dsp", 9);
  syscall(__NR_openat, 0xffffffffffffff9c, 0x20000000, 0, 0);
  syscall(__NR_read, r[0], 0x20000300, 0xfa);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
