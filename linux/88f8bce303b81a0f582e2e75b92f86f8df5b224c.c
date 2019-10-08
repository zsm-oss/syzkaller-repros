// general protection fault in propagate_entity_cfs_rq
// https://syzkaller.appspot.com/bug?id=88f8bce303b81a0f582e2e75b92f86f8df5b224c
// status:open
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/uio.h>
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

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
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

static void vsnprintf_check(char* str, size_t size, const char* format,
                            va_list args)
{
  int rv;

  rv = vsnprintf(str, size, format, args);
  if (rv < 0)
    fail("tun: snprintf failed");
  if ((size_t)rv >= size)
    fail("tun: string '%s...' doesn't fit into buffer", str);
}

#define COMMAND_MAX_LEN 128
#define PATH_PREFIX                                                            \
  "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin "
#define PATH_PREFIX_LEN (sizeof(PATH_PREFIX) - 1)

static void execute_command(bool panic, const char* format, ...)
{
  va_list args;
  char command[PATH_PREFIX_LEN + COMMAND_MAX_LEN];
  int rv;

  va_start(args, format);
  memcpy(command, PATH_PREFIX, PATH_PREFIX_LEN);
  vsnprintf_check(command + PATH_PREFIX_LEN, COMMAND_MAX_LEN, format, args);
  va_end(args);
  rv = system(command);
  if (rv) {
    if (panic)
      fail("command '%s' failed: %d", &command[0], rv);
  }
}

#define DEV_IPV4 "172.20.20.%d"
#define DEV_IPV6 "fe80::%02hx"
#define DEV_MAC "aa:aa:aa:aa:aa:%02hx"

static void snprintf_check(char* str, size_t size, const char* format, ...)
{
  va_list args;

  va_start(args, format);
  vsnprintf_check(str, size, format, args);
  va_end(args);
}

