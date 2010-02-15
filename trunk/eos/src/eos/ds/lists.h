#ifndef EOS_DS_LISTS_H
#define EOS_DS_LISTS_H
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


/// \file lists.h
/// Contains the traditional linked list data structure.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/mem/safety.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
class EOS_CLASS ListCode
{
 public:
  ListCode():elements(0) {dummy.next = &dummy; dummy.last = &dummy;}
  ListCode(nat32 elementSize,const ListCode & rhs) {Copy(elementSize,rhs);}
  ~ListCode() {}
  
  void Del(void (*Term)(void * ptr)); // Call before the class dies, and before Copy, leaves class in dangerous state.
  void Copy(nat32 elementSize,const ListCode & rhs); // Does not delete the existing data, call Del first.

  void AddFront(nat32 elementSize,byte * data);
  void AddBack(nat32 elementSize,byte * data);
  
  void * Front() {return (byte*)dummy.next + sizeof(Node);}
  void * Back() {return (byte*)dummy.last + sizeof(Node);}
  
  void RemFront();
  void RemBack();
  void RemFrontKill(void (*Term)(void * ptr));
  void RemBackKill(void (*Term)(void * ptr));
  
  
 class EOS_CLASS Node // Forms 2 way circular linked list, data follows structures in memory.
 {
  public:  
   Node * next;
   Node * last;	 
 }; // Tail data goes after node.
 
 void Rem(Node * targ);
 void RemKill(Node * targ,void (*Term)(void * ptr));

 Node * AddBefore(nat32 elementSize,byte * data,Node * targ); // Returns the node just created.
 Node * AddAfter(nat32 elementSize,byte * data,Node * targ); // Returns the node just created.
 
 nat32 elements;
 Node dummy; // Dummy node, marks the end and start of the list. Has no tail data.
};

//------------------------------------------------------------------------------
/// A linked list implimentation, comes with an iterator as well.
template <typename T, typename DT = mem::KillNull<T> >
class EOS_CLASS List : protected ListCode
{
 public:
  /// &nbsp;
   List() {}
  
  /// A copy constructor designed to take an arbitary deallocator. Remember that
  /// storing an object pointer in two objects that are going to delete it is
  /// like aimming for your foot. Really should only be used between non-deleting
  /// and deleting types.
   template <typename DTT>
   List(const List<T,DTT> & rhs):ListCode(sizeof(T),rhs) {}
  
  /// &nbsp;
   ~List() {Del(&DelFunc);}
  
  
  /// This emptys the list, by calling RemFrontKill() until it is empty.
   void Reset() {while (Size()!=0) RemFrontKill();}
   
  /// Same design as the copy constructor.
   template <typename DTT>
   List<T,DT> & operator = (const List<T,DTT> & rhs) {Del(&DelFunc); Copy(sizeof(T),rhs); return *this;}


  /// Retursn the size of the list. When the list contains 0 items most of the methods will do bad
  /// things, so don't call them!
   nat32 Size() const {return elements;}
  
  /// Like all the memory methods this only returns the data structure size including data stored in it,
  /// if the items in question point at extra data such data is not counted.
   nat32 Memory() const {return elements*(sizeof(Node) + sizeof(T)) + sizeof(ListCode);}

  
  /// Adds an item to the front of the list. 
   void AddFront(const T & rhs) {ListCode::AddFront(sizeof(T),(byte*)&rhs);}
   
  /// Adds an item to the back of the list.
   void AddBack(const T & rhs) {ListCode::AddBack(sizeof(T),(byte*)&rhs);}
    
  /// Returns a reference to the front item in the list.
   T & Front() {return *(T*)ListCode::Front();}
  
  /// Returns a reference to the back item in the list. 
   T & Back() {return *(T*)ListCode::Back();}
  
  /// Removes the front item in the list, without calling a deallocator on it.
   void RemFront() {ListCode::RemFront();}
   
  /// Removes the back item in the list, without calling a dealocator on it.
   void RemBack() {ListCode::RemBack();}
  
  /// Removes the front item in the list, calling a deallocator on it.
   void RemFrontKill() {ListCode::RemFrontKill(&DelFunc);}
  
  /// Removes the back item in the list, calling a dealocator on it.
   void RemBackKill() {ListCode::RemBackKill(&DelFunc);}
   

