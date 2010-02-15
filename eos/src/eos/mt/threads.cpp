//------------------------------------------------------------------------------
// Copyright 2003 Tom Haines
#include "eos/mt/threads.h"

#include "eos/mem/alloc.h"

//------------------------------------------------------------------------------

#ifdef WIN32
 #include <windows.h>
#else
 #include <unistd.h>
 #include <sys/types.h>
 #include <stdio.h>
 #include <sys/mman.h>
 #include <sched.h>
 #include <sys/wait.h>
 #include <signal.h>
 #include <sys/time.h> 
 #include <sys/resource.h>
#endif

namespace eos
{
 namespace mt
 {
//------------------------------------------------------------------------------
#ifdef WIN32

EOS_FUNC void Sleep(nat32 ms)
{
 ::Sleep(ms);	
}

EOS_FUNC nat32 ThreadID()
{
 return GetCurrentThreadId();	
}

EOS_FUNC nat32 CoreCount()
{
 SYSTEM_INFO si;
 GetSystemInfo(&si);
 return si.dwNumberOfProcessors;
}

#else

EOS_FUNC void Sleep(nat32 ms)
{
 ::usleep(ms*1000);	
}

EOS_FUNC nat32 ThreadID()
{
 return getpid();
}

EOS_FUNC nat32 CoreCount()
{
 nat32 ret = 0;
 // Open '/proc/cpuinfo' and count the number of lines that start 'processor'...
  FILE * f = fopen("/proc/cpuinfo","r");
  if (f)
  {
   while (!feof(f))
   {
    char line[256];
    fgets(line,256,f);
    if ((line[0]=='p')&&
        (line[1]=='r')&&
        (line[2]=='o')&&
        (line[3]=='c')&&
        (line[4]=='e')&&
        (line[5]=='s')&&
        (line[6]=='s')&&
        (line[7]=='o')&&
        (line[8]=='r'))
    {
     ++ret;
    }
   }
   fclose(f);
  }

 return (ret==0)?1:ret; // I consider it good practise to return at least 1, being told there are no cpus can be something of a mind blowing event for a running process.
}

#endif

//------------------------------------------------------------------------------
// Helper function, used by the below class...

#ifdef EOS_WIN32
DWORD WINAPI 
#else
int
#endif
thread_func(void * data)
{
 try
 {
  static_cast<Thread*>(data)->Execute();
 }
 catch(...)
 {
  return 1;
 }
 return 0;
}

//------------------------------------------------------------------------------
#ifdef WIN32

Thread::Thread()
:hand(null<void*>())
{}

Thread::~Thread()
{
 Kill();
 if (hand) CloseHandle((HANDLE)hand);
}

bit Thread::Run()
{
 unsigned long temp;
 hand = (void*)CreateThread(0,0,thread_func,this,0,&temp);
 return hand!=0;
}

void Thread::Kill()
{
 if (hand) TerminateThread((HANDLE)hand,1);
}

bit Thread::Running() const
{
 unsigned long code;
 if (GetExitCodeThread((HANDLE)hand,&code)==0) return false;
 return code==STILL_ACTIVE;
}

bit Thread::Error() const
{
 unsigned long code;
 if (GetExitCodeThread((HANDLE)hand,&code)==0) return true;
 if (code==STILL_ACTIVE) return false;
 return code!=0;
}

void Thread::Wait() const
{
 WaitForSingleObject((HANDLE)hand,INFINITE);	
}

void Thread::Priority(bit high)
{
 if (high) SetThreadPriority((HANDLE)hand,THREAD_PRIORITY_HIGHEST);
      else SetThreadPriority((HANDLE)hand,THREAD_PRIORITY_NORMAL);	
}

#else

Thread::Thread()
:stack(null<void*>()),procId(-1)
{}

Thread::~Thread()
{
 Kill();
}

bit Thread::Run()
{
 // Create the stack, using mmap...
  stack = mmap(0,start_size,PROT_EXEC | PROT_READ | PROT_WRITE,MAP_PRIVATE | MAP_GROWSDOWN | MAP_ANONYMOUS,0,0);
  if (stack==null<void*>()) return false;
 
 // Clone to create the new kernel space thread...
  procId = clone(thread_func,stack,SIGCHLD | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_PTRACE | CLONE_VM,this);

 return procId!=-1;
}

void Thread::Kill()
{
 if (procId!=-1) kill(procId,SIGKILL);
 if (stack) munmap(stack,start_size);
}

bit Thread::Running() const
{
 if (procId==-1) return false;
 return waitpid(procId,0,WNOHANG)==0;
}

bit Thread::Error() const
{
 if (procId==-1) return true;
 int status;
 int res = waitpid(procId,&status,WNOHANG);
 if (res!=procId)
 {
  if (res==0) return false;
         else return true;
 }
 return !WIFEXITED(status) || (WEXITSTATUS(status)!=0);
}

void Thread::Wait() const
{
 if (procId!=-1) waitpid(procId,0,0);
}

void Thread::Priority(bit high)
{
 if (high) setpriority(PRIO_PROCESS,procId,0);
      else setpriority(PRIO_PROCESS,procId,-1);
}

#endif

//------------------------------------------------------------------------------
 };
};
