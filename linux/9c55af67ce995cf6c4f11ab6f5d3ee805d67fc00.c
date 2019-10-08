// kernel BUG at net/core/skbuff.c:LINE! (3)
// https://syzkaller.appspot.com/bug?id=9c55af67ce995cf6c4f11ab6f5d3ee805d67fc00
// status:open
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
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
#include <stdarg.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdio.h>
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
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0xa, 0x4000000000000003, 5);
  *(uint16_t*)0x20024000 = 0xa;
  *(uint16_t*)0x20024002 = 0;
  *(uint32_t*)0x20024004 = 0;
  *(uint8_t*)0x20024008 = -1;
  *(uint8_t*)0x20024009 = 1;
  *(uint8_t*)0x2002400a = 0;
  *(uint8_t*)0x2002400b = 0;
  *(uint8_t*)0x2002400c = 0;
  *(uint8_t*)0x2002400d = 0;
  *(uint8_t*)0x2002400e = 0;
  *(uint8_t*)0x2002400f = 0;
  *(uint8_t*)0x20024010 = 0;
  *(uint8_t*)0x20024011 = 0;
  *(uint8_t*)0x20024012 = 0;
  *(uint8_t*)0x20024013 = 0;
  *(uint8_t*)0x20024014 = 0;
  *(uint8_t*)0x20024015 = 0;
  *(uint8_t*)0x20024016 = 0;
  *(uint8_t*)0x20024017 = 1;
  *(uint32_t*)0x20024018 = 7;
  syscall(__NR_connect, r[0], 0x20024000, 0x1c);
  *(uint64_t*)0x20000000 = 0x20000000;
  *(uint32_t*)0x20000008 = 0;
  *(uint64_t*)0x20000010 = 0x20023000;
  *(uint64_t*)0x20000018 = 0;
  *(uint64_t*)0x20000020 = 0x20023fd0;
  *(uint64_t*)0x20000028 = 0x1030;
  *(uint32_t*)0x20000030 = 0;
  *(uint64_t*)0x20023fd0 = 0x10;
  *(uint32_t*)0x20023fd8 = 0x10a;
  *(uint32_t*)0x20023fdc = 0xd930;
  *(uint64_t*)0x20023fe0 = 0x1010;
  *(uint32_t*)0x20023fe8 = 0x29;
  *(uint32_t*)0x20023fec = 3;
  memcpy(
      (void*)0x20023ff0,
      "\x10\xc0\x80\xe5\x80\x6b\x45\x07\x6a\x94\x59\xb8\x47\x75\xb5\x05\x81\x19"
      "\xc2\x11\x85\x72\x5d\xa3\x59\x55\x70\x6a\x52\xa7\x66\x54\x9a\x6d\x23\x59"
      "\x7a\xc0\x37\x8e\x59\x7f\x62\xa6\x75\x25\xdc\x57\xcb\x48\xac\x97\x0c\xf6"
      "\x62\xe7\x7d\x62\x40\x09\x89\xda\x2e\xa6\x62\xf1\xf7\x3d\x17\x7a\x92\x21"
      "\xdc\x71\x9a\x00\xcc\xfd\x15\x15\x7a\xd7\x2f\x58\x1c\xee\xdd\x0e\x89\x64"
      "\xd5\xc5\xf7\x58\x29\x9f\x18\x09\xd7\xa8\xeb\x26\xf2\xd5\x4b\x21\x85\x68"
      "\x9c\x9e\xfd\x02\x9f\x03\x71\xd3\x86\xfe\xa5\xc7\xb7\xc4\x55\x9e\x62\x58"
      "\x79\xdf\x74\x6e\x86\xc7\xd7\xbc\x28\x97\xe6\x91\x0c\x8d\xe4\xee\xc3\x66"
      "\x13\x9c\xc6\xc3\xe3\xd7\x9c\xbf\xa8\xde\x60\x2f\x7b\x1f\xea\xbd\x7e\x5b"
      "\xdd\x2b\xd2\x82\xa2\x77\x2d\xf0\xad\xa7\x4a\x05\xdb\x29\xf0\xd2\xb9\xb7"
      "\x3d\x46\x2e\xf7\x93\xc5\x1e\x5f\x45\xd1\xdd\x45\xaf\xf9\x2b\x05\xf0\x8d"
      "\xe4\x57\x25\x3a\xd0\xe9\xd0\x87\x40\x70\xde\x90\xb9\xde\x3c\x22\x39\x57"
      "\x2f\xd9\x58\x27\xf9\x50\xa9\xe4\xf4\xd9\x06\x4a\x97\x85\xa5\x6a\x42\x77"
      "\xad\x6f\x16\x97\x6f\xda\x4f\x3e\x2f\x8e\xfc\xd2\xdc\xc7\xfc\x0d\xfd\x7b"
      "\xcf\x87\x83\x7b\x1f\x2d\x55\x95\x28\x80\x26\xd9\x9a\x4f\x96\x3b\x50\x5d"
      "\xb7\xac\xc4\xb2\xa9\x17\x6a\x39\x9e\xbd\xdd\x0f\x14\x66\xef\xb8\xf5\xf4"
      "\x9e\xf5\x56\xc4\x4b\x30\x95\xdd\xc7\x28\xdb\x07\x65\x29\xda\xdc\x21\x35"
      "\x0f\x11\x35\xd3\xd8\x7e\x1d\x71\x0d\xba\x27\x66\x6b\x0b\x20\x66\xad\xeb"
      "\x23\xdc\xdf\xea\x03\x51\xed\x44\x10\xa1\xa8\xb8\xfe\x2f\xb8\x5d\x21\xc0"
      "\xf7\xad\xde\x62\x07\xc4\x85\x12\xe8\x96\x76\xf3\x2b\x3d\x37\x2b\x1f\x6d"
      "\x13\xc2\xdc\x58\x48\xbc\x6e\xb4\x37\xb0\xe2\x84\xf1\x2e\x26\xc8\xf8\x23"
      "\x73\xec\x3f\x99\xbe\xb9\xdf\x69\xe1\x88\xde\xd8\x6b\xbf\xae\xbc\x79\xa7"
      "\xbc\x98\x1f\x57\xfa\x53\xdf\xb6\x89\xcc\x0c\xb6\xc0\x2e\xa6\xf8\x71\xd6"
      "\x2b\x72\x2a\x8b\x29\xd5\x2e\x57\xc5\x79\x92\x20\xe0\x02\xee\x21\x58\xc2"
      "\xbb\xe0\x58\x67\x3c\xbd\xc0\xdd\x9b\x8e\xbc\x29\x27\xf9\xce\x22\x1e\x75"
      "\x68\x06\x87\x88\xab\xe8\x30\x2a\x40\xc8\xde\x37\x31\x35\xef\xbf\x90\xca"
      "\xb6\x3b\x8d\x58\x23\xaf\x54\x0b\xae\xc5\x76\xf9\x74\x1f\x08\x8f\x27\xc3"
      "\xdc\x53\xa0\xa7\xd6\xec\x4c\xcb\xcc\xaf\x2c\x12\x5c\x80\x0d\xcf\x36\x38"
      "\xd0\xac\xb1\x70\xfb\xc6\xf5\x7f\xdd\xb5\x97\x77\xdb\x4a\x50\x4a\xbd\xc3"
      "\x40\x98\xc4\x8f\xbe\x5e\x43\x90\x9c\xb2\x1e\x4d\x03\xdb\x82\x61\x15\xfe"
      "\x9b\x0c\x67\xdf\x07\x3c\xa2\xdc\xae\x84\xfa\x36\x5c\x72\x70\xaa\x69\xd5"
      "\x28\xfd\x7b\x11\x4f\x7d\xb4\x08\x49\x38\xd5\x01\x88\x91\x45\x3e\x93\xd0"
      "\x2a\xbf\x79\x4f\xce\x92\xfd\x21\xb7\x81\x52\x6c\x48\xb3\x1a\x57\x7a\x02"
      "\xcd\x9c\x9b\x80\x4b\xd3\x8e\x19\xe5\xf1\x79\x5e\x62\xb8\x79\x7e\x15\x39"
      "\xc8\x66\xc8\x61\x42\x38\x1b\x4f\x3a\x3b\x17\x6a\xb0\x39\x04\xb0\xaf\x95"
      "\xcf\x79\xc3\x84\x2c\x2f\x00\xaf\x10\x08\x22\x5b\x0a\x5c\x44\x29\xbd\x80"
      "\x47\x5b\xc9\xe9\x12\x1c\xf1\x8a\x01\x8c\x01\xed\x7f\x40\xbf\xe9\x21\xef"
      "\xc5\x02\x81\xea\x3e\x81\x49\xf2\x90\xf6\x31\x1c\xee\xe9\x27\x5a\x2e\x73"
      "\xf5\x86\xeb\xa5\x49\x59\xe4\x46\xc3\x0b\x65\x4f\xdd\x67\xe5\xd8\x5a\x17"
      "\xc1\x70\xcc\x93\x89\x43\x8d\x2e\x93\x0b\xfe\x20\xf7\x58\xc7\x25\xfc\x6d"
      "\x26\x90\x49\xef\x04\x2c\x50\x62\xac\x86\xc8\xa3\x20\x57\x16\xa6\xa3\x75"
      "\x87\x67\x87\xdf\xd2\x73\xe9\x2f\x3d\x39\xfa\x72\x2c\x8a\x96\x3a\xe2\xeb"
      "\x66\xc4\x82\x84\x32\x01\xd7\x51\x0c\x37\x89\x4a\x0a\x09\x3e\x2e\x03\xb0"
      "\x46\x64\x28\x7d\x54\xfc\x76\xd7\xf0\xe5\x51\x96\x6c\xd5\xbe\x09\x42\xe3"
      "\x10\xdc\x22\x86\x69\xa1\xd5\x6e\x57\x6e\x95\x8d\x40\x03\xa5\x29\x16\x52"
      "\x4a\xe6\x85\xdb\x7c\x46\x70\xc0\x1a\xdf\xfe\xe7\xdc\x2a\x10\xb5\x9a\x6a"
      "\xfa\x13\xb9\x1d\xd7\x82\x5b\xe7\x03\x59\x72\x8b\xb0\x82\x71\x9e\x29\xb5"
      "\x18\xe2\x0e\x59\x4d\x75\x21\xaa\x5a\xf9\x2f\xb5\xfc\x21\xf3\xcb\x2f\x39"
      "\x84\xcf\x97\xf5\x1d\xc2\x59\xe5\x23\x25\x54\x36\x4f\x30\x0a\x27\x6b\x22"
      "\x96\x80\x73\x66\x24\xa5\x70\x56\x9e\x0b\x07\x30\x2f\xb5\x60\x06\xfe\x50"
      "\xe1\x45\x72\x96\xa2\x9f\x8b\x91\x58\xdb\x8b\x32\x3e\x31\x6c\x3f\x2f\x1d"
      "\xa7\xda\xf1\x54\x16\xf0\x4f\x7d\x27\x11\xf0\xae\x78\x24\xca\x5e\x2b\xc3"
      "\x3a\x0f\x33\xd6\xa4\x29\xa3\x3c\xb1\x01\x4f\xb2\xb7\x28\xc9\xcd\x4c\x2f"
      "\x1f\xb4\x85\xde\x1c\x62\x99\x75\x96\x1c\x72\xe3\x79\x79\x11\xb9\xc0\x2e"
      "\x94\xc9\xcc\xf8\x21\xae\x17\x7b\x72\xb3\xf7\x8c\x2e\x72\x0d\x36\xf2\x86"
      "\xde\xdb\xd6\x07\x19\x27\xc7\x70\xeb\xfe\x1e\x26\x2d\x84\x3b\x3f\xaf\x11"
      "\xf9\x83\x2b\x10\x34\x94\x95\x92\xa1\x90\x27\xae\x0f\x25\xe4\x56\x15\xc0"
      "\x10\x10\x43\x2c\x3d\x15\xa5\x13\x04\xc6\x73\x64\xf9\x44\x2b\x63\x71\x7a"
      "\xec\xc3\x8d\xe2\xf2\xe4\xa6\x7c\xf6\x44\xf9\x0f\x5a\xba\x9d\xf4\x5d\x69"
      "\xf1\x72\x0e\x50\xa4\x00\xbd\x9b\xd7\xc4\x2d\xe7\x38\x4b\xc8\xcd\x5c\xc8"
      "\xd8\x6a\x5e\xf9\xac\xfc\xd5\xf1\x65\x78\x4f\x42\xa2\xdb\x3b\x7d\xd2\xe7"
      "\xa1\xe6\x35\xc7\xc2\xe6\x2f\x33\x02\x68\xb6\x39\x91\x43\x38\x62\xf3\x6c"
      "\x65\x1e\x34\xf5\x97\xa5\xd1\x29\xc0\x71\xf5\x17\xbc\x6f\x4d\xc1\xb1\xb1"
      "\x29\x72\xc9\x21\x81\xb6\x43\x21\x9e\xdc\x47\x39\x23\xef\x42\xb7\x10\x12"
      "\x64\x0d\xff\x44\x04\x6e\x9d\x81\x18\xe1\x00\x08\xe0\x30\x2e\xaa\xc6\xaf"
      "\x84\xb2\x59\x2a\x7e\x48\x98\x2a\x30\xd1\xd0\xec\xab\xb6\xf1\xb7\x80\x3b"
      "\xd2\x2d\x3e\x5f\x5c\x43\xc1\x5e\xa4\x69\xd4\xa2\x13\xbd\x4b\x14\xa4\x16"
      "\x32\xb9\xdc\x20\x00\xac\x34\x07\xe2\x1c\xbb\xc6\x3b\x17\x19\x80\x89\x0b"
      "\x06\x68\x7e\x5a\xef\x9d\x48\xcc\x9b\x9e\x25\x46\xb1\x41\x5d\x3d\xfd\x3d"
      "\xe8\xc0\xde\xa5\xd2\xee\x54\x2b\x0b\x5c\xa4\x2f\x33\x3f\xe7\xf8\x5b\x20"
      "\x52\xe3\x10\xdf\x68\xd7\xf5\x98\x68\x86\x5a\x0d\xee\xe4\x28\xae\x6f\x79"
      "\xc0\xf8\x04\x89\x7a\x5d\x2d\x22\x7e\xbc\xed\xd0\x03\x1d\x34\xfb\xcb\x2e"
      "\xb0\xfa\x06\xd4\x1c\xa9\x7b\x1a\x7d\x8b\xec\x5f\xdd\x34\xd5\xc6\xd8\xb4"
      "\xa7\xfe\xa9\x7f\xce\x8c\x02\xe9\x3d\x6e\xa1\xf2\xb2\x1f\xa0\xa4\xaa\xa2"
      "\x20\xad\xcb\x9e\xf9\xd6\xf3\xc4\x58\xd4\x41\x7a\x3a\xb5\x4f\xc0\x2b\x9a"
      "\x40\x46\xf8\x16\xa3\x58\x6b\xd9\x86\x6b\x2c\x48\x64\xb6\x07\x25\x60\x6e"
      "\x7e\xaa\x39\xae\x1d\xbe\x11\xcd\xf0\x6e\x62\x05\x19\xfd\x9d\xba\xc1\x8d"
      "\x5e\x69\x43\x56\x03\x86\x8c\x04\x7b\x8f\x32\x02\x55\x05\xf3\xdf\xf5\xb6"
      "\x62\x86\x20\x80\xe1\x5f\xae\xc2\xba\xe9\x4a\xb4\x2f\x92\x20\xa8\xcb\xbf"
      "\xfd\x80\xd3\x7e\xed\xfe\x2b\xe1\x98\xad\x36\xb2\x4b\xfb\x1b\x9a\x4d\x05"
      "\x2c\x22\x96\x87\x0c\xcb\xb5\x2f\xcb\x0a\xc9\x59\x86\xe8\xcf\xb9\x10\x3f"
      "\x2d\xa2\xe5\x2c\x26\x6d\x14\x58\x64\xb4\x29\xd3\xa3\x58\x71\xab\xb3\x68"
      "\xd6\xfe\xfb\x55\xdf\x30\x56\x4f\xdd\x3f\x2f\x4e\x3f\x51\x46\xae\x7e\x3e"
      "\x25\xfa\x62\xa5\x74\x86\x8a\x1e\x53\xa9\xd3\x99\xef\x06\x8b\x24\xd8\xff"
      "\xdc\x89\xcf\x41\x9c\x69\xd7\x10\x20\xc0\x6b\x3a\x79\x4b\x09\x24\x94\xfe"
      "\xe9\xab\x9d\x7a\xf5\x00\x8e\xe0\xfc\xc0\xf7\x93\x49\x94\x1c\x21\xba\xcd"
      "\xd8\x12\xb6\x72\xfa\x53\x3c\x32\x85\xf7\xef\xca\xec\x0c\x85\x88\xfc\x51"
      "\x5d\x64\xa3\x47\x16\x79\xb7\x96\x22\x8d\xd4\x3d\xb9\x81\x29\xaa\x2d\xf6"
      "\x75\xf0\x30\x19\xb2\xa0\x0c\xfc\xff\x10\x1f\x30\xf5\x7f\xce\x89\x27\xb9"
      "\xcb\x71\x13\x0c\xac\xea\xc4\x26\x5b\x2f\x04\x07\xe6\x3a\xfe\x66\xc6\x83"
      "\x54\xc0\x2d\xe2\xd6\x20\xc5\xb3\x7d\x4f\x2f\x9b\x2c\x3c\x59\xa7\xe3\x44"
      "\x3a\x05\x81\x83\x70\xac\x13\x17\x03\x3d\x30\x61\xe8\x4e\xc8\xa7\xdf\x90"
      "\x9f\x68\x09\x55\xad\x3d\x65\x5e\x41\x29\xf8\xd8\xf8\x24\x75\x63\x55\x98"
      "\xe0\xb3\x52\x3a\x4b\x4e\x68\x1a\xab\x76\x8f\x16\xb1\x23\x1f\x8d\x8b\xcd"
      "\xa4\xa1\xb0\x63\xf8\x23\xc1\x8f\xd9\x66\x5e\x6a\x95\xd8\x99\x81\xc2\xdd"
      "\x81\x7d\x39\x2f\x8b\x4d\xc1\x9e\x0f\xee\x91\xbe\x76\x2f\x77\xba\xde\x67"
      "\x5f\xe1\x68\xed\x14\xda\x44\xe3\xdd\x61\x1a\x15\x66\x52\x46\x6d\x79\xf3"
      "\x62\xf7\x89\x75\xda\x43\x8f\x02\x20\x99\x06\xed\x52\x93\x5c\x3e\x96\x4b"
      "\x1c\x1b\xbf\x3e\xdb\x63\xad\x54\xb7\xb8\xc8\x19\x59\x57\x4c\x48\x31\x0a"
      "\x6c\x71\x2a\x0c\x5a\x9f\x2d\x47\x20\xc9\xc8\xa1\xfc\x3d\x68\xb8\xab\x65"
      "\x68\x55\x5c\x8d\x14\x71\x1c\x09\xbe\xd9\xf4\xa1\xd6\x29\x4d\x26\xb5\x1d"
      "\xce\xef\xd5\xba\x2e\x71\x38\x35\x6b\x55\xb2\x0e\x02\xf5\x33\x6b\x67\x74"
      "\x26\x45\x1d\xf9\x02\xc9\x8e\xc8\xbb\x15\xd3\x20\xca\xbc\xfe\x97\x26\xfb"
      "\x98\x81\x22\x80\x4d\xd4\x48\xe6\xdf\xf3\xf5\x5a\x30\x72\x13\x05\x5f\x36"
      "\x61\x1d\x52\x34\xf8\x6d\x1a\x9b\xcf\xed\xd5\xca\xa9\xc4\xff\xea\xc4\x79"
      "\xc3\xf3\xa5\xd2\x7d\x1a\xb3\xd9\x3d\xfa\xae\x48\x7e\xe0\x96\x33\x10\x76"
      "\x50\x5e\x5e\xb7\x90\x6e\xe2\x4e\xe0\xe8\xde\xdc\xf2\xd8\x60\x91\xce\x96"
      "\x33\xe2\x79\xdc\xd6\xaa\x47\x60\x5c\x05\x02\xeb\xcc\x19\x86\xc4\xc2\x6e"
      "\x3a\x49\x5c\x38\x88\x94\xb4\x85\x03\xb7\xda\xd6\x11\xcf\x5c\xa9\x39\xc3"
      "\xe5\xf6\xa6\x2b\x0d\xdd\xac\x5c\xd9\xc8\xb4\x3a\xd4\xdd\x15\xce\x86\xb6"
      "\x40\xd4\xab\x71\xeb\x9b\x34\xa4\x4a\x5f\x59\xd0\x7b\xf0\x27\xba\x85\x7b"
      "\x41\x6b\x6b\xda\x70\x9b\xf0\x69\xdf\x37\xc2\xd3\x18\x7d\x9c\x86\xd3\x27"
      "\xcc\x28\xb5\x1c\xe0\xd3\x29\x0a\xd5\xaf\xab\xec\x58\x04\x6d\x25\x12\x49"
      "\x3d\xc1\x2c\xb5\xc9\xf5\xf0\x26\x5e\xc8\xef\x04\xbe\xf5\x3c\xef\xde\x59"
      "\x13\xe7\x39\x37\x7b\xf3\x66\xe1\x88\xfc\x01\x30\x09\xf5\x43\xe1\x6a\x8e"
      "\x26\xbf\xbe\xcd\x0a\x85\x61\xb8\xca\x8e\xb4\x72\x41\xcb\xa0\xd2\x7f\x2c"
      "\x56\x0b\x6d\xab\x2e\x04\x08\x6e\xc3\x7f\xe3\xe9\x93\xd7\x09\x65\x9a\xa7"
      "\xf1\xf1\x93\x79\xee\xf1\x93\xf1\xd0\x19\xec\x93\x14\xb0\xa9\x99\xfd\x9a"
      "\x01\xfe\x0c\xb8\x42\x8e\xba\xc6\x20\x52\x0b\x73\xf1\xde\x06\xce\xec\x09"
      "\x06\xee\x17\x5d\xdd\x49\xc7\x42\x02\x37\xbc\x89\xba\xad\xdd\x5f\x37\xf9"
      "\x3d\x01\xab\x48\xea\x95\x9d\x69\x27\x38\x62\x39\x4b\x0c\xac\x21\x28\xac"
      "\xac\xc8\x0c\x72\xec\x33\xf5\x4a\xf0\x0c\xf8\x0e\xdf\x53\xcc\x22\xf1\x12"
      "\x32\xda\x93\x32\xf2\x99\x0c\xcc\x2a\x15\xe9\x84\xbf\xce\x5c\xdf\x26\x33"
      "\xaf\x86\x95\x95\x78\x60\x40\x6e\xce\xb3\xec\xc9\x3a\x2e\x96\x3f\xb2\x32"
      "\xc9\xfe\x76\x63\x46\xd6\xc7\x39\x19\x05\x03\xbc\x0d\xef\x3f\x56\x60\x85"
      "\x7a\x9a\xce\x44\x12\xdc\xd2\xd4\xea\xfd\x4b\x29\xef\xf8\x38\x84\x45\xd5"
      "\xf8\xeb\xd9\x82\x95\x64\xb3\x69\xbf\x66\xcd\x35\xcf\xa3\x87\x6a\xb7\xe7"
      "\xd4\x3b\x41\x77\x9f\x45\x32\x92\xf6\x49\x97\x61\x7c\x50\x8d\x09\x68\x66"
      "\x48\xd4\xd8\x17\x28\x8b\x20\x5e\xce\x8a\xa8\x21\xa6\x70\x82\xf7\x24\xc7"
      "\x3d\xa0\xf8\x23\xd5\xcb\xf6\xf2\x23\xde\x9c\x67\x63\x60\x12\x71\xe3\x57"
      "\xbc\xae\x72\x03\x21\xd9\xe3\x70\xd8\xa8\x25\x44\x82\x17\x6a\xcb\x41\x7a"
      "\x15\x0e\x2f\x77\xa4\x73\x99\x81\x98\x64\xac\x59\xcb\x00\x04\x53\x16\xe4"
      "\xc8\x47\xfc\xa8\xaa\x5c\x92\x47\x48\xe0\x39\x01\x24\x58\x05\x3e\xbd\x04"
      "\x39\x2b\xe2\x22\xc8\xd4\x59\x3e\xcb\x22\x7e\xcb\xb7\x73\xc9\x4e\x6c\x1a"
      "\x40\x09\x12\xc5\xc7\x0c\xff\xb3\x73\x15\x68\xb7\x54\x8b\x78\x8f\x68\x0c"
      "\xd1\x6f\x2f\xa8\x56\x5d\x57\xed\x8b\xd0\xd4\x8e\xac\x88\x7e\x69\x06\xae"
      "\xea\xb6\x74\x67\xdd\xe0\x92\xb6\xef\xf4\xe2\x16\x08\x90\x90\x02\xd3\x19"
      "\xf6\x39\xf8\x12\xce\x89\x8d\x23\x0b\x20\xd2\x8a\x4c\x4a\xa1\x11\x02\x54"
      "\x4d\x41\xce\x7e\x73\x6b\x60\x22\x25\x40\x58\x93\x2c\xaf\xbd\xe0\x00\x13"
      "\x5a\x65\x0c\x07\xf8\x24\xac\xc5\xb7\x5d\x6c\x2a\x7b\x53\x1b\x6c\xa7\xd0"
      "\xc5\x43\xd3\x5b\x82\xac\x20\x05\xb4\x33\x65\xb8\x94\x7f\x0c\x68\xd5\x0f"
      "\x2f\x8d\x35\x55\xf1\xdc\x6d\x61\x7e\x3a\x2d\x82\xd5\x07\x7f\x2c\xdc\x0f"
      "\xff\x6c\x6d\xd8\xf3\x1b\x4d\xb1\x14\x8f\x3c\x30\x0c\x7d\x58\x23\x7b\x9a"
      "\xe6\x14\xc3\x22\x9d\x8d\x51\xfd\x68\x35\x2c\x54\x40\xe8\xf6\x51\xe2\x6e"
      "\x93\x59\x6b\x87\xff\x51\xf3\x50\xbc\x1e\xad\xa9\x59\xdd\xd2\xcf\x3b\x82"
      "\x0c\xd4\xc1\x9c\xe5\x35\xd7\x39\x56\x7f\xf7\xac\xcf\xf1\x6d\x38\x33\x4a"
      "\x9e\x6a\x85\xeb\xbe\xdb\x64\xfc\x78\x06\xa6\xaa\xae\xea\x1b\xf8\xb5\xcd"
      "\x49\x06\x0b\x27\x10\xec\xad\x68\x51\xcf\x37\x27\x3a\x4d\xe8\x5e\x90\x80"
      "\x32\x24\xe5\xc1\x28\xe3\x52\xd1\xca\x9d\x20\x35\xb1\xa4\x8a\x76\x40\xd3"
      "\xd4\xce\xd3\x0a\x98\xc7\x37\x09\xd8\x9f\x85\xf2\x42\xbf\xc2\xc2\x37\xa7"
      "\x94\x71\x7a\x94\xef\x13\x91\xea\x35\x61\x52\x25\x78\xc6\x80\x40\x01\x6d"
      "\x5a\x99\x9f\xba\xd0\x61\xfb\xf1\x91\x3b\x50\xed\x8f\xe3\x15\x14\x49\xd2"
      "\x90\x8e\xe0\x16\x8c\x04\x86\xc0\x67\x93\xf1\xf8\x2a\xd8\x78\x64\xa8\x7e"
      "\x5b\xca\x96\x4f\x78\x8b\x07\x2e\x19\x3d\x81\x5c\x3a\x96\x94\xd8\xf8\xa8"
      "\x76\x9e\x7d\x82\x9e\x6e\x41\x44\x30\x58\xe7\xdf\x0a\x18\xf3\x88\xc6\x07"
      "\x9f\x9d\x1a\xb5\x4f\xcb\x69\x62\xff\xed\x27\xa5\x45\x98\x93\x28\xb2\xff"
      "\x01\xe1\xdb\x5a\xa3\x1f\xd9\xb1\xaa\xca\x41\xa5\x6e\xdc\x09\x6a\x9d\xf8"
      "\x8d\x1c\x7e\xce\x1f\x59\xa2\x2d\x4a\xe3\xf0\xc7\xc1\x05\x2e\x82\xe0\x9f"
      "\xf9\x42\xd3\x00\x19\xc6\xde\xaa\x7d\x91\x54\xc9\xba\xa7\xef\x0f\x2c\x09"
      "\x2d\xb4\x42\x7d\x56\xb7\xb1\xee\x49\x2b\xd5\x02\xf3\xa5\x94\x33\x60\x67"
      "\x34\xee\xeb\xcc\x3a\x8e\xd2\xb5\xd8\x5e\xa4\x0d\x67\xcc\xe7\x10\x68\x0c"
      "\x7d\x66\xdd\x95\xb3\x1c\x4d\x3b\x2b\xf1\x05\xe0\x6d\xee\x7f\x87\x01\x43"
      "\x83\xd5\x2c\x17\x91\xb3\x49\xb6\x5d\x30\x60\x75\x96\x74\xc7\x32\x70\xbd"
      "\x29\x86\x73\x55\x02\x82\x28\x8b\x43\x65\x43\xae\xf4\xdc\xe3\x24\xa8\xac"
      "\x4f\x23\x44\xac\x76\xf8\x4f\x1b\xde\xa1\xd4\x19\x1a\x9b\x01\x71\x36\x67"
      "\x35\xd0\x28\x2c\xd6\x27\xee\xba\x13\x96\x3e\x4b\x45\x1b\x6d\x74\x6f\x2a"
      "\x9c\x94\xb4\xa2\x7a\xd9\xbc\xf4\xb9\x00\x30\x4e\x40\x89\x19\x71\x05\xde"
      "\xe9\xa2\x11\x5b\xb1\x5f\xe1\x44\x77\x4f\x0e\xd4\xae\x32\x02\x3c\x9e\x40"
      "\x70\xa8\x31\xce\xdf\xff\x06\xcd\x16\x80\xbd\x81\x49\x53\xcc\x69\x3e\xeb"
      "\x35\x50\x95\x01\x6e\x52\x10\x4b\xf7\xc0\xad\x36\x07\xd2\x95\x7f\x80\xd0"
      "\x69\x58\xbe\x8f\x92\xfd\xcc\x58\x5c\xfa\x64\xd2\x07\x1e\xdf\x3e\xaa\x61"
      "\xb1\x40\x22\x0d\x02\xad\xe4\xef\x69\x7a\xc2\xad\x1f\x8e\xe6\x3c\x5b\x7d"
      "\xd6\xfd\x1a\xb2\x5d\xd6\x61\x4c\x08\x5f\x0e\x4b\xf7\x4b\x2d\xce\x0d\x27"
      "\xa7\x8c\x64\xb0\x9f\x40\x2a\x95\x1a\xfa\x16\x24\x61\x19\x0d\x8e\x02\x72"
      "\xbc\x4d\x0e\xe1\xe7\xa7\x36\xeb\x37\xbd\x57\x1e\x3a\x04\x8f\x29\xfa\x86"
      "\x0f\xcd\xc9\xb3\x29\x19\x4e\x0f\x95\x51\xe2\xb4\x6e\xa3\x79\xd4\xa4\xda"
      "\x2c\xb6\xbd\x92\xf5\x19\xc5\xc0\x9a\xa5\x5e\xa6\x8c\xec\xf4\x30\x10\x1c"
      "\x15\x53\x40\xec\x0b\x58\x6c\xd1\x9b\x3f\x07\x3f\x9e\xf4\xcf\x02\xfc\xc7"
      "\x84\x63\x0e\xe6\xa4\x20\xff\x5b\x60\xfb\x5b\x7f\xe7\x50\x82\xb5\x78\xc9"
      "\x59\x7f\xe5\xa6\xa3\x3c\x58\x46\xea\xbc\x53\x12\xec\x72\x83\x9f\x86\xc0"
      "\x4c\x9a\x64\xae\xd7\xcf\xd6\x84\x11\x09\xb3\xc9\x64\x87\x52\x50\x42\x7e"
      "\x60\x1d\x93\xed\x63\xeb\xca\xb5\x26\xa4\x52\x49\x81\x62\x72\x26\x4b\x41"
      "\x55\x58\xb0\xb4\x3f\x31\x91\x62\x04\x6c\xba\xb8\x22\xe2\xa2\x81\x3d\x9a"
      "\x04\xd5\x49\x48\x47\xb7\xc1\x21\xbe\xc9\x8f\x4d\x28\x52\xf2\xc4\xe8\xbb"
      "\x7c\xd0\xd6\x6b\xb1\xe8\x27\xe7\x18\x91\x8b\xf8\x2d\x07\xd0\xa7\x8d\x65"
      "\xe0\x7c\xc2\xfa\x94\x0f\x7c\x21\xe0\xe2\x6c\xa5\xcc\xf5\x61\xe7\x34\x6b"
      "\x42\x30\x21\x18\xd3\x14\x05\x2c\x86\x5d\x53\x12\xf8\x1e\x77\xb6\xdd\x5e"
      "\xc8\x63\xab\xef\x3b\x9c\x0f\x97\x6c\xf6\x24\x01\x07\x85\xe9\x75\xcd\xcb"
      "\x3c\x4e\x6c\xd8\xe5\x4b\x8f\xaa\x58\x1d\x1d\xe1\x02\xb5\xc1\x7c\xce\x5d"
      "\x5a\x6c\xd4\xbb\x3a\x35\x87\x2d\x3f\xc9\x13\xea\xbc\x59\x00\x16\x5e\x32"
      "\x48\xd3\x37\xba\x98\x0a\x58\x97\xf6\x58\x7a\x32\x49\x6f\xd1\xec\x71\x99"
      "\x2d\x47\x08\x76\x9a\xae\x59\xb7\x1e\xf4\x42\x7b\x45\x58\xe9\x60\xec\x0d"
      "\xf7\xef\xc6\x92\xe4\x95\xb5\xc6\xff\x5b\xe1\xc8\x48\x35\xf3\xe0\x71\x26"
      "\xba\xad\xb5\xce\x8f\xe3\xe8\x98\xc5\x94\x8f\xc0\x62\x8f\x6d\xd3\x1c\xbc"
      "\x43\x4a\x7f\x31\xde\x8a\xb3\x0a\x08\x40\x57\x52\xc9\xd0\x79\xc9\x07\x37"
      "\xbc\xd3\xfb\x78\x8c\x6a\x8e\x44\xbd\x55\x0b\x13\x4c\x36\x9f\x90\x9e\xe3"
      "\x03\x8b\x34\xbe\x4d\xf3\x63\xfa\x60\x91\xe7\x96\x62\x7c\xd0\xf4\xb8\x75"
      "\xa8\x74\xcc\x99\xbd\xf3\x62\x34\x0e\x75\x87\xac\x19\x0a\xdc\x67\x8f\x19"
      "\x79\x8c\xb5\xe3\x77\x1d\x62\x62\x52\xc2\x93\xf4\x31\xc5\x1d\x50\x7e\xc9"
      "\xeb\xd6\x76\x90\x67\x14\xb7\xf6\x03\x95\x5b\x88\xbc\x27\x8d\xe1\xaa\x4b"
      "\x80\x49\x59\x99\x7c\xf3\x49\xd5\x99\x96\x93\xe6\xc0\x56\x48\xdd\x51\x12"
      "\x47\x16\xb0\xba\x7f\x30\xec\x82\x46\x08\x69\x24\x9d\xc4\xba\x2a\xec\x57"
      "\xb5\xe4\x2c\xde\xe3\x69\x70\x85\xef\xdb\x01\xab\xe1\x5a\x52\x9d\x86\x58"
      "\xc7\xde\x16\x34\x0e\x19\x0f\xed\x1e\x25\x29\xd9\xee\x86\x3a\xc0\x44\x53"
      "\x79\x9b\xb5\xb7\x66\x39\x8e\x41\x53\x67\x6c\xea\x2a\xc4\x2a\xdf\x66\x03"
      "\xc3\x43\x22\xb2\x4e\xfa\x68\xb7\x99\x3e\xd3\x51\xd5\x50\x30\x64\x21\x2f"
      "\xc3\xd3\xe6\x36\xa6\x17\xc1\x92\xf9\x86\x85\x2c\x1b\xea\x08\xa9\x09\x89"
      "\xa0\x17\xa3\x11\xea\xdc\xb6\x46\xae\xc2\x54\x02\xcd\x9d\x15\x56\x17\xdb"
      "\x88\xa7\x11\xf2\x6b\x0d\xed\xc9\xe8\x78\xc1\xc0\x1a\x9f\x8e\x2c\x42\x96"
      "\xed\x6b\xcc\xd9\x9c\x61\xe6\x4d\x2e\x2d\xdf\xad\x1b\x83\x09\x86\x63\x6e"
      "\xe2\xcd\xf7\x57\x79\x59\x1f\x18\x56\x82\x74\xce\x71\x20\x66\x8d\x23\xaa"
      "\xe9\x0a\xb5\x9f\xcb\x3c\xfd\xec\xe8\x32\xfc\x5c\x58\x6c\xfa\x71\x62\x74"
      "\x14\xb7\xd1\x08\x6f\x28\x18\xa5\x43\xe4\xea\x86\x3f\xe0\x2c\xfe\xb0\x7f"
      "\xa3\xe8\x5f\x63\x49\x09\xf5\x63\x90\xe6\x01\xd9\x82\xe0\x8f\x95\x73\xf5"
      "\x1f\x54\x4e\xfd\xe0\x2a\x59\xcb\x9b\x0a\x47\x46\x13\xbc\xa5\x4f\x14\xf1"
      "\xcd\x74\x00\xa7\xee\xac\xe3\xfe\x55\xbb\xfb\xe4\xd7\xe9\x3f\x70\x0b\x88"
      "\x0f\xe2\x75\x4b\x83\x6a\x72\xa2\x13\x8f\x04\x83\xa2\x6e\xf9\xe6\x0d\x02"
      "\xe2\xae\x79\x73\xae\xbb\x2c\x0b\xbb\x63\x49\xd7\x97\x7e\x7c\xb8\xc6\x03"
      "\x86\xae\x3f\xe0\x6f\xa1\x2c\x3b\x21\xdf\x94\xab\x8a\x09\x55\xb1\x81\xce"
      "\xd0\xc1\x75\x5a\x7d\x22\x5d\x33\x2e\xef\x29\x88\xd3\xaf\x1a\x4c\xdd\xfe"
      "\xf4\x6c\x30\x4a\xa0\x76\xc5\x47\x18\xdf\x6e\x77\x9d\x9c\xb4\x81\x3f\x6b"
      "\x00\x30\xc7\x2f\xca\x0b\x38\xae\x9c\xab\x07\xc6\x52\xc2\x7a\x87\xc2\xee"
      "\xf5\x98\x33\xf9\x35\xc0\xf2\xec\x92\x78\xc3\x7d\x1f\xaf\x6f\xa2\x59\x50"
      "\x8c\x9f\x09\xc8\x71\x24\xdc\x28\x66\xee\xe0\x47\xab\x0a\x94\x52\x5a\x26"
      "\x36\x9a\x2e\xe3\xc4\x47\x0c\xe6\xd0\xb8\x39\xa0\xc7\x0c\xf4\x24\xfa\xa3"
      "\x35\xbd\x26\x01\xc1\x3a\xd2\xbe\xb1\x08\x78\x42\x90\x6c\xbe\x38\xb3\x4a"
      "\x22\xd9\x70\xe8\x65\x97\x68\x14\x72\xf0\xcc\x59\x58\xb7\x0a\x0e\x3a\x89"
      "\x54\x4b\x0c\xda\x4b\x60\xc9\xd1\xb7\x85",
      4096);
  *(uint64_t*)0x20024ff0 = 0x10;
  *(uint32_t*)0x20024ff8 = 0x11f;
  *(uint32_t*)0x20024ffc = 0;
  syscall(__NR_sendmsg, r[0], 0x20000000, 0x8000);
  *(uint8_t*)0x209a8000 = 1;
  *(uint8_t*)0x209a8001 = 0x80;
  *(uint8_t*)0x209a8002 = 0xc2;
  *(uint8_t*)0x209a8003 = 0;
  *(uint8_t*)0x209a8004 = 0;
  *(uint8_t*)0x209a8005 = 0;
  *(uint8_t*)0x209a8006 = 0xaa;
  *(uint8_t*)0x209a8007 = 0xaa;
  *(uint8_t*)0x209a8008 = 0xaa;
  *(uint8_t*)0x209a8009 = 0xaa;
  *(uint8_t*)0x209a800a = 0;
  *(uint8_t*)0x209a800b = 0xaa;
  *(uint16_t*)0x209a800c = htobe16(0x800);
  STORE_BY_BITMASK(uint8_t, 0x209a800e, 5, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x209a800e, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x209a800f, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x209a800f, 0, 2, 6);
  *(uint16_t*)0x209a8010 = htobe16(0x58);
  *(uint16_t*)0x209a8012 = 0;
  *(uint16_t*)0x209a8014 = htobe16(0);
  *(uint8_t*)0x209a8016 = 0;
  *(uint8_t*)0x209a8017 = 1;
  *(uint16_t*)0x209a8018 = 0;
  *(uint8_t*)0x209a801a = 0xac;
  *(uint8_t*)0x209a801b = 0x14;
  *(uint8_t*)0x209a801c = 0;
  *(uint8_t*)0x209a801d = 0xbb;
  *(uint8_t*)0x209a801e = 0xac;
  *(uint8_t*)0x209a801f = 0x14;
  *(uint8_t*)0x209a8020 = 0;
  *(uint8_t*)0x209a8021 = 0x13;
  *(uint8_t*)0x209a8022 = 3;
  *(uint8_t*)0x209a8023 = 4;
  *(uint16_t*)0x209a8024 = 0;
  *(uint8_t*)0x209a8026 = 0;
  *(uint8_t*)0x209a8027 = 0;
  *(uint16_t*)0x209a8028 = htobe16(4);
  STORE_BY_BITMASK(uint8_t, 0x209a802a, 0xf, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x209a802a, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x209a802b, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x209a802b, 0, 2, 6);
  *(uint16_t*)0x209a802c = htobe16(0);
  *(uint16_t*)0x209a802e = 0;
  *(uint16_t*)0x209a8030 = htobe16(0);
  *(uint8_t*)0x209a8032 = 0;
  *(uint8_t*)0x209a8033 = 1;
  *(uint16_t*)0x209a8034 = htobe16(0);
  *(uint32_t*)0x209a8036 = htobe32(0);
  *(uint8_t*)0x209a803a = 0xac;
  *(uint8_t*)0x209a803b = 0x14;
  *(uint8_t*)0x209a803c = 0;
  *(uint8_t*)0x209a803d = 0x10;
  *(uint8_t*)0x209a803e = 0x94;
  *(uint8_t*)0x209a803f = 6;
  *(uint32_t*)0x209a8040 = htobe32(0x3800000);
  *(uint8_t*)0x209a8044 = 0x44;
  *(uint8_t*)0x209a8045 = 0x10;
  *(uint8_t*)0x209a8046 = 0;
  STORE_BY_BITMASK(uint8_t, 0x209a8047, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x209a8047, 0, 4, 4);
  *(uint32_t*)0x209a8048 = htobe32(0);
  *(uint8_t*)0x209a804c = 0xac;
  *(uint8_t*)0x209a804d = 0x14;
  *(uint8_t*)0x209a804e = 0;
  *(uint8_t*)0x209a804f = 0xaa;
  *(uint32_t*)0x209a8050 = htobe32(0);
  *(uint8_t*)0x209a8054 = 1;
  *(uint8_t*)0x209a8055 = 0x83;
  *(uint8_t*)0x209a8056 = 0xf;
  *(uint8_t*)0x209a8057 = 0;
  *(uint32_t*)0x209a8058 = htobe32(0);
  *(uint32_t*)0x209a805c = htobe32(0);
  *(uint32_t*)0x209a8060 = htobe32(0);
  *(uint32_t*)0x2048fff4 = 0;
  *(uint32_t*)0x2048fff8 = 1;
  *(uint32_t*)0x2048fffc = 0;
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x209a8022, 68);
  *(uint16_t*)0x209a8024 = csum_inet_digest(&csum_1);
  struct csum_inet csum_2;
  csum_inet_init(&csum_2);
  csum_inet_update(&csum_2, (const uint8_t*)0x209a800e, 20);
  *(uint16_t*)0x209a8018 = csum_inet_digest(&csum_2);
  syz_emit_ethernet(0x66, 0x209a8000, 0x2048fff4);
}

int main()
{
  int pid = do_sandbox_none(0, true);
  int status = 0;
  while (waitpid(pid, &status, __WALL) != pid) {
  }
  return 0;
}
