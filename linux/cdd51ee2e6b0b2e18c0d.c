// WARNING in netlbl_cipsov4_add
// https://syzkaller.appspot.com/bug?id=cdd51ee2e6b0b2e18c0d
// status:0
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/genetlink.h>
#include <linux/if_addr.h>
#include <linux/if_link.h>
#include <linux/in6.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/veth.h>

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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

const int kInitNetNsFd = 239;

static long syz_init_net_socket(volatile long domain, volatile long type,
                                volatile long proto)
{
  return syscall(__NR_socket, domain, type, proto);
}

static long syz_genetlink_get_family_id(volatile long name,
                                        volatile long sock_arg)
{
  int fd = sock_arg;
  if (fd < 0) {
    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (fd == -1) {
      return -1;
    }
  }
  struct nlmsg nlmsg_tmp;
  int ret = netlink_query_family_id(&nlmsg_tmp, fd, (char*)name);
  if ((int)sock_arg >= 0)
    close(fd);
  if (ret < 0) {
    return -1;
  }
  return ret;
}

uint64_t r[3] = {0xffffffffffffffff, 0x0, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = -1;
  res = syz_init_net_socket(0x10, 3, 0x10);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000b00, "NLBL_CIPSOv4\000", 13);
  res = -1;
  res = syz_genetlink_get_family_id(0x20000b00, r[0]);
  if (res != -1)
    r[1] = res;
  res = -1;
  res = syz_init_net_socket(0x10, 3, 0x10);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x20000140 = 0;
  *(uint32_t*)0x20000148 = 0;
  *(uint64_t*)0x20000150 = 0x20000100;
  *(uint64_t*)0x20000100 = 0x20000040;
  *(uint32_t*)0x20000040 = 0xac;
  *(uint16_t*)0x20000044 = r[1];
  *(uint16_t*)0x20000046 = 1;
  *(uint32_t*)0x20000048 = 0;
  *(uint32_t*)0x2000004c = 0;
  *(uint8_t*)0x20000050 = 1;
  *(uint8_t*)0x20000051 = 0;
  *(uint16_t*)0x20000052 = 0;
  *(uint16_t*)0x20000054 = 8;
  *(uint16_t*)0x20000056 = 2;
  *(uint32_t*)0x20000058 = 2;
  *(uint16_t*)0x2000005c = 0xc;
  STORE_BY_BITMASK(uint16_t, , 0x2000005e, 4, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000005f, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000005f, 1, 7, 1);
  *(uint16_t*)0x20000060 = 5;
  *(uint16_t*)0x20000062 = 3;
  *(uint8_t*)0x20000064 = 6;
  *(uint16_t*)0x20000068 = 0x38;
  STORE_BY_BITMASK(uint16_t, , 0x2000006a, 8, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000006b, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000006b, 1, 7, 1);
  *(uint16_t*)0x2000006c = 0x34;
  STORE_BY_BITMASK(uint16_t, , 0x2000006e, 7, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000006f, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000006f, 1, 7, 1);
  *(uint16_t*)0x20000070 = 8;
  *(uint16_t*)0x20000072 = 5;
  *(uint32_t*)0x20000074 = 0x5c338c7;
  *(uint16_t*)0x20000078 = 8;
  *(uint16_t*)0x2000007a = 5;
  *(uint32_t*)0x2000007c = 0x3a2e6e;
  *(uint16_t*)0x20000080 = 8;
  *(uint16_t*)0x20000082 = 6;
  *(uint32_t*)0x20000084 = 8;
  *(uint16_t*)0x20000088 = 8;
  *(uint16_t*)0x2000008a = 5;
  *(uint32_t*)0x2000008c = 0x7a657908;
  *(uint16_t*)0x20000090 = 8;
  *(uint16_t*)0x20000092 = 6;
  *(uint32_t*)0x20000094 = 0xf1;
  *(uint16_t*)0x20000098 = 8;
  *(uint16_t*)0x2000009a = 6;
  *(uint32_t*)0x2000009c = 0x47;
  *(uint16_t*)0x200000a0 = 8;
  *(uint16_t*)0x200000a2 = 2;
  *(uint32_t*)0x200000a4 = 2;
  *(uint16_t*)0x200000a8 = 8;
  *(uint16_t*)0x200000aa = 2;
  *(uint32_t*)0x200000ac = 3;
  *(uint16_t*)0x200000b0 = 8;
  *(uint16_t*)0x200000b2 = 1;
  *(uint32_t*)0x200000b4 = 0;
  *(uint16_t*)0x200000b8 = 8;
  *(uint16_t*)0x200000ba = 2;
  *(uint32_t*)0x200000bc = 1;
  *(uint16_t*)0x200000c0 = 0x2c;
  STORE_BY_BITMASK(uint16_t, , 0x200000c2, 4, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200000c3, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200000c3, 1, 7, 1);
  *(uint16_t*)0x200000c4 = 5;
  *(uint16_t*)0x200000c6 = 3;
  *(uint8_t*)0x200000c8 = 5;
  *(uint16_t*)0x200000cc = 5;
  *(uint16_t*)0x200000ce = 3;
  *(uint8_t*)0x200000d0 = 7;
  *(uint16_t*)0x200000d4 = 5;
  *(uint16_t*)0x200000d6 = 3;
  *(uint8_t*)0x200000d8 = 6;
  *(uint16_t*)0x200000dc = 5;
  *(uint16_t*)0x200000de = 3;
  *(uint8_t*)0x200000e0 = 4;
  *(uint16_t*)0x200000e4 = 5;
  *(uint16_t*)0x200000e6 = 3;
  *(uint8_t*)0x200000e8 = 2;
  *(uint64_t*)0x20000108 = 0xac;
  *(uint64_t*)0x20000158 = 1;
  *(uint64_t*)0x20000160 = 0;
  *(uint64_t*)0x20000168 = 0;
  *(uint32_t*)0x20000170 = 0x840;
  syscall(__NR_sendmsg, r[2], 0x20000140ul, 0x4904ul);
  return 0;
}
