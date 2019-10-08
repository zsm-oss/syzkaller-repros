// general protection fault in ipt_do_table
// https://syzkaller.appspot.com/bug?id=61714127fcffd45b73cbd408ae695751952b87aa
// status:dup
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
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/uio.h>
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

static int read_tun(char* data, int size)
{
  if (tunfd < 0)
    return -1;

  int rv = read(tunfd, data, size);
  if (rv < 0) {
    if (errno == EAGAIN)
      return -1;
    if (errno == EBADFD)
      return -1;
    fail("tun: read failed with %d", rv);
  }
  return rv;
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
  uint32_t seq;
  uint32_t ack;
};

static uintptr_t syz_extract_tcp_res(uintptr_t a0, uintptr_t a1, uintptr_t a2)
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
    if (length < sizeof(struct ethhdr) + iphdr->ihl * 4 + sizeof(struct tcphdr))
      return (uintptr_t)-1;
    tcphdr = (struct tcphdr*)&data[sizeof(struct ethhdr) + iphdr->ihl * 4];
  } else {
    if (length < sizeof(struct ethhdr) + sizeof(struct ipv6hdr))
      return (uintptr_t)-1;
    struct ipv6hdr* ipv6hdr = (struct ipv6hdr*)&data[sizeof(struct ethhdr)];
    if (ipv6hdr->nexthdr != IPPROTO_TCP)
      return (uintptr_t)-1;
    if (length <
        sizeof(struct ethhdr) + sizeof(struct ipv6hdr) + sizeof(struct tcphdr))
      return (uintptr_t)-1;
    tcphdr =
        (struct tcphdr*)&data[sizeof(struct ethhdr) + sizeof(struct ipv6hdr)];
  }

  struct tcp_resources* res = (struct tcp_resources*)a0;
  NONFAILING(res->seq = htonl((ntohl(tcphdr->seq) + (uint32_t)a1)));
  NONFAILING(res->ack = htonl((ntohl(tcphdr->ack_seq) + (uint32_t)a2)));

  return 0;
}

