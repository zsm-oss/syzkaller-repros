// possible deadlock in process_one_work
// https://syzkaller.appspot.com/bug?id=bd3860e571a12c18739719a7aff638acc9fe4072
// status:invalid
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/capability.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}

__attribute__((noreturn)) static void fail(const char* msg, ...)
{
  int e = errno;
  fflush(stdout);
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
}

__attribute__((noreturn)) static void exitf(const char* msg, ...)
{
  int e = errno;
  fflush(stdout);
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit(kRetryStatus);
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

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block",
            (uint8_t)a1, (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();

  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 128 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);

  unshare(CLONE_NEWNS);
  unshare(CLONE_NEWIPC);
  unshare(CLONE_IO);
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
    close(fd);
    return false;
  }
  close(fd);
  return true;
}

static int real_uid;
static int real_gid;
static int epid;
static bool etun;
__attribute__((aligned(64 << 10))) static char sandbox_stack[1 << 20];

static int namespace_sandbox_proc(void* arg)
{
  sandbox_common();

  write_file("/proc/self/setgroups", "deny");
  if (!write_file("/proc/self/uid_map", "0 %d 1\n", real_uid))
    fail("write of /proc/self/uid_map failed");
  if (!write_file("/proc/self/gid_map", "0 %d 1\n", real_gid))
    fail("write of /proc/self/gid_map failed");

  if (mkdir("./syz-tmp", 0777))
    fail("mkdir(syz-tmp) failed");
  if (mount("", "./syz-tmp", "tmpfs", 0, NULL))
    fail("mount(tmpfs) failed");
  if (mkdir("./syz-tmp/newroot", 0777))
    fail("mkdir failed");
  if (mkdir("./syz-tmp/newroot/dev", 0700))
    fail("mkdir failed");
  if (mount("/dev", "./syz-tmp/newroot/dev", NULL,
            MS_BIND | MS_REC | MS_PRIVATE, NULL))
    fail("mount(dev) failed");
  if (mkdir("./syz-tmp/newroot/proc", 0700))
    fail("mkdir failed");
  if (mount(NULL, "./syz-tmp/newroot/proc", "proc", 0, NULL))
    fail("mount(proc) failed");
  if (mkdir("./syz-tmp/pivot", 0777))
    fail("mkdir failed");
  if (syscall(SYS_pivot_root, "./syz-tmp", "./syz-tmp/pivot")) {
    if (chdir("./syz-tmp"))
      fail("chdir failed");
  } else {
    if (chdir("/"))
      fail("chdir failed");
    if (umount2("./pivot", MNT_DETACH))
      fail("umount failed");
  }
  if (chroot("./newroot"))
    fail("chroot failed");
  if (chdir("/"))
    fail("chdir failed");

  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    fail("capget failed");
  cap_data[0].effective &= ~(1 << CAP_SYS_PTRACE);
  cap_data[0].permitted &= ~(1 << CAP_SYS_PTRACE);
  cap_data[0].inheritable &= ~(1 << CAP_SYS_PTRACE);
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    fail("capset failed");

  loop();
  doexit(1);
}

static int do_sandbox_namespace(int executor_pid, bool enable_tun)
{
  real_uid = getuid();
  real_gid = getgid();
  epid = executor_pid;
  etun = enable_tun;
  mprotect(sandbox_stack, 4096, PROT_NONE);
  return clone(
      namespace_sandbox_proc,
      &sandbox_stack[sizeof(sandbox_stack) - 64],
      CLONE_NEWUSER | CLONE_NEWPID | CLONE_NEWUTS | CLONE_NEWNET, NULL);
}

static void remove_dir(const char* dir)
{
  DIR* dp;
  struct dirent* ep;
  int iter = 0;
retry:
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exitf("opendir(%s) failed due to NOFILE, exiting");
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

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void test();

void loop()
{
  int iter;
  for (iter = 0;; iter++) {
    char cwdbuf[256];
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
      test();
      doexit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      int res = waitpid(-1, &status, __WALL | WNOHANG);
      if (res == pid)
        break;
      usleep(1000);
      if (current_time_ms() - start > 5 * 1000) {
        kill(-pid, SIGKILL);
        kill(pid, SIGKILL);
        while (waitpid(-1, &status, __WALL) != pid) {
        }
        break;
      }
    }
    remove_dir(cwdbuf);
  }
}