  /// An iterator object, never delete the object it is pointing to as it will
  /// go screwy, other than that it should be safe.
   class EOS_CLASS Cursor
   {
    public:     
     // Undocumented, as internal use only.
      Cursor(List<T,DT> * l,Node * t):list(l),targ(t) {}
     
     /// &nbsp;
      Cursor(const Cursor & rhs):list(rhs.list),targ(rhs.targ) {}
      
     /// &nbsp;
      Cursor & operator = (const Cursor & rhs) {list = rhs.list; targ = rhs.targ; return *this;}
     
      
     /// Returns true when its a bad node, i.e. you have reached walked past the end. Do not use
     /// a Cursor that is marked as pad - the object memory returned will be bad.
      bit Bad() const {return &list->dummy==targ;}
      
     /// The opposite of Bad.
      bit Good() const {return &list->dummy!=targ;}
     
     /// Returns true if its the first item in the list.
      bit Start() const {return list->dummy.next==targ;} 
      
     /// Returns true if its the last item in the list.
      bit End() const {return list->dummy.last==targ;} 
      
     
     /// Moves to the next item in the list.
      Cursor & operator++ () {targ = targ->next; return *this;}
      
     /// Moves to the previous item in the list.
      Cursor & operator-- () {targ = targ->last; return *this;}


     /// Adds a new node before this node, and then moves the cursor to the new node.
      void AddBefore(const T & rhs) {targ = list->AddBefore(sizeof(T),(byte*)&rhs,targ);}

     /// Adds a new node after this node, and then moves the cursor to the new node.
      void AddAfter(const T & rhs) {targ = list->AddAfter(sizeof(T),(byte*)&rhs,targ);}


     /// Deletes the current item without using the deallocator and moves to the next item.
      void RemNext() {Node * n = targ->next; list->Rem(targ); targ = n;}
      
     /// Deletes the current item using the deallocator and moves to the next item.
      void RemKillNext() {Node * n = targ->next; list->RemKill(targ,&List<T,DT>::DelFunc); targ = n;}
      
     /// Deletes the current item without using the deallocator and moves to the previous item.
      void RemPrev() {Node * n = targ->last; list->Rem(targ); targ = n;}
      
     /// Deletes the current item using the deallocator and moves to the previous item.
      void RemKillPrev() {Node * n = targ->last; list->RemKill(targ,&List<T,DT>::DelFunc); targ = n;}
             
      
     /// &nbsp;
      T * Ptr() {return (T*)((byte*)targ + sizeof(Node));} 

     /// &nbsp;
      const T * Ptr() const {return (T*)((byte*)targ + sizeof(Node));} 

     /// &nbsp;
      T & operator* () {return *(T*)((byte*)targ + sizeof(Node));}

     /// &nbsp;
      const T & operator* () const {return *(T*)((byte*)targ + sizeof(Node));}
         
     /// &nbsp;
      T * operator->() {return (T*)((byte*)targ + sizeof(Node));}
  
     /// &nbsp;
      const T * operator->() const {return (T*)((byte*)targ + sizeof(Node));}


     /// &nbsp;
      static inline cstrconst TypeString()
      {
       static GlueStr ret(GlueStr() << "eos::ds::List::Cursor<" << typestring<T>() << "," << typestring<DT>() << ">");
       return ret;
      }     


    private:
     List<T,DT> * list; // For the dummy node, so it can identify ends.
     Node * targ;	   
   };
   
  /// Returns a Cursor pointing to the front of the list.
   Cursor FrontPtr() const {return Cursor(const_cast<List<T,DT>*>(this),dummy.next);}
      
  /// Returns a Cursor pointing to the back of the list.
   Cursor BackPtr() const {return Cursor(const_cast<List<T,DT>*>(this),dummy.last);}
   
   
  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::ds::List<" << typestring<T>() << "," << typestring<DT>() << ">");
    return ret;
   }


 private:
  static void DelFunc(void * ptr)
  {
   DT::Kill((T*)ptr);
  }
};

//------------------------------------------------------------------------------
/// This is a list class that automatically uses mem::KillOnlyDel on its subject,
/// this makes it safe to use this version with that need to die with style.
/// Requires a parameterless constructor off course.
template <typename T>
class EOS_CLASS ListDel : public List<T,mem::KillOnlyDel<T> >
{
 public:
  /// &nbsp;
   ListDel(nat32 sz = 0):List<T,mem::KillOnlyDel<T> >(sz) {}

  /// &nbsp;
   ~ListDel() {}
};

//------------------------------------------------------------------------------
 };
};
#endif
