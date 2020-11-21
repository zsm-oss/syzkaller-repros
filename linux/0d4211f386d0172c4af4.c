// KASAN: use-after-free Read in snd_seq_timer_interrupt
// https://syzkaller.appspot.com/bug?id=0d4211f386d0172c4af4
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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  memcpy((void*)0x200000c0,
         "queue1\000\000\000\000\000\000\000\0001;"
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\001\000\000\006\000\000\000\000\000\314\277}"
         "\335\000\000\000\000\000\000\000\000\000\000\000\000\000\262\036\000",
         62);
  res = syscall(__NR_memfd_create, 0x200000c0ul, 0ul);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000700, "/dev/snd/seq\000", 13);
  res = syz_open_dev(0x20000700, 0, 1);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_dup2, r[1], r[0]);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x20000200 = 0;
  *(uint32_t*)0x20000204 = 0;
  STORE_BY_BITMASK(uint32_t, , 0x20000208, 0, 0, 1);
  memcpy((void*)0x20000209, "queue1\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000",
         64);
  *(uint32_t*)0x2000024c = 0;
  *(uint8_t*)0x20000250 = 0;
  *(uint8_t*)0x20000251 = 0;
  *(uint8_t*)0x20000252 = 0;
  *(uint8_t*)0x20000253 = 0;
  *(uint8_t*)0x20000254 = 0;
  *(uint8_t*)0x20000255 = 0;
  *(uint8_t*)0x20000256 = 0;
  *(uint8_t*)0x20000257 = 0;
  *(uint8_t*)0x20000258 = 0;
  *(uint8_t*)0x20000259 = 0;
  *(uint8_t*)0x2000025a = 0;
  *(uint8_t*)0x2000025b = 0;
  *(uint8_t*)0x2000025c = 0;
  *(uint8_t*)0x2000025d = 0;
  *(uint8_t*)0x2000025e = 0;
  *(uint8_t*)0x2000025f = 0;
  *(uint8_t*)0x20000260 = 0;
  *(uint8_t*)0x20000261 = 0;
  *(uint8_t*)0x20000262 = 0;
  *(uint8_t*)0x20000263 = 0;
  *(uint8_t*)0x20000264 = 0;
  *(uint8_t*)0x20000265 = 0;
  *(uint8_t*)0x20000266 = 0;
  *(uint8_t*)0x20000267 = 0;
  *(uint8_t*)0x20000268 = 0;
  *(uint8_t*)0x20000269 = 0;
  *(uint8_t*)0x2000026a = 0;
  *(uint8_t*)0x2000026b = 0;
  *(uint8_t*)0x2000026c = 0;
  *(uint8_t*)0x2000026d = 0;
  *(uint8_t*)0x2000026e = 0;
  *(uint8_t*)0x2000026f = 0;
  *(uint8_t*)0x20000270 = 0;
  *(uint8_t*)0x20000271 = 0;
  *(uint8_t*)0x20000272 = 0;
  *(uint8_t*)0x20000273 = 0;
  *(uint8_t*)0x20000274 = 0;
  *(uint8_t*)0x20000275 = 0;
  *(uint8_t*)0x20000276 = 0;
  *(uint8_t*)0x20000277 = 0;
  *(uint8_t*)0x20000278 = 0;
  *(uint8_t*)0x20000279 = 0;
  *(uint8_t*)0x2000027a = 0;
  *(uint8_t*)0x2000027b = 0;
  *(uint8_t*)0x2000027c = 0;
  *(uint8_t*)0x2000027d = 0;
  *(uint8_t*)0x2000027e = 0;
  *(uint8_t*)0x2000027f = 0;
  *(uint8_t*)0x20000280 = 0;
  *(uint8_t*)0x20000281 = 0;
  *(uint8_t*)0x20000282 = 0;
  *(uint8_t*)0x20000283 = 0;
  *(uint8_t*)0x20000284 = 0;
  *(uint8_t*)0x20000285 = 0;
  *(uint8_t*)0x20000286 = 0;
  *(uint8_t*)0x20000287 = 0;
  *(uint8_t*)0x20000288 = 0;
  *(uint8_t*)0x20000289 = 0;
  *(uint8_t*)0x2000028a = 0;
  *(uint8_t*)0x2000028b = 0;
  syscall(__NR_ioctl, r[2], 0xc08c5332, 0x20000200ul);
  *(uint32_t*)0x20000100 = 0;
  *(uint32_t*)0x20000104 = 0;
  *(uint32_t*)0x20000108 = 2;
  *(uint32_t*)0x2000010c = 0;
  *(uint32_t*)0x20000110 = 0;
  *(uint32_t*)0x20000114 = 0;
  *(uint32_t*)0x20000118 = 0;
  *(uint32_t*)0x2000011c = 0xfffffffc;
  *(uint8_t*)0x20000120 = 0;
  *(uint8_t*)0x20000121 = 0;
  *(uint8_t*)0x20000122 = 0;
  *(uint8_t*)0x20000123 = 0;
  *(uint8_t*)0x20000124 = 0;
  *(uint8_t*)0x20000125 = 0;
  *(uint8_t*)0x20000126 = 0;
  *(uint8_t*)0x20000127 = 0;
  *(uint8_t*)0x20000128 = 0;
  *(uint8_t*)0x20000129 = 0;
  *(uint8_t*)0x2000012a = 0;
  *(uint8_t*)0x2000012b = 0;
  *(uint8_t*)0x2000012c = 0;
  *(uint8_t*)0x2000012d = 0;
  *(uint8_t*)0x2000012e = 0;
  *(uint8_t*)0x2000012f = 0;
  *(uint8_t*)0x20000130 = 0;
  *(uint8_t*)0x20000131 = 0;
  *(uint8_t*)0x20000132 = 0;
  *(uint8_t*)0x20000133 = 0;
  *(uint8_t*)0x20000134 = 0;
  *(uint8_t*)0x20000135 = 0;
  *(uint8_t*)0x20000136 = 0;
  *(uint8_t*)0x20000137 = 0;
  *(uint8_t*)0x20000138 = 0;
  *(uint8_t*)0x20000139 = 0;
  *(uint8_t*)0x2000013a = 0;
  *(uint8_t*)0x2000013b = 0;
  *(uint8_t*)0x2000013c = 0;
  *(uint8_t*)0x2000013d = 0;
  *(uint8_t*)0x2000013e = 0;
  *(uint8_t*)0x2000013f = 0;
  *(uint8_t*)0x20000140 = 0;
  *(uint8_t*)0x20000141 = 0;
  *(uint8_t*)0x20000142 = 0;
  *(uint8_t*)0x20000143 = 0;
  *(uint8_t*)0x20000144 = 0;
  *(uint8_t*)0x20000145 = 0;
  *(uint8_t*)0x20000146 = 0;
  *(uint8_t*)0x20000147 = 0;
  *(uint8_t*)0x20000148 = 0;
  *(uint8_t*)0x20000149 = 0;
  *(uint8_t*)0x2000014a = 0;
  *(uint8_t*)0x2000014b = 0;
  *(uint8_t*)0x2000014c = 0;
  *(uint8_t*)0x2000014d = 0;
  *(uint8_t*)0x2000014e = 0;
  *(uint8_t*)0x2000014f = 0;
  *(uint8_t*)0x20000150 = 0;
  *(uint8_t*)0x20000151 = 0;
  *(uint8_t*)0x20000152 = 0;
  *(uint8_t*)0x20000153 = 0;
  *(uint8_t*)0x20000154 = 0;
  *(uint8_t*)0x20000155 = 0;
  *(uint8_t*)0x20000156 = 0;
  *(uint8_t*)0x20000157 = 0;
  *(uint8_t*)0x20000158 = 0;
  *(uint8_t*)0x20000159 = 0;
  *(uint8_t*)0x2000015a = 0;
  *(uint8_t*)0x2000015b = 0;
  *(uint8_t*)0x2000015c = 0;
  *(uint8_t*)0x2000015d = 0;
  *(uint8_t*)0x2000015e = 0;
  *(uint8_t*)0x2000015f = 0;
  syscall(__NR_ioctl, r[2], 0x40605346, 0x20000100ul);
  *(uint8_t*)0x20000000 = 0;
  *(uint8_t*)0x20000001 = 0;
  *(uint8_t*)0x20000002 = 0;
  *(uint8_t*)0x20000003 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint8_t*)0x2000000c = 0;
  *(uint8_t*)0x2000000d = 0;
  *(uint8_t*)0x2000000e = 0;
  *(uint8_t*)0x2000000f = 0;
  *(uint8_t*)0x20000010 = 0;
  *(uint8_t*)0x20000011 = 0;
  *(uint8_t*)0x20000012 = 0;
  *(uint8_t*)0x20000013 = 0;
  syscall(__NR_write, r[2], 0x20000000ul, 0x72c6831cul);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0ul);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
