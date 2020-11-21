// BUG: sleeping function called from invalid context in do_page_fault
// https://syzkaller.appspot.com/bug?id=7f59c1e54e5ce4d95cf7
// status:3 arch:386
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

#ifndef __NR_bind
#define __NR_bind 361
#endif
#ifndef __NR_bpf
#define __NR_bpf 357
#endif
#ifndef __NR_dup2
#define __NR_dup2 63
#endif
#ifndef __NR_eventfd2
#define __NR_eventfd2 328
#endif
#ifndef __NR_getpgrp
#define __NR_getpgrp 65
#endif
#ifndef __NR_getsockopt
#define __NR_getsockopt 365
#endif
#ifndef __NR_ioctl
#define __NR_ioctl 54
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_mprotect
#define __NR_mprotect 125
#endif
#ifndef __NR_openat
#define __NR_openat 295
#endif
#ifndef __NR_perf_event_open
#define __NR_perf_event_open 336
#endif
#ifndef __NR_pipe
#define __NR_pipe 42
#endif
#ifndef __NR_process_vm_writev
#define __NR_process_vm_writev 348
#endif
#ifndef __NR_sendmsg
#define __NR_sendmsg 370
#endif
#ifndef __NR_setsockopt
#define __NR_setsockopt 366
#endif
#ifndef __NR_shmget
#define __NR_shmget 395
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#ifndef __NR_write
#define __NR_write 4
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  syscall(__NR_openat, 0xffffff9c, 0, 0, 0);
  *(uint32_t*)0x2001d000 = 1;
  *(uint32_t*)0x2001d004 = 0x70;
  *(uint8_t*)0x2001d008 = 0;
  *(uint8_t*)0x2001d009 = 0;
  *(uint8_t*)0x2001d00a = 0;
  *(uint8_t*)0x2001d00b = 0;
  *(uint32_t*)0x2001d00c = 0;
  *(uint64_t*)0x2001d010 = 0x7f;
  *(uint64_t*)0x2001d018 = 0;
  *(uint64_t*)0x2001d020 = 0;
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0x81, 5, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 29, 35);
  *(uint32_t*)0x2001d030 = 0;
  *(uint32_t*)0x2001d034 = 0;
  *(uint64_t*)0x2001d038 = 0;
  *(uint64_t*)0x2001d040 = 0;
  *(uint64_t*)0x2001d048 = 0;
  *(uint64_t*)0x2001d050 = 0;
  *(uint32_t*)0x2001d058 = 0;
  *(uint32_t*)0x2001d05c = 0;
  *(uint64_t*)0x2001d060 = 0;
  *(uint32_t*)0x2001d068 = 0;
  *(uint16_t*)0x2001d06c = 0;
  *(uint16_t*)0x2001d06e = 0;
  syscall(__NR_perf_event_open, 0x2001d000, 0, -1, -1, 0);
  syscall(__NR_mprotect, 0x20005000, 0x3000, 0);
  syscall(__NR_setsockopt, -1, 6, 0xd, 0, 0);
  syscall(__NR_pipe, 0);
  syscall(__NR_perf_event_open, 0, 0, -1, -1, 0);
  syscall(__NR_shmget, 0x798dd814, 0x4000, 0, 0x2096d000);
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  syscall(__NR_process_vm_writev, 0, 0x20000000, 1, 0, 0, 0);
  syscall(__NR_bind, -1, 0, 0);
  syscall(__NR_getpgrp, 0);
  syscall(__NR_bpf, 0xf, 0, 0);
  syscall(__NR_openat, 0xffffff9c, 0, 0, 0);
  syscall(__NR_write, -1, 0, 0);
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint32_t*)0x20000010 = 0;
  *(uint32_t*)0x20000014 = 0;
  *(uint32_t*)0x20000018 = 0;
  syscall(__NR_setsockopt, -1, 0x10f, 0x87, 0x20000000, 0x1c);
  syscall(__NR_dup2, -1, -1);
  syscall(__NR_eventfd2, 0, 0);
  syscall(__NR_getsockopt, -1, 0x65, 6, 0, 0);
  memcpy((void*)0x20000480, "/dev/fb0\000", 9);
  res = syscall(__NR_openat, 0xffffff9c, 0x20000480, 0, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000000 = 0x356;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0x280;
  *(uint32_t*)0x20000010 = 0;
  *(uint32_t*)0x20000014 = 0x4b76;
  *(uint32_t*)0x20000018 = 8;
  *(uint32_t*)0x2000001c = 0;
  *(uint32_t*)0x20000020 = 0;
  *(uint32_t*)0x20000024 = 0;
  *(uint32_t*)0x20000028 = 0;
  *(uint32_t*)0x2000002c = 0;
  *(uint32_t*)0x20000030 = 0;
  *(uint32_t*)0x20000034 = 0;
  *(uint32_t*)0x20000038 = 0;
  *(uint32_t*)0x2000003c = 0;
  *(uint32_t*)0x20000040 = 0;
  *(uint32_t*)0x20000044 = 0;
  *(uint32_t*)0x20000048 = 0;
  *(uint32_t*)0x2000004c = 0;
  *(uint32_t*)0x20000050 = 0;
  *(uint32_t*)0x20000054 = 0;
  *(uint32_t*)0x20000058 = 0;
  *(uint32_t*)0x2000005c = 0;
  *(uint32_t*)0x20000060 = 0;
  *(uint32_t*)0x20000064 = 0;
  *(uint32_t*)0x20000068 = 0;
  *(uint32_t*)0x2000006c = 0;
  *(uint32_t*)0x20000070 = 0;
  *(uint32_t*)0x20000074 = 0;
  *(uint32_t*)0x20000078 = 0;
  *(uint32_t*)0x2000007c = 0;
  *(uint32_t*)0x20000080 = 0;
  *(uint32_t*)0x20000084 = 0;
  *(uint32_t*)0x20000088 = 0;
  *(uint32_t*)0x2000008c = 0;
  *(uint32_t*)0x20000090 = 0;
  *(uint32_t*)0x20000094 = 0;
  *(uint32_t*)0x20000098 = 0;
  *(uint32_t*)0x2000009c = 0;
  syscall(__NR_ioctl, (intptr_t)r[0], 0x4601, 0x20000000);
  *(uint32_t*)0x2001d000 = 1;
  *(uint32_t*)0x2001d004 = 0x70;
  *(uint8_t*)0x2001d008 = 0;
  *(uint8_t*)0x2001d009 = 0;
  *(uint8_t*)0x2001d00a = 0;
  *(uint8_t*)0x2001d00b = 0;
  *(uint32_t*)0x2001d00c = 0;
  *(uint64_t*)0x2001d010 = 0x41c1;
  *(uint64_t*)0x2001d018 = 0;
  *(uint64_t*)0x2001d020 = 0;
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 3, 5, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 29, 35);
  *(uint32_t*)0x2001d030 = 0;
  *(uint32_t*)0x2001d034 = 0;
  *(uint64_t*)0x2001d038 = 0;
  *(uint64_t*)0x2001d040 = 0;
  *(uint64_t*)0x2001d048 = 0;
  *(uint64_t*)0x2001d050 = 0;
  *(uint32_t*)0x2001d058 = 0;
  *(uint32_t*)0x2001d05c = 0;
  *(uint64_t*)0x2001d060 = 0;
  *(uint32_t*)0x2001d068 = 0;
  *(uint16_t*)0x2001d06c = 0;
  *(uint16_t*)0x2001d06e = 0;
  syscall(__NR_perf_event_open, 0x2001d000, 0, -1, -1, 0);
  res = syscall(__NR_socket, 0x11, 3, 0x300);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  syscall(__NR_setsockopt, (intptr_t)r[1], 1, 0x1a, 0x20000000, 8);
  *(uint32_t*)0x20001140 = 0x20000040;
  *(uint16_t*)0x20000040 = 0x10;
  *(uint16_t*)0x20000042 = 0;
  *(uint32_t*)0x20000044 = 0;
  *(uint32_t*)0x20000048 = 0x100000;
  *(uint32_t*)0x20001144 = 0xc;
  *(uint32_t*)0x20001148 = 0x200000c0;
  *(uint32_t*)0x200000c0 = 0x20000080;
  *(uint32_t*)0x20000080 = 0x1c;
  *(uint8_t*)0x20000084 = 2;
  *(uint8_t*)0x20000085 = 2;
  *(uint16_t*)0x20000086 = 5;
  *(uint32_t*)0x20000088 = 0;
  *(uint32_t*)0x2000008c = 0;
  *(uint8_t*)0x20000090 = 1;
  *(uint8_t*)0x20000091 = 0;
  *(uint16_t*)0x20000092 = htobe16(9);
  *(uint16_t*)0x20000094 = 8;
  *(uint16_t*)0x20000096 = 0xb;
  memcpy((void*)0x20000098, "sip\000", 4);
  *(uint32_t*)0x200000c4 = 0x1c;
  *(uint32_t*)0x2000114c = 1;
  *(uint32_t*)0x20001150 = 0;
  *(uint32_t*)0x20001154 = 0;
  *(uint32_t*)0x20001158 = 0x4004814;
  syscall(__NR_sendmsg, -1, 0x20001140, 0x6000880);
  memcpy((void*)0x20000180, "/dev/fb0\000", 9);
  res = syscall(__NR_openat, 0xffffff9c, 0x20000180, 0, 0);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x20000340 = 0x30;
  *(uint32_t*)0x20000344 = 0x10;
  *(uint32_t*)0x20000348 = 0;
  *(uint32_t*)0x2000034c = 0;
  *(uint32_t*)0x20000350 = 0;
  *(uint32_t*)0x20000354 = 0;
  *(uint32_t*)0x20000358 = 4;
  *(uint32_t*)0x2000035c = 0;
  *(uint32_t*)0x20000360 = 0;
  *(uint32_t*)0x20000364 = 0x235df24b;
  *(uint32_t*)0x20000368 = 0;
  *(uint32_t*)0x2000036c = 0;
  *(uint32_t*)0x20000370 = 0;
  *(uint32_t*)0x20000374 = 0;
  *(uint32_t*)0x20000378 = 0;
  *(uint32_t*)0x2000037c = 0;
  *(uint32_t*)0x20000380 = 0;
  *(uint32_t*)0x20000384 = 1;
  *(uint32_t*)0x20000388 = 0;
  *(uint32_t*)0x2000038c = 0;
  *(uint32_t*)0x20000390 = 0;
  *(uint32_t*)0x20000394 = 0;
  *(uint32_t*)0x20000398 = 0;
  *(uint32_t*)0x2000039c = 0;
  *(uint32_t*)0x200003a0 = 0;
  *(uint32_t*)0x200003a4 = 0;
  *(uint32_t*)0x200003a8 = 0;
  *(uint32_t*)0x200003ac = 0;
  *(uint32_t*)0x200003b0 = 0;
  *(uint32_t*)0x200003b4 = 0;
  *(uint32_t*)0x200003b8 = 0;
  *(uint32_t*)0x200003bc = 0;
  *(uint32_t*)0x200003c0 = 0;
  *(uint32_t*)0x200003c4 = 0;
  *(uint32_t*)0x200003c8 = 0;
  *(uint32_t*)0x200003cc = 0;
  *(uint32_t*)0x200003d0 = 0;
  *(uint32_t*)0x200003d4 = 0;
  *(uint32_t*)0x200003d8 = 0;
  *(uint32_t*)0x200003dc = 0;
  syscall(__NR_ioctl, (intptr_t)r[2], 0x4601, 0x20000340);
  res = syscall(__NR_socket, 0x1d, 3, 1);
  if (res != -1)
    r[3] = res;
  syscall(__NR_getsockopt, (intptr_t)r[3], 0x65, 6, 0, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
