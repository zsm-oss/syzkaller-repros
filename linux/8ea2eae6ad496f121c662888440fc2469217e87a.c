// kernel BUG at net/core/dev.c:LINE!
// https://syzkaller.appspot.com/bug?id=8ea2eae6ad496f121c662888440fc2469217e87a
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <fcntl.h>
#include <linux/capability.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/time.h>
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

long r[82];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    r[1] = syscall(__NR_socket, 0x2ul, 0x1ul, 0x0ul);
    break;
  case 2:
    memcpy((void*)0x20c55000,
           "\x2f\x64\x65\x76\x2f\x6e\x65\x74\x2f\x74\x75\x6e\x00", 13);
    r[3] = syz_open_dev(0x20c55000ul, 0x0ul, 0x100000aul);
    break;
  case 3:
    r[4] = syscall(__NR_socket, 0x2000000011ul, 0x3ul, 0x0ul);
    break;
  case 4:
    *(uint8_t*)0x20927fd8 = (uint8_t)0x73;
    *(uint8_t*)0x20927fd9 = (uint8_t)0x79;
    *(uint8_t*)0x20927fda = (uint8_t)0x7a;
    *(uint8_t*)0x20927fdb = (uint8_t)0x30;
    *(uint8_t*)0x20927fdc = (uint8_t)0x0;
    *(uint32_t*)0x20927fe8 = (uint32_t)0x5;
    *(uint32_t*)0x20927fec = (uint32_t)0x0;
    *(uint64_t*)0x20927ff0 = (uint64_t)0x201a3ff4;
    *(uint32_t*)0x201a3ff4 = (uint32_t)0x0;
    *(uint32_t*)0x201a3ff8 = (uint32_t)0x0;
    *(uint16_t*)0x201a3ffc = (uint16_t)0x0;
    r[16] = syscall(__NR_ioctl, r[3], 0x400454caul, 0x20927fd8ul);
    break;
  case 5:
    *(uint8_t*)0x2062ffe0 = (uint8_t)0x73;
    *(uint8_t*)0x2062ffe1 = (uint8_t)0x79;
    *(uint8_t*)0x2062ffe2 = (uint8_t)0x7a;
    *(uint8_t*)0x2062ffe3 = (uint8_t)0x30;
    *(uint8_t*)0x2062ffe4 = (uint8_t)0x0;
    *(uint16_t*)0x2062fff0 = (uint16_t)0x20301;
    r[23] = syscall(__NR_ioctl, r[1], 0x8914ul, 0x2062ffe0ul);
    break;
  case 6:
    *(uint8_t*)0x20b13000 = (uint8_t)0x73;
    *(uint8_t*)0x20b13001 = (uint8_t)0x79;
    *(uint8_t*)0x20b13002 = (uint8_t)0x7a;
    *(uint8_t*)0x20b13003 = (uint8_t)0x30;
    *(uint8_t*)0x20b13004 = (uint8_t)0x0;
    *(uint32_t*)0x20b13010 = (uint32_t)0x0;
    *(uint8_t*)0x20b13014 = (uint8_t)0x0;
    *(uint8_t*)0x20b13015 = (uint8_t)0x0;
    *(uint8_t*)0x20b13016 = (uint8_t)0x0;
    *(uint8_t*)0x20b13017 = (uint8_t)0x0;
    *(uint8_t*)0x20b13018 = (uint8_t)0x0;
    *(uint8_t*)0x20b13019 = (uint8_t)0x0;
    *(uint8_t*)0x20b1301a = (uint8_t)0x0;
    *(uint8_t*)0x20b1301b = (uint8_t)0x0;
    *(uint8_t*)0x20b1301c = (uint8_t)0x0;
    *(uint8_t*)0x20b1301d = (uint8_t)0x0;
    *(uint8_t*)0x20b1301e = (uint8_t)0x0;
    *(uint8_t*)0x20b1301f = (uint8_t)0x0;
    *(uint8_t*)0x20b13020 = (uint8_t)0x0;
    *(uint8_t*)0x20b13021 = (uint8_t)0x0;
    *(uint8_t*)0x20b13022 = (uint8_t)0x0;
    *(uint8_t*)0x20b13023 = (uint8_t)0x0;
    *(uint8_t*)0x20b13024 = (uint8_t)0x0;
    *(uint8_t*)0x20b13025 = (uint8_t)0x0;
    *(uint8_t*)0x20b13026 = (uint8_t)0x0;
    *(uint8_t*)0x20b13027 = (uint8_t)0x0;
    r[50] = syscall(__NR_ioctl, r[4], 0x8933ul, 0x20b13000ul);
    if (r[50] != -1)
      r[51] = *(uint32_t*)0x20b13010;
    break;
  case 7:
    *(uint16_t*)0x20c85000 = (uint16_t)0x11;
    *(uint16_t*)0x20c85002 = (uint16_t)0x300;
    *(uint32_t*)0x20c85004 = r[51];
    *(uint16_t*)0x20c85008 = (uint16_t)0x1;
    *(uint8_t*)0x20c8500a = (uint8_t)0x0;
    *(uint8_t*)0x20c8500b = (uint8_t)0x6;
    *(uint8_t*)0x20c8500c = (uint8_t)0xbb;
    *(uint8_t*)0x20c8500d = (uint8_t)0xbb;
    *(uint8_t*)0x20c8500e = (uint8_t)0xbb;
    *(uint8_t*)0x20c8500f = (uint8_t)0xbb;
    *(uint8_t*)0x20c85010 = (uint8_t)0xbb;
    *(uint8_t*)0x20c85011 = (uint8_t)0x0;
    *(uint8_t*)0x20c85012 = (uint8_t)0x0;
    *(uint8_t*)0x20c85013 = (uint8_t)0x0;
    r[66] = syscall(__NR_bind, r[4], 0x20c85000ul, 0x14ul);
    break;
  case 8:
    *(uint16_t*)0x20041000 = (uint16_t)0x11;
    *(uint16_t*)0x20041002 = (uint16_t)0x1700;
    *(uint32_t*)0x20041004 = r[51];
    *(uint16_t*)0x20041008 = (uint16_t)0x1;
    *(uint8_t*)0x2004100a = (uint8_t)0x7;
    *(uint8_t*)0x2004100b = (uint8_t)0x6;
    *(uint8_t*)0x2004100c = (uint8_t)0x0;
    *(uint8_t*)0x2004100d = (uint8_t)0x0;
    *(uint8_t*)0x2004100e = (uint8_t)0x0;
    *(uint8_t*)0x2004100f = (uint8_t)0x0;
    *(uint8_t*)0x20041010 = (uint8_t)0x0;
    *(uint8_t*)0x20041011 = (uint8_t)0x0;
    *(uint8_t*)0x20041012 = (uint8_t)0x0;
    *(uint8_t*)0x20041013 = (uint8_t)0x0;
    r[81] = syscall(__NR_bind, r[4], 0x20041000ul, 0x14ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[18];

  memset(r, -1, sizeof(r));
  for (i = 0; i < 9; i++) {
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
