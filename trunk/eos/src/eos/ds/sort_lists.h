#ifndef EOS_DS_SORT_LISTS_H
#define EOS_DS_SORT_LISTS_H
//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines

/// \file sort_lists.h
/// Contains a list of automatically sorted elements, with no duplicates allowed,
/// also provides reasonably fast indexing making this a good general purpose
/// structure. Suitable for implimenting sets. Also makes a perfectly good
/// substitute for a hash table.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/mem/safety.h"
#include "eos/ds/sorting.h"
#include "eos/ds/iteration.h"
#include "eos/ds/stacks.h"


namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
class EOS_CLASS SortListCode
{
 protected:
   SortListCode():top(null<Node*>()) {}
   SortListCode(nat32 elementSize,const SortListCode & rhs);
  ~SortListCode() {}

  void Del(void (*Term)(void * ptr));

  void Copy(nat32 elementSize,const SortListCode & rhs); // Call Del before calling this.

  void Add(nat32 elementSize,byte * data,bit (*LessThan)(void * lhs,void * rhs),void (*Term)(void * ptr));
  void * Find(byte * dummy,bit (*LessThan)(void * lhs,void * rhs)) const; // Null on not found.
  void * Largest(byte * dummy,bit (*LessThan)(void * lhs,void * rhs)) const; // Returns the largest element thats smaller than the dummy but not equal to it, null if they are all larger or equal.
  void Rem(byte * data,bit (*LessThan)(void * lhs,void * rhs),void (*Term)(void * ptr));
  void Iter(void (*Func)(void * item,void * ptr),void * ptr) const; // Calls Func on all nodes.
  void * Get(nat32 i) const; // Returns one by index, O(log size)
  nat32 FindIndex(byte * dummy,bit (*LessThan)(void * lhs,void * rhs)) const; // nat32(-1) on not found.

  void * First() const;
  void * Last() const;

  bit Invariant() const; // Check the avl tree satisfies its invariant.


  // Internal node class, this is allways created with elementSize bytes appended, to store the data.
   class EOS_CLASS Node
   {
    public:
     static Node * MakeNode(nat32 elementSize,void * data); // A constructor, essentially.
     static Node * MakeNode(nat32 elementSize,const Node & copy); // A recursive function used to Copy sets of nodes, essentially a constructor.
     void Del(void (*Term)(void * ptr)); // Deletes all children and then comits suicide.
     int32 Height() const; // Can be saftly called with this==null.
     nat32 Count() const; // Includes node it is called on, safe to call when this==null.

     // The below methods all do the rotations, returning the new this pointer for this
     // location in the tree...
      Node * RotLeft();
      Node * RotRight();
      Node * RotDoubleLeft();
      Node * RotDoubleRight();

     // Does an insertion returning the new this. If it has to replace data it calls
     // Term on it first...
      Node * Insert(nat32 elementSize,byte * data,bit (*LessThan)(void * lhs,void * rhs),void (*Term)(void * ptr));

     // Removes a node. Calls Term on the data before its is blown away.
      Node * Remove(byte * data,bit (*LessThan)(void * lhs,void * rhs),void (*Term)(void * ptr));


     // Iterates every item...
      void Iter(void (*Func)(void * item,void * ptr),void * ptr);

     // This acts as an indexing Getter, find the node with a particular i.
     // offset should be 0 when called on the first node...
      Node * Get(nat32 i,nat32 offset);

     // Returns the data.
      void * Data() {return (byte*)this + sizeof(Node);}

     // Check the invariant of the node is correct, recursing down to sub trees.
     // Returns false if the tree has been buggered.
      bit Invariant() const;


     int32 height;
     int32 children; // How many children it has, so get can work.
     Node * parent;
     Node * left;
     Node * right;
   } * top;

  // Internal class, used for iterating the entire list...
   class EOS_CLASS CursorCode
   {
    public:
      CursorCode(Node * start):targ(start){}
     ~CursorCode() {}

     void ToStart();
     void ToEnd();

     CursorCode & operator ++ ();
     CursorCode & operator -- ();

     Node * targ; // Current target is the top of the stack.
   };


