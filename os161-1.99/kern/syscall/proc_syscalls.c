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
#include <mips/trapframe.h>
#include "opt-A2.h"
#include <synch.h>

#if OPT_A2
  /* this implementation of sys__exit does not do anything with the exit code */
  /* this needs to be fixed to get exit() and waitpid() working properly */

void sys__exit(int exitcode) {

  DEBUG(DB_SYSCALL,"Syscall: _exit(%d)\n",exitcode);

  struct addrspace *as;
  struct proc *p = curproc;
  
  bool can_delete = false;
  
  for (unsigned int i = 0 ; i < array_num(p->children); i++){
    struct proc *child = (struct proc *)array_get(p->children, i);
    if (!proc_check_alive(child)){
      proc_destroy(child);
      array_remove(p->children, i);
      i--;
    }
  }

  // a proc can only delete itself when its parent is NULL or dead
  lock_acquire(p->p_thread_lock);
  if (!p->parent || !p->parent->is_alive){
    can_delete = true;
  }
  lock_release(p->p_thread_lock);

  

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
  if (can_delete){
    proc_destroy(p);
  } else {
    proc_set_dead(p, exitcode);
  }
  
  
  thread_exit();
  /* thread_exit() does not return, so we should never get here */
  panic("return from thread_exit in sys_exit\n");
}


/* stub handler for getpid() system call                */
int
sys_getpid(pid_t *retval)
{
  spinlock_acquire(&curproc->p_lock);
  *retval = curproc->pid;
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
  if (options != 0) {
    return(EINVAL);
  }

  int exitstatus;
  int result;

  bool pid_is_children = false;
  //check children's pid
  for (unsigned int i = 0; i < array_num(curproc->children); i++){
    struct proc* child = array_get(curproc->children, i);
    if (child->pid == pid){
      pid_is_children = true;
      lock_acquire(child->p_thread_lock);
      //wait for the child to exit
      while (child->is_alive){
        cv_wait(child->p_cv, child->p_thread_lock);
      }
      exitstatus = _MKWAIT_EXIT(child->exit_code);
      lock_release(child->p_thread_lock);
      break;
    }
  }
  
  //if not found, return err
  if (!pid_is_children){
    *retval=-1;
    return (ESRCH);
  }



  
  /* for now, just pretend the exitstatus is 0 */
  
  result = copyout((void *)&exitstatus,status,sizeof(int));
  if (result) {
    return(result);
  }
  *retval = pid;
  return(0);
}



int sys_fork(struct trapframe* tf, pid_t* retval){
  //step1 create child proc
  struct proc* child = proc_create_runprogram("child");
  if (!child) {
    *retval = (pid_t)-1;
    return ENOMEM;
  }

  //step2 create as and copy
  struct addrspace* as = NULL;
  int error = as_copy(curproc_getas(), &as);
  if (error){
    proc_destroy(child);
    *retval = (pid_t)-1;
    return error;
  }
  spinlock_acquire(&child->p_lock);
  child->p_addrspace = as;
  spinlock_release(&child->p_lock);

  //step4 create a thread
  struct trapframe* parent_tf = kmalloc(sizeof(struct trapframe));
  if (!parent_tf) {
    *retval = (pid_t)-1;
    return ENOMEM;
  }
  spinlock_acquire(&curproc->p_lock);
  if (!tf){
    error = ENOMEM;
  } else {
    *parent_tf = *tf;
  }
  spinlock_release(&curproc->p_lock);
  if (error){
    *retval = (pid_t)-1;
    proc_destroy(child);
    return error;
  }
  //step3 create child/parent relation
  spinlock_acquire(&curproc->p_lock);
  child->parent = curproc;
  int idx;
  error = array_add(curproc->children, (void *)child, (unsigned*)&idx);
  spinlock_release(&curproc->p_lock);
  

  if (error){
    *retval = (pid_t)-1;
    return error;
  }

  error = thread_fork("thread_c", child, enter_forked_process, (void *)parent_tf, 0);
  if (error){
    *retval = (pid_t)-1;
    proc_destroy(child);
    return error;
  }

  //step5 return pid
  *retval = child->pid;
  return 0;
}


#endif
