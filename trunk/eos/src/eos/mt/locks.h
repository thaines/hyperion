#ifndef EOS_MT_LOCKS_H
#define EOS_MT_LOCKS_H
//------------------------------------------------------------------------------
// Copyright 2003 Tom Haines

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


/// \file locks.h
/// Provides many varieties of mutex, designed for many different scenarios.

#include "eos/types.h"

namespace eos
{
 namespace mt
 {
//------------------------------------------------------------------------------
/// The standard locked/unlocked mutex, where only one thread can have it locked
/// at any given time.
class EOS_CLASS OwnedLock
{
 public:
  /// Initialised unlocked.
   OwnedLock();
   
  /// &nbsp;
   ~OwnedLock();


  /// Locks the lock, blocks if another thread has allready locked it.
  /// This has no timeout, so be careful.
    void Lock() const;

  /// This trys to lock the mutex, returns true if it locks it in which case
  /// this thread has the lock and must unlock it when done.
  /// If it returns false then another thread has allready locked it,
  /// you can now call Lock or give up.
   bit TryLock() const;

  /// Unlocks a thread, must be called the same number of times as Lock(),
  /// or succesful attempts at TryLock().
   void Unlock() const;

   
  /// Allows you to poll the state of the lock, you should never do this,
  /// just provided for completness/just in case.
   bit Locked() const;


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::mt::OwnedLock";}


 private:
  #ifdef EOS_WIN32
   void * hand;
  #else
   int id;
  #endif
};

//------------------------------------------------------------------------------
/// This helper class is simply given a lock, its constructor locks it, its
/// deconstructor unlocks it. Useful for repeated returns.
class EOS_CLASS AutoLock
{
 public:
  /// &nbsp;
   AutoLock(OwnedLock & l):lock(l) {lock.Lock();}
   
  /// &nbsp;
   ~AutoLock() {lock.Unlock();}


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::mt::AutoLock";}


 private:
  OwnedLock & lock;	
};

//------------------------------------------------------------------------------
/// This is essentially a counter, you incriment it by adding events, and 
/// decriment it by removing them. If you try and remove an event when none
/// exist, i.e. the counter is zero, it blocks until one is added.
/// Ushally used alongside a queue of jobs that need doing, with several threads
/// awaiting events/items and other threads adding events/items.
class EOS_CLASS EventLock
{
 public:
  /// Initialises to no events, i.e. Get will block until Add is called.
   EventLock();
   
  /// &nbsp;
   ~EventLock();
   
   
  /// Adds n events, i.e. incriments the counter.
   void Add(nat32 n = 1);
   
  /// Removes an event, if no events are avaliable to be removed it blocks
  /// until an event is avaliable to be removed.
  /// \param timeout How long it waits before giving up, in milliseconds. 0xFFFFFFFF is taken to mean infinty.
  /// \returns true if an event was removed, false if it timedout.
   bit Get(nat32 timeout = 0xFFFFFFFF);


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::mt::EventLock";}  


 private:
  #ifdef EOS_WIN32
   void * hand;
  #else
   int id;
  #endif
};

//------------------------------------------------------------------------------
// Code for below.
class EOS_CLASS PassLockCode
{
 protected:
   PassLockCode();
  ~PassLockCode();
 
  void * Get(nat32 timeout);
  void Set(void * ptr);


 private:
  #ifdef EOS_WIN32
   void * hand;
  #else
   int id;
  #endif
  void * volatile ptr;	
};

//------------------------------------------------------------------------------
/// This is used to pass a pointer between threads, quite specific in its operation.
/// The general use is to tell something to create/do etc something then to call
/// Get. Get then blocks until something else calls Set, at which point the data
/// item is returned and the object reset to be used again. If Set happens to be 
/// called before Get then Get will return instantly and it will simply continue.
/// Calling Get/Set when someone else has allready called it is not allowed.
template <typename T>
class EOS_CLASS PassLock : public PassLockCode
{
 public:
  /// &nbsp;
   PassLock() {}
   
  /// &nbsp;
   ~PassLock() {}


  /// This blocks until Set has been called, adn then returns that which Set was
  /// called with. If Set has allready been called it returns instantly.
  /// Returns null on both a timeout and if Set is called with null.
  /// timeout is inmilliseconds, with 0xFFFFFFFF taken to be infinity.
   T * Get(nat32 timeout = 0xFFFFFFFF) {return static_cast<T*>(PassLockCode::Get(timeout));}
   
  /// Sets the pointer, allowing the Get method to return. Must not be called
  /// again until Get has in fact returned.
   void Set(T * ptr) {PassLockCode::Set(ptr);}


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::mt::PassLock";}
};

//------------------------------------------------------------------------------
/// I am not entirelly sure what this is used for, but the idea is simple enough.
/// You have a gate, when its closed threads behind the gate queue up and wait,
/// when open they all charge through. The gate can be open or closed. It can also
/// be 'pulsed', i.e. opened to let everybody queuing through before being 
/// instantly closed again.
class EOS_CLASS GateLock
{
 public:
  /// The gate is constructed closed.
   GateLock();
   
  /// &nbsp;
   ~GateLock();


  /// Tries to walk through the gate, if its open it returns instantly, if its
  /// closed it waits for someone to open it, or timesout. Returns true if it 
  /// walks through, false if it timesout.
   bit Queue(nat32 timeout = 0xFFFFFFFF);
   
   
  /// Opens the gate.
   void Open();
   
  /// Closes the gate.
   void Close();

  /// Lets everybody currently queueing through the gate but leaves it closed.
  /// If the gate is open it just sets it to be closed.
   void Pulse();

   
  /// Retursn true if the gate is open, false if it is closed.
   bit IsOpen();


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::mt::GateLock";}


 private:
  #ifdef EOS_WIN32
   void * hand;	
  #else
   int id;
  #endif
};

//------------------------------------------------------------------------------
 };
};
#endif
