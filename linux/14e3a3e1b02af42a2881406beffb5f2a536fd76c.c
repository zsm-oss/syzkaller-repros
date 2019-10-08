// INFO: task hung in ext4_fallocate
// https://syzkaller.appspot.com/bug?id=14e3a3e1b02af42a2881406beffb5f2a536fd76c
// status:fixed
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
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

unsigned long long procid;

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

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  int i;
  for (i = 0; i < 100; i++) {
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

#define SYZ_HAVE_SETUP_TEST 1
static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
}

#define SYZ_HAVE_RESET_TEST 1
static void reset_test()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
    close(fd);
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
      reset_test();
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  long res = 0;
  memcpy((void*)0x20000080,
         "\x73\x74\x61\x74\x09\xc0\xd2\xfe\xbc\xf9\xdf\x2d\xea\xc8\xc1\x77\xff"
         "\x17\x12\x48\xe9\x11\x93\x51\x30\x49\xf8\x31\x55\x0d\x6f\x7d\xe6\x6c"
         "\xf6\x37\xbd\xbf\x13\x11\x92\x0c\x8a\x26\xed\xa4\xdc\xc3\x78\x3f\x9d"
         "\xb5\x11\x6b\x34\xd3\x1b\x05\x12\xa5\x60\x8a\xaf\xf0\x1e\x79\x52\x34"
         "\x0c\xd6\xfd\x00\x00\x00\x00",
         75);
  res = syscall(__NR_openat, 0xffffff9c, 0x20000080, 0x275a, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000000, "./file0", 8);
  res = syscall(__NR_creat, 0x20000000, 0);
  if (res != -1)
    r[1] = res;
  syscall(__NR_fallocate, r[1], 0, 0, 0x4000fff);
  syscall(__NR_fallocate, r[0], 0, 0, 0x10001);
  *(uint32_t*)0x20000040 = 0;
  *(uint32_t*)0x20000044 = r[1];
  *(uint64_t*)0x20000048 = 0;
  *(uint64_t*)0x20000050 = 0xfffffff9;
  *(uint64_t*)0x20000058 = 0;
  *(uint64_t*)0x20000060 = 0;
  syscall(__NR_ioctl, r[0], 0xc028660f, 0x20000040);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
