#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
int main(int argc, char** argv) {
  char* fname = "expl.txt";
  pid_t pid;
  if((pid = fork()) < 0) {
    perror("fork");
    exit(1);
  } else if(pid == 0) {
    if(argc == 2) {
      close(open(fname, O_RDONLY));
      printf("Child read!\n");
    }
  }
  else {
    if(argc != 2) {
      close(open(fname, O_RDONLY));
      printf("Parent read!\n");
    }
    waitpid(pid,NULL, WNOHANG);
  }
}
