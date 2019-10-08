// BUG: using __this_cpu_add() in preemptible code in tcp_try_rmem_schedule
// https://syzkaller.appspot.com/bug?id=31bed0793fef1f709231e222e9f95eeeb466cdd8
// status:open
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
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
#include <setjmp.h>
#include <signal.h>
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

static void snprintf_check(char* str, size_t size, const char* format, ...)
{
  va_list args;

  va_start(args, format);
  vsnprintf_check(str, size, format, args);
  va_end(args);
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
  rv = system(command);
  if (panic && rv != 0)
    fail("tun: command \"%s\" failed with code %d", &command[0], rv);

  va_end(args);
}

static int tunfd = -1;
static int tun_frags_enabled;

#define SYZ_TUN_MAX_PACKET_SIZE 1000

#define MAX_PIDS 32
#define ADDR_MAX_LEN 32

#define LOCAL_MAC "aa:aa:aa:aa:%02hx:aa"
#define REMOTE_MAC "aa:aa:aa:aa:%02hx:bb"

#define LOCAL_IPV4 "172.20.%d.170"
#define REMOTE_IPV4 "172.20.%d.187"

#define LOCAL_IPV6 "fe80::%02hx:aa"
#define REMOTE_IPV6 "fe80::%02hx:bb"

#define IFF_NAPI 0x0010
#define IFF_NAPI_FRAGS 0x0020

static void initialize_tun(int id)
{
  if (id >= MAX_PIDS)
    fail("tun: no more than %d executors", MAX_PIDS);

  tunfd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
  if (tunfd == -1) {
    printf("tun: can't open /dev/net/tun: please enable CONFIG_TUN=y\n");
    printf("otherwise fuzzing or reproducing might not work as intended\n");
    return;
  }

  char iface[IFNAMSIZ];
  snprintf_check(iface, sizeof(iface), "syz%d", id);

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, iface, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_NAPI | IFF_NAPI_FRAGS;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0) {
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
      fail("tun: ioctl(TUNSETIFF) failed");
  }
  if (ioctl(tunfd, TUNGETIFF, (void*)&ifr) < 0)
    fail("tun: ioctl(TUNGETIFF) failed");
  tun_frags_enabled = (ifr.ifr_flags & IFF_NAPI_FRAGS) != 0;

  char local_mac[ADDR_MAX_LEN];
  snprintf_check(local_mac, sizeof(local_mac), LOCAL_MAC, id);
  char remote_mac[ADDR_MAX_LEN];
  snprintf_check(remote_mac, sizeof(remote_mac), REMOTE_MAC, id);

  char local_ipv4[ADDR_MAX_LEN];
  snprintf_check(local_ipv4, sizeof(local_ipv4), LOCAL_IPV4, id);
  char remote_ipv4[ADDR_MAX_LEN];
  snprintf_check(remote_ipv4, sizeof(remote_ipv4), REMOTE_IPV4, id);

  char local_ipv6[ADDR_MAX_LEN];
  snprintf_check(local_ipv6, sizeof(local_ipv6), LOCAL_IPV6, id);
  char remote_ipv6[ADDR_MAX_LEN];
  snprintf_check(remote_ipv6, sizeof(remote_ipv6), REMOTE_IPV6, id);

  execute_command(1, "sysctl -w net.ipv6.conf.%s.accept_dad=0", iface);

  execute_command(1, "sysctl -w net.ipv6.conf.%s.router_solicitations=0",
                  iface);

  execute_command(1, "ip link set dev %s address %s", iface, local_mac);
  execute_command(1, "ip addr add %s/24 dev %s", local_ipv4, iface);
  execute_command(1, "ip -6 addr add %s/120 dev %s", local_ipv6, iface);
  execute_command(1, "ip neigh add %s lladdr %s dev %s nud permanent",
                  remote_ipv4, remote_mac, iface);
  execute_command(1, "ip -6 neigh add %s lladdr %s dev %s nud permanent",
                  remote_ipv6, remote_mac, iface);
  execute_command(1, "ip link set dev %s up", iface);
}

#define DEV_IPV4 "172.20.%d.%d"
#define DEV_IPV6 "fe80::%02hx:%02hx"
#define DEV_MAC "aa:aa:aa:aa:%02hx:%02hx"

