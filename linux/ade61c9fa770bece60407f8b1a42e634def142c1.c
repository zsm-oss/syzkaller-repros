// general protection fault in __rds_rdma_map
// https://syzkaller.appspot.com/bug?id=ade61c9fa770bece60407f8b1a42e634def142c1
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <linux/capability.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
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

#define CLONE_NEWCGROUP 0x02000000

  unshare(CLONE_NEWNS);
  unshare(CLONE_NEWIPC);
  unshare(CLONE_NEWCGROUP);
  unshare(CLONE_NEWNET);
  unshare(CLONE_NEWUTS);
  unshare(CLONE_SYSVSEM);
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
  int pid;

  real_uid = getuid();
  real_gid = getgid();
  mprotect(sandbox_stack, 4096, PROT_NONE);
  pid = clone(namespace_sandbox_proc,
              &sandbox_stack[sizeof(sandbox_stack) - 64],
              CLONE_NEWUSER | CLONE_NEWPID, NULL);
  if (pid < 0)
    fail("sandbox clone failed");
  return pid;
}

long r[46];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    *(uint32_t*)0x2001d000 = (uint32_t)0x2;
    *(uint32_t*)0x2001d004 = (uint32_t)0x78;
    *(uint8_t*)0x2001d008 = (uint8_t)0x6;
    *(uint8_t*)0x2001d009 = (uint8_t)0x1;
    *(uint8_t*)0x2001d00a = (uint8_t)0x0;
    *(uint8_t*)0x2001d00b = (uint8_t)0x0;
    *(uint32_t*)0x2001d00c = (uint32_t)0x0;
    *(uint64_t*)0x2001d010 = (uint64_t)0x0;
    *(uint64_t*)0x2001d018 = (uint64_t)0x0;
    *(uint64_t*)0x2001d020 = (uint64_t)0x0;
    *(uint8_t*)0x2001d028 = (uint8_t)0xfffffffffffffffe;
    *(uint8_t*)0x2001d029 = (uint8_t)0x0;
    *(uint8_t*)0x2001d02a = (uint8_t)0x0;
    *(uint8_t*)0x2001d02b = (uint8_t)0x0;
    *(uint32_t*)0x2001d02c = (uint32_t)0x0;
    *(uint32_t*)0x2001d030 = (uint32_t)0x0;
    *(uint32_t*)0x2001d034 = (uint32_t)0x0;
    *(uint64_t*)0x2001d038 = (uint64_t)0x0;
    *(uint64_t*)0x2001d040 = (uint64_t)0x0;
    *(uint64_t*)0x2001d048 = (uint64_t)0x0;
    *(uint64_t*)0x2001d050 = (uint64_t)0x0;
    *(uint64_t*)0x2001d058 = (uint64_t)0xffffffffffffffff;
    *(uint32_t*)0x2001d060 = (uint32_t)0x0;
    *(uint64_t*)0x2001d068 = (uint64_t)0x0;
    *(uint32_t*)0x2001d070 = (uint32_t)0x0;
    *(uint16_t*)0x2001d074 = (uint16_t)0x0;
    *(uint16_t*)0x2001d076 = (uint16_t)0x0;
    r[28] = syscall(__NR_perf_event_open, 0x2001d000ul, 0x0ul,
                    0xfffffffffffffffful, 0xfffffffffffffffful, 0x0ul);
    break;
  case 2:
    r[29] = syscall(__NR_socket, 0x40000000015ul, 0x5ul, 0x0ul);
    break;
  case 3:
    *(uint16_t*)0x20000ff0 = (uint16_t)0x2;
    *(uint16_t*)0x20000ff2 = (uint16_t)0x234e;
    *(uint8_t*)0x20000ff4 = (uint8_t)0xac;
    *(uint8_t*)0x20000ff5 = (uint8_t)0x14;
    *(uint8_t*)0x20000ff6 = (uint8_t)0x0;
    *(uint8_t*)0x20000ff7 = (uint8_t)0xaa;
    *(uint8_t*)0x20000ff8 = (uint8_t)0x0;
    *(uint8_t*)0x20000ff9 = (uint8_t)0x0;
    *(uint8_t*)0x20000ffa = (uint8_t)0x0;
    *(uint8_t*)0x20000ffb = (uint8_t)0x0;
    *(uint8_t*)0x20000ffc = (uint8_t)0x0;
    *(uint8_t*)0x20000ffd = (uint8_t)0x0;
    *(uint8_t*)0x20000ffe = (uint8_t)0x0;
    *(uint8_t*)0x20000fff = (uint8_t)0x0;
    r[44] = syscall(__NR_bind, r[29], 0x20000ff0ul, 0x10ul);
    break;
  case 4:
    r[45] = syscall(__NR_setsockopt, r[29], 0x400000000114ul, 0x7ul,
                    0x20000ffcul, 0xa0ul);
    break;
  }
  return 0;
}

void loop()
{
  long i;
  pthread_t th[10];

  memset(r, -1, sizeof(r));
  srand(getpid());
  for (i = 0; i < 5; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  for (i = 0; i < 5; i++) {
    pthread_create(&th[5 + i], 0, thr, (void*)i);
    if (rand() % 2)
      usleep(rand() % 10000);
  }
  usleep(rand() % 100000);
}

int main()
{
  use_temporary_dir();
  int pid = do_sandbox_namespace(0, false);
  int status = 0;
  while (waitpid(pid, &status, __WALL) != pid) {
  }
  return 0;
}
