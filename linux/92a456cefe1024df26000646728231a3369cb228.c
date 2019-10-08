// divide error in usbnet_update_max_qlen
// https://syzkaller.appspot.com/bug?id=92a456cefe1024df26000646728231a3369cb228
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <linux/capability.h>
#include <linux/usb/ch9.h>

unsigned long long procid;

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

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

#define USB_DEBUG 0

#define USB_MAX_IFACE_NUM 4
#define USB_MAX_EP_NUM 32

struct usb_iface_index {
  struct usb_interface_descriptor* iface;
  struct usb_endpoint_descriptor* eps[USB_MAX_EP_NUM];
  unsigned eps_num;
};

struct usb_device_index {
  struct usb_device_descriptor* dev;
  struct usb_config_descriptor* config;
  unsigned config_length;
  struct usb_iface_index ifaces[USB_MAX_IFACE_NUM];
  unsigned ifaces_num;
};

static bool parse_usb_descriptor(char* buffer, size_t length,
                                 struct usb_device_index* index)
{
  if (length < sizeof(*index->dev) + sizeof(*index->config))
    return false;
  memset(index, 0, sizeof(*index));
  index->dev = (struct usb_device_descriptor*)buffer;
  index->config = (struct usb_config_descriptor*)(buffer + sizeof(*index->dev));
  index->config_length = length - sizeof(*index->dev);
  size_t offset = 0;
  while (true) {
    if (offset + 1 >= length)
      break;
    uint8_t desc_length = buffer[offset];
    uint8_t desc_type = buffer[offset + 1];
    if (desc_length <= 2)
      break;
    if (offset + desc_length > length)
      break;
    if (desc_type == USB_DT_INTERFACE &&
        index->ifaces_num < USB_MAX_IFACE_NUM) {
      struct usb_interface_descriptor* iface =
          (struct usb_interface_descriptor*)(buffer + offset);
      index->ifaces[index->ifaces_num++].iface = iface;
    }
    if (desc_type == USB_DT_ENDPOINT && index->ifaces_num > 0) {
      struct usb_iface_index* iface = &index->ifaces[index->ifaces_num - 1];
      if (iface->eps_num < USB_MAX_EP_NUM)
        iface->eps[iface->eps_num++] =
            (struct usb_endpoint_descriptor*)(buffer + offset);
    }
    offset += desc_length;
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
#define USB_FUZZER_IOCTL_EVENT_FETCH _IOR('U', 2, struct usb_fuzzer_event)
#define USB_FUZZER_IOCTL_EP0_WRITE _IOW('U', 3, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_EP0_READ _IOWR('U', 4, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_EP_ENABLE _IOW('U', 5, struct usb_endpoint_descriptor)
#define USB_FUZZER_IOCTL_EP_WRITE _IOW('U', 7, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_EP_READ _IOWR('U', 8, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_CONFIGURE _IO('U', 9)
#define USB_FUZZER_IOCTL_VBUS_DRAW _IOW('U', 10, uint32_t)

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

int usb_fuzzer_event_fetch(int fd, struct usb_fuzzer_event* event)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EVENT_FETCH, event);
}

int usb_fuzzer_ep0_write(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP0_WRITE, io);
}

int usb_fuzzer_ep0_read(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP0_READ, io);
}

int usb_fuzzer_ep_write(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP_WRITE, io);
}

int usb_fuzzer_ep_read(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP_READ, io);
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
  char data[USB_MAX_PACKET_SIZE];
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

static const char default_string[] = {8, USB_DT_STRING, 's', 0, 'y', 0, 'z', 0};

static const char default_lang_id[] = {4, USB_DT_STRING, 0x09, 0x04};

