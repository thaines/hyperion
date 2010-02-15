#ifndef EOS_DS_WINDOWS_H
#define EOS_DS_WINDOWS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file windows.h
/// A circular buffer implimentation - drops data when it runs out of space.
/// Stack like interface, array like access to past entrys.
/// Implimented as an extension of the array data structure.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/ds/arrays.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
/// The window class is an array variant, that acts as a circular buffer with a
/// stack-style interface. it continues to allow array indexing, but from the 
/// start of the circular buffer. It additionally has a seperate window size 
/// and array size, where array size is the number of valid entrys and can be 
/// less than the total number of entrys that can actually be contained.
template <typename T, typename DT = mem::KillNull<T> >
class EOS_CLASS Window : public ArrayCode
{
 public:
  /// &nbsp;
   Window(nat32 sz):ArrayCode(sizeof(T),sz),offset(sz-1),size(0) {}

  /// &nbsp;
   ~Window()
   {for (nat32 i=0;i<size;i++) DT::Kill(&(*this)[i]);}


  /// Makes the data structure empty...
   void MakeEmpty()
   {
    while (size!=0) Pop();
   }


  /// Returns the size of the window, the maximum number of items it can contain.
   nat32 MaxSize() const {return elements;}

  /// Returns the number of items currently within the window.
   nat32 Size() const {return size;}
   
  /// Returns how many bytes of data the structure is using, excluding data at 
  /// the other ends of pointers stored in the contained objects.
   nat32 Memory() const {return elements*sizeof(T) + sizeof(ArrayCode);}


  /// &nbsp;
   T & operator[] (nat32 i) const {return *(T*)Item(sizeof(T),(i+offset)%elements);}


  /// Adds an item to the 'stack', will lose items if its allready full.
   void Push(const T & in)
   {
    offset = (offset+elements-1)%elements;   
    if (size==elements)
    {
     DT::Kill(&(*this)[offset]);
     mem::Copy((T*)Item(sizeof(T),offset),&in);
    }
    else
    {
     mem::Copy((T*)Item(sizeof(T),offset),&in);
     ++size;
    }
   }

  /// Equivalent to (*this)[0], just provided to finish the pattoffset = (offset+elements-1)%elements;ern.
   T & Peek() const {return *(T*)Item(sizeof(T),offset);}

  /// Removes an item from the stack, must not be called if Size()==0.
   void Pop()
   {
    DT::Kill(&(*this)[offset]);
    offset = (offset+1)%elements;
    --size;
   }


  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::ds::Window<" << typestring<T>() << "," << typestring<DT>() << ">");
    return ret;
   }


 private:
  nat32 offset; // Offset from start of array to zeroth entry.
  nat32 size; // Number of entrys that are actually filled in.
};

//------------------------------------------------------------------------------
 };
};
#endif
