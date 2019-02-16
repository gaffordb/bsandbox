#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/user.h>
#include <getopt.h>
#include <string.h>
#include <sys/prctl.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>

#include "sandbox_types.h"

#define NUM_SYSCALLS 320

bool syscall_whitelist_g[NUM_SYSCALLS];
sfp_list_t g_sandbox_perms;

void handle_violation(struct user_regs_struct* regs, pid_t child_pid);
int handle_syscall(struct user_regs_struct* regs, pid_t child_pid,\
                   bool* syscall_whitelist, size_t syscall_whitelist_length);
void print_reg(long long reg_val, pid_t child_pid, char* reg_name);
void reg_to_str(long long reg_val, pid_t child_pid, char str_val[],     \
                size_t str_val_len);
bool is_ascii(char* data);

void allow_file_access_syscalls(void) {
  syscall_whitelist_g[2]  = true; //open
  syscall_whitelist_g[4]  = true; //stat
  syscall_whitelist_g[6]  = true; //lstat
  syscall_whitelist_g[21] = true; //access
  syscall_whitelist_g[76] = true; //truncate
  //syscall_whitelist_g[82] = true; //rename
  syscall_whitelist_g[83] = true; //mkdir
  syscall_whitelist_g[84] = true; //rmdir
  //... there are more, but not these can be toggled as req.
  // Also have to handle them in some way which can be trickyyy
}

void init_allowed_syscalls(void) {
  memset(syscall_whitelist_g, 0, NUM_SYSCALLS*sizeof(bool));
  
  int allowed_syscalls[NUM_SYSCALLS] = {
    0, 1, 3, 5, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
    28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 54, 55,
    60, 61, 63, 66, 67, 69, 70, 71, 72, 73, 74, 75, 77,
    78, 80, 81, 91, 93, 96, 97, 100, 102, 104, 105, 106, 107,
    108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118,
    119, 120, 121, 122, 123, 124, 137, 158, 218, 231, 273, -1
  };
  
  for(int i = 0; allowed_syscalls[i] != -1; i++) {
    syscall_whitelist_g[allowed_syscalls[i]] = true;
  }
}
void print_usage(void) {
  printf("usage: sandbox executable [arg1 arg2 ...]\n");
  printf("[-p syscall_number] : whitelist given syscall number\n");
  printf("[-r file/folder] : designate file readable\n");
  printf("[-w file/folder] : designate file writable\n");
  printf("[-f] : allow program to fork\n");
  printf("[-e] : allow program to exec\n");
  printf("[-h] : print help information\n");
}

char** parse_args(int argc, char** argv) {
  int c;
  int p_syscall;
  opterr = 0;
    
  while((c = getopt(argc, argv, "p:r:w:feh")) != -1) {
    switch(c) {
    case 'r':
      allow_file_access_syscalls();
      sfp_add(&g_sandbox_perms, optarg, 'r');
      break;
    case 'w':
      allow_file_access_syscalls();
      sfp_add(&g_sandbox_perms, optarg, 'w');
      break;
    case 'f':
      syscall_whitelist_g[56] = true; //clone
      syscall_whitelist_g[57] = true; //fork
      syscall_whitelist_g[58] = true; //vfork
      break;
    case 'e':
      syscall_whitelist_g[59] = true; //execve
      break;
    case 'p':
      p_syscall = atoi(optarg);
      assert(p_syscall != 0 && p_syscall < NUM_SYSCALLS);
      syscall_whitelist_g[p_syscall] = true;
      break;
    case 'h':
      print_usage();
      exit(1);
      break;
    case '?':
      fprintf(stderr, "Invalid flag.\n");
      print_usage();
      exit(1);
      break;
    default:
      abort();
    }
  }
  
  char** tracee_args = malloc(sizeof(char*)*(argc-optind+1));
  if(tracee_args == NULL) {
    perror("malloc failed");
    exit(1);
  }
  
  if(optind >= argc) {
    fprintf(stderr, "Please enter a process name to be sandboxed.\n");
    print_usage();
    exit(1);
  }

  /* Get the program name to be executed and args for later exec */
  for(int i = 0; optind < argc; optind++, i++) {
    tracee_args[i] = argv[optind];
  }
  tracee_args[optind] = NULL;
  
  return tracee_args;
}

