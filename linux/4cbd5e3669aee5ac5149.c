// possible deadlock in xfrm_policy_lookup_bytype
// https://syzkaller.appspot.com/bug?id=4cbd5e3669aee5ac5149
// status:3
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
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
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <linux/capability.h>
#include <linux/if_addr.h>
#include <linux/if_ether.h>
#include <linux/if_link.h>
#include <linux/if_tun.h>
#include <linux/in6.h>
#include <linux/ip.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/tcp.h>
#include <linux/veth.h>

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  int skip = __atomic_load_n(&skip_segv, __ATOMIC_RELAXED) != 0;
  int valid = addr < prog_start || addr > prog_end;
  if (skip && valid) {
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

struct nlmsg {
  char* pos;
  int nesting;
  struct nlattr* nested[8];
  char buf[1024];
};

static struct nlmsg nlmsg;

static void netlink_init(struct nlmsg* nlmsg, int typ, int flags,
                         const void* data, int size)
{
  memset(nlmsg, 0, sizeof(*nlmsg));
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg->buf;
  hdr->nlmsg_type = typ;
  hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | flags;
  memcpy(hdr + 1, data, size);
  nlmsg->pos = (char*)(hdr + 1) + NLMSG_ALIGN(size);
}

static void netlink_attr(struct nlmsg* nlmsg, int typ, const void* data,
                         int size)
{
  struct nlattr* attr = (struct nlattr*)nlmsg->pos;
  attr->nla_len = sizeof(*attr) + size;
  attr->nla_type = typ;
  memcpy(attr + 1, data, size);
  nlmsg->pos += NLMSG_ALIGN(attr->nla_len);
}

static int netlink_send_ext(struct nlmsg* nlmsg, int sock, uint16_t reply_type,
                            int* reply_len)
{
  if (nlmsg->pos > nlmsg->buf + sizeof(nlmsg->buf) || nlmsg->nesting)
    exit(1);
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg->buf;
  hdr->nlmsg_len = nlmsg->pos - nlmsg->buf;
  struct sockaddr_nl addr;
  memset(&addr, 0, sizeof(addr));
  addr.nl_family = AF_NETLINK;
  unsigned n = sendto(sock, nlmsg->buf, hdr->nlmsg_len, 0,
                      (struct sockaddr*)&addr, sizeof(addr));
  if (n != hdr->nlmsg_len)
    exit(1);
  n = recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0);
  if (reply_len)
    *reply_len = 0;
  if (hdr->nlmsg_type == NLMSG_DONE)
    return 0;
  if (n < sizeof(struct nlmsghdr))
    exit(1);
  if (reply_len && hdr->nlmsg_type == reply_type) {
    *reply_len = n;
    return 0;
  }
  if (n < sizeof(struct nlmsghdr) + sizeof(struct nlmsgerr))
    exit(1);
  if (hdr->nlmsg_type != NLMSG_ERROR)
    exit(1);
  return -((struct nlmsgerr*)(hdr + 1))->error;
}

static int netlink_send(struct nlmsg* nlmsg, int sock)
{
  return netlink_send_ext(nlmsg, sock, 0, NULL);
}

