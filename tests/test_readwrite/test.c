#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
  char* fname = "expl.txt";
  close(open(fname, O_RDWR));
}
