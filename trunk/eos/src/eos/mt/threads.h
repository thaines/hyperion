#ifndef EOS_MT_THREADS_H
#define EOS_MT_THREADS_H
//------------------------------------------------------------------------------
// Copyright 2003 Tom Haines

/// \file threads.h
/// Provides the ability to create threads, this is done in the java style where
/// you inherit from a threading object adn impliment the correct method.

/// \namespace eos::mt
/// An abreviation of multi-threaded, this being the capability this module 
/// provides. (e.g. Threads and mutexes.)

#include "eos/types.h"

namespace eos
{
 namespace mt
 {
//------------------------------------------------------------------------------
/// Allows you to make a thread sleep, to avoid a thread dominating other 
/// threads. Calling it with 0 will just make it 'give way' to other threads.
/// \param ms Millisecodns to sleep for.
EOS_FUNC void Sleep(nat32 ms);

/// Returns a unique number for the current thread, so you can tell them appart.
/// Mostly used by the logging system.
EOS_FUNC nat32 ThreadID();

/// Returns how many proccessing units the computer has, i.e. the minimum
/// number of un-blocked threads required to keep the computer maxed out.
/// The sum of the number of cores for each proccessor.
EOS_FUNC nat32 CoreCount();

//------------------------------------------------------------------------------
/// To create a thread inherit from this class and impliment the Execute()
/// method, when the Run() method is then called the execute method will be run
/// in its own thread. It is the users responsibility to arrange for a
/// reasonable mechanism to stop the thread, ushally two modes are used:
/// - Thread completes regardless, i.e. it does its job then dies.
/// - Thread proccesses a work queue, doing jobs infinity, until it comes across a suicide job.
class EOS_CLASS Thread : public Deletable
{
 public:
  /// &nbsp;
   Thread();
   
  /// &nbsp;
   ~Thread();


  /// Sets a thread running. i.e. the Execute() method will be called in a
  /// seperate thread. Only call this once - the consequence of multiple calls
  /// are bad.
  /// Returns true on success, false on failure.
   bit Run();

   
  /// Instantly kills the thread in question. This is very bad, and its use
  /// should be avoided at all costs. Memory leaks etc are inevitable if this
  /// is used.
   void Kill();

   
  /// Returns true if the thread is currently running, false if it is not running.
   bit Running() const;
   
  /// Returns true if an error has happened during the execution of the thread,
  /// i.e. it has died, horribly.
   bit Error() const;


  /// This blocks until the thread finishes running.
   void Wait() const;
   
  /// Sets the priority of the thread, only two levels are provided.
  /// true is the higher level, with all threads defaulting to false.
   void Priority(bit high);


  /// This is to be overridden by the child type, running a thread consists of
  /// this method being called. Generally not called directly, but indeirectly
  /// by Run. Can be called directly if you have another thread that wants to
  /// do the job.
   virtual void Execute() = 0;


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::mt::Thread";}


 private:
  #ifdef WIN32
   void * hand;
  #else
   static const int start_size = 1024; // Starting stack size.
   void * stack;
   int procId;
  #endif
};

//------------------------------------------------------------------------------
 };
};
#endif
