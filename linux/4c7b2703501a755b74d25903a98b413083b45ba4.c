// BUG: workqueue lockup (3)
// https://syzkaller.appspot.com/bug?id=4c7b2703501a755b74d25903a98b413083b45ba4
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

static void fail(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
}

static void exitf(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit(kRetryStatus);
}

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void use_temporary_dir()
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    fail("failed to mkdtemp");
  if (chmod(tmpdir, 0777))
    fail("failed to chmod");
  if (chdir(tmpdir))
    fail("failed to chdir");
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
  if (!write_file("/syzcgroup/unified/cgroup.subtree_control",
                  "+cpu +memory +io +pids +rdma")) {
  }
  if (mkdir("/syzcgroup/cpu", 0777)) {
  }
  if (mount("none", "/syzcgroup/cpu", "cgroup", 0,
            "cpuset,cpuacct,perf_event,hugetlb")) {
  }
  if (!write_file("/syzcgroup/cpu/cgroup.clone_children", "1")) {
  }
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

static void setup_binfmt_misc()
{
  if (!write_file("/proc/sys/fs/binfmt_misc/register",
                  ":syz0:M:0:syz0::./file0:")) {
  }
  if (!write_file("/proc/sys/fs/binfmt_misc/register",
                  ":syz1:M:1:yz1::./file0:POC")) {
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

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid < 0)
    fail("sandbox fork failed");
  if (pid)
    return pid;

  setup_cgroups();
  setup_binfmt_misc();
  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }
  loop();
  doexit(1);
}

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
      exitf("opendir(%s) failed due to NOFILE, exiting", dir);
    }
    exitf("opendir(%s) failed", dir);
  }
  while ((ep = readdir(dp))) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", dir, ep->d_name);
    struct stat st;
    if (lstat(filename, &st))
      exitf("lstat(%s) failed", filename);
    if (S_ISDIR(st.st_mode)) {
      remove_dir(filename);
      continue;
    }
    int i;
    for (i = 0;; i++) {
      if (unlink(filename) == 0)
        break;
      if (errno == EROFS) {
        break;
      }
      if (errno != EBUSY || i > 100)
        exitf("unlink(%s) failed", filename);
      if (umount2(filename, MNT_DETACH))
        exitf("umount(%s) failed", filename);
    }
  }
  closedir(dp);
  int i;
  for (i = 0;; i++) {
    if (rmdir(dir) == 0)
      break;
    if (i < 100) {
      if (errno == EROFS) {
        break;
      }
      if (errno == EBUSY) {
        if (umount2(dir, MNT_DETACH))
          exitf("umount(%s) failed", dir);
        continue;
      }
      if (errno == ENOTEMPTY) {
        if (iter < 100) {
          iter++;
          goto retry;
        }
      }
    }
    exitf("rmdir(%s) failed", dir);
  }
}

static void execute_one();
extern unsigned long long procid;

static void loop()
{
  char cgroupdir[64];
  snprintf(cgroupdir, sizeof(cgroupdir), "/syzcgroup/unified/syz%llu", procid);
  char cgroupdir_cpu[64];
  snprintf(cgroupdir_cpu, sizeof(cgroupdir_cpu), "/syzcgroup/cpu/syz%llu",
           procid);
  char cgroupdir_net[64];
  snprintf(cgroupdir_net, sizeof(cgroupdir_net), "/syzcgroup/net/syz%llu",
           procid);
  if (mkdir(cgroupdir, 0777)) {
  }
  if (mkdir(cgroupdir_cpu, 0777)) {
  }
  if (mkdir(cgroupdir_net, 0777)) {
  }
  int pid = getpid();
  char procs_file[128];
  snprintf(procs_file, sizeof(procs_file), "%s/cgroup.procs", cgroupdir);
  if (!write_file(procs_file, "%d", pid)) {
  }
  snprintf(procs_file, sizeof(procs_file), "%s/cgroup.procs", cgroupdir_cpu);
  if (!write_file(procs_file, "%d", pid)) {
  }
  snprintf(procs_file, sizeof(procs_file), "%s/cgroup.procs", cgroupdir_net);
  if (!write_file(procs_file, "%d", pid)) {
  }
  int iter;
  for (iter = 0;; iter++) {
    char cwdbuf[32];
    sprintf(cwdbuf, "./%d", iter);
    if (mkdir(cwdbuf, 0777))
      fail("failed to mkdir");
    int pid = fork();
    if (pid < 0)
      fail("clone failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
      if (chdir(cwdbuf))
        fail("failed to chdir");
      if (symlink(cgroupdir, "./cgroup")) {
      }
      if (symlink(cgroupdir_cpu, "./cgroup.cpu")) {
      }
      if (symlink(cgroupdir_net, "./cgroup.net")) {
      }
      execute_one();
      doexit(0);
    }

    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      int res = waitpid(-1, &status, __WALL | WNOHANG);
      if (res == pid) {
        break;
      }
      usleep(1000);
      if (current_time_ms() - start < 3 * 1000)
        continue;
      kill(-pid, SIGKILL);
      kill(pid, SIGKILL);
      while (waitpid(-1, &status, __WALL) != pid) {
      }
      break;
    }
    remove_dir(cwdbuf);
  }
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

unsigned long long procid;
void execute_one()
{
  *(uint64_t*)0x20000200 = 0x20000140;
  memcpy((void*)0x20000140, "%/file0/file0", 14);
  *(uint32_t*)0x20000208 = 0;
  *(uint32_t*)0x2000020c = 0x10;
  syscall(__NR_bpf, 7, 0x20000200, 0x10);
  memcpy((void*)0x20000000, "%/file0", 8);
  memcpy((void*)0x20000040, "/", 2);
  memcpy((void*)0x20000080, "nfs", 4);
  syscall(__NR_mount, 0x20000000, 0x20000040, 0x20000080, 0x860020, 0x200000c0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  char* cwd = get_current_dir_name();
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        if (chdir(cwd))
          fail("failed to chdir");
        use_temporary_dir();
        int pid = do_sandbox_none();
        int status = 0;
        while (waitpid(pid, &status, __WALL) != pid) {
        }
      }
    }
  }
  sleep(1000000);
  return 0;
}