static bool lookup_connect_response(struct vusb_connect_descriptors* descs,
                                    struct usb_device_index* index,
                                    struct usb_ctrlrequest* ctrl,
                                    char** response_data,
                                    uint32_t* response_length)
{
  uint8_t str_idx;
  switch (ctrl->bRequestType & USB_TYPE_MASK) {
  case USB_TYPE_STANDARD:
    switch (ctrl->bRequest) {
    case USB_REQ_GET_DESCRIPTOR:
      switch (ctrl->wValue >> 8) {
      case USB_DT_DEVICE:
        *response_data = (char*)index->dev;
        *response_length = sizeof(*index->dev);
        return true;
      case USB_DT_CONFIG:
        *response_data = (char*)index->config;
        *response_length = index->config_length;
        return true;
      case USB_DT_STRING:
        str_idx = (uint8_t)ctrl->wValue;
        if (descs && str_idx < descs->strs_len) {
          *response_data = descs->strs[str_idx].str;
          *response_length = descs->strs[str_idx].len;
          return true;
        }
        if (str_idx == 0) {
          *response_data = (char*)&default_lang_id[0];
          *response_length = default_lang_id[0];
          return true;
        }
        *response_data = (char*)&default_string[0];
        *response_length = default_string[0];
        return true;
      case USB_DT_BOS:
        *response_data = descs->bos;
        *response_length = descs->bos_len;
        return true;
      case USB_DT_DEVICE_QUALIFIER:
        *response_data = descs->qual;
        *response_length = descs->qual_len;
        return true;
      default:
        exit(1);
        return false;
      }
      break;
    default:
      exit(1);
      return false;
    }
    break;
  default:
    exit(1);
    return false;
  }
  return false;
}

static volatile long syz_usb_connect(volatile long a0, volatile long a1,
                                     volatile long a2, volatile long a3)
{
  uint64_t speed = a0;
  uint64_t dev_len = a1;
  char* dev = (char*)a2;
  struct vusb_connect_descriptors* descs = (struct vusb_connect_descriptors*)a3;
  if (!dev) {
    return -1;
  }
  struct usb_device_index index;
  memset(&index, 0, sizeof(index));
  int rv = 0;
  rv = parse_usb_descriptor(dev, dev_len, &index);
  if (!rv) {
    return rv;
  }
  int fd = usb_fuzzer_open();
  if (fd < 0) {
    return fd;
  }
  char device[32];
  sprintf(&device[0], "dummy_udc.%llu", procid);
  rv = usb_fuzzer_init(fd, speed, "dummy_udc", &device[0]);
  if (rv < 0) {
    return rv;
  }
  rv = usb_fuzzer_run(fd);
  if (rv < 0) {
    return rv;
  }
  bool done = false;
  while (!done) {
    struct usb_fuzzer_control_event event;
    event.inner.type = 0;
    event.inner.length = sizeof(event.ctrl);
    rv = usb_fuzzer_event_fetch(fd, (struct usb_fuzzer_event*)&event);
    if (rv < 0) {
      return rv;
    }
    if (event.inner.type != USB_FUZZER_EVENT_CONTROL)
      continue;
    bool response_found = false;
    char* response_data = NULL;
    uint32_t response_length = 0;
    if (event.ctrl.bRequestType & USB_DIR_IN) {
      response_found = lookup_connect_response(
          descs, &index, &event.ctrl, &response_data, &response_length);
      if (!response_found) {
        return -1;
      }
    } else {
      if ((event.ctrl.bRequestType & USB_TYPE_MASK) != USB_TYPE_STANDARD ||
          event.ctrl.bRequest != USB_REQ_SET_CONFIGURATION) {
        exit(1);
        return -1;
      }
      done = true;
    }
    if (done) {
      rv = usb_fuzzer_vbus_draw(fd, index.config->bMaxPower);
      if (rv < 0) {
        return rv;
      }
      rv = usb_fuzzer_configure(fd);
      if (rv < 0) {
        return rv;
      }
      unsigned ep;
      for (ep = 0; ep < index.ifaces[0].eps_num; ep++) {
        rv = usb_fuzzer_ep_enable(fd, index.ifaces[0].eps[ep]);
        if (rv < 0) {
        } else {
        }
      }
    }
    struct usb_fuzzer_ep_io_data response;
    response.inner.ep = 0;
    response.inner.flags = 0;
    if (response_length > sizeof(response.data))
      response_length = 0;
    if (event.ctrl.wLength < response_length)
      response_length = event.ctrl.wLength;
    response.inner.length = response_length;
    if (response_data)
      memcpy(&response.data[0], response_data, response_length);
    else
      memset(&response.data[0], 0, response_length);
    if (event.ctrl.bRequestType & USB_DIR_IN) {
      rv = usb_fuzzer_ep0_write(fd, (struct usb_fuzzer_ep_io*)&response);
    } else {
      rv = usb_fuzzer_ep0_read(fd, (struct usb_fuzzer_ep_io*)&response);
    }
    if (rv < 0) {
      return rv;
    }
  }
  sleep_ms(200);
  return fd;
}