long r[2];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  syz_extract_tcp_res(0x204bcff8, 1, 0);
  r[0] = syscall(__NR_socket, 2, 2, 0);
  r[1] = syscall(__NR_socket, 0xa, 2, 0);
  NONFAILING(memcpy((void*)0x20015cf0, "\x66\x69\x6c\x74\x65\x72\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00",
                    32));
  NONFAILING(*(uint32_t*)0x20015d10 = 0xe);
  NONFAILING(*(uint32_t*)0x20015d14 = 4);
  NONFAILING(*(uint32_t*)0x20015d18 = 0x2b0);
  NONFAILING(*(uint32_t*)0x20015d1c = -1);
  NONFAILING(*(uint32_t*)0x20015d20 = 0);
  NONFAILING(*(uint32_t*)0x20015d24 = 0xe8);
  NONFAILING(*(uint32_t*)0x20015d28 = 0x180);
  NONFAILING(*(uint32_t*)0x20015d2c = -1);
  NONFAILING(*(uint32_t*)0x20015d30 = -1);
  NONFAILING(*(uint32_t*)0x20015d34 = 0x218);
  NONFAILING(*(uint32_t*)0x20015d38 = 0x218);
  NONFAILING(*(uint32_t*)0x20015d3c = 0x218);
  NONFAILING(*(uint32_t*)0x20015d40 = -1);
  NONFAILING(*(uint32_t*)0x20015d44 = 4);
  NONFAILING(*(uint64_t*)0x20015d48 = 0x20006fc0);
  NONFAILING(*(uint8_t*)0x20015d50 = 0);
  NONFAILING(*(uint8_t*)0x20015d51 = 0);
  NONFAILING(*(uint8_t*)0x20015d52 = 0);
  NONFAILING(*(uint8_t*)0x20015d53 = 0);
  NONFAILING(*(uint8_t*)0x20015d54 = 0);
  NONFAILING(*(uint8_t*)0x20015d55 = 0);
  NONFAILING(*(uint8_t*)0x20015d56 = 0);
  NONFAILING(*(uint8_t*)0x20015d57 = 0);
  NONFAILING(*(uint8_t*)0x20015d58 = 0);
  NONFAILING(*(uint8_t*)0x20015d59 = 0);
  NONFAILING(*(uint8_t*)0x20015d5a = 0);
  NONFAILING(*(uint8_t*)0x20015d5b = 0);
  NONFAILING(*(uint8_t*)0x20015d5c = 0);
  NONFAILING(*(uint8_t*)0x20015d5d = 0);
  NONFAILING(*(uint8_t*)0x20015d5e = 0);
  NONFAILING(*(uint8_t*)0x20015d5f = 0);
  NONFAILING(*(uint8_t*)0x20015d60 = 0);
  NONFAILING(*(uint8_t*)0x20015d61 = 0);
  NONFAILING(*(uint8_t*)0x20015d62 = 0);
  NONFAILING(*(uint8_t*)0x20015d63 = 0);
  NONFAILING(*(uint8_t*)0x20015d64 = 0);
  NONFAILING(*(uint8_t*)0x20015d65 = 0);
  NONFAILING(*(uint8_t*)0x20015d66 = 0);
  NONFAILING(*(uint8_t*)0x20015d67 = 0);
  NONFAILING(*(uint8_t*)0x20015d68 = 0);
  NONFAILING(*(uint8_t*)0x20015d69 = 0);
  NONFAILING(*(uint8_t*)0x20015d6a = 0);
  NONFAILING(*(uint8_t*)0x20015d6b = 0);
  NONFAILING(*(uint8_t*)0x20015d6c = 0);
  NONFAILING(*(uint8_t*)0x20015d6d = 0);
  NONFAILING(*(uint8_t*)0x20015d6e = 0);
  NONFAILING(*(uint8_t*)0x20015d6f = 0);
  NONFAILING(*(uint8_t*)0x20015d70 = 0);
  NONFAILING(*(uint8_t*)0x20015d71 = 0);
  NONFAILING(*(uint8_t*)0x20015d72 = 0);
  NONFAILING(*(uint8_t*)0x20015d73 = 0);
  NONFAILING(*(uint8_t*)0x20015d74 = 0);
  NONFAILING(*(uint8_t*)0x20015d75 = 0);
  NONFAILING(*(uint8_t*)0x20015d76 = 0);
  NONFAILING(*(uint8_t*)0x20015d77 = 0);
  NONFAILING(*(uint8_t*)0x20015d78 = 0);
  NONFAILING(*(uint8_t*)0x20015d79 = 0);
  NONFAILING(*(uint8_t*)0x20015d7a = 0);
  NONFAILING(*(uint8_t*)0x20015d7b = 0);
  NONFAILING(*(uint8_t*)0x20015d7c = 0);
  NONFAILING(*(uint8_t*)0x20015d7d = 0);
  NONFAILING(*(uint8_t*)0x20015d7e = 0);
  NONFAILING(*(uint8_t*)0x20015d7f = 0);
  NONFAILING(*(uint8_t*)0x20015d80 = 0);
  NONFAILING(*(uint8_t*)0x20015d81 = 0);
  NONFAILING(*(uint8_t*)0x20015d82 = 0);
  NONFAILING(*(uint8_t*)0x20015d83 = 0);
  NONFAILING(*(uint8_t*)0x20015d84 = 0);
  NONFAILING(*(uint8_t*)0x20015d85 = 0);
  NONFAILING(*(uint8_t*)0x20015d86 = 0);
  NONFAILING(*(uint8_t*)0x20015d87 = 0);
  NONFAILING(*(uint8_t*)0x20015d88 = 0);
  NONFAILING(*(uint8_t*)0x20015d89 = 0);
  NONFAILING(*(uint8_t*)0x20015d8a = 0);
  NONFAILING(*(uint8_t*)0x20015d8b = 0);
  NONFAILING(*(uint8_t*)0x20015d8c = 0);
  NONFAILING(*(uint8_t*)0x20015d8d = 0);
  NONFAILING(*(uint8_t*)0x20015d8e = 0);
  NONFAILING(*(uint8_t*)0x20015d8f = 0);
  NONFAILING(*(uint8_t*)0x20015d90 = 0);
  NONFAILING(*(uint8_t*)0x20015d91 = 0);
  NONFAILING(*(uint8_t*)0x20015d92 = 0);
  NONFAILING(*(uint8_t*)0x20015d93 = 0);
  NONFAILING(*(uint8_t*)0x20015d94 = 0);
  NONFAILING(*(uint8_t*)0x20015d95 = 0);
  NONFAILING(*(uint8_t*)0x20015d96 = 0);
  NONFAILING(*(uint8_t*)0x20015d97 = 0);
  NONFAILING(*(uint8_t*)0x20015d98 = 0);
  NONFAILING(*(uint8_t*)0x20015d99 = 0);
  NONFAILING(*(uint8_t*)0x20015d9a = 0);
  NONFAILING(*(uint8_t*)0x20015d9b = 0);
  NONFAILING(*(uint8_t*)0x20015d9c = 0);
  NONFAILING(*(uint8_t*)0x20015d9d = 0);
  NONFAILING(*(uint8_t*)0x20015d9e = 0);
  NONFAILING(*(uint8_t*)0x20015d9f = 0);
  NONFAILING(*(uint8_t*)0x20015da0 = 0);
  NONFAILING(*(uint8_t*)0x20015da1 = 0);
  NONFAILING(*(uint8_t*)0x20015da2 = 0);
  NONFAILING(*(uint8_t*)0x20015da3 = 0);
  NONFAILING(*(uint32_t*)0x20015da4 = 0);
  NONFAILING(*(uint16_t*)0x20015da8 = 0xc0);
  NONFAILING(*(uint16_t*)0x20015daa = 0xe8);
  NONFAILING(*(uint32_t*)0x20015dac = 0);
  NONFAILING(*(uint64_t*)0x20015db0 = 0);
  NONFAILING(*(uint64_t*)0x20015db8 = 0);
  NONFAILING(*(uint16_t*)0x20015dc0 = 0x28);
  NONFAILING(memcpy((void*)0x20015dc2, "\x74\x74\x6c\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00",
                    29));
  NONFAILING(*(uint8_t*)0x20015ddf = 0);
  NONFAILING(*(uint8_t*)0x20015de0 = 3);
  NONFAILING(*(uint8_t*)0x20015de1 = 0);
  NONFAILING(*(uint16_t*)0x20015de8 = 0x28);
  NONFAILING(memcpy((void*)0x20015dea, "\x74\x74\x6c\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00",
                    29));
  NONFAILING(*(uint8_t*)0x20015e07 = 0);
  NONFAILING(*(uint8_t*)0x20015e08 = 2);
  NONFAILING(*(uint8_t*)0x20015e09 = 0xfa);
  NONFAILING(*(uint16_t*)0x20015e10 = 0x28);
  NONFAILING(memcpy((void*)0x20015e12, "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00",
                    29));
  NONFAILING(*(uint8_t*)0x20015e2f = 0);
  NONFAILING(*(uint32_t*)0x20015e30 = 0x180);
  NONFAILING(*(uint32_t*)0x20015e38 = htobe32(0x7f000001));
  NONFAILING(*(uint8_t*)0x20015e3c = 0xac);
  NONFAILING(*(uint8_t*)0x20015e3d = 0x14);
  NONFAILING(*(uint8_t*)0x20015e3e = 0);
  NONFAILING(*(uint8_t*)0x20015e3f = 0);
  NONFAILING(*(uint32_t*)0x20015e40 = htobe32(0));
  NONFAILING(*(uint32_t*)0x20015e44 = htobe32(0));
  NONFAILING(memcpy(
      (void*)0x20015e48,
      "\x5e\x01\x7b\x8a\x8f\xab\x9f\xce\x01\x61\xe3\xaf\x8f\x6f\xe3\xd6", 16));
  NONFAILING(memcpy(
      (void*)0x20015e58,
      "\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x30\x00\x00\x00\x00\x00\x00", 16));
  NONFAILING(*(uint8_t*)0x20015e68 = 4);
  NONFAILING(*(uint8_t*)0x20015e69 = 0);
  NONFAILING(*(uint8_t*)0x20015e6a = 0);
  NONFAILING(*(uint8_t*)0x20015e6b = 0);
  NONFAILING(*(uint8_t*)0x20015e6c = 0);
  NONFAILING(*(uint8_t*)0x20015e6d = 0);
  NONFAILING(*(uint8_t*)0x20015e6e = 0);
  NONFAILING(*(uint8_t*)0x20015e6f = 0);
  NONFAILING(*(uint8_t*)0x20015e70 = 0);
  NONFAILING(*(uint8_t*)0x20015e71 = 0);
  NONFAILING(*(uint8_t*)0x20015e72 = 0);
  NONFAILING(*(uint8_t*)0x20015e73 = 0);
  NONFAILING(*(uint8_t*)0x20015e74 = 0);
  NONFAILING(*(uint8_t*)0x20015e75 = 0);
  NONFAILING(*(uint8_t*)0x20015e76 = 0);
  NONFAILING(*(uint8_t*)0x20015e77 = 0);
  NONFAILING(*(uint8_t*)0x20015e78 = 0);
  NONFAILING(*(uint8_t*)0x20015e79 = 0);
  NONFAILING(*(uint8_t*)0x20015e7a = 0);
  NONFAILING(*(uint8_t*)0x20015e7b = 0);
  NONFAILING(*(uint8_t*)0x20015e7c = 0);
  NONFAILING(*(uint8_t*)0x20015e7d = 0);
  NONFAILING(*(uint8_t*)0x20015e7e = 0);
  NONFAILING(*(uint8_t*)0x20015e7f = 0);
  NONFAILING(*(uint8_t*)0x20015e80 = 0);
  NONFAILING(*(uint8_t*)0x20015e81 = 0);
  NONFAILING(*(uint8_t*)0x20015e82 = 0);
  NONFAILING(*(uint8_t*)0x20015e83 = 0);
  NONFAILING(*(uint8_t*)0x20015e84 = 0);
  NONFAILING(*(uint8_t*)0x20015e85 = 0);
  NONFAILING(*(uint8_t*)0x20015e86 = 0);
  NONFAILING(*(uint8_t*)0x20015e87 = 0);
  NONFAILING(*(uint16_t*)0x20015e88 = 0);
  NONFAILING(*(uint8_t*)0x20015e8a = 0);
  NONFAILING(*(uint8_t*)0x20015e8b = 0);
  NONFAILING(*(uint32_t*)0x20015e8c = 0);
  NONFAILING(*(uint16_t*)0x20015e90 = 0x70);
  NONFAILING(*(uint16_t*)0x20015e92 = 0x98);
  NONFAILING(*(uint32_t*)0x20015e94 = 0);
  NONFAILING(*(uint64_t*)0x20015e98 = 0);
  NONFAILING(*(uint64_t*)0x20015ea0 = 0);
  NONFAILING(*(uint16_t*)0x20015ea8 = 0x28);
  NONFAILING(memcpy((void*)0x20015eaa, "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00",
                    29));
  NONFAILING(*(uint8_t*)0x20015ec7 = 0);
  NONFAILING(*(uint32_t*)0x20015ec8 = 0xfffffffb);
  NONFAILING(*(uint32_t*)0x20015ed0 = htobe32(-1));
  NONFAILING(*(uint8_t*)0x20015ed4 = 0xac);
  NONFAILING(*(uint8_t*)0x20015ed5 = 0x14);
  NONFAILING(*(uint8_t*)0x20015ed6 = 0);
  NONFAILING(*(uint8_t*)0x20015ed7 = 0xaa);
  NONFAILING(*(uint32_t*)0x20015ed8 = htobe32(0));
  NONFAILING(*(uint32_t*)0x20015edc = htobe32(0));
  NONFAILING(memcpy(
      (void*)0x20015ee0,
      "\x74\x65\x71\x6c\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16));
  NONFAILING(*(uint8_t*)0x20015ef0 = 0x73);
  NONFAILING(*(uint8_t*)0x20015ef1 = 0x79);
  NONFAILING(*(uint8_t*)0x20015ef2 = 0x7a);
  NONFAILING(*(uint8_t*)0x20015ef3 = 0);
  NONFAILING(*(uint8_t*)0x20015ef4 = 0);
  NONFAILING(*(uint8_t*)0x20015f00 = 0);
  NONFAILING(*(uint8_t*)0x20015f01 = 0);
  NONFAILING(*(uint8_t*)0x20015f02 = 0);
  NONFAILING(*(uint8_t*)0x20015f03 = 0);
  NONFAILING(*(uint8_t*)0x20015f04 = 0);
  NONFAILING(*(uint8_t*)0x20015f05 = 0);
  NONFAILING(*(uint8_t*)0x20015f06 = 0);
  NONFAILING(*(uint8_t*)0x20015f07 = 0);
  NONFAILING(*(uint8_t*)0x20015f08 = 0);
  NONFAILING(*(uint8_t*)0x20015f09 = 0);
  NONFAILING(*(uint8_t*)0x20015f0a = 0);
  NONFAILING(*(uint8_t*)0x20015f0b = 0);
  NONFAILING(*(uint8_t*)0x20015f0c = 0);
  NONFAILING(*(uint8_t*)0x20015f0d = 0);
  NONFAILING(*(uint8_t*)0x20015f0e = 0);
  NONFAILING(*(uint8_t*)0x20015f0f = 0);
  NONFAILING(*(uint8_t*)0x20015f10 = 0);
  NONFAILING(*(uint8_t*)0x20015f11 = 0);
  NONFAILING(*(uint8_t*)0x20015f12 = 0);
  NONFAILING(*(uint8_t*)0x20015f13 = 0);
  NONFAILING(*(uint8_t*)0x20015f14 = 0);
  NONFAILING(*(uint8_t*)0x20015f15 = 0);
  NONFAILING(*(uint8_t*)0x20015f16 = 0);
  NONFAILING(*(uint8_t*)0x20015f17 = 0);
  NONFAILING(*(uint8_t*)0x20015f18 = 0);
  NONFAILING(*(uint8_t*)0x20015f19 = 0);
  NONFAILING(*(uint8_t*)0x20015f1a = 0);
  NONFAILING(*(uint8_t*)0x20015f1b = 0);
  NONFAILING(*(uint8_t*)0x20015f1c = 0);
  NONFAILING(*(uint8_t*)0x20015f1d = 0);
  NONFAILING(*(uint8_t*)0x20015f1e = 0);
  NONFAILING(*(uint8_t*)0x20015f1f = 0);
  NONFAILING(*(uint16_t*)0x20015f20 = 0);
  NONFAILING(*(uint8_t*)0x20015f22 = 0);
  NONFAILING(*(uint8_t*)0x20015f23 = 0);
  NONFAILING(*(uint32_t*)0x20015f24 = 0);
  NONFAILING(*(uint16_t*)0x20015f28 = 0x70);
  NONFAILING(*(uint16_t*)0x20015f2a = 0x98);
  NONFAILING(*(uint32_t*)0x20015f2c = 0);
  NONFAILING(*(uint64_t*)0x20015f30 = 0);
  NONFAILING(*(uint64_t*)0x20015f38 = 0);
  NONFAILING(*(uint16_t*)0x20015f40 = 0x28);
  NONFAILING(memcpy((void*)0x20015f42, "\x52\x45\x4a\x45\x43\x54\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00",
                    29));
  NONFAILING(*(uint8_t*)0x20015f5f = 0);
  NONFAILING(*(uint32_t*)0x20015f60 = 0);
  NONFAILING(*(uint8_t*)0x20015f68 = 0);
  NONFAILING(*(uint8_t*)0x20015f69 = 0);
  NONFAILING(*(uint8_t*)0x20015f6a = 0);
  NONFAILING(*(uint8_t*)0x20015f6b = 0);
  NONFAILING(*(uint8_t*)0x20015f6c = 0);
  NONFAILING(*(uint8_t*)0x20015f6d = 0);
  NONFAILING(*(uint8_t*)0x20015f6e = 0);
  NONFAILING(*(uint8_t*)0x20015f6f = 0);
  NONFAILING(*(uint8_t*)0x20015f70 = 0);
  NONFAILING(*(uint8_t*)0x20015f71 = 0);
  NONFAILING(*(uint8_t*)0x20015f72 = 0);
  NONFAILING(*(uint8_t*)0x20015f73 = 0);
  NONFAILING(*(uint8_t*)0x20015f74 = 0);
  NONFAILING(*(uint8_t*)0x20015f75 = 0);
  NONFAILING(*(uint8_t*)0x20015f76 = 0);
  NONFAILING(*(uint8_t*)0x20015f77 = 0);
  NONFAILING(*(uint8_t*)0x20015f78 = 0);
  NONFAILING(*(uint8_t*)0x20015f79 = 0);
  NONFAILING(*(uint8_t*)0x20015f7a = 0);
  NONFAILING(*(uint8_t*)0x20015f7b = 0);
  NONFAILING(*(uint8_t*)0x20015f7c = 0);
  NONFAILING(*(uint8_t*)0x20015f7d = 0);
  NONFAILING(*(uint8_t*)0x20015f7e = 0);
  NONFAILING(*(uint8_t*)0x20015f7f = 0);
  NONFAILING(*(uint8_t*)0x20015f80 = 0);
  NONFAILING(*(uint8_t*)0x20015f81 = 0);
  NONFAILING(*(uint8_t*)0x20015f82 = 0);
  NONFAILING(*(uint8_t*)0x20015f83 = 0);
  NONFAILING(*(uint8_t*)0x20015f84 = 0);
  NONFAILING(*(uint8_t*)0x20015f85 = 0);
  NONFAILING(*(uint8_t*)0x20015f86 = 0);
  NONFAILING(*(uint8_t*)0x20015f87 = 0);
  NONFAILING(*(uint8_t*)0x20015f88 = 0);
  NONFAILING(*(uint8_t*)0x20015f89 = 0);
  NONFAILING(*(uint8_t*)0x20015f8a = 0);
  NONFAILING(*(uint8_t*)0x20015f8b = 0);
  NONFAILING(*(uint8_t*)0x20015f8c = 0);
  NONFAILING(*(uint8_t*)0x20015f8d = 0);
  NONFAILING(*(uint8_t*)0x20015f8e = 0);
  NONFAILING(*(uint8_t*)0x20015f8f = 0);
  NONFAILING(*(uint8_t*)0x20015f90 = 0);
  NONFAILING(*(uint8_t*)0x20015f91 = 0);
  NONFAILING(*(uint8_t*)0x20015f92 = 0);
  NONFAILING(*(uint8_t*)0x20015f93 = 0);
  NONFAILING(*(uint8_t*)0x20015f94 = 0);
  NONFAILING(*(uint8_t*)0x20015f95 = 0);
  NONFAILING(*(uint8_t*)0x20015f96 = 0);
  NONFAILING(*(uint8_t*)0x20015f97 = 0);
  NONFAILING(*(uint8_t*)0x20015f98 = 0);
  NONFAILING(*(uint8_t*)0x20015f99 = 0);
  NONFAILING(*(uint8_t*)0x20015f9a = 0);
  NONFAILING(*(uint8_t*)0x20015f9b = 0);
  NONFAILING(*(uint8_t*)0x20015f9c = 0);
  NONFAILING(*(uint8_t*)0x20015f9d = 0);
  NONFAILING(*(uint8_t*)0x20015f9e = 0);
  NONFAILING(*(uint8_t*)0x20015f9f = 0);
  NONFAILING(*(uint8_t*)0x20015fa0 = 0);
  NONFAILING(*(uint8_t*)0x20015fa1 = 0);
  NONFAILING(*(uint8_t*)0x20015fa2 = 0);
  NONFAILING(*(uint8_t*)0x20015fa3 = 0);
  NONFAILING(*(uint8_t*)0x20015fa4 = 0);
  NONFAILING(*(uint8_t*)0x20015fa5 = 0);
  NONFAILING(*(uint8_t*)0x20015fa6 = 0);
  NONFAILING(*(uint8_t*)0x20015fa7 = 0);
  NONFAILING(*(uint8_t*)0x20015fa8 = 0);
  NONFAILING(*(uint8_t*)0x20015fa9 = 0);
  NONFAILING(*(uint8_t*)0x20015faa = 0);
  NONFAILING(*(uint8_t*)0x20015fab = 0);
  NONFAILING(*(uint8_t*)0x20015fac = 0);
  NONFAILING(*(uint8_t*)0x20015fad = 0);
  NONFAILING(*(uint8_t*)0x20015fae = 0);
  NONFAILING(*(uint8_t*)0x20015faf = 0);
  NONFAILING(*(uint8_t*)0x20015fb0 = 0);
  NONFAILING(*(uint8_t*)0x20015fb1 = 0);
  NONFAILING(*(uint8_t*)0x20015fb2 = 0);
  NONFAILING(*(uint8_t*)0x20015fb3 = 0);
  NONFAILING(*(uint8_t*)0x20015fb4 = 0);
  NONFAILING(*(uint8_t*)0x20015fb5 = 0);
  NONFAILING(*(uint8_t*)0x20015fb6 = 0);
  NONFAILING(*(uint8_t*)0x20015fb7 = 0);
  NONFAILING(*(uint8_t*)0x20015fb8 = 0);
  NONFAILING(*(uint8_t*)0x20015fb9 = 0);
  NONFAILING(*(uint8_t*)0x20015fba = 0);
  NONFAILING(*(uint8_t*)0x20015fbb = 0);
  NONFAILING(*(uint32_t*)0x20015fbc = 0);
  NONFAILING(*(uint16_t*)0x20015fc0 = 0x70);
  NONFAILING(*(uint16_t*)0x20015fc2 = 0x98);
  NONFAILING(*(uint32_t*)0x20015fc4 = 0);
  NONFAILING(*(uint64_t*)0x20015fc8 = 0);
  NONFAILING(*(uint64_t*)0x20015fd0 = 0);
  NONFAILING(*(uint16_t*)0x20015fd8 = 0x28);
  NONFAILING(memcpy((void*)0x20015fda, "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                                       "\x00\x00",
                    29));
  NONFAILING(*(uint8_t*)0x20015ff7 = 0);
  NONFAILING(*(uint32_t*)0x20015ff8 = 0xfffffffe);
  syscall(__NR_setsockopt, r[0], 0, 0x40, 0x20015cf0, 0x310);
  NONFAILING(*(uint16_t*)0x2000d000 = 0xa);
  NONFAILING(*(uint16_t*)0x2000d002 = 0);
  NONFAILING(*(uint32_t*)0x2000d004 = 0);
  NONFAILING(*(uint8_t*)0x2000d008 = 0xfe);
  NONFAILING(*(uint8_t*)0x2000d009 = 0x80);
  NONFAILING(*(uint8_t*)0x2000d00a = 0);
  NONFAILING(*(uint8_t*)0x2000d00b = 0);
  NONFAILING(*(uint8_t*)0x2000d00c = 0);
  NONFAILING(*(uint8_t*)0x2000d00d = 0);
  NONFAILING(*(uint8_t*)0x2000d00e = 0);
  NONFAILING(*(uint8_t*)0x2000d00f = 0);
  NONFAILING(*(uint8_t*)0x2000d010 = 0);
  NONFAILING(*(uint8_t*)0x2000d011 = 0);
  NONFAILING(*(uint8_t*)0x2000d012 = 0);
  NONFAILING(*(uint8_t*)0x2000d013 = 0);
  NONFAILING(*(uint8_t*)0x2000d014 = 0);
  NONFAILING(*(uint8_t*)0x2000d015 = 0);
  NONFAILING(*(uint8_t*)0x2000d016 = 0);
  NONFAILING(*(uint8_t*)0x2000d017 = 0);
  NONFAILING(*(uint32_t*)0x2000d018 = 6);
  syscall(__NR_connect, r[1], 0x2000d000, 0x1c);
  NONFAILING(*(uint16_t*)0x2000afe4 = 0xa);
  NONFAILING(*(uint16_t*)0x2000afe6 = htobe16(0x4e23));
  NONFAILING(*(uint32_t*)0x2000afe8 = 0);
  NONFAILING(*(uint8_t*)0x2000afec = 0);
  NONFAILING(*(uint8_t*)0x2000afed = 0);
  NONFAILING(*(uint8_t*)0x2000afee = 0);
  NONFAILING(*(uint8_t*)0x2000afef = 0);
  NONFAILING(*(uint8_t*)0x2000aff0 = 0);
  NONFAILING(*(uint8_t*)0x2000aff1 = 0);
  NONFAILING(*(uint8_t*)0x2000aff2 = 0);
  NONFAILING(*(uint8_t*)0x2000aff3 = 0);
  NONFAILING(*(uint8_t*)0x2000aff4 = 0);
  NONFAILING(*(uint8_t*)0x2000aff5 = 0);
  NONFAILING(*(uint8_t*)0x2000aff6 = -1);
  NONFAILING(*(uint8_t*)0x2000aff7 = -1);
  NONFAILING(*(uint32_t*)0x2000aff8 = htobe32(0xe0000001));
  NONFAILING(*(uint32_t*)0x2000affc = 0);
  syscall(__NR_connect, r[1], 0x2000afe4, 0x1c);
  NONFAILING(*(uint64_t*)0x20000000 = 0x20000000);
  NONFAILING(*(uint32_t*)0x20000008 = 0);
  NONFAILING(*(uint64_t*)0x20000010 = 0x2000eff0);
  NONFAILING(*(uint64_t*)0x20000018 = 0);
  NONFAILING(*(uint64_t*)0x20000020 = 0x20011000);
  NONFAILING(*(uint64_t*)0x20000028 = 0);
  NONFAILING(*(uint32_t*)0x20000030 = 0);
  syscall(__NR_sendmsg, r[1], 0x20000000, 0);
}

int main()
{
  install_segv_handler();
  use_temporary_dir();
  setup_tun(0, true);
  loop();
  return 0;
}