static void initialize_netdevices(void)
{
  unsigned i;
  const char* devtypes[] = {"ip6gretap", "bridge", "vcan", "bond", "team"};
  const char* devnames[] = {"lo",
                            "sit0",
                            "bridge0",
                            "vcan0",
                            "tunl0",
                            "gre0",
                            "gretap0",
                            "ip_vti0",
                            "ip6_vti0",
                            "ip6tnl0",
                            "ip6gre0",
                            "ip6gretap0",
                            "erspan0",
                            "bond0",
                            "veth0",
                            "veth1",
                            "team0",
                            "veth0_to_bridge",
                            "veth1_to_bridge",
                            "veth0_to_bond",
                            "veth1_to_bond",
                            "veth0_to_team",
                            "veth1_to_team"};
  const char* devmasters[] = {"bridge", "bond", "team"};

  for (i = 0; i < sizeof(devtypes) / (sizeof(devtypes[0])); i++)
    execute_command(0, "ip link add dev %s0 type %s", devtypes[i], devtypes[i]);
  execute_command(0, "ip link add type veth");

  for (i = 0; i < sizeof(devmasters) / (sizeof(devmasters[0])); i++) {
    execute_command(
        0, "ip link add name %s_slave_0 type veth peer name veth0_to_%s",
        devmasters[i], devmasters[i]);
    execute_command(
        0, "ip link add name %s_slave_1 type veth peer name veth1_to_%s",
        devmasters[i], devmasters[i]);
    execute_command(0, "ip link set %s_slave_0 master %s0", devmasters[i],
                    devmasters[i]);
    execute_command(0, "ip link set %s_slave_1 master %s0", devmasters[i],
                    devmasters[i]);
    execute_command(0, "ip link set veth0_to_%s up", devmasters[i]);
    execute_command(0, "ip link set veth1_to_%s up", devmasters[i]);
  }
  execute_command(0, "ip link set bridge_slave_0 up");
  execute_command(0, "ip link set bridge_slave_1 up");

  for (i = 0; i < sizeof(devnames) / (sizeof(devnames[0])); i++) {
    char addr[32];
    snprintf_check(addr, sizeof(addr), DEV_IPV4, i + 10);
    execute_command(0, "ip -4 addr add %s/24 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_IPV6, i + 10);
    execute_command(0, "ip -6 addr add %s/120 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_MAC, i + 10);
    execute_command(0, "ip link set dev %s address %s", devnames[i], addr);
    execute_command(0, "ip link set dev %s up", devnames[i]);
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

  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }
  initialize_netdevices();
  loop();
  doexit(1);
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

uint64_t r[1] = {0xffffffffffffffff};
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    *(uint64_t*)0x20000740 = 0x20000040;
    *(uint16_t*)0x20000040 = 2;
    *(uint16_t*)0x20000042 = htobe16(0);
    *(uint32_t*)0x20000044 = htobe32(0xe0000002);
    *(uint8_t*)0x20000048 = 0;
    *(uint8_t*)0x20000049 = 0;
    *(uint8_t*)0x2000004a = 0;
    *(uint8_t*)0x2000004b = 0;
    *(uint8_t*)0x2000004c = 0;
    *(uint8_t*)0x2000004d = 0;
    *(uint8_t*)0x2000004e = 0;
    *(uint8_t*)0x2000004f = 0;
    *(uint32_t*)0x20000748 = 0x80;
    *(uint64_t*)0x20000750 = 0x20000200;
    *(uint64_t*)0x20000758 = 0;
    *(uint64_t*)0x20000760 = 0x20000140;
    memcpy((void*)0x20000140, "\x20\x00\x00\x00\x4a\xf2\xf9\x0d\x54\x4f\x25\x66"
                              "\xff\xc7\xe5\x1a\xbb\xe8\xb0\x9f\x77\xae\x51\xbc"
                              "\x86",
           25);
    *(uint64_t*)0x20000768 = 0x19;
    *(uint32_t*)0x20000770 = 0;
    syscall(__NR_sendmsg, -1, 0x20000740, 0);
    break;
  case 1:
    res = syscall(__NR_socket, 0x11, 3, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 2:
    *(uint64_t*)0x20000340 = 0;
    *(uint32_t*)0x20000348 = 0;
    *(uint64_t*)0x20000350 = 0x20000040;
    *(uint64_t*)0x20000040 = 0x20000140;
    *(uint64_t*)0x20000048 = 0;
    *(uint64_t*)0x20000358 = 1;
    *(uint64_t*)0x20000360 = 0;
    *(uint64_t*)0x20000368 = 0;
    *(uint32_t*)0x20000370 = 0;
    syscall(__NR_sendmsg, -1, 0x20000340, 0);
    break;
  case 3:
    *(uint64_t*)0x20000380 = 0x20000140;
    *(uint16_t*)0x20000140 = 4;
    *(uint16_t*)0x20000142 = htobe16(1);
    *(uint32_t*)0x20000144 = htobe32(0);
    memcpy((void*)0x20000148, "\x09\x50\xfe\x4a\xdb\xa7", 6);
    *(uint8_t*)0x2000014e = 0;
    *(uint8_t*)0x2000014f = 0;
    *(uint32_t*)0x20000388 = 0x80;
    *(uint64_t*)0x20000390 = 0x20000000;
    *(uint64_t*)0x20000398 = 0;
    *(uint64_t*)0x200003a0 = 0x20000240;
    *(uint64_t*)0x200003a8 = 0;
    *(uint32_t*)0x200003b0 = 0;
    syscall(__NR_sendmsg, -1, 0x20000380, 0);
    break;
  case 4:
    *(uint32_t*)0x20000080 = -1;
    syscall(__NR_setsockopt, r[0], 0x107, 0xf, 0x20000080, 0x1ce);
    break;
  case 5:
    *(uint64_t*)0x20000480 = 0x200000c0;
    *(uint16_t*)0x200000c0 = 4;
    *(uint16_t*)0x200000c2 = htobe16(0x894f);
    *(uint32_t*)0x200000c4 = 5;
    *(uint8_t*)0x200000c8 = 0xfe;
    *(uint8_t*)0x200000c9 = 0x80;
    *(uint8_t*)0x200000ca = 0;
    *(uint8_t*)0x200000cb = 0;
    *(uint8_t*)0x200000cc = 0;
    *(uint8_t*)0x200000cd = 0;
    *(uint8_t*)0x200000ce = 0;
    *(uint8_t*)0x200000cf = 0;
    *(uint8_t*)0x200000d0 = 0;
    *(uint8_t*)0x200000d1 = 0;
    *(uint8_t*)0x200000d2 = 0;
    *(uint8_t*)0x200000d3 = 0;
    *(uint8_t*)0x200000d4 = 0;
    *(uint8_t*)0x200000d5 = 0;
    *(uint8_t*)0x200000d6 = 0;
    *(uint8_t*)0x200000d7 = 0;
    *(uint32_t*)0x200000d8 = 0;
    *(uint32_t*)0x20000488 = 0x80;
    *(uint64_t*)0x20000490 = 0x20000340;
    *(uint64_t*)0x20000498 = 0x25d;
    *(uint64_t*)0x200004a0 = 0x20000380;
    *(uint64_t*)0x200004a8 = 0;
    *(uint32_t*)0x200004b0 = 0;
    syscall(__NR_sendmsg, r[0], 0x20000480, 0);
    break;
  case 6:
    *(uint32_t*)0x20000140 = 2;
    *(uint32_t*)0x20000144 = 0x70;
    *(uint8_t*)0x20000148 = 0;
    *(uint8_t*)0x20000149 = -1;
    *(uint8_t*)0x2000014a = 4;
    *(uint8_t*)0x2000014b = 0x38;
    *(uint32_t*)0x2000014c = 0;
    *(uint64_t*)0x20000150 = 9;
    *(uint64_t*)0x20000158 = 0x40000;
    *(uint64_t*)0x20000160 = 8;
    STORE_BY_BITMASK(uint64_t, 0x20000168, 0x101, 0, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 2, 1, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 7, 2, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 0x200, 3, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 0x100000001, 4, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 7, 5, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 0, 6, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 5, 7, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 4, 8, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 4, 9, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 3, 10, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 0xb84, 11, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 8, 12, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 0x8000, 13, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 0x2ef, 14, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 0x212, 15, 2);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 2, 17, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 4, 18, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 0xffff, 19, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 5, 20, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 4, 21, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 7, 22, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 5, 23, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 0x601d, 24, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 0x100000001, 25, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 0x93fa, 26, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 4, 27, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 0x80000001, 28, 1);
    STORE_BY_BITMASK(uint64_t, 0x20000168, 0, 29, 35);
    *(uint32_t*)0x20000170 = 7;
    *(uint32_t*)0x20000174 = 4;
    *(uint64_t*)0x20000178 = 9;
    *(uint64_t*)0x20000180 = 3;
    *(uint64_t*)0x20000188 = 0x802;
    *(uint64_t*)0x20000190 = 0x400;
    *(uint32_t*)0x20000198 = 6;
    *(uint32_t*)0x2000019c = 6;
    *(uint64_t*)0x200001a0 = 0;
    *(uint32_t*)0x200001a8 = 7;
    *(uint16_t*)0x200001ac = 0x338;
    *(uint16_t*)0x200001ae = 0;
    syscall(__NR_perf_event_open, 0x20000140, -1, 4, 0xffffff9c, 4);
    break;
  }
}

void loop()
{
  execute(7);
  collide = 1;
  execute(7);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  use_temporary_dir();
  do_sandbox_none();
  return 0;
}
