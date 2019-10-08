// general protection fault in vsscanf
// https://syzkaller.appspot.com/bug?id=ce80a10f7234f816218d7c257396c00d575b52d8
// status:fixed
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
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

unsigned long long procid;

struct fs_image_segment {
  void* data;
  uintptr_t size;
  uintptr_t offset;
};

#define IMAGE_MAX_SEGMENTS 4096
#define IMAGE_MAX_SIZE (129 << 20)

#define SYZ_memfd_create 319

static long syz_mount_image(long fsarg, long dir, unsigned long size,
                            unsigned long nsegs, long segments, long flags,
                            long optsarg)
{
  char loopname[64], fs[32], opts[256];
  int loopfd, err = 0, res = -1;
  unsigned long i;
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
  memset(fs, 0, sizeof(fs));
  strncpy(fs, (char*)fsarg, sizeof(fs) - 1);
  memset(opts, 0, sizeof(opts));
  strncpy(opts, (char*)optsarg, sizeof(opts) - 32);
  if (strcmp(fs, "iso9660") == 0) {
    flags |= MS_RDONLY;
  } else if (strncmp(fs, "ext", 3) == 0) {
    if (strstr(opts, "errors=panic") || strstr(opts, "errors=remount-ro") == 0)
      strcat(opts, ",errors=continue");
  } else if (strcmp(fs, "xfs") == 0) {
    strcat(opts, ",nouuid");
  }
  if (mount(loopname, (char*)dir, fs, flags, opts)) {
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

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);

  memcpy((void*)0x20000000, "vfat", 5);
  memcpy((void*)0x200004c0, "./file0", 8);
  syz_mount_image(0x20000000, 0x200004c0, 0, 0, 0x20000300, 0, 0x20000240);
  memcpy((void*)0x20000080, "127.0.0.1", 10);
  memcpy((void*)0x20000100, "./file1", 8);
  memcpy((void*)0x20000140, "9p", 3);
  memcpy((void*)0x200001c0, "trans=tcp,", 10);
  memcpy((void*)0x200001ca, "port", 4);
  *(uint8_t*)0x200001ce = 0x3d;
  sprintf((char*)0x200001cf, "0x%016llx", (long long)0);
  *(uint8_t*)0x200001e1 = 0x2c;
  *(uint8_t*)0x200001e2 = 0;
  syscall(__NR_mount, 0x20000080, 0x20000100, 0x20000140, 0, 0x200001c0);
  memcpy((void*)0x200000c0, "./file0", 8);
  memcpy((void*)0x20000040, "9p", 3);
  syscall(__NR_mount, 0, 0x200000c0, 0x20000040, 0, 0x200001c0);
  return 0;
}
