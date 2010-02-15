#ifndef EOS_DS_ARRAYS_RESIZE_H
#define EOS_DS_ARRAYS_RESIZE_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file arrays_resize.h
/// Provides an array that is meant to be resized regularly, uses blocks of 
/// memory and resizes the block index instead of the data.

#include "eos/types.h"
#include "eos/typestring.h"

#include "eos/mem/alloc.h"
#include "eos/mem/functions.h"
#include "eos/mem/safety.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
class EOS_CLASS ArrayResizeCode
{
 protected:
   ArrayResizeCode(nat32 elementSize,nat32 indexBlocking,nat32 elementBlocking,
                   nat32 elements,void (*Make)(void * ptr));
  ~ArrayResizeCode();
  
  void Del(nat32 elementSize,void (*Term)(void * ptr)); // Calls given function on all elements.
  
  nat32 Resize(nat32 elementSize,nat32 newSize,void (*Make)(void * ptr),void (*Term)(void * ptr));
  
  byte * Item(nat32 elementSize,nat32 i) const;
  
  
  nat32 indexBlocking; // How much to increase the block index each time it overflows. The index never shrinks.
  nat32 elementBlocking; // Number of items to keep in each block.
  
  nat32 indexSize; // Size of index. indexSize%indexBlocking must allways ==0.
  nat32 size; // Number of elements contained.
  
  byte ** index; // Block of memory containing pointers to the actual blocks of data.
};

//------------------------------------------------------------------------------
/// Provides an array class that is fast to resize, i.e. it uses blocks of memory
/// and resizes an index rather than the blocks. The index only ever grows in
/// size, though the minimum number of blocks are allocated at any given time.
/// Has the extra advantage that resizing the array does not change the memory 
/// location of data items within the array, so you can keep pointers to them.
template <typename T, typename MT = mem::MakeDont<T>,typename DT = mem::KillNull<T> >
class EOS_CLASS ArrayResize : public ArrayResizeCode
{
 public:
  /// \param sz The size of the new array.
  /// \param blockSize How many array items to put into a single memory block.
  /// \param indexBlockSize how many new block pointers to create each time overflow happens.
   ArrayResize(nat32 sz = 0,nat32 blockSize = 8,nat32 indexBlockSize = 8)
   :ArrayResizeCode(sizeof(T),indexBlockSize,blockSize,sz,MakeFunc) {}
   
  /// &nbsp;
   ~ArrayResize() {Del(sizeof(T),DelFunc);}


  /// Returns the size of the array.
   nat32 Size() const {return size;}
   
  /// Sets the size of the array, returning the new size.
   nat32 Size(nat32 ns) {return Resize(sizeof(T),ns,MakeFunc,DelFunc);}
   
  /// Returns how many bytes of data the structure is using, excluding data at 
  /// the other ends of pointers stored in the contained objects.
   nat32 Memory() const 
   {
    nat32 usedIndex = (size==0)?0:((size-1)/elementBlocking + 1);
    return sizeof(ArrayResize) + sizeof(byte*)*indexSize + usedIndex*elementBlocking*sizeof(T);
   }


  /// &nbsp;
   T & operator[] (nat32 i) {return *(T*)Item(sizeof(T),i);}

  /// &nbsp;
   const T & operator[] (nat32 i) const {return *(T*)Item(sizeof(T),i);}

  /// Returns a pointer to an item in the object, the pointer will not change 
  /// through the lifetime of the object being pointed to.
   T * Ptr(nat32 i) {return (T*)Item(sizeof(T),i);}
   
  /// &nbsp;
   const T * Ptr(nat32 i) const {return (T*)Item(sizeof(T),i);}


  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::ds::ArrayResize<" << typestring<T>() 
                       << "," << typestring<MT>() << "," << typestring<DT>() << ">");
    return ret;
   }  

  
 private:
  static void MakeFunc(void * ptr)
  {
   MT::Make((T*)ptr);
  }
   
  static void DelFunc(void * ptr)
  {
   DT::Kill((T*)ptr);
  }   
};

//------------------------------------------------------------------------------
/// This is an array resize class that automatically uses mem::MakeNew and 
/// mem::KillOnlyDel on its subject, this makes it safe to use this version with
/// objects that need to be both constructed and destructed. Requires a 
/// parameterless constructor off course.
template <typename T>
class EOS_CLASS ArrayResizeDel : public ArrayResize<T,mem::MakeNew<T>,mem::KillOnlyDel<T> >
{
 public:
  /// &nbsp;
   ArrayResizeDel(nat32 sz = 0,nat32 blockSize = 8,nat32 indexBlockSize = 8)
   :ArrayResize<T,mem::MakeNew<T>,mem::KillOnlyDel<T> >(sz,blockSize,indexBlockSize) {}

  /// &nbsp;
   ~ArrayResizeDel() {}
};

//------------------------------------------------------------------------------
 };
};
#endif
