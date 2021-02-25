#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/wait.h>
#include <lib.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <thread.h>
#include <addrspace.h>
#include <copyinout.h>
#include "opt-A2.h"



  /* this implementation of sys__exit does not do anything with the exit code */
  /* this needs to be fixed to get exit() and waitpid() working properly */

void sys__exit(int exitcode) {

  struct addrspace *as;
  struct proc *p = curproc;
  /* for now, just include this to keep the compiler from complaining about
     an unused variable */
  (void)exitcode;

  DEBUG(DB_SYSCALL,"Syscall: _exit(%d)\n",exitcode);

  KASSERT(curproc->p_addrspace != NULL);
  as_deactivate();
  /*
   * clear p_addrspace before calling as_destroy. Otherwise if
   * as_destroy sleeps (which is quite possible) when we
   * come back we'll be calling as_activate on a
   * half-destroyed address space. This tends to be
   * messily fatal.
   */
  as = curproc_setas(NULL);
  as_destroy(as);

  /* detach this thread from its process */
  /* note: curproc cannot be used after this call */
  proc_remthread(curthread);

  /* if this is the last user process in the system, proc_destroy()
     will wake up the kernel menu thread */
  proc_destroy(p);
  
  thread_exit();
  /* thread_exit() does not return, so we should never get here */
  panic("return from thread_exit in sys_exit\n");
}


/* stub handler for getpid() system call                */
int
sys_getpid(pid_t *retval)
{
  spinlock_acquire(&curproc->p_lock);
  *retval = 0
  spinlock_release(&curproc->p_lock);
  return(0);
}

/* stub handler for waitpid() system call                */

int
sys_waitpid(pid_t pid,
	    userptr_t status,
	    int options,
	    pid_t *retval)
{
  int exitstatus;
  int result;

  /* this is just a stub implementation that always reports an
     exit status of 0, regardless of the actual exit status of
     the specified process.   
     In fact, this will return 0 even if the specified process
     is still running, and even if it never existed in the first place.

     Fix this!
  */

  if (options != 0) {
    return(EINVAL);
  }
  /* for now, just pretend the exitstatus is 0 */
  exitstatus = 0;
  result = copyout((void *)&exitstatus,status,sizeof(int));
  if (result) {
    return(result);
  }
  *retval = pid;
  return(0);
}



int sys_fork(struct trampframe* tf, pid_t* retval){
  //step1 create child proc
  struct proc* child = proc_create_runprogram("child");
  if (!child) {
    *retval = (pid_t)-1;
    return -1;
  }

  //step2 create as and copy
  struct addrspace* as;
  int error = as_copy(curproc_getas(), &as);
  if (error){
    *retval = (pid_t)-1;
    return -1;
  }
  spinlock_acquire(&child->p_lock);
  child->p_addrspace = as;
  spinlock_release(&child->lock);

  //step3 create child/parent relation
  spinlock_acquire(&curproc->p_lock);
  child->parent = curproc;
  array_add(&curproc->children, &curproc, (unsigned*)&error);
  spinlock_release(&curproc->p_lock);
  if (error){
    *retval = (pid_t)-1;
    return error;
  }

  //step4 create a thread
  struct trampframe* parent_tf = kmalloc(sizeof(struct trampframe));
  if (!parent_tf) {
    *retval = (pid_t)-1;
    return -1;
  }
  spinlock_acquire(&curproc->p_lock);
  error = copy_trapframe(tf, parent_tf);
  spinlock_release(&curproc->p_lock);
  if (error){
    *retval = (pid_t)-1;
    return error;
  }
  error = thread_fork("thread_c", child, enter_forked_process, (void *)parent_tf, 1);
  if (error){
    *retval = (pid_t)-1;
    return error;
  }

  //step5 return pid
  *retval = child->pid;
  return 0;
}
