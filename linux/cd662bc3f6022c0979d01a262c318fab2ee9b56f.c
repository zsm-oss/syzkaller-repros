// INFO: task hung in __blkdev_get
// https://syzkaller.appspot.com/bug?id=cd662bc3f6022c0979d01a262c318fab2ee9b56f
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
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

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block",
            (uint8_t)a1, (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
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

  unshare(CLONE_NEWNS);
  unshare(CLONE_NEWIPC);
  unshare(CLONE_IO);
}

static int do_sandbox_none(int executor_pid, bool enable_tun)
{
  int pid = fork();
  if (pid)
    return pid;

  sandbox_common();

  loop();
  doexit(1);
}

#ifndef __NR_memfd_create
#define __NR_memfd_create 319
#endif

long r[32];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  memcpy((void*)0x2040aff5,
         "\x2f\x64\x65\x76\x2f\x6c\x6f\x6f\x70\x23\x00", 11);
  r[2] = syz_open_dev(0x2040aff5ul, 0x0ul, 0x1ul);
  memcpy((void*)0x2058dff8, "\x5c\x73\x79\x73\x74\x65\x6d\x00", 8);
  r[4] = syscall(__NR_memfd_create, 0x2058dff8ul, 0x2ul);
  *(uint64_t*)0x200a3ff0 = (uint64_t)0x20c13e00;
  *(uint64_t*)0x200a3ff8 = (uint64_t)0x1bb;
  memcpy(
      (void*)0x20c13e00,
      "\xb5\x10\x7b\x98\x1c\x0d\xed\x89\x94\x42\xee\xc2\xca\xb7\xe2\xc9"
      "\xe6\xcd\x9a\xee\xb4\x18\xde\xd0\x08\x97\x23\x03\x33\x9e\xa7\x48"
      "\x1a\xa2\xb3\xd0\x78\x6a\x90\x17\x40\xa8\x6a\xbe\x45\x07\x45\x32"
      "\x17\xc9\xf6\xa4\xaf\xf9\xf4\x2d\x64\xe9\xdf\xe9\x5e\xdc\x99\xab"
      "\x6e\x28\xe2\xcf\xb8\x89\xb9\x00\xa9\xd9\x99\x14\x0b\x47\x17\xbd"
      "\x64\x80\xea\x00\x0f\x7e\x16\x7e\xe7\xbe\xc2\xcf\xe3\x16\x73\xa8"
      "\x8b\xa1\x60\xa9\xbd\x59\x3e\xb0\xd9\x0f\x1e\x0c\x2e\xee\x40\xa1"
      "\x42\xa5\x0a\x08\x13\x7a\xe8\x65\x8d\xe4\xd1\xa0\x8a\xd6\xd2\x38"
      "\x1c\xfd\x97\x30\x02\x09\x06\x5a\x18\x01\x69\xb7\x94\x7c\x32\xce"
      "\x3f\x3f\xf4\x22\xa7\x65\x28\x04\x50\x5a\x06\xa0\x56\x73\x8b\x62"
      "\x74\x58\x82\x32\xde\x69\xf6\x78\x25\xae\x03\x13\x9e\x7d\x60\xca"
      "\x79\xc5\x94\x08\xe6\x49\xfa\xc9\x3d\x6b\xab\x13\x11\x2e\x70\xb8"
      "\xa5\x2e\x68\xad\x63\xba\xc4\xf0\xaa\xba\x1b\x71\x33\x77\x9f\xda"
      "\xb0\x6f\x09\xe0\x22\xf9\xd6\x05\xe8\x00\x51\xce\x42\x9d\x28\x12"
      "\xf6\x32\xf6\x58\x58\x28\x93\x3f\x8b\xc1\x5d\x79\x27\xe4\x1b\x1e"
      "\xbb\x16\xe7\x15\x93\xd9\x2e\xce\x21\xd5\xf0\x42\x54\xc2\xc7\x41"
      "\x67\xab\xcb\x2f\x27\x76\x13\xbc\x7f\x7c\xdb\x6a\x36\x98\xe2\xa8"
      "\x79\xfa\xbb\x48\x10\x15\x92\x8f\x06\xd3\x92\x5c\xa8\x03\xf5\xcb"
      "\x27\x5c\x38\x1c\x3e\x91\xf4\x52\x70\x8e\xc8\x82\x7d\x8c\x76\x2c"
      "\xb8\x86\xe8\x68\x69\x0d\x1a\x62\x59\x78\xd7\xb9\x78\x6f\x21\xa7"
      "\xd6\x0f\x27\x3a\x5f\xb5\xc8\xa0\xdd\xdc\xaa\x65\x38\x72\x64\xa1"
      "\x04\xca\xf8\x2a\x45\x1f\x4e\x83\x92\x8a\xa5\xd5\xfa\xa7\x9f\x00"
      "\x95\xbf\x9c\xae\x38\x31\x6a\xb8\xa2\x73\xdf\x80\xdd\x74\xc0\xc1"
      "\xa0\xf8\xda\xa2\x00\x6d\x01\xad\xcb\x97\x22\x9d\xeb\x1b\xdf\xdb"
      "\x57\xd1\x7a\x98\x41\x5d\x1a\xba\x54\x26\x97\xf1\xa9\x2b\x45\xca"
      "\x50\x2d\xc8\xb5\x1b\xf5\x4b\xa1\x7d\xcc\xa0\xe1\xfe\x5c\x52\x52"
      "\x24\x3f\x0c\xf9\x7a\xd6\x10\x44\xc5\xef\xb4\x3a\x26\xae\x00\x4b"
      "\x06\xbf\xf6\x9d\xf7\xed\xf2\x48\xc0\x3c\x11",
      443);
  r[8] = syscall(__NR_pwritev, r[4], 0x200a3ff0ul, 0x1ul, 0x49ul);
  r[9] = syscall(__NR_ioctl, r[2], 0x4c00ul, r[4]);
  *(uint64_t*)0x200ddff8 = (uint64_t)0x0;
  r[11] =
      syscall(__NR_sendfile, r[2], r[4], 0x200ddff8ul, 0x100000001ul);
  r[12] = syscall(__NR_gettid);
  *(uint64_t*)0x206fdf18 = (uint64_t)0x0;
  *(uint64_t*)0x206fdf20 = (uint64_t)0x0;
  *(uint64_t*)0x206fdf28 = (uint64_t)0x0;
  *(uint64_t*)0x206fdf30 = (uint64_t)0xfffffffffffff000;
  *(uint64_t*)0x206fdf38 = (uint64_t)0x3ea;
  *(uint32_t*)0x206fdf40 = (uint32_t)0x0;
  *(uint32_t*)0x206fdf44 = (uint32_t)0xfffffffffffffffe;
  *(uint32_t*)0x206fdf48 = (uint32_t)0x1e;
  *(uint32_t*)0x206fdf4c = (uint32_t)0xd;
  memcpy((void*)0x206fdf50,
         "\x32\xb0\x5d\x80\x1e\x66\x49\xb8\x12\xd2\xe6\xb6\xcc\x15\x94"
         "\x42\xb1\x21\x53\x13\x2c\x9b\xf9\x44\xcf\x27\x00\x12\xda\x21"
         "\xba\x80\xc0\x40\x39\x77\x14\xf7\x7f\xaa\x1e\x8e\xd3\xfc\x72"
         "\x09\xd6\x4d\x14\xce\xc7\x47\x9b\x05\xca\xe0\xc7\x32\xde\x3a"
         "\xb6\xcc\x5f\x4b",
         64);
  memcpy((void*)0x206fdf90,
         "\x94\x14\x06\x56\x9a\x73\xd0\x65\x56\xae\xe2\x94\x3f\x5e\x94"
         "\xc8\x44\xa5\x55\x27\xaf\x84\xfb\x28\xf0\x11\x82\x5a\xf6\x09"
         "\x10\xe7\x70\x6e\xb4\x78\x6b\x3a\x97\x6d\xa7\xb5\x72\x11\xaf"
         "\x83\x4e\x7d\x98\xb8\x39\x62\x96\x89\xa1\x47\x2f\x55\x78\xb2"
         "\x99\x8b\xd7\xda",
         64);
  memcpy((void*)0x206fdfd0, "\xef\xd5\x59\x22\xa7\xdc\x32\x93\x94\x61"
                            "\x0a\xfb\xac\xd2\x37\xd2\x34\xaf\xb8\x05"
                            "\xeb\xc3\xb9\x4e\x4f\xb9\xd5\x6d\xd0\x3f"
                            "\x1f\x8b",
         32);
  *(uint64_t*)0x206fdff0 = (uint64_t)0x905;
  *(uint64_t*)0x206fdff8 = (uint64_t)0x8;
  r[27] = syscall(__NR_ioctl, r[2], 0x4c04ul, 0x206fdf18ul);
  *(uint64_t*)0x2033dfe8 = (uint64_t)0xaa;
  *(uint64_t*)0x2033dff0 = (uint64_t)0x1a;
  *(uint64_t*)0x2033dff8 = (uint64_t)0x0;
  r[31] = syscall(__NR_ioctl, 0xfffffffffffffffful, 0xc018aa3ful,
                  0x2033dfe8ul);
}

int main()
{
  int pid = do_sandbox_none(0, false);
  int status = 0;
  while (waitpid(pid, &status, __WALL) != pid) {
  }
  return 0;
}
