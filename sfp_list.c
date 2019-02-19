#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <unistd.h>
#include "sandbox_types.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

bool is_accessible_case(char* authorized_file, char* intended_file) {
  /* Get canonical representations */
  char* abs_authorized_file = realpath(authorized_file, NULL);
  char* abs_intended_file = realpath(intended_file, NULL);

  if(!abs_authorized_file || !abs_intended_file) {
    perror("realpath failed");
    exit(1);
  }

  /* 
     If they are pointing to the same file, or if the intended 
     file is directly contained within the authorized directory, it's okay
  */
  if(strcmp(abs_authorized_file, abs_intended_file) == 0 ||
     (strstr(abs_intended_file, abs_authorized_file) == abs_intended_file &&
      strchr(abs_intended_file+strlen(abs_authorized_file)+1, '/') == NULL)) {
       return true;
  }
  return false;
}

bool sfp_is_accessible(sfp_list_t* lst, char* fname, char perm) {
  if(lst->first == NULL) { return false; }
  sfp_node_t* cur = lst->first;
  for(; cur != NULL; cur = cur->next) {
    sandbox_fperms_t val = cur->val;
    if((perm == 'r' && val.read) ||
       (perm == 'w' && val.write)) {
      if(is_accessible_case(val.fname, fname)) {
        return true;
      }
    }
  }
  return false;
}

void sfp_destroy_list(sfp_list_t* lst) {
  if(lst->first == NULL) { return; }
  sfp_node_t* cur = lst->first;
  while(cur != NULL) {
    sfp_node_t* tmp = cur;
    cur = cur->next;
    free(tmp);
  }
}

void sfp_add(sfp_list_t* lst, char* fname, char perm) {

  bool _read = perm == 'r';
  bool _write = perm == 'w';
  
  /* Make sure values make sense */
  assert(_read || _write);
  if(_read) {
    if(access(fname, R_OK) == -1) {
      fprintf(stderr, "Note: Attempted to add %s with perms %c. This failed because either the file does not exist or the specified permission does not make sense with the calling process's permissions.\n", fname, perm);
      return;
    }
  }
  if(_write) {
    if(access(fname, W_OK) == -1) {
      fprintf(stderr, "Note: Attempted to add %s with perms %c. This failed because either the file does not exist or the specified permission does not make sense with the calling process's permissions.\n", fname, perm);
    }
  }
  
  sfp_node_t* to_add = malloc(sizeof(sfp_node_t));
  if(!to_add) {
    perror("malloc failed");
    exit(1);
  }

  to_add->val = (sandbox_fperms_t){ fname, _read, _write };
  if(lst->first == NULL) {
    lst->first = to_add;
  } else {
    sfp_node_t* cur = lst->first;
    while(cur->next != NULL) {
      cur = cur->next;
    }
    cur->next = to_add;
  }
}
