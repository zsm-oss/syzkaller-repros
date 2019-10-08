// KASAN: stack-out-of-bounds Read in wait_consider_task
// https://syzkaller.appspot.com/bug?id=a0a6756482ffbd2f850d55aeea015282e3f20439
// status:dup
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
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
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* uctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  doexit(sig);
}

static void install_segv_handler()
{
  struct sigaction sa;

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);

  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                        \
  {                                                                            \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    }                                                                          \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
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

int wait_for_loop(int pid)
{
  if (pid < 0)
    fail("sandbox fork failed");
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

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;
static int collide;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    while (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE))
      syscall(SYS_futex, &th->running, FUTEX_WAIT, 0, 0);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    __atomic_store_n(&th->running, 0, __ATOMIC_RELEASE);
    syscall(SYS_futex, &th->running, FUTEX_WAKE);
  }
  return 0;
}

static void execute(int num_calls)
{
  int call, thread;
  running = 0;
  for (call = 0; call < num_calls; call++) {
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 128 << 10);
        pthread_create(&th->th, &attr, thr, th);
      }
      if (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE)) {
        th->call = call;
        __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
        __atomic_store_n(&th->running, 1, __ATOMIC_RELEASE);
        syscall(SYS_futex, &th->running, FUTEX_WAKE);
        if (collide && call % 2)
          break;
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (__atomic_load_n(&running, __ATOMIC_RELAXED))
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};
unsigned long long procid;
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    res = syscall(__NR_socketpair, 0, 0, 0, 0x20000140);
    if (res != -1)
      NONFAILING(r[0] = *(uint32_t*)0x20000140);
    break;
  case 1:
    NONFAILING(*(uint64_t*)0x20000200 = 0x200001c0);
    NONFAILING(memcpy((void*)0x200001c0, "]", 2));
    NONFAILING(*(uint32_t*)0x20000208 = r[0]);
    syscall(__NR_bpf, 0x11, 0x20000200, 0x10);
    break;
  case 2:
    NONFAILING(*(uint32_t*)0x20000100 = r[0]);
    NONFAILING(*(uint64_t*)0x20000108 = 0x20000080);
    NONFAILING(*(uint64_t*)0x20000110 = 0x200000c0);
    NONFAILING(*(uint64_t*)0x20000118 = 1);
    syscall(__NR_bpf, 2, 0x20000100, 0x20);
    break;
  case 3:
    syscall(__NR_socket, 0xa, 1, 0);
    break;
  case 4:
    NONFAILING(*(uint32_t*)0x20000280 = 0xf);
    NONFAILING(*(uint32_t*)0x20000284 = 4);
    NONFAILING(*(uint32_t*)0x20000288 = 4);
    NONFAILING(*(uint32_t*)0x2000028c = 0x70);
    NONFAILING(*(uint32_t*)0x20000290 = 0);
    NONFAILING(*(uint32_t*)0x20000294 = -1);
    NONFAILING(*(uint32_t*)0x20000298 = 0);
    NONFAILING(*(uint8_t*)0x2000029c = 0);
    NONFAILING(*(uint8_t*)0x2000029d = 0);
    NONFAILING(*(uint8_t*)0x2000029e = 0);
    NONFAILING(*(uint8_t*)0x2000029f = 0);
    NONFAILING(*(uint8_t*)0x200002a0 = 0);
    NONFAILING(*(uint8_t*)0x200002a1 = 0);
    NONFAILING(*(uint8_t*)0x200002a2 = 0);
    NONFAILING(*(uint8_t*)0x200002a3 = 0);
    NONFAILING(*(uint8_t*)0x200002a4 = 0);
    NONFAILING(*(uint8_t*)0x200002a5 = 0);
    NONFAILING(*(uint8_t*)0x200002a6 = 0);
    NONFAILING(*(uint8_t*)0x200002a7 = 0);
    NONFAILING(*(uint8_t*)0x200002a8 = 0);
    NONFAILING(*(uint8_t*)0x200002a9 = 0);
    NONFAILING(*(uint8_t*)0x200002aa = 0);
    NONFAILING(*(uint8_t*)0x200002ab = 0);
    res = syscall(__NR_bpf, 0, 0x20000280, 0x2c);
    if (res != -1)
      r[1] = res;
    break;
  case 5:
    NONFAILING(memcpy((void*)0x20000240, "./cgroup.cpu/syz0", 18));
    syscall(__NR_openat, 0xffffffffffffff9c, 0x20000240, 0x200002, 0);
    break;
  case 6:
    res = syscall(__NR_socketpair, 0x955a7f081fc4adb5, 0xf, 9, 0x20000000);
    if (res != -1) {
      NONFAILING(r[2] = *(uint32_t*)0x20000000);
      NONFAILING(r[3] = *(uint32_t*)0x20000004);
    }
    break;
  case 7:
    NONFAILING(*(uint32_t*)0x20000040 = r[2]);
    NONFAILING(*(uint32_t*)0x20000044 = r[3]);
    NONFAILING(*(uint32_t*)0x20000048 = 0);
    NONFAILING(*(uint32_t*)0x2000004c = 0);
    syscall(__NR_bpf, 8, 0x20000040, 0x10);
    break;
  case 8:
    NONFAILING(memcpy((void*)0x200002c0, "cgroup.threads", 15));
    syscall(__NR_openat, r[0], 0x200002c0, 2, 0);
    break;
  case 9:
    NONFAILING(*(uint32_t*)0x20000180 = r[1]);
    NONFAILING(*(uint64_t*)0x20000188 = 0x20000000);
    NONFAILING(*(uint64_t*)0x20000190 = 0x20000140);
    NONFAILING(*(uint64_t*)0x20000198 = 0);
    syscall(__NR_bpf, 2, 0x20000180, 0x20);
    break;
  case 10:
    NONFAILING(*(uint32_t*)0x20000180 = r[1]);
    NONFAILING(*(uint64_t*)0x20000188 = 0x20000000);
    NONFAILING(*(uint64_t*)0x20000190 = 0x20000140);
    NONFAILING(*(uint64_t*)0x20000198 = 1);
    syscall(__NR_bpf, 2, 0x20000180, 0x20);
    break;
  }
}

void execute_one()
{
  execute(11);
  collide = 1;
  execute(11);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  install_segv_handler();
  char* cwd = get_current_dir_name();
  for (;;) {
    if (chdir(cwd))
      fail("failed to chdir");
    use_temporary_dir();
    do_sandbox_none();
  }
}
