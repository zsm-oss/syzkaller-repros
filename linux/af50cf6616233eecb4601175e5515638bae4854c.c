// KASAN: use-after-free Read in vgem_gem_dumb_create
// https://syzkaller.appspot.com/bug?id=af50cf6616233eecb4601175e5515638bae4854c
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sched.h>
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
#include <sys/uio.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/capability.h>
#include <linux/futex.h>
#include <linux/genetlink.h>
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

unsigned long long procid;

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
  int i;
  for (i = 0; i < 100; i++) {
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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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
    if (__atomic_load_n(&ev->state, __ATOMIC_RELAXED))
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

static void netlink_nest(struct nlmsg* nlmsg, int typ)
{
  struct nlattr* attr = (struct nlattr*)nlmsg->pos;
  attr->nla_type = typ;
  nlmsg->pos += sizeof(*attr);
  nlmsg->nested[nlmsg->nesting++] = attr;
}

static void netlink_done(struct nlmsg* nlmsg)
{
  struct nlattr* attr = nlmsg->nested[--nlmsg->nesting];
  attr->nla_len = nlmsg->pos - (char*)attr;
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
  if (hdr->nlmsg_type == NLMSG_DONE) {
    *reply_len = 0;
    return 0;
  }
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

static int netlink_next_msg(struct nlmsg* nlmsg, unsigned int offset,
                            unsigned int total_len)
{
  struct nlmsghdr* hdr = (struct nlmsghdr*)(nlmsg->buf + offset);
  if (offset == total_len || offset + hdr->nlmsg_len > total_len)
    return -1;
  return hdr->nlmsg_len;
}

static void netlink_add_device_impl(struct nlmsg* nlmsg, const char* type,
                                    const char* name)
{
  struct ifinfomsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  netlink_init(nlmsg, RTM_NEWLINK, NLM_F_EXCL | NLM_F_CREATE, &hdr,
               sizeof(hdr));
  if (name)
    netlink_attr(nlmsg, IFLA_IFNAME, name, strlen(name));
  netlink_nest(nlmsg, IFLA_LINKINFO);
  netlink_attr(nlmsg, IFLA_INFO_KIND, type, strlen(type));
}

static void netlink_add_device(struct nlmsg* nlmsg, int sock, const char* type,
                               const char* name)
{
  netlink_add_device_impl(nlmsg, type, name);
  netlink_done(nlmsg);
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

static void netlink_add_veth(struct nlmsg* nlmsg, int sock, const char* name,
                             const char* peer)
{
  netlink_add_device_impl(nlmsg, "veth", name);
  netlink_nest(nlmsg, IFLA_INFO_DATA);
  netlink_nest(nlmsg, VETH_INFO_PEER);
  nlmsg->pos += sizeof(struct ifinfomsg);
  netlink_attr(nlmsg, IFLA_IFNAME, peer, strlen(peer));
  netlink_done(nlmsg);
  netlink_done(nlmsg);
  netlink_done(nlmsg);
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

static void netlink_add_hsr(struct nlmsg* nlmsg, int sock, const char* name,
                            const char* slave1, const char* slave2)
{
  netlink_add_device_impl(nlmsg, "hsr", name);
  netlink_nest(nlmsg, IFLA_INFO_DATA);
  int ifindex1 = if_nametoindex(slave1);
  netlink_attr(nlmsg, IFLA_HSR_SLAVE1, &ifindex1, sizeof(ifindex1));
  int ifindex2 = if_nametoindex(slave2);
  netlink_attr(nlmsg, IFLA_HSR_SLAVE2, &ifindex2, sizeof(ifindex2));
  netlink_done(nlmsg);
  netlink_done(nlmsg);
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

static void netlink_add_linked(struct nlmsg* nlmsg, int sock, const char* type,
                               const char* name, const char* link)
{
  netlink_add_device_impl(nlmsg, type, name);
  netlink_done(nlmsg);
  int ifindex = if_nametoindex(link);
  netlink_attr(nlmsg, IFLA_LINK, &ifindex, sizeof(ifindex));
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

static void netlink_add_vlan(struct nlmsg* nlmsg, int sock, const char* name,
                             const char* link, uint16_t id, uint16_t proto)
{
  netlink_add_device_impl(nlmsg, "vlan", name);
  netlink_nest(nlmsg, IFLA_INFO_DATA);
  netlink_attr(nlmsg, IFLA_VLAN_ID, &id, sizeof(id));
  netlink_attr(nlmsg, IFLA_VLAN_PROTOCOL, &proto, sizeof(proto));
  netlink_done(nlmsg);
  netlink_done(nlmsg);
  int ifindex = if_nametoindex(link);
  netlink_attr(nlmsg, IFLA_LINK, &ifindex, sizeof(ifindex));
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

static void netlink_add_macvlan(struct nlmsg* nlmsg, int sock, const char* name,
                                const char* link)
{
  netlink_add_device_impl(nlmsg, "macvlan", name);
  netlink_nest(nlmsg, IFLA_INFO_DATA);
  uint32_t mode = MACVLAN_MODE_BRIDGE;
  netlink_attr(nlmsg, IFLA_MACVLAN_MODE, &mode, sizeof(mode));
  netlink_done(nlmsg);
  netlink_done(nlmsg);
  int ifindex = if_nametoindex(link);
  netlink_attr(nlmsg, IFLA_LINK, &ifindex, sizeof(ifindex));
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

static void netlink_add_geneve(struct nlmsg* nlmsg, int sock, const char* name,
                               uint32_t vni, struct in_addr* addr4,
                               struct in6_addr* addr6)
{
  netlink_add_device_impl(nlmsg, "geneve", name);
  netlink_nest(nlmsg, IFLA_INFO_DATA);
  netlink_attr(nlmsg, IFLA_GENEVE_ID, &vni, sizeof(vni));
  if (addr4)
    netlink_attr(nlmsg, IFLA_GENEVE_REMOTE, addr4, sizeof(*addr4));
  if (addr6)
    netlink_attr(nlmsg, IFLA_GENEVE_REMOTE6, addr6, sizeof(*addr6));
  netlink_done(nlmsg);
  netlink_done(nlmsg);
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

#define IFLA_IPVLAN_FLAGS 2
#define IPVLAN_MODE_L3S 2
#undef IPVLAN_F_VEPA
#define IPVLAN_F_VEPA 2

static void netlink_add_ipvlan(struct nlmsg* nlmsg, int sock, const char* name,
                               const char* link, uint16_t mode, uint16_t flags)
{
  netlink_add_device_impl(nlmsg, "ipvlan", name);
  netlink_nest(nlmsg, IFLA_INFO_DATA);
  netlink_attr(nlmsg, IFLA_IPVLAN_MODE, &mode, sizeof(mode));
  netlink_attr(nlmsg, IFLA_IPVLAN_FLAGS, &flags, sizeof(flags));
  netlink_done(nlmsg);
  netlink_done(nlmsg);
  int ifindex = if_nametoindex(link);
  netlink_attr(nlmsg, IFLA_LINK, &ifindex, sizeof(ifindex));
  int err = netlink_send(nlmsg, sock);
  (void)err;
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

#define DEVLINK_FAMILY_NAME "devlink"

#define DEVLINK_CMD_PORT_GET 5
#define DEVLINK_ATTR_BUS_NAME 1
#define DEVLINK_ATTR_DEV_NAME 2
#define DEVLINK_ATTR_NETDEV_NAME 7

static int netlink_devlink_id_get(struct nlmsg* nlmsg, int sock)
{
  struct genlmsghdr genlhdr;
  struct nlattr* attr;
  int err, n;
  uint16_t id = 0;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = CTRL_CMD_GETFAMILY;
  netlink_init(nlmsg, GENL_ID_CTRL, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, CTRL_ATTR_FAMILY_NAME, DEVLINK_FAMILY_NAME,
               strlen(DEVLINK_FAMILY_NAME) + 1);
  err = netlink_send_ext(nlmsg, sock, GENL_ID_CTRL, &n);
  if (err) {
    return -1;
  }
  attr = (struct nlattr*)(nlmsg->buf + NLMSG_HDRLEN +
                          NLMSG_ALIGN(sizeof(genlhdr)));
  for (; (char*)attr < nlmsg->buf + n;
       attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
    if (attr->nla_type == CTRL_ATTR_FAMILY_ID) {
      id = *(uint16_t*)(attr + 1);
      break;
    }
  }
  if (!id) {
    return -1;
  }
  recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0); /* recv ack */
  return id;
}

static struct nlmsg nlmsg2;

static void initialize_devlink_ports(const char* bus_name, const char* dev_name,
                                     const char* netdev_prefix)
{
  struct genlmsghdr genlhdr;
  int len, total_len, id, err, offset;
  uint16_t netdev_index;
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock == -1)
    exit(1);
  int rtsock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (rtsock == -1)
    exit(1);
  id = netlink_devlink_id_get(&nlmsg, sock);
  if (id == -1)
    goto error;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = DEVLINK_CMD_PORT_GET;
  netlink_init(&nlmsg, id, NLM_F_DUMP, &genlhdr, sizeof(genlhdr));
  netlink_attr(&nlmsg, DEVLINK_ATTR_BUS_NAME, bus_name, strlen(bus_name) + 1);
  netlink_attr(&nlmsg, DEVLINK_ATTR_DEV_NAME, dev_name, strlen(dev_name) + 1);
  err = netlink_send_ext(&nlmsg, sock, id, &total_len);
  if (err) {
    goto error;
  }
  offset = 0;
  netdev_index = 0;
  while ((len = netlink_next_msg(&nlmsg, offset, total_len)) != -1) {
    struct nlattr* attr = (struct nlattr*)(nlmsg.buf + offset + NLMSG_HDRLEN +
                                           NLMSG_ALIGN(sizeof(genlhdr)));
    for (; (char*)attr < nlmsg.buf + offset + len;
         attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
      if (attr->nla_type == DEVLINK_ATTR_NETDEV_NAME) {
        char* port_name;
        char netdev_name[IFNAMSIZ];
        port_name = (char*)(attr + 1);
        snprintf(netdev_name, sizeof(netdev_name), "%s%d", netdev_prefix,
                 netdev_index);
        netlink_device_change(&nlmsg2, rtsock, port_name, true, 0, 0, 0,
                              netdev_name);
        break;
      }
    }
    offset += len;
    netdev_index++;
  }
error:
  close(rtsock);
  close(sock);
}

#define DEV_IPV4 "172.20.20.%d"
#define DEV_IPV6 "fe80::%02x"
#define DEV_MAC 0x00aaaaaaaaaa

static void netdevsim_add(unsigned int addr, unsigned int port_count)
{
  char buf[16];
  sprintf(buf, "%u %u", addr, port_count);
  if (write_file("/sys/bus/netdevsim/new_device", buf)) {
    snprintf(buf, sizeof(buf), "netdevsim%d", addr);
    initialize_devlink_ports("netdevsim", buf, "netdevsim");
  }
}
static void initialize_netdevices(void)
{
  char netdevsim[16];
  sprintf(netdevsim, "netdevsim%d", (int)procid);
  struct {
    const char* type;
    const char* dev;
  } devtypes[] = {
      {"ip6gretap", "ip6gretap0"}, {"bridge", "bridge0"},
      {"vcan", "vcan0"},           {"bond", "bond0"},
      {"team", "team0"},           {"dummy", "dummy0"},
      {"nlmon", "nlmon0"},         {"caif", "caif0"},
      {"batadv", "batadv0"},       {"vxcan", "vxcan1"},
      {"netdevsim", netdevsim},    {"veth", 0},
      {"xfrm", "xfrm0"},           {"wireguard", "wireguard0"},
      {"wireguard", "wireguard1"},
  };
  const char* devmasters[] = {"bridge", "bond", "team", "batadv"};
  struct {
    const char* name;
    int macsize;
    bool noipv6;
  } devices[] = {
      {"lo", ETH_ALEN},
      {"sit0", 0},
      {"bridge0", ETH_ALEN},
      {"vcan0", 0, true},
      {"tunl0", 0},
      {"gre0", 0},
      {"gretap0", ETH_ALEN},
      {"ip_vti0", 0},
      {"ip6_vti0", 0},
      {"ip6tnl0", 0},
      {"ip6gre0", 0},
      {"ip6gretap0", ETH_ALEN},
      {"erspan0", ETH_ALEN},
      {"bond0", ETH_ALEN},
      {"veth0", ETH_ALEN},
      {"veth1", ETH_ALEN},
      {"team0", ETH_ALEN},
      {"veth0_to_bridge", ETH_ALEN},
      {"veth1_to_bridge", ETH_ALEN},
      {"veth0_to_bond", ETH_ALEN},
      {"veth1_to_bond", ETH_ALEN},
      {"veth0_to_team", ETH_ALEN},
      {"veth1_to_team", ETH_ALEN},
      {"veth0_to_hsr", ETH_ALEN},
      {"veth1_to_hsr", ETH_ALEN},
      {"hsr0", 0},
      {"dummy0", ETH_ALEN},
      {"nlmon0", 0},
      {"vxcan0", 0, true},
      {"vxcan1", 0, true},
      {"caif0", ETH_ALEN},
      {"batadv0", ETH_ALEN},
      {netdevsim, ETH_ALEN},
      {"xfrm0", ETH_ALEN},
      {"veth0_virt_wifi", ETH_ALEN},
      {"veth1_virt_wifi", ETH_ALEN},
      {"virt_wifi0", ETH_ALEN},
      {"veth0_vlan", ETH_ALEN},
      {"veth1_vlan", ETH_ALEN},
      {"vlan0", ETH_ALEN},
      {"vlan1", ETH_ALEN},
      {"macvlan0", ETH_ALEN},
      {"macvlan1", ETH_ALEN},
      {"ipvlan0", ETH_ALEN},
      {"ipvlan1", ETH_ALEN},
      {"veth0_macvtap", ETH_ALEN},
      {"veth1_macvtap", ETH_ALEN},
      {"macvtap0", ETH_ALEN},
      {"macsec0", ETH_ALEN},
      {"veth0_to_batadv", ETH_ALEN},
      {"veth1_to_batadv", ETH_ALEN},
      {"batadv_slave_0", ETH_ALEN},
      {"batadv_slave_1", ETH_ALEN},
      {"geneve0", ETH_ALEN},
      {"geneve1", ETH_ALEN},
      {"wireguard0", 0},
      {"wireguard1", 0},
  };
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock == -1)
    exit(1);
  unsigned i;
  for (i = 0; i < sizeof(devtypes) / sizeof(devtypes[0]); i++)
    netlink_add_device(&nlmsg, sock, devtypes[i].type, devtypes[i].dev);
  for (i = 0; i < sizeof(devmasters) / (sizeof(devmasters[0])); i++) {
    char master[32], slave0[32], veth0[32], slave1[32], veth1[32];
    sprintf(slave0, "%s_slave_0", devmasters[i]);
    sprintf(veth0, "veth0_to_%s", devmasters[i]);
    netlink_add_veth(&nlmsg, sock, slave0, veth0);
    sprintf(slave1, "%s_slave_1", devmasters[i]);
    sprintf(veth1, "veth1_to_%s", devmasters[i]);
    netlink_add_veth(&nlmsg, sock, slave1, veth1);
    sprintf(master, "%s0", devmasters[i]);
    netlink_device_change(&nlmsg, sock, slave0, false, master, 0, 0, NULL);
    netlink_device_change(&nlmsg, sock, slave1, false, master, 0, 0, NULL);
  }
  netlink_device_change(&nlmsg, sock, "bridge_slave_0", true, 0, 0, 0, NULL);
  netlink_device_change(&nlmsg, sock, "bridge_slave_1", true, 0, 0, 0, NULL);
  netlink_add_veth(&nlmsg, sock, "hsr_slave_0", "veth0_to_hsr");
  netlink_add_veth(&nlmsg, sock, "hsr_slave_1", "veth1_to_hsr");
  netlink_add_hsr(&nlmsg, sock, "hsr0", "hsr_slave_0", "hsr_slave_1");
  netlink_device_change(&nlmsg, sock, "hsr_slave_0", true, 0, 0, 0, NULL);
  netlink_device_change(&nlmsg, sock, "hsr_slave_1", true, 0, 0, 0, NULL);
  netlink_add_veth(&nlmsg, sock, "veth0_virt_wifi", "veth1_virt_wifi");
  netlink_add_linked(&nlmsg, sock, "virt_wifi", "virt_wifi0",
                     "veth1_virt_wifi");
  netlink_add_veth(&nlmsg, sock, "veth0_vlan", "veth1_vlan");
  netlink_add_vlan(&nlmsg, sock, "vlan0", "veth0_vlan", 0, htons(ETH_P_8021Q));
  netlink_add_vlan(&nlmsg, sock, "vlan1", "veth0_vlan", 1, htons(ETH_P_8021AD));
  netlink_add_macvlan(&nlmsg, sock, "macvlan0", "veth1_vlan");
  netlink_add_macvlan(&nlmsg, sock, "macvlan1", "veth1_vlan");
  netlink_add_ipvlan(&nlmsg, sock, "ipvlan0", "veth0_vlan", IPVLAN_MODE_L2, 0);
  netlink_add_ipvlan(&nlmsg, sock, "ipvlan1", "veth0_vlan", IPVLAN_MODE_L3S,
                     IPVLAN_F_VEPA);
  netlink_add_veth(&nlmsg, sock, "veth0_macvtap", "veth1_macvtap");
  netlink_add_linked(&nlmsg, sock, "macvtap", "macvtap0", "veth0_macvtap");
  netlink_add_linked(&nlmsg, sock, "macsec", "macsec0", "veth1_macvtap");
  char addr[32];
  sprintf(addr, DEV_IPV4, 14 + 10);
  struct in_addr geneve_addr4;
  if (inet_pton(AF_INET, addr, &geneve_addr4) <= 0)
    exit(1);
  struct in6_addr geneve_addr6;
  if (inet_pton(AF_INET6, "fc00::01", &geneve_addr6) <= 0)
    exit(1);
  netlink_add_geneve(&nlmsg, sock, "geneve0", 0, &geneve_addr4, 0);
  netlink_add_geneve(&nlmsg, sock, "geneve1", 1, 0, &geneve_addr6);
  netdevsim_add((int)procid, 4);
  for (i = 0; i < sizeof(devices) / (sizeof(devices[0])); i++) {
    char addr[32];
    sprintf(addr, DEV_IPV4, i + 10);
    netlink_add_addr4(&nlmsg, sock, devices[i].name, addr);
    if (!devices[i].noipv6) {
      sprintf(addr, DEV_IPV6, i + 10);
      netlink_add_addr6(&nlmsg, sock, devices[i].name, addr);
    }
    uint64_t macaddr = DEV_MAC + ((i + 10ull) << 40);
    netlink_device_change(&nlmsg, sock, devices[i].name, true, 0, &macaddr,
                          devices[i].macsize, NULL);
  }
  close(sock);
}
static void initialize_netdevices_init(void)
{
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock == -1)
    exit(1);
  struct {
    const char* type;
    int macsize;
    bool noipv6;
    bool noup;
  } devtypes[] = {
      {"nr", 7, true}, {"rose", 5, true, true},
  };
  unsigned i;
  for (i = 0; i < sizeof(devtypes) / sizeof(devtypes[0]); i++) {
    char dev[32], addr[32];
    sprintf(dev, "%s%d", devtypes[i].type, (int)procid);
    sprintf(addr, "172.30.%d.%d", i, (int)procid + 1);
    netlink_add_addr4(&nlmsg, sock, dev, addr);
    if (!devtypes[i].noipv6) {
      sprintf(addr, "fe88::%02x:%02x", i, (int)procid + 1);
      netlink_add_addr6(&nlmsg, sock, dev, addr);
    }
    int macsize = devtypes[i].macsize;
    uint64_t macaddr = 0xbbbbbb +
                       ((unsigned long long)i << (8 * (macsize - 2))) +
                       (procid << (8 * (macsize - 1)));
    netlink_device_change(&nlmsg, sock, dev, !devtypes[i].noup, 0, &macaddr,
                          macsize, NULL);
  }
  close(sock);
}

#define MAX_FDS 30

static long syz_open_dev(volatile long a0, volatile long a1, volatile long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
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

int wait_for_loop(int pid)
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
  initialize_netdevices_init();
  if (unshare(CLONE_NEWNET)) {
  }
  initialize_netdevices();
  loop();
  exit(1);
}

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  int i;
  for (i = 0; i < 100; i++) {
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

static void close_fds()
{
  int fd;
  for (fd = 3; fd < MAX_FDS; fd++)
    close(fd);
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
  for (call = 0; call < 21; call++) {
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
  close_fds();
  if (!collide) {
    collide = 1;
    goto again;
  }
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter;
  for (iter = 0;; iter++) {
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
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0,
                 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res;
  switch (call) {
  case 0:
    syscall(__NR_perf_event_open, 0ul, 0, 0ul, -1, 0ul);
    break;
  case 1:
    *(uint32_t*)0x2001d000 = 1;
    *(uint32_t*)0x2001d004 = 0x70;
    *(uint8_t*)0x2001d008 = 0;
    *(uint8_t*)0x2001d009 = 0;
    *(uint8_t*)0x2001d00a = 0;
    *(uint8_t*)0x2001d00b = 0;
    *(uint32_t*)0x2001d00c = 0;
    *(uint64_t*)0x2001d010 = 0x41c1;
    *(uint64_t*)0x2001d018 = 0;
    *(uint64_t*)0x2001d020 = 0;
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 0, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 1, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 2, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 3, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 4, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 5, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 6, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 7, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 8, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 9, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 10, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 11, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 12, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 13, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 14, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 15, 2);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 17, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 18, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 19, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 20, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 21, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 22, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 23, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 24, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 25, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 26, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 27, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 28, 1);
    STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 29, 35);
    *(uint32_t*)0x2001d030 = 0;
    *(uint32_t*)0x2001d034 = 0;
    *(uint64_t*)0x2001d038 = 0;
    *(uint64_t*)0x2001d040 = 0;
    *(uint64_t*)0x2001d048 = 0;
    *(uint64_t*)0x2001d050 = 0;
    *(uint32_t*)0x2001d058 = 0;
    *(uint32_t*)0x2001d05c = 0;
    *(uint64_t*)0x2001d060 = 0;
    *(uint32_t*)0x2001d068 = 0;
    *(uint16_t*)0x2001d06c = 0;
    *(uint16_t*)0x2001d06e = 0;
    syscall(__NR_perf_event_open, 0x2001d000ul, 0, -1ul, -1, 0ul);
    break;
  case 2:
    res = syscall(__NR_creat, 0ul, 2ul);
    if (res != -1)
      r[0] = res;
    break;
  case 3:
    syscall(__NR_link, 0ul, 0ul);
    break;
  case 4:
    syscall(__NR_prctl, 0x17ul, 0x8000004, 0, 0);
    break;
  case 5:
    syscall(__NR_llistxattr, 0ul, 0ul, 0ul);
    break;
  case 6:
    syscall(__NR_ioctl, r[0], 0x4018620dul, 0ul);
    break;
  case 7:
    syscall(__NR_clone, 0x2102001ffcul, 0ul, 0x9999999999999999ul, 0ul, -1ul);
    break;
  case 8:
    memcpy((void*)0x20000240, "/dev/dri/card#\000", 15);
    res = syz_open_dev(0x20000240, 0, 0);
    if (res != -1)
      r[1] = res;
    break;
  case 9:
    *(uint32_t*)0x20000000 = 6;
    *(uint32_t*)0x20000004 = 6;
    *(uint32_t*)0x20000008 = 7;
    *(uint32_t*)0x2000000c = 0;
    *(uint32_t*)0x20000010 = 0;
    *(uint32_t*)0x20000014 = 0;
    *(uint64_t*)0x20000018 = 0;
    res = syscall(__NR_ioctl, r[1], 0xc02064b2ul, 0x20000000ul);
    if (res != -1)
      r[2] = *(uint32_t*)0x20000010;
    break;
  case 10:
    syscall(__NR_socket, 2ul, 3ul, 1ul);
    break;
  case 11:
    syscall(__NR_dup, -1);
    break;
  case 12:
    syscall(__NR_ioctl, -1, 0x8912ul, 0x400200ul);
    break;
  case 13:
    syscall(__NR_dup, -1);
    break;
  case 14:
    syscall(__NR_ioctl, -1, 0xc02064b2ul, 0ul);
    break;
  case 15:
    syscall(__NR_socketpair, 1ul, 0ul, 0ul, 0ul);
    break;
  case 16:
    syscall(__NR_socket, 2ul, 2ul, 0x88ul);
    break;
  case 17:
    syscall(__NR_getsockopt, -1, 1ul, 0x11ul, 0ul, 0ul);
    break;
  case 18:
    res = syscall(__NR_socket, 0x10ul, 2ul, 0);
    if (res != -1)
      r[3] = res;
    break;
  case 19:
    syscall(__NR_dup, r[3]);
    break;
  case 20:
    *(uint32_t*)0x20000280 = r[2];
    syscall(__NR_ioctl, r[1], 0xc00464b4ul, 0x20000280ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      do_sandbox_none();
    }
  }
  sleep(1000000);
  return 0;
}
