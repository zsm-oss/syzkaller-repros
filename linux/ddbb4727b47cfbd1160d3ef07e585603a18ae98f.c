// general protection fault in skb_release_data
// https://syzkaller.appspot.com/bug?id=ddbb4727b47cfbd1160d3ef07e585603a18ae98f
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <pthread.h>
#include <sched.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
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

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                          \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)              \
  if ((bf_off) == 0 && (bf_len) == 0) {                                \
    *(type*)(addr) = (type)(val);                                      \
  } else {                                                             \
    type new_val = *(type*)(addr);                                     \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));             \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);  \
    *(type*)(addr) = new_val;                                          \
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
  for (;;) {
  }
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

#define NONFAILING(...)                                                \
  {                                                                    \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);               \
    if (_setjmp(segv_env) == 0) {                                      \
      __VA_ARGS__;                                                     \
    }                                                                  \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);               \
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

static void snprintf_check(char* str, size_t size, const char* format,
                           ...)
{
  va_list args;

  va_start(args, format);
  vsnprintf_check(str, size, format, args);
  va_end(args);
}

#define COMMAND_MAX_LEN 128

static void execute_command(const char* format, ...)
{
  va_list args;
  char command[COMMAND_MAX_LEN];
  int rv;

  va_start(args, format);

  vsnprintf_check(command, sizeof(command), format, args);
  rv = system(command);
  if (rv != 0)
    fail("tun: command \"%s\" failed with code %d", &command[0], rv);

  va_end(args);
}

static int tunfd = -1;

#define SYZ_TUN_MAX_PACKET_SIZE 1000

#define MAX_PIDS 32
#define ADDR_MAX_LEN 32

#define LOCAL_MAC "aa:aa:aa:aa:aa:%02hx"
#define REMOTE_MAC "bb:bb:bb:bb:bb:%02hx"

#define LOCAL_IPV4 "172.20.%d.170"
#define REMOTE_IPV4 "172.20.%d.187"

#define LOCAL_IPV6 "fe80::%02hxaa"
#define REMOTE_IPV6 "fe80::%02hxbb"

static void initialize_tun(uint64_t pid)
{
  if (pid >= MAX_PIDS)
    fail("tun: no more than %d executors", MAX_PIDS);
  int id = pid;

  tunfd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
  if (tunfd == -1)
    fail("tun: can't open /dev/net/tun");

  char iface[IFNAMSIZ];
  snprintf_check(iface, sizeof(iface), "syz%d", id);

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, iface, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
    fail("tun: ioctl(TUNSETIFF) failed");

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

  execute_command("sysctl -w net.ipv6.conf.%s.accept_dad=0", iface);

  execute_command("sysctl -w net.ipv6.conf.%s.router_solicitations=0",
                  iface);

  execute_command("ip link set dev %s address %s", iface, local_mac);
  execute_command("ip addr add %s/24 dev %s", local_ipv4, iface);
  execute_command("ip -6 addr add %s/120 dev %s", local_ipv6, iface);
  execute_command("ip neigh add %s lladdr %s dev %s nud permanent",
                  remote_ipv4, remote_mac, iface);
  execute_command("ip -6 neigh add %s lladdr %s dev %s nud permanent",
                  remote_ipv6, remote_mac, iface);
  execute_command("ip link set dev %s up", iface);
}

static void setup_tun(uint64_t pid, bool enable_tun)
{
  if (enable_tun)
    initialize_tun(pid);
}

static int read_tun(char* data, int size)
{
  int rv = read(tunfd, data, size);
  if (rv < 0) {
    if (errno == EAGAIN)
      return -1;
    fail("tun: read failed with %d, errno: %d", rv, errno);
  }
  return rv;
}

struct csum_inet {
  uint32_t acc;
};

static void csum_inet_init(struct csum_inet* csum)
{
  csum->acc = 0;
}

