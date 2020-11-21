// WARNING in ieee80211_check_rate_mask
// https://syzkaller.appspot.com/bug?id=be0e03ca215b06199629
// status:0
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sched.h>
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
#include <linux/genetlink.h>
#include <linux/if_addr.h>
#include <linux/if_ether.h>
#include <linux/if_link.h>
#include <linux/in6.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/nl80211.h>
#include <linux/rtnetlink.h>
#include <linux/veth.h>

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
  char buf[4096];
};

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
  if (size > 0)
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
  return ((struct nlmsgerr*)(hdr + 1))->error;
}

static int netlink_send(struct nlmsg* nlmsg, int sock)
{
  return netlink_send_ext(nlmsg, sock, 0, NULL);
}

static int netlink_query_family_id(struct nlmsg* nlmsg, int sock,
                                   const char* family_name)
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = CTRL_CMD_GETFAMILY;
  netlink_init(nlmsg, GENL_ID_CTRL, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, CTRL_ATTR_FAMILY_NAME, family_name,
               strnlen(family_name, GENL_NAMSIZ - 1) + 1);
  int n = 0;
  int err = netlink_send_ext(nlmsg, sock, GENL_ID_CTRL, &n);
  if (err < 0) {
    return -1;
  }
  uint16_t id = 0;
  struct nlattr* attr = (struct nlattr*)(nlmsg->buf + NLMSG_HDRLEN +
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
  recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0);
  return id;
}

static struct nlmsg nlmsg;

#define WIFI_INITIAL_DEVICE_COUNT 2
#define WIFI_MAC_BASE                                                          \
  {                                                                            \
    0x08, 0x02, 0x11, 0x00, 0x00, 0x00                                         \
  }
#define WIFI_IBSS_BSSID                                                        \
  {                                                                            \
    0x50, 0x50, 0x50, 0x50, 0x50, 0x50                                         \
  }
#define WIFI_IBSS_SSID                                                         \
  {                                                                            \
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10                                         \
  }
#define WIFI_DEFAULT_FREQUENCY 2412
#define WIFI_DEFAULT_SIGNAL 0
#define WIFI_DEFAULT_RX_RATE 1
#define HWSIM_CMD_REGISTER 1
#define HWSIM_CMD_FRAME 2
#define HWSIM_CMD_NEW_RADIO 4
#define HWSIM_ATTR_SUPPORT_P2P_DEVICE 14
#define HWSIM_ATTR_PERM_ADDR 22

#define IF_OPER_UP 6
struct join_ibss_props {
  int wiphy_freq;
  bool wiphy_freq_fixed;
  uint8_t* mac;
  uint8_t* ssid;
  int ssid_len;
};

static int set_interface_state(const char* interface_name, int on)
{
  struct ifreq ifr;
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    return -1;
  }
  memset(&ifr, 0, sizeof(ifr));
  strcpy(ifr.ifr_name, interface_name);
  int ret = ioctl(sock, SIOCGIFFLAGS, &ifr);
  if (ret < 0) {
    close(sock);
    return -1;
  }
  if (on)
    ifr.ifr_flags |= IFF_UP;
  else
    ifr.ifr_flags &= ~IFF_UP;
  ret = ioctl(sock, SIOCSIFFLAGS, &ifr);
  close(sock);
  if (ret < 0) {
    return -1;
  }
  return 0;
}

static int nl80211_set_interface(struct nlmsg* nlmsg, int sock,
                                 int nl80211_family, uint32_t ifindex,
                                 uint32_t iftype)
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = NL80211_CMD_SET_INTERFACE;
  netlink_init(nlmsg, nl80211_family, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, NL80211_ATTR_IFINDEX, &ifindex, sizeof(ifindex));
  netlink_attr(nlmsg, NL80211_ATTR_IFTYPE, &iftype, sizeof(iftype));
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
    return -1;
  }
  return 0;
}

static int nl80211_join_ibss(struct nlmsg* nlmsg, int sock, int nl80211_family,
                             uint32_t ifindex, struct join_ibss_props* props)
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = NL80211_CMD_JOIN_IBSS;
  netlink_init(nlmsg, nl80211_family, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, NL80211_ATTR_IFINDEX, &ifindex, sizeof(ifindex));
  netlink_attr(nlmsg, NL80211_ATTR_SSID, props->ssid, props->ssid_len);
  netlink_attr(nlmsg, NL80211_ATTR_WIPHY_FREQ, &(props->wiphy_freq),
               sizeof(props->wiphy_freq));
  if (props->mac)
    netlink_attr(nlmsg, NL80211_ATTR_MAC, props->mac, ETH_ALEN);
  if (props->wiphy_freq_fixed)
    netlink_attr(nlmsg, NL80211_ATTR_FREQ_FIXED, NULL, 0);
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
    return -1;
  }
  return 0;
}

