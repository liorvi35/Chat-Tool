#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

int main() {

  // for uds fgram:
  //  int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
  int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("socket");
    exit(1);
  }

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, "/tmp/uds_socket");

  // Connect to the server
  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("connect");
    exit(1);
  }

  char buf[] = "Hello, world!";
  int bytes_written = write(sockfd, buf, strlen(buf));
  if (bytes_written < 0) {
    perror("write");
    exit(1);
  }

  close(sockfd);

  return 0;
}
