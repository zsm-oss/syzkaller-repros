// general protection fault in bdev_read_page (2)
// https://syzkaller.appspot.com/bug?id=662448179365dddc1880
// status:0
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static bool write_file(const char* file, const char* what, ...)
{
  char buf[1024];
  va_list args;
  va_start(args, what);
  vsnprintf(buf, sizeof(buf), what, args);
  va_end(args);
  buf[sizeof(buf) - 1] = 0;
  int len = strlen(buf);
  int fd = open(file, O_WRONLY | O_CLOEXEC);
  if (fd == -1)
    return false;
  if (write(fd, buf, len) != len) {
    int err = errno;
    close(fd);
    errno = err;
    return false;
  }
  close(fd);
  return true;
}

static long syz_open_dev(volatile long a0, volatile long a1, volatile long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  for (int i = 0; i < 100; i++) {
    if (waitpid(-1, status, WNOHANG | __WALL) == pid)
      return;
    usleep(1000);
  }
  DIR* dir = opendir("/sys/fs/fuse/connections");
  if (dir) {
    for (;;) {
      struct dirent* ent = readdir(dir);
      if (!ent)
        break;
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        continue;
      char abort[300];
      snprintf(abort, sizeof(abort), "/sys/fs/fuse/connections/%s/abort",
               ent->d_name);
      int fd = open(abort, O_WRONLY);
      if (fd == -1) {
        continue;
      }
      if (write(fd, abort, 1) < 0) {
      }
      close(fd);
    }
    closedir(dir);
  } else {
  }
  while (waitpid(-1, status, __WALL) != pid) {
  }
}

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter = 0;
  for (;; iter++) {
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  memcpy((void*)0x20000000, "/dev/loop#\000", 11);
  res = -1;
  res = syz_open_dev(0x20000000, 0, 0x8602);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000400 = 0x600;
  *(uint16_t*)0x20000408 = 0x17e;
  *(uint64_t*)0x20000410 = 0;
  *(uint16_t*)0x20000418 = 0;
  *(uint32_t*)0x20000420 = 0;
  *(uint32_t*)0x20000424 = 0;
  *(uint32_t*)0x20000428 = 0x1b;
  *(uint32_t*)0x2000042c = 0;
  memcpy((void*)0x20000430,
         "\xb0\x4a\x83\x70\x66\x94\xa7\x51\x54\xc6\x29\xf7\x5b\x71\xa9\x5a\x8f"
         "\x3b\x28\x92\x71\xc6\x07\xad\xb2\x2d\x4d\x75\xc3\x01\x00\x7b\x12\x91"
         "\xb1\x15\x0d\x4d\x3a\xa2\x0d\x7b\xee\xbc\x68\x9e\x92\x6d\x5e\x29\x06"
         "\x79\xd1\x04\x2f\x1b\x38\xe9\xe8\xaf\x69\x4c\x0d\x43",
         64);
  memcpy((void*)0x20000470, "\x52\x44\x53\x4b\x0f\x00\x00\x0d\x57\x45\x56\x82"
                            "\x00\x00\xa4\x82\xeb\xf7\x88\x8c\x30\x5b\xfe\x6f"
                            "\xd6\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint64_t*)0x20000490 = 0;
  *(uint64_t*)0x20000498 = 0;
  *(uint32_t*)0x200004a0 = 0;
  syscall(__NR_ioctl, r[0], 0x4c09, 0x20000400ul);
  memcpy((void*)0x20000280, "/dev/loop#\000", 11);
  res = -1;
  res = syz_open_dev(0x20000280, 0, 0x100082);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x200003c0,
         "\006\000\000!"
         "\000\000\000\000u\206e\000\000\t\000\251\003\000\000\326\373mZ\204~"
         "\236\366\267\300\212c\206l\377\177\000\000\220`"
         "\304\361L\231\006\000\341\vn\340\215\212\362g\353\242q\202\326\037"
         "\230\330\306k\353\262O\214\201Y\274*"
         "\263\263\177t\253\354oB\314\0338\2429\247`"
         "\221\353\017\242\245r\0164\t\343\277\3243\201\246\031\341\354\205\351"
         "+\0317\361\323\342\306\373L\221\337\305\262\234\242\a3\262\213N\031"
         "\271x-\344{\335~\345\277\300]"
         "l\367\000\000\000\000\000\000\000\000\000\000\000",
         151);
  res = syscall(__NR_memfd_create, 0x200003c0ul, 0ul);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x20001400 = 0x200007c0;
  memcpy((void*)0x200007c0, "\xcd", 1);
  *(uint64_t*)0x20001408 = 1;
  syscall(__NR_pwritev, r[2], 0x20001400ul, 1ul, 0x8180a, 0);
  syscall(__NR_ioctl, r[1], 0x4c00, r[2]);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
