// memory leak in mpihelp_mul_karatsuba_case
// https://syzkaller.appspot.com/bug?id=44f8b2235e763b79f247e3f273ff85e700656526
// status:fixed
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
#include <sys/ioctl.h>
#include <sys/mount.h>
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

static void use_temporary_dir(void)
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    exit(1);
  if (chmod(tmpdir, 0777))
    exit(1);
  if (chdir(tmpdir))
    exit(1);
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

#define FS_IOC_SETFLAGS _IOW('f', 2, long)
static void remove_dir(const char* dir)
{
  DIR* dp;
  struct dirent* ep;
  int iter = 0;
retry:
  while (umount2(dir, MNT_DETACH) == 0) {
  }
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exit(1);
    }
    exit(1);
  }
  while ((ep = readdir(dp))) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", dir, ep->d_name);
    while (umount2(filename, MNT_DETACH) == 0) {
    }
    struct stat st;
    if (lstat(filename, &st))
      exit(1);
    if (S_ISDIR(st.st_mode)) {
      remove_dir(filename);
      continue;
    }
    int i;
    for (i = 0;; i++) {
      if (unlink(filename) == 0)
        break;
      if (errno == EPERM) {
        int fd = open(filename, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0)
            close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno != EBUSY || i > 100)
        exit(1);
      if (umount2(filename, MNT_DETACH))
        exit(1);
    }
  }
  closedir(dp);
  int i;
  for (i = 0;; i++) {
    if (rmdir(dir) == 0)
      break;
    if (i < 100) {
      if (errno == EPERM) {
        int fd = open(dir, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0)
            close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno == EBUSY) {
        if (umount2(dir, MNT_DETACH))
          exit(1);
        continue;
      }
      if (errno == ENOTEMPTY) {
        if (iter < 100) {
          iter++;
          goto retry;
        }
      }
    }
    exit(1);
  }
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

#define KMEMLEAK_FILE "/sys/kernel/debug/kmemleak"

static void setup_leak()
{
  if (!write_file(KMEMLEAK_FILE, "scan"))
    exit(1);
  sleep(5);
  if (!write_file(KMEMLEAK_FILE, "scan"))
    exit(1);
  if (!write_file(KMEMLEAK_FILE, "clear"))
    exit(1);
}

static void check_leaks(void)
{
  int fd = open(KMEMLEAK_FILE, O_RDWR);
  if (fd == -1)
    exit(1);
  uint64_t start = current_time_ms();
  if (write(fd, "scan", 4) != 4)
    exit(1);
  sleep(1);
  while (current_time_ms() - start < 4 * 1000)
    sleep(1);
  if (write(fd, "scan", 4) != 4)
    exit(1);
  static char buf[128 << 10];
  ssize_t n = read(fd, buf, sizeof(buf) - 1);
  if (n < 0)
    exit(1);
  int nleaks = 0;
  if (n != 0) {
    sleep(1);
    if (write(fd, "scan", 4) != 4)
      exit(1);
    if (lseek(fd, 0, SEEK_SET) < 0)
      exit(1);
    n = read(fd, buf, sizeof(buf) - 1);
    if (n < 0)
      exit(1);
    buf[n] = 0;
    char* pos = buf;
    char* end = buf + n;
    while (pos < end) {
      char* next = strstr(pos + 1, "unreferenced object");
      if (!next)
        next = end;
      char prev = *next;
      *next = 0;
      fprintf(stderr, "BUG: memory leak\n%s\n", pos);
      *next = prev;
      pos = next;
      nleaks++;
    }
  }
  if (write(fd, "clear", 5) != 5)
    exit(1);
  close(fd);
  if (nleaks)
    exit(1);
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter;
  for (iter = 0;; iter++) {
    char cwdbuf[32];
    sprintf(cwdbuf, "./%d", iter);
    if (mkdir(cwdbuf, 0777))
      exit(1);
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      if (chdir(cwdbuf))
        exit(1);
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
    remove_dir(cwdbuf);
    check_leaks();
  }
}

