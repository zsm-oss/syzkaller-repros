// INFO: task hung in rwsem_down_read_failed
// https://syzkaller.appspot.com/bug?id=f182dd0b0f4f1348d7347faee837cdad8a403d31
// status:open
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

static uintptr_t syz_open_pts(uintptr_t a0, uintptr_t a1)
{
  int ptyno = 0;
  if (ioctl(a0, TIOCGPTN, &ptyno))
    return -1;
  char buf[128];
  sprintf(buf, "/dev/pts/%d", ptyno);
  return open(buf, a1, 0);
}

static void test();

void loop()
{
  while (1) {
    test();
  }
}

long r[2];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
            0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    memcpy((void*)0x20fe4000, "/dev/ptmx", 10);
    r[0] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20fe4000ul,
                   0x400ul, 0x0ul);
    break;
  case 2:
    *(uint16_t*)0x20fd6000 = (uint16_t)0x0;
    *(uint16_t*)0x20fd6002 = (uint16_t)0x0;
    *(uint16_t*)0x20fd6004 = (uint16_t)0x0;
    *(uint16_t*)0x20fd6006 = (uint16_t)0x0;
    *(uint8_t*)0x20fd6008 = (uint8_t)0x0;
    *(uint8_t*)0x20fd6009 = (uint8_t)0x0;
    *(uint8_t*)0x20fd600a = (uint8_t)0x0;
    *(uint8_t*)0x20fd600b = (uint8_t)0x0;
    *(uint32_t*)0x20fd600c = (uint32_t)0xfffffffffffffff7;
    *(uint8_t*)0x20fd6010 = (uint8_t)0x0;
    syscall(__NR_ioctl, r[0], 0x5402ul, 0x20fd6000ul);
    break;
  case 3:
    *(uint32_t*)0x203fffdc = (uint32_t)0x0;
    *(uint32_t*)0x203fffe0 = (uint32_t)0x1;
    *(uint32_t*)0x203fffe4 = (uint32_t)0x2b;
    *(uint32_t*)0x203fffe8 = (uint32_t)0xfffffffffffffff8;
    *(uint8_t*)0x203fffec = (uint8_t)0x6;
    *(uint8_t*)0x203fffed = (uint8_t)0x7;
    *(uint8_t*)0x203fffee = (uint8_t)0x0;
    *(uint8_t*)0x203fffef = (uint8_t)0xfffffffffffffffe;
    *(uint32_t*)0x203ffff0 = (uint32_t)0x0;
    *(uint32_t*)0x203ffff4 = (uint32_t)0x0;
    *(uint32_t*)0x203ffff8 = (uint32_t)0x0;
    *(uint32_t*)0x203ffffc = (uint32_t)0x69953b77;
    syscall(__NR_ioctl, r[0], 0x40045431ul, 0x203fffdcul);
    break;
  case 4:
    r[1] = syz_open_pts(r[0], 0x0ul);
    break;
  case 5:
    syscall(__NR_ioctl, r[1], 0x541bul, 0x20a6bffcul);
    break;
  case 6:
    *(uint64_t*)0x20fd6000 = (uint64_t)0x20313f29;
    *(uint64_t*)0x20fd6008 = (uint64_t)0x1;
    syscall(__NR_readv, r[1], 0x20fd6000ul, 0x1ul);
    break;
  case 7:
    syscall(__NR_ioctl, r[0], 0x540aul, 0x2ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[16];

  memset(r, -1, sizeof(r));
  srand(getpid());
  for (i = 0; i < 8; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  for (i = 0; i < 8; i++) {
    pthread_create(&th[8 + i], 0, thr, (void*)i);
    if (rand() % 2)
      usleep(rand() % 10000);
  }
  usleep(rand() % 100000);
}

int main()
{
  int i;
  for (i = 0; i < 8; i++) {
    if (fork() == 0) {
      loop();
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}
