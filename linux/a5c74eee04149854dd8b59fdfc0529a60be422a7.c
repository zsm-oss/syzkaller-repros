// WARNING in skb_warn_bad_offload (2)
// https://syzkaller.appspot.com/bug?id=a5c74eee04149854dd8b59fdfc0529a60be422a7
// status:invalid
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
#include <stdint.h>
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

struct csum_inet {
  uint32_t acc;
};

static void csum_inet_init(struct csum_inet* csum)
{
  csum->acc = 0;
}

static void csum_inet_update(struct csum_inet* csum, const uint8_t* data,
                             size_t length)
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
  const char* devtypes[] = {"ip6gretap", "bridge", "vcan", "bond", "veth"};
  const char* devnames[] = {"lo",       "sit0",    "bridge0", "vcan0",
                            "tunl0",    "gre0",    "gretap0", "ip_vti0",
                            "ip6_vti0", "ip6tnl0", "ip6gre0", "ip6gretap0",
                            "erspan0",  "bond0",   "veth0",   "veth1"};

  for (i = 0; i < sizeof(devtypes) / (sizeof(devtypes[0])); i++)
    execute_command(0, "ip link add dev %s0 type %s", devtypes[i], devtypes[i]);
  execute_command(0, "ip link add dev veth1 type veth");
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

