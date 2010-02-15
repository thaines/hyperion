#ifndef EOS_MEM_SAFETY_H
#define EOS_MEM_SAFETY_H
//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines

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


/// \file safety.h
/// Provides classes to manage memory such that problems won't occur. This
/// covers stack-terminated memory blocks and smart pointers.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/mem/alloc.h"

namespace eos
{
 namespace mem
 {
//------------------------------------------------------------------------------
/// An empty class, simply provided to unify the children classes, and to make
/// referencing this set of classes easier when documenting the system. (!)
/// All Make derivative classes impliment a method Make that takes a
/// pointer to the data to be initialised.
class EOS_CLASS Make
{};

/// This does nothing, simply leaves the data random.
template <typename T>
class EOS_CLASS MakeDont : public Make
{
 public:
  /// &nbsp;
   static void Make(T * t) {}

  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::mem::MakeDont<" << typestring<T>() << ">");
    return ret;
   }
};

/// This nulls the data, filling it with zeros.
template <typename T>
class EOS_CLASS MakeNull : public Make
{
 public:
  /// &nbsp;
   static void Make(T * t)
   {
    Null(t);
   }

  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::mem::MakeNull<" << typestring<T>() << ">");
    return ret;
   }
};

/// This uses the new operator with no parameters to create the object,
/// obviously requires the existance of such a constructor.
/// (This is of course a new that uses the memory in the array. Must be
/// matched with the KillOnlyDel to make sense.)
template <typename T>
class EOS_CLASS MakeNew : public Make
{
 public:
  /// &nbsp;
   static void Make(T * t)
   {
    new(t) T();
   }

  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::mem::MakeNew<" << typestring<T>() << ">");
    return ret;
   }
};

/// This uses the new operator with no parameters to create the object,
/// obviously requires the existance of such a constructor. It then calls
/// ->Acquire() on the newly created object.
/// (This is of course a new that uses the memory in the array. Must be
/// matched with the KillOnlyDel to make sense.)
template <typename T>
class EOS_CLASS MakeNewAcquire : public Make
{
 public:
  /// &nbsp;
   static void Make(T * t)
   {
    new(t) T();
    t->Acquire();
   }

  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::mem::MakeNewAcquire<" << typestring<T>() << ">");
    return ret;
   }
};

//------------------------------------------------------------------------------
/// An empty class, simply provided to unify the children classes, and to make
/// referencing this set of classes easier when documenting the system. (!)
/// All Kill derivative classes impliment a method Kill that takes a
/// pointer to the data to be killed.
class EOS_CLASS Kill
{};

/// This is a deallocation functor. Handed to classes that need to terminate objects,
/// this does nothing, for when nothing is required! (I am yet to think of a use for
/// this, however symetry of design is important. I think.)
template <typename T>
class EOS_CLASS KillNull : public Kill
{
 public:
  /// &nbsp;
   static void Kill(T *) {}

  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::mem::KillNull<" << typestring<T>() << ">");
    return ret;
   }
};

/// This is a deallocation functor. Handed to classes that need to terminate objects,
/// this one calls the ->Release() method on the pointer given for reference counting
/// object types.
template <typename T>
class EOS_CLASS KillRelease : public Kill
{
 public:
  /// &nbsp;
   static void Kill(T ** t) {(*t)->Release();}

  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::mem::KillRelease<" << typestring<T>() << ">");
    return ret;
   }
};

/// This is a deallocation functor. Handed to classes that need to terminate objects,
/// this one deletes via the normal 'delete' operator.
template <typename T>
class EOS_CLASS KillDel : public Kill
{
 public:
  /// &nbsp;
   static void Kill(T ** t) {delete *t;}

  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::mem::KillDel<" << typestring<T>() << ">");
    return ret;
   }
};

/// This is a deallocation functor. Handed to classes that need to terminate objects,
/// this one calls delete on the given object without actually deallocating the memory.
/// Specifically very useful for the array object types.
template <typename T>
class EOS_CLASS KillOnlyDel : public Kill
{
 public:
  /// &nbsp;
   static void Kill(T * t) {t->~T();}

  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::mem::KillDel<" << typestring<T>() << ">");
    return ret;
   }
};

/// This is a deallocation functor. Handed to classes that need to terminate objects,
/// this one deletes via the 'delete[]' operator.
template <typename T>
class EOS_CLASS KillDelArray : public Kill
{
 public:
  /// &nbsp;
   static void Kill(T ** t) {delete[] *t;}

  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::mem::KillDelArray<" << typestring<T>() << ">");
    return ret;
   }
};

/// This is a deallocation functor. Handed to classes that need to terminate objects,
/// this one deletes via the 'eos::mem::Free()' function.
template <typename T>
class EOS_CLASS KillFree : public Kill
{
 public:
  /// &nbsp;
   static void Kill(T ** t) {Free(*t);}

  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::mem::KillFree<" << typestring<T>() << ">");
    return ret;
   }
};

