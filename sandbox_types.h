#pragma once
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

typedef struct _sandbox_fperms_t {
  char* fname;
  bool read;
  bool write;
} sandbox_fperms_t;

typedef struct _sfp_node_t {
  sandbox_fperms_t val;
  struct _sfp_node_t* next;
} sfp_node_t;

typedef struct _sfp_list_t {
  sfp_node_t* first;
} sfp_list_t;

bool is_accessible_case(char* authorized_file, char* intended_file);
bool sfp_is_accessible(sfp_list_t* lst, char* fname, char perm);
void sfp_destroy_list(sfp_list_t* lst);
void sfp_add(sfp_list_t* lst, char* fname, char perm);
