#ifndef EOS_DS_QUEUES_H
#define EOS_DS_QUEUES_H
//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines

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


/// \file queues.h
/// Contains the queue data structure, a simple and fast first in first out (FIFO)
/// structure.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/mem/safety.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
// Code for the Queue flat template...
class EOS_CLASS QueueCode
{
 protected:
   QueueCode():elements(0),top(null<Node*>()),end(&top) {}
   QueueCode(nat32 elementSize,const QueueCode & rhs) {Copy(elementSize,rhs);}
  ~QueueCode() {}

  void Del(void (*Term)(void * ptr)); // Call before the class dies, and before Copy, leaves class in dangerous state.
  void Copy(nat32 elementSize,const QueueCode & rhs); // Does not delete the existing data, call Del first.

  void Add(nat32 elementSize,byte * data);
  byte * Peek() const {return (byte*)top + sizeof(Node);}
  void Rem();
  void RemKill(void (*Term)(void * ptr));

  
 nat32 elements;
 class Node // The data is stored after the node.
 {
  public:
   Node * next;
 };
 
 Node * top;
 Node ** end; // Pointer to the pointer that needs to contain a pointer to the next item. Never null.
};

//------------------------------------------------------------------------------
/// A very standard queue. Nothing to write home about.
template <typename T, typename DT = mem::KillNull<T> >
class EOS_CLASS Queue : public QueueCode
{
 public:
  /// &nbsp;
   Queue() {}
  
  /// A copy constructor designed to take an arbitary deallocator. Remember that
  /// storing an object pointer in two objects that are going to delete it is
  /// like aimming for your foot. Really should only be used between non-deleting
  /// and deleting types.
   template <typename DTT>
   Queue(const Queue<T,DTT> & rhs):QueueCode(sizeof(T),rhs) {}
  
  /// &nbsp;
   ~Queue() {Del(&DelFunc);}
  
   
  /// Same design as the copy constructor.
   template <typename DTT>
   Queue<T,DT> & operator = (const Queue<T,DTT> & rhs) {Del(&DelFunc); Copy(sizeof(T),rhs); return *this;}
 
   
  /// &nbsp;
   nat32 Size() const {return elements;}
  
  /// Like all the memory methods this only returns the data structure size including data stored in it,
  /// if the items in question point at extra data such data is not counted.
   nat32 Memory() const {return elements*(sizeof(Node) + sizeof(T)) + sizeof(QueueCode);}


  /// &nbsp;
   void Add(const T & rhs) {QueueCode::Add(sizeof(T),(byte*)&rhs);}
  
  /// &nbsp;
   T & Peek() const {return *(T*)QueueCode::Peek();}
  
  /// Removes the next item, without calling any deallocator on it.
   void Rem() {QueueCode::Rem();}
  
  /// Removes the next item, calling the deallocator on it, much to the decrement of the item in question.
   void RemKill() {QueueCode::RemKill(&DelFunc);}
  
  
  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::ds::Queue<" << typestring<T>() << "," << typestring<DT>() << ">");
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