uint64_t r[3] = {0x0, 0x0, 0x0};

void execute_one(void)
{
  intptr_t res = 0;
  memcpy((void*)0x20000000, "user\000", 5);
  memcpy((void*)0x20000100, "syz", 3);
  *(uint8_t*)0x20000103 = 0x20;
  *(uint8_t*)0x20000104 = 0;
  memcpy((void*)0x20000400,
         "\x76\x85\x42\x78\x01\xbf\x90\x10\x06\x30\x37\x78\x09\x53\xc8\x4a\x64"
         "\x66\x31\x71\x2c\xda\x94\x31\x46\xfb\x57\x01\x15\x93\x1d\xd1\x99\x3f"
         "\x80\x43\xb7\x88\xd2\x4c\x15\x51\xce\xfc\xb5\x15\x3d\x20\xb4\xe6\x32"
         "\x69\x7c\x97\x19\x35\x02\xdf\x6d\xea\xe7\xb4\xc9\x21\x95\x5f\xb6\x08"
         "\x62\x44\x11\x31\x82\xb8\xf3\x13\x59\x3c\x4f\x22\x6e\xb6\x13\xcd\x2f"
         "\x6c\xa4\xa3\x88\xa9\x78\x2d\x21\x5e\x58\x84\xf5\x2b\x2d\x77\x74\x00"
         "\x1b\xd2\x38\x96\x51\x72\xba\xcf\x7a\x82\xf5\xb4\xed\x47\x00\x00\xe2"
         "\x8d\x20\xa1\xb6\x33\x16\x03\x59\x11\xbc\xc9\x16\x65\xae\x6e\x37\xb5"
         "\x24\x6b\x01\x38\x94\x48\x91\x2f\x20\x7c\xaf\xed\x83\x92\x9b\xe5\xe9",
         153);
  res = syscall(__NR_add_key, 0x20000000, 0x20000100, 0x20000400, 0x99,
                0xfffffffd);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000300, "user\000", 5);
  memcpy((void*)0x200000c0, "syz", 3);
  *(uint8_t*)0x200000c3 = 0;
  *(uint8_t*)0x200000c4 = 0;
  memcpy((void*)0x20000240, "X", 1);
  res =
      syscall(__NR_add_key, 0x20000300, 0x200000c0, 0x20000240, 1, 0xfffffffe);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000380, "user\000", 5);
  memcpy((void*)0x20000140, "syz", 3);
  *(uint8_t*)0x20000143 = 0;
  *(uint8_t*)0x20000144 = 0;
  memcpy((void*)0x20000600,
         "\xe3\xfe\x88\xb8\x84\xda\xaf\x37\xb0\xbb\xfd\xf5\x43\x74\xd7\x8a\x26"
         "\x95\x62\x5b\xb3\x21\x76\xba\x9a\xe9\xf6\xa8\x0f\xc2\x09\x5e\x52\xe6"
         "\x88\x56\xad\xdb\xd6\x9c\x87\x86\x23\x81\x31\x1b\x1d\x98\x05\xad\x8d"
         "\x2f\xe8\xe0\xaa\xc6\x72\xfa\x46\x30\x5a\x5e\xef\xa6\xb6\x8e\xe3\xa9"
         "\x31\x39\x50\xd2\x25\x22\x96\x00\x22\xb3\xf3\x5c\x11\x49\xe2\xfb\x87"
         "\x6a\xf8\xe4\x49\xd1\x55\x8d\x8c\x09\x55\xfd\x8d\x43\x51\x47\xbd\x8b"
         "\x5d\x7a\x11\x80\xe2\xfc\xa5\xd8\x9c\x37\x38\xdd\x63\xd3\x9d\xf5\x2c"
         "\x19\xd7\x4c\x9e\x3a\xd3\xbd\x16\x38\xcc\x49\x0f\xd6\xef\xfd\x67\xfc"
         "\xf0\x07\x79\x9d\x59\x7f\xd8\x68\x29\x38\x7d\x21\x0c\x37\xa4\x15\xb4"
         "\xb3\xc9\xcb\x05\xe5\x4c\x43\x4b\xea\xc9\xd4\x0a\xf7\xd5\xa4\x3e\x13"
         "\xa9\xa5\x1b\x0c\x0d\xf6\xca\x48\xc9\x5c\x38\x7e\x34\x58\x54\xea\xb4"
         "\x51\xac\x01\xb1\xf6\xf8\xca\xca\x4a\x5a\xf7\xb8\x34\x63\x27\xed\x95"
         "\x1f\xaf\xff\xf4\xf2\x3d\x52\x69\x2b\x08\xc3\x57\x4a\x9a\xa8\x77\x6d"
         "\x6f\xee\xac\x3b\x4e\xe4\x38\xcc\x62\xc4\x6e\x38\x7e\x66\x25\x3c\x6a"
         "\x34\x7b\x40\x90\x04\x3e\x0f\x9b\x2c\x6f\x04\x16\x44\xd0\xb0\x1b\xf6"
         "\xe9\x39\xc5\xa8\xbb\x23\x46\x35\x2d\x87\x4b\xa4\x7a\x4a\xc7\x96\xb6"
         "\xf6\x10\x7e\x45\xc2\x4b\x7f\xc5\xd0\x45\xb9\xb1\x87\xcb\x18\xcd\x00"
         "\xfe\x80\xce\xd7\x24\x48\x77\x31\xc3\x44\x52\x18\x8f\x52\x23\xa3\x36"
         "\x7a\x3c\x6b\x64\xd5\x3b\x75\xbc\x6a\x70\x20\xd7\x91\x9f\xf7\xae\x33"
         "\x43\x53\x58\x79\xc4\x71\x42\x3a\xd4\xff\x0e\x13\xb9\xce\x3e\x2d\x3f"
         "\xf3\x8b\xa5\x79\xfc\xc3\x70\x5b\xfd\xb4\x1e\x6b\xdf\x92\xcd\xc2\x03"
         "\x22\xb6\xf3\x06\x22\x2d\x67\xce\xae\xd7\x84\xf8\x32\x7f\x29\x7c\xcd"
         "\x71\xbb\xd6\x8c\x43\x4d\x3b\xe7\x8d\x8c\xc2\x6c\xf5\x6e\xd3\x7e\x3c"
         "\xb4\x1e\xac\x62\x3e\x47\x52\x51\x24\xea\xae\x65\x13\x96\xd8\x3f\xb4"
         "\x26\x60\x2d\xef\x78\x1b\x62\x53\xa6\x9c\xc9\x16\xd5\x04\x81\xcb\x13"
         "\xe6\x77\x17\x44\x89\x6a\x19\xef\x78\xe9\x32\x07\x3e\x11\x71\x01",
         441);
  res = syscall(__NR_add_key, 0x20000380, 0x20000140, 0x20000600, 0x1b9,
                0xfffffffd);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x20000080 = r[1];
  *(uint32_t*)0x20000084 = r[2];
  *(uint32_t*)0x20000088 = r[0];
  *(uint64_t*)0x200001c0 = 0x20000040;
  memcpy((void*)0x20000040, "crct10dif\000\000\000 "
                            "\000\000\000+"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000\000"
                            "\000\000\000\000\000\000\000\000\000\000\000",
         64);
  *(uint64_t*)0x200001c8 = 0;
  *(uint32_t*)0x200001d0 = 0;
  *(uint32_t*)0x200001d4 = 0;
  *(uint32_t*)0x200001d8 = 0;
  *(uint32_t*)0x200001dc = 0;
  *(uint32_t*)0x200001e0 = 0;
  *(uint32_t*)0x200001e4 = 0;
  *(uint32_t*)0x200001e8 = 0;
  *(uint32_t*)0x200001ec = 0;
  *(uint32_t*)0x200001f0 = 0;
  inject_fault(27);
  syscall(__NR_keyctl, 0x17, 0x20000080, 0, 0, 0x200001c0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  setup_leak();
  setup_fault();
  use_temporary_dir();
  loop();
  return 0;
}
