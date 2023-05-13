#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

int main() {
  
  // this is in uds stream! - if we want to change it to uds dgram:
  // int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
  int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("socket");
    exit(1);
  }

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, "/tmp/uds_socket");

  if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
    exit(1);
  }

  listen(sockfd, 5);

  int clientfd = accept(sockfd, NULL, NULL);
  if (clientfd < 0) {
    perror("accept");
    exit(1);
  }

  char buf[1024];
  int bytes_read = read(clientfd, buf, sizeof(buf));
  if (bytes_read < 0) {
    perror("read");
    exit(1);
  }

  printf("Received: %s\n", buf);

  close(clientfd);

  close(sockfd);

  return 0;
}
