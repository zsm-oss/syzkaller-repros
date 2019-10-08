// WARNING in __queue_work
// https://syzkaller.appspot.com/bug?id=62843f3219094a2a5585afdf72aa9101023e8e88
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <linux/futex.h>
#include <linux/net.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/time.h>
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
#include <stdint.h>
#include <string.h>

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

static uint64_t current_time_ms()
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    fail("clock_gettime failed");
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

#define XT_TABLE_SIZE 1536
#define XT_MAX_ENTRIES 10

struct xt_counters {
  uint64_t pcnt, bcnt;
};

struct ipt_getinfo {
  char name[32];
  unsigned int valid_hooks;
  unsigned int hook_entry[5];
  unsigned int underflow[5];
  unsigned int num_entries;
  unsigned int size;
};

struct ipt_get_entries {
  char name[32];
  unsigned int size;
  void* entrytable[XT_TABLE_SIZE / sizeof(void*)];
};

struct ipt_replace {
  char name[32];
  unsigned int valid_hooks;
  unsigned int num_entries;
  unsigned int size;
  unsigned int hook_entry[5];
  unsigned int underflow[5];
  unsigned int num_counters;
  struct xt_counters* counters;
  char entrytable[XT_TABLE_SIZE];
};

struct ipt_table_desc {
  const char* name;
  struct ipt_getinfo info;
  struct ipt_replace replace;
};

static struct ipt_table_desc ipv4_tables[] = {
    {.name = "filter"}, {.name = "nat"},      {.name = "mangle"},
    {.name = "raw"},    {.name = "security"},
};

static struct ipt_table_desc ipv6_tables[] = {
    {.name = "filter"}, {.name = "nat"},      {.name = "mangle"},
    {.name = "raw"},    {.name = "security"},
};

#define IPT_BASE_CTL 64
#define IPT_SO_SET_REPLACE (IPT_BASE_CTL)
#define IPT_SO_GET_INFO (IPT_BASE_CTL)
#define IPT_SO_GET_ENTRIES (IPT_BASE_CTL + 1)

struct arpt_getinfo {
  char name[32];
  unsigned int valid_hooks;
  unsigned int hook_entry[3];
  unsigned int underflow[3];
  unsigned int num_entries;
  unsigned int size;
};

struct arpt_get_entries {
  char name[32];
  unsigned int size;
  void* entrytable[XT_TABLE_SIZE / sizeof(void*)];
};

struct arpt_replace {
  char name[32];
  unsigned int valid_hooks;
  unsigned int num_entries;
  unsigned int size;
  unsigned int hook_entry[3];
  unsigned int underflow[3];
  unsigned int num_counters;
  struct xt_counters* counters;
  char entrytable[XT_TABLE_SIZE];
};

struct arpt_table_desc {
  const char* name;
  struct arpt_getinfo info;
  struct arpt_replace replace;
};

static struct arpt_table_desc arpt_tables[] = {
    {.name = "filter"},
};

#define ARPT_BASE_CTL 96
#define ARPT_SO_SET_REPLACE (ARPT_BASE_CTL)
#define ARPT_SO_GET_INFO (ARPT_BASE_CTL)
#define ARPT_SO_GET_ENTRIES (ARPT_BASE_CTL + 1)