long r[77];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    memcpy((void*)0x20816000,
           "\x2f\x64\x65\x76\x2f\x73\x6e\x64\x2f\x73\x65\x71\x00", 13);
    r[2] = syz_open_dev(0x20816000ul, 0x0ul, 0x200ul);
    break;
  case 2:
    memcpy((void*)0x20fb6ff7, "\x2f\x64\x65\x76\x2f\x6b\x76\x6d\x00",
           9);
    r[4] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20fb6ff7ul,
                   0x0ul, 0x0ul);
    break;
  case 3:
    r[5] = syscall(__NR_ioctl, r[4], 0xae01ul, 0x0ul);
    break;
  case 4:
    *(uint32_t*)0x20fb7000 = (uint32_t)0x7b;
    *(uint32_t*)0x20fb7004 = (uint32_t)0x0;
    *(uint64_t*)0x20fb7008 = (uint64_t)0x5;
    *(uint64_t*)0x20fb7010 = (uint64_t)0x511;
    *(uint64_t*)0x20fb7018 = (uint64_t)0xe45;
    *(uint64_t*)0x20fb7020 = (uint64_t)0x1ff;
    *(uint8_t*)0x20fb7028 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7029 = (uint8_t)0x0;
    *(uint8_t*)0x20fb702a = (uint8_t)0x0;
    *(uint8_t*)0x20fb702b = (uint8_t)0x0;
    *(uint8_t*)0x20fb702c = (uint8_t)0x0;
    *(uint8_t*)0x20fb702d = (uint8_t)0x0;
    *(uint8_t*)0x20fb702e = (uint8_t)0x0;
    *(uint8_t*)0x20fb702f = (uint8_t)0x0;
    *(uint8_t*)0x20fb7030 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7031 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7032 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7033 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7034 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7035 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7036 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7037 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7038 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7039 = (uint8_t)0x0;
    *(uint8_t*)0x20fb703a = (uint8_t)0x0;
    *(uint8_t*)0x20fb703b = (uint8_t)0x0;
    *(uint8_t*)0x20fb703c = (uint8_t)0x0;
    *(uint8_t*)0x20fb703d = (uint8_t)0x0;
    *(uint8_t*)0x20fb703e = (uint8_t)0x0;
    *(uint8_t*)0x20fb703f = (uint8_t)0x0;
    *(uint8_t*)0x20fb7040 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7041 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7042 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7043 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7044 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7045 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7046 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7047 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7048 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7049 = (uint8_t)0x0;
    *(uint8_t*)0x20fb704a = (uint8_t)0x0;
    *(uint8_t*)0x20fb704b = (uint8_t)0x0;
    *(uint8_t*)0x20fb704c = (uint8_t)0x0;
    *(uint8_t*)0x20fb704d = (uint8_t)0x0;
    *(uint8_t*)0x20fb704e = (uint8_t)0x0;
    *(uint8_t*)0x20fb704f = (uint8_t)0x0;
    *(uint8_t*)0x20fb7050 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7051 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7052 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7053 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7054 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7055 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7056 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7057 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7058 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7059 = (uint8_t)0x0;
    *(uint8_t*)0x20fb705a = (uint8_t)0x0;
    *(uint8_t*)0x20fb705b = (uint8_t)0x0;
    *(uint8_t*)0x20fb705c = (uint8_t)0x0;
    *(uint8_t*)0x20fb705d = (uint8_t)0x0;
    *(uint8_t*)0x20fb705e = (uint8_t)0x0;
    *(uint8_t*)0x20fb705f = (uint8_t)0x0;
    *(uint8_t*)0x20fb7060 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7061 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7062 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7063 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7064 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7065 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7066 = (uint8_t)0x0;
    *(uint8_t*)0x20fb7067 = (uint8_t)0x0;
    r[76] = syscall(__NR_ioctl, 0xfffffffffffffffful, 0x4068aea3ul,
                    0x20fb7000ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[10];

  memset(r, -1, sizeof(r));
  for (i = 0; i < 5; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  usleep(rand() % 100000);
}

int main()
{
  int i;
  for (i = 0; i < 8; i++) {
    if (fork() == 0) {
      use_temporary_dir();
      int pid = do_sandbox_namespace(i, false);
      int status = 0;
      while (waitpid(pid, &status, __WALL) != pid) {
      }
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}
