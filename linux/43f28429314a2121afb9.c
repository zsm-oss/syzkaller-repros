// BUG: unable to handle kernel paging request in dqput
// https://syzkaller.appspot.com/bug?id=43f28429314a2121afb9
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

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);

  memcpy((void*)0x20000000, "ext4\000", 5);
  memcpy((void*)0x20000100, "./file0\000", 8);
  *(uint64_t*)0x20000200 = 0x20010000;
  memcpy((void*)0x20010000,
         "\x20\x00\x00\x00\x00\x02\x00\x00\x19\x00\x00\x00\x90\x01\x00\x00\x0f"
         "\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x06\x00\x00\x00\x00\x00"
         "\x08\x00\x00\x80\x00\x00\x20\x00\x00\x00\xd5\xf4\x65\x5f\xd5\xf4\x65"
         "\x5f\x01\x00\xff\xff\x53\xef\x01\x00\x01\x00\x00\x00\xd4\xf4\x65\x5f"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x0b"
         "\x00\x00\x00\x00\x04\x00\x00\x08\x00\x00\x00\xd2\xc2\x00\x00\x12\x03",
         102);
  *(uint64_t*)0x20000208 = 0x66;
  *(uint64_t*)0x20000210 = 0x400;
  *(uint64_t*)0x20000218 = 0x20010100;
  memcpy((void*)0x20010100, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x91\xb7\x3e\xf4\xb8\xd9\x44\xc4\xbe\x6a\xea\xa0"
                            "\xd6\xc4\x7e\x6c\x01\x00\x40",
         31);
  *(uint64_t*)0x20000220 = 0x1f;
  *(uint64_t*)0x20000228 = 0x4e0;
  *(uint64_t*)0x20000230 = 0x20010300;
  memcpy((void*)0x20010300, "\x03\x00\x00\x00\x04", 5);
  *(uint64_t*)0x20000238 = 5;
  *(uint64_t*)0x20000240 = 0x640;
  *(uint64_t*)0x20000248 = 0x20010400;
  memcpy((void*)0x20010400, "\x02\x00\x00\x00\x03\x00\x00\x00\x04\x00\x00\x00"
                            "\x19\x00\x0f\x00\x03\x00\x04\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x0f\x00\x2e\x69",
         32);
  *(uint64_t*)0x20000250 = 0x20;
  *(uint64_t*)0x20000258 = 0x1000;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint64_t*)0x20000270 = 0x2000;
  *(uint64_t*)0x20000278 = 0x20012600;
  memcpy((void*)0x20012600, "\xed\x41\x00\x00\x00\x10\x00\x00\xd4\xf4\x65\x5f"
                            "\xd5\xf4\x65\x5f\xd5\xf4\x65\x5f\x00\x00\x00\x00"
                            "\x00\x00\x04\x00\x80",
         29);
  *(uint64_t*)0x20000280 = 0x1d;
  *(uint64_t*)0x20000288 = 0x4400;
  *(uint64_t*)0x20000290 = 0x20000080;
  memcpy((void*)0x20000080,
         "\x80\x81\x00\x00\x00\x18\x00\x00\xd4\xf4\x65\x5f\xd4\xf4\x65\x5f\xd4"
         "\xf4\x65\x5f\x00\x00\x00\x00\x00\x00\x01\x00\x80\x00\x00\x00\x10\x00"
         "\x08\x00\x00\x00\x00\x00\x0a\xf3\x01\x00\x04\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x02\x00\x00\x00\x30",
         61);
  *(uint64_t*)0x20000298 = 0x3d;
  *(uint64_t*)0x200002a0 = 0x4800;
  *(uint64_t*)0x200002a8 = 0x20012a00;
  memcpy((void*)0x20012a00,
         "\x80\x81\x00\x00\x00\x18\x00\x00\xd4\xf4\x65\x5f\xd4\xf4\x65\x5f\xd4"
         "\xf4\x65\x5f\x00\x00\x00\x00\x00\x00\x01\x00\x80\x00\x00\x00\x10\x00"
         "\x08\x00\x00\x00\x00\x00\x0a\xf3\x01\x00\x04\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x02\x00\x00\x00\x40",
         61);
  *(uint64_t*)0x200002b0 = 0x3d;
  *(uint64_t*)0x200002b8 = 0x4c00;
  *(uint64_t*)0x200002c0 = 0x20013900;
  memcpy((void*)0x20013900, "\x11\x1f\xc0\xd9\x01", 5);
  *(uint64_t*)0x200002c8 = 5;
  *(uint64_t*)0x200002d0 = 0x30000;
  *(uint64_t*)0x200002d8 = 0x20013a00;
  memcpy((void*)0x20013a00, "\x02", 1);
  *(uint64_t*)0x200002e0 = 1;
  *(uint64_t*)0x200002e8 = 0x30400;
  *(uint64_t*)0x200002f0 = 0x20013b00;
  memcpy((void*)0x20013b00, "\x03", 1);
  *(uint64_t*)0x200002f8 = 1;
  *(uint64_t*)0x20000300 = 0x30800;
  *(uint64_t*)0x20000308 = 0x20013c00;
  memcpy((void*)0x20013c00, "\x04", 1);
  *(uint64_t*)0x20000310 = 1;
  *(uint64_t*)0x20000318 = 0x30c00;
  *(uint64_t*)0x20000320 = 0x20013d00;
  memcpy((void*)0x20013d00, "\x05", 1);
  *(uint64_t*)0x20000328 = 1;
  *(uint64_t*)0x20000330 = 0x31000;
  *(uint64_t*)0x20000338 = 0x20013e00;
  memcpy((void*)0x20013e00, "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x08",
         41);
  *(uint64_t*)0x20000340 = 0x29;
  *(uint64_t*)0x20000348 = 0x31400;
  *(uint64_t*)0x20000350 = 0x20013f00;
  memcpy((void*)0x20013f00, "\x27\x19\xc0\xd9\x01\x00\x00\x00\x80\x3a\x09\x00"
                            "\x80\x3a\x09\x00\x00\x00\x00\x00\x06\x00\x00\xb7"
                            "\x17\xaf\x18\xd8\xb6",
         29);
  *(uint64_t*)0x20000358 = 0x1d;
  *(uint64_t*)0x20000360 = 0x40000;
  *(uint64_t*)0x20000368 = 0;
  *(uint64_t*)0x20000370 = 0;
  *(uint64_t*)0x20000378 = 0;
  *(uint64_t*)0x20000380 = 0;
  *(uint64_t*)0x20000388 = 0;
  *(uint64_t*)0x20000390 = 0x41400;
  syz_mount_image(0x20000000, 0x20000100, 0x200000, 0x11, 0x20000200, 0,
                  0x20000700);
  memcpy((void*)0x20000000, "./file0\000", 8);
  syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000000ul, 0x410481ul, 0ul);
  return 0;
}