 // So the Dialler can be implimented...
  friend class DiallerCode;
};

//------------------------------------------------------------------------------
/// A SortList is internally a balanced binary tree, it provides a sorted list
/// of items where no duplicates (In terms of the given sorting functor) are
/// allowed. It provides the ability to access elements as an array too, making
/// it a very flexible data structure. Its memory usage is also a lot tighter
/// than the hash tables, with 16 bytes overhead per contained item.
template <typename T, typename SO = SortOp<T>, typename DT = mem::KillNull<T> >
class EOS_CLASS SortList : public SortListCode
{
 public:
  /// &nbsp;
   SortList() {}

  /// The copy constructor is templated on the type of DT, so any DT type can be
  /// assigned to any other. Note that assignments should only be a deleting type
  /// and a non-deleting type, as between two deleting types will result in double
  /// deletion. Which is bad.
   template <typename DTT>
   SortList(const SortList<T,SO,DTT> & rhs):SortListCode(sizeof(T),rhs) {}

  /// &nbsp;
   ~SortList() {Del(&DelFunc);}

  /// Resets the data structure to contain no data.
   void MakeEmpty() {Del(&DelFunc);}

  /// Resets the data structure to contain no data.
  /// This version does not call the object killer on its
  /// contents, making it a memory leak. It is assumed that
  /// you will have allready copied the data elsewhere and
  /// made provision for its termination.
   void MakeEmptyUnsafe() {Del(&NullDelFunc);}


  /// This works on the same design principals as the copy constructor.
  /// Warning: I am 90% sure that the code behind this has a bug, and will create a bad avl tree! Needs testing/fixing.
   template <typename DTT>
   SortList<T,SO,DT> & operator = (const SortList<T,SO,DTT> & rhs)
   {Del(&DelFunc); Copy(sizeof(T),rhs); return *this;}

  /// Identical to operator =, except it also emptys the right hand side at the
  /// same time. This makes it rather more efficient for those situations where
  /// it makes sense.
   template <typename DTT>
   void Take(SortList<T,SO,DTT> & rhs)
   {Del(&DelFunc); top = rhs.top; rhs.top = null<Node*>();}


  /// Given another list of the same type this adds to this list all items in
  /// the other list. Doing this with two lists that delete there items would be
  /// a bad idea.
   template <typename DTT>
   SortList<T,SO,DT> & operator += (const SortList<T,SO,DTT> & rhs);


  /// Returns how many items are in the list.
   nat32 Size() const {return top->Count();}

  /// Returns how much memory the list is consuming
   nat32 Memory() const {return top->Count()*(sizeof(T) + sizeof(Node)) + sizeof(*this);}


  /// Adds a new element, replacing and deleting any old element that is equal to it.
   void Add(const T & elem) {SortListCode::Add(sizeof(T),(byte*)(void*)&elem,SortFunc,DelFunc);}

  /// Returns an element on being given a dummy element for it to be equal to, returns null
  /// if no such element exists.
   T * Get(const T & dummy) const {return (T*)Find((byte*)(void*)&dummy,SortFunc);}

  /// Returns the largest element as based on the ordering that is smaller than the given dummy, or
  /// null if no elements are smaller than the dummy.
   T * Largest(const T & dummy) const {return (T*)SortListCode::Largest((byte*)(void*)&dummy,SortFunc);}

  /// Deletes an element on being given a dummy element for it to be equal to.
  /// Quite safe to be called with a reference to the data actually stored within
  /// this data structure.
   void Rem(const T & dummy) {SortListCode::Rem((byte*)(void*)&dummy,SortFunc,DelFunc);}


  /// This is given an object and a pointer to one of its methods that takes
  /// just this type, it then calls that method for the object for every single
  /// element in the data structure.
   template <typename OT, void (OT::*FUNC)(T&)>
   void Iter(OT * obj) const
   {
    SortListCode::Iter(ElemIter<OT,T,FUNC>::F,obj);
   }


  /// This allows access to the elements as an array, only request elements less than
  /// Size(). This method should only be used for the occasioanl random access, to
  /// iterate all see Cursor.
   T & operator [] (nat32 i) {return *((T*)(SortListCode::Get(i)));}

