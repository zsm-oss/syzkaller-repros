// WARNING in bpf_cgroup_link_release
// https://syzkaller.appspot.com/bug?id=8a5dadc5c0b1d7055945
// status:0
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
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
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/capability.h>

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

#define MAX_FDS 30

static void setup_cgroups()
{
  if (mkdir("/syzcgroup", 0777)) {
  }
  if (mkdir("/syzcgroup/unified", 0777)) {
  }
  if (mount("none", "/syzcgroup/unified", "cgroup2", 0, NULL)) {
  }
  if (chmod("/syzcgroup/unified", 0777)) {
  }
  write_file("/syzcgroup/unified/cgroup.subtree_control",
             "+cpu +memory +io +pids +rdma");
  if (mkdir("/syzcgroup/cpu", 0777)) {
  }
  if (mount("none", "/syzcgroup/cpu", "cgroup", 0,
            "cpuset,cpuacct,perf_event,hugetlb")) {
  }
  write_file("/syzcgroup/cpu/cgroup.clone_children", "1");
  write_file("/syzcgroup/cpu/cpuset.memory_pressure_enabled", "1");
  if (chmod("/syzcgroup/cpu", 0777)) {
  }
  if (mkdir("/syzcgroup/net", 0777)) {
  }
  if (mount("none", "/syzcgroup/net", "cgroup", 0,
            "net_cls,net_prio,devices,freezer")) {
  }
  if (chmod("/syzcgroup/net", 0777)) {
  }
}

static void setup_cgroups_loop()
{
  int pid = getpid();
  char file[128];
  char cgroupdir[64];
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/unified/syz%llu", procid);
  if (mkdir(cgroupdir, 0777)) {
  }
  snprintf(file, sizeof(file), "%s/pids.max", cgroupdir);
  write_file(file, "32");
  snprintf(file, sizeof(file), "%s/memory.low", cgroupdir);
  write_file(file, "%d", 298 << 20);
  snprintf(file, sizeof(file), "%s/memory.high", cgroupdir);
  write_file(file, "%d", 299 << 20);
  snprintf(file, sizeof(file), "%s/memory.max", cgroupdir);
  write_file(file, "%d", 300 << 20);
  snprintf(file, sizeof(file), "%s/cgroup.procs", cgroupdir);
  write_file(file, "%d", pid);
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/cpu/syz%llu", procid);
  if (mkdir(cgroupdir, 0777)) {
  }
  snprintf(file, sizeof(file), "%s/cgroup.procs", cgroupdir);
  write_file(file, "%d", pid);
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/net/syz%llu", procid);
  if (mkdir(cgroupdir, 0777)) {
  }
  snprintf(file, sizeof(file), "%s/cgroup.procs", cgroupdir);
  write_file(file, "%d", pid);
}

static void setup_cgroups_test()
{
  char cgroupdir[64];
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/unified/syz%llu", procid);
  if (symlink(cgroupdir, "./cgroup")) {
  }
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/cpu/syz%llu", procid);
  if (symlink(cgroupdir, "./cgroup.cpu")) {
  }
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/net/syz%llu", procid);
  if (symlink(cgroupdir, "./cgroup.net")) {
  }
}

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
  setup_cgroups();
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = (200 << 20);
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 32 << 20;
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
  if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(0x02000000)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
  typedef struct {
    const char* name;
    const char* value;
  } sysctl_t;
  static const sysctl_t sysctls[] = {
      {"/proc/sys/kernel/shmmax", "16777216"},
      {"/proc/sys/kernel/shmall", "536870912"},
      {"/proc/sys/kernel/shmmni", "1024"},
      {"/proc/sys/kernel/msgmax", "8192"},
      {"/proc/sys/kernel/msgmni", "1024"},
      {"/proc/sys/kernel/msgmnb", "1024"},
      {"/proc/sys/kernel/sem", "1024 1048576 500 1024"},
  };
  unsigned i;
  for (i = 0; i < sizeof(sysctls) / sizeof(sysctls[0]); i++)
    write_file(sysctls[i].name, sysctls[i].value);
}