//------------------------------------------------------------------------------
/// This is the simplest smart pointer class I can think of. It simply pretends
/// its a pointer and deletes its contents when it goes out of scope. The DT
/// template parameter specifies the method of deletion, see the MemKill* functors
/// for other options here.
template <typename T,typename DT = KillDel<T> >
class EOS_CLASS StackPtr
{
 public:
  /// &nbsp;
   StackPtr(T * p = null<T*>()):ptr(p) {}

  /// &nbsp;
   ~StackPtr() {DT::Kill(&ptr);}

  /// &nbsp;
   T * Ptr() {return ptr;}
   
  /// &nbsp;
   bit IsNull() {return ptr==null<T*>();}

  /// &nbsp;
   T & operator* () {return *ptr;}

  /// &nbsp;
   const T & operator* () const {return *ptr;}

  /// &nbsp;
   T * operator->() {return ptr;}

  /// &nbsp;
   const T * operator->() const {return ptr;}

  /// &nbsp;
   T & operator[](int32 i) {return ptr[i];}

  /// &nbsp;
   const T & operator[](int32 i) const {return ptr[i];}
   
  /// &nbsp;
   T * operator + (int32 val) {return ptr + val;}

  /// &nbsp;
   const T * operator + (int32 val) const {return ptr + val;}


  /// This deletes the objects current contents and assignes the given pointer.
   StackPtr<T,DT> & operator= (T * rhs) {DT::Kill(&ptr); ptr = rhs; return *this;}

  /// The assignment operator is not like a normal assignment operator, in that
  /// it edits the class it is assigning from. This is because it the nature of
  /// its deletion means any given chunk of memory can only be held in one
  /// instance of this class, so on assignment the memory block is moved across.
  /// This means that any memory currently stored by this is deleted and rhs
  /// becomes null.
   StackPtr<T,DT> & operator= (StackPtr<T,DT> & rhs) {DT::Kill(&ptr); ptr = rhs.ptr; rhs.ptr = null<T*>(); return *this;}

  /// This swaps the memory pointed to with the passed object, so they both point
  /// to what the other object pointed to before the call.
   void Swap(StackPtr<T,DT> & rhs) {T * temp = ptr; ptr = rhs.ptr; rhs.ptr = temp;}

  /// This extracts a StackPtr and returns it as a normal one, setting the
  /// StackPtr to null.
   T * Extract() {T * ret = ptr; ptr = null<T*>(); return ret;}


 private:
  T * ptr;
};

//------------------------------------------------------------------------------
/// A smart pointer class, implimented via reference linking. This acts exactly
/// like any pointer, except it deletes its contained data when the number of
/// pointers drops to zero. Note that any object pointed to by one of these
/// should only be pointed to by these, you can't mix and match. (The exception
/// to this is with normal pointers being used where one of these exists, and
/// the normal pointers are known to go out of scope before or at the same time
/// as one of these.) Note that to enforce this requirement a SmartPtr can only
/// be constructed from a normal pointer, it can not be assigned from one.
/// It uses the MemKill* functors as the second template parameter to indicate
/// how to free memory, defaults to operator delete.
template <typename T,typename DT = KillDel<T> >
class EOS_CLASS SmartPtr
{
 public:
  /// &nbsp;
   SmartPtr(T * p = null<T>())
   :ptr(p),next(this),last(this)
   {}

  /// &nbsp;
   SmartPtr(const SmartPtr<T,DT> & rhs)
   :ptr(rhs.ptr),next(&rhs),last(&rhs.last)
   {next->last = this; last->next = this;}

  /// &nbsp;
   ~SmartPtr()
   {Release();}


  /// &nbsp;
   T & operator* () {return *ptr;}

  /// &nbsp;
   const T & operator* () const {return *ptr;}

  /// &nbsp;
   T * operator->() {return ptr;}

  /// &nbsp;
   const T * operator->() const {return ptr;}


  /// &nbsp;
   SmartPtr<T,DT> & operator = (const SmartPtr<T,DT> & rhs)
   {
    Release();
    ptr = rhs.ptr; next = &rhs; last = &rhs.last;
    next->last = this; last->next = this;
   }

  /// This sets the pointer to null<T>(), releasing the memory if relevent.
   void Release()
   {
    if (next==this) DT(ptr);
    else
    {
     next->last = last; last->next = next;
     ptr = null<T>(); next = this; last = this;
    }
   }

  /// Swaps what it points to with another SmartPtr.
   void Swap(SmartPtr<T,DT> & rhs)
   {
    T * tp = ptr; ptr = rhs.ptr; rhs.ptr = tp;
    SmartPtr<T,DT> * tl;
    tl = next; next = rhs.next; rhs.next = tl;
    tl = last; last = rhs.last; rhs.last = tl;
   }

 private:
  T * ptr;
  SmartPtr<T,DT> * next; // A circular linked list of all pointers to the object in question.
  SmartPtr<T,DT> * last; // Reference counting is probably better in some senses, but this has a certain elegance.
};

//------------------------------------------------------------------------------
 };
};
#endif
