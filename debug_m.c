
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <unistd.h>
#include <errno.h>
#include <execinfo.h>



void print(const char* format, ...)
{
    va_list ap;
    fprintf(stdout, "[%d] ", getpid());
    va_start(ap, format);
    vfprintf(stdout, format, ap);
    va_end(ap);
}

/* For some reason this declaration isn't available from string.h */
int waitchild(pid_t pid,int status) {

      waitpid(pid, &status, 0);
      if(WIFSTOPPED(status)) {  //test status of child if he is stopped or not
          return 0;
      }
      else if (WIFEXITED(status)) {
          return 1;
      }
      else {
          printf("%d raised an unexpected status %d", pid, status);
          return 2;
      }
  }

//Run a target process in tracing mode by execve()-ing the given program name.



void run_target(const char* program)
{
    print("target started. will run '%s'\n", program);

    /* Allow tracing of this process */
    if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
        perror("ptrace");
        return;
    }

    /* Replace this process's image with the given program */
  //  execl(program, program, NULL);}
}


void breakpoint(pid_t child, uint64_t addr)
{
    int status;
    struct user_regs_struct regs;
    print("***************debugger started*****************\n");

    /* Wait for child to stop on its first instruction */
    wait(&status);

    /* Obtain and show child's instruction pointer */
    ptrace(PTRACE_GETREGS, child, 0, &regs);
    print("Child started at adress RIP = %p\n", regs.rip);

    /* Look at the word at the address we're interested in */

    long data = ptrace(PTRACE_PEEKTEXT, child, (void*)addr, 0);
    print("Original data at %p: %p\n", addr, data);

    /* Write the trap instruction 'int 3' into the address */
    long data_with_trap = (data & ~0xFF) | 0xCC;
    ptrace(PTRACE_POKETEXT, child, (void*)addr, (void*)data_with_trap);

    /* See what's there again... */
    long readback_data = ptrace(PTRACE_PEEKTEXT, child, (void*)addr, 0);
    print("After trap, data at %p: %p\n", addr, readback_data);

    /* Let the child run to the breakpoint and wait for it to
    ** reach it
    */
    ptrace(PTRACE_CONT, child, 0, 0);


    waitchild(child,status);


    /* See where the child is now */
    ptrace(PTRACE_GETREGS, child, 0, &regs);
    print("Child stopped at RIP = %p\n", regs.rip);

    /* Remove the breakpoint by restoring the previous data
    ** at the target address, and unwind the EIP back by 1 to
    ** let the CPU execute the original instruction that was
    ** there.
    */
    ptrace(PTRACE_POKETEXT, child, (void*)addr, (void*)data);
    regs.rip -= 1;
    ptrace(PTRACE_SETREGS, child, 0, &regs);

    /* The child can continue running now */
    ptrace(PTRACE_CONT, child, 0, 0);

    waitchild(child,status);
}


void do_backtrace(pid_t pid,siginfo_t targetsig)
{
    void    *array[100];
    size_t   size, i;
    char   **strings;
    // if we don't recived a signal we don't do backtrace
    if(ptrace(PTRACE_GETSIGINFO, pid, NULL, &targetsig) == -1){
       print("no stack\n");
   }else{
    printf("*********************** Backtrace ***********************\n" );
    size = backtrace(array, 100);
    strings = backtrace_symbols(array, size);

    for (i = 0; i < size; i++) {
        printf("%p : %s\n", array[i], strings[i]);//show function call on stack
    }

    free(strings);  // malloced by backtrace_symbols
    }
  }

  siginfo_t ptrace_getsiginfo(pid_t target)
  {
      siginfo_t targetsig;
      if(ptrace(PTRACE_GETSIGINFO, target, NULL, &targetsig) == -1)
      {
        //  fprintf(stderr, "ptrace(PTRACE_GETSIGINFO) failed\n");
        //  exit(1);
      }
      return targetsig;
  }

  void Signal_number(siginfo_t sig)
    {
    print("******************** Signal Catched ********************\n" );

      switch(sig.si_signo)
        {
          case SIGSEGV :
                    print("Got signal %d :  	seg fault.\n", sig.si_signo);
                    break;
          case SIGFPE  :
                    print("Got signal %d :  	failing instruction.\n", sig.si_signo);
                    break;
          case SIGBUS  :
                    print("Got signal %d :  	Bad memory access (SIGBUS) .\n", sig.si_signo);

          case SIGXFSZ :
                      print("Got signal %d :  	exceeded file size.\n ", sig.si_signo);
                      break;
          case SIGPIPE :
                      print("Got signal %d :  	the pipe has no reader.\n", sig.si_signo);
                      break;
          case SIGXCPU:
                      print("Got signal %d :  	 exceed of limit processor time consumed.\n", sig.si_signo);
                      break;


        /*
          default      :
                      print("no signal catched\n");
                      break;*/
        }

    }


int main(int argc, char **argv)
{
    pid_t child;
    siginfo_t targetsig;

   if (argc < 3) {
        fprintf(stderr, "Expected <program name> and <breakpoint address as hex string> as argument\n");
        return -1;
    }

    child = fork();//creat a new process
    if (child == 0){   //child process
      ptrace(PTRACE_TRACEME, 0, 0, 0);
      execve(argv[1], argv + 1, NULL);
    }
    else if (child > 0) { //parent process


      uint64_t addr = (uint64_t) strtol(argv[2], NULL, 16);//get address of breakpoint
      breakpoint(child, addr);
    // do_backtrace(child);

     do_backtrace(child,targetsig);

     targetsig = ptrace_getsiginfo(child);
     Signal_number(targetsig);

    do_backtrace(child,targetsig);
        //do_backtrace(child);

      }


    else {
        perror("fork");
        return -1;
    }

    return 0;
}
