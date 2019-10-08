// BUG: corrupted list in team_nl_cmd_options_set
// https://syzkaller.appspot.com/bug?id=fc36b8e053a3de897ec8e28a8753ffc66ccd897d
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/genetlink.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/netlink.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
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
  va_end(args);
  rv = system(command);
  if (rv) {
    if (panic)
      fail("command '%s' failed: %d", &command[0], rv);
  }
}

static int tunfd = -1;
static int tun_frags_enabled;

#define SYZ_TUN_MAX_PACKET_SIZE 1000

#define TUN_IFACE "syz_tun"

#define LOCAL_MAC "aa:aa:aa:aa:aa:aa"
#define REMOTE_MAC "aa:aa:aa:aa:aa:bb"

#define LOCAL_IPV4 "172.20.20.170"
#define REMOTE_IPV4 "172.20.20.187"

#define LOCAL_IPV6 "fe80::aa"
#define REMOTE_IPV6 "fe80::bb"

#define IFF_NAPI 0x0010
#define IFF_NAPI_FRAGS 0x0020

static void initialize_tun(void)
{
  tunfd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
  if (tunfd == -1) {
    printf("tun: can't open /dev/net/tun: please enable CONFIG_TUN=y\n");
    printf("otherwise fuzzing or reproducing might not work as intended\n");
    return;
  }
  const int kTunFd = 252;
  if (dup2(tunfd, kTunFd) < 0)
    fail("dup2(tunfd, kTunFd) failed");
  close(tunfd);
  tunfd = kTunFd;

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, TUN_IFACE, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_NAPI | IFF_NAPI_FRAGS;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0) {
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
      fail("tun: ioctl(TUNSETIFF) failed");
  }
  if (ioctl(tunfd, TUNGETIFF, (void*)&ifr) < 0)
    fail("tun: ioctl(TUNGETIFF) failed");
  tun_frags_enabled = (ifr.ifr_flags & IFF_NAPI_FRAGS) != 0;

  execute_command(1, "sysctl -w net.ipv6.conf.%s.accept_dad=0", TUN_IFACE);

  execute_command(1, "sysctl -w net.ipv6.conf.%s.router_solicitations=0",
                  TUN_IFACE);

  execute_command(1, "ip link set dev %s address %s", TUN_IFACE, LOCAL_MAC);
  execute_command(1, "ip addr add %s/24 dev %s", LOCAL_IPV4, TUN_IFACE);
  execute_command(1, "ip -6 addr add %s/120 dev %s", LOCAL_IPV6, TUN_IFACE);
  execute_command(1, "ip neigh add %s lladdr %s dev %s nud permanent",
                  REMOTE_IPV4, REMOTE_MAC, TUN_IFACE);
  execute_command(1, "ip -6 neigh add %s lladdr %s dev %s nud permanent",
                  REMOTE_IPV6, REMOTE_MAC, TUN_IFACE);
  execute_command(1, "ip link set dev %s up", TUN_IFACE);
}

#define DEV_IPV4 "172.20.20.%d"
#define DEV_IPV6 "fe80::%02hx"
#define DEV_MAC "aa:aa:aa:aa:aa:%02hx"

