// KASAN: use-after-free Read in skb_release_data
// https://syzkaller.appspot.com/bug?id=585fae98d901d5b81438ddff3ee7f9510562f104
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

long r[124];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0x18000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    r[1] = syscall(__NR_socket, 0x2ul, 0x1ul, 0x0ul);
    break;
  case 2:
    r[2] = syscall(__NR_shutdown, r[1], 0x1ul);
    break;
  case 3:
    r[3] = syscall(__NR_socket, 0x2ul, 0x1ul, 0x0ul);
    break;
  case 4:
    NONFAILING(*(uint16_t*)0x20013ff0 = (uint16_t)0x2);
    NONFAILING(*(uint16_t*)0x20013ff2 = (uint16_t)0x204e);
    NONFAILING(*(uint32_t*)0x20013ff4 = (uint32_t)0x100007f);
    NONFAILING(*(uint8_t*)0x20013ff8 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20013ff9 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20013ffa = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20013ffb = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20013ffc = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20013ffd = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20013ffe = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20013fff = (uint8_t)0x0);
    r[15] =
        syscall(__NR_bind, 0xfffffffffffffffful, 0x20013ff0ul, 0x10ul);
    break;
  case 5:
    r[16] = syscall(__NR_listen, r[3], 0x0ul);
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
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2000200f, 0x400, 2, 6));
    NONFAILING(*(uint16_t*)0x20002010 = (uint16_t)0x2800);
    NONFAILING(*(uint16_t*)0x20002012 = (uint16_t)0x6400);
    NONFAILING(*(uint16_t*)0x20002014 = (uint16_t)0x900);
    NONFAILING(*(uint8_t*)0x20002016 = (uint8_t)0x2);
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
    NONFAILING(*(uint32_t*)0x20002026 = (uint32_t)0x42424242);
    NONFAILING(*(uint32_t*)0x2000202a = (uint32_t)0x42424242);
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2000202e, 0x0, 0, 1));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2000202e, 0x0, 1, 3));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2000202e, 0x5, 4, 4));
    NONFAILING(*(uint8_t*)0x2000202f = (uint8_t)0x2);
    NONFAILING(*(uint16_t*)0x20002030 = (uint16_t)0x0);
    NONFAILING(*(uint16_t*)0x20002032 = (uint16_t)0x0);
    NONFAILING(*(uint16_t*)0x20002034 = (uint16_t)0x0);
    struct csum_inet csum_54;
    csum_inet_init(&csum_54);
    NONFAILING(
        csum_inet_update(&csum_54, (const uint8_t*)0x2000201a, 4));
    NONFAILING(
        csum_inet_update(&csum_54, (const uint8_t*)0x2000201e, 4));
    uint16_t csum_54_chunk_2 = 0x600;
    csum_inet_update(&csum_54, (const uint8_t*)&csum_54_chunk_2, 2);
    uint16_t csum_54_chunk_3 = 0x1400;
    csum_inet_update(&csum_54, (const uint8_t*)&csum_54_chunk_3, 2);
    NONFAILING(
        csum_inet_update(&csum_54, (const uint8_t*)0x20002022, 20));
    NONFAILING(*(uint16_t*)0x20002032 = csum_inet_digest(&csum_54));
    struct csum_inet csum_55;
    csum_inet_init(&csum_55);
    NONFAILING(
        csum_inet_update(&csum_55, (const uint8_t*)0x2000200e, 20));
    NONFAILING(*(uint16_t*)0x20002018 = csum_inet_digest(&csum_55));
    r[56] = syz_emit_ethernet(0x36ul, 0x20002000ul);
    break;
  case 7:
    r[57] = syz_extract_tcp_res(0x20014000ul, 0x78aul, 0x0ul);
    break;
  case 8:
    r[58] = syscall(__NR_inotify_init1, 0x80800ul);
    break;
  case 9:
    NONFAILING(*(uint16_t*)0x2000cff0 = (uint16_t)0x2);
    NONFAILING(*(uint16_t*)0x2000cff2 = (uint16_t)0x234e);
    NONFAILING(*(uint32_t*)0x2000cff4 = (uint32_t)0x100007f);
    NONFAILING(*(uint8_t*)0x2000cff8 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x2000cff9 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x2000cffa = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x2000cffb = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x2000cffc = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x2000cffd = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x2000cffe = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x2000cfff = (uint8_t)0x0);
    r[70] =
        syscall(__NR_bind, 0xfffffffffffffffful, 0x2000cff0ul, 0x10ul);
    break;
  case 10:
    r[71] = syz_extract_tcp_res(0x20010ff8ul, 0xcul, 0x8ul);
    break;
  case 11:
    NONFAILING(*(uint8_t*)0x20002000 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x20002001 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x20002002 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x20002003 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x20002004 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x20002005 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x20002006 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x20002007 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x20002008 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x20002009 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2000200a = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2000200b = (uint8_t)0x0);
    NONFAILING(*(uint16_t*)0x2000200c = (uint16_t)0xc788);
    NONFAILING(*(uint8_t*)0x2000200e = (uint8_t)0x3);
    NONFAILING(*(uint8_t*)0x2000200f = (uint8_t)0x80000001);
    NONFAILING(*(uint8_t*)0x20002010 = (uint8_t)0xff);
    r[88] = syz_emit_ethernet(0x11ul, 0x20002000ul);
    break;
  case 12:
    NONFAILING(*(uint8_t*)0x2000df17 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2000df18 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2000df19 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2000df1a = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2000df1b = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2000df1c = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x2000df1d = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2000df1e = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2000df1f = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2000df20 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2000df21 = (uint8_t)0xbb);
    NONFAILING(*(uint8_t*)0x2000df22 = (uint8_t)0x0);
    NONFAILING(*(uint16_t*)0x2000df23 = (uint16_t)0x8);
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2000df25, 0x6, 0, 4));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2000df25, 0x4, 4, 4));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2000df26, 0x0, 0, 2));
    NONFAILING(STORE_BY_BITMASK(uint8_t, 0x2000df26, 0x0, 2, 6));
    NONFAILING(*(uint16_t*)0x2000df27 = (uint16_t)0x2000);
    NONFAILING(*(uint16_t*)0x2000df29 = (uint16_t)0x6400);
    NONFAILING(*(uint16_t*)0x2000df2b = (uint16_t)0x3100);
    NONFAILING(*(uint8_t*)0x2000df2d = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x2000df2e = (uint8_t)0x6);
    NONFAILING(*(uint16_t*)0x2000df2f = (uint16_t)0x0);
    NONFAILING(*(uint32_t*)0x2000df31 = (uint32_t)0x20000e0);
    NONFAILING(*(uint32_t*)0x2000df35 = (uint32_t)0x100007f);
    NONFAILING(*(uint8_t*)0x2000df39 = (uint8_t)0x0);
    NONFAILING(*(uint8_t*)0x2000df3a = (uint8_t)0x83);
    NONFAILING(*(uint8_t*)0x2000df3b = (uint8_t)0x2);
    NONFAILING(*(uint8_t*)0x2000df3d = (uint8_t)0x33);
    NONFAILING(*(uint8_t*)0x2000df3e = (uint8_t)0x9);
    NONFAILING(*(uint16_t*)0x2000df3f = (uint16_t)0x0);
    NONFAILING(*(uint32_t*)0x2000df41 = (uint32_t)0x10000e0);
    struct csum_inet csum_121;
    csum_inet_init(&csum_121);
    NONFAILING(
        csum_inet_update(&csum_121, (const uint8_t*)0x2000df3d, 8));
    NONFAILING(*(uint16_t*)0x2000df3f = csum_inet_digest(&csum_121));
    struct csum_inet csum_122;
    csum_inet_init(&csum_122);
    NONFAILING(
        csum_inet_update(&csum_122, (const uint8_t*)0x2000df25, 24));
    NONFAILING(*(uint16_t*)0x2000df2f = csum_inet_digest(&csum_122));
    r[123] = syz_emit_ethernet(0x2eul, 0x2000df17ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[26];

  memset(r, -1, sizeof(r));
  srand(getpid());
  for (i = 0; i < 13; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  for (i = 0; i < 13; i++) {
    pthread_create(&th[13 + i], 0, thr, (void*)i);
    if (rand() % 2)
      usleep(rand() % 10000);
  }
  usleep(rand() % 100000);
}

int main()
{
  int i;
  for (i = 0; i < 8; i++) {
    if (fork() == 0) {
      install_segv_handler();
      use_temporary_dir();
      int pid = do_sandbox_none(i, true);
      int status = 0;
      while (waitpid(pid, &status, __WALL) != pid) {
      }
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}
