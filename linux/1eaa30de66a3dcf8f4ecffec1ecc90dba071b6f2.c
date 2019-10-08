// possible deadlock in sch_direct_xmit
// https://syzkaller.appspot.com/bug?id=1eaa30de66a3dcf8f4ecffec1ecc90dba071b6f2
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
  rlim.rlim_cur = rlim.rlim_max = 160 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
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
  loop();
  doexit(1);
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 0x11, 0x800000000002, 0x300);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x2095bffc = 0;
  *(uint8_t*)0x2095bffe = 0;
  *(uint8_t*)0x2095bfff = 0xfc;
  syscall(__NR_setsockopt, r[0], 0x107, 0x12, 0x2095bffc, 4);
  res = syscall(__NR_socket, 2, 2, 0);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x20000040 = 2;
  *(uint16_t*)0x20000042 = htobe16(0);
  *(uint8_t*)0x20000044 = 0xac;
  *(uint8_t*)0x20000045 = 0x14;
  *(uint8_t*)0x20000046 = 0x14;
  *(uint8_t*)0x20000047 = 0xaa;
  *(uint8_t*)0x20000048 = 0;
  *(uint8_t*)0x20000049 = 0;
  *(uint8_t*)0x2000004a = 0;
  *(uint8_t*)0x2000004b = 0;
  *(uint8_t*)0x2000004c = 0;
  *(uint8_t*)0x2000004d = 0;
  *(uint8_t*)0x2000004e = 0;
  *(uint8_t*)0x2000004f = 0;
  syscall(__NR_connect, r[1], 0x20000040, 0x10);
  *(uint16_t*)0x2057bff0 = 2;
  *(uint16_t*)0x2057bff2 = htobe16(0x4e20);
  *(uint32_t*)0x2057bff4 = htobe32(0xe0000001);
  *(uint8_t*)0x2057bff8 = 0;
  *(uint8_t*)0x2057bff9 = 0;
  *(uint8_t*)0x2057bffa = 0;
  *(uint8_t*)0x2057bffb = 0;
  *(uint8_t*)0x2057bffc = 0;
  *(uint8_t*)0x2057bffd = 0;
  *(uint8_t*)0x2057bffe = 0;
  *(uint8_t*)0x2057bfff = 0;
  syscall(__NR_sendto, r[1], 0x20762fff, 0xfdc7, 0, 0x2057bff0, 0x10);
  *(uint8_t*)0x20000080 = 0xaa;
  *(uint8_t*)0x20000081 = 0xaa;
  *(uint8_t*)0x20000082 = 0xaa;
  *(uint8_t*)0x20000083 = 0xaa;
  *(uint8_t*)0x20000084 = 0xaa;
  *(uint8_t*)0x20000085 = 0xaa;
  *(uint8_t*)0x20000086 = 0xaa;
  *(uint8_t*)0x20000087 = 0xaa;
  *(uint8_t*)0x20000088 = 0xaa;
  *(uint8_t*)0x20000089 = 0xaa;
  *(uint8_t*)0x2000008a = 0xaa;
  *(uint8_t*)0x2000008b = 0xaa;
  *(uint16_t*)0x2000008c = htobe16(0x86dd);
  STORE_BY_BITMASK(uint8_t, 0x2000008e, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x2000008e, 6, 4, 4);
  memcpy((void*)0x2000008f, "\xdc\xd5\x0f", 3);
  *(uint16_t*)0x20000092 = htobe16(0xb);
  *(uint8_t*)0x20000094 = 0x2c;
  *(uint8_t*)0x20000095 = 0;
  *(uint8_t*)0x20000096 = 0xfe;
  *(uint8_t*)0x20000097 = 0x80;
  *(uint8_t*)0x20000098 = 0;
  *(uint8_t*)0x20000099 = 0;
  *(uint8_t*)0x2000009a = 0;
  *(uint8_t*)0x2000009b = 0;
  *(uint8_t*)0x2000009c = 0;
  *(uint8_t*)0x2000009d = 0;
  *(uint8_t*)0x2000009e = 0xc0;
  *(uint8_t*)0x2000009f = 0;
  *(uint8_t*)0x200000a0 = 0xe;
  *(uint8_t*)0x200000a1 = 0;
  *(uint8_t*)0x200000a2 = 0;
  *(uint8_t*)0x200000a3 = 0;
  *(uint8_t*)0x200000a4 = 0;
  *(uint8_t*)0x200000a5 = 0xaa;
  *(uint8_t*)0x200000a6 = 0xfe;
  *(uint8_t*)0x200000a7 = 0x80;
  *(uint8_t*)0x200000a8 = 0;
  *(uint8_t*)0x200000a9 = 0;
  *(uint8_t*)0x200000aa = 0;
  *(uint8_t*)0x200000ab = 0;
  *(uint8_t*)0x200000ac = 0;
  *(uint8_t*)0x200000ad = 0;
  *(uint8_t*)0x200000ae = 0;
  *(uint8_t*)0x200000af = 0;
  *(uint8_t*)0x200000b0 = 0;
  *(uint8_t*)0x200000b1 = 0;
  *(uint8_t*)0x200000b2 = 0;
  *(uint8_t*)0x200000b3 = 0;
  *(uint8_t*)0x200000b4 = 0;
  *(uint8_t*)0x200000b5 = 0xaa;
  *(uint8_t*)0x200000b6 = 0x81;
  *(uint8_t*)0x200000b7 = 0;
  *(uint16_t*)0x200000b8 = 0;
  *(uint16_t*)0x200000ba = htobe16(0);
  *(uint16_t*)0x200000bc = htobe16(0);
  memcpy((void*)0x200000be, ":Jw", 3);
  *(uint32_t*)0x20000340 = 0;
  *(uint32_t*)0x20000344 = 0;
  *(uint32_t*)0x20000348 = 0;
  *(uint32_t*)0x2000034c = 0;
  *(uint32_t*)0x20000350 = 0;
  *(uint32_t*)0x20000354 = 0;
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x20000096, 16);
  csum_inet_update(&csum_1, (const uint8_t*)0x200000a6, 16);
  uint32_t csum_1_chunk_2 = 0xb000000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_2, 4);
  uint32_t csum_1_chunk_3 = 0x3a000000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_3, 4);
  csum_inet_update(&csum_1, (const uint8_t*)0x200000b6, 11);
  *(uint16_t*)0x200000b8 = csum_inet_digest(&csum_1);
  syz_emit_ethernet(0x41, 0x20000080, 0x20000340);
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
