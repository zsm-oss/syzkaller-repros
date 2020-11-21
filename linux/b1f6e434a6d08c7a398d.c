// KASAN: slab-out-of-bounds Read in squashfs_export_iget
// https://syzkaller.appspot.com/bug?id=b1f6e434a6d08c7a398d
// status:0
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/loop.h>

static unsigned long long procid;

struct fs_image_segment {
  void* data;
  uintptr_t size;
  uintptr_t offset;
};

#define IMAGE_MAX_SEGMENTS 4096
#define IMAGE_MAX_SIZE (129 << 20)

#define sys_memfd_create 319

static unsigned long fs_image_segment_check(unsigned long size,
                                            unsigned long nsegs,
                                            struct fs_image_segment* segs)
{
  if (nsegs > IMAGE_MAX_SEGMENTS)
    nsegs = IMAGE_MAX_SEGMENTS;
  for (size_t i = 0; i < nsegs; i++) {
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
  return size;
}
static int setup_loop_device(long unsigned size, long unsigned nsegs,
                             struct fs_image_segment* segs,
                             const char* loopname, int* memfd_p, int* loopfd_p)
{
  int err = 0, loopfd = -1;
  size = fs_image_segment_check(size, nsegs, segs);
  int memfd = syscall(sys_memfd_create, "syzkaller", 0);
  if (memfd == -1) {
    err = errno;
    goto error;
  }
  if (ftruncate(memfd, size)) {
    err = errno;
    goto error_close_memfd;
  }
  for (size_t i = 0; i < nsegs; i++) {
    if (pwrite(memfd, segs[i].data, segs[i].size, segs[i].offset) < 0) {
    }
  }
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
  *memfd_p = memfd;
  *loopfd_p = loopfd;
  return 0;

error_close_loop:
  close(loopfd);
error_close_memfd:
  close(memfd);
error:
  errno = err;
  return -1;
}

static long syz_mount_image(volatile long fsarg, volatile long dir,
                            volatile unsigned long size,
                            volatile unsigned long nsegs,
                            volatile long segments, volatile long flags,
                            volatile long optsarg)
{
  struct fs_image_segment* segs = (struct fs_image_segment*)segments;
  int res = -1, err = 0, loopfd = -1, memfd = -1, need_loop_device = !!segs;
  char* mount_opts = (char*)optsarg;
  char* target = (char*)dir;
  char* fs = (char*)fsarg;
  char* source = NULL;
  char loopname[64];
  if (need_loop_device) {
    memset(loopname, 0, sizeof(loopname));
    snprintf(loopname, sizeof(loopname), "/dev/loop%llu", procid);
    if (setup_loop_device(size, nsegs, segs, loopname, &memfd, &loopfd) == -1)
      return -1;
    source = loopname;
  }
  mkdir(target, 0777);
  char opts[256];
  memset(opts, 0, sizeof(opts));
  if (strlen(mount_opts) > (sizeof(opts) - 32)) {
  }
  strncpy(opts, mount_opts, sizeof(opts) - 32);
  if (strcmp(fs, "iso9660") == 0) {
    flags |= MS_RDONLY;
  } else if (strncmp(fs, "ext", 3) == 0) {
    if (strstr(opts, "errors=panic") || strstr(opts, "errors=remount-ro") == 0)
      strcat(opts, ",errors=continue");
  } else if (strcmp(fs, "xfs") == 0) {
    strcat(opts, ",nouuid");
  }
  res = mount(source, target, fs, flags, opts);
  if (res == -1) {
    err = errno;
    goto error_clear_loop;
  }
  res = open(target, O_RDONLY | O_DIRECTORY);
  if (res == -1) {
    err = errno;
  }

error_clear_loop:
  if (need_loop_device) {
    ioctl(loopfd, LOOP_CLR_FD, 0);
    close(loopfd);
    close(memfd);
  }
  errno = err;
  return res;
}

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  memcpy((void*)0x20000000, "squashfs\000", 9);
  memcpy((void*)0x200000c0, "./file0\000", 8);
  *(uint64_t*)0x20000080 = 0x20000400;
  memcpy((void*)0x20000400,
         "\x68\x73\x71\x73\x07\x00\x00\x00\x91\x1d\x67\x5f\x00\x10\x00\x00\x07"
         "\x00\x00\x00\x01\x00\x0c\x00\xe0\x00\x02\x00\x04\x00\x00\x00\x1a\x01"
         "\x00\x00\x00\x00\x00\x00\xf5\x01\x00\x00\x00\x00\x00\x00\xa4\x01\x00"
         "\x00\x00\x00\x00\x00\xdd\x01\x00\x00\x00\x00\x00\x00\x81\x00\x00\x00"
         "\x00\x00\x00\x00\x15\x01\x00\x00\x00\x00\x00\x00\x6c\x01\x00\x00\x00"
         "\x00\x00\x00\x92\x01\x00\x00\x00\x00\x00\x00\x78\xda\x2b\xae\xac\xca"
         "\x4e\xcc\xc9\x49\x2d\x2a\xa6\x1d\xa3\x98\x0e\x76\x8c\x32\x46\x19\xa3"
         "\x8c\xa1\xc6\x40\x70\x01\x34\x43\xf4\x14\x92\x00\x78\xda\x63\x62\x78"
         "\xcb\xc8\xc0\xc0\xc8\x30\x51\x36\x3d\x1e\xc8\x40\x01\x29\x40\xcc\x84"
         "\x24\xcf\x8c\x26\x27\xc5\xc2\xc0\xc0\xcc\xf0\x1f\x2e\x0f\xe4\x32\x80"
         "\xcc\x50\x03\x62\xfd\x92\xdc\x02\xfd\xe2\xca\x2a\xdd\xcc\xdc\xc4\xf4"
         "\xd4\xf4\xd4\x3c\x13\x43\x4b\x73\x73\x33\x63\x4b\x23\xfd\xb4\xcc\x9c"
         "\x54\x03\x08\xc9\x88\x64\x3a\x13\xd4\x64\x10\xad\x09\xc4\xec\x40\xcc"
         "\x89\x24\xcf\x8a\x64\x3b\x17\x9a\x4b\x61\x2e\xaf\x63\x81\xd0\xc8\xfa"
         "\xd8\x80\xfc\x04\xa8\xbc\x86\x32\xaa\x3e\x90\x5d\xff\x81\x80\x01\x89"
         "\x46\x98\x79\x00\x6e\x06\x3b\x54\x0c\x14\x02\x21\x40\xff\x71\x00\x69"
         "\x00\x62\xdd\x28\x25\x45\x00\x78\xda\x63\x64\x80\x00\x66\x20\x56\x00"
         "\x62\x26\x06\x16\x86\xb4\xcc\x9c\x54\x03\x07\x06\x46\xa0\x20\x84\x63"
         "\xc8\x02\x55\xc5\x08\xa5\x99\x18\x38\xc0\x12\x7a\xc9\xf9\x39\x29\x75"
         "\x40\x61\x46\x98\xb6\x79\x40\x06\xcc\x0c\xc3\x6b\x0c\xac\x70\x8e\x11"
         "\x32\xc7\x18\x00\x38\xd7\x11\xc7\x0e\x00\x78\xda\x4b\x60\x80\x00\x45"
         "\x28\x0d\x00\x07\x18\x00\x82\x5c\x01\x00\x00\x00\x00\x00\x00\x1c\x00"
         "\x78\xda\x63\x60\x80\x80\x3a\x28\xad\x00\xa5\x1d\xa0\xf4\x3c\x28\x7d"
         "\x0d\x4a\x4b\x31\x42\x68\x00\x41\xdf\x02\x6e\x74\x01\x00\x00\x00\x00"
         "\x00\x00\x08\x80\x5c\xf9\x01\x00\x53\x5f\x01\x00\x9a\x01\x00\x00\x00"
         "\x00\x00\x00\x1d\x00\x78\xda\x63\x60\x60\x63\xa8\x48\x2c\x29\x29\x32"
         "\x64\x63\x60\x80\xb2\x18\x60\x62\x46\x70\x31\x23\x00\xb5\xbc\x09\xab"
         "\x10\x80\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x24\x00\x00"
         "\x00\xac\x01\x00\x00\x00\x00\x00\x00\x01",
         486);
  *(uint64_t*)0x20000088 = 0x1e6;
  *(uint64_t*)0x20000090 = 0;
  res = -1;
  res = syz_mount_image(0x20000000, 0x200000c0, 0x80000000001f5, 1, 0x20000080,
                        0x1000008, 0x20010200);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000100 = 8;
  *(uint32_t*)0x20000104 = 1;
  syscall(__NR_open_by_handle_at, r[0], 0x20000100ul, 0x490400ul);
  return 0;
}