static void checkpoint_iptables(struct ipt_table_desc* tables, int num_tables,
                                int family, int level)
{
  struct ipt_get_entries entries;
  socklen_t optlen;
  int fd, i;

  fd = socket(family, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(%d, SOCK_STREAM, IPPROTO_TCP)", family);
  for (i = 0; i < num_tables; i++) {
    struct ipt_table_desc* table = &tables[i];
    strcpy(table->info.name, table->name);
    strcpy(table->replace.name, table->name);
    optlen = sizeof(table->info);
    if (getsockopt(fd, level, IPT_SO_GET_INFO, &table->info, &optlen)) {
      switch (errno) {
      case EPERM:
      case ENOENT:
      case ENOPROTOOPT:
        continue;
      }
      fail("getsockopt(IPT_SO_GET_INFO)");
    }
    if (table->info.size > sizeof(table->replace.entrytable))
      fail("table size is too large: %u", table->info.size);
    if (table->info.num_entries > XT_MAX_ENTRIES)
      fail("too many counters: %u", table->info.num_entries);
    memset(&entries, 0, sizeof(entries));
    strcpy(entries.name, table->name);
    entries.size = table->info.size;
    optlen = sizeof(entries) - sizeof(entries.entrytable) + table->info.size;
    if (getsockopt(fd, level, IPT_SO_GET_ENTRIES, &entries, &optlen))
      fail("getsockopt(IPT_SO_GET_ENTRIES)");
    table->replace.valid_hooks = table->info.valid_hooks;
    table->replace.num_entries = table->info.num_entries;
    table->replace.size = table->info.size;
    memcpy(table->replace.hook_entry, table->info.hook_entry,
           sizeof(table->replace.hook_entry));
    memcpy(table->replace.underflow, table->info.underflow,
           sizeof(table->replace.underflow));
    memcpy(table->replace.entrytable, entries.entrytable, table->info.size);
  }
  close(fd);
}

static void reset_iptables(struct ipt_table_desc* tables, int num_tables,
                           int family, int level)
{
  struct xt_counters counters[XT_MAX_ENTRIES];
  struct ipt_get_entries entries;
  struct ipt_getinfo info;
  socklen_t optlen;
  int fd, i;

  fd = socket(family, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(%d, SOCK_STREAM, IPPROTO_TCP)", family);
  for (i = 0; i < num_tables; i++) {
    struct ipt_table_desc* table = &tables[i];
    if (table->info.valid_hooks == 0)
      continue;
    memset(&info, 0, sizeof(info));
    strcpy(info.name, table->name);
    optlen = sizeof(info);
    if (getsockopt(fd, level, IPT_SO_GET_INFO, &info, &optlen))
      fail("getsockopt(IPT_SO_GET_INFO)");
    if (memcmp(&table->info, &info, sizeof(table->info)) == 0) {
      memset(&entries, 0, sizeof(entries));
      strcpy(entries.name, table->name);
      entries.size = table->info.size;
      optlen = sizeof(entries) - sizeof(entries.entrytable) + entries.size;
      if (getsockopt(fd, level, IPT_SO_GET_ENTRIES, &entries, &optlen))
        fail("getsockopt(IPT_SO_GET_ENTRIES)");
      if (memcmp(table->replace.entrytable, entries.entrytable,
                 table->info.size) == 0)
        continue;
    }
    table->replace.num_counters = info.num_entries;
    table->replace.counters = counters;
    optlen = sizeof(table->replace) - sizeof(table->replace.entrytable) +
             table->replace.size;
    if (setsockopt(fd, level, IPT_SO_SET_REPLACE, &table->replace, optlen))
      fail("setsockopt(IPT_SO_SET_REPLACE)");
  }
  close(fd);
}

static void checkpoint_arptables(void)
{
  struct arpt_get_entries entries;
  socklen_t optlen;
  unsigned i;
  int fd;

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)");
  for (i = 0; i < sizeof(arpt_tables) / sizeof(arpt_tables[0]); i++) {
    struct arpt_table_desc* table = &arpt_tables[i];
    strcpy(table->info.name, table->name);
    strcpy(table->replace.name, table->name);
    optlen = sizeof(table->info);
    if (getsockopt(fd, SOL_IP, ARPT_SO_GET_INFO, &table->info, &optlen)) {
      switch (errno) {
      case EPERM:
      case ENOENT:
      case ENOPROTOOPT:
        continue;
      }
      fail("getsockopt(ARPT_SO_GET_INFO)");
    }
    if (table->info.size > sizeof(table->replace.entrytable))
      fail("table size is too large: %u", table->info.size);
    if (table->info.num_entries > XT_MAX_ENTRIES)
      fail("too many counters: %u", table->info.num_entries);
    memset(&entries, 0, sizeof(entries));
    strcpy(entries.name, table->name);
    entries.size = table->info.size;
    optlen = sizeof(entries) - sizeof(entries.entrytable) + table->info.size;
    if (getsockopt(fd, SOL_IP, ARPT_SO_GET_ENTRIES, &entries, &optlen))
      fail("getsockopt(ARPT_SO_GET_ENTRIES)");
    table->replace.valid_hooks = table->info.valid_hooks;
    table->replace.num_entries = table->info.num_entries;
    table->replace.size = table->info.size;
    memcpy(table->replace.hook_entry, table->info.hook_entry,
           sizeof(table->replace.hook_entry));
    memcpy(table->replace.underflow, table->info.underflow,
           sizeof(table->replace.underflow));
    memcpy(table->replace.entrytable, entries.entrytable, table->info.size);
  }
  close(fd);
}

static void reset_arptables()
{
  struct xt_counters counters[XT_MAX_ENTRIES];
  struct arpt_get_entries entries;
  struct arpt_getinfo info;
  socklen_t optlen;
  unsigned i;
  int fd;

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)");
  for (i = 0; i < sizeof(arpt_tables) / sizeof(arpt_tables[0]); i++) {
    struct arpt_table_desc* table = &arpt_tables[i];
    if (table->info.valid_hooks == 0)
      continue;
    memset(&info, 0, sizeof(info));
    strcpy(info.name, table->name);
    optlen = sizeof(info);
    if (getsockopt(fd, SOL_IP, ARPT_SO_GET_INFO, &info, &optlen))
      fail("getsockopt(ARPT_SO_GET_INFO)");
    if (memcmp(&table->info, &info, sizeof(table->info)) == 0) {
      memset(&entries, 0, sizeof(entries));
      strcpy(entries.name, table->name);
      entries.size = table->info.size;
      optlen = sizeof(entries) - sizeof(entries.entrytable) + entries.size;
      if (getsockopt(fd, SOL_IP, ARPT_SO_GET_ENTRIES, &entries, &optlen))
        fail("getsockopt(ARPT_SO_GET_ENTRIES)");
      if (memcmp(table->replace.entrytable, entries.entrytable,
                 table->info.size) == 0)
        continue;
    }
    table->replace.num_counters = info.num_entries;
    table->replace.counters = counters;
    optlen = sizeof(table->replace) - sizeof(table->replace.entrytable) +
             table->replace.size;
    if (setsockopt(fd, SOL_IP, ARPT_SO_SET_REPLACE, &table->replace, optlen))
      fail("setsockopt(ARPT_SO_SET_REPLACE)");
  }
  close(fd);
}

