// WARNING in ext4_da_update_reserve_space
// https://syzkaller.appspot.com/bug?id=67e4f16db666b1c8253c
// status:0
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <setjmp.h>
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

#include <linux/futex.h>

static unsigned long long procid;

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  exit(sig);
}

static void install_segv_handler(void)
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

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  int i;
  for (i = 0; i < 100; i++) {
    if (pthread_create(&th, &attr, fn, arg) == 0) {
      pthread_attr_destroy(&attr);
      return;
    }
    if (errno == EAGAIN) {
      usleep(50);
      continue;
    }
    break;
  }
  exit(1);
}

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1000000);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
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

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void execute_one(void)
{
  int i, call, thread;
  for (call = 0; call < 7; call++) {
    for (thread = 0; thread < (int)(sizeof(threads) / sizeof(threads[0]));
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      event_timedwait(&th->done, 45);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
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
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res;
  switch (call) {
  case 0:
    NONFAILING(memcpy((void*)0x20000740, "cgroup.controllers\000", 19));
    res = syscall(__NR_openat, 0xffffff9c, 0x20000740ul, 0x275aul, 0ul);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    NONFAILING(memcpy((void*)0x200001c0, "cgroup.controllers\000", 19));
    res = syscall(__NR_openat, 0xffffff9c, 0x200001c0ul, 0x275aul, 0ul);
    if (res != -1)
      r[1] = res;
    break;
  case 2:
    syscall(__NR_write, r[1], 0x20000040ul, 0x208e24bul);
    break;
  case 3:
    syscall(__NR_mmap, 0x20000000ul, 0xb36000ul, 2ul, 0x28011ul, r[1], 0ul);
    break;
  case 4:
    NONFAILING(*(uint16_t*)0x20001900 = 0);
    NONFAILING(*(uint16_t*)0x20001902 = 0);
    NONFAILING(*(uint64_t*)0x20001908 = 0);
    NONFAILING(*(uint64_t*)0x20001910 = 0x80000000);
    NONFAILING(*(uint32_t*)0x20001918 = 0);
    NONFAILING(*(uint32_t*)0x2000191c = 0);
    NONFAILING(*(uint32_t*)0x20001920 = 0);
    NONFAILING(*(uint32_t*)0x20001924 = 0);
    NONFAILING(*(uint32_t*)0x20001928 = 0);
    NONFAILING(*(uint32_t*)0x2000192c = 0);
    syscall(__NR_ioctl, r[0], 0x40305828, 0x20001900ul);
    break;
  case 5:
    NONFAILING(*(uint64_t*)0x20000080 = 0);
    syscall(__NR_ioctl, r[0], 0x40086602, 0x20000080ul);
    break;
  case 6:
    NONFAILING(*(uint64_t*)0x200006c0 = 0);
    NONFAILING(*(uint64_t*)0x200006c8 = 0);
    NONFAILING(*(uint64_t*)0x200006d0 = 0);
    NONFAILING(*(uint64_t*)0x200006d8 = 0);
    NONFAILING(*(uint64_t*)0x200006e0 = 0x20000400);
    NONFAILING(memcpy(
        (void*)0x20000400,
        "\x2e\x94\xa8\xf3\xca\x40\x8a\x06\x73\x5d\xd3\x7e\x30\x49\x6a\x62\x51"
        "\x01\x36\xb1\x0b\x4f\x5e\xd5\xe2\x64\x7e\xeb\x60\x16\x2d\x8b\xf0\x97"
        "\xc4\x3c\x0f\x8b\xcd\xe0\x94\x1f\x8d\x3f\x9d\x90\x91\xb6\x86\x40\x30"
        "\x10\x27\x67\x1c\x56\x90\x35\x77\xec\xbb\x3c\x2c\x94\xfd\xa3\xee\xb3"
        "\xed\xb8\x5f\x4f\x8d\x6a\x1a\x8c\x20\x14\xca\xeb\x14\x0f\xfe\xa0\xf6"
        "\x17\x55\xbb\xb6\x41\x99\xb8\x0b\x5b\x18\x60\x37\xe9\xd9\xa6\xae\xdc"
        "\x5c\xde\x2c\x15\xd2\x42\x14\x8f\xd1\xd8\x44\x6b\x74\xc2\x19\x08\x01"
        "\xfe\x37\x70\x69\x2d\xd8\x04\x70\xd9\x78\x29\xd6\xcf\xbe\x0f\x78\x68"
        "\xf3\x3e\x00\xdb\x25\x43\xee\x48\x06\xc0\xec\x55\xd6\x8f\x8b\xac\x5c"
        "\x24\x43\xc6\xd7\xa9\xfe\x99\x01\xdb\x33\x90\x24\xc4\x95\xec\x68\xfb"
        "\xa6\xed\x3f\xec\x3a\xd5\x97\x0c\x71\x00\xa7\x2a\x00\xe1\x21\xf5\x37"
        "\x7f\x39\x5a\x5b\x0c\x88\xe9\x37\x88\xbc\x03\x4f\xd5\x02\xfb\x34\x43"
        "\xfb\xc9\xe0\x0b\x25\x1b\x5b\x7a\x8b\xac\x98\x9d\xd0\x85\xc0\x40\x7e"
        "\x9a\x79\xfd\x32\x35\xeb\xd4\xa6\xfd\xdf\xb4\xde\xad\x1d",
        235));
    NONFAILING(*(uint64_t*)0x200006e8 = 0xeb);
    NONFAILING(*(uint64_t*)0x200006f0 = 0);
    NONFAILING(*(uint64_t*)0x200006f8 = 0);
    NONFAILING(*(uint64_t*)0x20000700 = 0);
    NONFAILING(*(uint64_t*)0x20000708 = 0);
    NONFAILING(*(uint64_t*)0x20000710 = 0x20000640);
    NONFAILING(*(uint64_t*)0x20000718 = 0);
    NONFAILING(*(uint64_t*)0x20000720 = 0);
    NONFAILING(*(uint64_t*)0x20000728 = 0);
    syscall(__NR_writev, r[0], 0x200006c0ul, 7ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0ul);
  install_segv_handler();
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      use_temporary_dir();
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
