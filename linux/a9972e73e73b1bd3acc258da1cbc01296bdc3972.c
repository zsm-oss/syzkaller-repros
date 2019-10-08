// WARNING in __flush_work (3)
// https://syzkaller.appspot.com/bug?id=a9972e73e73b1bd3acc258da1cbc01296bdc3972
// status:fixed
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/usb/ch9.h>

unsigned long long procid;

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

#define USB_MAX_EP_NUM 32

struct usb_device_index {
  struct usb_device_descriptor* dev;
  struct usb_config_descriptor* config;
  unsigned config_length;
  struct usb_interface_descriptor* iface;
  struct usb_endpoint_descriptor* eps[USB_MAX_EP_NUM];
  unsigned eps_num;
};

static bool parse_usb_descriptor(char* buffer, size_t length,
                                 struct usb_device_index* index)
{
  if (length <
      sizeof(*index->dev) + sizeof(*index->config) + sizeof(*index->iface))
    return false;
  index->dev = (struct usb_device_descriptor*)buffer;
  index->config = (struct usb_config_descriptor*)(buffer + sizeof(*index->dev));
  index->config_length = length - sizeof(*index->dev);
  index->iface =
      (struct usb_interface_descriptor*)(buffer + sizeof(*index->dev) +
                                         sizeof(*index->config));
  index->eps_num = 0;
  size_t offset = 0;
  while (true) {
    if (offset == length)
      break;
    if (offset + 1 < length)
      break;
    uint8_t length = buffer[offset];
    uint8_t type = buffer[offset + 1];
    if (type == USB_DT_ENDPOINT) {
      index->eps[index->eps_num] =
          (struct usb_endpoint_descriptor*)(buffer + offset);
      index->eps_num++;
    }
    if (index->eps_num == USB_MAX_EP_NUM)
      break;
    offset += length;
  }
  return true;
}

enum usb_fuzzer_event_type {
  USB_FUZZER_EVENT_INVALID,
  USB_FUZZER_EVENT_CONNECT,
  USB_FUZZER_EVENT_DISCONNECT,
  USB_FUZZER_EVENT_SUSPEND,
  USB_FUZZER_EVENT_RESUME,
  USB_FUZZER_EVENT_CONTROL,
};

struct usb_fuzzer_event {
  uint32_t type;
  uint32_t length;
  char data[0];
};

struct usb_fuzzer_init {
  uint64_t speed;
  const char* driver_name;
  const char* device_name;
};

struct usb_fuzzer_ep_io {
  uint16_t ep;
  uint16_t flags;
  uint32_t length;
  char data[0];
};

#define USB_FUZZER_IOCTL_INIT _IOW('U', 0, struct usb_fuzzer_init)
#define USB_FUZZER_IOCTL_RUN _IO('U', 1)
#define USB_FUZZER_IOCTL_EP0_READ _IOWR('U', 2, struct usb_fuzzer_event)
#define USB_FUZZER_IOCTL_EP0_WRITE _IOW('U', 3, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_EP_ENABLE _IOW('U', 4, struct usb_endpoint_descriptor)
#define USB_FUZZER_IOCTL_EP_WRITE _IOW('U', 6, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_CONFIGURE _IO('U', 8)
#define USB_FUZZER_IOCTL_VBUS_DRAW _IOW('U', 9, uint32_t)

int usb_fuzzer_open()
{
  return open("/sys/kernel/debug/usb-fuzzer", O_RDWR);
}

int usb_fuzzer_init(int fd, uint32_t speed, const char* driver,
                    const char* device)
{
  struct usb_fuzzer_init arg;
  arg.speed = speed;
  arg.driver_name = driver;
  arg.device_name = device;
  return ioctl(fd, USB_FUZZER_IOCTL_INIT, &arg);
}

int usb_fuzzer_run(int fd)
{
  return ioctl(fd, USB_FUZZER_IOCTL_RUN, 0);
}

int usb_fuzzer_ep0_read(int fd, struct usb_fuzzer_event* event)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP0_READ, event);
}

int usb_fuzzer_ep0_write(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP0_WRITE, io);
}

int usb_fuzzer_ep_write(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP_WRITE, io);
}

int usb_fuzzer_ep_enable(int fd, struct usb_endpoint_descriptor* desc)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP_ENABLE, desc);
}

