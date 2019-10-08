// BUG: corrupted list in remove_wait_queue
// https://syzkaller.appspot.com/bug?id=4aeceeb1204dfc6707f976a7a702e18b773b26f6
// status:open
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
  write_file("/proc/self/oom_score_adj", "1000");
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
  res = syscall(__NR_socket, 0x1e, 1, 0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x203bbfc8 = 0x20fdbf80;
  *(uint16_t*)0x20fdbf80 = 0x1e;
  memcpy((void*)0x20fdbf82,
         "\x02\xff\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x0a\xe7\x7f"
         "\x5b\xf8\x6c\x48\x02\x00\x02\x00\x00\x00\xf1\xff\xff\xff\x00\x9a\x48"
         "\x00\x75\xe6\xa5\x00\x00\xde\x01\x03\x00\x00\x00\x00\xe4\xff\x06\x4b"
         "\x3f\x01\x3a\x00\x00\x00\x08\x00\x00\x00\x8f\x00\x00\x00\x00\xac\x50"
         "\xd5\xfe\x32\xc4\x00\x00\x00\x00\x7f\xff\xff\xff\x6a\x00\x83\x56\xed"
         "\xb9\xa6\x34\x1c\x1f\xd4\x56\x24\x28\x1e\x00\x07\x0e\xcd\xdd\x02\x06"
         "\xc3\x97\x50\xc4\x00\x00\xfd\x00\x00\x09\x00\x00\x00\x00\x00\x0b\x00"
         "\x00\xdb\x00\x00\x04\xda\x36",
         126);
  *(uint32_t*)0x203bbfd0 = 0x80;
  *(uint64_t*)0x203bbfd8 = 0;
  *(uint64_t*)0x203bbfe0 = 0;
  *(uint64_t*)0x203bbfe8 = 0;
  *(uint64_t*)0x203bbff0 = 0;
  *(uint32_t*)0x203bbff8 = 0;
  syscall(__NR_sendmsg, r[0], 0x203bbfc8, 0);
  *(uint32_t*)0x200000c0 = 5;
  *(uint16_t*)0x200000c4 = 0x10;
  *(uint16_t*)0x200000c6 = 0xfa00;
  *(uint64_t*)0x200000c8 = 0x20000280;
  *(uint32_t*)0x200000d0 = -1;
  *(uint32_t*)0x200000d4 = 0;
  syscall(__NR_write, r[0], 0x200000c0, 0x152610);
  syscall(__NR_bind, -1, 0, 0);
  syscall(__NR_ioctl, -1, 0xc0045516, 0);
  syscall(__NR_setsockopt, -1, 0, 0x48c, 0, 0);
  syscall(__NR_signalfd4, -1, 0, 0, 0x80800);
  syscall(__NR_ioctl, -1, 0xae41, 0);
  syscall(__NR_setsockopt, -1, 0x28, 2, 0, 0);
  syscall(__NR_ioctl, -1, 0x800000000008982, 0);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0, 0, 0);
  if (res != -1)
    r[1] = res;
  syscall(__NR_socket, 0x10, 1, 0);
  syscall(__NR_ioctl, r[1], 0xae01, 0);
  syscall(__NR_getsockopt, -1, 0x84, 0x20, 0, 0);
  syscall(__NR_mremap, 0x20ffd000, 0x3000, 0x4000, 0, 0x20ffc000);
  syscall(__NR_openat, 0xffffff9c, 0, 0, 0);
  syscall(__NR_setsockopt, -1, 1, 0x1a, 0, 0);
  syscall(__NR_semctl, 0, 3, 3, 0);
  syscall(__NR_setsockopt, -1, 0x84, 0x12, 0, 0);
  syz_open_dev(0, 0, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
