// general protection fault in get_work_pool
// https://syzkaller.appspot.com/bug?id=18cfd49c158d37dd54481bd376cec513444cd571
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
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

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 160 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 256;
  setrlimit(RLIMIT_NOFILE, &rlim);
  if (unshare(CLONE_NEWNS)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(0x02000000)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
}

int wait_for_loop(int pid)
{
  if (pid < 0)
    exit(1);
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);
  setup_common();
  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }
  loop();
  exit(1);
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

uint64_t r[2] = {0xffffffffffffffff, 0x0};

void execute_one(void)
{
  long res = 0;
  res = syscall(__NR_socket, 0x10, 3, 6);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000300 = 0x20000000;
  *(uint16_t*)0x20000000 = 0x10;
  *(uint16_t*)0x20000002 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x20000308 = 0xc;
  *(uint64_t*)0x20000310 = 0x200002c0;
  *(uint64_t*)0x200002c0 = 0x20000100;
  *(uint64_t*)0x200002c8 = 1;
  *(uint64_t*)0x20000318 = 1;
  *(uint64_t*)0x20000320 = 0;
  *(uint64_t*)0x20000328 = 0;
  *(uint32_t*)0x20000330 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000300, 0);
  *(uint32_t*)0x20000180 = 0x80;
  res = syscall(__NR_getsockname, r[0], 0x20000540, 0x20000180);
  if (res != -1)
    r[1] = *(uint32_t*)0x20000544;
  *(uint64_t*)0x20000500 = 0x200000c0;
  *(uint16_t*)0x200000c0 = 0x10;
  *(uint16_t*)0x200000c2 = 0;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = 0x20000000;
  *(uint32_t*)0x20000508 = 0xc;
  *(uint64_t*)0x20000510 = 0x20000280;
  *(uint64_t*)0x20000280 = 0x20000340;
  *(uint32_t*)0x20000340 = 0x150;
  *(uint16_t*)0x20000344 = 0x19;
  *(uint16_t*)0x20000346 = 0x309;
  *(uint32_t*)0x20000348 = 0x70bd26;
  *(uint32_t*)0x2000034c = 0x25dfdbfe;
  *(uint8_t*)0x20000350 = 0;
  *(uint8_t*)0x20000351 = 0;
  *(uint8_t*)0x20000352 = 0;
  *(uint8_t*)0x20000353 = 0;
  *(uint8_t*)0x20000354 = 0;
  *(uint8_t*)0x20000355 = 0;
  *(uint8_t*)0x20000356 = 0;
  *(uint8_t*)0x20000357 = 0;
  *(uint8_t*)0x20000358 = 0;
  *(uint8_t*)0x20000359 = 0;
  *(uint8_t*)0x2000035a = -1;
  *(uint8_t*)0x2000035b = -1;
  *(uint32_t*)0x2000035c = htobe32(0xe0000001);
  *(uint64_t*)0x20000360 = htobe64(0);
  *(uint64_t*)0x20000368 = htobe64(1);
  *(uint16_t*)0x20000370 = htobe16(0x4e23);
  *(uint16_t*)0x20000372 = htobe16(0xf11);
  *(uint16_t*)0x20000374 = htobe16(0x4e20);
  *(uint16_t*)0x20000376 = htobe16(0x1ff);
  *(uint16_t*)0x20000378 = 0xa;
  *(uint8_t*)0x2000037a = 0x20;
  *(uint8_t*)0x2000037b = 0x80;
  *(uint8_t*)0x2000037c = 0x2b;
  *(uint32_t*)0x20000380 = r[1];
  *(uint32_t*)0x20000384 = 0;
  *(uint64_t*)0x20000388 = 0x6a;
  *(uint64_t*)0x20000390 = 0xfffffffffffffff8;
  *(uint64_t*)0x20000398 = 4;
  *(uint64_t*)0x200003a0 = 8;
  *(uint64_t*)0x200003a8 = 0x200000;
  *(uint64_t*)0x200003b0 = 1;
  *(uint64_t*)0x200003b8 = 0x17c;
  *(uint64_t*)0x200003c0 = -1;
  *(uint64_t*)0x200003c8 = 9;
  *(uint64_t*)0x200003d0 = 0xc3ef;
  *(uint64_t*)0x200003d8 = 4;
  *(uint64_t*)0x200003e0 = 0x400;
  *(uint32_t*)0x200003e8 = 0;
  *(uint32_t*)0x200003ec = 0x6e6bb6;
  *(uint8_t*)0x200003f0 = 2;
  *(uint8_t*)0x200003f1 = 1;
  *(uint8_t*)0x200003f2 = 0;
  *(uint8_t*)0x200003f3 = 0;
  *(uint16_t*)0x200003f8 = 0x48;
  *(uint16_t*)0x200003fa = 3;
  memcpy((void*)0x200003fc,
         "\x64\x65\x66\x6c\x61\x74\x65\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  *(uint32_t*)0x2000043c = 0;
  *(uint16_t*)0x20000440 = 0x48;
  *(uint16_t*)0x20000442 = 3;
  memcpy((void*)0x20000444,
         "\x6c\x7a\x6a\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  *(uint32_t*)0x20000484 = 0;
  *(uint16_t*)0x20000488 = 8;
  *(uint16_t*)0x2000048a = 0x16;
  *(uint32_t*)0x2000048c = 8;
  *(uint64_t*)0x20000288 = 0x150;
  *(uint64_t*)0x20000518 = 1;
  *(uint64_t*)0x20000520 = 0;
  *(uint64_t*)0x20000528 = 0;
  *(uint32_t*)0x20000530 = 0x880;
  syscall(__NR_sendmsg, r[0], 0x20000500, 0xc800);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      do_sandbox_none();
    }
  }
  sleep(1000000);
  return 0;
}