static void checkpoint_net_namespace(void)
{
  checkpoint_arptables();
  checkpoint_iptables(ipv4_tables, sizeof(ipv4_tables) / sizeof(ipv4_tables[0]),
                      AF_INET, SOL_IP);
  checkpoint_iptables(ipv6_tables, sizeof(ipv6_tables) / sizeof(ipv6_tables[0]),
                      AF_INET6, SOL_IPV6);
}

static void reset_net_namespace(void)
{
  reset_arptables();
  reset_iptables(ipv4_tables, sizeof(ipv4_tables) / sizeof(ipv4_tables[0]),
                 AF_INET, SOL_IP);
  reset_iptables(ipv6_tables, sizeof(ipv6_tables) / sizeof(ipv6_tables[0]),
                 AF_INET6, SOL_IPV6);
}

static void test();

void loop()
{
  int iter;
  checkpoint_net_namespace();
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      fail("loop fork failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
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
    reset_net_namespace();
  }
}

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

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
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (running)
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

long r[1];
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
    break;
  case 1:
    r[0] = syscall(__NR_socket, 0xa, 2, 0);
    break;
  case 2:
    memcpy((void*)0x20103ba0, "\x73\x65\x63\x75\x72\x69\x74\x79\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x00\x00\x00\x00",
           32);
    *(uint32_t*)0x20103bc0 = 0xe;
    *(uint32_t*)0x20103bc4 = 4;
    *(uint32_t*)0x20103bc8 = 0x400;
    *(uint32_t*)0x20103bcc = -1;
    *(uint32_t*)0x20103bd0 = 0;
    *(uint32_t*)0x20103bd4 = 0;
    *(uint32_t*)0x20103bd8 = 0x260;
    *(uint32_t*)0x20103bdc = -1;
    *(uint32_t*)0x20103be0 = -1;
    *(uint32_t*)0x20103be4 = 0x330;
    *(uint32_t*)0x20103be8 = 0x330;
    *(uint32_t*)0x20103bec = 0x330;
    *(uint32_t*)0x20103bf0 = -1;
    *(uint32_t*)0x20103bf4 = 4;
    *(uint64_t*)0x20103bf8 = 0x20bf8fc0;
    *(uint8_t*)0x20103c00 = 0xfe;
    *(uint8_t*)0x20103c01 = 0x80;
    *(uint8_t*)0x20103c02 = 0;
    *(uint8_t*)0x20103c03 = 0;
    *(uint8_t*)0x20103c04 = 0;
    *(uint8_t*)0x20103c05 = 0;
    *(uint8_t*)0x20103c06 = 0;
    *(uint8_t*)0x20103c07 = 0;
    *(uint8_t*)0x20103c08 = 0;
    *(uint8_t*)0x20103c09 = 0;
    *(uint8_t*)0x20103c0a = 0;
    *(uint8_t*)0x20103c0b = 0;
    *(uint8_t*)0x20103c0c = 0;
    *(uint8_t*)0x20103c0d = 0;
    *(uint8_t*)0x20103c0e = 0;
    *(uint8_t*)0x20103c0f = 0xbb;
    *(uint8_t*)0x20103c10 = 0;
    *(uint8_t*)0x20103c11 = 0;
    *(uint8_t*)0x20103c12 = 0;
    *(uint8_t*)0x20103c13 = 0;
    *(uint8_t*)0x20103c14 = 0;
    *(uint8_t*)0x20103c15 = 0;
    *(uint8_t*)0x20103c16 = 0;
    *(uint8_t*)0x20103c17 = 0;
    *(uint8_t*)0x20103c18 = 0;
    *(uint8_t*)0x20103c19 = 0;
    *(uint8_t*)0x20103c1a = -1;
    *(uint8_t*)0x20103c1b = -1;
    *(uint32_t*)0x20103c1c = htobe32(-1);
    *(uint32_t*)0x20103c20 = htobe32(0xffffff00);
    *(uint32_t*)0x20103c24 = htobe32(0xff000000);
    *(uint32_t*)0x20103c28 = htobe32(0);
    *(uint32_t*)0x20103c2c = htobe32(0xff);
    *(uint32_t*)0x20103c30 = htobe32(0xff);
    *(uint32_t*)0x20103c34 = htobe32(0xffffff00);
    *(uint32_t*)0x20103c38 = htobe32(0);
    *(uint32_t*)0x20103c3c = htobe32(-1);
    memcpy((void*)0x20103c40,
           "\x74\x64\x2b\x13\xb5\x72\x2e\x37\x64\x74\xc1\x04\xcc\xe4\x92\x42",
           16);
    memcpy((void*)0x20103c50,
           "\x8f\xd4\xba\x00\x2b\x0f\x5d\x06\xd7\x1c\xe4\xc1\x9b\x0d\xca\xac",
           16);
    *(uint8_t*)0x20103c60 = -1;
    *(uint8_t*)0x20103c61 = 0;
    *(uint8_t*)0x20103c62 = 0;
    *(uint8_t*)0x20103c63 = 0;
    *(uint8_t*)0x20103c64 = 0;
    *(uint8_t*)0x20103c65 = 0;
    *(uint8_t*)0x20103c66 = 0;
    *(uint8_t*)0x20103c67 = 0;
    *(uint8_t*)0x20103c68 = 0;
    *(uint8_t*)0x20103c69 = 0;
    *(uint8_t*)0x20103c6a = 0;
    *(uint8_t*)0x20103c6b = 0;
    *(uint8_t*)0x20103c6c = 0;
    *(uint8_t*)0x20103c6d = 0;
    *(uint8_t*)0x20103c6e = 0;
    *(uint8_t*)0x20103c6f = 0;
    *(uint8_t*)0x20103c70 = 0;
    *(uint8_t*)0x20103c71 = 0;
    *(uint8_t*)0x20103c72 = 0;
    *(uint8_t*)0x20103c73 = 0;
    *(uint8_t*)0x20103c74 = 0;
    *(uint8_t*)0x20103c75 = 0;
    *(uint8_t*)0x20103c76 = 0;
    *(uint8_t*)0x20103c77 = 0;
    *(uint8_t*)0x20103c78 = 0;
    *(uint8_t*)0x20103c79 = 0;
    *(uint8_t*)0x20103c7a = 0;
    *(uint8_t*)0x20103c7b = 0;
    *(uint8_t*)0x20103c7c = 0;
    *(uint8_t*)0x20103c7d = 0;
    *(uint8_t*)0x20103c7e = 0;
    *(uint8_t*)0x20103c7f = 0;
    *(uint16_t*)0x20103c80 = 0x7f;
    *(uint8_t*)0x20103c82 = 7;
    *(uint8_t*)0x20103c83 = 7;
    *(uint8_t*)0x20103c84 = 8;
    *(uint32_t*)0x20103c88 = 0;
    *(uint16_t*)0x20103c8c = 0xf0;
    *(uint16_t*)0x20103c8e = 0x138;
    *(uint32_t*)0x20103c90 = 0;
    *(uint64_t*)0x20103c98 = 0;
    *(uint64_t*)0x20103ca0 = 0;
    *(uint16_t*)0x20103ca8 = 0x48;
    memcpy((void*)0x20103caa, "\x68\x62\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x00",
           29);
    *(uint8_t*)0x20103cc7 = 0;
    *(uint32_t*)0x20103cc8 = 7;
    *(uint8_t*)0x20103ccc = 2;
    *(uint8_t*)0x20103ccd = 1;
    *(uint16_t*)0x20103cce = 3;
    *(uint16_t*)0x20103cd0 = 0x800;
    *(uint16_t*)0x20103cd2 = 4;
    *(uint16_t*)0x20103cd4 = 0;
    *(uint16_t*)0x20103cd6 = 5;
    *(uint16_t*)0x20103cd8 = 0x101;
    *(uint16_t*)0x20103cda = 0x100;
    *(uint16_t*)0x20103cdc = 4;
    *(uint16_t*)0x20103cde = 0x800;
    *(uint16_t*)0x20103ce0 = 0xd50f;
    *(uint16_t*)0x20103ce2 = 5;
    *(uint16_t*)0x20103ce4 = 0xfffc;
    *(uint16_t*)0x20103ce6 = 0x52f0;
    *(uint16_t*)0x20103ce8 = 4;
    *(uint16_t*)0x20103cea = 0x8000;
    *(uint16_t*)0x20103cec = 0x100;
    *(uint8_t*)0x20103cee = 8;
    *(uint16_t*)0x20103cf0 = 0x48;
    memcpy((void*)0x20103cf2, "\x49\x44\x4c\x45\x54\x49\x4d\x45\x52\x00\x00\x00"
                              "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x00",
           29);
    *(uint8_t*)0x20103d0f = 0;
    *(uint32_t*)0x20103d10 = 0x80000000;
    memcpy((void*)0x20103d14, "\x73\x79\x7a\x30\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00",
           28);
    *(uint64_t*)0x20103d30 = 0x80000001;
    *(uint8_t*)0x20103d38 = -1;
    *(uint8_t*)0x20103d39 = 1;
    *(uint8_t*)0x20103d3a = 0;
    *(uint8_t*)0x20103d3b = 0;
    *(uint8_t*)0x20103d3c = 0;
    *(uint8_t*)0x20103d3d = 0;
    *(uint8_t*)0x20103d3e = 0;
    *(uint8_t*)0x20103d3f = 0;
    *(uint8_t*)0x20103d40 = 0;
    *(uint8_t*)0x20103d41 = 0;
    *(uint8_t*)0x20103d42 = 0;
    *(uint8_t*)0x20103d43 = 0;
    *(uint8_t*)0x20103d44 = 0;
    *(uint8_t*)0x20103d45 = 0;
    *(uint8_t*)0x20103d46 = 0;
    *(uint8_t*)0x20103d47 = 1;
    *(uint8_t*)0x20103d48 = 0xfe;
    *(uint8_t*)0x20103d49 = 0x80;
    *(uint8_t*)0x20103d4a = 0;
    *(uint8_t*)0x20103d4b = 0;
    *(uint8_t*)0x20103d4c = 0;
    *(uint8_t*)0x20103d4d = 0;
    *(uint8_t*)0x20103d4e = 0;
    *(uint8_t*)0x20103d4f = 0;
    *(uint8_t*)0x20103d50 = 0;
    *(uint8_t*)0x20103d51 = 0;
    *(uint8_t*)0x20103d52 = 0;
    *(uint8_t*)0x20103d53 = 0;
    *(uint8_t*)0x20103d54 = 0;
    *(uint8_t*)0x20103d55 = 0;
    *(uint8_t*)0x20103d56 = 0;
    *(uint8_t*)0x20103d57 = 0xaa;
    *(uint32_t*)0x20103d58 = htobe32(-1);
    *(uint32_t*)0x20103d5c = htobe32(0);
    *(uint32_t*)0x20103d60 = htobe32(0xff000000);
    *(uint32_t*)0x20103d64 = htobe32(-1);
    *(uint32_t*)0x20103d68 = htobe32(0xffffff00);
    *(uint32_t*)0x20103d6c = htobe32(0xff);
    *(uint32_t*)0x20103d70 = htobe32(0);
    *(uint32_t*)0x20103d74 = htobe32(-1);
    *(uint8_t*)0x20103d78 = 0x73;
    *(uint8_t*)0x20103d79 = 0x79;
    *(uint8_t*)0x20103d7a = 0x7a;
    *(uint8_t*)0x20103d7b = 0x30;
    *(uint8_t*)0x20103d7c = 0;
    *(uint8_t*)0x20103d88 = 0x73;
    *(uint8_t*)0x20103d89 = 0x79;
    *(uint8_t*)0x20103d8a = 0x7a;
    *(uint8_t*)0x20103d8b = 0x30;
    *(uint8_t*)0x20103d8c = 0;
    *(uint8_t*)0x20103d98 = 0;
    *(uint8_t*)0x20103d99 = 0;
    *(uint8_t*)0x20103d9a = 0;
    *(uint8_t*)0x20103d9b = 0;
    *(uint8_t*)0x20103d9c = 0;
    *(uint8_t*)0x20103d9d = 0;
    *(uint8_t*)0x20103d9e = 0;
    *(uint8_t*)0x20103d9f = 0;
    *(uint8_t*)0x20103da0 = 0;
    *(uint8_t*)0x20103da1 = 0;
    *(uint8_t*)0x20103da2 = 0;
    *(uint8_t*)0x20103da3 = 0;
    *(uint8_t*)0x20103da4 = 0;
    *(uint8_t*)0x20103da5 = 0;
    *(uint8_t*)0x20103da6 = 0;
    *(uint8_t*)0x20103da7 = 0;
    *(uint8_t*)0x20103da8 = -1;
    *(uint8_t*)0x20103da9 = 0;
    *(uint8_t*)0x20103daa = 0;
    *(uint8_t*)0x20103dab = 0;
    *(uint8_t*)0x20103dac = 0;
    *(uint8_t*)0x20103dad = 0;
    *(uint8_t*)0x20103dae = 0;
    *(uint8_t*)0x20103daf = 0;
    *(uint8_t*)0x20103db0 = 0;
    *(uint8_t*)0x20103db1 = 0;
    *(uint8_t*)0x20103db2 = 0;
    *(uint8_t*)0x20103db3 = 0;
    *(uint8_t*)0x20103db4 = 0;
    *(uint8_t*)0x20103db5 = 0;
    *(uint8_t*)0x20103db6 = 0;
    *(uint8_t*)0x20103db7 = 0;
    *(uint16_t*)0x20103db8 = 0;
    *(uint8_t*)0x20103dba = 8;
    *(uint8_t*)0x20103dbb = 4;
    *(uint8_t*)0x20103dbc = 0x4b;
    *(uint32_t*)0x20103dc0 = 0;
    *(uint16_t*)0x20103dc4 = 0x100;
    *(uint16_t*)0x20103dc6 = 0x128;
    *(uint32_t*)0x20103dc8 = 0;
    *(uint64_t*)0x20103dd0 = 0;
    *(uint64_t*)0x20103dd8 = 0;
    *(uint16_t*)0x20103de0 = 0x30;
    memcpy((void*)0x20103de2, "\x61\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x00",
           29);
    *(uint8_t*)0x20103dff = 0;
    *(uint32_t*)0x20103e00 = htobe32(0x4d4);
    *(uint32_t*)0x20103e04 = htobe32(0x4d4);
    *(uint32_t*)0x20103e08 = 9;
    *(uint8_t*)0x20103e0c = 0xf9;
    *(uint8_t*)0x20103e0d = 1;
    *(uint16_t*)0x20103e10 = 0x28;
    memcpy((void*)0x20103e12, "\x68\x6c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x00",
           29);
    *(uint8_t*)0x20103e2f = 0;
    *(uint8_t*)0x20103e30 = 0;
    *(uint8_t*)0x20103e31 = 3;
    *(uint16_t*)0x20103e38 = 0x28;
    memcpy((void*)0x20103e3a, "\x54\x43\x50\x4d\x53\x53\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x00",
           29);
    *(uint8_t*)0x20103e57 = 0;
    *(uint16_t*)0x20103e58 = 3;
    *(uint8_t*)0x20103e60 = 0;
    *(uint8_t*)0x20103e61 = 0;
    *(uint8_t*)0x20103e62 = 0;
    *(uint8_t*)0x20103e63 = 0;
    *(uint8_t*)0x20103e64 = 0;
    *(uint8_t*)0x20103e65 = 0;
    *(uint8_t*)0x20103e66 = 0;
    *(uint8_t*)0x20103e67 = 0;
    *(uint8_t*)0x20103e68 = 0;
    *(uint8_t*)0x20103e69 = 0;
    *(uint8_t*)0x20103e6a = 0;
    *(uint8_t*)0x20103e6b = 0;
    *(uint8_t*)0x20103e6c = 0;
    *(uint8_t*)0x20103e6d = 0;
    *(uint8_t*)0x20103e6e = 0;
    *(uint8_t*)0x20103e6f = 0;
    *(uint8_t*)0x20103e70 = 0;
    *(uint8_t*)0x20103e71 = 0;
    *(uint8_t*)0x20103e72 = 0;
    *(uint8_t*)0x20103e73 = 0;
    *(uint8_t*)0x20103e74 = 0;
    *(uint8_t*)0x20103e75 = 0;
    *(uint8_t*)0x20103e76 = 0;
    *(uint8_t*)0x20103e77 = 0;
    *(uint8_t*)0x20103e78 = 0;
    *(uint8_t*)0x20103e79 = 0;
    *(uint8_t*)0x20103e7a = 0;
    *(uint8_t*)0x20103e7b = 0;
    *(uint8_t*)0x20103e7c = 0;
    *(uint8_t*)0x20103e7d = 0;
    *(uint8_t*)0x20103e7e = 0;
    *(uint8_t*)0x20103e7f = 0;
    *(uint8_t*)0x20103e80 = 0;
    *(uint8_t*)0x20103e81 = 0;
    *(uint8_t*)0x20103e82 = 0;
    *(uint8_t*)0x20103e83 = 0;
    *(uint8_t*)0x20103e84 = 0;
    *(uint8_t*)0x20103e85 = 0;
    *(uint8_t*)0x20103e86 = 0;
    *(uint8_t*)0x20103e87 = 0;
    *(uint8_t*)0x20103e88 = 0;
    *(uint8_t*)0x20103e89 = 0;
    *(uint8_t*)0x20103e8a = 0;
    *(uint8_t*)0x20103e8b = 0;
    *(uint8_t*)0x20103e8c = 0;
    *(uint8_t*)0x20103e8d = 0;
    *(uint8_t*)0x20103e8e = 0;
    *(uint8_t*)0x20103e8f = 0;
    *(uint8_t*)0x20103e90 = 0;
    *(uint8_t*)0x20103e91 = 0;
    *(uint8_t*)0x20103e92 = 0;
    *(uint8_t*)0x20103e93 = 0;
    *(uint8_t*)0x20103e94 = 0;
    *(uint8_t*)0x20103e95 = 0;
    *(uint8_t*)0x20103e96 = 0;
    *(uint8_t*)0x20103e97 = 0;
    *(uint8_t*)0x20103e98 = 0;
    *(uint8_t*)0x20103e99 = 0;
    *(uint8_t*)0x20103e9a = 0;
    *(uint8_t*)0x20103e9b = 0;
    *(uint8_t*)0x20103e9c = 0;
    *(uint8_t*)0x20103e9d = 0;
    *(uint8_t*)0x20103e9e = 0;
    *(uint8_t*)0x20103e9f = 0;
    *(uint8_t*)0x20103ea0 = 0;
    *(uint8_t*)0x20103ea1 = 0;
    *(uint8_t*)0x20103ea2 = 0;
    *(uint8_t*)0x20103ea3 = 0;
    *(uint8_t*)0x20103ea4 = 0;
    *(uint8_t*)0x20103ea5 = 0;
    *(uint8_t*)0x20103ea6 = 0;
    *(uint8_t*)0x20103ea7 = 0;
    *(uint8_t*)0x20103ea8 = 0;
    *(uint8_t*)0x20103ea9 = 0;
    *(uint8_t*)0x20103eaa = 0;
    *(uint8_t*)0x20103eab = 0;
    *(uint8_t*)0x20103eac = 0;
    *(uint8_t*)0x20103ead = 0;
    *(uint8_t*)0x20103eae = 0;
    *(uint8_t*)0x20103eaf = 0;
    *(uint8_t*)0x20103eb0 = 0;
    *(uint8_t*)0x20103eb1 = 0;
    *(uint8_t*)0x20103eb2 = 0;
    *(uint8_t*)0x20103eb3 = 0;
    *(uint8_t*)0x20103eb4 = 0;
    *(uint8_t*)0x20103eb5 = 0;
    *(uint8_t*)0x20103eb6 = 0;
    *(uint8_t*)0x20103eb7 = 0;
    *(uint8_t*)0x20103eb8 = 0;
    *(uint8_t*)0x20103eb9 = 0;
    *(uint8_t*)0x20103eba = 0;
    *(uint8_t*)0x20103ebb = 0;
    *(uint8_t*)0x20103ebc = 0;
    *(uint8_t*)0x20103ebd = 0;
    *(uint8_t*)0x20103ebe = 0;
    *(uint8_t*)0x20103ebf = 0;
    *(uint8_t*)0x20103ec0 = 0;
    *(uint8_t*)0x20103ec1 = 0;
    *(uint8_t*)0x20103ec2 = 0;
    *(uint8_t*)0x20103ec3 = 0;
    *(uint8_t*)0x20103ec4 = 0;
    *(uint8_t*)0x20103ec5 = 0;
    *(uint8_t*)0x20103ec6 = 0;
    *(uint8_t*)0x20103ec7 = 0;
    *(uint8_t*)0x20103ec8 = 0;
    *(uint8_t*)0x20103ec9 = 0;
    *(uint8_t*)0x20103eca = 0;
    *(uint8_t*)0x20103ecb = 0;
    *(uint8_t*)0x20103ecc = 0;
    *(uint8_t*)0x20103ecd = 0;
    *(uint8_t*)0x20103ece = 0;
    *(uint8_t*)0x20103ecf = 0;
    *(uint8_t*)0x20103ed0 = 0;
    *(uint8_t*)0x20103ed1 = 0;
    *(uint8_t*)0x20103ed2 = 0;
    *(uint8_t*)0x20103ed3 = 0;
    *(uint8_t*)0x20103ed4 = 0;
    *(uint8_t*)0x20103ed5 = 0;
    *(uint8_t*)0x20103ed6 = 0;
    *(uint8_t*)0x20103ed7 = 0;
    *(uint8_t*)0x20103ed8 = 0;
    *(uint8_t*)0x20103ed9 = 0;
    *(uint8_t*)0x20103eda = 0;
    *(uint8_t*)0x20103edb = 0;
    *(uint8_t*)0x20103edc = 0;
    *(uint8_t*)0x20103edd = 0;
    *(uint8_t*)0x20103ede = 0;
    *(uint8_t*)0x20103edf = 0;
    *(uint8_t*)0x20103ee0 = 0;
    *(uint8_t*)0x20103ee1 = 0;
    *(uint8_t*)0x20103ee2 = 0;
    *(uint8_t*)0x20103ee3 = 0;
    *(uint8_t*)0x20103ee4 = 0;
    *(uint8_t*)0x20103ee5 = 0;
    *(uint8_t*)0x20103ee6 = 0;
    *(uint8_t*)0x20103ee7 = 0;
    *(uint32_t*)0x20103ee8 = 0;
    *(uint16_t*)0x20103eec = 0xa8;
    *(uint16_t*)0x20103eee = 0xd0;
    *(uint32_t*)0x20103ef0 = 0;
    *(uint64_t*)0x20103ef8 = 0;
    *(uint64_t*)0x20103f00 = 0;
    *(uint16_t*)0x20103f08 = 0x28;
    memcpy((void*)0x20103f0a, "\x53\x45\x54\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x00",
           29);
    *(uint8_t*)0x20103f27 = 1;
    *(uint16_t*)0x20103f28 = 0x81;
    *(uint8_t*)0x20103f2a = 8;
    *(uint8_t*)0x20103f2b = 3;
    *(uint16_t*)0x20103f2c = 3;
    *(uint8_t*)0x20103f2e = -1;
    *(uint8_t*)0x20103f2f = 4;
    *(uint8_t*)0x20103f30 = 0;
    *(uint8_t*)0x20103f31 = 0;
    *(uint8_t*)0x20103f32 = 0;
    *(uint8_t*)0x20103f33 = 0;
    *(uint8_t*)0x20103f34 = 0;
    *(uint8_t*)0x20103f35 = 0;
    *(uint8_t*)0x20103f36 = 0;
    *(uint8_t*)0x20103f37 = 0;
    *(uint8_t*)0x20103f38 = 0;
    *(uint8_t*)0x20103f39 = 0;
    *(uint8_t*)0x20103f3a = 0;
    *(uint8_t*)0x20103f3b = 0;
    *(uint8_t*)0x20103f3c = 0;
    *(uint8_t*)0x20103f3d = 0;
    *(uint8_t*)0x20103f3e = 0;
    *(uint8_t*)0x20103f3f = 0;
    *(uint8_t*)0x20103f40 = 0;
    *(uint8_t*)0x20103f41 = 0;
    *(uint8_t*)0x20103f42 = 0;
    *(uint8_t*)0x20103f43 = 0;
    *(uint8_t*)0x20103f44 = 0;
    *(uint8_t*)0x20103f45 = 0;
    *(uint8_t*)0x20103f46 = 0;
    *(uint8_t*)0x20103f47 = 0;
    *(uint8_t*)0x20103f48 = 0;
    *(uint8_t*)0x20103f49 = 0;
    *(uint8_t*)0x20103f4a = 0;
    *(uint8_t*)0x20103f4b = 0;
    *(uint8_t*)0x20103f4c = 0;
    *(uint8_t*)0x20103f4d = 0;
    *(uint8_t*)0x20103f4e = 0;
    *(uint8_t*)0x20103f4f = 0;
    *(uint8_t*)0x20103f50 = 0;
    *(uint8_t*)0x20103f51 = 0;
    *(uint8_t*)0x20103f52 = 0;
    *(uint8_t*)0x20103f53 = 0;
    *(uint8_t*)0x20103f54 = 0;
    *(uint8_t*)0x20103f55 = 0;
    *(uint8_t*)0x20103f56 = 0;
    *(uint8_t*)0x20103f57 = 0;
    *(uint8_t*)0x20103f58 = 0;
    *(uint8_t*)0x20103f59 = 0;
    *(uint8_t*)0x20103f5a = 0;
    *(uint8_t*)0x20103f5b = 0;
    *(uint8_t*)0x20103f5c = 0;
    *(uint8_t*)0x20103f5d = 0;
    *(uint8_t*)0x20103f5e = 0;
    *(uint8_t*)0x20103f5f = 0;
    *(uint8_t*)0x20103f60 = 0;
    *(uint8_t*)0x20103f61 = 0;
    *(uint8_t*)0x20103f62 = 0;
    *(uint8_t*)0x20103f63 = 0;
    *(uint8_t*)0x20103f64 = 0;
    *(uint8_t*)0x20103f65 = 0;
    *(uint8_t*)0x20103f66 = 0;
    *(uint8_t*)0x20103f67 = 0;
    *(uint8_t*)0x20103f68 = 0;
    *(uint8_t*)0x20103f69 = 0;
    *(uint8_t*)0x20103f6a = 0;
    *(uint8_t*)0x20103f6b = 0;
    *(uint8_t*)0x20103f6c = 0;
    *(uint8_t*)0x20103f6d = 0;
    *(uint8_t*)0x20103f6e = 0;
    *(uint8_t*)0x20103f6f = 0;
    *(uint8_t*)0x20103f70 = 0;
    *(uint8_t*)0x20103f71 = 0;
    *(uint8_t*)0x20103f72 = 0;
    *(uint8_t*)0x20103f73 = 0;
    *(uint8_t*)0x20103f74 = 0;
    *(uint8_t*)0x20103f75 = 0;
    *(uint8_t*)0x20103f76 = 0;
    *(uint8_t*)0x20103f77 = 0;
    *(uint8_t*)0x20103f78 = 0;
    *(uint8_t*)0x20103f79 = 0;
    *(uint8_t*)0x20103f7a = 0;
    *(uint8_t*)0x20103f7b = 0;
    *(uint8_t*)0x20103f7c = 0;
    *(uint8_t*)0x20103f7d = 0;
    *(uint8_t*)0x20103f7e = 0;
    *(uint8_t*)0x20103f7f = 0;
    *(uint8_t*)0x20103f80 = 0;
    *(uint8_t*)0x20103f81 = 0;
    *(uint8_t*)0x20103f82 = 0;
    *(uint8_t*)0x20103f83 = 0;
    *(uint8_t*)0x20103f84 = 0;
    *(uint8_t*)0x20103f85 = 0;
    *(uint8_t*)0x20103f86 = 0;
    *(uint8_t*)0x20103f87 = 0;
    *(uint8_t*)0x20103f88 = 0;
    *(uint8_t*)0x20103f89 = 0;
    *(uint8_t*)0x20103f8a = 0;
    *(uint8_t*)0x20103f8b = 0;
    *(uint8_t*)0x20103f8c = 0;
    *(uint8_t*)0x20103f8d = 0;
    *(uint8_t*)0x20103f8e = 0;
    *(uint8_t*)0x20103f8f = 0;
    *(uint8_t*)0x20103f90 = 0;
    *(uint8_t*)0x20103f91 = 0;
    *(uint8_t*)0x20103f92 = 0;
    *(uint8_t*)0x20103f93 = 0;
    *(uint8_t*)0x20103f94 = 0;
    *(uint8_t*)0x20103f95 = 0;
    *(uint8_t*)0x20103f96 = 0;
    *(uint8_t*)0x20103f97 = 0;
    *(uint8_t*)0x20103f98 = 0;
    *(uint8_t*)0x20103f99 = 0;
    *(uint8_t*)0x20103f9a = 0;
    *(uint8_t*)0x20103f9b = 0;
    *(uint8_t*)0x20103f9c = 0;
    *(uint8_t*)0x20103f9d = 0;
    *(uint8_t*)0x20103f9e = 0;
    *(uint8_t*)0x20103f9f = 0;
    *(uint8_t*)0x20103fa0 = 0;
    *(uint8_t*)0x20103fa1 = 0;
    *(uint8_t*)0x20103fa2 = 0;
    *(uint8_t*)0x20103fa3 = 0;
    *(uint8_t*)0x20103fa4 = 0;
    *(uint8_t*)0x20103fa5 = 0;
    *(uint8_t*)0x20103fa6 = 0;
    *(uint8_t*)0x20103fa7 = 0;
    *(uint8_t*)0x20103fa8 = 0;
    *(uint8_t*)0x20103fa9 = 0;
    *(uint8_t*)0x20103faa = 0;
    *(uint8_t*)0x20103fab = 0;
    *(uint8_t*)0x20103fac = 0;
    *(uint8_t*)0x20103fad = 0;
    *(uint8_t*)0x20103fae = 0;
    *(uint8_t*)0x20103faf = 0;
    *(uint8_t*)0x20103fb0 = 0;
    *(uint8_t*)0x20103fb1 = 0;
    *(uint8_t*)0x20103fb2 = 0;
    *(uint8_t*)0x20103fb3 = 0;
    *(uint8_t*)0x20103fb4 = 0;
    *(uint8_t*)0x20103fb5 = 0;
    *(uint8_t*)0x20103fb6 = 0;
    *(uint8_t*)0x20103fb7 = 0;
    *(uint32_t*)0x20103fb8 = 0;
    *(uint16_t*)0x20103fbc = 0xa8;
    *(uint16_t*)0x20103fbe = 0xd0;
    *(uint32_t*)0x20103fc0 = 0;
    *(uint64_t*)0x20103fc8 = 0;
    *(uint64_t*)0x20103fd0 = 0;
    *(uint16_t*)0x20103fd8 = 0x28;
    memcpy((void*)0x20103fda, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                              "\x00\x00\x00\x00\x00",
           29);
    *(uint8_t*)0x20103ff7 = 0;
    *(uint32_t*)0x20103ff8 = 0xfffffffe;
    syscall(__NR_setsockopt, r[0], 0x29, 0x40, 0x20103ba0, 0x460);
    break;
  }
}

void test()
{
  memset(r, -1, sizeof(r));
  execute(3);
}

int main()
{
  for (;;) {
    loop();
  }
}
