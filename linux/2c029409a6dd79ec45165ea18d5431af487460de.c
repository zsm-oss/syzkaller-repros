// WARNING: kmalloc bug in get_valid_checkpoint
// https://syzkaller.appspot.com/bug?id=2c029409a6dd79ec45165ea18d5431af487460de
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/loop.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
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
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
  const char* devtypes[] = {"ip6gretap", "bridge", "vcan", "bond", "team"};
  const char* devnames[] = {"lo",
                            "sit0",
                            "bridge0",
                            "vcan0",
                            "tunl0",
                            "gre0",
                            "gretap0",
                            "ip_vti0",
                            "ip6_vti0",
                            "ip6tnl0",
                            "ip6gre0",
                            "ip6gretap0",
                            "erspan0",
                            "bond0",
                            "veth0",
                            "veth1",
                            "team0",
                            "veth0_to_bridge",
                            "veth1_to_bridge",
                            "veth0_to_bond",
                            "veth1_to_bond",
                            "veth0_to_team",
                            "veth1_to_team"};
  const char* devmasters[] = {"bridge", "bond", "team"};

  for (i = 0; i < sizeof(devtypes) / (sizeof(devtypes[0])); i++)
    execute_command(0, "ip link add dev %s0 type %s", devtypes[i], devtypes[i]);
  execute_command(0, "ip link add type veth");

  for (i = 0; i < sizeof(devmasters) / (sizeof(devmasters[0])); i++) {
    execute_command(
        0, "ip link add name %s_slave_0 type veth peer name veth0_to_%s",
        devmasters[i], devmasters[i]);
    execute_command(
        0, "ip link add name %s_slave_1 type veth peer name veth1_to_%s",
        devmasters[i], devmasters[i]);
    execute_command(0, "ip link set %s_slave_0 master %s0", devmasters[i],
                    devmasters[i]);
    execute_command(0, "ip link set %s_slave_1 master %s0", devmasters[i],
                    devmasters[i]);
    execute_command(0, "ip link set veth0_to_%s up", devmasters[i]);
    execute_command(0, "ip link set veth1_to_%s up", devmasters[i]);
  }
  execute_command(0, "ip link set bridge_slave_0 up");
  execute_command(0, "ip link set bridge_slave_1 up");

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

extern unsigned long long procid;

struct fs_image_segment {
  void* data;
  uintptr_t size;
  uintptr_t offset;
};

#define IMAGE_MAX_SEGMENTS 4096
#define IMAGE_MAX_SIZE (129 << 20)

#define SYZ_memfd_create 319

static uintptr_t syz_mount_image(uintptr_t fs, uintptr_t dir, uintptr_t size,
                                 uintptr_t nsegs, uintptr_t segments,
                                 uintptr_t flags, uintptr_t opts)
{
  char loopname[64];
  int loopfd, err = 0, res = -1;
  uintptr_t i;
  struct fs_image_segment* segs = (struct fs_image_segment*)segments;

  if (nsegs > IMAGE_MAX_SEGMENTS)
    nsegs = IMAGE_MAX_SEGMENTS;
  for (i = 0; i < nsegs; i++) {
    if (segs[i].size > IMAGE_MAX_SIZE)
      segs[i].size = IMAGE_MAX_SIZE;
    segs[i].offset %= IMAGE_MAX_SIZE;
    if (segs[i].offset > IMAGE_MAX_SIZE - segs[i].size)
      segs[i].offset = IMAGE_MAX_SIZE - segs[i].size;
    if (size < segs[i].offset + segs[i].offset)
      size = segs[i].offset + segs[i].offset;
  }
  if (size > IMAGE_MAX_SIZE)
    size = IMAGE_MAX_SIZE;
  int memfd = syscall(SYZ_memfd_create, "syz_mount_image", 0);
  if (memfd == -1) {
    err = errno;
    goto error;
  }
  if (ftruncate(memfd, size)) {
    err = errno;
    goto error_close_memfd;
  }
  for (i = 0; i < nsegs; i++) {
    if (pwrite(memfd, segs[i].data, segs[i].size, segs[i].offset) < 0) {
    }
  }
  snprintf(loopname, sizeof(loopname), "/dev/loop%llu", procid);
  loopfd = open(loopname, O_RDWR);
  if (loopfd == -1) {
    err = errno;
    goto error_close_memfd;
  }
  if (ioctl(loopfd, LOOP_SET_FD, memfd)) {
    if (errno != EBUSY) {
      err = errno;
      goto error_close_loop;
    }
    ioctl(loopfd, LOOP_CLR_FD, 0);
    usleep(1000);
    if (ioctl(loopfd, LOOP_SET_FD, memfd)) {
      err = errno;
      goto error_close_loop;
    }
  }
  mkdir((char*)dir, 0777);
  NONFAILING(if (strcmp((char*)fs, "iso9660") == 0) flags |= MS_RDONLY);
  if (mount(loopname, (char*)dir, (char*)fs, flags, (char*)opts)) {
    err = errno;
    goto error_clear_loop;
  }
  res = 0;
error_clear_loop:
  ioctl(loopfd, LOOP_CLR_FD, 0);
error_close_loop:
  close(loopfd);
error_close_memfd:
  close(memfd);
error:
  errno = err;
  return res;
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
  initialize_netdevices();

  loop();
  doexit(1);
}

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;
static int collide;

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
        if (collide && call % 2)
          break;
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