long r[2];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_socket, 0x11, 3, 0x300);
  *(uint8_t*)0x20f3afd8 = 0x73;
  *(uint8_t*)0x20f3afd9 = 0x79;
  *(uint8_t*)0x20f3afda = 0x7a;
  *(uint8_t*)0x20f3afdb = 0x30;
  *(uint8_t*)0x20f3afdc = 0;
  *(uint32_t*)0x20f3afe8 = 0;
  *(uint8_t*)0x20f3afec = 0;
  *(uint8_t*)0x20f3afed = 0;
  *(uint8_t*)0x20f3afee = 0;
  *(uint8_t*)0x20f3afef = 0;
  *(uint8_t*)0x20f3aff0 = 0;
  *(uint8_t*)0x20f3aff1 = 0;
  *(uint8_t*)0x20f3aff2 = 0;
  *(uint8_t*)0x20f3aff3 = 0;
  *(uint8_t*)0x20f3aff4 = 0;
  *(uint8_t*)0x20f3aff5 = 0;
  *(uint8_t*)0x20f3aff6 = 0;
  *(uint8_t*)0x20f3aff7 = 0;
  *(uint8_t*)0x20f3aff8 = 0;
  *(uint8_t*)0x20f3aff9 = 0;
  *(uint8_t*)0x20f3affa = 0;
  *(uint8_t*)0x20f3affb = 0;
  *(uint8_t*)0x20f3affc = 0;
  *(uint8_t*)0x20f3affd = 0;
  *(uint8_t*)0x20f3affe = 0;
  *(uint8_t*)0x20f3afff = 0;
  if (syscall(__NR_ioctl, r[0], 0x8933, 0x20f3afd8) != -1)
    r[1] = *(uint32_t*)0x20f3afe8;
  *(uint32_t*)0x2008c000 = 8;
  syscall(__NR_setsockopt, r[0], 0x107, 0xf, 0x2008c000, 4);
  memcpy((void*)0x20000000,
         "\x00\x01\x00\x00\x3e\x02\x00\x14\x00\xf4\x97\x7d\x7b\x41\xb8\xd7\x00"
         "\x4f\x65\x3b\xcb\x05\x85\x00\x87\xff\xff\xff\xf4\x00\x00\x00\x01\x29"
         "\xb0\x93\x54\x61\x68\xec\xd9\xca\x02\x0c\x5e\x75\xe2\x27\x06\x2c\x9f"
         "\x49\x01\x00\x00\x00\x00\x00\x00\x00\xcc\x30\xc4\x00\x80\xff\x37\x8d"
         "\x58\x08\xd8\xc1\x99\x77\x34\xeb\xc0\xcb\x84\x0b\xf3\xb4\x76\x8a\x94"
         "\xd1\xed\xdc\x10\x02\x06\x37\x6a\xc4\xc9\x4c\x27\x89\x36\x4e",
         100);
  *(uint16_t*)0x200d9fec = 0x11;
  *(uint16_t*)0x200d9fee = htobe16(0x800);
  *(uint32_t*)0x200d9ff0 = r[1];
  *(uint16_t*)0x200d9ff4 = 1;
  *(uint8_t*)0x200d9ff6 = 0;
  *(uint8_t*)0x200d9ff7 = 6;
  *(uint8_t*)0x200d9ff8 = 0;
  *(uint8_t*)0x200d9ff9 = 0;
  *(uint8_t*)0x200d9ffa = 0;
  *(uint8_t*)0x200d9ffb = 0;
  *(uint8_t*)0x200d9ffc = 0;
  *(uint8_t*)0x200d9ffd = 0;
  *(uint8_t*)0x200d9ffe = 0;
  *(uint8_t*)0x200d9fff = 0;
  syscall(__NR_sendto, r[0], 0x20000000, 0x64, 0, 0x200d9fec, 0x14);
  *(uint8_t*)0x20000040 = 0xaa;
  *(uint8_t*)0x20000041 = 0xaa;
  *(uint8_t*)0x20000042 = 0xaa;
  *(uint8_t*)0x20000043 = 0xaa;
  *(uint8_t*)0x20000044 = 0;
  *(uint8_t*)0x20000045 = 0xbb;
  *(uint8_t*)0x20000046 = 0xaa;
  *(uint8_t*)0x20000047 = 0xaa;
  *(uint8_t*)0x20000048 = 0xaa;
  *(uint8_t*)0x20000049 = 0xaa;
  *(uint8_t*)0x2000004a = 0;
  *(uint8_t*)0x2000004b = 0xaa;
  *(uint16_t*)0x2000004c = htobe16(0x9100);
  STORE_BY_BITMASK(uint16_t, 0x2000004e, 0, 0, 3);
  STORE_BY_BITMASK(uint16_t, 0x2000004e, 7, 3, 1);
  STORE_BY_BITMASK(uint16_t, 0x2000004e, 2, 4, 12);
  *(uint16_t*)0x20000050 = htobe16(0x8100);
  STORE_BY_BITMASK(uint16_t, 0x20000052, 0x862, 0, 3);
  STORE_BY_BITMASK(uint16_t, 0x20000052, 0x7ffd, 3, 1);
  STORE_BY_BITMASK(uint16_t, 0x20000052, 4, 4, 12);
  *(uint16_t*)0x20000054 = htobe16(0x800);
  STORE_BY_BITMASK(uint8_t, 0x20000056, 5, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x20000056, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x20000057, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x20000057, 0, 2, 6);
  *(uint16_t*)0x20000058 = htobe16(0x24);
  *(uint16_t*)0x2000005a = 0;
  *(uint16_t*)0x2000005c = htobe16(0);
  *(uint8_t*)0x2000005e = 0;
  *(uint8_t*)0x2000005f = 6;
  *(uint16_t*)0x20000060 = 0;
  *(uint8_t*)0x20000062 = 0xac;
  *(uint8_t*)0x20000063 = 0x14;
  *(uint8_t*)0x20000064 = 0;
  *(uint8_t*)0x20000065 = 0xbb;
  *(uint8_t*)0x20000066 = 0xac;
  *(uint8_t*)0x20000067 = 0x14;
  *(uint8_t*)0x20000068 = 0;
  *(uint8_t*)0x20000069 = 0xbb;
  STORE_BY_BITMASK(uint16_t, 0x2000006a, 0, 0, 1);
  STORE_BY_BITMASK(uint16_t, 0x2000006a, 0, 1, 1);
  STORE_BY_BITMASK(uint16_t, 0x2000006a, 1, 2, 1);
  STORE_BY_BITMASK(uint16_t, 0x2000006a, 0, 3, 1);
  STORE_BY_BITMASK(uint16_t, 0x2000006a, 0, 4, 4);
  STORE_BY_BITMASK(uint16_t, 0x2000006a, 0, 8, 1);
  STORE_BY_BITMASK(uint16_t, 0x2000006a, 0, 9, 4);
  STORE_BY_BITMASK(uint16_t, 0x2000006a, 1, 13, 3);
  *(uint16_t*)0x2000006c = htobe16(0x880b);
  *(uint16_t*)0x2000006e = htobe16(0);
  *(uint16_t*)0x20000070 = htobe16(0);
  STORE_BY_BITMASK(uint16_t, 0x20000072, 0, 0, 1);
  STORE_BY_BITMASK(uint16_t, 0x20000072, 0, 1, 1);
  STORE_BY_BITMASK(uint16_t, 0x20000072, 0, 2, 1);
  STORE_BY_BITMASK(uint16_t, 0x20000072, 0, 3, 1);
  STORE_BY_BITMASK(uint16_t, 0x20000072, 0, 4, 9);
  STORE_BY_BITMASK(uint16_t, 0x20000072, 0, 13, 3);
  *(uint16_t*)0x20000074 = htobe16(0x800);
  STORE_BY_BITMASK(uint16_t, 0x20000076, 0, 0, 1);
  STORE_BY_BITMASK(uint16_t, 0x20000076, 0, 1, 1);
  STORE_BY_BITMASK(uint16_t, 0x20000076, 0, 2, 1);
  STORE_BY_BITMASK(uint16_t, 0x20000076, 0, 3, 1);
  STORE_BY_BITMASK(uint16_t, 0x20000076, 0, 4, 9);
  STORE_BY_BITMASK(uint16_t, 0x20000076, 0, 13, 3);
  *(uint16_t*)0x20000078 = htobe16(0x86dd);
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x20000056, 20);
  *(uint16_t*)0x20000060 = csum_inet_digest(&csum_1);
  syz_emit_ethernet(0x3a, 0x20000040, 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  setup_tun(0, true);
  loop();
  return 0;
}
