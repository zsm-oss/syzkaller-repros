// KASAN: stack-out-of-bounds Write in bitmap_from_arr32
// https://syzkaller.appspot.com/bug?id=9d39fa49d4df294aab93
// status:6
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0x10);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x200000c0, "ethtool\000", 8);
  res = -1;
  res = syz_genetlink_get_family_id(0x200000c0);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x20000440 = 0;
  *(uint32_t*)0x20000448 = 0;
  *(uint64_t*)0x20000450 = 0x200003c0;
  *(uint64_t*)0x200003c0 = 0x20000480;
  memcpy((void*)0x20000480, "\x40\x10\x00\x00", 4);
  *(uint16_t*)0x20000484 = r[1];
  memcpy(
      (void*)0x20000486,
      "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00\x18\x00\x01\x80"
      "\x14\x00\x02\x00\x74\x75\x6e\x6c\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x14\x10\x03\x80\x04\x00\x01\x00\x08\x00\x02\x00\xff\x7f\x00\x00"
      "\x04\x10\x04\x00\xfa\x08\xea\xa0\x66\x4b\xa4\x00\x00\x00\x00\xa3\x49\x94"
      "\x0c\x62\xa8\xd8\xad\x19\xc1\xce\x22\x02\x7a\x91\x20\x3b\x36\xc0\x92\x96"
      "\x16\x3d\xfb\x80\xd9\x0b\x89\x5a\x0d\x88\x70\x2e\x0d\x8c\x8a\x7d\x6d\xd4"
      "\x91\x28\x16\x27\xad\x7c\x7f\x79\xd1\x01\x58\x5b\x02\x2d\x81\xe6\xc6\xb6"
      "\x52\xe4\xc2\x15\xbc\x13\x7c\xb7\x75\xdc\xa9\xe1\xe7\x62\x4f\xa5\xef\x0f"
      "\x6a\x60\xfa\xbf\xb5\x81\x7c\xf8\x41\x6f\xd1\x66\x74\x8d\xa7\x7e\x98\x8a"
      "\x9f\x8b\xcd\xcf\xc8\x0d\x4b\x3a\x68\x8e\x2f\x80\xf4\xad\x71\x43\x8f\x71"
      "\x2b\xe5\x41\xef\xea\x88\x64\x4e\xd2\x2b\x17\xb6\x47\xef\x01\xc1\xb9\xa5"
      "\xf7\x8b\x8b\x0f\xf1\x9f\xaa\x50\x55\x24\x45\x2a\xc8\x12\xba\x2a\x48\x2d"
      "\xd8\xba\x3f\xef\xba\x3d\x10\x7a\xcc\x9c\x28\xd4\xc1\x09\xee\xb5\x48\xe1"
      "\x56\xf8\xc7\x9e\x57\x18\x79\xa8\x16\xff\x39\xa7\x5c\x16\xcd\x83\xdb\x44"
      "\xd0\x06\x53\x30\xb4\x7c\x27\xa6\xba\xb8\xac\x94\x3f\x85\x48\xbb\x02\x30"
      "\x21\xe7\x6e\x7d\xef\xef\xdd\x48\xf0\xc8\xde\x5c\x6a\x44\xf2\xb5\x54\x0b"
      "\xcf\x37\x6d\x6c\x91\x50\x0b\xdd\xe1\x87\xa1\xd9\x5d\x06\x8c\x4a\x31\x94"
      "\x2a\x88\x0a\x49\x61\x6a\x77\xbd\xf1\x64\x70\x4f\x34\xb9\xbb\x34\xd2\xa6"
      "\xed\xb9\x89\xc5\x69\x5d\xff\xa3\xfb\xa3\x46\x5d\x0a\xce\x10\xa2\x20\x4f"
      "\xba\x7b\x75\x40\x2c\x7c\x89\x47\xe7\xf1\xb2\x8f\xd2\x9e\x5e\xa3\x6c\xb1"
      "\xcc\x4b\x10\x08\x8c\x16\x11\x31\xca\xf1\xaf\x8c\xf4\x31\xed\xab\x65\xe5"
      "\x13\xa4\xac\xde\x59\x25\x26\x8b\xa8\x08\x7d\xe8\x10\xc7\x9b\x4a\x4d\x94"
      "\x65\x68\xac\xb2\xfc\x8d\x3a\x07\x7c\x51\x7d\xdb\x01\x96\xba\x31\xeb\xa6"
      "\xc0\x29\xb7\x26\x7d\x5d\x55\x32\x77\xda\x3c\xc7\x15\x46\xbd\x11\xd6\xfb"
      "\xd4\x02\x89\xc3\xab\xe4\xf2\xf3\xef\x64\x5a\x81\xf2\xd2\x14\x38\xd9\x5c"
      "\x75\xff\x69\xc0\x4c\x93\xd8\xb6\x0f\x80\xd7\xc1\x16\x15\xd8\xab\x82\x46"
      "\x18\x76\x53\x6b\x31\x86\x98\x53\x2f\x1e\x1e\xc7\xb3\xb0\x82\x26\xb9\xbe"
      "\xe6\x04\xb4\x0f\xac\x0f\xd0\xe8\x18\x0e\x96\x61\xf0\x2e\x0e\x8f\xaf\xaa"
      "\x9f\xa5\x5a\xf7\xb9\xf5\xe9\x34\x40\x7f\xad\xaf\xd2\x6c\x38\x25\x76\xe8"
      "\x21\x0f\x4b\x51\xac\x45\x27\xc0\x7b\x26\xc4\xf1\xa8\x20\xe5\x6c\x60\x66"
      "\x62\x01\x32\x21\x3d\x20\xbe\x1e\x4d\x10\xc9\x72\x22\x5b\x51\xf0\xf4\x48"
      "\x49\xa5\xbc\x0f\xdb\x51\xf4\x75\x35\x64\xd6\x35\x2f\x9c\xac\x28\x3a\x9a"
      "\xa4\xf9\xa8\xe7\x61\xa4\x9f\x54\xf4\xf2\x70\x56\xad\xc7\x23\x99\xd2\x62"
      "\xb2\x08\x6c\x83\xeb\x65\x80\xad\x3e\x37\xf1\xad\xaf\xa8\xeb\xef\x6f\x9a"
      "\x23\x9c\x90\x16\xa5\xac\x19\xae\x0b\x46\x1f\x9d\x35\x6b\x27\x08\x60\x52"
      "\x1f\x28\x63\xc9\x43\xe5\xfe\x02\xd7\xb7\xd0\x1d\xce\x83\xa8\x6a\x56\x20"
      "\xcf\x67\xa6\x51\xc2\x2e\x0d\x68\xd7\x5d\x6e\xbe\xb0\xdb\x4a\x5e\x50\x96"
      "\x52\x38\x34\xcc\xb3\x51\x00\xb7\xdd\x1f\xaa\xc5\xe6\x89\xbd\xa9\x6e\x67"
      "\x4b\xad\x75\xe3\x2b\x32\xb8\x9e\x98\x8b\xf3\x09\x70\x52\x05\x10\xc6\x96"
      "\x1d\x60\x55\xe2\x8a\xfa\xf2\xb8\x71\x7c\x71\x5d\x39\xec\xaa\x09\xfa\xc1"
      "\xb5\x58\xd5\x47\x01\xc2\x1b\x4f\xc0\x59\x9c\xd6\x37\x2a\xa6\xd8\x2b\x95"
      "\xbe\xff\xfe\x8e\x52\xfd\x4c\x19\xe9\xe7\xf9\x1e\x2d\x88\x28\xad\x6f\x38"
      "\xa7\x36\xb6\x27\x74\x85\x75\x21\x9c\x77\x66\xb0\x98\x35\xec\xbd\xfe\xd6"
      "\x9c\xfa\xe5\x07\x34\x89\x78\xae\xa3\x5a\x77\xd0\xf0\xd8\x95\x13\x5b\x23"
      "\xad\xda\x36\xf3\x3a\x63\x07\xf3\x35\x28\x81\x35\xcb\xbc\x7c\x34\x30\xd6"
      "\x0c\x0b\xb7\xb8\x26\x77\x4c\x06\x32\x78\x49\x95\x3d\xa2\x2d\x62\x4a\x0c"
      "\x2d\x05\xc3\x15\x41\xe5\xe4\x9d\x5b\xe3\x2a\x7d\x45\x2d\x2d\xf6\x3b\x03"
      "\x40\xb4\x43\xbb\xf9\xe1\x8e\xbe\xba\x6d\xd8\xcf\x36\x41\x5d\x01\xf8\xd7"
      "\x16\xfa\x08\x6c\xc1\x19\x17\x2a\xeb\xc9\x69\x40\x68\x13\x95\x1d\x26\xda"
      "\xde\x4d\xe3\x13\x65\x6e\x72\x10\xf0\x0c\x63\x5f\x51\xee\xd4\xa3\xd7\xb8"
      "\x58\x6c\x2f\x00\xaf\x04\xa2\xa1\xec\x8a\x4d\x08\x4c\xfa\xa5\xd8\xcd\x0a"
      "\xd4\xcd\xdd\xc1\xcc\x17\xa1\xf6\xe7\xc9\x9d\xa9\x03\x5b\x77\x39\xa0\xb3"
      "\xbc\x50\x3a\x4e\xdc\x7a\x29\xbe\x1f\x10\x5a\xfd\x6e\x65\x41\x5c\xca\xea"
      "\x26\xf8\xae\x32\x9e\x27\x72\xfe\xd0\x16\x41\x05\x3d\xc0\x55\x81\x5a\x92"
      "\x27\xd7\xc8\xe8\x51\x05\x6e\x7e\x2b\xdf\x64\xd7\x61\x84\x01\xe5\xca\x04"
      "\x2e\x20\x4d\x5e\x61\x53\x2f\x11\x89\xb7\x99\x0e\xed\x02\x3e\xe3\xdc\xa2"
      "\x65\x65\x9c\x3a\x50\x7b\xe9\x53\x93\x06\x4a\xd5\x8e\xba\x47\xe5\xae\xa5"
      "\x0b\xc5\x6b\x1f\x11\xf2\xef\x89\xcb\x33\x32\x73\x72\x3b\xb3\x22\xf9\x98"
      "\xa2\xf0\xca\x8c\x5f\x3f\x89\x26\x5c\x8c\xe6\x7c\xb7\xec\xcf\x5a\x99\xe1"
      "\xa6\x33\xda\xbd\x5b\x0e\xb5\x50\x78\xdf\x9e\xb7\x6f\x99\xab\x1e\x27\xae"
      "\xba\xc8\x29\x3b\x29\xd5\x7f\xd9\x27\xce\x2c\xeb\x36\x30\x99\xf2\xb1\xd9"
      "\x71\x1b\xf5\x45\x13\x59\x76\x31\x14\xf2\x2c\x0a\x8d\xca\x23\xa0\x24\xe6"
      "\xec\x24\x37\x81\x34\xf1\x6a\xcb\x4e\xc7\xe3\x31\x55\xfa\xde\x81\x92\x09"
      "\x46\x1f\x97\x91\x20\xcd\x5e\x69\x3a\xfc\x97\x4b\x2f\xcc\x42\x9a\x34\xd2"
      "\x3d\xe5\xf9\x2c\x5d\xac\x86\x88\xca\xc4\x00\xe5\x55\xad\x05\x0e\xfe\xb0"
      "\xf4\xcc\x2e\x8e\xd0\x06\x35\xdf\xa0\x4f\xc9\x27\x25\xf1\x01\xf4\xe2\x07"
      "\x7d\x6d\x5f\xc9\xbf\x4e\x17\x7a\xdf\xc4\x8f\xba\x2c\x50\xb2\x70\xdf\x63"
      "\x9e\x21\x61\xfa\x87\x2b\xfb\x8b\x23\x14\xad\x8f\xab\x7d\x99\xff\x7c\xe6"
      "\x08\xe4\x98\x31\x3f\xce\xd0\xd4\x92\x99\x06\x30\x68\x60\x2d\xa0\x89\x98"
      "\x67\x59\x95\xfe\x8f\x98\x05\x20\x89\x17\xe9\x81\x00\x33\x3f\x08\xd5\xc4"
      "\x13\xd8\x6d\x06\x7e\xbb\x1c\x11\x2e\x7a\x6f\xda\x4e\xec\x46\x22\xce\xf2"
      "\x35\xdd\x5a\x43\x85\x47\x00\x37\x05\xa6\xfd\x02\x6e\x95\x82\x03\x9d\x73"
      "\x17\x66\x43\xa4\x50\x5f\x29\x82\x44\xf5\x99\x92\xca\x50\xd3\xc8\x1d\x3d"
      "\x1c\xeb\x50\x13\x6c\x3b\xe9\x99\x2c\x34\x30\x56\x4e\x9c\xd5\x8d\x34\xd0"
      "\x1c\xb4\x31\x67\x8d\x76\x93\x92\x38\x5b\x29\xa4\x74\x5d\xb6\x6d\x27\xfe"
      "\x1c\x74\xc7\xa4\xb0\x23\x59\x57\x5d\xa8\x87\x97\x06\xd4\x22\xed\x58\x0e"
      "\x78\x27\xfb\x0e\x92\x5b\xf6\xac\x8e\x63\xef\x12\xc9\xc9\x05\x73\xd2\x57"
      "\x29\x9c\x94\x8f\x7f\xaf\x51\x24\xca\xff\xde\xa9\xeb\x90\x0c\x8c\xed\x18"
      "\xe4\x9a\xfd\x7c\x48\x18\xc7\xec\x89\xdb\x9e\x6a\xd2\xa3\x56\xe7\x84\x68"
      "\x11\x80\xd8\x6f\x5d\x43\xc1\x9b\x4f\xb4\xea\x15\xd1\x2d\xea\x18\xc2\x46"
      "\xf4\xe2\x4d\x50\xaf\x59\x0c\x16\x33\x63\xd5\xdb\x53\x0a\xdb\x69\xad\xfb"
      "\xd6\x70\xc5\x38\xda\x37\x98\xf6\x90\x75\x7b\xd8\x9c\xa5\xb4\x34\xbd\x73"
      "\x8a\xf6\x8d\x8a\xea\xcb\x3a\xef\x3f\x91\xa8\x49\xef\x8c\xdc\x8e\x91\xfe"
      "\x6e\xee\xf6\xf9\xf0\xfc\x91\x90\xbd\xa5\x78\x6a\x52\x5f\x9c\x98\x61\xdd"
      "\xa5\x2c\x39\x54\xe4\xe2\x06\xdb\x93\xab\xf8\x56\x21\xbe\x4f\xde\xf5\x0b"
      "\x20\x77\xbb\xfb\xa2\x0a\xaf\x82\x17\x94\x4b\xcf\x2e\x2c\x2a\x88\x3a\x22"
      "\x9d\xf8\xb0\x54\xee\xb4\x40\xf4\xac\x41\x93\xd0\x90\x24\xfa\x1d\xc8\x21"
      "\x2a\x0f\x72\xe7\x7d\x85\xa9\x8e\x1f\x7e\x04\x9f\x87\xc0\x6a\x92\x38\x25"
      "\xce\x8c\xbc\x48\xcb\xb5\xea\xce\xbf\xeb\x7e\xf7\x39\x5f\x77\x9b\xfa\x36"
      "\xe0\x64\xe1\xa5\x6c\xca\x75\x7a\x79\xd1\x76\x15\xfe\x8f\x09\x11\x98\x07"
      "\xe9\x11\x52\xaf\x7a\x12\x79\x3e\xdd\xa1\x3c\x0c\x4d\x93\x5c\x71\xf4\x33"
      "\xdf\x47\x97\x44\x5a\x2d\x82\x05\x96\xa9\xce\x88\xfb\x91\x03\x93\x55\x65"
      "\xb2\x5b\x7e\x86\x29\x7b\x0c\x36\x9c\x20\x7f\x63\xe1\x85\xdd\xc5\x4d\x67"
      "\x9b\xa2\x69\x5d\x7a\x9c\x33\xb7\xb2\x72\x0b\xb1\x89\x58\x32\xfc\xde\x7f"
      "\xbc\x04\x42\x39\x4f\x05\x5b\x94\x73\x5b\x8d\x01\x8e\x5b\x30\xc6\x72\xfe"
      "\x71\x38\x78\x08\xd5\x7a\x5b\x3d\x23\x2f\xb6\x57\x49\xc2\x00\x8a\x8b\x0f"
      "\x5a\x17\xc6\x24\x38\xa9\x69\x2f\x31\x77\x78\x23\x7f\xc2\xf7\x34\x37\xd5"
      "\x01\x1b\xdd\xc4\xf2\x25\xd5\x19\x98\x7c\x8a\x74\x63\xe5\x2e\xe8\xb8\x39"
      "\x3b\x18\x87\x38\xb3\xf7\xdf\x46\xa9\x96\x9f\xbb\x55\x6d\x36\x81\xed\x6d"
      "\x9e\x97\xbc\x45\x7b\x03\x7b\x0a\xd4\x08\xcf\x75\xb3\x75\xc3\x81\x87\x0f"
      "\x2c\x0c\x86\x00\x3b\x4a\xb5\xce\x5b\x61\xf3\xf6\xf7\xea\xf0\x90\x26\xb2"
      "\xb4\x36\xae\xc0\x0d\x5e\xe3\x99\x32\xd9\xf3\x28\x28\x9b\xb4\x29\x55\x39"
      "\xeb\x99\xf6\xbd\xa9\x78\x18\xe6\xe6\xd7\x2e\xa8\x16\x6b\xf8\x1d\xbf\xf9"
      "\x2b\x3e\x46\x58\xa3\xa6\xc9\xcb\xe8\x29\xd7\x95\xaf\x92\xcf\xa6\x65\xf5"
      "\x57\x16\x48\x0f\xde\x51\x80\xba\xb7\xbc\x9c\x47\xc4\xab\xf2\x58\xd2\x8f"
      "\x58\x98\x3e\xee\xb7\x50\xb1\xe9\xa8\xfc\x42\x7b\x14\x53\x72\x91\x38\xad"
      "\xd0\x1d\xb5\x23\x3d\x31\xd6\x24\x7e\x56\xcd\xb4\xe7\xeb\x73\x4b\xa5\x70"
      "\xce\x3e\xfe\xde\x90\xae\xdc\x77\x18\xe1\xc4\xd0\xf1\x9b\x18\x12\xec\x6a"
      "\x3c\xe1\xf9\xd3\xfe\x92\x01\xef\xf9\xea\x25\xda\x71\x27\x25\x51\xfd\xe3"
      "\x20\x38\xd6\x1a\xac\xc9\xd0\x55\xda\x7d\x81\xcb\x69\x05\x28\x89\x6a\xc2"
      "\x22\x68\xaa\x00\x9c\x67\x1a\x9e\x43\x61\xb0\xf5\x86\xe0\xf4\xde\x52\x80"
      "\x3a\xdc\x34\x24\x17\xa5\xcd\x24\x39\x75\xbc\x1f\x90\x5d\xbd\x7c\x3e\x40"
      "\x4d\xfe\xc1\x8b\xf6\x7b\x2f\xc5\xc9\x2c\x01\x1c\xe5\x9b\xf5\x3c\x54\x8c"
      "\x2c\x79\xd4\x8a\xcc\x71\x4d\x85\x24\xca\x5e\x6c\xdd\x93\x9e\xa3\xbd\xe6"
      "\xda\xf9\x84\xd1\xfa\x92\x21\xce\x0d\x57\x73\x81\x03\xa5\xe6\x79\xad\x9d"
      "\xb8\x20\x05\xbd\x82\x01\xd8",
      2077);
  *(uint64_t*)0x200003c8 = 0x1040;
  *(uint64_t*)0x20000458 = 1;
  *(uint64_t*)0x20000460 = 0;
  *(uint64_t*)0x20000468 = 0;
  *(uint32_t*)0x20000470 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000440ul, 0ul);
  return 0;
}
