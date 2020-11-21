// KASAN: vmalloc-out-of-bounds Read in bpf_trace_run2
// https://syzkaller.appspot.com/bug?id=845923d2172947529b58
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

static unsigned long long procid;

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

static int inject_fault(int nth)
{
  int fd;
  fd = open("/proc/thread-self/fail-nth", O_RDWR);
  if (fd == -1)
    exit(1);
  char buf[16];
  sprintf(buf, "%d", nth + 1);
  if (write(fd, buf, strlen(buf)) != (ssize_t)strlen(buf))
    exit(1);
  return fd;
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

static void setup_fault()
{
  static struct {
    const char* file;
    const char* val;
    bool fatal;
  } files[] = {
      {"/sys/kernel/debug/failslab/ignore-gfp-wait", "N", true},
      {"/sys/kernel/debug/fail_futex/ignore-private", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/ignore-gfp-highmem", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/ignore-gfp-wait", "N", false},
      {"/sys/kernel/debug/fail_page_alloc/min-order", "0", false},
  };
  unsigned i;
  for (i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    if (!write_file(files[i].file, files[i].val)) {
      if (files[i].fatal)
        exit(1);
    }
  }
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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  *(uint32_t*)0x20000440 = 0x11;
  *(uint32_t*)0x20000444 = 6;
  *(uint64_t*)0x20000448 = 0x200004c0;
  memcpy((void*)0x200004c0,
         "\x05\x00\x00\x00\x00\x00\x00\x00\x61\x11\x0c\x00\x00\x00\x00\x00\x85"
         "\x10\x00\x00\x02\x00\x00\x00\x85\x00\x00\x00\x05\x00\x00\x00\x95\x00"
         "\x00\x00\x00\x00\x00\x00\x95\x00\xa5\x05\x00\x00\x00\x00\x77\x51\xe8"
         "\xba\x63\x9a\x67\x88\xa3\x41\xcc\xa5\x55\xfe\xdb\xe9\xd8\xf3\xb4\x23"
         "\xcd\xac\xfa\x7e\x32\xfe\xff\x31\x36\x8b\x22\x64\xf9\xc5\x04\xc9\xf1"
         "\xf6\x55\x15\xb0\xe1\xa3\x8d\x86\x65\x52\x2b\xe1\x8b\xd1\x0a\x48\xb0"
         "\x43\xcc\xc4\x26\x46\xd2\x42\xfd\x73\xd0\x6d\x75\x35\xf7\x86\x69\x07"
         "\xdc\x67\x51\xdf\xb2\x65\xa0\xe3\xcc\xae\x66\x9e\x17\x3a\x64\x9c\x1c"
         "\xfd\x65\x87\xd4\x52\xd6\x4e\x7c\xc9\x57\xd7\x75\x78\xf4\xc3\x52\x35"
         "\x13\x8d\x55\x21\xa0\x89\xfa\x43\xda\xd2\x43\x01\xa4\xf9\x45\x35\x59"
         "\xc3\x5d\xa8\x59\x12\xb4\x52\x6f\x2b\x2a\x3e\x31\x73\xd5\x66\x1c\xfe"
         "\xec\x79\xc6\x6c\x54\xc3\x07\xb7\x90\x06\x3d\x0d\xfe\x98\x9b\x7d\x22"
         "\x90\x43\x15\x64\xb4\xa1\xcd\xac\xd0\xec\x1f\x16\x2e\x56\x50\x6e\x31"
         "\x3d\xfa\x50\x52\xec\x7b\xe9\xf7\xc6\xae\x0c\xd4\x78\x31\xfa\xda\x8c"
         "\x9a\x0b\xa9\x0f\x4d\x0f\x41\xc8\xff\xf6\x48\xd4\x8e\xe2\x99\x40\xf4"
         "\x49\x01\xa4\xba\xac\x0e\x01\x16\xd2\x89\x17\x85\x75\x56\x9a\xfc\xfe"
         "\x5a\xb3\x7c\xb0\xd0\x8c\x0c\x6e\x8a\xd8\x53\x15\xf1\x63\x08\xee\x85"
         "\x5c\x8e\xb2\x7b\x47\x05\x52\xa6\x8c\x94\xcd\x7d\x40\x80\xa3\x57\xd9"
         "\xe3\x73\x8a\x13\x84\xe3\x88\x10\xaa\xfd\x24\xb2\x7e\x1b\x74\x49\x6f"
         "\x7f\x56\xf7\x82\xa2\xbd\xa8\x03\xfb\x02\x04\x3d\x51\x1d\x33\xb2\x96"
         "\x65\xb0\x7e\xe0\x8b\xbd\x5c\xd3\x47\x41\x2f\x0a\xed\x8d\xcf\xfc\x5d"
         "\xbb\xa5\x0d\x70\x19\x94\x48\xa2\x16\xe3\xaf\x1a\x2f\xb5\x4c\xad\xb7"
         "\x47\x9f\x5b\x85\x30\x04\x40\xf1\xf5\xd3\x1e\xa3\xef\xe9\xf4\xf9\xc2"
         "\xde\xfd\xa2\xca\x6b\x93\x6f\xc4\x75\x32\x08\xc2\x38\x24\x71\xb6\x35"
         "\x54\x2c\x59\xfa\x43\xb6\x75\xbf\xd2\x5e\xf3\x92\xd9\xc3\x98\xd3\x58"
         "\xa0\xe9\x18\x94\xe8\x15\xef\x5b\x47\xa9\x5d\xc4\xfe\xb7\xcb\x7a\x44"
         "\x8d\xce\x00\xa3\x2b\x77\x43\xd9\x7d\x2d\x1d\x56\xa0\x10\x3c\x91\xa8"
         "\x86\x10\x1f\xe1\x97\x4b\x0d\xe8\xd6\xd8\x89\x74\x20\xd5\x42",
         474);
  *(uint64_t*)0x20000450 = 0x20000080;
  memcpy((void*)0x20000080, "GPL\000", 4);
  *(uint32_t*)0x20000458 = 5;
  *(uint32_t*)0x2000045c = 0x29e;
  *(uint64_t*)0x20000460 = 0x2000cf3d;
  *(uint32_t*)0x20000468 = 0;
  *(uint32_t*)0x2000046c = 0;
  *(uint8_t*)0x20000470 = 0;
  *(uint8_t*)0x20000471 = 0;
  *(uint8_t*)0x20000472 = 0;
  *(uint8_t*)0x20000473 = 0;
  *(uint8_t*)0x20000474 = 0;
  *(uint8_t*)0x20000475 = 0;
  *(uint8_t*)0x20000476 = 0;
  *(uint8_t*)0x20000477 = 0;
  *(uint8_t*)0x20000478 = 0;
  *(uint8_t*)0x20000479 = 0;
  *(uint8_t*)0x2000047a = 0;
  *(uint8_t*)0x2000047b = 0;
  *(uint8_t*)0x2000047c = 0;
  *(uint8_t*)0x2000047d = 0;
  *(uint8_t*)0x2000047e = 0;
  *(uint8_t*)0x2000047f = 0;
  *(uint32_t*)0x20000480 = 0;
  *(uint32_t*)0x20000484 = 0;
  *(uint32_t*)0x20000488 = -1;
  *(uint32_t*)0x2000048c = 6;
  *(uint64_t*)0x20000490 = 0;
  *(uint32_t*)0x20000498 = 0;
  *(uint32_t*)0x2000049c = 0x10;
  *(uint64_t*)0x200004a0 = 0;
  *(uint32_t*)0x200004a8 = 0;
  *(uint32_t*)0x200004ac = 0;
  *(uint32_t*)0x200004b0 = -1;
  res = syscall(__NR_bpf, 5ul, 0x20000440ul, 0x70ul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x200001c0 = 0x20000200;
  memcpy((void*)0x20000200, "sys_enter\000", 10);
  *(uint32_t*)0x200001c8 = r[0];
  res = syscall(__NR_bpf, 0x11ul, 0x200001c0ul, 0x10ul);
  if (res != -1)
    r[1] = res;
  syscall(__NR_bpf, 0x20ul, 0ul, 0ul);
  inject_fault(0);
  syscall(__NR_close, r[1]);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  setup_fault();
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
