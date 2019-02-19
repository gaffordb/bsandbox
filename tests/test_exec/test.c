#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main(void) {
  execl("../test_read/test", "../test_read/test", NULL , NULL);
}