int main(int argc, char** argv) {
  init_allowed_syscalls();
  char** exec_args = parse_args(argc, argv);
  
  // Call fork to create a child process
  pid_t child_pid = fork();
  if(child_pid == -1) {
    perror("fork failed");
    exit(2);
  }

  // If this is the child, ask to be traced
  if(child_pid == 0) {
    if(ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
      perror("ptrace traceme failed");
      exit(2);
    }

    // Stop the process so the tracer can catch it
    raise(SIGSTOP);
    
    if(execvp(exec_args[0], exec_args)) {
      perror("execve failed");
      exit(2);
    }
  } else {
    // Don't need this anymore!
    free(exec_args);
    
    // Wait for the child to stop
    int status;
    int result;
    bool initial_exec = true;
    
    do {
      result = waitpid(child_pid, &status, 0);
      if(result != child_pid) {
        perror("waitpid failed");
        exit(2);
      }
    } while(!WIFSTOPPED(status));

    // We are now attached to the child process
    printf("Attached!\n");

    ptrace(PTRACE_SETOPTIONS, child_pid, 0,
           (PTRACE_O_TRACECLONE | PTRACE_O_TRACEEXEC | 
            PTRACE_O_TRACEFORK  | PTRACE_O_TRACEVFORK |
            PTRACE_O_EXITKILL/* | PTRACE_O_TRACESYSGOOD*/));

    // Now repeatedly resume and trace the program
    bool running = true;
    int last_signal = 0;
    bool entry = true;
    while(running) {
      // Continue the process, delivering the last signal we received (if any)
      if(ptrace(PTRACE_SYSCALL, child_pid, NULL, last_signal) == -1) {
        perror("ptrace CONT failed");
        exit(2);
      }
      
      // No signal to send yet
      last_signal = 0;

      // Wait for the child to stop again
      if(waitpid(child_pid, &status, 0) != child_pid) {
        perror("waitpid failed");
        exit(2);
      }

      if(WIFEXITED(status)) {
        printf("Child exited with status %d\n", WEXITSTATUS(status));
        running = false;
      } else if(WIFSIGNALED(status)) {
        printf("Child terminated with signal %d\n", WTERMSIG(status));
        running = false;
      } else if(WIFSTOPPED(status)) {
        // Get the signal delivered to the child
        last_signal = WSTOPSIG(status);
        
        // If the signal was a SIGTRAP, we stopped because of a system call
        if(last_signal == SIGTRAP) {

          /* We only care about entries to syscalls (every other call) */
          if(!entry) {
            entry = true;
            last_signal = 0;
            continue;
          } else {
            entry = false;
          }

          // Read register state from the child process
          struct user_regs_struct regs;
          if(ptrace(PTRACE_GETREGS, child_pid, NULL, &regs)) {
            perror("ptrace GETREGS failed");
            exit(2);
          }

          /* We want 1 successful exec call to allow the tracee to start */
          if(!initial_exec) {
            handle_syscall(&regs, child_pid,
                           syscall_whitelist_g, NUM_SYSCALLS);
          } else {
            if(regs.rax == 0 && regs.orig_rax == 59) {
              initial_exec = false;
            }
#ifdef DBG
            handle_syscall(&regs, child_pid, syscall_whitelist_g, NUM_SYSCALLS);
#endif
          }

        last_signal = 0;
        }
      }
    }
  
    return 0;
  }
}