struct vusb_descriptor {
  uint8_t req_type;
  uint8_t desc_type;
  uint32_t len;
  char data[0];
} __attribute__((packed));

struct vusb_descriptors {
  uint32_t len;
  struct vusb_descriptor* generic;
  struct vusb_descriptor* descs[0];
} __attribute__((packed));

struct vusb_response {
  uint8_t type;
  uint8_t req;
  uint32_t len;
  char data[0];
} __attribute__((packed));

struct vusb_responses {
  uint32_t len;
  struct vusb_response* generic;
  struct vusb_response* resps[0];
} __attribute__((packed));

static bool lookup_control_response(struct vusb_descriptors* descs,
                                    struct vusb_responses* resps,
                                    struct usb_ctrlrequest* ctrl,
                                    char** response_data,
                                    uint32_t* response_length)
{
  int descs_num = 0;
  int resps_num = 0;
  if (descs)
    descs_num = (descs->len - offsetof(struct vusb_descriptors, descs)) /
                sizeof(descs->descs[0]);
  if (resps)
    resps_num = (resps->len - offsetof(struct vusb_responses, resps)) /
                sizeof(resps->resps[0]);
  uint8_t req = ctrl->bRequest;
  uint8_t req_type = ctrl->bRequestType & USB_TYPE_MASK;
  uint8_t desc_type = ctrl->wValue >> 8;
  if (req == USB_REQ_GET_DESCRIPTOR) {
    int i;
    for (i = 0; i < descs_num; i++) {
      struct vusb_descriptor* desc = descs->descs[i];
      if (!desc)
        continue;
      if (desc->req_type == req_type && desc->desc_type == desc_type) {
        *response_length = desc->len;
        if (*response_length != 0)
          *response_data = &desc->data[0];
        else
          *response_data = NULL;
        return true;
      }
    }
    if (descs && descs->generic) {
      *response_data = &descs->generic->data[0];
      *response_length = descs->generic->len;
      return true;
    }
  } else {
    int i;
    for (i = 0; i < resps_num; i++) {
      struct vusb_response* resp = resps->resps[i];
      if (!resp)
        continue;
      if (resp->type == req_type && resp->req == req) {
        *response_length = resp->len;
        if (*response_length != 0)
          *response_data = &resp->data[0];
        else
          *response_data = NULL;
        return true;
      }
    }
    if (resps && resps->generic) {
      *response_data = &resps->generic->data[0];
      *response_length = resps->generic->len;
      return true;
    }
  }
  return false;
}

static volatile long syz_usb_control_io(volatile long a0, volatile long a1,
                                        volatile long a2)
{
  int fd = a0;
  struct vusb_descriptors* descs = (struct vusb_descriptors*)a1;
  struct vusb_responses* resps = (struct vusb_responses*)a2;
  struct usb_fuzzer_control_event event;
  event.inner.type = 0;
  event.inner.length = USB_MAX_PACKET_SIZE;
  int rv = usb_fuzzer_event_fetch(fd, (struct usb_fuzzer_event*)&event);
  if (rv < 0) {
    return rv;
  }
  if (event.inner.type != USB_FUZZER_EVENT_CONTROL) {
    return -1;
  }
  bool response_found = false;
  char* response_data = NULL;
  uint32_t response_length = 0;
  if (event.ctrl.bRequestType & USB_DIR_IN) {
    response_found = lookup_control_response(descs, resps, &event.ctrl,
                                             &response_data, &response_length);
    if (!response_found) {
      return -1;
    }
  } else {
    response_length = event.ctrl.wLength;
  }
  struct usb_fuzzer_ep_io_data response;
  response.inner.ep = 0;
  response.inner.flags = 0;
  if (response_length > sizeof(response.data))
    response_length = 0;
  if (event.ctrl.wLength < response_length)
    response_length = event.ctrl.wLength;
  response.inner.length = response_length;
  if (response_data)
    memcpy(&response.data[0], response_data, response_length);
  else
    memset(&response.data[0], 0, response_length);
  if (event.ctrl.bRequestType & USB_DIR_IN) {
    rv = usb_fuzzer_ep0_write(fd, (struct usb_fuzzer_ep_io*)&response);
  } else {
    rv = usb_fuzzer_ep0_read(fd, (struct usb_fuzzer_ep_io*)&response);
  }
  if (rv < 0) {
    return rv;
  }
  sleep_ms(200);
  return 0;
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

int wait_for_loop(int pid)
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
  loop();
  exit(1);
}