static void netlink_device_change(struct nlmsg* nlmsg, int sock,
                                  const char* name, bool up, const char* master,
                                  const void* mac, int macsize,
                                  const char* new_name)
{
  struct ifinfomsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  if (up)
    hdr.ifi_flags = hdr.ifi_change = IFF_UP;
  hdr.ifi_index = if_nametoindex(name);
  netlink_init(nlmsg, RTM_NEWLINK, 0, &hdr, sizeof(hdr));
  if (new_name)
    netlink_attr(nlmsg, IFLA_IFNAME, new_name, strlen(new_name));
  if (master) {
    int ifindex = if_nametoindex(master);
    netlink_attr(nlmsg, IFLA_MASTER, &ifindex, sizeof(ifindex));
  }
  if (macsize)
    netlink_attr(nlmsg, IFLA_ADDRESS, mac, macsize);
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

static int netlink_add_addr(struct nlmsg* nlmsg, int sock, const char* dev,
                            const void* addr, int addrsize)
{
  struct ifaddrmsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  hdr.ifa_family = addrsize == 4 ? AF_INET : AF_INET6;
  hdr.ifa_prefixlen = addrsize == 4 ? 24 : 120;
  hdr.ifa_scope = RT_SCOPE_UNIVERSE;
  hdr.ifa_index = if_nametoindex(dev);
  netlink_init(nlmsg, RTM_NEWADDR, NLM_F_CREATE | NLM_F_REPLACE, &hdr,
               sizeof(hdr));
  netlink_attr(nlmsg, IFA_LOCAL, addr, addrsize);
  netlink_attr(nlmsg, IFA_ADDRESS, addr, addrsize);
  return netlink_send(nlmsg, sock);
}

static void netlink_add_addr4(struct nlmsg* nlmsg, int sock, const char* dev,
                              const char* addr)
{
  struct in_addr in_addr;
  inet_pton(AF_INET, addr, &in_addr);
  int err = netlink_add_addr(nlmsg, sock, dev, &in_addr, sizeof(in_addr));
  (void)err;
}

static void netlink_add_addr6(struct nlmsg* nlmsg, int sock, const char* dev,
                              const char* addr)
{
  struct in6_addr in6_addr;
  inet_pton(AF_INET6, addr, &in6_addr);
  int err = netlink_add_addr(nlmsg, sock, dev, &in6_addr, sizeof(in6_addr));
  (void)err;
}

static void netlink_add_neigh(struct nlmsg* nlmsg, int sock, const char* name,
                              const void* addr, int addrsize, const void* mac,
                              int macsize)
{
  struct ndmsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  hdr.ndm_family = addrsize == 4 ? AF_INET : AF_INET6;
  hdr.ndm_ifindex = if_nametoindex(name);
  hdr.ndm_state = NUD_PERMANENT;
  netlink_init(nlmsg, RTM_NEWNEIGH, NLM_F_EXCL | NLM_F_CREATE, &hdr,
               sizeof(hdr));
  netlink_attr(nlmsg, NDA_DST, addr, addrsize);
  netlink_attr(nlmsg, NDA_LLADDR, mac, macsize);
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

static int tunfd = -1;

#define TUN_IFACE "syz_tun"
#define LOCAL_MAC 0xaaaaaaaaaaaa
#define REMOTE_MAC 0xaaaaaaaaaabb
#define LOCAL_IPV4 "172.20.20.170"
#define REMOTE_IPV4 "172.20.20.187"
#define LOCAL_IPV6 "fe80::aa"
#define REMOTE_IPV6 "fe80::bb"

#define IFF_NAPI 0x0010

static void initialize_tun(void)
{
  tunfd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
  if (tunfd == -1) {
    printf("tun: can't open /dev/net/tun: please enable CONFIG_TUN=y\n");
    printf("otherwise fuzzing or reproducing might not work as intended\n");
    return;
  }
  const int kTunFd = 240;
  if (dup2(tunfd, kTunFd) < 0)
    exit(1);
  close(tunfd);
  tunfd = kTunFd;
  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, TUN_IFACE, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0) {
    exit(1);
  }
  char sysctl[64];
  sprintf(sysctl, "/proc/sys/net/ipv6/conf/%s/accept_dad", TUN_IFACE);
  write_file(sysctl, "0");
  sprintf(sysctl, "/proc/sys/net/ipv6/conf/%s/router_solicitations", TUN_IFACE);
  write_file(sysctl, "0");
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock == -1)
    exit(1);
  netlink_add_addr4(&nlmsg, sock, TUN_IFACE, LOCAL_IPV4);
  netlink_add_addr6(&nlmsg, sock, TUN_IFACE, LOCAL_IPV6);
  uint64_t macaddr = REMOTE_MAC;
  struct in_addr in_addr;
  inet_pton(AF_INET, REMOTE_IPV4, &in_addr);
  netlink_add_neigh(&nlmsg, sock, TUN_IFACE, &in_addr, sizeof(in_addr),
                    &macaddr, ETH_ALEN);
  struct in6_addr in6_addr;
  inet_pton(AF_INET6, REMOTE_IPV6, &in6_addr);
  netlink_add_neigh(&nlmsg, sock, TUN_IFACE, &in6_addr, sizeof(in6_addr),
                    &macaddr, ETH_ALEN);
  macaddr = LOCAL_MAC;
  netlink_device_change(&nlmsg, sock, TUN_IFACE, true, 0, &macaddr, ETH_ALEN,
                        NULL);
  close(sock);
}

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = (200 << 20);
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 32 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 256;
  setrlimit(RLIMIT_NOFILE, &rlim);
  if (unshare(CLONE_NEWNS)) {
  }
  if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(0x02000000)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
  typedef struct {
    const char* name;
    const char* value;
  } sysctl_t;
  static const sysctl_t sysctls[] = {
      {"/proc/sys/kernel/shmmax", "16777216"},
      {"/proc/sys/kernel/shmall", "536870912"},
      {"/proc/sys/kernel/shmmni", "1024"},
      {"/proc/sys/kernel/msgmax", "8192"},
      {"/proc/sys/kernel/msgmni", "1024"},
      {"/proc/sys/kernel/msgmnb", "1024"},
      {"/proc/sys/kernel/sem", "1024 1048576 500 1024"},
  };
  unsigned i;
  for (i = 0; i < sizeof(sysctls) / sizeof(sysctls[0]); i++)
    write_file(sysctls[i].name, sysctls[i].value);
}