static int wait_for_loop(int pid)
{
  if (pid < 0)
    exit(1);
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static void drop_caps(void)
{
  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    exit(1);
  const int drop = (1 << CAP_SYS_PTRACE) | (1 << CAP_SYS_NICE);
  cap_data[0].effective &= ~drop;
  cap_data[0].permitted &= ~drop;
  cap_data[0].inheritable &= ~drop;
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    exit(1);
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
  drop_caps();
  if (unshare(CLONE_NEWNET)) {
  }
  loop();
  exit(1);
}

#define FS_IOC_SETFLAGS _IOW('f', 2, long)
static void remove_dir(const char* dir)
{
  int iter = 0;
  DIR* dp = 0;
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
  struct dirent* ep = 0;
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
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0) {
          }
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
  for (int i = 0;; i++) {
    if (rmdir(dir) == 0)
      break;
    if (i < 100) {
      if (errno == EPERM) {
        int fd = open(dir, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0) {
          }
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

static void setup_loop()
{
  setup_cgroups_loop();
}

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setup_cgroups_test();
  write_file("/proc/self/oom_score_adj", "1000");
}

static void close_fds()
{
  for (int fd = 3; fd < MAX_FDS; fd++)
    close(fd);
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
  setup_loop();
  int iter = 0;
  for (;; iter++) {
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
      close_fds();
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
  }
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[5] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  *(uint32_t*)0x20000080 = 0;
  *(uint32_t*)0x20000084 = -1;
  *(uint32_t*)0x20000088 = 0;
  *(uint32_t*)0x2000008c = 7;
  *(uint64_t*)0x20000090 = 0x20000000;
  memcpy((void*)0x20000000, "cgroup\000", 7);
  syscall(__NR_bpf, 0x14ul, 0x20000080ul, 0x30ul);
  res =
      syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000000ul, 0x200002ul, 0ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000080 = 9;
  *(uint32_t*)0x20000084 = 4;
  *(uint64_t*)0x20000088 = 0x20000440;
  memcpy((void*)0x20000440,
         "\x18\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x61"
         "\x12\x00\x00\x00\x00\x00\x00\x95\x00\x00\x00\x00\x00\x00\x00\xd7\x02"
         "\x88\x98\x4c\x2a\x01\xff\xb5\x18\x5d\xc1\x13\x69\xa1\xd1\xe0\xe6\x16"
         "\x40\x98\xe7\xec\x30\x53\x8f\x4c\x98\x43\xeb\x24\x41\x1d\x0f\x01\x0d"
         "\xbd\x2d\xfc\xff\xa3\x3e\xbd\x41\xad\x09\x10\x35\x6d\x40\xa0\xc2\x7b"
         "\x86\xaa\xe8\x66\xdd\xb7\xe9\xe0\xb4\x43\x3f\xe4\x2d\xea\xed\x81\x75"
         "\x8c\xab\x3d\x07\x3c\x7d\x4f\xb6\x43\x9a\xff\xb6\xef\xdb\x20\x8d\xc2"
         "\x19\x1d\x17\x86\x60\xfc\xd7\x06\x70\x3b\xc0\x52\x44\x59\x5c\xbb\x7b"
         "\xdd\x56\x1b\xd5\xd9\x5d\xb3\x50\x4e\xa0\xa3\x47\xee\x23\x11\xdf\x07"
         "\xeb\xf8\xf0\x49\xf2\x46\x91\x64\xe0\xa1\x23\xa9\x3e\x21\x54\xa1\x3d"
         "\xfb\x8a\x05\xd7\x29\x83\x3d\xaf\x2b\xba\x23\x5e\xbc\xe0\xb2\xa2\xc3"
         "\x1d\x6f\x3c\x8e\x3a\xc8\xd8\x6f\x74\xe0\xdb\x84\x20\x4c\x7c\xdd\x1a"
         "\xab\xd8\xc4\x04\x3a\xda\x3e\x9c\x44\x29\xda\x4a\x20\x63\x63\x59\x86"
         "\xd7\x4c\x8b\xe7\xde\xa5\x3b\x3c\x75\xa8\xe4\x33\x51\x00\x00\x00\x00"
         "\x00\x35\x22\x56\x7c\x73\x1f\x8e\x49\xed\x29\xfb\xb7\x7b\xdd\x07\x0d"
         "\xc1\x6c\xe4\x0f\xdf\x01\xa5\x59\x75\x88\xe6\xf3\x73\x08\x0a\xa7\x64"
         "\x3a\xe4\x2e\x6c\x65\x23\xfc\xa9\xdf\x78\xac\x4d\xc0\xe7\xf1\x36\x24"
         "\x72\x4c\xb4\x49\xc4\x2c\x15\xc4\x56\x32\xe6\xe7\xdd\xcc\xb5\xd9\x94"
         "\x59\xe7\xb8\x18\x06\x93\xbc\x5c\xee\x90\x77\xd4\x12\xdd\x98\xf9\x44"
         "\xfc\xb0\x43\x0e\xd7\xc1\xba\xa9\x16\x1a\x25\xa4\xef\x6b\xca\xc2\x11"
         "\x17\xd3\x11\xcc\x6d\x0b\xd9\xa2\xbf\x64\x2f\x1b\x15\x04\x18\x4f\x02"
         "\x01\x00\x47\x03\xf6\xf5\x62\x38\xe2\xa0\xab\x10\x05\x96\xac\x4f\xf2"
         "\x00\x5e\x49\x1e\x8f\x45\x0d\xfd\x12\x72\x3f\xb8\x40\xf7\xcc\x04\x70"
         "\x81\xef\xe9\xca\x54\xa3\x2d\x80\xd8\x23\x57\xa9\xc8\x7c\x0c",
         406);
  *(uint64_t*)0x20000090 = 0x20000100;
  memcpy((void*)0x20000100, "GPL\000", 4);
  *(uint32_t*)0x20000098 = 0;
  *(uint32_t*)0x2000009c = 0;
  *(uint64_t*)0x200000a0 = 0;
  *(uint32_t*)0x200000a8 = 0;
  *(uint32_t*)0x200000ac = 0;
  *(uint8_t*)0x200000b0 = 0;
  *(uint8_t*)0x200000b1 = 0;
  *(uint8_t*)0x200000b2 = 0;
  *(uint8_t*)0x200000b3 = 0;
  *(uint8_t*)0x200000b4 = 0;
  *(uint8_t*)0x200000b5 = 0;
  *(uint8_t*)0x200000b6 = 0;
  *(uint8_t*)0x200000b7 = 0;
  *(uint8_t*)0x200000b8 = 0;
  *(uint8_t*)0x200000b9 = 0;
  *(uint8_t*)0x200000ba = 0;
  *(uint8_t*)0x200000bb = 0;
  *(uint8_t*)0x200000bc = 0;
  *(uint8_t*)0x200000bd = 0;
  *(uint8_t*)0x200000be = 0;
  *(uint8_t*)0x200000bf = 0;
  *(uint32_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = -1;
  *(uint32_t*)0x200000cc = 8;
  *(uint64_t*)0x200000d0 = 0;
  *(uint32_t*)0x200000d8 = 0;
  *(uint32_t*)0x200000dc = 0x10;
  *(uint64_t*)0x200000e0 = 0;
  *(uint32_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000ec = 0;
  *(uint32_t*)0x200000f0 = -1;
  res = syscall(__NR_bpf, 5ul, 0x20000080ul, 0x70ul);
  if (res != -1)
    r[1] = res;
  res =
      syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000000ul, 0x200002ul, 0ul);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x20000080 = 9;
  *(uint32_t*)0x20000084 = 4;
  *(uint64_t*)0x20000088 = 0x20000000;
  *(uint8_t*)0x20000000 = 0x18;
  STORE_BY_BITMASK(uint8_t, , 0x20000001, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000001, 0, 4, 4);
  *(uint16_t*)0x20000002 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint8_t*)0x20000008 = 0;
  *(uint8_t*)0x20000009 = 0;
  *(uint16_t*)0x2000000a = 0;
  *(uint32_t*)0x2000000c = 0;
  STORE_BY_BITMASK(uint8_t, , 0x20000010, 0x19, 0, 3);
  STORE_BY_BITMASK(uint8_t, , 0x20000010, 0, 3, 1);
  STORE_BY_BITMASK(uint8_t, , 0x20000010, 6, 4, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000011, 2, 0, 4);
  STORE_BY_BITMASK(uint8_t, , 0x20000011, 1, 4, 4);
  *(uint16_t*)0x20000012 = 0;
  *(uint32_t*)0x20000014 = 0;
  *(uint8_t*)0x20000018 = 0x95;
  *(uint8_t*)0x20000019 = 0;
  *(uint16_t*)0x2000001a = 0;
  *(uint32_t*)0x2000001c = 0;
  *(uint64_t*)0x20000090 = 0x20000100;
  memcpy((void*)0x20000100, "GPL\000", 4);
  *(uint32_t*)0x20000098 = 0;
  *(uint32_t*)0x2000009c = 0;
  *(uint64_t*)0x200000a0 = 0;
  *(uint32_t*)0x200000a8 = 0;
  *(uint32_t*)0x200000ac = 0;
  *(uint8_t*)0x200000b0 = 0;
  *(uint8_t*)0x200000b1 = 0;
  *(uint8_t*)0x200000b2 = 0;
  *(uint8_t*)0x200000b3 = 0;
  *(uint8_t*)0x200000b4 = 0;
  *(uint8_t*)0x200000b5 = 0;
  *(uint8_t*)0x200000b6 = 0;
  *(uint8_t*)0x200000b7 = 0;
  *(uint8_t*)0x200000b8 = 0;
  *(uint8_t*)0x200000b9 = 0;
  *(uint8_t*)0x200000ba = 0;
  *(uint8_t*)0x200000bb = 0;
  *(uint8_t*)0x200000bc = 0;
  *(uint8_t*)0x200000bd = 0;
  *(uint8_t*)0x200000be = 0;
  *(uint8_t*)0x200000bf = 0;
  *(uint32_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c4 = 0;
  *(uint32_t*)0x200000c8 = -1;
  *(uint32_t*)0x200000cc = 8;
  *(uint64_t*)0x200000d0 = 0;
  *(uint32_t*)0x200000d8 = 0;
  *(uint32_t*)0x200000dc = 0x10;
  *(uint64_t*)0x200000e0 = 0;
  *(uint32_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000ec = 0;
  *(uint32_t*)0x200000f0 = -1;
  res = syscall(__NR_bpf, 5ul, 0x20000080ul, 0x70ul);
  if (res != -1)
    r[3] = res;
  *(uint32_t*)0x20000040 = r[3];
  *(uint32_t*)0x20000044 = r[2];
  *(uint32_t*)0x20000048 = 2;
  *(uint32_t*)0x2000004c = 0;
  syscall(__NR_bpf, 0x1cul, 0x20000040ul, 0x10ul);
  *(uint32_t*)0x20000100 = r[1];
  *(uint32_t*)0x20000104 = r[0];
  *(uint32_t*)0x20000108 = 2;
  *(uint32_t*)0x2000010c = 0;
  res = syscall(__NR_bpf, 0x1cul, 0x20000100ul, 0x10ul);
  if (res != -1)
    r[4] = res;
  *(uint32_t*)0x20000040 = r[4];
  *(uint32_t*)0x20000044 = 0;
  inject_fault(0);
  syscall(__NR_bpf, 0x22ul, 0x20000040ul, 8ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  setup_fault();
  use_temporary_dir();
  do_sandbox_none();
  return 0;
}
