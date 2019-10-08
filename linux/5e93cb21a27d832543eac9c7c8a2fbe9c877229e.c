// KASAN: use-after-free Read in ip_cmsg_recv_offset
// https://syzkaller.appspot.com/bug?id=5e93cb21a27d832543eac9c7c8a2fbe9c877229e
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/futex.h>

unsigned long long procid;

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  if (pthread_create(&th, &attr, fn, arg))
    exit(1);
  pthread_attr_destroy(&attr);
}

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_RELAXED))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
}

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  int i;
  for (i = 0; i < 100; i++) {
    if (waitpid(-1, status, WNOHANG | __WALL) == pid)
      return;
    usleep(1000);
  }
  DIR* dir = opendir("/sys/fs/fuse/connections");
  if (dir) {
    for (;;) {
      struct dirent* ent = readdir(dir);
      if (!ent)
        break;
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        continue;
      char abort[300];
      snprintf(abort, sizeof(abort), "/sys/fs/fuse/connections/%s/abort",
               ent->d_name);
      int fd = open(abort, O_WRONLY);
      if (fd == -1) {
        continue;
      }
      if (write(fd, abort, 1) < 0) {
      }
      close(fd);
    }
    closedir(dir);
  } else {
  }
  while (waitpid(-1, status, __WALL) != pid) {
  }
}

#define SYZ_HAVE_SETUP_TEST 1
static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
}