static int get_ifla_operstate(struct nlmsg* nlmsg, int ifindex)
{
  struct ifinfomsg info;
  memset(&info, 0, sizeof(info));
  info.ifi_family = AF_UNSPEC;
  info.ifi_index = ifindex;
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock == -1) {
    return -1;
  }
  netlink_init(nlmsg, RTM_GETLINK, 0, &info, sizeof(info));
  int n;
  int err = netlink_send_ext(nlmsg, sock, RTM_NEWLINK, &n);
  close(sock);
  if (err) {
    return -1;
  }
  struct rtattr* attr = IFLA_RTA(NLMSG_DATA(nlmsg->buf));
  for (; RTA_OK(attr, n); attr = RTA_NEXT(attr, n)) {
    if (attr->rta_type == IFLA_OPERSTATE)
      return *((int32_t*)RTA_DATA(attr));
  }
  return -1;
}

static int await_ifla_operstate(struct nlmsg* nlmsg, char* interface,
                                int operstate)
{
  int ifindex = if_nametoindex(interface);
  while (true) {
    usleep(1000);
    int ret = get_ifla_operstate(nlmsg, ifindex);
    if (ret < 0)
      return ret;
    if (ret == operstate)
      return 0;
  }
  return 0;
}

static int nl80211_setup_ibss_interface(struct nlmsg* nlmsg, int sock,
                                        int nl80211_family_id, char* interface,
                                        struct join_ibss_props* ibss_props)
{
  int ifindex = if_nametoindex(interface);
  if (ifindex == 0) {
    return -1;
  }
  int ret = nl80211_set_interface(nlmsg, sock, nl80211_family_id, ifindex,
                                  NL80211_IFTYPE_ADHOC);
  if (ret < 0) {
    return -1;
  }
  ret = set_interface_state(interface, 1);
  if (ret < 0) {
    return -1;
  }
  ret = nl80211_join_ibss(nlmsg, sock, nl80211_family_id, ifindex, ibss_props);
  if (ret < 0) {
    return -1;
  }
  return 0;
}

static int hwsim80211_create_device(struct nlmsg* nlmsg, int sock,
                                    int hwsim_family,
                                    uint8_t mac_addr[ETH_ALEN])
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = HWSIM_CMD_NEW_RADIO;
  netlink_init(nlmsg, hwsim_family, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, HWSIM_ATTR_SUPPORT_P2P_DEVICE, NULL, 0);
  netlink_attr(nlmsg, HWSIM_ATTR_PERM_ADDR, mac_addr, ETH_ALEN);
  int err = netlink_send(nlmsg, sock);
  if (err < 0) {
    return -1;
  }
  return 0;
}

static void initialize_wifi_devices(void)
{
  uint8_t mac_addr[6] = WIFI_MAC_BASE;
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock < 0) {
    return;
  }
  int hwsim_family_id = netlink_query_family_id(&nlmsg, sock, "MAC80211_HWSIM");
  int nl80211_family_id = netlink_query_family_id(&nlmsg, sock, "nl80211");
  uint8_t ssid[] = WIFI_IBSS_SSID;
  uint8_t bssid[] = WIFI_IBSS_BSSID;
  struct join_ibss_props ibss_props = {.wiphy_freq = WIFI_DEFAULT_FREQUENCY,
                                       .wiphy_freq_fixed = true,
                                       .mac = bssid,
                                       .ssid = ssid,
                                       .ssid_len = sizeof(ssid)};
  for (int device_id = 0; device_id < WIFI_INITIAL_DEVICE_COUNT; device_id++) {
    mac_addr[5] = device_id;
    int ret = hwsim80211_create_device(&nlmsg, sock, hwsim_family_id, mac_addr);
    if (ret < 0)
      exit(1);
    char interface[6] = "wlan0";
    interface[4] += device_id;
    if (nl80211_setup_ibss_interface(&nlmsg, sock, nl80211_family_id, interface,
                                     &ibss_props) < 0)
      exit(1);
  }
  for (int device_id = 0; device_id < WIFI_INITIAL_DEVICE_COUNT; device_id++) {
    char interface[6] = "wlan0";
    interface[4] += device_id;
    int ret = await_ifla_operstate(&nlmsg, interface, IF_OPER_UP);
    if (ret < 0)
      exit(1);
  }
  close(sock);
}

