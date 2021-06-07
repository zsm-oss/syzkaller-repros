// possible deadlock in free_huge_page
// https://syzkaller.appspot.com/bug?id=2054819d6fda52ce059e
// status:0
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
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

#include <linux/futex.h>

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

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  int i = 0;
  for (; i < 100; i++) {
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

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
}

static void setup_sysctl()
{
  static struct {
    const char* name;
    const char* data;
  } files[] = {
      {"/sys/kernel/debug/x86/nmi_longest_ns", "10000000000"},
      {"/proc/sys/kernel/hung_task_check_interval_secs", "20"},
      {"/proc/sys/net/core/bpf_jit_enable", "1"},
      {"/proc/sys/net/core/bpf_jit_kallsyms", "1"},
      {"/proc/sys/net/core/bpf_jit_harden", "0"},
      {"/proc/sys/kernel/kptr_restrict", "0"},
      {"/proc/sys/kernel/softlockup_all_cpu_backtrace", "1"},
      {"/proc/sys/fs/mount-max", "100"},
      {"/proc/sys/vm/oom_dump_tasks", "0"},
      {"/proc/sys/debug/exception-trace", "0"},
      {"/proc/sys/kernel/printk", "7 4 1 3"},
      {"/proc/sys/net/ipv4/ping_group_range", "0 65535"},
      {"/proc/sys/kernel/keys/gc_delay", "1"},
      {"/proc/sys/vm/nr_overcommit_hugepages", "4"},
      {"/proc/sys/vm/oom_kill_allocating_task", "1"},
  };
  for (size_t i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    if (!write_file(files[i].name, files[i].data))
      printf("write to %s failed: %s\n", files[i].name, strerror(errno));
  }
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
  int collide = 0;
again:
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
      if (collide && (call % 2) == 0)
        break;
      event_timedwait(&th->done, 45);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
  if (!collide) {
    collide = 1;
    goto again;
  }
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter = 0;
  for (;; iter++) {
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
      if (current_time_ms() - start < 5000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    memcpy(
        (void*)0x20000f00,
        "\003\021&\350\\\031\330\207o\217\364\"\210\344^\005\322\327\250\2321{-"
        "\277!"
        "\331\376\316\205\326\234Y\364o\357\220\317\316\347\372\316\260\2402"
        "\031\223\032%CZ\312\201\000\\\035\034)42\262\335\321\207|"
        "\341X\032by\355o\242+z\366i$\204s\322e\005\000\321g\300\2148,"
        "TQ\250Bz\226:\332q/"
        "\321\230\223\2577\027\372\020\231\207\316\325Q\253;OOPr:i\322\263-"
        "i\000\274\031M\"\325s\205\247Bo|\225\320\274\274-\200lS\t-"
        "H\206Y\342\336\324K\\\035F\207b\362y=\f\022\212w\374\027\2469/"
        "w\352H\200\220.\364\277:\225!RO\017\363\002\001\240)\vL/"
        "\201Zo\016\v<\361_\323\336\016B\001\217~\037\\@D\24374z\257\255\262="
        "7\374\213\032\213\025\300\3742\016IIv\311f\213\305M\257\337#"
        "H\002\306\240\222\200\024:\251\202\a\306+Z\352\244m\253L\261\025)\324<"
        "<I\024+__\203:\353x\254p\245Ut^\256\210(OX:\226\2324C\273\375N\353~|"
        "\377\273$B\f\223\245\354\303\024\252\266\3355E\205\307f\325_"
        "F\236\245BX\320\353\301s3\343\326\205*O\\n|\253d\365\304\333\226\017["
        "\305\244\340\0314\317\023\347dg\304]\376 "
        "\266`\036<_p\306\272\260>\345\344x&\204\243\n\312\256y\207\365\263<"
        "\214\353\2760\351\347\272u\233\365\313\310\373\227\312I\177\205_"
        "\245\340J,"
        "\240\357\347\342\226\272\a\270\031\235\324\366\347\"\3527\006\247o\255"
        "B\005\333\021\307\274M\2532\207\250F\031p\353#a;"
        "g\214yn\3733\225x\3567\367\002Q$"
        "\374\206\324\214y\017\033\036\265\317\324\251\272\344L\371\356\263;TP:"
        ",\250*Z,Du\a\231\373*\235\232\355\324t\364\255\370\215j\375\342\036)3>"
        ".;\370\026\355\333J\321\204K\346(j\313\223\b\000]"
        "\202\205\017\254\363\022}pi\017\342d\236\017\\\n\b\031\214!"
        "\301b\034\025\211\036\207\320\326\357\005,WI\316]\334\263N;:"
        "\323\343\343\002\310\361\246\222\006S\201\302\231\236U\323\025R\035\a "
        "\016\022f\004\203?\226\030s\200xw\231\313\207\035j+mp\030|\276;"
        "\022\'y\301\027\332\216b\'\276l\344\343\332M\a\"|\343\275|"
        "LRk\001\032\027\201\342\207k\350\362\220\264\350\362\263V\025\316\310N"
        "qRJ\005\326tm\377\b@\264\005\223\307\317\267M\023\226_m0|"
        "\237\223\326\346\321\252\0331\355\277AzI\265\277\002\3459\262\264\355"
        "\032\253\342\036S\325N0\234\000\3266fD\324\313O2v\240\244\310\362\\"
        "\356\240\314\2779\341\255\202\206\203\347\254\257\334\265\004\200\344k"
        "\235Z\222i\247P\213\347\003q.\377?\243\036\227\234W\027ipm>("
        "\215\370\371\244\264.\017\256M\234\231\201\024\277\273\256\302:"
        "\0371\225%\226\206Y\250\253\205Y)\205v\036\351B\302IF/"
        "9\244B\354\314\236\321\370\352N\001\340t$SW\341\025\345\034\263,-"
        "\230\333\227\274\360\370[&"
        "\217\021\237V\0213x\255\223\270\207fm\026\272\'4\271\252\036\226p?"
        "i\264\230Siym#\200po\351\207\336\260\035\343\001\351\324\031\233)"
        "\345ur\000",
        882);
    res = syscall(__NR_memfd_create, 0x20000f00ul, 4ul);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    syscall(__NR_mmap, 0x20000000ul, 0x600000ul, 0xbul, 0x12ul, r[0], 0ul);
    break;
  case 2:
    res = syscall(__NR_socket, 2ul, 1ul, 0);
    if (res != -1)
      r[1] = res;
    break;
  case 3:
    *(uint16_t*)0x200001c0 = 2;
    *(uint16_t*)0x200001c2 = htobe16(0x4e20);
    *(uint32_t*)0x200001c4 = htobe32(0xe0000002);
    syscall(__NR_bind, r[1], 0x200001c0ul, 0x10ul);
    break;
  case 4:
    *(uint32_t*)0x20000000 = 1;
    syscall(__NR_setsockopt, r[1], 1, 0x3c, 0x20000000ul, 0x1039cul);
    break;
  case 5:
    *(uint16_t*)0x20000080 = 2;
    *(uint16_t*)0x20000082 = htobe16(0x4e20);
    *(uint32_t*)0x20000084 = htobe32(0);
    syscall(__NR_sendto, r[1], 0ul, 0ul, 0x20000000ul, 0x20000080ul, 0x10ul);
    break;
  case 6:
    memcpy((void*)0x20000140,
           "\x7c\x0d\x11\x13\x17\xb1\xff\x8e\xc8\xf2\x9f\x81\x31\x9e\xc5\xb1"
           "\x0d\x0d\x41\x3f\xfa\xef\xd9\x44\x8d\xbe\xf1\xff\xb4\xe3\xa6\xaf"
           "\x87\x13\x15\x12\xda\x52\x8f\x62\x35\xfe\x35\xd3\x05\x3b\x5c\xb4"
           "\x87\x7c\x1b\x89\xbb\xf8\x3f\x6d\x74\x9d\x5b\x00\x67\x96\x01\xa3"
           "\xab\xdc\x20\x4c\xf1\xad\x79\x72\x52\xbb\x0c\x7b\x43\xe3\xe9\x76"
           "\x32\x21\xcb\x2c\x8d\x76\xf8\x24\xf4\xbe\xa3\x38\xf5\x3a\xa7\x57"
           "\x03\xf4\x65\xe5\x0e\x43\x4e\x21\x93\x90\x0c\x39\xab\x79\xd3\x7c"
           "\xc0\xce\x7a",
           115);
    syscall(__NR_sendto, r[1], 0x20000140ul, 0xffffffffffffffd0ul, 0x401c005ul,
            0ul, 0xffffffffffffff36ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  setup_sysctl();
  loop();
  return 0;
}
