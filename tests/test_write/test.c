#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
  char* fname = "expl.txt";
  //  printf("Reading file: %s\n", fname);
  // printf("wronly == wronly: %d\n", (O_WRONLY & 3) == O_WRONLY);
  //printf("rdwr == rdwr: %d\n", (O_RDWR & 3) == O_RDWR);
  //printf("rd == rd: %d\n", (O_RDONLY & 3) == O_RDONLY);
  close(open(fname, O_WRONLY));
}
