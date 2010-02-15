#ifndef EOS_DS_PRIORITY_QUEUES_H
#define EOS_DS_PRIORITY_QUEUES_H
//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines

/// \file priority_queues.h
/// A standard priority queue implimentation, using a heap internally.

#include "eos/types.h"
#include "eos/typestring.h"

#include "eos/mem/safety.h"
#include "eos/ds/sorting.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
// The code part of the priority queue flat template...
class EOS_CLASS PriorityQueueCode
{
 protected:
   PriorityQueueCode(nat32 elementSize,nat32 startSize);
   PriorityQueueCode(nat32 elementSize,const PriorityQueueCode & rhs);
  ~PriorityQueueCode();

  void Del(nat32 elementSize,void (*Term)(void * ptr));
    
  void Copy(nat32 elementSize,const PriorityQueueCode & rhs);

  
  void Add(nat32 elementSize,void * in,bit (*LessThan)(void * lhs,void * rhs)); // Copys the data in.
  void * Peek() const {return data;}
  void Rem(nat32 elementSize,bit (*LessThan)(void * lhs,void * rhs)); // Returns nothing - the user must peek before death.


 nat32 size; // How big the array is.
 nat32 elements; // How many elements are in it.
 byte * data;
 
 static const nat32 growthFactor = 32;
};

//------------------------------------------------------------------------------
/// A standard priority queue implimentation, internally a heap in a fixed size
/// block of memory, so knowing how many items you need to deal with helps. Note
/// that when you remove an item from the queue it is *not* deleted, as you will
/// often want to take it with you, if you need to delete it that is the users
/// responsability. The mem mode should only be changed for pointer types, as it
/// can only work with pointers. The sorting operation is used so the smallest 
/// items are returned first.
template <typename T, typename SO = SortOp<T>, typename DT = mem::KillNull<T> >
class EOS_CLASS PriorityQueue : public PriorityQueueCode
{
 public:
  /// &nbsp;
   PriorityQueue(nat32 startSize = 15):PriorityQueueCode(sizeof(T),startSize) {}
   
  /// The copy constructor is templated on the type of DT, so any DT type can be
  /// assigned to any other. Note that assignments should only be a deleting type
  /// and a non-deleting type, as between two deleting types will result in double
  /// deletion. Which is bad.
   template <typename DTT>
   PriorityQueue(const PriorityQueue<T,SO,DTT> & rhs):PriorityQueueCode(sizeof(T),rhs) {}
   
  /// &nbsp;
   ~PriorityQueue() {Del(sizeof(T),&DelFunc);}
 

  /// This makes the priority queue empty, i.e. Size()==0.
   void MakeEmpty() {Del(sizeof(T),&DelFunc); elements = 0;}

  /// This works on the same design principals as the copy constructor.
   template <typename DTT>
   PriorityQueue<T,SO,DT> operator = (const PriorityQueue<T,SO,DTT> & rhs) {Del(sizeof(T),&DelFunc); Copy(sizeof(T),rhs); return *this;}

   
  /// Returns how many items are in the table.
   nat32 Size() const {return elements;}
   
  /// Returns how much memory the table is consuming.
   nat32 Memory() const {return sizeof(T)*size;}
   
   
  /// This adds an item to the priority queue, copying in its data.
   void Add(const T & rhs) {PriorityQueueCode::Add(sizeof(T),(void*)&rhs,&SortFunc);} 
   
  /// This returns a reference to the top item in the queue, note that as Rem does
  /// not give you access to the object you will need to use this.
   T & Peek() const {return *(T*)PriorityQueueCode::Peek();}
   
  /// Removes the top item from the queue, do not call if Size()==0 as it will 
  /// all go pear shaped. Note that it does not call the deallocator on the object
  /// as its removed, on the ground it will often be wanted elsewhere. If the user 
  /// wants to delete it they must do it themselves.
   void Rem() {PriorityQueueCode::Rem(sizeof(T),&SortFunc);}
    
   
  /// &nbsp;
   static inline cstrconst TypeString() 
   {
    static GlueStr ret(GlueStr() << "eos::ds::PriorityQueue<" << typestring<T>() << "," << typestring<SO>() << "," << typestring<DT>() << ">");
    return ret;
   }
   
   
 protected:  
  static bit SortFunc(void * lhs,void * rhs)
  {
   return SO::LessThan(*((T*)lhs),*((T*)rhs));
  }
      
  static void DelFunc(void * ptr)
  {
   DT::Kill((T*)ptr);
  }
};

//------------------------------------------------------------------------------
 };
};
#endif
