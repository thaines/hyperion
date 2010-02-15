//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/log/logs.h"

#include "eos/time/times.h"
#include "eos/file/csv.h"
#include "eos/ds/sort_lists.h"
#include "eos/mt/threads.h"
#include "eos/mt/locks.h"

#ifdef EOS_LINUX
 #include <execinfo.h>
 #include <ucontext.h>
#endif

namespace eos
{
 namespace log
 {
//------------------------------------------------------------------------------
#ifdef EOS_LINUX
void SignalEatter(int sig,siginfo_t * info,void * context)
{
 // Log the signal, special case a seg fault...
  switch (sig)
  {
   case SIGSEGV:
    LogLog("[exe] Segmentation Fault (" << sig << ")");
   break;
   case SIGILL:
    LogLog("[exe] Illegal Instruction (" << sig << ")");
   break;
   case SIGFPE:
    LogLog("[exe] Floating Point Exception (" << sig << ")");
   break;
   case SIGKILL:
    LogLog("[exe] Kill Signal (" << sig << ")");
   break;
   case SIGINT:
    LogLog("[exe] Interrupt (" << sig << ")");
   break;          
   default:
    LogLog("[exe] Got signal " << sig);
   break;
  }

 // Log the stack...
  void * trace[256];
  int trace_size = backtrace(trace,256);
  #ifdef REG_RIP
   trace[1] = (void *)((ucontext_t*)context)->uc_mcontext.gregs[REG_RIP];
  #else
  #ifdef REG_EIP
   trace[1] = (void *)((ucontext_t*)context)->uc_mcontext.gregs[REG_EIP];
  #else
   trace[1] = 0;
  #endif
  #endif
  char ** msg = backtrace_symbols(trace,trace_size);
  
  for (int i=1;i<trace_size;i++)
  {
   LogLog("[exe] Stack -" << i << LogDiv() << msg[i]);
  }
  free(msg);
  
 // Die...
  exit(1);
}
#endif
//------------------------------------------------------------------------------
// Helper structure that stores profile information.
struct Block
{
 bit operator < (const Block & rhs) const {return name<rhs.name;}
  
 cstrconst name;
   
 nat32 calls;

 real64 realSum;
 real64 userSum;
 real64 systemSum;
};

//------------------------------------------------------------------------------
// Helper class that defines all logging data...
class EOS_CLASS LogStore
{
 public: 
  nat32 startTime;
  nat32 lastTime;
  ds::SortList<Block> profile;
  mt::OwnedLock logLock;
  
