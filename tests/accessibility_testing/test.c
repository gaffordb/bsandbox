#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "../../sandbox_types.h"

int main(void) {
  printf("Testing...\n");

  /* Setting permissions */
  sfp_list_t fperms;
  sfp_add(&fperms, ".", 'r');
  sfp_add(&fperms, ".", 'w');
  sfp_add(&fperms, "../", 'r');

  /* Testing permissions (abs paths not tested here bc annoying to get cwd)*/
  assert(sfp_is_accessible(&fperms, ".", 'r'));
  assert(sfp_is_accessible(&fperms, "./Makefile", 'w'));
  assert(sfp_is_accessible(&fperms, ".", 'w'));
  assert(!sfp_is_accessible(&fperms, "../", 'w'));
  assert(!sfp_is_accessible(&fperms, "/usr/local/bin", 'r'));
  
  printf("Testing successful.\n");
}