#define SYZ_HAVE_RESET_TEST 1
static void reset_test()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
    close(fd);
}

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void execute_one(void)
{
  int i, call, thread;
  for (call = 0; call < 7; call++) {
    for (thread = 0; thread < (int)(sizeof(threads) / sizeof(threads[0]));
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      event_timedwait(&th->done, 45);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
      reset_test();
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[1] = {0xffffffffffffffff};

void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 0x40000000002, 3, 2);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint32_t*)0x20000100 = 5;
    syscall(__NR_setsockopt, r[0], 0, 0x14, 0x20000100, 4);
    break;
  case 2:
    memcpy((void*)0x200012c0,
           "\x73\x79\x7a\x5f\x74\x75\x6e\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           16);
    syscall(__NR_setsockopt, r[0], 1, 0x19, 0x200012c0, 0x269);
    break;
  case 3:
    *(uint16_t*)0x20000d00 = 0;
    *(uint8_t*)0x20000d02 = 0;
    *(uint32_t*)0x20000d04 = 0x10000e0;
    syscall(__NR_sendto, r[0], 0x20000280, 0, 0x8000, 0x20000d00, 0x6e);
    break;
  case 4:
    *(uint64_t*)0x20000500 = 0x20000140;
    *(uint64_t*)0x20000508 = 0xaf;
    syscall(__NR_readv, r[0], 0x20000500, 1);
    break;
  case 5:
    memcpy(
        (void*)0x20001300,
        "\xeb\xf8\xf9\xb6\x7e\x55\x61\x88\x8d\xd0\x3d\xc6\x12\x1e\x4e\x12\x1f"
        "\x65\x78\xd4\x36\x0e\x20\xcb\x68\x3c\x9e\x59\x66\xc6\xfd\x11\x60\x88"
        "\xb2\xc1\xdd\x66\xef\xea\x4d\x1b\x98\x06\xf5\x9a\x71\x78\x5f\x2f\x4e"
        "\xfd\xc8\xd9\xc9\x56\x12\x82\x7d\x54\x65\xf7\x0c\xb7\x60\x5e\x7c\x0f"
        "\x02\x4b\xae\x35\x85\x90\xa5\x23\xbc\x6a\x43\x61\x76\x0a\xab\x4a\x9d"
        "\x3f\x4b\x80\xa9\xb9\x8f\x40\x1b\xb1\x3b\xc3\xb1\xde\xb6\xd3\xc0\x1f"
        "\x4a\x64\xc1\x17\x50\xb9\x2e\x70\x5c\x18\x91\x29\xa6\x88\x15\x80\xa9"
        "\xda\x4b\x98\x43\x17\x98\x38\x92\xaf\x80\x3e\x8b\x2c\xbd\x42\xe9\x1c"
        "\x2d\xbc\x85\x77\xe8\x57\xfe\x46\x4d\x86\x4c\x51\xb1\x25\x04\x36\x75"
        "\x5f\x61\x2b\x3a\xb4\x14\x6c\x48\x47\xfb\x73\xe6\x71\x13\x44\x14\xe2"
        "\x2a\x27\x9b\x7e\x09\x6c\x89\x84\xfb\xe7\x2a\x40\x91\xc0\x59\xeb\x03"
        "\x8e\x5b\x5a\xef\x44\x6f\x76\x1d\x82\x07\x32\xc8\xdb\x09\xe7\x9d\xe8"
        "\x94\x3e\x87\xb2\x1b\xaf\x50\xd6\x05\x86\x95\xe6\xbb\x4b\x22\x62\x57"
        "\xcf\x15\x2e\xe8\xdc\xa4\x19\xb6\x4e\x0f\x6f\x45\x11\x42\x59\x4e\x68"
        "\xbb\xaf\xd2\x16\x65\x18\xd0\xaa\xb5\xed\x83\x98\xa7\x78\x69\xb1\x63"
        "\xc4\xbf\x57\xfb\xc3\x76\xff\xee\x6b\xf4\xbd\x26\x68\x39\xa8\xd2\x4b"
        "\x88\xbe\xfc\x59\x5f\xfa\x7b\x96\xe0\x00\xa6\x6c\xf5\x1d\xdf\xce\x95"
        "\xcb\xd0\x65\x6d\xd3\xca\xa6\xe6\x13\x39\xbd\xa9\x0d\xd4\x64\x84\xfa"
        "\xf8\xb9\x30\x1e\x3b\x8b\x0b\xd6\xfd\x1d\x6f\x72\x3c\x33\x3e\xc2\xcc"
        "\xb8\x9d\x92\x26\x58\xa6\xc6\xd2\xf8\x1c\x1e\xcc\x1d\x4a\x8d\xfc\x6f"
        "\x0b\x2e\xbc\xf1\x1a\x83\x51\xf1\x43\x91\x44\x85\xe8\xf6\x0e\xce\x77"
        "\xb5\x3d\x5b\xd9\x7c\xe2\xc1\xbc\xcc\x99\x02\x35\x04\x7d\x2b\x3e\x96"
        "\xe3\xfa\x79\x26\x26\x75\xd4\x88\xec\x66\x66\x3c\x54\x8d\x11\x4b\x55"
        "\x06\xdb\x66\xf4\x5c\x03\x85\xb0\x95\x96\x83\xe4\x80\x19\xd5\x7c\x9c"
        "\x4d\x6a\x18\x07\xd2\x1f\x40\xbf\x01\x15\xaa\x13\x42\x63\xe0\xb5\xf6"
        "\xfe\xe9\x75\xc5\x1c\x7b\x13\x71\x7b\xc1\x80\x55\x44\x7b\x9c\x2e\x7b"
        "\xad\x6f\xb9\xde\xaf\x3f\x54\x95\x15\xf2\xe2\x89\x12\x50\x51\xc7\x84"
        "\xbc\xd9\x76\xea\x60\x43\xe8\xfe\x19\x37\xb6\x1a\x98\x22\x83\xf7\x01"
        "\x51\x32\x67\xaf\x6f\xdc\x6a\x0b\xc4\x98\xf4\xf0\xd8\x1c\xcc\x22\x17"
        "\xec\x4e\xcd\x74\x58\x16\x7f\x5e\x67\xd3\xaf\x8e\xa8\xef\xca\xe0\x42"
        "\x88\xeb\x09\xc4\xaa\xc8\x98\xf5\x4e\xf1\xb7\x41\x11\xb2\xc2\xb8\xc4"
        "\x53\x27\x0a\x04\x72\xf5\x19\x64\x8b\xa6\x74\xc0\x23\xeb\x9d\x27\x58"
        "\xa8\xcd\x4c\x88\xa0\xc5\x7e\x50\x41\x9b\xc5\x65\xf4\xda\x41\x52\xaf"
        "\xd0\x93\xa2\x92\x74\x59\xd4\x76\xd2\x73\x95\xa0\x22\x3c\xfe\x5f\xa2"
        "\xdf\xe4\xad\xda\x39\xe4\x7a\xae\x70\x66\x5e\xd6\xc9\x04\xde\xf2\x6e"
        "\x6f\xed\x7a\xa6\x3e\x48\xba\x5f\x02\x6c\x29\xb7\xb7\x16\x44\x43\x7b"
        "\xc5\x6c\xb6\x49\xc2\x58\x49\x63\x10\x92\xa3\x63\xeb\x60\xcb\x45\xdc"
        "\x97\x80\xa5\x97\x77\xd5\x48\x88\xba\xde\xfa\x8a\xe8\xc6\x5b\x91\x2d"
        "\x80\x31\x16\x11\x2d\x1b\x10\x45\x00\x4b\x95\xb4\x39\x1d\x66\xda\xa2"
        "\x67\x88\x58\xba\x5a\x54\x0f\x3c\x11\xf3\x13\x93\xe7\xcb\xc8\x0a\x01"
        "\x2d\xf2\x8e\xd4\x71\x15\x81\xb7\xc9\xad\x60\x48\x53\x8d\xe3\x89\x6f"
        "\x85\x3f\x04\x27\x00\xe0\x2c\xf1\x28\x4e\x81\xa0\xa0\xa9\x3c\xb4\x7e"
        "\x35\xdc\xdf\x15\xf7\x41\x69\xfa\x1e\x2a\x72\x8b\x63\x5e\xe8\x95\xde"
        "\x1d\xf5\x6b\x25\x6a\x9f\xce\x14\xbc\x5c\x21\xd2\x48\x54\xa7\xfb\xe0"
        "\x58\x50\x79\xb5\x36\xbc\x2e\x1c\x31\xc9\xcf\x6a\x50\x39\x47\x07\xd7"
        "\x2c\x59\x96\x15\x70\xcc\xfc\xf1\x71\xac\xe7\x91\xb9\xd0\x8b\x48\xea"
        "\xae\x11\x70\x05\xab\xd5\x08\x29\xb5\xa7\xdd\xba\xcf\x6d\x0c\xb2\xfd"
        "\x33\xc9\x45\x8c\x0b\xa4\xbd\xf4\x6d\x73\x44\x5d\x55\x62\x73\x50\x42"
        "\x07\xf9\xa4\x51\x0f\x35\xab\x39\x9b\x64\xfe\x1c\x35\x09\x08\x2a\x2f"
        "\x54\xe9\x49\xf1\xa2\xe4\x9e\xe6\xb8\x2c\xc1\xfe\x5c\xd2\x0f\x15\xc4"
        "\xba\xb0\x8e\x62\xc8\xbc\x7d\x21\x27\xd9\x18\xc1\x9b\x89\x5c\xd9\xb5"
        "\xe5\x6e\xdb\xe5\x9a\xfb\x6e\x22\x99\xab\x3e\x75\x40\x75\x12\x9d\xa3"
        "\x76\x40\x03\x97\x4a\x68\xc4\x3f\x81\xef\x60\x38\x2f\xbb\xc6\x38\x19"
        "\xa7\x3e\x67\xb8\xd1\x34\x7e\x55\x5c\x1e\x63\xf4\x83\xa2\xf7\x79\xcd"
        "\xb9\x8c\x07\x13\x2f\x9c\x8e\x4c\x79\x3f\x0e\x61\x6d\xd6\xb0\x8c\x33"
        "\x48\x2d\x5b\x50\xd7\x79\x20\x2c\x87\x9e\xf0\x82\x9e\x15\x88\xba\x25"
        "\x85\x7f\xe4\xd2\x65\xb5\x9a\x69\xad\xd8\x7b\xd7\xfe\x90\xe8\xbc\x05"
        "\x51\x46\xfc\xd0\xc1\x02\x15\x31\x6f\x53\x8d\x18\x83\x9a\x22\x62\xda"
        "\xf4\x09\x64\xcb\x7b\x5d\x7d\xbd\x87\x2f\x76\xc8\x92\x3e\xc4\xdc\xf1"
        "\xe1\xa2\xa5\x01\xbc\xee\x83\x4e\x2e\x91\xd9\xbe\x20\xb2\x6c\xae\xcd"
        "\x00\x99\x0f\x7b\xca\xe1\xcf\x70\x7f\x11\x7e\xc5\x69\x39\xaf\x0d\x23"
        "\x52\x3f\xca\x12\xd7\xf0\x21\x41\x6c\x0e\xda\x63\x3f\x28\x8a\x6a\xf8"
        "\x2a\x03\xa7\x85\xf7\x8d\x5e\x1b\x79\x06\x6e\x62\x77\xa8\xe3\xf6\x13"
        "\x40\x1b\xc8\x5f\xc4\xc1\xbf\x56\x17\xf1\x6b\x45\xae\x88\xc9\x23\xc0"
        "\x1f\x72\x05\x68\xea\x03\x26\x83\x56\x68\x86\x15\x09\x83\xd7\x7d\x82"
        "\x45\xd3\x48\xb1\xc5\x51\x73\x47\xd6\x82\x0d\x04\x86\x47\xdc\xd3\xbf"
        "\x8d\x53\xa5\xdc\x54\x7f\xfe\x8e\xd4\xea\x92\xdc\x14\xc2\x93\x60\x4c"
        "\xab\xfe\xd3\x67\x8c\x0f\xbc\x92\x0d\x36\xd3\x96\x1d\x0e\xc2\x9a\x3e"
        "\x68\xf5\x90\x44\x33\x0f\xce\xb8\x19\xc3\x42\x10\x09\xc2\x5e\x58\xad"
        "\x72\x7c\xb4\x3a\x2b\x66\xd5\xa1\x91\x5b\x7e\x05\xd4\x67\x4f\xd2\xb3"
        "\xb2\x33\x3a\x77\x7a\x8d\x72\x58\x51\x58\x31\x7d\xd1\x5a\x51\x56\xca"
        "\x7b\x02\x8d\x5e\xbd\x01\x31\xd7\x42\xcb\xe2\x6c\x48\x0b\xcc\xd2\x7c"
        "\x7d\xce\xe3\xf4\x64\x09\x19\x85\xca\x79\x1e\x57\x6f\x7f\x35\x80\x34"
        "\xdb\x99\xcf\xb6\xae\xbb\x8d\xfd\xf8\xca\xe5\xba\x98\xb5\xb0\x34\x8f"
        "\xf6\x04\x4f\x64\x17\x11\x12\xb6\xdb\xee\x0a\x18\x2f\x1a\x97\x7f\x37"
        "\x45\xf1\xdf\xb7\xf5\xcf\x81\x6a\x1a\xf6\x9c\xda\xec\x13\x41\x41\x20"
        "\xe3\x81\x50\xd9\x28\x2b\x29\x23\xfe\x63\x9b\xed\x4b\xa8\x3e\xd7\x0d"
        "\x03\x7f\x31\xf3\x60\x59\x90\xc1\xea\xa8\xc2\x59\x98\x1b\x07\x9f\x61"
        "\x39\x93\xfe\xc5\xb0\x6a\x19\xc2\x7a\x62\x9b\x4d\x7f\x6c\x14\x09\x61"
        "\xb2\x0e\x30\x84\xbc\xe5\x94\x07\xab\xbe\xed\x4c\x9d\xfe\x01\x7f\x59"
        "\x7c\x4d\x8e\x36\x32\xb9\x6d\x77\xbd\x9d\x6f\x5d\x42\x62\x1a\xec\xed"
        "\x4f\x42\xc4\xba\x5d\xad\xe3\x2c\x17\x61\xca\xf7\xb3\x97\xb8\xe1\x7f"
        "\x94\x96\x7c\x5a\x84\x32\x3a\xce\xfc\xe0\x9c\x8f\x10\xde\x07\x20\x6b"
        "\xf0\x69\xf3\x41\xdb\xd0\x6d\x11\xf4\x80\x87\x16\x9e\x16\xb2\x14\xa9"
        "\x73\x13\x2e\x2c\xa9\x31\x2c\x03\xc2\x22\xed\x71\x64\xf3\x0c\x4f\xfb"
        "\x92\x11\x62\x6c\x66\xed\x64\x31\x73\xa1\x70\x42\xc0\x85\xf6\xb1\x9b"
        "\x70\x2f\x55\x62\x50\x92\x4f\x0d\xe1\xad\x32\xe7\x62\x40\x36",
        1477);
    *(uint16_t*)0x20000200 = 2;
    *(uint16_t*)0x20000202 = htobe16(0);
    *(uint32_t*)0x20000204 = htobe32(0xe0000002);
    *(uint8_t*)0x20000208 = 0;
    *(uint8_t*)0x20000209 = 0;
    *(uint8_t*)0x2000020a = 0;
    *(uint8_t*)0x2000020b = 0;
    *(uint8_t*)0x2000020c = 0;
    *(uint8_t*)0x2000020d = 0;
    *(uint8_t*)0x2000020e = 0;
    *(uint8_t*)0x2000020f = 0;
    syscall(__NR_sendto, r[0], 0x20001300, 0x5c5, 0xc000, 0x20000200, 0x10);
    break;
  case 6:
    *(uint64_t*)0x20005dc0 = 0x20000280;
    *(uint16_t*)0x20000280 = 0;
    *(uint8_t*)0x20000282 = 0;
    *(uint32_t*)0x20000284 = 0x4e20;
    *(uint32_t*)0x20005dc8 = 0x6e;
    *(uint64_t*)0x20005dd0 = 0x20000980;
    *(uint64_t*)0x20000980 = 0x20000300;
    memcpy((void*)0x20000300, "\x0b\x77\x0c\xe2", 4);
    *(uint64_t*)0x20000988 = 4;
    *(uint64_t*)0x20005dd8 = 1;
    *(uint64_t*)0x20005de0 = 0x20000e80;
    *(uint64_t*)0x20005de8 = 0;
    *(uint32_t*)0x20005df0 = 0;
    syscall(__NR_sendmmsg, r[0], 0x20005dc0, 1, 0);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