static int wait_for_loop(int pid)
{
  if (pid < 0)
    exit(1);
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static void drop_caps(void)
{
  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    exit(1);
  const int drop = (1 << CAP_SYS_PTRACE) | (1 << CAP_SYS_NICE);
  cap_data[0].effective &= ~drop;
  cap_data[0].permitted &= ~drop;
  cap_data[0].inheritable &= ~drop;
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    exit(1);
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);
  setup_common();
  sandbox_common();
  drop_caps();
  if (unshare(CLONE_NEWNET)) {
  }
  initialize_tun();
  loop();
  exit(1);
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void loop(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0xful, 3ul, 2);
  if (res != -1)
    r[0] = res;
  NONFAILING(*(uint64_t*)0x205f5000 = 0);
  NONFAILING(*(uint32_t*)0x205f5008 = 0);
  NONFAILING(*(uint64_t*)0x205f5010 = 0x208feff0);
  NONFAILING(*(uint64_t*)0x208feff0 = 0x2080e000);
  NONFAILING(*(uint8_t*)0x2080e000 = 2);
  NONFAILING(*(uint8_t*)0x2080e001 = 0xe);
  NONFAILING(*(uint8_t*)0x2080e002 = 0);
  NONFAILING(*(uint8_t*)0x2080e003 = 0);
  NONFAILING(*(uint16_t*)0x2080e004 = 0x14);
  NONFAILING(*(uint16_t*)0x2080e006 = 0);
  NONFAILING(*(uint32_t*)0x2080e008 = 0);
  NONFAILING(*(uint32_t*)0x2080e00c = 0);
  NONFAILING(*(uint16_t*)0x2080e010 = 5);
  NONFAILING(*(uint16_t*)0x2080e012 = 6);
  NONFAILING(*(uint8_t*)0x2080e014 = 0);
  NONFAILING(*(uint8_t*)0x2080e015 = 0);
  NONFAILING(*(uint16_t*)0x2080e016 = 0);
  NONFAILING(*(uint16_t*)0x2080e018 = 0xa);
  NONFAILING(*(uint16_t*)0x2080e01a = htobe16(0));
  NONFAILING(*(uint32_t*)0x2080e01c = htobe32(0));
  NONFAILING(memcpy(
      (void*)0x2080e020,
      "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 16));
  NONFAILING(*(uint32_t*)0x2080e030 = 0);
  NONFAILING(*(uint16_t*)0x2080e038 = 8);
  NONFAILING(*(uint16_t*)0x2080e03a = 0x12);
  NONFAILING(*(uint16_t*)0x2080e03c = 0);
  NONFAILING(*(uint8_t*)0x2080e03e = 2);
  NONFAILING(*(uint8_t*)0x2080e03f = 0);
  NONFAILING(*(uint32_t*)0x2080e040 = 0);
  NONFAILING(*(uint32_t*)0x2080e044 = 0);
  NONFAILING(*(uint16_t*)0x2080e048 = 6);
  NONFAILING(*(uint16_t*)0x2080e04a = 0);
  NONFAILING(*(uint8_t*)0x2080e04c = 0);
  NONFAILING(*(uint8_t*)0x2080e04d = 0);
  NONFAILING(*(uint16_t*)0x2080e04e = 0);
  NONFAILING(*(uint32_t*)0x2080e050 = 0);
  NONFAILING(*(uint32_t*)0x2080e054 = 0);
  NONFAILING(*(uint8_t*)0x2080e058 = 0xac);
  NONFAILING(*(uint8_t*)0x2080e059 = 0x14);
  NONFAILING(*(uint8_t*)0x2080e05a = -1);
  NONFAILING(*(uint8_t*)0x2080e05b = 0xbb);
  NONFAILING(*(uint8_t*)0x2080e068 = 0xac);
  NONFAILING(*(uint8_t*)0x2080e069 = 0x14);
  NONFAILING(*(uint8_t*)0x2080e06a = 0x14);
  NONFAILING(*(uint8_t*)0x2080e06b = 0);
  NONFAILING(*(uint16_t*)0x2080e078 = 5);
  NONFAILING(*(uint16_t*)0x2080e07a = 5);
  NONFAILING(*(uint8_t*)0x2080e07c = 0);
  NONFAILING(*(uint8_t*)0x2080e07d = 0);
  NONFAILING(*(uint16_t*)0x2080e07e = 0);
  NONFAILING(*(uint16_t*)0x2080e080 = 0xa);
  NONFAILING(*(uint16_t*)0x2080e082 = htobe16(0));
  NONFAILING(*(uint32_t*)0x2080e084 = htobe32(0));
  NONFAILING(*(uint8_t*)0x2080e088 = 0xfe);
  NONFAILING(*(uint8_t*)0x2080e089 = 0x80);
  NONFAILING(*(uint8_t*)0x2080e08a = 0);
  NONFAILING(*(uint8_t*)0x2080e08b = 0);
  NONFAILING(*(uint8_t*)0x2080e08c = 0);
  NONFAILING(*(uint8_t*)0x2080e08d = 0);
  NONFAILING(*(uint8_t*)0x2080e08e = 0);
  NONFAILING(*(uint8_t*)0x2080e08f = 0);
  NONFAILING(*(uint8_t*)0x2080e090 = 0);
  NONFAILING(*(uint8_t*)0x2080e091 = 0);
  NONFAILING(*(uint8_t*)0x2080e092 = 0);
  NONFAILING(*(uint8_t*)0x2080e093 = 0);
  NONFAILING(*(uint8_t*)0x2080e094 = 0);
  NONFAILING(*(uint8_t*)0x2080e095 = 0);
  NONFAILING(*(uint8_t*)0x2080e096 = 0);
  NONFAILING(*(uint8_t*)0x2080e097 = -1);
  NONFAILING(*(uint32_t*)0x2080e098 = 0);
  NONFAILING(*(uint64_t*)0x208feff8 = 0xa0);
  NONFAILING(*(uint64_t*)0x205f5018 = 1);
  NONFAILING(*(uint64_t*)0x205f5020 = 0);
  NONFAILING(*(uint64_t*)0x205f5028 = 0);
  NONFAILING(*(uint32_t*)0x205f5030 = 0);
  syscall(__NR_sendmsg, r[0], 0x205f5000ul, 0ul);
  res = syscall(__NR_socket, 0x10ul, 3ul, 6);
  if (res != -1)
    r[1] = res;
  NONFAILING(*(uint64_t*)0x200001c0 = 0);
  NONFAILING(*(uint32_t*)0x200001c8 = 0);
  NONFAILING(*(uint64_t*)0x200001d0 = 0x20000180);
  NONFAILING(*(uint64_t*)0x20000180 = 0x20000040);
  NONFAILING(*(uint32_t*)0x20000040 = 0x1c);
  NONFAILING(*(uint16_t*)0x20000044 = 0x24);
  NONFAILING(*(uint16_t*)0x20000046 = 1);
  NONFAILING(*(uint32_t*)0x20000048 = 0);
  NONFAILING(*(uint32_t*)0x2000004c = 0);
  NONFAILING(*(uint32_t*)0x20000050 = 0);
  NONFAILING(*(uint16_t*)0x20000054 = 6);
  NONFAILING(*(uint16_t*)0x20000056 = 3);
  NONFAILING(*(uint8_t*)0x20000058 = 0);
  NONFAILING(*(uint8_t*)0x20000059 = 0);
  NONFAILING(*(uint64_t*)0x20000188 = 0x1c);
  NONFAILING(*(uint64_t*)0x200001d8 = 1);
  NONFAILING(*(uint64_t*)0x200001e0 = 0);
  NONFAILING(*(uint64_t*)0x200001e8 = 0);
  NONFAILING(*(uint32_t*)0x200001f0 = 0);
  syscall(__NR_sendmsg, r[1], 0x200001c0ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  install_segv_handler();
  use_temporary_dir();
  do_sandbox_none();
  return 0;
}
