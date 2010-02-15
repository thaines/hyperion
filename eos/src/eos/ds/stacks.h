#ifndef EOS_DS_STACKS_H
#define EOS_DS_STACKS_H
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


/// \file stacks.h
/// Contains the stack data structure, a simple and fast first in last out (FILO)
/// structure.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/mem/safety.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
// Code for the Stack flat template...
class EOS_CLASS StackCode
{
 protected:
   StackCode():elements(0),top(null<Node*>()) {}
   StackCode(nat32 elementSize,const StackCode & rhs) {Copy(elementSize,rhs);}
  ~StackCode() {}

  void Del(void (*Term)(void * ptr)); // Call before the class dies, and before Copy, leaves class in dangerous state.

  void Copy(nat32 elementSize,const StackCode & rhs); // Does not delete the existing data, call Del first.

  void Push(nat32 elementSize,byte * data);
  byte * Peek() const {return (byte*)top + sizeof(Node);}
  void Pop();
  void PopKill(void (*Term)(void * ptr));

  
 nat32 elements;
 class Node // The data is stored after the node.
 {
  public:
   Node * next;
 } * top;
};

//------------------------------------------------------------------------------
/// The Stack class, a standard implimentation by all accounts.
template <typename T, typename DT = mem::KillNull<T> >
class EOS_CLASS Stack : public StackCode
{
 public:
  /// &nbsp;
   Stack() {}
  
  /// A copy constructor designed to take an arbitary deallocator. Remember that
  /// storing an object pointer in two objects that are going to delete it is
  /// like aimming for your foot. Really should only be used between non-deleting
  /// and deleting types.
   template <typename DTT>
   Stack(const Stack<T,DTT> & rhs):StackCode(sizeof(T),rhs) {}
  
  /// &nbsp;
   ~Stack() {Del(&DelFunc);}
  
   
  /// Same design as the copy constructor.
   template <typename DTT>
   Stack<T,DT> & operator = (const Stack<T,DTT> & rhs) {Del(&DelFunc); Copy(sizeof(T),rhs); return *this;}
 
   
  /// &nbsp;
   nat32 Size() const {return elements;}
  
  /// Like all the memory methods this only returns the data structure size including data stored in it,
  /// if the items in question point at extra data such data is not counted.
   nat32 Memory() const {return elements*(sizeof(Node) + sizeof(T)) + sizeof(StackCode);}


  /// &nbsp;
   void Push(const T & rhs) {StackCode::Push(sizeof(T),(byte*)&rhs);}
  
  /// &nbsp;
   T & Peek() const {return *(T*)StackCode::Peek();}
  
  /// Removes the next item, without calling any deallocator on it.
   void Pop() {StackCode::Pop();}
  
  /// Removes the next item, calling the deallocator on it, much to the decrement of the item in question.
   void PopKill() {StackCode::PopKill(&DelFunc);}
  
  
  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::ds::Stack<" << typestring<T>() << "," << typestring<DT>() << ">");
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