static void initialize_netdevices(int id)
{
  unsigned i;
  const char* devtypes[] = {"ip6gretap", "bridge", "vcan"};
  const char* devnames[] = {"lo",       "sit0",    "bridge0", "vcan0",
                            "tunl0",    "gre0",    "gretap0", "ip_vti0",
                            "ip6_vti0", "ip6tnl0", "ip6gre0", "ip6gretap0",
                            "erspan0"};

  for (i = 0; i < sizeof(devtypes) / (sizeof(devtypes[0])); i++)
    execute_command(0, "ip link add dev %s0 type %s", devtypes[i], devtypes[i]);
  for (i = 0; i < sizeof(devnames) / (sizeof(devnames[0])); i++) {
    char addr[ADDR_MAX_LEN];
    snprintf_check(addr, sizeof(addr), DEV_IPV4, id, id + 10);
    execute_command(0, "ip -4 addr add %s/24 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_IPV6, id, id + 10);
    execute_command(0, "ip -6 addr add %s/120 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_MAC, id, id + 10);
    execute_command(0, "ip link set dev %s address %s", devnames[i], addr);
    execute_command(0, "ip link set dev %s up", devnames[i]);
  }
}

static void setup_tun(uint64_t pid, bool enable_tun)
{
  if (enable_tun) {
    initialize_tun(pid);
    initialize_netdevices(pid);
  }
}

#define MAX_FRAGS 4
struct vnet_fragmentation {
  uint32_t full;
  uint32_t count;
  uint32_t frags[MAX_FRAGS];
};

static uintptr_t syz_emit_ethernet(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (tunfd < 0)
    return (uintptr_t)-1;

  uint32_t length = a0;
  char* data = (char*)a1;

  struct vnet_fragmentation* frags = (struct vnet_fragmentation*)a2;
  struct iovec vecs[MAX_FRAGS + 1];
  uint32_t nfrags = 0;
  if (!tun_frags_enabled || frags == NULL) {
    vecs[nfrags].iov_base = data;
    vecs[nfrags].iov_len = length;
    nfrags++;
  } else {
    bool full = true;
    uint32_t i, count = 0;
    NONFAILING(full = frags->full);
    NONFAILING(count = frags->count);
    if (count > MAX_FRAGS)
      count = MAX_FRAGS;
    for (i = 0; i < count && length != 0; i++) {
      uint32_t size = 0;
      NONFAILING(size = frags->frags[i]);
      if (size > length)
        size = length;
      vecs[nfrags].iov_base = data;
      vecs[nfrags].iov_len = size;
      nfrags++;
      data += size;
      length -= size;
    }
    if (length != 0 && (full || nfrags == 0)) {
      vecs[nfrags].iov_base = data;
      vecs[nfrags].iov_len = length;
      nfrags++;
    }
  }
  return writev(tunfd, vecs, nfrags);
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

  if (unshare(CLONE_NEWNS)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(CLONE_NEWCGROUP)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
}

static int do_sandbox_none(int executor_pid, bool enable_tun)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid < 0)
    fail("sandbox fork failed");
  if (pid)
    return pid;

  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }
  setup_tun(executor_pid, enable_tun);

  loop();
  doexit(1);
}

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_socket, 2, 1, 0);
  NONFAILING(*(uint32_t*)0x20c81000 = 1);
  syscall(__NR_setsockopt, r[0], 6, 0x10000000013, 0x20c81000, 4);
  NONFAILING(*(uint32_t*)0x200b2000 = 1);
  syscall(__NR_setsockopt, r[0], 6, 0x14, 0x200b2000, 4);
  NONFAILING(*(uint16_t*)0x20172000 = 2);
  NONFAILING(*(uint16_t*)0x20172002 = 0);
  NONFAILING(*(uint8_t*)0x20172004 = 0xac);
  NONFAILING(*(uint8_t*)0x20172005 = 0x14);
  NONFAILING(*(uint8_t*)0x20172006 = 0);
  NONFAILING(*(uint8_t*)0x20172007 = 0xa);
  NONFAILING(*(uint8_t*)0x20172008 = 0);
  NONFAILING(*(uint8_t*)0x20172009 = 0);
  NONFAILING(*(uint8_t*)0x2017200a = 0);
  NONFAILING(*(uint8_t*)0x2017200b = 0);
  NONFAILING(*(uint8_t*)0x2017200c = 0);
  NONFAILING(*(uint8_t*)0x2017200d = 0);
  NONFAILING(*(uint8_t*)0x2017200e = 0);
  NONFAILING(*(uint8_t*)0x2017200f = 0);
  syscall(__NR_sendto, r[0], 0x20482000, 0xfffffffffffffd1f, 0x800000120000001,
          0x20172000, 0x10);
  NONFAILING(*(uint32_t*)0x205b5ffc = 0);
  syscall(__NR_setsockopt, r[0], 1, 8, 0x205b5ffc, 4);
  NONFAILING(*(uint64_t*)0x20f4a000 = 0x20756000);
  NONFAILING(*(uint16_t*)0x20756000 = 0x1d);
  NONFAILING(*(uint32_t*)0x20756004 = 0);
  NONFAILING(*(uint32_t*)0x20756008 = 0);
  NONFAILING(*(uint32_t*)0x2075600c = 0);
  NONFAILING(*(uint32_t*)0x20f4a008 = 0x10);
  NONFAILING(*(uint64_t*)0x20f4a010 = 0x20205000);
  NONFAILING(*(uint64_t*)0x20205000 = 0x202b2000);
  NONFAILING(memcpy((void*)0x202b2000, "A", 1));
  NONFAILING(*(uint64_t*)0x20205008 = 1);
  NONFAILING(*(uint64_t*)0x20f4a018 = 1);
  NONFAILING(*(uint64_t*)0x20f4a020 = 0x20095d08);
  NONFAILING(*(uint64_t*)0x20f4a028 = 0);
  NONFAILING(*(uint32_t*)0x20f4a030 = 0);
  syscall(__NR_sendmsg, r[0], 0x20f4a000, 0);
  NONFAILING(*(uint8_t*)0x208b2000 = 1);
  NONFAILING(*(uint8_t*)0x208b2001 = 0x80);
  NONFAILING(*(uint8_t*)0x208b2002 = 0xc2);
  NONFAILING(*(uint8_t*)0x208b2003 = 0);
  NONFAILING(*(uint8_t*)0x208b2004 = 0);
  NONFAILING(*(uint8_t*)0x208b2005 = 0);
  NONFAILING(*(uint8_t*)0x208b2006 = 0xaa);
  NONFAILING(*(uint8_t*)0x208b2007 = 0xaa);
  NONFAILING(*(uint8_t*)0x208b2008 = 0xaa);
  NONFAILING(*(uint8_t*)0x208b2009 = 0xaa);
  NONFAILING(*(uint8_t*)0x208b200a = 0);
  NONFAILING(*(uint8_t*)0x208b200b = 0);
  NONFAILING(*(uint16_t*)0x208b200c = htobe16(0x86dd));
  NONFAILING(STORE_BY_BITMASK(uint8_t, 0x208b200e, 0, 0, 4));
  NONFAILING(STORE_BY_BITMASK(uint8_t, 0x208b200e, 6, 4, 4));
  NONFAILING(memcpy((void*)0x208b200f, "\x54\x61\xa4", 3));
  NONFAILING(*(uint16_t*)0x208b2012 = htobe16(0x10));
  NONFAILING(*(uint8_t*)0x208b2014 = 0);
  NONFAILING(*(uint8_t*)0x208b2015 = 0);
  NONFAILING(*(uint8_t*)0x208b2016 = 0xfe);
  NONFAILING(*(uint8_t*)0x208b2017 = 0x80);
  NONFAILING(*(uint8_t*)0x208b2018 = 0);
  NONFAILING(*(uint8_t*)0x208b2019 = 0);
  NONFAILING(*(uint8_t*)0x208b201a = 0);
  NONFAILING(*(uint8_t*)0x208b201b = 0);
  NONFAILING(*(uint8_t*)0x208b201c = 0);
  NONFAILING(*(uint8_t*)0x208b201d = 0);
  NONFAILING(*(uint8_t*)0x208b201e = 0);
  NONFAILING(*(uint8_t*)0x208b201f = 0);
  NONFAILING(*(uint8_t*)0x208b2020 = 0);
  NONFAILING(*(uint8_t*)0x208b2021 = 0);
  NONFAILING(*(uint8_t*)0x208b2022 = 0);
  NONFAILING(*(uint8_t*)0x208b2023 = 0);
  NONFAILING(*(uint8_t*)0x208b2024 = 0);
  NONFAILING(*(uint8_t*)0x208b2025 = 0xaa);
  NONFAILING(*(uint8_t*)0x208b2026 = 0);
  NONFAILING(*(uint8_t*)0x208b2027 = 0);
  NONFAILING(*(uint8_t*)0x208b2028 = 0);
  NONFAILING(*(uint8_t*)0x208b2029 = 0);
  NONFAILING(*(uint8_t*)0x208b202a = 0);
  NONFAILING(*(uint8_t*)0x208b202b = 0);
  NONFAILING(*(uint8_t*)0x208b202c = 0);
  NONFAILING(*(uint8_t*)0x208b202d = 0);
  NONFAILING(*(uint8_t*)0x208b202e = 0);
  NONFAILING(*(uint8_t*)0x208b202f = 0);
  NONFAILING(*(uint8_t*)0x208b2030 = 0);
  NONFAILING(*(uint8_t*)0x208b2031 = 0);
  NONFAILING(*(uint8_t*)0x208b2032 = 0);
  NONFAILING(*(uint8_t*)0x208b2033 = 0);
  NONFAILING(*(uint8_t*)0x208b2034 = 0);
  NONFAILING(*(uint8_t*)0x208b2035 = 0);
  NONFAILING(STORE_BY_BITMASK(uint16_t, 0x208b2036, 0, 0, 1));
  NONFAILING(STORE_BY_BITMASK(uint16_t, 0x208b2036, 0, 1, 1));
  NONFAILING(STORE_BY_BITMASK(uint16_t, 0x208b2036, 1, 2, 1));
  NONFAILING(STORE_BY_BITMASK(uint16_t, 0x208b2036, 0, 3, 1));
  NONFAILING(STORE_BY_BITMASK(uint16_t, 0x208b2036, 0, 4, 4));
  NONFAILING(STORE_BY_BITMASK(uint16_t, 0x208b2036, 0, 8, 1));
  NONFAILING(STORE_BY_BITMASK(uint16_t, 0x208b2036, 0, 9, 4));
  NONFAILING(STORE_BY_BITMASK(uint16_t, 0x208b2036, 1, 13, 3));
  NONFAILING(*(uint16_t*)0x208b2038 = htobe16(0x880b));
  NONFAILING(*(uint16_t*)0x208b203a = htobe16(0));
  NONFAILING(*(uint16_t*)0x208b203c = htobe16(0));
  NONFAILING(STORE_BY_BITMASK(uint16_t, 0x208b203e, 0, 0, 1));
  NONFAILING(STORE_BY_BITMASK(uint16_t, 0x208b203e, 0, 1, 1));
  NONFAILING(STORE_BY_BITMASK(uint16_t, 0x208b203e, 0, 2, 1));
  NONFAILING(STORE_BY_BITMASK(uint16_t, 0x208b203e, 0, 3, 1));
  NONFAILING(STORE_BY_BITMASK(uint16_t, 0x208b203e, 0, 4, 9));
  NONFAILING(STORE_BY_BITMASK(uint16_t, 0x208b203e, 0, 13, 3));
  NONFAILING(*(uint16_t*)0x208b2040 = htobe16(0x800));
  NONFAILING(STORE_BY_BITMASK(uint16_t, 0x208b2042, 0, 0, 1));
  NONFAILING(STORE_BY_BITMASK(uint16_t, 0x208b2042, 0, 1, 1));
  NONFAILING(STORE_BY_BITMASK(uint16_t, 0x208b2042, 0, 2, 1));
  NONFAILING(STORE_BY_BITMASK(uint16_t, 0x208b2042, 0, 3, 1));
  NONFAILING(STORE_BY_BITMASK(uint16_t, 0x208b2042, 0, 4, 9));
  NONFAILING(STORE_BY_BITMASK(uint16_t, 0x208b2042, 0, 13, 3));
  NONFAILING(*(uint16_t*)0x208b2044 = htobe16(0x86dd));
  NONFAILING(*(uint32_t*)0x20988ff0 = 0);
  NONFAILING(*(uint32_t*)0x20988ff4 = 1);
  NONFAILING(*(uint32_t*)0x20988ff8 = 0);
  syz_emit_ethernet(0x46, 0x208b2000, 0x20988ff0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0, 3, 0x32, -1, 0);
  install_segv_handler();
  use_temporary_dir();
  int pid = do_sandbox_none(0, true);
  int status = 0;
  while (waitpid(pid, &status, __WALL) != pid) {
  }
  return 0;
}
