//------------------------------------------------------------------------------
// Copyright 2003 Tom Haines
#include "eos/mt/locks.h"

#include "eos/mt/threads.h"

#ifdef WIN32
 #include <windows.h>
#else
 #include <sys/types.h>
 #include <sys/ipc.h>
 #include <sys/sem.h>
#endif

namespace eos
{
 namespace mt
 {
//------------------------------------------------------------------------------
#ifdef EOS_WIN32

OwnedLock::OwnedLock()
{
 hand = mem::Malloc<CRITICAL_SECTION>();
 InitializeCriticalSection((CRITICAL_SECTION*)hand);	
}

OwnedLock::~OwnedLock()
{
 DeleteCriticalSection((CRITICAL_SECTION*)hand);
 mem::Free(hand);	
}

void OwnedLock::Lock() const
{
 EnterCriticalSection((CRITICAL_SECTION*)hand);	
}

bit OwnedLock::TryLock() const
{
 if (TryEnterCriticalSection((CRITICAL_SECTION*)hand)) return true;
                                                  else return false;	
}

void OwnedLock::Unlock() const
{
 LeaveCriticalSection((CRITICAL_SECTION*)hand);
}

bit OwnedLock::Locked() const
{
 if (TryEnterCriticalSection((CRITICAL_SECTION*)hand))
 {
  LeaveCriticalSection((CRITICAL_SECTION*)hand);
  return false;
 }
 else return true;	
}

#else

OwnedLock::OwnedLock()
{
 id = semget(IPC_PRIVATE,1,IPC_CREAT | 0x01FF);
 int val = 1;
 semctl(id,0,SETVAL,val); // Make it start unlocked.
}

OwnedLock::~OwnedLock()
{
 semctl(id,0,IPC_RMID);	
}

void OwnedLock::Lock() const
{
 sembuf op;
  op.sem_num = 0;
  op.sem_op = -1;
  op.sem_flg = 0;
 semop(id,&op,1);
}

bit OwnedLock::TryLock() const
{
 sembuf op;
  op.sem_num = 0;
  op.sem_op = -1;
  op.sem_flg = IPC_NOWAIT;
 return semop(id,&op,1)==0;
}

void OwnedLock::Unlock() const
{
 sembuf op;
  op.sem_num = 0;
  op.sem_op = 1;
  op.sem_flg = 0;
 semop(id,&op,1);
}

bit OwnedLock::Locked() const
{
 return semctl(id,0,GETVAL)==0;
}

#endif

//------------------------------------------------------------------------------
#ifdef EOS_WIN32

EventLock::EventLock()
{
 hand = CreateSemaphore(0,0,0xFFFFFFFF,0);	
}

EventLock::~EventLock()
{
 CloseHandle((HANDLE)hand);
}

void EventLock::Add(nat32 n)
{
 ReleaseSemaphore((HANDLE)hand,n,0);	
}

bit EventLock::Get(nat32 timeout)
{
 if (timeout==0xFFFFFFFF) timeout = INFINITE;
 return WaitForSingleObject((HANDLE)hand,timeout)==WAIT_OBJECT_0;
}

#else

EventLock::EventLock()
{
 id = semget(IPC_PRIVATE,1,IPC_CREAT | 0x01FF);
}

EventLock::~EventLock()
{
 semctl(id,0,IPC_RMID);	
}

void EventLock::Add(nat32 n)
{
 sembuf op;
  op.sem_num = 0;
  op.sem_op = n;
  op.sem_flg = 0;
 semop(id,&op,1);
}

bit EventLock::Get(nat32 timeout)
{
 if (timeout==0xFFFFFFFF)
 {
  sembuf op;
   op.sem_num = 0;
   op.sem_op = -1;
   op.sem_flg = 0;
  return semop(id,&op,1)==0;   
 }
 else
 {
  sembuf op;
   op.sem_num = 0;
   op.sem_op = -1;
   op.sem_flg = 0;

  timespec to;
   to.tv_sec = timeout/1000;
   to.tv_nsec = (timeout%1000)*1000000;

  return semtimedop(id,&op,1,&to)==0;
 }
}

#endif
//------------------------------------------------------------------------------
#ifdef EOS_WIN32

PassLockCode::PassLockCode()
:ptr(null<void*>())
{
 hand = CreateSemaphore(0,0,1,0);	
}

PassLockCode::~PassLockCode()
{
 CloseHandle((HANDLE)hand);
}

void * PassLockCode::Get(nat32 timeout)
{
 if (timeout==0xFFFFFFFF) timeout = INFINITE;
 if (WaitForSingleObject((HANDLE)hand,timeout)!=WAIT_OBJECT_0) return null<void*>();;
 void * ret = ptr; ptr = null<void*>();
 return ret;
}

void PassLockCode::Set(void * p)
{
 ptr = p;
 ReleaseSemaphore((HANDLE)hand,1,0);
}

#else

PassLockCode::PassLockCode()
:ptr(null<void*>())
{
 id = semget(IPC_PRIVATE,1,IPC_CREAT | 0x01FF);
}

PassLockCode::~PassLockCode()
{
 semctl(id,0,IPC_RMID);
}

void * PassLockCode::Get(nat32 timeout)
{
 if (timeout==0xFFFFFFFF)
 {
  sembuf op;
   op.sem_num = 0;
   op.sem_op = -1;
   op.sem_flg = 0;
  if (semop(id,&op,1)!=0) return null<void*>();
 }
 else
 {
  sembuf op;
   op.sem_num = 0;
   op.sem_op = -1;
   op.sem_flg = 0;

  timespec to;
   to.tv_sec = timeout/1000;
   to.tv_nsec = (timeout%1000)*1000000;

  if (semtimedop(id,&op,1,&to)!=0) return null<void*>();
 }
 
 void * ret = ptr;
 ptr = null<void*>();
 return ret;
}

void PassLockCode::Set(void * p)
{
 ptr = p;

 sembuf op;
  op.sem_num = 0;
  op.sem_op = 1;
  op.sem_flg = 0;
 semop(id,&op,1);
}

#endif
//------------------------------------------------------------------------------
#ifdef EOS_WIN32

GateLock::GateLock()
{
 hand = CreateEvent(0,TRUE,FALSE,0);	
}

GateLock::~GateLock()
{
 CloseHandle((HANDLE)hand);
}

bit GateLock::Queue(nat32 timeout)
{
 if (timeout==0xFFFFFFFF) timeout = INFINITE;
 return WaitForSingleObject((HANDLE)hand,timeout)==WAIT_OBJECT_0;
}

void GateLock::Open()
{
 SetEvent((HANDLE)hand);	
}

void GateLock::Close()
{
 ResetEvent((HANDLE)hand);	
}

void GateLock::Pulse()
{
 PulseEvent((HANDLE)hand);
}

bit GateLock::IsOpen()
{
 return WaitForSingleObject((HANDLE)hand,0)!=WAIT_TIMEOUT;
}

#else

GateLock::GateLock()
{
 id = semget(IPC_PRIVATE,1,IPC_CREAT | 0x01FF);
}

GateLock::~GateLock()
{
 semctl(id,0,IPC_RMID);
}

bit GateLock::Queue(nat32 timeout)
{
 sembuf op;
  op.sem_num = 0;
  op.sem_op = 0;
  op.sem_flg = 0;

 if (timeout==0xFFFFFFFF)
 {
  return semop(id,&op,1)==0;
 }
 else
 {
  timespec to;
   to.tv_sec = timeout/1000;
   to.tv_nsec = (timeout%1000)*1000000;

  return semtimedop(id,&op,1,&to)==0;
 }
}

void GateLock::Open()
{
 int val = 0;
 semctl(id,0,SETVAL,val);	
}

void GateLock::Close()
{
 int val = 1;
 semctl(id,0,SETVAL,val);	
}

void GateLock::Pulse()
{
 Open();
 Sleep(0);
 Close();
}

bit GateLock::IsOpen()
{
 return semctl(id,0,GETVAL)==0;
}

#endif
//------------------------------------------------------------------------------
 };
};