int usb_fuzzer_configure(int fd)
{
  return ioctl(fd, USB_FUZZER_IOCTL_CONFIGURE, 0);
}

int usb_fuzzer_vbus_draw(int fd, uint32_t power)
{
  return ioctl(fd, USB_FUZZER_IOCTL_VBUS_DRAW, power);
}

#define USB_MAX_PACKET_SIZE 1024

struct usb_fuzzer_control_event {
  struct usb_fuzzer_event inner;
  struct usb_ctrlrequest ctrl;
};

struct usb_fuzzer_ep_io_data {
  struct usb_fuzzer_ep_io inner;
  char data[USB_MAX_PACKET_SIZE];
};

struct vusb_connect_string_descriptor {
  uint32_t len;
  char* str;
} __attribute__((packed));

struct vusb_connect_descriptors {
  uint32_t qual_len;
  char* qual;
  uint32_t bos_len;
  char* bos;
  uint32_t strs_len;
  struct vusb_connect_string_descriptor strs[0];
} __attribute__((packed));

static volatile long syz_usb_connect(volatile long a0, volatile long a1,
                                     volatile long a2, volatile long a3)
{
  int64_t speed = a0;
  int64_t dev_len = a1;
  char* dev = (char*)a2;
  struct vusb_connect_descriptors* conn_descs =
      (struct vusb_connect_descriptors*)a3;
  if (!dev)
    return -1;
  struct usb_device_index index;
  memset(&index, 0, sizeof(index));
  int rv = parse_usb_descriptor(dev, dev_len, &index);
  if (!rv)
    return -1;
  int fd = usb_fuzzer_open();
  if (fd < 0)
    return -1;
  char device[32];
  sprintf(&device[0], "dummy_udc.%llu", procid);
  rv = usb_fuzzer_init(fd, speed, "dummy_udc", &device[0]);
  if (rv < 0)
    return -1;
  rv = usb_fuzzer_run(fd);
  if (rv < 0)
    return -1;
  bool done = false;
  while (!done) {
    char* response_data = NULL;
    uint32_t response_length = 0;
    unsigned ep;
    uint8_t str_idx;
    struct usb_fuzzer_control_event event;
    event.inner.type = 0;
    event.inner.length = sizeof(event.ctrl);
    rv = usb_fuzzer_ep0_read(fd, (struct usb_fuzzer_event*)&event);
    if (rv < 0)
      return -1;
    if (event.inner.type != USB_FUZZER_EVENT_CONTROL)
      continue;
    switch (event.ctrl.bRequestType & USB_TYPE_MASK) {
    case USB_TYPE_STANDARD:
      switch (event.ctrl.bRequest) {
      case USB_REQ_GET_DESCRIPTOR:
        switch (event.ctrl.wValue >> 8) {
        case USB_DT_DEVICE:
          response_data = (char*)index.dev;
          response_length = sizeof(*index.dev);
          goto reply;
        case USB_DT_CONFIG:
          response_data = (char*)index.config;
          response_length = index.config_length;
          goto reply;
        case USB_DT_STRING:
          str_idx = (uint8_t)event.ctrl.wValue;
          if (str_idx >= conn_descs->strs_len)
            goto reply;
          response_data = conn_descs->strs[str_idx].str;
          response_length = conn_descs->strs[str_idx].len;
          goto reply;
        case USB_DT_BOS:
          response_data = conn_descs->bos;
          response_length = conn_descs->bos_len;
          goto reply;
        case USB_DT_DEVICE_QUALIFIER:
          response_data = conn_descs->qual;
          response_length = conn_descs->qual_len;
          goto reply;
        default:
          exit(1);
          continue;
        }
        break;
      case USB_REQ_SET_CONFIGURATION:
        rv = usb_fuzzer_vbus_draw(fd, index.config->bMaxPower);
        if (rv < 0)
          return -1;
        rv = usb_fuzzer_configure(fd);
        if (rv < 0)
          return -1;
        for (ep = 0; ep < index.eps_num; ep++) {
          rv = usb_fuzzer_ep_enable(fd, index.eps[ep]);
          if (rv < 0)
            exit(1);
        }
        done = true;
        goto reply;
      default:
        exit(1);
        continue;
      }
      break;
    default:
      exit(1);
      continue;
    }
    struct usb_fuzzer_ep_io_data response;
  reply:
    response.inner.ep = 0;
    response.inner.flags = 0;
    if (response_length > sizeof(response.data))
      response_length = 0;
    response.inner.length = response_length;
    if (response_data)
      memcpy(&response.data[0], response_data, response_length);
    if (event.ctrl.wLength < response.inner.length)
      response.inner.length = event.ctrl.wLength;
    usb_fuzzer_ep0_write(fd, (struct usb_fuzzer_ep_io*)&response);
  }
  sleep_ms(200);
  return fd;
}

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);

  *(uint8_t*)0x20000140 = 0x12;
  *(uint8_t*)0x20000141 = 1;
  *(uint16_t*)0x20000142 = 0;
  *(uint8_t*)0x20000144 = 0xd4;
  *(uint8_t*)0x20000145 = 0x8e;
  *(uint8_t*)0x20000146 = 0x9c;
  *(uint8_t*)0x20000147 = 0x20;
  *(uint16_t*)0x20000148 = 0x6f8;
  *(uint16_t*)0x2000014a = 0x300c;
  *(uint16_t*)0x2000014c = 0x6891;
  *(uint8_t*)0x2000014e = 0;
  *(uint8_t*)0x2000014f = 0;
  *(uint8_t*)0x20000150 = 0;
  *(uint8_t*)0x20000151 = 1;
  *(uint8_t*)0x20000152 = 9;
  *(uint8_t*)0x20000153 = 2;
  *(uint16_t*)0x20000154 = 0x40;
  *(uint8_t*)0x20000156 = 1;
  *(uint8_t*)0x20000157 = 0;
  *(uint8_t*)0x20000158 = 0;
  *(uint8_t*)0x20000159 = 0;
  *(uint8_t*)0x2000015a = 0;
  *(uint8_t*)0x2000015b = 9;
  *(uint8_t*)0x2000015c = 4;
  *(uint8_t*)0x2000015d = 0xf6;
  *(uint8_t*)0x2000015e = 0;
  *(uint8_t*)0x2000015f = 0;
  *(uint8_t*)0x20000160 = 0xe;
  *(uint8_t*)0x20000161 = 1;
  *(uint8_t*)0x20000162 = 0;
  *(uint8_t*)0x20000163 = 0;
  *(uint8_t*)0x20000164 = 6;
  *(uint8_t*)0x20000165 = 0x24;
  *(uint8_t*)0x20000166 = 0x1a;
  *(uint16_t*)0x20000167 = 4;
  *(uint8_t*)0x20000169 = 5;
  *(uint8_t*)0x2000016a = 0xd;
  *(uint8_t*)0x2000016b = 0x24;
  *(uint8_t*)0x2000016c = 0xf;
  *(uint8_t*)0x2000016d = 7;
  *(uint32_t*)0x2000016e = 4;
  *(uint16_t*)0x20000172 = 0xfff;
  *(uint16_t*)0x20000174 = 0x81;
  *(uint8_t*)0x20000176 = 0;
  *(uint8_t*)0x20000177 = 4;
  *(uint8_t*)0x20000178 = 0x24;
  *(uint8_t*)0x20000179 = 0x13;
  *(uint8_t*)0x2000017a = 5;
  *(uint8_t*)0x2000017b = 3;
  *(uint8_t*)0x2000017c = 0x24;
  *(uint8_t*)0x2000017d = 0;
  *(uint8_t*)0x2000017e = 7;
  *(uint8_t*)0x2000017f = 0x24;
  *(uint8_t*)0x20000180 = 0x14;
  *(uint16_t*)0x20000181 = 1;
  *(uint16_t*)0x20000183 = 0x3ff;
  *(uint8_t*)0x20000185 = 5;
  *(uint8_t*)0x20000186 = 0x24;
  *(uint8_t*)0x20000187 = 6;
  *(uint8_t*)0x20000188 = 0x9b;
  *(uint8_t*)0x20000189 = -1;
  *(uint8_t*)0x2000018a = 4;
  *(uint8_t*)0x2000018b = 0x24;
  *(uint8_t*)0x2000018c = 1;
  *(uint8_t*)0x2000018d = 1;
  *(uint8_t*)0x2000018e = 4;
  *(uint8_t*)0x2000018f = 0x24;
  *(uint8_t*)0x20000190 = 0x13;
  *(uint8_t*)0x20000191 = 0;
  syz_usb_connect(6, 0x52, 0x20000140, 0);
  return 0;
}