static void csum_inet_update(struct csum_inet* csum,
                             const uint8_t* data, size_t length)
{
  if (length == 0)
    return;

  size_t i;
  for (i = 0; i < length - 1; i += 2)
    csum->acc += *(uint16_t*)&data[i];

  if (length & 1)
    csum->acc += (uint16_t)data[length - 1];

  while (csum->acc > 0xffff)
    csum->acc = (csum->acc & 0xffff) + (csum->acc >> 16);
}

static uint16_t csum_inet_digest(struct csum_inet* csum)
{
  return ~csum->acc;
}

static uintptr_t syz_emit_ethernet(uintptr_t a0, uintptr_t a1)
{

  if (tunfd < 0)
    return (uintptr_t)-1;

  int64_t length = a0;
  char* data = (char*)a1;
  return write(tunfd, data, length);
}

struct ipv6hdr {
  __u8 priority : 4, version : 4;
  __u8 flow_lbl[3];

  __be16 payload_len;
  __u8 nexthdr;
  __u8 hop_limit;

  struct in6_addr saddr;
  struct in6_addr daddr;
};

struct tcp_resources {
  int32_t seq;
  int32_t ack;
};

static uintptr_t syz_extract_tcp_res(uintptr_t a0, uintptr_t a1,
                                     uintptr_t a2)
{

  if (tunfd < 0)
    return (uintptr_t)-1;

  char data[SYZ_TUN_MAX_PACKET_SIZE];
  int rv = read_tun(&data[0], sizeof(data));
  if (rv == -1)
    return (uintptr_t)-1;
  size_t length = rv;

  struct tcphdr* tcphdr;

  if (length < sizeof(struct ethhdr))
    return (uintptr_t)-1;
  struct ethhdr* ethhdr = (struct ethhdr*)&data[0];

  if (ethhdr->h_proto == htons(ETH_P_IP)) {
    if (length < sizeof(struct ethhdr) + sizeof(struct iphdr))
      return (uintptr_t)-1;
    struct iphdr* iphdr = (struct iphdr*)&data[sizeof(struct ethhdr)];
    if (iphdr->protocol != IPPROTO_TCP)
      return (uintptr_t)-1;
    if (length <
        sizeof(struct ethhdr) + iphdr->ihl * 4 + sizeof(struct tcphdr))
      return (uintptr_t)-1;
    tcphdr =
        (struct tcphdr*)&data[sizeof(struct ethhdr) + iphdr->ihl * 4];
  } else {
    if (length < sizeof(struct ethhdr) + sizeof(struct ipv6hdr))
      return (uintptr_t)-1;
    struct ipv6hdr* ipv6hdr =
        (struct ipv6hdr*)&data[sizeof(struct ethhdr)];
    if (ipv6hdr->nexthdr != IPPROTO_TCP)
      return (uintptr_t)-1;
    if (length < sizeof(struct ethhdr) + sizeof(struct ipv6hdr) +
                     sizeof(struct tcphdr))
      return (uintptr_t)-1;
    tcphdr = (struct tcphdr*)&data[sizeof(struct ethhdr) +
                                   sizeof(struct ipv6hdr)];
  }

  struct tcp_resources* res = (struct tcp_resources*)a0;
  NONFAILING(res->seq = htonl((ntohl(tcphdr->seq) + (uint32_t)a1)));
  NONFAILING(res->ack = htonl((ntohl(tcphdr->ack_seq) + (uint32_t)a2)));

  return 0;
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

static int do_sandbox_none(int executor_pid, bool enable_tun)
{
  int pid = fork();
  if (pid)
    return pid;

  sandbox_common();
  setup_tun(executor_pid, enable_tun);

  loop();
  doexit(1);
}

static void test();

void loop()
{
  while (1) {
    test();
  }
}

long r[185];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0x1f000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    NONFAILING(*(uint32_t*)0x20014f60 = (uint32_t)0x0);
    NONFAILING(*(uint16_t*)0x20014f64 = (uint16_t)0x2);
    NONFAILING(*(uint16_t*)0x20014f66 = (uint16_t)0x204e);
    NONFAILING(*(uint8_t*)0x20014f68 = (uint8_t)0xac);
    NONFAILING(*(uint8_t*)0x20014f69 = (uint8_t)0x14);
    NONFAILING(*(uint8_t*)0x20014f6a = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20014f6b = (uint8_t)0xaa);
    NONFAILING(*(uint8_t*)0x20014f6c = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20014f6d = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20014f6e = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20014f6f = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20014f70 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20014f71 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20014f72 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20014f73 = (uint8_t)0x0);
    NONFAILING(*(uint64_t*)0x20014f74 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20014f7c = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20014f84 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20014f8c = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20014f94 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20014f9c = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20014fa4 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20014fac = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20014fb4 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20014fbc = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20014fc4 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20014fcc = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20014fd4 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20014fdc = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20014fe4 = (uint64_t)0x0);
    NONFAILING(*(uint32_t*)0x20014fec = (uint32_t)0x5);
    NONFAILING(*(uint32_t*)0x20014ff0 = (uint32_t)0xffffffffffffffff);
    NONFAILING(*(uint32_t*)0x20014ff4 = (uint32_t)0x2);
    NONFAILING(*(uint32_t*)0x20014ff8 = (uint32_t)0x40);
    NONFAILING(*(uint32_t*)0x20014ffc = (uint32_t)0x20);
    NONFAILING(*(uint32_t*)0x2001affc = (uint32_t)0xa0);
    r[37] = syscall(__NR_getsockopt, 0xfffffffffffffffful, 0x84ul,
                    0xful, 0x20014f60ul, 0x2001affcul);
    break;
  case 2:
    r[38] = syscall(__NR_socket, 0x2ul, 0x1ul, 0x0ul);
    break;
  case 3:
    r[39] =
        syz_extract_tcp_res(0x20006ff8ul, 0x8ul, 0x4000000000003fdul);
    if (r[39] != -1)
      NONFAILING(r[40] = *(uint32_t*)0x20006ffc);
    break;
  case 4:
    NONFAILING(*(uint16_t*)0x20001000 = (uint16_t)0x2);
    NONFAILING(*(uint16_t*)0x20001002 = (uint16_t)0x204e);
    NONFAILING(*(uint32_t*)0x20001004 = (uint32_t)0x0);
    NONFAILING(*(uint8_t*)0x20001008 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20001009 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x2000100a = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x2000100b = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x2000100c = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x2000100d = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x2000100e = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x2000100f = (uint8_t)0x0);
    r[52] = syscall(__NR_bind, r[38], 0x20001000ul, 0x10ul);
    break;
  case 5:
    r[53] = syscall(__NR_listen, r[38], 0x100000000ul);
    break;
  case 6:
    NONFAILING(*(uint8_t*)0x20002000 = (uint8_t)0xaa);
    NONFAILING(*(uint8_t*)0x20002001 = (uint8_t)0xaa);
    NONFAILING(*(uint8_t*)0x20002002 = (uint8_t)0xaa);
    NONFAILING(*(uint8_t*)0x20002003 = (uint8_t)0xaa);
    NONFAILING(*(uint8_t*)0x20002004 = (uint8_t)0xaa);
    NONFAILING(*(uint8_t*)0x20002005 = (uint8_t)0x0);
    NONFAILING(
        memcpy((void*)0x20002006, "\x4c\x61\x12\xcc\x15\xd8", 6));
    NONFAILING(*(uint16_t*)0x2000200c = (uint16_t)0x8);
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2000200e, 0x5, 0, 4));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2000200e, 0x4, 4, 4));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2000200f, 0x0, 0, 2));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2000200f, 0x0, 2, 6));
    NONFAILING(*(uint16_t*)0x20002010 = (uint16_t)0x2800);
    NONFAILING(*(uint16_t*)0x20002012 = (uint16_t)0x6400);
    NONFAILING(*(uint16_t*)0x20002014 = (uint16_t)0x0);
    NONFAILING(*(uint8_t*)0x20002016 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20002017 = (uint8_t)0x6);
    NONFAILING(*(uint16_t*)0x20002018 = (uint16_t)0x0);
    NONFAILING(*(uint8_t*)0x2000201a = (uint8_t)0xac);
    NONFAILING(*(uint8_t*)0x2000201b = (uint8_t)0x14);
    NONFAILING(*(uint8_t*)0x2000201c = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x2000201d = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2000201e = (uint8_t)0xac);
    NONFAILING(*(uint8_t*)0x2000201f = (uint8_t)0x14);
    NONFAILING(*(uint8_t*)0x20002020 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20002021 = (uint8_t)0xaa);
    NONFAILING(*(uint16_t*)0x20002022 = (uint16_t)0x214e);
    NONFAILING(*(uint16_t*)0x20002024 = (uint16_t)0x204e);
    NONFAILING(*(uint32_t*)0x20002026 = r[40]);
    NONFAILING(*(uint32_t*)0x2000202a = (uint32_t)0x42424242);
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2000202e, 0x0, 0, 1));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2000202e, 0x0, 1, 3));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2000202e, 0x5, 4, 4));
    NONFAILING(*(uint8_t*)0x2000202f = (uint8_t)0x2);
    NONFAILING(*(uint16_t*)0x20002030 = (uint16_t)0x0);
    NONFAILING(*(uint16_t*)0x20002032 = (uint16_t)0x0);
    NONFAILING(*(uint16_t*)0x20002034 = (uint16_t)0x0);
    struct csum_inet csum_91;
    csum_inet_init(&csum_91);
    NONFAILING(
        csum_inet_update(&csum_91, (const uint8_t*)0x2000201a, 4));
    NONFAILING(
        csum_inet_update(&csum_91, (const uint8_t*)0x2000201e, 4));
    uint16_t csum_91_chunk_2 = 0x600;
    csum_inet_update(&csum_91, (const uint8_t*)&csum_91_chunk_2, 2);
    uint16_t csum_91_chunk_3 = 0x1400;
    csum_inet_update(&csum_91, (const uint8_t*)&csum_91_chunk_3, 2);
    NONFAILING(
        csum_inet_update(&csum_91, (const uint8_t*)0x20002022, 20));
    NONFAILING(*(uint16_t*)0x20002032 = csum_inet_digest(&csum_91));
    struct csum_inet csum_92;
    csum_inet_init(&csum_92);
    NONFAILING(
        csum_inet_update(&csum_92, (const uint8_t*)0x2000200e, 20));
    NONFAILING(*(uint16_t*)0x20002018 = csum_inet_digest(&csum_92));
    r[93] = syz_emit_ethernet(0x36ul, 0x20002000ul);
    break;
  case 7:
    r[94] = syz_extract_tcp_res(0x20013000ul, 0x2ul, 0x1000ul);
    break;
  case 8:
    r[95] = syscall(__NR_socket, 0x2ul, 0x1ul, 0x0ul);
    break;
  case 9:
    NONFAILING(*(uint16_t*)0x20016ff0 = (uint16_t)0x2);
    NONFAILING(*(uint16_t*)0x20016ff2 = (uint16_t)0x214e);
    NONFAILING(*(uint32_t*)0x20016ff4 = (uint32_t)0x20000e0);
    NONFAILING(*(uint8_t*)0x20016ff8 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20016ff9 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20016ffa = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20016ffb = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20016ffc = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20016ffd = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20016ffe = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20016fff = (uint8_t)0x0);
    r[107] = syscall(__NR_bind, r[95], 0x20016ff0ul, 0x10ul);
    break;
  case 10:
    NONFAILING(*(uint32_t*)0x20015f70 = (uint32_t)0x7f);
    NONFAILING(*(uint16_t*)0x20015f78 = (uint16_t)0x2);
    NONFAILING(*(uint16_t*)0x20015f7a = (uint16_t)0x234e);
    NONFAILING(*(uint8_t*)0x20015f7c = (uint8_t)0xac);
    NONFAILING(*(uint8_t*)0x20015f7d = (uint8_t)0x14);
    NONFAILING(*(uint8_t*)0x20015f7e = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20015f7f = (uint8_t)0xaa);
    NONFAILING(*(uint8_t*)0x20015f80 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20015f81 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20015f82 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20015f83 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20015f84 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20015f85 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20015f86 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20015f87 = (uint8_t)0x0);
    NONFAILING(*(uint64_t*)0x20015f88 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20015f90 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20015f98 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20015fa0 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20015fa8 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20015fb0 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20015fb8 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20015fc0 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20015fc8 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20015fd0 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20015fd8 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20015fe0 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20015fe8 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20015ff0 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20015ff8 = (uint64_t)0x0);
    r[138] = syscall(__NR_setsockopt, r[38], 0x0ul, 0x2aul,
                     0x20015f70ul, 0x90ul);
    break;
  case 11:
    NONFAILING(*(uint32_t*)0x2001c000 = (uint32_t)0x0);
    r[140] = syscall(__NR_getsockopt, r[95], 0x100000005ul, 0x40009ul,
                     0x2001b000ul, 0x2001c000ul);
    break;
  case 12:
    NONFAILING(*(uint32_t*)0x20004ffc = (uint32_t)0x10);
    r[142] =
        syscall(__NR_getsockname, r[95], 0x2001aff0ul, 0x20004ffcul);
    break;
  case 13:
    NONFAILING(*(uint8_t*)0x2001df3f = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2001df40 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2001df41 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2001df42 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2001df43 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2001df44 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x2001df45 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2001df46 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2001df47 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2001df48 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2001df49 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2001df4a = (uint8_t)0x0);
    NONFAILING(*(uint16_t*)0x2001df4b = (uint16_t)0x608);
    NONFAILING(*(uint8_t*)0x2001df4d = (uint8_t)0x3);
    NONFAILING(*(uint8_t*)0x2001df4e = (uint8_t)0x1);
    NONFAILING(*(uint8_t*)0x2001df4f = (uint8_t)0x1f);
    r[159] = syz_emit_ethernet(0x11ul, 0x2001df3ful);
    break;
  case 14:
    NONFAILING(*(uint8_t*)0x2000cedd = (uint8_t)0xaa);
    NONFAILING(*(uint8_t*)0x2000cede = (uint8_t)0xaa);
    NONFAILING(*(uint8_t*)0x2000cedf = (uint8_t)0xaa);
    NONFAILING(*(uint8_t*)0x2000cee0 = (uint8_t)0xaa);
    NONFAILING(*(uint8_t*)0x2000cee1 = (uint8_t)0xaa);
    NONFAILING(*(uint8_t*)0x2000cee2 = (uint8_t)0x0);
    NONFAILING(
        memcpy((void*)0x2000cee3, "\x4c\x61\x12\xcc\x15\xd8", 6));
    NONFAILING(*(uint16_t*)0x2000cee9 = (uint16_t)0x8);
    NONFAILING(*(uint16_t*)0x2000ceeb = (uint16_t)0xffff);
    NONFAILING(*(uint16_t*)0x2000ceed = (uint16_t)0x1501);
    NONFAILING(*(uint8_t*)0x2000ceef = (uint8_t)0x5);
    NONFAILING(*(uint8_t*)0x2000cef0 = (uint8_t)0x1);
    NONFAILING(*(uint32_t*)0x2000cef1 = (uint32_t)0xffffffff);
    NONFAILING(*(uint8_t*)0x2000cef5 = (uint8_t)0xff);
    NONFAILING(*(uint8_t*)0x2000cef6 = (uint8_t)0xff);
    NONFAILING(*(uint8_t*)0x2000cef7 = (uint8_t)0xff);
    NONFAILING(*(uint8_t*)0x2000cef8 = (uint8_t)0xff);
    NONFAILING(*(uint8_t*)0x2000cef9 = (uint8_t)0xff);
    NONFAILING(*(uint8_t*)0x2000cefa = (uint8_t)0xff);
    NONFAILING(*(uint16_t*)0x2000cefb = (uint16_t)0x1401);
    NONFAILING(*(uint32_t*)0x2000cefd = (uint32_t)0xffffffff);
    NONFAILING(
        memcpy((void*)0x2000cf01, "\x53\x06\x62\x58\xf9\x35", 6));
    NONFAILING(*(uint16_t*)0x2000cf07 = (uint16_t)0xff01);
    NONFAILING(memcpy(
        (void*)0x2000cf09,
        "\xbc\x1c\x4f\xe9\x0a\xf6\x37\xaa\x53\xdd\x7a\xef\x82\x88\x63"
        "\x52\xc4\x4a\x60\x5f\xa6\x44\xf9\x14\x6d\x60\x9c\x1a\xbe\xf8"
        "\x61\x8e\xeb\x24\x25\x4b\xd0\x78\xc0\xfc\xbe\x9e\x77\x6c\x1a"
        "\x15\x78\x6e\x66\x40\xde\x42\x54\xc7\x7c\x63\xf4\xf5\xb1\xa2"
        "\xcf\xb1\xbf\xe3\x8b\xbf\x05\xa7\x2b\x56\x3e\x6f\x74\x5a\x91"
        "\x0e\x2e\x08\xce\xc3\x2d\x99\x06\x27\xdb\x96\x6c\x3e\x21\xba"
        "\x38\xf3\xc4\x66\x8a\xf5\xb5\x16\x79\xc9\x20\x9d\x53\x87\xcd"
        "\xab\x9b\xef\x63\x32\x95\xbc\x91\xc0\xb8\x87\xad\xd4\x92\xb1"
        "\x54\x25\x29\xcb\x2a\x09\x68\xc9\xe0\x48\x80\xf0\xc5\x9e\xd3"
        "\xeb\x80\xd0\x4e\x00\x31\xdc\xf9\x6d\x8b\x42\xa0\xe2\x29\x6a"
        "\x97\xbe\x32\xee\x51\x93\xbc\x16\xc7\x85\x9d\x47\xc5\xff\xea"
        "\xfc\x2e\xf3\x63\x9c\x34\x03\x45\x9e\x3c\xea\x12\xe7\x82\xe5"
        "\x92\x24\x19\x93\x0b\xe9\x00\xb2\x36\xf7\xbd\x41\x60\xc4\x38"
        "\xef\x2a\xcf\xb4\xc2\x66\x0d\x3c\x74\x83\x9a\x32\x1b\x9a\x42"
        "\xd6\xaf\x32\x92\x7b\xbd\x33\x32\xc9\xb3\xfa\x76\x24\xcc\x0d"
        "\x13\x70\x92\x53\x84\xc7\xa9\xca\xc4\xc6\xef\xf5\x6e\xf7\x78"
        "\x51\x82\x27\xc7\x1c\x62\x66",
        247));
    r[184] = syz_emit_ethernet(0x123ul, 0x2000ceddul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[30];

  memset(r, -1, sizeof(r));
  srand(getpid());
  for (i = 0; i < 15; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  for (i = 0; i < 15; i++) {
    pthread_create(&th[15 + i], 0, thr, (void*)i);
    if (rand() % 2)
      usleep(rand() % 10000);
  }
  usleep(rand() % 100000);
}

int main()
{
  install_segv_handler();
  use_temporary_dir();
  int pid = do_sandbox_none(0, true);
  int status = 0;
  while (waitpid(pid, &status, __WALL) != pid) {
  }
  return 0;
}