static void close_fds()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
    close(fd);
}

uint64_t r[1] = {0xffffffffffffffff};

void loop(void)
{
  intptr_t res = 0;
  *(uint8_t*)0x20001580 = 0x12;
  *(uint8_t*)0x20001581 = 1;
  *(uint16_t*)0x20001582 = 0;
  *(uint8_t*)0x20001584 = 2;
  *(uint8_t*)0x20001585 = 0;
  *(uint8_t*)0x20001586 = 0;
  *(uint8_t*)0x20001587 = 0x40;
  *(uint16_t*)0x20001588 = 0x525;
  *(uint16_t*)0x2000158a = 0xa4a1;
  *(uint16_t*)0x2000158c = 0x40;
  *(uint8_t*)0x2000158e = 0;
  *(uint8_t*)0x2000158f = 0;
  *(uint8_t*)0x20001590 = 0;
  *(uint8_t*)0x20001591 = 1;
  *(uint8_t*)0x20001592 = 9;
  *(uint8_t*)0x20001593 = 2;
  *(uint16_t*)0x20001594 = 0x5c;
  *(uint8_t*)0x20001596 = 1;
  *(uint8_t*)0x20001597 = 1;
  *(uint8_t*)0x20001598 = 0;
  *(uint8_t*)0x20001599 = 0;
  *(uint8_t*)0x2000159a = 0;
  *(uint8_t*)0x2000159b = 9;
  *(uint8_t*)0x2000159c = 4;
  *(uint8_t*)0x2000159d = 0;
  *(uint8_t*)0x2000159e = 0;
  *(uint8_t*)0x2000159f = 1;
  *(uint8_t*)0x200015a0 = 2;
  *(uint8_t*)0x200015a1 = 0xd;
  *(uint8_t*)0x200015a2 = 0;
  *(uint8_t*)0x200015a3 = 0;
  *(uint8_t*)0x200015a4 = 5;
  *(uint8_t*)0x200015a5 = 0x24;
  *(uint8_t*)0x200015a6 = 6;
  *(uint8_t*)0x200015a7 = 0;
  *(uint8_t*)0x200015a8 = 1;
  *(uint8_t*)0x200015a9 = 5;
  *(uint8_t*)0x200015aa = 0x24;
  *(uint8_t*)0x200015ab = 0;
  *(uint16_t*)0x200015ac = 0;
  *(uint8_t*)0x200015ae = 0xd;
  *(uint8_t*)0x200015af = 0x24;
  *(uint8_t*)0x200015b0 = 0xf;
  *(uint8_t*)0x200015b1 = 1;
  *(uint32_t*)0x200015b2 = 0;
  *(uint16_t*)0x200015b6 = 0;
  *(uint16_t*)0x200015b8 = 0;
  *(uint8_t*)0x200015ba = 0;
  *(uint8_t*)0x200015bb = 6;
  *(uint8_t*)0x200015bc = 0x24;
  *(uint8_t*)0x200015bd = 0x1a;
  *(uint16_t*)0x200015be = 0;
  *(uint8_t*)0x200015c0 = 0xfc;
  *(uint8_t*)0x200015c1 = 9;
  *(uint8_t*)0x200015c2 = 5;
  *(uint8_t*)0x200015c3 = 0x81;
  *(uint8_t*)0x200015c4 = 3;
  *(uint16_t*)0x200015c5 = 0;
  *(uint8_t*)0x200015c7 = 0;
  *(uint8_t*)0x200015c8 = 0;
  *(uint8_t*)0x200015c9 = 0;
  *(uint8_t*)0x200015ca = 9;
  *(uint8_t*)0x200015cb = 4;
  *(uint8_t*)0x200015cc = 1;
  *(uint8_t*)0x200015cd = 0;
  *(uint8_t*)0x200015ce = 0;
  *(uint8_t*)0x200015cf = 2;
  *(uint8_t*)0x200015d0 = 0xd;
  *(uint8_t*)0x200015d1 = 0;
  *(uint8_t*)0x200015d2 = 0;
  *(uint8_t*)0x200015d3 = 9;
  *(uint8_t*)0x200015d4 = 4;
  *(uint8_t*)0x200015d5 = 1;
  *(uint8_t*)0x200015d6 = 1;
  *(uint8_t*)0x200015d7 = 2;
  *(uint8_t*)0x200015d8 = 2;
  *(uint8_t*)0x200015d9 = 0xd;
  *(uint8_t*)0x200015da = 0;
  *(uint8_t*)0x200015db = 0;
  *(uint8_t*)0x200015dc = 9;
  *(uint8_t*)0x200015dd = 5;
  *(uint8_t*)0x200015de = 0x82;
  *(uint8_t*)0x200015df = 2;
  *(uint16_t*)0x200015e0 = 0;
  *(uint8_t*)0x200015e2 = 0;
  *(uint8_t*)0x200015e3 = 0;
  *(uint8_t*)0x200015e4 = 0;
  *(uint8_t*)0x200015e5 = 9;
  *(uint8_t*)0x200015e6 = 5;
  *(uint8_t*)0x200015e7 = 3;
  *(uint8_t*)0x200015e8 = 2;
  *(uint16_t*)0x200015e9 = 0;
  *(uint8_t*)0x200015eb = 0;
  *(uint8_t*)0x200015ec = 0;
  *(uint8_t*)0x200015ed = 0;
  res = syz_usb_connect(0, 0x6e, 0x20001580, 0);
  if (res != -1)
    r[0] = res;
  syz_usb_control_io(r[0], 0, 0);
  syz_usb_control_io(r[0], 0, 0);
  *(uint32_t*)0x20000880 = 0xac;
  *(uint64_t*)0x20000884 = 0x200002c0;
  *(uint8_t*)0x200002c0 = 0;
  *(uint8_t*)0x200002c1 = 0x16;
  *(uint32_t*)0x200002c2 = 0;
  *(uint64_t*)0x2000088c = 0;
  *(uint64_t*)0x20000894 = 0;
  *(uint64_t*)0x2000089c = 0;
  *(uint64_t*)0x200008a4 = 0;
  *(uint64_t*)0x200008ac = 0;
  *(uint64_t*)0x200008b4 = 0;
  *(uint64_t*)0x200008bc = 0;
  *(uint64_t*)0x200008c4 = 0;
  *(uint64_t*)0x200008cc = 0;
  *(uint64_t*)0x200008d4 = 0;
  *(uint64_t*)0x200008dc = 0;
  *(uint64_t*)0x200008e4 = 0;
  *(uint64_t*)0x200008ec = 0;
  *(uint64_t*)0x200008f4 = 0;
  *(uint64_t*)0x200008fc = 0;
  *(uint64_t*)0x20000904 = 0;
  *(uint64_t*)0x2000090c = 0;
  *(uint64_t*)0x20000914 = 0;
  *(uint64_t*)0x2000091c = 0;
  *(uint64_t*)0x20000924 = 0;
  syz_usb_control_io(r[0], 0, 0x20000880);
  syz_usb_control_io(r[0], 0, 0);
  syz_usb_control_io(r[0], 0, 0);
  *(uint32_t*)0x200011c0 = 0x14;
  *(uint64_t*)0x200011c4 = 0;
  *(uint64_t*)0x200011cc = 0x20000180;
  *(uint8_t*)0x20000180 = 0;
  *(uint8_t*)0x20000181 = 3;
  *(uint32_t*)0x20000182 = 0x1a;
  *(uint8_t*)0x20000186 = 0x1a;
  *(uint8_t*)0x20000187 = 3;
  *(uint64_t*)0x20000188 = htobe64(0x3400320034003200);
  *(uint64_t*)0x20000190 = htobe64(0x3400320034003200);
  *(uint64_t*)0x20000198 = htobe64(0x3400320034003200);
  syz_usb_control_io(r[0], 0x200011c0, 0);
  *(uint32_t*)0x20000a40 = 0x14;
  *(uint64_t*)0x20000a44 = 0;
  *(uint64_t*)0x20000a4c = 0x20000a00;
  *(uint8_t*)0x20000a00 = 0;
  *(uint8_t*)0x20000a01 = 3;
  *(uint32_t*)0x20000a02 = 0x1a;
  *(uint8_t*)0x20000a06 = 0x1a;
  *(uint8_t*)0x20000a07 = 3;
  *(uint64_t*)0x20000a08 = htobe64(0x3400320034003200);
  *(uint64_t*)0x20000a10 = htobe64(0x3400320034003200);
  *(uint64_t*)0x20000a18 = htobe64(0x3400320034003200);
  syz_usb_control_io(r[0], 0x20000a40, 0);
  close_fds();
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  do_sandbox_none();
  return 0;
}
