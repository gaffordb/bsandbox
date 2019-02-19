#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
  close(open("expl.txt", O_RDONLY));
}