static long syz_genetlink_get_family_id(volatile long name)
{
  struct nlmsg nlmsg_tmp;
  int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (fd == -1) {
    return -1;
  }
  int ret = netlink_query_family_id(&nlmsg_tmp, fd, (char*)name);
  close(fd);
  if (ret < 0) {
    return -1;
  }
  return ret;
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
  initialize_wifi_devices();
  loop();
  exit(1);
}

uint64_t r[7] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

void loop(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0x10);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000200, "nl80211\000", 8);
  res = -1;
  res = syz_genetlink_get_family_id(0x20000200);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000700, "wlan0\000\000\000\000\000\000\000\000\000\000\000",
         16);
  res = syscall(__NR_ioctl, r[0], 0x8933, 0x20000700ul);
  if (res != -1)
    r[2] = *(uint32_t*)0x20000710;
  *(uint64_t*)0x20000340 = 0;
  *(uint32_t*)0x20000348 = 0;
  *(uint64_t*)0x20000350 = 0x20000300;
  *(uint64_t*)0x20000300 = 0x20000240;
  *(uint32_t*)0x20000240 = 0x24;
  *(uint16_t*)0x20000244 = r[1];
  *(uint16_t*)0x20000246 = 5;
  *(uint32_t*)0x20000248 = 0;
  *(uint32_t*)0x2000024c = 0;
  *(uint8_t*)0x20000250 = 6;
  *(uint8_t*)0x20000251 = 0;
  *(uint16_t*)0x20000252 = 0;
  *(uint16_t*)0x20000254 = 8;
  *(uint16_t*)0x20000256 = 3;
  *(uint32_t*)0x20000258 = r[2];
  *(uint16_t*)0x2000025c = 8;
  *(uint16_t*)0x2000025e = 5;
  *(uint32_t*)0x20000260 = 3;
  *(uint64_t*)0x20000308 = 0x24;
  *(uint64_t*)0x20000358 = 1;
  *(uint64_t*)0x20000360 = 0;
  *(uint64_t*)0x20000368 = 0;
  *(uint32_t*)0x20000370 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000340ul, 0ul);
  *(uint64_t*)0x20000340 = 0;
  *(uint32_t*)0x20000348 = 0;
  *(uint64_t*)0x20000350 = 0x20000300;
  *(uint64_t*)0x20000300 = 0x20000380;
  memcpy((void*)0x20000380, "\x00\x03\x00\x00", 4);
  *(uint16_t*)0x20000384 = r[1];
  memcpy((void*)0x20000386, "\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0f\x00"
                            "\x00\x00\x08\x00\x03\x00",
         18);
  *(uint32_t*)0x20000398 = r[2];
  memcpy(
      (void*)0x2000039c,
      "\x28\x00\x0e\x00\x80\x00\x00\x00\xff\xff\xff\xff\xff\xff\x08\x02\x11\x00"
      "\x00\x01\x4c\x88\xe8\xeb\x7e\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x64\x00\xff\xff\x08\x00\x26\x00\x6c\x09\x00\x00\x08\x00\x0c\x00\x64\x00"
      "\x00\x00\x08\x00\x0d\x00\x00\x00\x00\x00\x0a\x00\x34\x00\x02\x02\x02\x02"
      "\x02\x02\x00\x00\x08\x00\x35\x00\x00\x00\x00\x00\x90\x02\x0f\x00\x04\x4e"
      "\x79\xc1\x99\xe8\x3b\xed\x43\x6d\x9b\x18\xd6\x35\xd7\xd6\xe8\x00\x08\x00"
      "\x00\x88\x54\x41\x3f\x64\xd1\x7b\x8d\x7a\x90\x41\xea\x1a\xa6\xcb\x32\x8e"
      "\x4e\xae\x74\xed\xc1\xe0\xad\x5b\x56\x00\x31\x9a\x0c\xf9\x6d\x64\xa3\x28"
      "\xee\xca\xfa\xc1\x12\xed\x3e\xc8\x84\xa1\x53\x1d\xfd\xf1\x7a\x1c\xbe\x07"
      "\xe1\x3a\xd5\x79\x11\xd1\x2d\x26\x9b\x02\x2c\xcf\x8f\xf9\xaf\x66\xb4\x1d"
      "\x84\xa1\xf4\x7c\xd1\x6e\xce\xac\xcf\x5e\xf9\x15\x9b\x14\xc6\xd0\x25\xae"
      "\xa1\x2d\xba\xe5\x03\xb9\x48\x22\xfd\x6a\x1c\x7b\xbb\x4c\x9d\x1e\xff\xa4"
      "\x6a\x99\xc6\x02\x5a\xba\x12\x6a\xb4\xd7\x43\x3b\x72\x2b\x8c\xf4\x0f\xaa"
      "\xb9\x1f\x78\x6c\x32\x81\x3e\xc3\x1c\xbb\xec\xee\xeb\xe4\x4f\xb3\x52\x7d"
      "\x98\x35\x6b\x7c\xef\x74\x53\xa6\xd0\xbc\x7c\x65\x73\xe1\xe9\x31\x55\x7e"
      "\x85\x94\x57\x2f\xe5\x20\xd0\xb3\x90\x3b\xd7\xe8\x5e\xc4\xab\xc6\x4e\x96"
      "\x4b\x56\x55\xb2\x40\xa4\xe6\x02\x0c\x7a\x9f\x72\xb1\x6a\xf0\xfb\xcd\xcb"
      "\x35\x4e\x91\xc6\x9c\x1a\x6b\xa7\x57\x64\x92\x63\xf8\xb0\xbc\x97\xce\x3f"
      "\xfc\x1f\xff\xa6\x04\x7b\xb4\xa1\x8a\xad\x45\xa0\xfa\x6f\x1c\xf4\x40\x8c"
      "\x8d\xf7\x9b\xce\xdd\xae\xff\xb0\x50\x1f\x61\x5f\xeb\x70\x80\x1a\xac\x79"
      "\xa1\xb3\xb7\x40\x01\x06\x6b\xfe\x7b\x65\x2c\x5b\xa9\xe6\xdf\x56\x06\x10"
      "\xaf\xa7\xb2\xb0\xae\x89\xd8\x16\x7e\xf9\x5c\xab\x5f\x20\x73\x20\x67\x98"
      "\x08\x9f\x72\x5a\x5d\x9f\x20\x7a\x14\x09\x90\x61\x12\xda\x7e\x52\xb9\x0d"
      "\x5e\xa3\x45\x37\xd6\xd6\xce\x24\xa2\xdb\x63\x14\x41\xbe\x1e\x6d\x63\x23"
      "\xd6\x16\x55\xd4\x15\x6b\x50\x6f\xdb\xe3\xf6\xc9\x81\x35\x28\xa9\x50\xdf"
      "\x8a\x4c\x2a\x6a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
      467);
  *(uint64_t*)0x20000308 = 0x300;
  *(uint64_t*)0x20000358 = 1;
  *(uint64_t*)0x20000360 = 0;
  *(uint64_t*)0x20000368 = 0;
  *(uint32_t*)0x20000370 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000340ul, 0ul);
  res = syscall(__NR_socket, 0x10ul, 3ul, 0x10);
  if (res != -1)
    r[3] = res;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0x10);
  if (res != -1)
    r[4] = res;
  memcpy((void*)0x20000700, "wlan0\000\000\000\000\000\000\000\000\000\000\000",
         16);
  res = syscall(__NR_ioctl, r[4], 0x8933, 0x20000700ul);
  if (res != -1)
    r[5] = *(uint32_t*)0x20000710;
  memcpy((void*)0x20000200, "nl80211\000", 8);
  res = -1;
  res = syz_genetlink_get_family_id(0x20000200);
  if (res != -1)
    r[6] = res;
  *(uint64_t*)0x20000140 = 0;
  *(uint32_t*)0x20000148 = 0;
  *(uint64_t*)0x20000150 = 0x20000100;
  *(uint64_t*)0x20000100 = 0x20000740;
  memcpy((void*)0x20000740, "8\000\000\000", 4);
  *(uint16_t*)0x20000744 = r[6];
  memcpy((void*)0x20000746, "\x01\x00\xb3\x00\x00\x00\x08\x00\x00\x00\x19\x8f"
                            "\x00\x00\x08\x00\x03\x00",
         18);
  *(uint32_t*)0x20000758 = r[5];
  memcpy((void*)0x2000075c, "\x05\x00\x24", 3);
  *(uint64_t*)0x20000108 = 0x38;
  *(uint64_t*)0x20000158 = 1;
  *(uint64_t*)0x20000160 = 0;
  *(uint64_t*)0x20000168 = 0;
  *(uint32_t*)0x20000170 = 0;
  syscall(__NR_sendmsg, r[3], 0x20000140ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  do_sandbox_none();
  return 0;
}