int handle_syscall(struct user_regs_struct* regs, pid_t child_pid,
                   bool* syscall_whitelist, size_t syscall_whitelist_length) {
  if(regs == NULL) { return -1; }
  size_t syscall_num = regs->orig_rax;
  
  if(syscall_num >= syscall_whitelist_length) {
    fprintf(stderr, "Invalid syscall number: %lu.\n", syscall_num);
    exit(1);
  }
  if(!syscall_whitelist[syscall_num]) {
    handle_violation(regs, child_pid);
  } else {
    /* open w/ read, stat, lstat, access */
    if((syscall_num == 2 && (regs->rsi & O_RDONLY || regs->rsi & O_RDWR)) ||
       syscall_num == 4  || syscall_num == 6 || syscall_num == 21) {
      char try_file[100] = {0};
      reg_to_str(regs->rdi, child_pid, try_file, 100);
      printf("syscall: %lu, file: %s\n", syscall_num, try_file);
      if(!strlen(try_file)) { try_file[0] = '.'; }
      /* Handle weird case with libraries that are loaded in that don't actually exist in user filesystem?? Suspected NFS shenanigans. Just let these happen... */
      if(access(try_file, R_OK) != -1) {

        /* Test if readable by user-specified sandbox perms */
        if(!sfp_is_accessible(&g_sandbox_perms, try_file, 'r')) {
          fprintf(stderr,
                  "Sandboxed process tried to read file: %s\n", try_file);
          handle_violation(regs, child_pid);
        }
      }
      /* open w/ write, truncate, mkdir, rmdir */
    }
    if((syscall_num == 2 &&
        (regs->rsi & O_WRONLY || regs->rsi & O_RDWR)) ||
       syscall_num == 76 ||
       syscall_num == 83 || syscall_num == 84) {
      char try_file[100] = {0};
      reg_to_str(regs->rdi, child_pid, try_file, 100);

      if(access(try_file, W_OK) != -1) {
        if(!sfp_is_accessible(&g_sandbox_perms, try_file, 'w')) {
          fprintf(stderr,
                  "Sandboxed process tried to write file: %s\n", try_file);
          handle_violation(regs, child_pid);
        }
      }
    }
  }

  return 0;
}
void print_reg(long long reg_val, pid_t child_pid, char* reg_name) {
  char reg_str[100] = {0};
  reg_to_str(reg_val, child_pid, reg_str, 100);

  if(strlen(reg_str) != 0 && is_ascii(reg_str)) {
    printf("%s: %s\n", reg_name, reg_str);
  } else {
    printf("%s: 0x%llx\n", reg_name, reg_val);
  }
}

bool is_ascii(char* data) {
  for(int i = 0; i < strlen(data); i++) {
    if((unsigned char)data[i] > 128) {
      return false;
    }
  }
  return true;
}

void handle_violation(struct user_regs_struct* regs, pid_t child_pid) {
  printf("Program made system call syscall #%llu\n",
           regs->orig_rax);
    
  /* print out relevant reg data */
  print_reg(regs->rdi, child_pid, "%rdi");
  print_reg(regs->rsi, child_pid, "%rsi");
  print_reg(regs->rdx, child_pid, "%rdx");
  print_reg(regs->rax, child_pid, "%rax");
  
  printf("Sandboxed process killed.\n");
#ifndef DBG
  kill(child_pid, SIGKILL); //TODO kill children's children??
  
  exit(1);
#endif
}

void reg_to_str(long long reg_val, pid_t child_pid, char str_val[], size_t str_val_len) {

  /* Definitely not an address if < 100... */
  if(reg_val < 100) {
    return;
  }

  /* Peek text and copy into str_val until you can peek no more */
  for(int i = 0; i < str_val_len; i+=sizeof(long long)) {
    errno = 0;

    /* Peek 8 bytes and copy it into str_val */
    long long val = ptrace(PTRACE_PEEKTEXT, child_pid, reg_val+i, 0);

    memcpy(str_val+i, &val, sizeof(long long));

#ifdef DBGG
      printf("DEBUG: val: %llx, err=%s\n", val, strerror(errno));
      printf("DEBUG: str_val: %s\n", str_val);
#endif

    /* check if text-so-far contains null terminator in copied area */
    if(strlen(str_val) < i) {
      return;
    }

    /* error reading data at reg_val */
    if(errno && i == 0) {
      return;
    } else if(errno){
      return;
    }
  }
}