  LogStore()
  {
   startTime = time::Time();
   lastTime = startTime;
  }  
};

//------------------------------------------------------------------------------
LogObj::LogObj()
:n(0),depth(0),
out(null<file::Csv*>()),
ls(new LogStore())
{
 #ifdef EOS_LINUX
  // Setup a signal handler to catch things such as segmentation faults etc...
   struct sigaction sa;
   sa.sa_sigaction = SignalEatter;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = SA_RESTART | SA_SIGINFO;

   sigaction(SIGSEGV,&sa,0);
   sigaction(SIGILL,&sa,0);
   sigaction(SIGTERM,&sa,0);
   sigaction(SIGINT,&sa,0);
 #endif
}

LogObj::~LogObj()
{
 // If logging has happened add in the closing line and close the file...
  if (out)
  {
   // Add in a 'program closing line'...
    LogLog("[exe] End");
    delete out;
  }
  
 // If any profiling details were recorded write out the profilling information
 // to a file, name the file based on the time the program closes...
  if (ls->profile.Size()!=0)
  {
   time::Date startTime(false,ls->startTime);
  
   cstrchar fn[256];
   str::Copy(fn,"profile_");
   str::Append(fn,startTime.Str("Y-m-d_H-i-s"));
   str::Append(fn,".csv");  
    
   file::Csv po(fn);
   if (po.Active())
   {
    po << "block count = " << ls->profile.Size() << file::EndRow();
   
    po << "block" << file::EndField()
       << "calls" << file::EndField()
       << "real total" << file::EndField()
       << "user total" << file::EndField()
       << "system total" << file::EndField()
       << "real mean" << file::EndField()
       << "user mean" << file::EndField()
       << "system mean" << file::EndRow();
       
    nat32 callSum = 0;
    for (nat32 i=0;i<ls->profile.Size();i++)
    {
     Block & targ = ls->profile[i];
      
     po << targ.name << file::EndField()
        << targ.calls << file::EndField()
        << targ.realSum << file::EndField()
        << targ.userSum << file::EndField()
        << targ.systemSum << file::EndField()
        << (targ.realSum/real64(targ.calls)) << file::EndField()
        << (targ.userSum/real64(targ.calls)) << file::EndField()
        << (targ.systemSum/real64(targ.calls)) << file::EndRow();
        
     callSum += targ.calls;
    }
    
    po << file::EndRow();
    po << "total" << file::EndField() 
       << callSum << file::EndRow();
   }
  }
  
 // Delete the LogStore object...
  delete ls;
}

file::Csv & LogObj::BeginEntry()
{
 nat32 now = time::Time();
 nat32 milli = nat32(time::MilliTime()%1000);
 ls->logLock.Lock();

 // Check for no logging having actually occured upto this point, i.e. no file is open...
  if (out==null<file::Csv*>())
  {
   // No logging has yet occured - we must open the output file and log the start of the program.
   // (It allready happened, quite possibly hours ago.)
    time::Date startTime(false,ls->startTime);
    CreateFile(&startTime);

    *out << n << file::EndField()
         << startTime.Str("Y-m-d H:i:s ") << milli << file::EndField()
         << "-" << file::EndField()
         << "0" << file::EndField() // 0 rather than depth incase the first logging is done by a block logger.
         << "[exe] Begin" << file::EndRow();

    ++n;
    ls->lastTime = now;
  }
 
 // Check if we have changed date since the last call, if so open a new file...
  time::Date lastTime(false,ls->lastTime);
  time::Date nowTime(false,now);
 
  if (lastTime.day!=nowTime.day)
  {
   delete out;
   CreateFile(&nowTime);
  }

 // Write the standard header log information...
  *out << n << file::EndField()
       << nowTime.Str("Y-m-d H:i:s ") << milli << file::EndField()
       << mt::ThreadID() << file::EndField()
       << depth << file::EndField();

  ++n;
  ls->lastTime = now;

 // Return the Csv object so the user can add there stuff...
  return *out; 
}

void LogObj::EndEntry()
{
 (*out) << file::EndRow();
 out->Flush();
 ls->logLock.Unlock();
}

void LogObj::StoreBlock(cstrconst name,real64 real,real64 user,real64 system)
{
 Block dummy;
  dummy.name = name;
 Block * targ = ls->profile.Get(dummy);
 if (targ)
 {
  targ->calls += 1;
  targ->realSum += real;
  targ->userSum += user;
  targ->systemSum += system;
 }
 else
 {
  dummy.calls = 1;
  dummy.realSum = real;
  dummy.userSum = user;
  dummy.systemSum = system;
  ls->profile.Add(dummy);
 }
}

void LogObj::CreateFile(time::Date * d)
{
 /*file::Dir dir(file::WorkDir);
 dir.Go("logs");
 if (dir.Create()==false) abort(); // No choice...
*/
 cstrchar fn[256];
  str::Copy(fn,"log_"); // logs/
  str::Append(fn,d->Str("Y-m-d_H-i-s"));
  str::Append(fn,".csv");  
    
 out = new file::Csv(fn);
 if (!out->Active())
 {
  delete out; out = null<file::Csv*>();
  abort(); // No choice in the matter - in all cases a crash is imminant, might as well pull the trigger ourself.
 }
 
 *out << "#" << file::EndField()
      << "time" << file::EndField()
      << "thread" << file::EndField()
      << "depth" << file::EndField()
      << "details" << file::EndRow();
}

//------------------------------------------------------------------------------
// The actual decleration of the logging object...
EOS_VAR_DEF LogObj logger;

//------------------------------------------------------------------------------
BlockLogger::BlockLogger(cstrconst n,bit le)
:name(n),logEnd(le)
{
 real = time::UltraTime();
 time::ThreadTime(user,system);
}

BlockLogger::~BlockLogger()
{
 real64 endUser = 0.0,endSystem = 0.0;
 time::ThreadTime(endUser,endSystem);
 real64 endReal = time::UltraTime();
 
 logger.StoreBlock(name,endReal-real,endUser-user,endSystem-system); 
 
 if (logEnd)
 {
  LogLog("[block] End {name}" << LogDiv() << name);
 }
 logger.Depth() -= 1;
}

//------------------------------------------------------------------------------
EOS_FUNC void AssertFailure(cstrconst error)
{
 LogLog("[exe] Assert failed {error msg}" << LogDiv() << error);

 #ifdef EOS_LINUX
 {
  void * trace[256];
  int trace_size = backtrace(trace,256);
  char ** msg = backtrace_symbols(trace,trace_size);
  
  for (int i=0;i<trace_size;i++)
  {
   LogLog("[exe] Stack -" << i << LogDiv() << msg[i]);
  }
  free(msg);
 }
 #endif

 #ifdef EOS_WIN32
  byte * v = null<byte*>();
  while (v[0]) ++v; // To force a crash under windows - gdb just won't catch it otherwise.
 #endif

 abort();
}

//------------------------------------------------------------------------------
 };
};
