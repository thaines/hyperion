#ifndef EOS_DS_DIALLER_H
#define EOS_DS_DIALLER_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file dialler.h
/// This is a bad analogy. A data structure that indexes items via sets of 
/// items, i.e. each set of items has another associated item.
/// Primarilly used by optimisation procedures to cache results.

#include "eos/types.h"
#include "eos/typestring.h"

#include "eos/mem/safety.h"
#include "eos/math/functions.h"
#include "eos/ds/arrays.h"
#include "eos/ds/sort_lists.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
// Code for Dialler flat template...
class EOS_CLASS DiallerCode
{
 protected:
  struct Node
  {
   nat32 size; // How many integers are in the sorting set, size of index.
   int32 * index; // Pointer to sorted list of integers that make up indexing set. Malloced.
   // Appended to end is associated item.
   void * End() {return this+1;}
  };
 

   DiallerCode();
  ~DiallerCode();
  void DelAll(void (*Term)(void * ptr));

  void HangupCode();
  void DialCode(int32 code);
  void * PickupCode(nat32 elementSize,Node * temp,void (*MakeFunc)(void * ptr)); // temp is tempory storage.
  bit ExistsCode(Node * temp) const;
  void DelCode(Node * temp,void (*Term)(void * ptr));


 private:
  static const nat32 indexBlockSize = 16; // How many extra integers are added to the index on overflow.
 
  nat32 indexSize;
  Array<int32> index; // Array of numbers, only indexSize are used. Never shrinks.
  
  SortListCode store; // Contains instances of the above Node class.
  Node * curr; // Cached current Node, to avoid repeated calculation.


  // Sorting function...
   static bit LessThan(void * lhs,void * rhs);
};

//------------------------------------------------------------------------------
/// An indexing data structure built on a bad analogy.
/// Uses a set of integers to request each stored item. (Of type T.)
/// The sets can be of arbitary size, including the empty set, when requesting
/// the set order obviously doesn't matter, and duplicate Dial calls do nothing.
template <typename T,typename MT = mem::MakeDont<T>,typename DT = mem::KillNull<T> >
class EOS_CLASS Dialler : public DiallerCode
{
 public:
  /// &nbsp;
   Dialler() {temp = (Node*)(void*)mem::Malloc<byte>(sizeof(Node) + sizeof(T));}

  /// &nbsp;
   ~Dialler() {mem::Free(temp); DelAll(&DelFunc);}


  /// Resets the indexing set to the empty set.
   void Hangup() {HangupCode();}

  /// Adds a number to the indexing set.
   void Dial(int32 code) {DialCode(code);}

  /// Returns a reference to the item indexed by the current indexing set.
  /// Caches the calculation so multiple calls are not inefficient.
  /// If the indexing set hasn't been Picked up before a new object will be
  /// created.
   T & Pickup() {return *(T*)PickupCode(sizeof(T),temp,MakeFunc);}

  /// Returns true if an object has already been created for the current
  /// indexing set, i.e. Pickup has been called for it; false otherwise.
   bit Exists() const {return ExistsCode(temp);}

  /// Deletes the item associated with the current indexing set. You could then
  /// call Pickup to get a new object associated with the same indexing set, or
  /// hangup to go off and do something else.
   void Del() {DelCode(temp,DelFunc);}


  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::ds::Dialler<" << typestring<T>() << "," << typestring<MT>() << "," << typestring<DT>() << ">");
    return ret;
   }


 private:
  // Suport buffer - tempory storage for 'new' operation.
   Node * temp; // New nodes are constructed in here - has correct size.

  // Not modified like below... 
   static void MakeFunc(void * ptr)
   {
    MT::Make((T*)ptr);
   } 
 
  // Modified to suport the fact the pointers are nodes with appended T's rather than straight T's...
   static void DelFunc(void * ptr)
   {
    Node * p = (Node*)ptr;
    mem::Free(p->index);
    DT::Kill((T*)p->End());
   }
};

//------------------------------------------------------------------------------
 };
};
#endif
