// WARNING in strp_done
// https://syzkaller.appspot.com/bug?id=253324b496b749185232dff7c28a7ffb42c377aa
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
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

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    while (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE))
      syscall(SYS_futex, &th->running, FUTEX_WAIT, 0, 0);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    __atomic_store_n(&th->running, 0, __ATOMIC_RELEASE);
    syscall(SYS_futex, &th->running, FUTEX_WAKE);
  }
  return 0;
}

static void execute(int num_calls)
{
  int call, thread;
  running = 0;
  for (call = 0; call < num_calls; call++) {
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 128 << 10);
        pthread_create(&th->th, &attr, thr, th);
      }
      if (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE)) {
        th->call = call;
        __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
        __atomic_store_n(&th->running, 1, __ATOMIC_RELEASE);
        syscall(SYS_futex, &th->running, FUTEX_WAKE);
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (running)
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

long r[3];
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
    break;
  case 1:
    r[0] = syscall(__NR_socket, 0xa, 1, 0);
    break;
  case 2:
    *(uint16_t*)0x20f09fe4 = 0xa;
    *(uint16_t*)0x20f09fe6 = 0;
    *(uint32_t*)0x20f09fe8 = 0;
    *(uint8_t*)0x20f09fec = 0xfe;
    *(uint8_t*)0x20f09fed = 0x80;
    *(uint8_t*)0x20f09fee = 0;
    *(uint8_t*)0x20f09fef = 0;
    *(uint8_t*)0x20f09ff0 = 0;
    *(uint8_t*)0x20f09ff1 = 0;
    *(uint8_t*)0x20f09ff2 = 0;
    *(uint8_t*)0x20f09ff3 = 0;
    *(uint8_t*)0x20f09ff4 = 0;
    *(uint8_t*)0x20f09ff5 = 0;
    *(uint8_t*)0x20f09ff6 = 0;
    *(uint8_t*)0x20f09ff7 = 0;
    *(uint8_t*)0x20f09ff8 = 0;
    *(uint8_t*)0x20f09ff9 = 0;
    *(uint8_t*)0x20f09ffa = 0;
    *(uint8_t*)0x20f09ffb = 0xbb;
    *(uint32_t*)0x20f09ffc = 7;
    syscall(__NR_connect, r[0], 0x20f09fe4, 0x1c);
    break;
  case 3:
    *(uint32_t*)0x20bef000 = 1;
    *(uint32_t*)0x20bef004 = 3;
    *(uint64_t*)0x20bef008 = 0x209ff000;
    *(uint64_t*)0x20bef010 = 0x20849000;
    *(uint32_t*)0x20bef018 = 4;
    *(uint32_t*)0x20bef01c = 0xb7;
    *(uint64_t*)0x20bef020 = 0x2079d000;
    *(uint32_t*)0x20bef028 = 0;
    *(uint32_t*)0x20bef02c = 0;
    *(uint8_t*)0x20bef030 = 0;
    *(uint8_t*)0x20bef031 = 0;
    *(uint8_t*)0x20bef032 = 0;
    *(uint8_t*)0x20bef033 = 0;
    *(uint8_t*)0x20bef034 = 0;
    *(uint8_t*)0x20bef035 = 0;
    *(uint8_t*)0x20bef036 = 0;
    *(uint8_t*)0x20bef037 = 0;
    *(uint8_t*)0x20bef038 = 0;
    *(uint8_t*)0x20bef039 = 0;
    *(uint8_t*)0x20bef03a = 0;
    *(uint8_t*)0x20bef03b = 0;
    *(uint8_t*)0x20bef03c = 0;
    *(uint8_t*)0x20bef03d = 0;
    *(uint8_t*)0x20bef03e = 0;
    *(uint8_t*)0x20bef03f = 0;
    *(uint32_t*)0x20bef040 = 0;
    *(uint8_t*)0x209ff000 = 0x18;
    STORE_BY_BITMASK(uint8_t, 0x209ff001, 0, 0, 4);
    STORE_BY_BITMASK(uint8_t, 0x209ff001, 0, 4, 4);
    *(uint16_t*)0x209ff002 = 0;
    *(uint32_t*)0x209ff004 = 0;
    *(uint8_t*)0x209ff008 = 0;
    *(uint8_t*)0x209ff009 = 0;
    *(uint16_t*)0x209ff00a = 0;
    *(uint32_t*)0x209ff00c = 0;
    *(uint8_t*)0x209ff010 = 0x95;
    *(uint8_t*)0x209ff011 = 0;
    *(uint16_t*)0x209ff012 = 0;
    *(uint32_t*)0x209ff014 = 0;
    memcpy((void*)0x20849000, "syzkaller", 10);
    r[1] = syscall(__NR_bpf, 5, 0x20bef000, 0x48);
    break;
  case 4:
    r[2] = syscall(__NR_socket, 0x29, 2, 0);
    break;
  case 5:
    *(uint32_t*)0x20954ff8 = r[0];
    *(uint32_t*)0x20954ffc = r[1];
    syscall(__NR_ioctl, r[2], 0x89e0, 0x20954ff8);
    break;
  case 6:
    *(uint32_t*)0x20ed6ff8 = r[0];
    *(uint32_t*)0x20ed6ffc = r[1];
    syscall(__NR_ioctl, r[2], 0x89e0, 0x20ed6ff8);
    break;
  case 7:
    *(uint8_t*)0x20eeecaa = 1;
    *(uint8_t*)0x20eeecab = 0x80;
    *(uint8_t*)0x20eeecac = 0xc2;
    *(uint8_t*)0x20eeecad = 0;
    *(uint8_t*)0x20eeecae = 0;
    *(uint8_t*)0x20eeecaf = 0;
    memcpy((void*)0x20eeecb0, "\xec\x35\x0f\xf6\x48\x81", 6);
    *(uint16_t*)0x20eeecb6 = htobe16(0x86dd);
    STORE_BY_BITMASK(uint8_t, 0x20eeecb8, 0, 0, 4);
    STORE_BY_BITMASK(uint8_t, 0x20eeecb8, 6, 4, 4);
    memcpy((void*)0x20eeecb9, "\x05\x01\x01", 3);
    *(uint16_t*)0x20eeecbc = htobe16(3);
    *(uint8_t*)0x20eeecbe = 0;
    *(uint8_t*)0x20eeecbf = 0;
    *(uint8_t*)0x20eeecc0 = 0;
    *(uint8_t*)0x20eeecc1 = 0;
    *(uint8_t*)0x20eeecc2 = 0;
    *(uint8_t*)0x20eeecc3 = 0;
    *(uint8_t*)0x20eeecc4 = 0;
    *(uint8_t*)0x20eeecc5 = 0;
    *(uint8_t*)0x20eeecc6 = 0;
    *(uint8_t*)0x20eeecc7 = 0;
    *(uint8_t*)0x20eeecc8 = 0;
    *(uint8_t*)0x20eeecc9 = 0;
    *(uint8_t*)0x20eeecca = -1;
    *(uint8_t*)0x20eeeccb = -1;
    *(uint32_t*)0x20eeeccc = htobe32(0xe0000002);
    *(uint8_t*)0x20eeecd0 = -1;
    *(uint8_t*)0x20eeecd1 = 2;
    *(uint8_t*)0x20eeecd2 = 0;
    *(uint8_t*)0x20eeecd3 = 0;
    *(uint8_t*)0x20eeecd4 = 0;
    *(uint8_t*)0x20eeecd5 = 0;
    *(uint8_t*)0x20eeecd6 = 0;
    *(uint8_t*)0x20eeecd7 = 0;
    *(uint8_t*)0x20eeecd8 = 0;
    *(uint8_t*)0x20eeecd9 = 0;
    *(uint8_t*)0x20eeecda = 0;
    *(uint8_t*)0x20eeecdb = 0;
    *(uint8_t*)0x20eeecdc = 0;
    *(uint8_t*)0x20eeecdd = 0;
    *(uint8_t*)0x20eeecde = 0;
    *(uint8_t*)0x20eeecdf = 1;
    STORE_BY_BITMASK(uint16_t, 0x20eeece0, 0, 0, 1);
    STORE_BY_BITMASK(uint16_t, 0x20eeece0, 0, 1, 1);
    STORE_BY_BITMASK(uint16_t, 0x20eeece0, 1, 2, 1);
    STORE_BY_BITMASK(uint16_t, 0x20eeece0, 0, 3, 1);
    STORE_BY_BITMASK(uint16_t, 0x20eeece0, 0, 4, 4);
    STORE_BY_BITMASK(uint16_t, 0x20eeece0, 0, 8, 1);
    STORE_BY_BITMASK(uint16_t, 0x20eeece0, 0, 9, 4);
    STORE_BY_BITMASK(uint16_t, 0x20eeece0, 1, 13, 3);
    *(uint16_t*)0x20eeece2 = htobe16(0x880b);
    *(uint16_t*)0x20eeece4 = htobe16(0);
    *(uint16_t*)0x20eeece6 = htobe16(0);
    STORE_BY_BITMASK(uint16_t, 0x20eeece8, 0, 0, 1);
    STORE_BY_BITMASK(uint16_t, 0x20eeece8, 0, 1, 1);
    STORE_BY_BITMASK(uint16_t, 0x20eeece8, 0, 2, 1);
    STORE_BY_BITMASK(uint16_t, 0x20eeece8, 0, 3, 1);
    STORE_BY_BITMASK(uint16_t, 0x20eeece8, 0, 4, 9);
    STORE_BY_BITMASK(uint16_t, 0x20eeece8, 0, 13, 3);
    *(uint16_t*)0x20eeecea = htobe16(0);
    STORE_BY_BITMASK(uint16_t, 0x20eeecec, 0, 0, 1);
    STORE_BY_BITMASK(uint16_t, 0x20eeecec, 0, 1, 1);
    STORE_BY_BITMASK(uint16_t, 0x20eeecec, 0, 2, 1);
    STORE_BY_BITMASK(uint16_t, 0x20eeecec, 0, 3, 1);
    STORE_BY_BITMASK(uint16_t, 0x20eeecec, 0, 4, 9);
    STORE_BY_BITMASK(uint16_t, 0x20eeecec, 0, 13, 3);
    *(uint16_t*)0x20eeecee = htobe16(0);
    *(uint32_t*)0x20efe000 = 0;
    *(uint32_t*)0x20efe004 = 1;
    *(uint32_t*)0x20efe008 = 0xf89;
    syz_emit_ethernet(0x3e, 0x20eeecaa, 0x20efe000);
    break;
  }
}

void loop()
{
  memset(r, -1, sizeof(r));
  execute(8);
}

int main()
{
  int pid = do_sandbox_none(0, true);
  int status = 0;
  while (waitpid(pid, &status, __WALL) != pid) {
  }
  return 0;
}