  /// &nbsp;
   const T & operator [] (nat32 i) const {return *((T*)(SortListCode::Get(i)));}

  /// Given an element this returns its offset in the list, as used when accessing
  /// the list as an array. Returns nat32(-1) if not found. If you have written
  /// the sort list to an array this can be very useful for finding the relevant
  /// offset into that array, especially if saving data such that its already on
  /// disk.
   nat32 Index(const T & dummy) const {return FindIndex((byte*)(void*)&dummy,SortFunc);}


  /// Cursor class, used to iterate the list
   class EOS_CLASS Cursor : protected CursorCode
   {
    public:
     // Undocumented, as internal use only.
      Cursor(Node * top)
      :CursorCode(top) {}

     /// &nbsp;
      Cursor(const Cursor & rhs):CursorCode(rhs.targ) {}

     /// &nbsp;
      Cursor & operator = (const Cursor & rhs) {targ = rhs.targ; return *this;}


     /// Returns true if you have walked off the start/end of the list. Once you have
     /// done this there is no recovery other than assigning a Good() Cursor to this.
      bit Bad() const {return targ==null<Node*>();}

     /// The opposite of Bad().
      bit Good() const {return targ!=null<Node*>();}


     /// Sets the cursor to the front of the list.
      void ToFront() {ToStart();}

     /// Sets the cursor to the back of the list.
      void ToBack() {ToEnd();}


     /// Moves to the next item in the list.
      Cursor & operator++ () {CursorCode::operator++(); return *this;}

     /// Moves to the previous item in the list.
      Cursor & operator-- () {CursorCode::operator--(); return *this;}


     /// &nbsp;
      T & operator* () {return *(T*)targ->Data();}

     /// &nbsp;
      const T & operator* () const {return *(T*)targ->Data();}

     /// &nbsp;
      T & Obj() {return *(T*)targ->Data();}

     /// &nbsp;
      const T & Obj() const {return *(T*)targ->Data();}

     /// &nbsp;
      T * operator->() {return (T*)targ->Data();}

     /// &nbsp;
      const T * operator->() const {return (T*)targ->Data();}

     /// &nbsp;
      T * Ptr() {return (T*)targ->Data();}

     /// &nbsp;
      const T * Ptr() const {return (T*)targ->Data();}


     /// &nbsp;
      static inline cstrconst TypeString()
      {
       static GlueStr ret(GlueStr() << "eos::ds::SortList::Cursor<" << typestring<T>() << "," << typestring<SO>() << "," << typestring<DT>() << ">");
       return ret;
      }
   };

  /// Returns a Cursor pointing to the first item in the list.
  /// You can edit the list whilst iterating it, just be aware that if you
  /// delete a node any cursor pointing to it will be screwed.
   Cursor FrontPtr() const
   {
    Cursor ret = Cursor(top);
    ret.ToFront();
    return ret;
   }

  /// Returns a Cursor pointing to the last item in the list.
  /// You can edit the list whilst iterating it, just be aware that if you
  /// delete a node any cursor pointing to it will be screwed.
   Cursor BackPtr() const
   {
    Cursor ret = Cursor(top);
    ret.ToBack();
    return ret;
   }


  /// Returns the first item in the list.
   T & First() {return *(T*)SortListCode::First();}

  /// Returns the last item in the list.
   T & Last() {return *(T*)SortListCode::Last();}


  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::ds::SortList<" << typestring<T>() << "," << typestring<SO>() << "," << typestring<DT>() << ">");
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

  static void NullDelFunc(void * ptr)
  {}
};

//------------------------------------------------------------------------------
// Method which has to be stored outside the class due to using the class for
// its implimentation...
template <typename T, typename SO, typename DT>
template <typename DTT>
SortList<T,SO,DT> & SortList<T,SO,DT>::operator += (const SortList<T,SO,DTT> & rhs)
{
 typename SortList<T,SO,DTT>::Cursor targ = rhs.FrontPtr();
 while (!targ.Bad())
 {
  Add(*targ);
  ++targ;
 }
 return *this;
}

//------------------------------------------------------------------------------
 };
};
#endif
