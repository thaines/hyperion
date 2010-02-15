#ifndef EOS_DS_SCHEDULING_H
#define EOS_DS_SCHEDULING_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

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


/// \file scheduling.h
/// Provides a data structures tuned for running scheduling algorithms to
/// order tasks. Provides various ways of scheduling, with certain 
/// enahancments over a dependancy aware sort.

#include "eos/types.h"
#include "eos/typestring.h"

#include "eos/mem/alloc.h"
#include "eos/mem/safety.h"
#include "eos/mem/preempt.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
// The code for the flat template scheduler object...
class EOS_CLASS SchedulerCode
{
 protected:
  SchedulerCode();
  ~SchedulerCode();
  
  void Del(void (*Term)(void * ptr));
    
  void * MakeVert(nat32 elementSize,const void * data); // Returns a handle to the vertex.
  void MakeEdge(void * first,void * second); // vert handle first must happen before vert handle second.
  
  void DelVert(void * handle,void (*Term)(void * ptr));
  void DelEdge(void * first,void * second);

  bit Sort(); // Makes the first linked list the correct order. Returns true on success, false on failure to having a cycle.

  
  struct Edge;
  
  struct Vert // Malloc-ed, followed by the data contained.
  {
   void * Data() const {return ((byte*)this) + sizeof(Vert);}
   
   Edge * first; // Linked list of edges pointing to nodes that must follow this one.
   nat32 incomming;
   nat32 inLeft; // Number of edges that need to be 'deleted' before we can do this node.
   Vert * next; // Linked list in execution order, so it can be traversed. Also stored in non-execution order, and used to make a queue of nodes to be proccessed during the actual algorithm run.
   Vert * prev; // Previous node, a doubly linked list is easier to manage.
  };
  
  struct Edge // Allocated using the mem::pre8 or mem::pre16 allocator, depending on platform.
  {
   Vert * to;
   Edge * next;
   
   #ifdef EOS_64BIT
   void * operator new (size_t) {return eos::mem::pre16.Malloc<Edge>();}
   void operator delete (void * ptr) {eos::mem::pre16.Free(static_cast<Edge*>(ptr));}   
   #endif
   #ifdef EOS_32BIT
   void * operator new (size_t) {return eos::mem::pre8.Malloc<Edge>();}
   void operator delete (void * ptr) {eos::mem::pre8.Free(static_cast<Edge*>(ptr));}
   #endif
  };
  
  nat32 verts;
  nat32 edges;
  Vert * first;
};

//------------------------------------------------------------------------------
/// A schedular, given a sequence of task and a set of constraints in the form 
/// of x needs to be done before y. At any time a schedule can be requested,
/// this will then provide a handle from which you can iterate the schedule in
/// the order tasks should be done. You can continue to edit after requesting a 
/// schedule, though you can't trust the handle after that point. If a cycle is
/// created making a schedule impossible the schedular will return a bad handle,
/// you can then either give up or delete constraints to remove the cycle.
template <typename T,typename DT = mem::KillNull<T> >
class EOS_CLASS Scheduler : public SchedulerCode
{
 public:
  /// &nbsp;
   Scheduler() {}
   
  /// &nbsp;
   ~Scheduler() {SchedulerCode::Del(DelFunc);}


  /// This represents a job, you can obtain the associated data from one of these,
  /// use it to delete the job, add constraints involving the job and get the next
  /// job once a schedule has been formed.
   typedef void * Job;
   
  /// Returns the bad Job, so you can assign it to Jobs you might not set.
   static Job NullJob() {return null<void*>();}


  /// Adds a new Job.
   Job AddJob(const T & item) {return MakeVert(sizeof(T),&item);}
   
  /// Deletes a job, calls the DT killer on the contained data.
   void Del(Job job) {DelVert(job,DelFunc);}
   
  /// Adds a new constraint, that first happens before second.
   void AddCon(Job first,Job second) {MakeEdge(first,second);}
   
  /// Deletes a constraint that first happens before second.
   void Del(Job first,Job second) {DelEdge(first,second);}

   
  /// Schedules the jobs, returning a Job handle to the
  /// first Job that needs doing. The handle will be bad if a schedule
  /// can not be found, i.e. a loop exists in the constraints.
   Job Schedule()
   {
    if (Sort()) return first;
           else return null<void*>();
   }


  /// Converts a Job into its associated item.
   static T & Item(Job job) {return *(T*)(((Vert*)job)->Data());}
   
  /// Given a Job returns the next Job that follows it, for traversing
  /// the sequence of jobs returned from Schedule().
   static Job Next(Job job) {return ((Vert*)job)->next;}
   
  /// Returns true if the Job is a good Job, i.e. there is data on
  /// it and it can be used, false if its a Bad job, indicating end 
  /// of sequence or that a sequence was never found in the first place.
   static bit Good(Job job) {return job!=null<void*>();}
  
  /// The inverse of good.
   static bit Bad(Job job) {return job==null<void*>();}
   
   
  /// Number of jobs stored within.
   nat32 Jobs() const {return verts;}

  /// Number of constraints stored within.
   nat32 Cons() const {return edges;}
   
  /// Amount of memory, in bytes, consumed.
   nat32 Memory() const {return verts*(sizeof(Vert)+sizeof(T)) + edges*sizeof(Edge) + sizeof(*this);}


  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::ds::Scheduler<" << typestring<T>() << "," << typestring<DT>() << ">");
    return ret;
   } 

   
 private:
  static void DelFunc(void * ptr)
  {
   DT::Kill((T*)ptr);
  } 
};

//------------------------------------------------------------------------------
 };
};
#endif