static void initialize_netdevices(void)
{
  unsigned i;
  const char* devtypes[] = {"ip6gretap", "bridge", "vcan",
                            "bond",      "veth",   "team"};
  const char* devnames[] = {
      "lo",      "sit0",    "bridge0",  "vcan0",   "tunl0",   "gre0",
      "gretap0", "ip_vti0", "ip6_vti0", "ip6tnl0", "ip6gre0", "ip6gretap0",
      "erspan0", "bond0",   "veth0",    "veth1",   "team0"};

  for (i = 0; i < sizeof(devtypes) / (sizeof(devtypes[0])); i++)
    execute_command(0, "ip link add dev %s0 type %s", devtypes[i], devtypes[i]);
  execute_command(0, "ip link add dev veth1 type veth");
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
    full = frags->full;
    count = frags->count;
    if (count > MAX_FRAGS)
      count = MAX_FRAGS;
    for (i = 0; i < count && length != 0; i++) {
      uint32_t size = 0;
      size = frags->frags[i];
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

static uintptr_t syz_genetlink_get_family_id(uintptr_t name)
{
  char buf[512] = {0};
  struct nlmsghdr* hdr = (struct nlmsghdr*)buf;
  struct genlmsghdr* genlhdr = (struct genlmsghdr*)NLMSG_DATA(hdr);
  struct nlattr* attr = (struct nlattr*)(genlhdr + 1);
  hdr->nlmsg_len =
      sizeof(*hdr) + sizeof(*genlhdr) + sizeof(*attr) + GENL_NAMSIZ;
  hdr->nlmsg_type = GENL_ID_CTRL;
  hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
  genlhdr->cmd = CTRL_CMD_GETFAMILY;
  attr->nla_type = CTRL_ATTR_FAMILY_NAME;
  attr->nla_len = sizeof(*attr) + GENL_NAMSIZ;
  strncpy((char*)(attr + 1), (char*)name, GENL_NAMSIZ);
  struct iovec iov = {hdr, hdr->nlmsg_len};
  struct sockaddr_nl addr = {0};
  addr.nl_family = AF_NETLINK;
  int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (fd == -1) {
    return -1;
  }
  struct msghdr msg = {&addr, sizeof(addr), &iov, 1, NULL, 0, 0};
  if (sendmsg(fd, &msg, 0) == -1) {
    close(fd);
    return -1;
  }
  ssize_t n = recv(fd, buf, sizeof(buf), 0);
  close(fd);
  if (n <= 0) {
    return -1;
  }
  if (hdr->nlmsg_type != GENL_ID_CTRL) {
    return -1;
  }
  for (; (char*)attr < buf + n;
       attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
    if (attr->nla_type == CTRL_ATTR_FAMILY_ID)
      return *(uint16_t*)(attr + 1);
  }
  return -1;
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
  rlim.rlim_cur = rlim.rlim_max = 32 << 20;
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

static int do_sandbox_none(void)
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
  initialize_tun();
  initialize_netdevices();

  loop();
  doexit(1);
}

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0, 0x0};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 0xa, 3, 0x3c);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0x10, 3, 0x10);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000240, "team", 5);
  res = syz_genetlink_get_family_id(0x20000240);
  if (res != -1)
    r[2] = res;
  memcpy((void*)0x20000980,
         "\x74\x65\x61\x6d\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint32_t*)0x20000990 = 0;
  res = syscall(__NR_ioctl, r[0], 0x8933, 0x20000980);
  if (res != -1)
    r[3] = *(uint32_t*)0x20000990;
  *(uint64_t*)0x20000d00 = 0x20000200;
  *(uint16_t*)0x20000200 = 0x10;
  *(uint16_t*)0x20000202 = 0;
  *(uint32_t*)0x20000204 = 0;
  *(uint32_t*)0x20000208 = 0;
  *(uint32_t*)0x20000d08 = 0xc;
  *(uint64_t*)0x20000d10 = 0x20000cc0;
  *(uint64_t*)0x20000cc0 = 0x200009c0;
  *(uint32_t*)0x200009c0 = 0x118;
  *(uint16_t*)0x200009c4 = r[2];
  *(uint16_t*)0x200009c6 = 1;
  *(uint32_t*)0x200009c8 = 0;
  *(uint32_t*)0x200009cc = 0;
  *(uint8_t*)0x200009d0 = 1;
  *(uint8_t*)0x200009d1 = 0;
  *(uint16_t*)0x200009d2 = 0;
  *(uint16_t*)0x200009d4 = 8;
  *(uint16_t*)0x200009d6 = 1;
  *(uint32_t*)0x200009d8 = r[3];
  *(uint16_t*)0x200009dc = 0xfc;
  *(uint16_t*)0x200009de = 2;
  *(uint16_t*)0x200009e0 = 0x38;
  *(uint16_t*)0x200009e2 = 1;
  *(uint16_t*)0x200009e4 = 0x24;
  *(uint16_t*)0x200009e6 = 1;
  memcpy((void*)0x200009e8, "\x6e\x6f\x74\x69\x66\x79\x5f\x70\x65\x65\x72\x73"
                            "\x5f\x69\x6e\x74\x65\x72\x76\x61\x6c\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint16_t*)0x20000a08 = 8;
  *(uint16_t*)0x20000a0a = 3;
  *(uint8_t*)0x20000a0c = 3;
  *(uint16_t*)0x20000a10 = 8;
  *(uint16_t*)0x20000a12 = 4;
  *(uint32_t*)0x20000a14 = 0;
  *(uint16_t*)0x20000a18 = 0x38;
  *(uint16_t*)0x20000a1a = 1;
  *(uint16_t*)0x20000a1c = 0x24;
  *(uint16_t*)0x20000a1e = 1;
  memcpy((void*)0x20000a20, "\x6e\x6f\x74\x69\x66\x79\x5f\x70\x65\x65\x72\x73"
                            "\x5f\x69\x6e\x74\x65\x72\x76\x61\x6c\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint16_t*)0x20000a40 = 8;
  *(uint16_t*)0x20000a42 = 3;
  *(uint8_t*)0x20000a44 = 3;
  *(uint16_t*)0x20000a48 = 8;
  *(uint16_t*)0x20000a4a = 4;
  *(uint32_t*)0x20000a4c = 0;
  *(uint16_t*)0x20000a50 = 0x4c;
  *(uint16_t*)0x20000a52 = 1;
  *(uint16_t*)0x20000a54 = 0x24;
  *(uint16_t*)0x20000a56 = 1;
  memcpy((void*)0x20000a58, "\x6c\x62\x5f\x74\x78\x5f\x6d\x65\x74\x68\x6f\x64"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint16_t*)0x20000a78 = 8;
  *(uint16_t*)0x20000a7a = 3;
  *(uint8_t*)0x20000a7c = 5;
  *(uint16_t*)0x20000a80 = 0x1c;
  *(uint16_t*)0x20000a82 = 4;
  memcpy((void*)0x20000a84, "hash_to_port_mapping", 21);
  *(uint16_t*)0x20000a9c = 0x3c;
  *(uint16_t*)0x20000a9e = 1;
  *(uint16_t*)0x20000aa0 = 0x24;
  *(uint16_t*)0x20000aa2 = 1;
  memcpy((void*)0x20000aa4, "\x6c\x62\x5f\x74\x78\x5f\x6d\x65\x74\x68\x6f\x64"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint16_t*)0x20000ac4 = 8;
  *(uint16_t*)0x20000ac6 = 3;
  *(uint8_t*)0x20000ac8 = 5;
  *(uint16_t*)0x20000acc = 0xc;
  *(uint16_t*)0x20000ace = 4;
  memcpy((void*)0x20000ad0, "hash", 5);
  *(uint64_t*)0x20000cc8 = 0x118;
  *(uint64_t*)0x20000d18 = 1;
  *(uint64_t*)0x20000d20 = 0;
  *(uint64_t*)0x20000d28 = 0;
  *(uint32_t*)0x20000d30 = 4;
  syscall(__NR_sendmsg, r[1], 0x20000d00, 0x10);
  *(uint8_t*)0x20000040 = 0xaa;
  *(uint8_t*)0x20000041 = 0xaa;
  *(uint8_t*)0x20000042 = 0xaa;
  *(uint8_t*)0x20000043 = 0xaa;
  *(uint8_t*)0x20000044 = 0xaa;
  *(uint8_t*)0x20000045 = 0xaa;
  *(uint8_t*)0x20000046 = 0xaa;
  *(uint8_t*)0x20000047 = 0xaa;
  *(uint8_t*)0x20000048 = 0xaa;
  *(uint8_t*)0x20000049 = 0xaa;
  *(uint8_t*)0x2000004a = 0xaa;
  *(uint8_t*)0x2000004b = 0xaa;
  *(uint16_t*)0x2000004c = htobe16(0x11);
  *(uint8_t*)0x2000004e = 0xf8;
  *(uint8_t*)0x2000004f = 0xaa;
  memcpy((void*)0x20000050, "E", 1);
  *(uint32_t*)0x20000100 = 1;
  *(uint32_t*)0x20000104 = 2;
  *(uint32_t*)0x20000108 = 0x5f8;
  *(uint32_t*)0x2000010c = 0x789;
  *(uint32_t*)0x20000110 = 0x6db;
  *(uint32_t*)0x20000114 = 0x215;
  syz_emit_ethernet(0x11, 0x20000040, 0x20000100);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  int pid = do_sandbox_none();
  int status = 0;
  while (waitpid(pid, &status, __WALL) != pid) {
  }
  return 0;
}
