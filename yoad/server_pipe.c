#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
  // Create a pipe
  int pipefd[2];
  if (pipe(pipefd) < 0) {
    perror("pipe");
    exit(1);
  }

  // Fork a child process
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    exit(1);
  }

  // Child process
  if (pid == 0) {
    // Close the read end of the pipe
    close(pipefd[0]);

    // Read data from the pipe
    char buf[1024];
    int bytes_read = read(pipefd[1], buf, sizeof(buf));
    if (bytes_read < 0) {
      perror("read");
      exit(1);
    }

    // Print the data
    printf("Received: %s\n", buf);

    // Close the write end of the pipe
    close(pipefd[1]);

    exit(0);
  }

  // Parent process
  // Close the write end of the pipe
  close(pipefd[1]);

  // Write data to the pipe
  char buf[] = "Hello, world!";
  int bytes_written = write(pipefd[0], buf, strlen(buf));
  if (bytes_written < 0) {
    perror("write");
    exit(1);
  }

  // Wait for the child process to exit
  int status;
  wait(&status);

  // Close the read end of the pipe
  close(pipefd[0]);

  return 0;
}
