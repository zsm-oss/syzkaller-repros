// INFO: task hung in __down
// https://syzkaller.appspot.com/bug?id=588dba2a037382ff91d5cfcba4220f130f9bdca1
// status:open
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <linux/net.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
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
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
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
  if (fd == -1) {
    switch (errno) {
    case EAFNOSUPPORT:
    case ENOPROTOOPT:
      return;
    }
    fail("socket(%d, SOCK_STREAM, IPPROTO_TCP)", family);
  }
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
  if (fd == -1) {
    switch (errno) {
    case EAFNOSUPPORT:
    case ENOPROTOOPT:
      return;
    }
    fail("socket(%d, SOCK_STREAM, IPPROTO_TCP)", family);
  }
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
#include <linux/if.h>
#include <linux/netfilter_bridge/ebtables.h>

struct ebt_table_desc {
  const char* name;
  struct ebt_replace replace;
  char entrytable[XT_TABLE_SIZE];
};

static struct ebt_table_desc ebt_tables[] = {
    {.name = "filter"}, {.name = "nat"}, {.name = "broute"},
};

static void checkpoint_ebtables(void)
{
  socklen_t optlen;
  unsigned i;
  int fd;

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)");
  for (i = 0; i < sizeof(ebt_tables) / sizeof(ebt_tables[0]); i++) {
    struct ebt_table_desc* table = &ebt_tables[i];
    strcpy(table->replace.name, table->name);
    optlen = sizeof(table->replace);
    if (getsockopt(fd, SOL_IP, EBT_SO_GET_INIT_INFO, &table->replace,
                   &optlen)) {
      switch (errno) {
      case EPERM:
      case ENOENT:
      case ENOPROTOOPT:
        continue;
      }
      fail("getsockopt(EBT_SO_GET_INIT_INFO)");
    }
    if (table->replace.entries_size > sizeof(table->entrytable))
      fail("table size is too large: %u", table->replace.entries_size);
    table->replace.num_counters = 0;
    table->replace.entries = table->entrytable;
    optlen = sizeof(table->replace) + table->replace.entries_size;
    if (getsockopt(fd, SOL_IP, EBT_SO_GET_INIT_ENTRIES, &table->replace,
                   &optlen))
      fail("getsockopt(EBT_SO_GET_INIT_ENTRIES)");
  }
  close(fd);
}

static void reset_ebtables()
{
  struct ebt_replace replace;
  char entrytable[XT_TABLE_SIZE];
  socklen_t optlen;
  unsigned i, j, h;
  int fd;

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (fd == -1)
    fail("socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)");
  for (i = 0; i < sizeof(ebt_tables) / sizeof(ebt_tables[0]); i++) {
    struct ebt_table_desc* table = &ebt_tables[i];
    if (table->replace.valid_hooks == 0)
      continue;
    memset(&replace, 0, sizeof(replace));
    strcpy(replace.name, table->name);
    optlen = sizeof(replace);
    if (getsockopt(fd, SOL_IP, EBT_SO_GET_INFO, &replace, &optlen))
      fail("getsockopt(EBT_SO_GET_INFO)");
    replace.num_counters = 0;
    table->replace.entries = 0;
    for (h = 0; h < NF_BR_NUMHOOKS; h++)
      table->replace.hook_entry[h] = 0;
    if (memcmp(&table->replace, &replace, sizeof(table->replace)) == 0) {
      memset(&entrytable, 0, sizeof(entrytable));
      replace.entries = entrytable;
      optlen = sizeof(replace) + replace.entries_size;
      if (getsockopt(fd, SOL_IP, EBT_SO_GET_ENTRIES, &replace, &optlen))
        fail("getsockopt(EBT_SO_GET_ENTRIES)");
      if (memcmp(table->entrytable, entrytable, replace.entries_size) == 0)
        continue;
    }
    for (j = 0, h = 0; h < NF_BR_NUMHOOKS; h++) {
      if (table->replace.valid_hooks & (1 << h)) {
        table->replace.hook_entry[h] =
            (struct ebt_entries*)table->entrytable + j;
        j++;
      }
    }
    table->replace.entries = table->entrytable;
    optlen = sizeof(table->replace) + table->replace.entries_size;
    if (setsockopt(fd, SOL_IP, EBT_SO_SET_ENTRIES, &table->replace, optlen))
      fail("setsockopt(EBT_SO_SET_ENTRIES)");
  }
  close(fd);
}

static void checkpoint_net_namespace(void)
{
  checkpoint_ebtables();
  checkpoint_arptables();
  checkpoint_iptables(ipv4_tables, sizeof(ipv4_tables) / sizeof(ipv4_tables[0]),
                      AF_INET, SOL_IP);
  checkpoint_iptables(ipv6_tables, sizeof(ipv6_tables) / sizeof(ipv6_tables[0]),
                      AF_INET6, SOL_IPV6);
}

static void reset_net_namespace(void)
{
  reset_ebtables();
  reset_arptables();
  reset_iptables(ipv4_tables, sizeof(ipv4_tables) / sizeof(ipv4_tables[0]),
                 AF_INET, SOL_IP);
  reset_iptables(ipv6_tables, sizeof(ipv6_tables) / sizeof(ipv6_tables[0]),
                 AF_INET6, SOL_IPV6);
}

static void execute_one();
extern unsigned long long procid;

static void loop()
{
  checkpoint_net_namespace();
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      fail("clone failed");
    if (pid == 0) {
      prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
      setpgrp();
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
    reset_net_namespace();
  }
}

uint64_t r[1] = {0xffffffffffffffff};
unsigned long long procid;
void execute_one()
{
  long res = 0;
  res = syscall(__NR_socket, 0xa, 2, 0);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20002000 = 0;
  *(uint64_t*)0x20002008 = 0;
  *(uint64_t*)0x20002010 = 0;
  *(uint64_t*)0x20002018 = 0;
  *(uint64_t*)0x20002020 = 0;
  *(uint64_t*)0x20002028 = 0;
  *(uint64_t*)0x20002030 = 0;
  *(uint64_t*)0x20002038 = 0;
  *(uint64_t*)0x20000040 = 0;
  *(uint64_t*)0x20000048 = 0;
  *(uint64_t*)0x20000050 = 0;
  *(uint64_t*)0x20000058 = 0;
  *(uint64_t*)0x20000060 = 0;
  *(uint64_t*)0x20000068 = 0;
  *(uint64_t*)0x20000070 = 0;
  *(uint64_t*)0x20000078 = 0;
  *(uint64_t*)0x20000fc0 = 0x20007fffffff;
  *(uint64_t*)0x20000fc8 = 0;
  *(uint64_t*)0x20000fd0 = 0;
  *(uint64_t*)0x20000fd8 = 0;
  *(uint64_t*)0x20000fe0 = 0;
  *(uint64_t*)0x20000fe8 = 0;
  *(uint64_t*)0x20000ff0 = 2;
  *(uint64_t*)0x20000ff8 = 0;
  *(uint64_t*)0x20001000 = 0;
  *(uint64_t*)0x20001008 = 0x2710;
  syscall(__NR_select, 0x40, 0x20002000, 0x20000040, 0x20000fc0, 0x20001000);
  syscall(__NR_setsockopt, r[0], 0x29, 0x40, 0x20000f9b, 0x4000000000000000);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}
