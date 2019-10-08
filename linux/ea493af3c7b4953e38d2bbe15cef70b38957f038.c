// kernel BUG at fs/userfaultfd.c:LINE!
// https://syzkaller.appspot.com/bug?id=ea493af3c7b4953e38d2bbe15cef70b38957f038
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <pthread.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

#ifndef __NR_userfaultfd
#define __NR_userfaultfd 323
#endif

long r[1];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
            0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    r[0] = syscall(__NR_userfaultfd, 0x0ul);
    break;
  case 2:
    *(uint64_t*)0x20842fe8 = (uint64_t)0xaa;
    *(uint64_t*)0x20842ff0 = (uint64_t)0x2;
    *(uint64_t*)0x20842ff8 = (uint64_t)0x0;
    syscall(__NR_ioctl, r[0], 0xc018aa3ful, 0x20842fe8ul);
    break;
  case 3:
    *(uint64_t*)0x20c18fe0 = (uint64_t)0x204a4000;
    *(uint64_t*)0x20c18fe8 = (uint64_t)0x800000;
    *(uint64_t*)0x20c18ff0 = (uint64_t)0x1;
    *(uint64_t*)0x20c18ff8 = (uint64_t)0x0;
    syscall(__NR_ioctl, r[0], 0xc020aa00ul, 0x20c18fe0ul);
    break;
  case 4:
    syscall(__NR_clone, 0x600ul, 0x20687000ul, 0x20b4c000ul,
            0x20552ffcul, 0x207a4f71ul);
    break;
  case 5:
    syscall(__NR_close, r[0]);
    break;
  }
  return 0;
}

void loop()
{
  long i;
  pthread_t th[12];

  memset(r, -1, sizeof(r));
  for (i = 0; i < 6; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  usleep(rand() % 100000);
}

int main()
{
  loop();
  return 0;
}