unsigned long long procid;
void execute_call(int call)
{
  switch (call) {
  case 0:
    NONFAILING(memcpy((void*)0x20000040, "f2fs", 5));
    NONFAILING(memcpy((void*)0x20000080, "./file0", 8));
    NONFAILING(*(uint64_t*)0x20000200 = 0x20000140);
    NONFAILING(
        memcpy((void*)0x20000140,
               "\x10\x20\xf5\xf2\x01\x00\x07\x00\x09\x00\x00\x00\x03\x00\x00"
               "\x00\x0c\x00\x00\x00\x09\x00\x00\x00\x02\x00\x00\x00\x01\x00"
               "\x00\x00\x00\x00\x00\x00\x00\x48\x00\x00\x00\x00\x00\x00\x0d"
               "\x00\x00\x00\x22\x00\x00\x00\x02\x00\x00\x00\x02\x00\x00\x00"
               "\x02\x00\x00\x00\x02\x00\x00\x00\x1a\x00\x00\x00\x00\x04\x00"
               "\x00\x00\x04\x00\x00\x00\x08\x00\x00\x00\x0c\x00\x00\x00\x10"
               "\x00\x00\x00\x14\x00\x00\x03\x00\x00\x00\x01\x00\x00\x00\x02",
               105));
    NONFAILING(*(uint64_t*)0x20000208 = 0x69);
    NONFAILING(*(uint64_t*)0x20000210 = 0x1400);
    NONFAILING(*(uint64_t*)0x200000c0 = 0x20001300);
    NONFAILING(*(uint64_t*)0x20001300 = 0x20000240);
    NONFAILING(*(uint64_t*)0x20000240 = 0);
    NONFAILING(*(uint16_t*)0x20000248 = -1);
    NONFAILING(*(uint64_t*)0x2000024a = -1);
    NONFAILING(*(uint64_t*)0x20000252 = 0);
    NONFAILING(*(uint64_t*)0x2000025a = 0);
    NONFAILING(*(uint64_t*)0x20000262 = 0);
    NONFAILING(*(uint64_t*)0x2000026a = 0);
    NONFAILING(*(uint64_t*)0x20000272 = -1);
    NONFAILING(*(uint64_t*)0x2000027a = 0);
    NONFAILING(*(uint64_t*)0x20001308 = 0);
    NONFAILING(*(uint64_t*)0x20001310 = -1);
    NONFAILING(*(uint64_t*)0x20001318 = 0x200002c0);
    NONFAILING(memcpy(
        (void*)0x200002c0,
        "\x27\xd5\x2b\xeb\x73\x2e\x56\x25\x34\x6d\x2e\x8f\x4c\x19\xf7\x55\x41"
        "\xe9\x00\x36\xeb\xc4\x91\x02\xd3\xcc\x58\xe5\xd5\x9e\x14\x1a\x74\x84"
        "\x06\x87\xc2\x40\x8c\xb1\x90\x39\xb3\x9c\xf9\x86\xeb\x60\xce\xfc\xca"
        "\x9d\xee\x60\xd9\xe7\x1c\xb9\xa2\x72\x57\xeb\xd7\x81\x00\x87\x6b\xc2"
        "\x13\x63\x77\x46\xb3\x5c\xaf\x73\x51\x94\x1e\x47\x18\x8d\x08\x38\x71"
        "\x6c\x4c\xcb\x37\xe5\x71\x0e\x1f\x4a\xa7\x89\x5b\xac\x0b\xf0\xcb\x4a"
        "\xab\x64\xa7\x41\x72\x4c\x49\xc6\x6f\x05\x04\x8d\x07\xa0\xd0\x42\xfd"
        "\x6e\x38\x18\x53\x94\xd4\xb6\x6b\x87\x53\x64\x0f\xb1\xe2\xb4\xe1\x3e"
        "\x70\x7c\x02\xe1\x9a\x31\xee\x0f\xd9\x7a\xae\x68\x6b\x30\xa6\xb2\x71"
        "\xb6\x31\x6b\xe8\xac\x4f\x99\x9d\xe6\x81\xf5\xe1\xd1\xe9\xb4\x25\xe4"
        "\x2f\x78\xa9\x4a\x87\x02\xca\x41\x3b\x16\x4a\x5a\x85\xa8\xf6\x08\x86"
        "\x68\xe6\x68\xe0\x15\x27\xac\xdf\x78\x96\x29\xc8\x18\xfd\xea\x06\x9b"
        "\x89\xf9\x0f\x8e\x28\x31\x97\xa5\xe8\x36\x6b\xd6\xfa\x1f\x81\xd3\x69"
        "\x54\x82\x42\x54\x3f\x9d\xc7\x6c\xe9\x80\x52\x03\x8f\x4d\x43\x6f\xa0"
        "\xad\x3e\x65\x42\xe7\xf1\xce\x77\x8c\xee\x7d\x2b\x49\xc4\xb8\x37\x0a"
        "\xf9\xe6\x30\x4b\xa8\x63\x47\x13\x28\xe3\x1c\xa9\x96\x3c\x07\x52\x5a"
        "\xb7\x1b\x10\x1c\x6b\xb2\x4a\xc4\x58\x2b\x27\x57\xf7\xea\xce\xa7\x0a"
        "\x07\x66\xe2\x0b\xa2\x02\xdf\x27\x4f\xf0\xe0\x93\x65\x6e\xfa\xf1\x51"
        "\xa7\x9b\x6d\x33\x60\x58\xee\x72\xeb\x6b\x2c\xcc\x07\xb3\x02\xfd\xc9"
        "\x9c\xb3\x18\x38\x49\x27\x3b\xe7\x82\x34\x59\xf0\x86\x78\x4e\x46\x5c"
        "\x81\x1f\xc6\x23\xe0\xc4\xbb\x15\x7e\x2c\xee\xbf\xc7\x2c\x95\x5e\x30"
        "\xc4\x4b\x16\xc3\xda\xb7\xe8\xca\xd4\x00\x57\x7e\xd4\x55\xe0\x9f\x55"
        "\x92\x3a\xfc\x5c\x96\x27\xcc\x9d\x12\x07\xe3\xc4\xfb\x34\x5a\xc0\x70"
        "\x37\xc0\xe2\xa1\x7e\xdb\xaf\xfb\xab\x07\x4b\x75\xa1\x57\x9a\xaa\xb9"
        "\x10\x74\x08\x6f\xf3\x58\xa3\xf7\x5f\x3c\x5a\x4c\x2c\xbf\x2c\xaf\x75"
        "\x75\x74\xb0\x6d\x6f\x49\x72\xa3\x99\xc5\xa1\xb4\x61\x24\xe4\xda\x4c"
        "\xf5\x26\x6a\x71\x44\x34\x8c\x95\xe7\xa5\x34\xea\xac\x72\x6f\xf5\x75"
        "\x20\xc4\x2d\xf4\x81\x4a\x8b\x20\x93\xbe\x92\xe3\x10\x01\xf7\x66\xb6"
        "\xa7\x8c\x98\xaa\x30\x1f\xd4\x98\x4b\xfe\x97\xf5\xfe\x32\x9b\xfa\x24"
        "\x5a\xa0\xbb\xa6\x98\xde\x88\x63\x3a\x1a\xc2\xb7\x6c\x85\xdd\x9e\x46"
        "\x1b\x00\x34\xde\x81\x7c\xea\x29\xc9\xb1\x9e\xff\xc9\x6e\xd4\xc0\x2c"
        "\xf0\xe1\x1c\x13\x69\x2a\x29\x09\xad\xc0\x47\x65\xc5\x0a\x0f\x4e\x67"
        "\x8c\x15\x2e\x01\xf8\x5b\x14\xd8\x02\xae\xce\xf1\x6b\x63\xac\x15\x84"
        "\x44\x2b\x5c\x5a\x9b\x88\x1e\xd5\xfa\x8f\x40\xd7\x77\x2f\xe4\x2f\x63"
        "\x4f\x66\xa0\x31\xe5\xdb\xc0\x58\xc6\x2a\xcc\xc6\xd7\xb5\x64\x86\x8b"
        "\x62\xbf\x8a\xf3\x33\xec\x32\x8e\xd1\x81\x2e\x2d\x17\x5b\xa4\x39\xb9"
        "\x8b\xd8\x44\xfb\x91\x0f\x53\x08\x2e\x76\x82\x2f\xa9\xc8\x62\x81\x22"
        "\xc2\xc7\x1e\xe3\x1c\xc0\x8d\x86\xde\xef\x74\xdb\x84\x5c\x18\x21\x2b"
        "\x06\x23\x74\xaf\xf8\x90\xad\xb8\xfd\xe1\x04\xae\x61\xe9\x95\x34\xa7"
        "\x8d\xf1\x62\x55\xf9\x6a\xb2\x30\xfb\x33\x73\x0d\x88\x05\xcf\xe1\x5d"
        "\x16\xc0\xe7\x59\x04\x12\xa0\xbc\x85\x68\x74\xcb\xb9\x78\x8a\x63\x94"
        "\xe7\x83\x93\xfb\x15\xf8\xa5\x1e\xae\x89\x90\xeb\x5f\xc5\xcb\x26\x4d"
        "\x98\x3e\xec\xd9\x76\x49\xc2\x93\x35\xb7\xe8\x32\x58\xe5\x8a\xc3\x89"
        "\x49\xd7\x30\xb4\x47\xe5\x5a\x3d\x03\x47\xa0\xd0\x54\x8d\x03\xd9\x00"
        "\xe8\x96\x9b\x72\x24\xdf\x87\x40\xc8\xf2\xb0\x08\xac\x30\x9e\x83\xa6"
        "\x73\xf3\x3e\xa1\x27\x8d\xbf\x59\x4d\xee\x3f\xa6\xe6\xe5\x2f\x27\x25"
        "\x48\xe6\x41\x70\xf5\xcc\xd6\x70\xd2\xbf\xba\x24\x68\xb0\xd6\xb4\xd6"
        "\x53\x9c\xc1\x94\xaa\x92\xf3\x41\x06\x98\xc7\xe2\xef\xbf\x1a\x9e\x76"
        "\x98\x23\xbb\x17\x38\x64\x32\x0e\xf8\x83\x82\x37\x45\xee\xa5\x01\x55"
        "\x21\x0c\x97\x45\xdb\xed\x8c\xae\x23\x3e\xfe\xe0\x84\xb6\x95\x27\x17"
        "\x54\x1d\xdd\x2d\x20\x63\x07\x0a\x1d\x7e\x61\xd6\xc0\xbe\xe5\xaf\x2f"
        "\xef\xaa\xd3\xb3\xd7\x50\x96\xc9\xca\xd9\xd3\x88\xe4\x33\x6c\xc1\xa5"
        "\x6c\xd5\x9d\x9d\x26\xb9\x67\x4a\xf3\xfc\x1d\x8c\x37\x10\xab\x6d\x23"
        "\x25\xa2\xee\x6e\xd3\xfd\x54\xf2\xf5\xd8\xe0\xc6\x82\x20\xc9\x4b\x81"
        "\x83\xf5\x95\x60\x1a\x25\x5d\xb5\x6b\x00\xdf\xcd\x4a\x15\x01\xce\x7d"
        "\xce\xc9\xd8\xe4\x89\x53\x5a\x39\xa8\x49\x79\xd0\x8f\xa7\xcc\xf3\xc2"
        "\x21\xd2\xd5\x66\x86\xf2\xd1\x67\xe8\x1b\xc6\xa4\x90\x04\x8f\x4e\x3e"
        "\x4b\xcf\x4d\x67\xfc\x6c\x76\x79\x73\x8f\x28\xe5\x7d\x9a\xc1\x27\xbd"
        "\x74\x65\xdb\xff\xe1\x59\x0e\xbd\xc8\x90\x77\x59\x69\x18\xc8\x2c\x42"
        "\x65\xf8\xd2\xdc\xbc\x93\x5b\xfc\x94\x9e\xf1\x59\xe1\xc8\x96\x73\xcc"
        "\x58\x8b\x8d\xfa\xae\x7e\x2b\xb4\xff\xf5\xa1\x26\x7d\x20\xe1\x0c\xf2"
        "\xf0\xe6\x97\xd8\x67\xc8\x41\x21\x7d\x31\xc9\x8c\x9b\x0d\x4f\xa0\x6c"
        "\x5a\xd4\x7b\x28\x16\x29\xb6\x83\xe2\x41\xb5\x2f\x45\x2c\xdd\xcb\x36"
        "\x21\xdf\xbc\x54\x53\x3c\x76\x04\x42\xfe\xfe\x7d\x8a\x7b\x57\xc4\xce"
        "\x95\x6d\xbb\x07\x16\xcf\x59\x31\x41\x93\x7d\xa6\x92\xe6\xa9\x4b\x99"
        "\x34\xaa\x6f\xd1\x60\x9d\xa5\xf9\x34\x00\xdc\x45\xfc\x84\xeb\x73\xa9"
        "\x93\x3f\x47\xa7\xde\xbe\x63\xd1\x78\xf1\x3f\x8e\xc7\xa2\xda\xf9\x18"
        "\x5a\x18\xb3\x81\x74\x1f\xb5\x5c\xd4\xda\x27\x3a\x14\xfa\x61\x6e\xb9"
        "\x01\x80\xae\x9f\x4c\xf5\xaa\xd2\xad\x9f\x12\xa1\xd1\x62\x6c\x05\x08"
        "\xac\x44\x77\xe9\xfd\x14\xd4\xcc\x22\xda\x08\x43\x15\xe9\xc0\xfb\x31"
        "\x5d\x44\x4d\x5b\x8a\xe6\xec\x34\x6a\xcf\x86\x6a\x30\x0a\x9f\xf5\x01"
        "\x6a\x01\x57\xa9\x66\xb8\x80\x72\x95\x4c\x83\x33\x5b\x67\xec\xc7\x85"
        "\xd2\x74\xda\x04\xdc\x08\x22\x7b\x30\x4a\x9c\xe9\x67\x57\x22\x72\x6a"
        "\xff\xd8\x56\x2c\x56\x70\x47\xfb\xd5\x8c\xb1\x54\xec\xa0\x0f\x71\x20"
        "\x4d\x22\x1e\xd9\xe1\xe5\x07\x70\xa7\x5e\xc4\x1f\xe7\x5a\x7b\x1e\x26"
        "\xac\x18\x28\xcd\x0f\x2f\xa0\xe3\x01\x64\x7f\x4e\x70\xa1\xb0\xef\x77"
        "\xde\xf8\x48\x13\x62\x22\x49\xe4\x2d\x8a\x59\x35\x0d\x01\xb3\xdc\x21"
        "\xeb\x95\x04\x46\x61\xf7\xb4\xa1\xe7\x49\x57\x16\x25\xda\xfa\x29\x17"
        "\xb6\x1c\x68\xb1\xe0\xff\x5c\x0f\x68\x6d\x05\x3e\x78\x1c\x5d\x92\x13"
        "\x1b\x7c\x1c\xc0\x9b\xf4\x0d\xca\x92\x46\xd7\xff\x66\x20\x16\xca\xe8"
        "\xad\x1d\xad\x84\xf9\xc6\xd8\x05\x61\xdf\x16\x24\x1a\x86\x25\x4b\x73"
        "\x27\xf4\x04\xbd\xc0\x9f\x43\x8c\xde\x63\x27\x02\x8e\xe3\xc6\x5c\x69"
        "\x75\x6a\xaf\x19\x67\x98\xe3\xcb\x82\x4e\xa6\x16\xe1\x63\x63\x6f\xa3"
        "\xab\x4e\x5a\xba\x1a\x3b\x02\xa4\x1c\xee\x78\x9e\xd3\x38\xb6\x79\xa6"
        "\x1d\x33\x8f\xa6\x23\xce\x2d\xef\xc1\x53\xa1\x68\xc3\xdd\xf9\xb2\xe0"
        "\xf6\x38\xf2\x9a\x27\xfc\xf5\x59\x83\xb7\x01\x1f\x98\x33\xc7\xee\x1c"
        "\x95\x65\x4e\x6a\xda\xec\xbd\xac\xf2\x4f\x7f\xb6\x5a\xda\xdd\x53\xcd"
        "\x52\x01\xcb\x22\x82\x39\x41\x65\xa5\x02\x0d\x28\x9f\x30\x04\x8a\xee"
        "\x3e\x39\xbf\xcd\x01\xf9\x95\x5a\x8b\xdb\xb8\xc0\x0f\x09\x46\x91\x46"
        "\x68\x54\xcb\x4b\x04\x37\x37\xca\x1b\x9f\xd7\x32\xd8\xe2\x36\x6f\x8b"
        "\x45\xab\xd7\x89\xd9\xc0\xb3\x52\x85\xa8\x2c\x41\xee\xb0\x33\x60\xa8"
        "\x79\x06\x51\xd1\x17\x89\xff\x30\x8b\xcd\x57\x41\x70\x42\x77\x16\x76"
        "\x6a\xb0\x60\x31\x2e\x29\xc7\xd9\xe8\xbc\xd9\x98\x81\xdc\x4f\x1a\xd8"
        "\x8d\x62\x0f\x78\xd6\x8a\xdd\x31\x3c\xa9\x71\x14\x8a\x8a\xb6\xf9\x0a"
        "\x70\xc4\xda\x31\x41\x58\x58\xc6\x28\x31\x9a\x47\xa3\x71\xee\x39\x4e"
        "\xa6\x77\x78\xb1\x5e\x8d\x4e\xee\x93\x28\xb9\xa4\x58\xf1\xae\x10\x78"
        "\x66\xd9\xd1\xad\x7a\x9b\xec\x55\x25\x43\xeb\x08\x14\x00\x1d\xfd\xb3"
        "\x3b\x69\x16\x82\xee\x89\x2b\xdf\x99\x2e\x04\x94\x5d\x63\x5e\xef\x72"
        "\x93\x2b\xed\x92\x5b\x1c\xc8\x28\xf8\x5f\x1a\xdc\x37\xc1\x20\xb8\xc3"
        "\xe0\x21\x52\x9d\xb8\x18\xb9\x8c\x1f\x74\xb3\x47\x34\x40\xf4\xbd\x8f"
        "\x76\x77\x65\xd0\x39\xa0\x75\xbf\xd6\x6a\x9f\x3b\x9c\x6e\x4d\xcb\x12"
        "\x7d\x9c\x32\x27\x9b\x04\x41\xf2\xdd\x6e\x2a\xaa\xb3\xd7\x7a\x6e\xcd"
        "\xdb\xf8\x37\xc7\x5c\x1a\x7a\x84\x4f\xcd\x49\x98\x10\x73\x8d\x5f\xc7"
        "\xec\x08\x5d\x9f\xfc\xaa\x74\x56\x5f\x07\x70\xc7\x2a\x20\xde\xa4\xd6"
        "\xe4\xe8\x0b\x6d\x6d\x13\x9b\x66\xbd\xf7\xe0\xc1\x6c\x53\x81\x2f\x8e"
        "\x5a\x3c\x20\xed\x25\x0d\x7b\x4c\xb4\xdc\x9a\x09\x90\x1c\x2f\x91\xfa"
        "\x72\x31\xdf\x41\x59\x60\x2b\xe3\x5c\x25\x9e\x2e\xac\xf0\x08\xdc\xc6"
        "\x60\xfa\x79\xe7\x1d\x9b\xbc\x5b\x97\x77\x95\xdc\x13\xe4\x1d\x7c\xf2"
        "\x7f\x8d\x2c\x90\x7c\xca\x13\x2e\x30\x0b\x2f\x44\x98\x59\x32\x7b\x71"
        "\x49\xd5\x5d\xe5\x04\x87\xdf\xc9\xd2\x4b\x1a\x91\x62\xad\xa9\x8f\xda"
        "\xf0\x78\x34\x81\x8f\x06\x78\x40\x21\xa3\x4f\xc0\xf3\xbb\x9a\xbc\x49"
        "\x7e\x47\x5a\x7a\x15\xdd\xdc\xbe\x90\x43\x18\x31\x82\x50\x3a\xc1\x73"
        "\x9b\xb0\xbe\xfe\x37\xd4\xd1\xfa\x57\xd8\x1f\x0d\xf4\xaa\x05\x64\x5e"
        "\xaa\xef\x0b\xfd\xcb\x46\xa0\x51\xb9\x93\x57\x1b\x4a\x7a\xcb\x0f\xcf"
        "\x72\xef\x3b\x56\xff\x60\x5a\x41\x45\x55\xf2\x2d\xf8\xb3\xdd\xe0\x54"
        "\xb7\xed\xec\xe4\xe3\x8f\x18\x1f\xac\x36\x3e\x9b\xf4\x52\xed\x79\xb7"
        "\x55\x4e\x7c\xc7\xb0\x4f\xc4\x90\x45\xfb\xb6\x12\x63\x91\x72\x0a\x8b"
        "\xb1\xc1\x28\x10\x8b\xc3\x06\xcf\xd5\xa4\xcb\x94\x20\x1e\x87\x94\xd5"
        "\x65\xbe\xc4\xb9\x16\x02\x30\x5c\xe7\xb3\x3c\xed\x43\x5e\xcd\x47\x79"
        "\x03\xee\xad\x79\x8e\x75\x49\x48\x8f\x15\x80\x17\xe5\x5d\xaf\x85\xc7"
        "\xa4\x11\xd8\xcc\xc0\x0f\x7c\x5f\x43\xb7\xaa\xc1\xb8\xa5\x46\x1b\x56"
        "\x8a\x50\x95\xa3\xa4\x27\x2c\x13\x80\x7b\xcb\xb7\x9e\xf6\x6b\x9a\x6b"
        "\xe6\x2f\xde\xd2\x91\xd8\x16\x85\xbd\x3a\x78\x6b\x81\xad\xbb\x2a\x5f"
        "\x00\xa1\x5f\x1e\xe9\x2b\xc3\x5b\x8c\x9c\x01\x84\x1a\x4c\xa4\xea\x7d"
        "\xec\x15\xe8\xb7\x6b\x34\x8a\x22\x38\x76\x9c\x75\xac\xec\x24\x7e\x80"
        "\xa9\x80\xa0\x79\x84\xdc\xba\xb7\xab\x08\xba\xa6\xd7\x0d\x84\x32\x69"
        "\x9a\xa4\xcb\xb9\x2a\xbc\x48\x66\xf0\x54\x04\xf7\x3a\xfa\x0d\x6b\xd3"
        "\xd2\x7e\x77\xe5\x47\x6c\x1f\x4c\x00\x07\x5a\x8f\x9b\x33\xb1\x6b\xef"
        "\x9d\x62\x89\xa7\xbb\x11\x6c\x52\xd6\xa7\xf3\xf8\x9a\x56\xb9\x7d\x3a"
        "\xe9\x87\x21\x47\xa7\x3c\x5a\x2b\xdc\x70\x8d\x62\x8c\x0e\x54\xfc\x51"
        "\x28\xcb\xb5\x63\x87\x99\xa8\xfe\xb2\x33\x8e\x40\xd3\x6f\x2b\x8f\x3e"
        "\xe1\x05\xdc\x7f\x0d\x9d\x33\x70\x11\x5a\x47\xb6\x02\xce\x24\x9a\xf4"
        "\x53\x42\xa4\xa2\x0a\x9c\x58\xd9\x91\x63\xf8\x69\x4e\xcc\xa7\xf4\xf8"
        "\x15\xdd\xa0\xc2\x3f\x8f\x57\x63\x32\x33\x93\x3e\x27\x4b\x0c\x19\xf0"
        "\xf1\x2e\xe7\xbc\x19\x30\xde\x67\x1d\x08\x6b\x00\xcd\x65\xb1\x6a\xff"
        "\x92\xbd\x7a\xcd\xed\x41\xbc\x7d\x88\x83\xd2\x10\x8e\x5c\xc3\x80\xbe"
        "\x68\x1a\xd1\xae\x37\x60\x4a\x19\xcd\x90\x82\xbc\x3f\x54\xd7\x6d\xb6"
        "\x5a\xcc\x42\x73\x12\xcf\xd2\xe0\xea\xfb\xe9\x98\x92\x96\x2d\x32\xf7"
        "\x85\x61\xb4\x2d\x9f\xdb\x70\x26\xf8\x4b\x99\x40\x57\x89\x0a\x5b\x27"
        "\x0a\x74\xf7\xbe\x26\x5a\xf2\xed\x7b\x6b\x40\xd8\x68\x02\x58\x11\x58"
        "\x91\x88\x7b\xa5\xaf\x4c\xf6\xf6\x75\xb9\xa6\x2b\xd0\xe9\x0c\x12\xa4"
        "\x8d\x24\xcd\x54\x56\x1f\xe5\x4b\xd4\xdb\x99\xb4\x18\x09\xed\xc6\x8b"
        "\xc9\xc5\x87\x28\xde\xc9\xea\xf4\x56\xf9\x31\xca\x57\x34\x73\xf4\x89"
        "\x6e\x8e\xd9\x0e\x5d\xdd\x48\x33\x30\x1e\x83\x4c\x0e\x29\x5f\x41\xe7"
        "\xba\xb0\x91\xba\xb2\xeb\x55\x04\xac\x09\x22\x65\x42\x70\xb4\x9e\x82"
        "\xc1\xc8\x51\xf3\x92\xc2\xce\xbd\xd8\x85\x00\x67\x53\x05\x43\xd6\x8c"
        "\x57\xa0\x0b\xd6\x07\xd7\x5a\x9c\xba\x4c\xf0\xd7\xbe\x1a\xc9\xa5\xa4"
        "\x1c\xbd\xc4\x6c\xc3\x25\xce\xb6\xc1\x4b\x1c\xba\x52\xb3\x8c\x23\xd2"
        "\xc8\x24\x3c\xfc\x7b\x2a\xa9\x4c\x83\x96\xc7\x57\x14\x3e\xd7\xa4\xbf"
        "\x3e\xb6\x2e\x5a\x56\x96\xaa\xa1\xde\xfe\x43\x63\xb0\x16\x87\x62\x19"
        "\x51\x4c\x6f\x2c\x11\x0b\x5d\xd6\x1a\xfb\xe8\xbf\xb7\xbb\x51\x95\x4d"
        "\x8f\xaa\xdc\x27\x03\x2c\xdd\xb1\xd0\x76\xfd\x8a\x65\xdc\xd9\x8a\x31"
        "\x0e\x2d\xaa\x17\xaa\xee\x72\xae\xaa\x3f\x9b\x0e\x6e\x00\x7c\xe8\xf9"
        "\x79\x61\x9b\x17\x17\xde\x63\xf2\xfe\x72\x8f\xbf\xfe\x51\x5b\xe7\x48"
        "\xf9\xb6\xbe\x9c\xe0\xdb\xc8\xb4\x85\xc5\x92\x12\x44\x46\x36\x84\xe9"
        "\x9c\x53\xbc\x87\xd4\xa7\x71\xd8\x4b\x2b\xf0\x08\x7d\x79\xda\x25\xd2"
        "\x68\x0d\xe7\xbe\x5d\x59\x0d\xae\x9c\xbc\xd5\xfd\xb5\x20\x7e\xb0\xb6"
        "\xcb\xba\x37\x4d\xd0\x21\xd6\x83\x41\x3f\x76\x05\x9b\x47\xe7\xbf\x58"
        "\x64\xe0\x4b\x45\x62\x57\x34\x6c\xa1\x12\x23\x91\xcb\xf9\x94\x92\xde"
        "\xb4\x32\xea\x65\xab\x5f\x61\x63\x80\xd4\x08\x01\x39\x23\x11\x17\xad"
        "\x1c\x45\xd2\xc9\x31\x4f\x60\x02\xf0\xd1\x88\xa3\x0e\xc2\x11\x2e\x23"
        "\x5e\x7c\x80\xfd\xfb\x4c\x68\xa5\xcb\x5d\xb7\x69\x02\xf7\xd0\xfd\x01"
        "\x48\x9f\x07\xfb\x2b\x0b\xce\x94\x96\xc8\xb8\xc3\xaa\x3c\x91\xaf\xed"
        "\x11\x3c\x09\xc4\x73\x1d\xb4\x6a\xea\x72\xb7\x0c\x58\xb4\x3d\x33\xde"
        "\xe4\x34\x77\x13\xa3\x6f\x22\x44\xa3\x3c\x14\x02\x41\x9c\x5f\x1e\x4c"
        "\xfc\x99\xe1\x0b\xcb\xe3\xf9\x36\xb0\x32\xfa\x95\x8b\x7b\xcc\xea\xea"
        "\xb3\xf9\x4b\x21\x60\xa0\x4b\x30\x46\x46\x8a\x27\xa1\xbd\x98\xd1\xa3"
        "\xeb\xc3\x1a\xee\x82\x4a\xe3\xe0\x7f\xf4\x11\xf1\xdd\xa8\x81\x7c\x95"
        "\xcd\x66\x06\x3b\x7a\xa2\x58\xc1\x7a\x51\x33\x93\x24\x4c\xa1\x3e\x27"
        "\x88\xd1\xc5\xce\x4b\xab\x1c\x8a\x3e\x0a\x8b\x80\xb1\x2c\x38\x17\x00"
        "\x78\xd8\x1d\xdb\xa4\x4c\xdc\x96\x1a\x36\x30\x92\xa4\x01\xc1\x04\x72"
        "\x2b\x58\xd1\x45\x0a\xa7\xb7\xff\x34\x0f\xac\xae\x70\x97\x4b\x88\x5d"
        "\x32\xb2\x26\xb4\x4f\xdf\x16\x4c\x02\x73\x2a\x8c\x58\x81\x63\xf2\xc3"
        "\x85\x9d\x9a\x22\x48\xb9\xb9\x69\x1a\x6e\x72\x6d\x8b\xfc\xfc\x15\x5e"
        "\x33\xd4\x07\x43\x90\xfc\x98\x8c\xf5\xab\x6d\x4b\x9b\x40\xf1\x53\xb9"
        "\x38\xa7\x20\x66\xd0\xeb\x37\xca\x65\xd8\xe6\x61\x92\xc1\x9f\xb7\xc6"
        "\xf9\x12\x3c\x48\x83\xd7\x7a\x31\xeb\xfd\xe3\x2c\x30\x32\x90\x66\x85"
        "\x43\x43\xc7\x54\xd9\x95\x01\x80\xaf\xb8\x23\x43\x33\xc8\x7e\xf8\xdb"
        "\xca\x33\x94\x5e\xf5\x5c\xd6\x26\xc9\x0a\xbc\x41\x32\x38\x16\xa0\x0c"
        "\xcb\x5a\x6f\x3a\xe2\xf7\x69\xc3\x94\x4a\xb4\x9f\xc6\x08\x46\xf7\xe6"
        "\xc9\x39\x38\x24\x1d\xc6\xd4\x64\xb5\x58\xdb\x96\xc3\x09\xee\x53\xb1"
        "\x0e\xca\x57\xba\x90\xcb\x4b\x4c\x3c\xf4\x09\x87\xec\x2b\x86\xa2\xf1"
        "\xc1\x0d\x96\x05\x6e\x50\x78\x0a\x4b\x0f\xec\x2f\x44\x5f\x81\x04\x94"
        "\xc8\xba\x79\xdc\x3a\x9e\x05\x35\x93\x0c\x17\xe4\xb9\x02\xe5\x7d\xf7"
        "\xbf\x76\xb3\x89\xd2\x42\xfa\x6c\xff\x52\xf6\xf3\xd2\xda\x7b\xfc\x94"
        "\xbe\x33\x73\xa3\xcd\x8e\x0e\xcc\xa0\xc5\x6d\xa5\xd1\xfd\x9b\x65\x29"
        "\x0b\xd6\xaf\x38\xd7\x8c\x0c\x93\x52\x6b\xcc\x7d\x21\x4f\xde\xe8\x07"
        "\x9e\x08\x0a\xa1\xbc\x08\x31\x1d\xdc\x0d\x9a\xdd\xbc\x9d\x42\x62\x33"
        "\x4b\xa5\x61\xd0\xf4\xea\xee\x85\x4d\x7e\x5c\x23\x06\xb9\x1d\x7a\x0a"
        "\xa1\xe8\x90\x99\x94\xd8\x79\xb8\xba\x75\x55\xba\x67\x41\xba\x98\xf6"
        "\x3a\xfc\x89\x8c\x09\x22\x0f\xdc\xd2\x2c\x30\x57\xe8\xee\xfc\x78\x27"
        "\x60\xa9\x7f\x6c\x91\xc1\xf4\xb4\x44\x3f\xfe\xb8\xc9\xa0\xa8\x5e\x14"
        "\x20\x07\xc0\xd8\x04\x58\xd5\x30\x52\x69\xae\x9a\xf7\xea\x6a\xdd\x36"
        "\xa6\x1a\xc7\x08\x93\x50\x9b\x9c\xc8\x97\x77\x7c\xa7\x13\x13\xc4\x5b"
        "\x33\xff\x9f\x35\x70\x19\xed\xa1\x1f\x5d\x9d\x9b\xf6\x64\x23\x7c\x4b"
        "\x6c\x2d\xbd\xef\x0f\xeb\xf8\xa3\xe7\xa5\x61\x62\xbb\x0b\xdb\x10\x29"
        "\x36\x2a\x3b\x29\xf6\xa5\x69\xe3\x27\xc6\x86\xff\xf5\x40\x89\xec\x4e"
        "\xfd\x76\x70\x22\x17\x48\x34\x4d\x44\x0c\xf1\x0b\xcf\x8c\x32\x82\xce"
        "\xfd\x8f\xa2\x2d\xce\x27\x14\x22\x47\xdd\xbf\xda\x40\x80\x69\x46\x78"
        "\xb1\x7c\x24\x64\x4f\xf1\xaa\x5d\xea\xd1\x40\xf6\xc8\xee\x79\xf9\x21"
        "\x67\xe2\xbb\x57\xc6\xe6\x44\x3b\x08\x5e\x71\x42\xef\x6c\xbc\x86\xc7"
        "\x78\xb8\x6a\x24\xd6\xc8\x23\x99\x47\x6d\xaf\x57\x58\xcb\x49\xbd\xc6"
        "\xca\x57\x07\xf5\x7f\x62\xb4\xad\x51\xea\x3d\x6f\x69\xe4\x6a\xcc\x6a"
        "\x51\x81\xe9\x89\x96\xcd\xf9\x0f\xc5\x5e\x8a\x2f\xaa\x23\xa4\x7d\xdc"
        "\xd6\xa4\x66\xdb\x22\x27\x7e\x12\xd1\xee\xbd\x83\xab\xd7\x7f\xec\xc6"
        "\xb5\xbe\x73\xce\xa3\x20\x58\x2a\xe4\xf3\x8d\x56\x33\x40\x5f\x96\x3c"
        "\x42\x16\x93\xa3\xc3\x39\xb4\xb4\xdd\x94\x31\xa2\x82\x41\xbf\x42\x83"
        "\xce\x17\xa9\xb4\x7b\x3e\x6f\x64\x4b\x8f\xc8\x5b\xeb\x7c\xc7\x5d\xad"
        "\xf4\x37\x54\xc3\x4c\x2e\xe5\x2d\xb1\xb5\x5c\x71\x1f\x95\x85\x8c\x26"
        "\xf0\x60\x83\xe2\x77\x05\xbe\x26\x99\xc4\xac\xd6\xb0\x7c\x1a\x57\x54"
        "\x57\xcb\x8c\x95\xe0\xde\x64\x88\xe1\x11\xb1\x56\xd3\x9e\x85\x89\x4d"
        "\x37\xa3\xf7\x22\xc8\xf5\x4d\x8b\x65\x86\x5d\x28\x5b\xce\x47\x24\x93"
        "\x01\xb0\x5c\x26\x26\xd9\xd2\x99\x16\xb8\x2d\x0c\x42\x39\x07\xde\xc5"
        "\xe4\xff\xf6\x1f\xda\x3b\x8b\x67\x15\xc2\x05\xcd\xd6\x02\x20\x91\x77"
        "\x2a\xd3\xb6\x6c\x37\x8b\x69\x46\x5b\x5d\x7d\xcc\xa5\x9a\xf0\xa4\x29"
        "\x70\x1b\x3f\xb9\xae\x04\x90\x71\x66\x30\xc8\x26\xc0\x02\x26\x9d\x17"
        "\xe7\x3c\x46\x70\xaf\xa2\x7d\x08\x45\xf7\xb1\x22\x50\xb9\xc3\x77\x16"
        "\x92\xc3\x04\xa6\x18\xe0\x4f\x10\xc5\xce\x10\xa1\x19\x66\x61\x4f\x2b"
        "\x17\x42\x62\x52\x05\x6c\x92\xf8\x0d\xd2\xef\xd1\x37\x23\x5e\x46\x07"
        "\x09\x56\xb6\x7d\x95\xae\xb2\xb3\x11\xab\xdc\x81\xee\x21\xd6\x2e\x50"
        "\x1f\xd0\x10\x65\x2e\x18\x1b\x51\xed\x4d\xeb\x52\x42\xdf\x4b\x23\xb9"
        "\x55\xc3\x8a\x13\x93\x10\xc6\x29\x28\x7f\x58\xb6\x62\x0a\xef\x97\x6e"
        "\xc9\x4e\xaa\xe5\x0f\x8f\xb7\xd6\x03\xbc\x7c\x38\x40\x8b\x4b\x61\xfe"
        "\x8b\x00\x34\x86\xc2\x15\x1b\x2c\x02\x8e\xf1\xfb\xa7\x90\xdd\x98\x51"
        "\x98\xe2\xbc\xa1\x42\xa8\x52\xeb\xe7\xcf\x30\x66\x13\x08\x25\x1e\x21"
        "\xff\x69\x18\x3e\x57\xcd\xc0\x26\x68\x54\x5c\xa2\x10\xac\x06\xa7\x60"
        "\x76\x67\xa7\x5f\xc2\x11\x35\x4c\x77\xde\x8a\xd8\xb6\x7c\x1e\xf1\x1b"
        "\x0e\x2c\x57\x3b\x62\xc6\xae\x17\x2c\x0e\xe4\x71\x48\x63\x8f\x20\x29"
        "\xec\xd6\xe0\xaf\x90\xd5\x73\xc9\x84\x81\x96\x63\xb2\x5d\x00\x0f\x72"
        "\x16\x7f\x9c\xbf\xb6\x52\xb8\x33\x7a\x29\x86\x60\x00\xd6\xa2\x55\x01"
        "\x90\x18\x75\xd6\xc9\xd5\x26\x60\xab\x7e\xda\xc9\x38\xd3\x57\xdf\xb3"
        "\x15\x69\xbf\x27\x5e\x0f\x66\x4c\x7d\x41\x40\xf2\x5c\xeb\x3d\x50\x93"
        "\xbc\x50\x1f\x6e\x37\x36\xa1\x73\x48\xcf\xe3\x00\x20\xc7\x2a\xe5\x8f"
        "\x5e\xc7\x11\x48\xb2\x0a\xeb\x1e\x9d\xf8\x51\xde\x9f\xa8\x09\x8e\x1d"
        "\x53\x2b\x65\xa9\x2f\xc5\x46\xd2\x79\x76\x18\x67\x83\xb4\x48\xc2\x96"
        "\xf8\xb3\xb4\xa0\x4e\xd5\xa4\x7a\xa1\x8a\xa5\xfd\xe8\xd0\xdc\x2d\xf4"
        "\x37\x63\x0c\x19\xc4\xa0\x5a\x0a\xbe\xa9\xd3\xb2\xca\xfa\x49\x6f\xfa"
        "\x81\x78\x5d\xd2\x81\x43\x68\x1e\xf4\x71\x3c\x1f\x80\xb9\x11\xe5\x17"
        "\xd1\x7e\xfc\xfb\xd0\x4d\xfe\x20\xba\x6d\x88\xe6\x8a\xc1\x75\xd4\xf7"
        "\xcc\x85\x0f\xfa\xe4\x4c\x44\xc6\x4f\xa6\x0c\xce\xd1\x4b\xf7\xb9\xba"
        "\x79\x17\xf5\x8d\x90\xff\x2c\xfd\xc3\x66\xbb\xf6\x1b\xd1\x7b\xf4",
        4096));
    NONFAILING(*(uint32_t*)0x200012c0 = -1);
    NONFAILING(*(uint16_t*)0x20001320 = -1);
    NONFAILING(memcpy(
        (void*)0x20001322,
        "\x66\xc7\xea\x99\xe5\x27\x3f\xa9\x6a\x9a\x2b\x1b\x85\xf3\xf3\xb3\xb7"
        "\x8d\x37\x21\x0d\xa7\xcd\x26\xd3\x6c\x59\x7e\x90\xa3\xcb\x2b\x22\x99"
        "\x93\xf3\x6b\x92\x66\x7f\xb7\x52\x6c\x58\xd1\x03\x92\x64\xec\x47\x93"
        "\x5f\x0a\x0d\xbc\x54\x13\xe0\xe3\x87\x78\xd4\xa5\x35\x83\x63\x28\x68"
        "\xb0\x95\x80\xdb\xb8\xcc\x62\x37\x36\xdf\xf5\xa0\x56\xe7\x7b\x2b\xf9"
        "\xfe\x30\x08\x27\x01\xdb\xca\x7a\xa8\xa9\x62\xf4\x24\xad\xbc\x38\xc2"
        "\x1a\xc5\x66\xd4\xf4\x0d\x48\x64\x77\x6d\x8f\xea\x36\x26\x64\x42\xe4"
        "\xac\x4f\xae\xa9\xe4\x20\xbe\x4a\xc0\xca\xea\x2e\x26\x0b\xb7\xd7\xc2"
        "\x21\xbe\x89\xc8\x08\x72\x5f\x75\xc6\xa8\xbc\x72\xc0\x07\xad\x91\xec"
        "\x6d\x69\x1a\x42\xd7\x9f\xf4\x01\x70\x9a\x89\x25\x1a\xd1\x17\xfc\x65"
        "\x7c\xc8\x28\xbc\xe4\x25\x17\x4d\xdd\x97\xb5\x6b\x52\x10\x76\xc2\xab"
        "\xc4\x81\x20\xea\x45\x8d\xff\xef\x88\xda\xd0\x89\x22\xa1\x91\xe5\x81"
        "\x85\x55\xc0\x68\x36\xc3\x1c\x63\xeb\x83\x63\x76\x09\xcd\xbf\x1e\x36"
        "\x44\x19\xb4\x90\xf0\xe1\x78\xf2\x6a\xa6\x7e\x38\x20\xa5\xc1\xce\x33"
        "\xe8\x26\xd5\x5c\xed\xa2",
        244));
    NONFAILING(memcpy(
        (void*)0x20001416,
        "\xd2\x94\x69\xe2\xda\x7b\x52\x6e\xdc\x0e\x8a\x75\x4b\xdf\x5c\x8f\x87"
        "\xd3\x07\x81\xbb\x7c\x8a\x9b\x85\xfe\x56\x5f\x59\x62\x33\xd1\x25\xb9"
        "\xf9\x52\x01\x1e\x41\x6f\x2b\xf3\x43\xd9\x98\x1f\x99\xa8\x02\xad\x95"
        "\x82\x74\xaa\x93\x31\xbc\x26\xca\x29\x71\x8b\xbd\x2a\x48\x3b\x59\x94"
        "\x14\xaa\xbc\xeb\x62\x95\x52\x5a\x9b\x85\x28\x2e\xbb\xbd\x2c\x1e\x3b"
        "\x54\x38\xfa\x0e\x89\xe0\x06\x38\x15\xb6\x3c\xe1\x51\x1f\x74\xc1\xfa"
        "\x47\x15\x42\x18\x35\x71\x54\xe2\x76\x1d\x21\x23\x35\x2d\x33\x06\xdc"
        "\x6f\x57\x7a\x21\x32\x91\x2d\x80\x25\xa9\xe5\x8f\xc0\xca\xee\xf7\xf5"
        "\x4c\xe1\x58\x6f\xb7\xbd\xc8\xf5\x42\xad\x5f\x20\xbf\x63\xbc\x7d\x00"
        "\x12\x82\x1e\xa9\x50\x92\x18\xd8\x98\xc3\x13\x8b\x59\x20\xe7\x35\x3f"
        "\x9b\xce\x1c\x1d\x3f\xe1\x77\xca\xbf\xc5\x49\xce\x57\x40\x5e\x1b\x60"
        "\x80\x00\xd5\xa2\x5c\x12\x09\x59\x50\xba\x18\xbc\x79\x35\x44\x86\x84"
        "\x14\x39\x28\xb3\x22\xf4\x77\xe3\x72\xee\x67\xaf\x49\x0c\x9e\x48\xa2"
        "\x92\xc6\xef\x27\x1b\xf2\x6f\x84\x2a\x8e\xe4",
        232));
    NONFAILING(*(uint64_t*)0x200014fe = -1);
    syz_mount_image(0x20000040, 0x20000080, 0, 1, 0x20000200, 0, 0x200000c0);
    break;
  }
}

void execute_one()
{
  execute(1);
  collide = 1;
  execute(1);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  char* cwd = get_current_dir_name();
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      install_segv_handler();
      for (;;) {
        if (chdir(cwd))
          fail("failed to chdir");
        use_temporary_dir();
        int pid = do_sandbox_none();
        int status = 0;
        while (waitpid(pid, &status, __WALL) != pid) {
        }
      }
    }
  }
  sleep(1000000);
  return 0;
}
