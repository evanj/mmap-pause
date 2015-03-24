/*
Times writes to a memory mapped file, printing times that are extremely slow.

gcc --std=gnu99 -O3 -Wall -Wextra -o mmapwritepause mmapwritepause.c


Tested on various filesystems:

EXT2/3/4: Occurs, even if the writeback load is on another device
XFS: Occurs but much more rarely?
BRTFS: Terrible, with load on other devices: frequent pauses!
*/

#include <assert.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>

#define PAGE_SIZE 4096

void zerofill(int fd, int size) {
  char buffer[PAGE_SIZE];
  memset(buffer, 0, sizeof(buffer));

  int offset = 0;
  while (offset < size) {
    int toWrite = sizeof(buffer);
    if (offset+toWrite > size) {
      toWrite = size - offset;
    }
    int written = write(fd, buffer, toWrite);
    assert(written == toWrite);
    offset += toWrite;
  }
}

int isotime(char* buffer, int length, const struct timeval* t) {
  struct tm timestamp;
  struct tm* r = gmtime_r(&t->tv_sec, &timestamp);
  assert(r == &timestamp);
  int len = snprintf(buffer, length, "%04d-%02d-%02dT%02d:%02d:%02d.%06dZ",
    timestamp.tm_year+1900, timestamp.tm_mon+1, timestamp.tm_mday, timestamp.tm_hour,
    timestamp.tm_min, timestamp.tm_sec, (int32_t) t->tv_usec);
  return len;
}

static const char FLAG_MLOCK[] = "-mlock";

int main(int argc, const char* const* argv) {
  static const int SLEEP_US = 50 * 1000;
  static const int SUSPICIOUS_US = 10 * 1000;
  static const int FILE_SIZE = 32 << 10;
  static const int NUM_UINT64 = FILE_SIZE / 8;

  int useMlock = 0;
  int nextArg = 1;
  if (nextArg < argc && strncmp(FLAG_MLOCK, argv[nextArg], sizeof(FLAG_MLOCK)) == 0) {
    useMlock = 1;
    nextArg += 1;
  }
  if (argc - nextArg != 1) {
    fprintf(stderr, "Usage: mmapwritepause [-mlock] (file to write)\n");
    return 1;
  }
  const char* path = argv[nextArg];
  int fd = open(path, O_RDWR|O_CREAT|O_TRUNC, 0600);
  assert(fd >= 0);
  zerofill(fd, FILE_SIZE);

  uint64_t* data = (uint64_t*) mmap(0, FILE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  assert(data != MAP_FAILED);
  if (useMlock) {
    // NOTE: This doesn't prevent this problem from occurring
    printf("mlocking data ...\n");
    int err = mlock(data, FILE_SIZE);
    assert(err == 0);
  }

  int64_t sum = 0;
  for (int i = 0; i < NUM_UINT64; i += PAGE_SIZE / 8) {
    sum += data[i];
  }
  printf("read all pages (sum %" PRId64 ")\n", sum);
 
  printf("writing sequentially ...\n");
  int index = 0;
  struct timeval start;
  struct timeval end;
  char line[1024];
  while (1) {
    int err = gettimeofday(&start, NULL);
    assert(err == 0);
    data[index] = 0x0102030405060708;
    err = gettimeofday(&end, NULL);
    assert(err == 0);

    int64_t delta = (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec);
    if (delta > SUSPICIOUS_US) {
      int len = isotime(line, sizeof(line), &start);
      assert(0 < len && len < (ssize_t) sizeof(line));
      int len2 = snprintf(line+len, sizeof(line)-len,
        ": byte %zu (index %d) delay %" PRId64 " us\n",
        index*sizeof(*data), index, delta);
      int total = len+len2;
      assert(0 < len2 && total < (ssize_t) sizeof(line));
      assert(line[total] == '\0');
      len = write(0, line, total);
      assert(len == total);
    }

    index = (index + 1) % NUM_UINT64;
    err = usleep(SLEEP_US);
    assert(err == 0);
  }

  int err = munmap(data, FILE_SIZE);
  assert(err == 0);
  err = close(fd);
  assert(err == 0);
  return 0;
}
