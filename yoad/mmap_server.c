// Server
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
  // Open the shared memory file
  int fd = open("shared_memory", O_RDWR | O_CREAT, 0666);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  // Get the size of the shared memory file
  struct stat stat;
  fstat(fd, &stat);
  size_t size = stat.st_size;

  // Map the shared memory file into memory
  void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (ptr == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  // Write data to the shared memory
  // we can share up to 4 GB in 32 byets system so its ok to upload 100 mb 
  char buf[] = "Hello, world!";
  memcpy(ptr, buf, strlen(buf));

  // Close the shared memory file
  close(fd);

  return 0;
}
