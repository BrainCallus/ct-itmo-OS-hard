#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int bufread(int src, int cnt, char *bf) {
  int temp = 0;
  int sh = 0;
  while ((sizeof(bf) - sh) > cnt) {
    temp = read(src, bf + sh, cnt);
    sh += temp;
    if (temp <= 0) {
      break;
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  int fd1[2];
  int fd2[2];
  int pid;
  int *pidd;
  char buf[1024];
  char *message;
  pidd = &pid;
  message = "ping";
  if (pipe(fd1) < 0) {
    printf("Error creating pipe!");
    exit(1);
  }
  if (pipe(fd2) < 0) {
    printf("Error creating pipe!");
    exit(1);
  }
  if ((pid = fork()) < 0) {
    printf("error fork()\n");
    exit(1);
  } else if (pid == 0) {
    write(fd2[1], message, sizeof(message) + 1);
    bufread(fd2[0], 2, buf);
    close(fd2[1]);
    close(fd2[0]);
    printf("%d: got %s\n", pid, &buf);
    exit(0);
  } else {
    wait(pidd);
    message = "pong";
    write(fd1[1], message, sizeof(message) + 1);
    bufread(fd1[0], 2, buf);
    close(fd1[0]);
    close(fd1[1]);
    printf("%d: got %s\n", pid + 1, &buf);
    exit(0);
  }
  exit(0);
}
