// BUG: spinlock bad magic
// https://syzkaller.appspot.com/bug?id=ef5db7606b46c2741e271d7c2a21e0dafb5e05d3
// status:invalid
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}

__attribute__((noreturn)) static void fail(const char* msg, ...)
{
  int e = errno;
  fflush(stdout);
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
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

static void snprintf_check(char* str, size_t size, const char* format,
                           ...)
{
  va_list args;

  va_start(args, format);
  vsnprintf_check(str, size, format, args);
  va_end(args);
}

#define COMMAND_MAX_LEN 128

static void execute_command(const char* format, ...)
{
  va_list args;
  char command[COMMAND_MAX_LEN];
  int rv;

  va_start(args, format);

  vsnprintf_check(command, sizeof(command), format, args);
  rv = system(command);
  if (rv != 0)
    fail("tun: command \"%s\" failed with code %d", &command[0], rv);

  va_end(args);
}

static int tunfd = -1;

#define SYZ_TUN_MAX_PACKET_SIZE 1000

#define MAX_PIDS 32
#define ADDR_MAX_LEN 32

#define LOCAL_MAC "aa:aa:aa:aa:aa:%02hx"
#define REMOTE_MAC "bb:bb:bb:bb:bb:%02hx"

#define LOCAL_IPV4 "172.20.%d.170"
#define REMOTE_IPV4 "172.20.%d.187"

#define LOCAL_IPV6 "fe80::%02hxaa"
#define REMOTE_IPV6 "fe80::%02hxbb"

static void initialize_tun(uint64_t pid)
{
  if (pid >= MAX_PIDS)
    fail("tun: no more than %d executors", MAX_PIDS);
  int id = pid;

  tunfd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
  if (tunfd == -1)
    fail("tun: can't open /dev/net/tun");

  char iface[IFNAMSIZ];
  snprintf_check(iface, sizeof(iface), "syz%d", id);

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, iface, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
    fail("tun: ioctl(TUNSETIFF) failed");

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

  execute_command("sysctl -w net.ipv6.conf.%s.accept_dad=0", iface);

  execute_command("sysctl -w net.ipv6.conf.%s.router_solicitations=0",
                  iface);

  execute_command("ip link set dev %s address %s", iface, local_mac);
  execute_command("ip addr add %s/24 dev %s", local_ipv4, iface);
  execute_command("ip -6 addr add %s/120 dev %s", local_ipv6, iface);
  execute_command("ip neigh add %s lladdr %s dev %s nud permanent",
                  remote_ipv4, remote_mac, iface);
  execute_command("ip -6 neigh add %s lladdr %s dev %s nud permanent",
                  remote_ipv6, remote_mac, iface);
  execute_command("ip link set dev %s up", iface);
}

static void setup_tun(uint64_t pid, bool enable_tun)
{
  if (enable_tun)
    initialize_tun(pid);
}

static void test();

void loop()
{
  while (1) {
    test();
  }
}

long r[25];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    *(uint32_t*)0x20bacffc = (uint32_t)0x0;
    r[2] = syscall(__NR_accept4, 0xfffffffffffffffful, 0x20000000ul,
                   0x20bacffcul, 0x80000ul);
    break;
  case 2:
    r[3] = syscall(__NR_close, 0xfffffffffffffffful);
    break;
  case 3:
    r[4] = syscall(__NR_socket, 0x11ul, 0x3ul, 0x300ul);
    break;
  case 4:
    r[5] = syscall(__NR_socket, 0x2ul, 0x3ul, 0xcul);
    break;
  case 5:
    *(uint16_t*)0x201ef000 = (uint16_t)0x41d;
    *(uint8_t*)0x201ef002 = (uint8_t)0x3;
    *(uint8_t*)0x201ef003 = (uint8_t)0x8000;
    r[9] = syscall(__NR_setsockopt, r[4], 0x107ul, 0x12ul, 0x201ef000ul,
                   0x4ul);
    break;
  case 6:
    *(uint16_t*)0x2006bfec = (uint16_t)0x11;
    *(uint16_t*)0x2006bfee = (uint16_t)0x1100;
    *(uint32_t*)0x2006bff0 = (uint32_t)0x0;
    *(uint16_t*)0x2006bff4 = (uint16_t)0x1;
    *(uint8_t*)0x2006bff6 = (uint8_t)0x2a;
    *(uint8_t*)0x2006bff7 = (uint8_t)0x6;
    *(uint8_t*)0x2006bff8 = (uint8_t)0xbb;
    *(uint8_t*)0x2006bff9 = (uint8_t)0xbb;
    *(uint8_t*)0x2006bffa = (uint8_t)0xbb;
    *(uint8_t*)0x2006bffb = (uint8_t)0xbb;
    *(uint8_t*)0x2006bffc = (uint8_t)0xbb;
    *(uint8_t*)0x2006bffd = (uint8_t)0x0;
    *(uint8_t*)0x2006bffe = (uint8_t)0x0;
    *(uint8_t*)0x2006bfff = (uint8_t)0x0;
    r[24] = syscall(__NR_bind, r[4], 0x2006bfecul, 0x14ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[14];

  memset(r, -1, sizeof(r));
  srand(getpid());
  for (i = 0; i < 7; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  for (i = 0; i < 7; i++) {
    pthread_create(&th[7 + i], 0, thr, (void*)i);
    if (rand() % 2)
      usleep(rand() % 10000);
  }
  usleep(rand() % 100000);
}

int main()
{
  int i;
  for (i = 0; i < 8; i++) {
    if (fork() == 0) {
      use_temporary_dir();
      setup_tun(i, true);
      loop();
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}
