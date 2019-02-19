# bsandbox
This is a simple sandboxing application built for GNU/Linux systems. Please don't actually use this on malicious programs expecting it to be sufficiently robust.  

Allows user to run a program in an environment that will limit the freedom of a potentially malicious program. By default, blocks all access to system resources. Selected directories/files can be specified as readable/writable via command line options. Additionally, more syscalls can be whitelisted via command line options as needed. 

If a sandboxed application makes a request that would violate the specified rules, the program will be terminated and information regarding the request (syscall number, register state) will be printed to the console. If registers hold pointers to ASCII string values, these values will be printed as strings. 

A set number of syscalls are allowed by default for sandboxed programs. The default whitelisted syscalls are intended to allow for syscalls that do not directly use/access system resources that could be favorable for a malicious program.  

Due to the conservative nature of these whitelisted syscalls, users are provided with an additional mechanism to allow selected syscalls if they notice that the sandboxed process requires a syscall that is not on the whitelist, but the user deems it acceptable. The intended use of such functionality is to start running the program with the default whitelist, and then only add additional syscalls as desired.  

Generally, one should be wary of whitelisting the following:
* Any of the `*at` syscalls (e.g., 257-269) -- allow for path traversal relative to a given file descriptor. This will allow for processes to read/write to anywhere in the filesystem if there exists a known readable-file (e.g., libc). 
* Any system-wide resources/IPC mechanisms: shared memory, semaphores, pipes, FIFOs, message passing, etc
* Anything that will provide user with a read or write -- when whitelisted, they can then bypass the fine-grained r/w permissions provided by sandbox (b/c these processes will not be accounted for)


Important things to note:  
* Allowing programs to fork will allow the child of this sandboxed process to do whatever it wants. i.e., forking escapes sandbox.
  * This has not been implemented due to current time constraints -- will require some program restructuring and additional bookkeeping.
* Programs do not escape sandbox via exec'ing another program.
* Programs are allowed to exec other programs provided on the user's system if exec permission is allowed.
* Libraries are allowed by default. These are tailored to Grinnell College's MATHLAN system. 

# Build
When at root directory:  `make`  

# Usage
```
usage: sandbox executable [arg1 arg2 ...]  
  [-p syscall_number] : whitelist given syscall number  
  [-r file/folder] : designate file readable  
  [-w file/folder] : designate file writable
  [-f] : allow program to fork
  [-e] : allow program to exec
  [-h] : print help information
```
Note: Reading in CLI options is lazily done. To specify multiple readable/writable/whitelisted arguments, you need to provide an additional [-[p/r/w] arg] for each.  
e.g., `sandbox ls -r . -r ../ -r /path/to/file`  
Further usage can be found in test cases (specifically run_tests.sh files).  
 
# Testing
* Each sub-directory within the `tests` directory will contain a Makefile. Within each directory, run `make test` and verify output is as expected.  
* Tests include: 
  * Accessibility testing
    * Ensures that the internal fine-grained r/w permission library is functioning as intended.
  * Custom whitelist testing
    * Ensures that additional whitelisting of additional syscalls works as intended.
  * Read permission testing
    * Ensures that sandboxed programs may only read files which have been explicitly allowed by the sandbox.
  * Write permission testing
    * Ensures that sandboxed programs may only write files which have been explicitly allowed by the sandbox. 
  * Read/Write permission testing
   * Ensures that sandboxed programs may only read&write files which have been explicitly allowed by the sandbox.
  * Exec testing
    * Only programs with the `-e` flag may exec. After executing another binary, the program still obeys the laws of the sandbox. 
  * Fork testing
    * Only programs with the `-f` flag may fork. After forking, the parent remains sandboxed but the child is free to do whatever it wants. 
  * Testing a couple Unix binaries for some very light 'real-world' testing
    * Shows how whitelisting syscalls may be necessary for some binaries
