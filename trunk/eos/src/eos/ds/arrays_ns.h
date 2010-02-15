#ifndef EOS_DS_ARRAYS_NS_H
#define EOS_DS_ARRAYS_NS_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file arrays_ns.h
/// Provides a simple array class, that never shrinks its memeory block, only 
/// grows it. Provides limited functionality compared to the full array class,
/// as this growth only feature is obviously for speed and some of the standard
/// array classes features would slow it down.

#include "eos/types.h"
#include "eos/typestring.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
// Code for ArrayNS flat template...
class EOS_CLASS ArrayNSCode
{
 protected:
  ArrayNSCode(nat32 elementSize,nat32 e)
  :elements(e),dataSize(e),data(mem::Malloc<byte>(elementSize*e))
  {}
 
  ArrayNSCode(nat32 elementSize,const ArrayNSCode & rhs);
  
  ~ArrayNSCode() {mem::Free(data);}
  
  
  void Copy(nat32 elementSize,const ArrayNSCode & rhs);
  nat32 Resize(nat32 elementSize,nat32 newSize);
  
  byte * Item(nat32 elementSize,nat32 i) const {return data + elementSize*i;}
 
 
 nat32 elements; 
 nat32 dataSize; // Number of elements data is actually allocated for, often larger than elements.
 byte * data;
};

//------------------------------------------------------------------------------
template <typename T>
class EOS_CLASS ArrayNS : public ArrayNSCode
{
 public:
  /// &nbsp;
  /// \param sz Size of the array.
   ArrayNS(nat32 sz=0)
   :ArrayNSCode(sizeof(T),sz)
   {}
   
  /// &nbsp;
   ArrayNS(const ArrayNS<T> & rhs)
   :ArrayNSCode(sizeof(T),rhs)
   {}
   
  /// &nbsp;
   ~ArrayNS()
   {}
   
   
  /// &nbsp;
   ArrayNS<T> & operator = (const ArrayNS<T> & rhs) {Copy(sizeof(T),rhs);return *this;}   


  /// &nbsp;
   nat32 Size() const {return elements;}
   
  /// Changes the size of the array, returning the new size.
   nat32 Size(nat32 size) {return Resize(sizeof(T),size);}
   
  /// Returns how many bytes of data the structure is using, excluding data at 
  /// the other ends of pointers stored in the contained objects.
   nat32 Memory() const {return dataSize*sizeof(T) + sizeof(ArrayNSCode);}


  /// &nbsp;
   T & operator[] (nat32 i) const {return *(T*)Item(sizeof(T),i);}
   
   
  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::ds::ArrayNS<" << typestring<T>() << ">");
    return ret;
   }
};

//------------------------------------------------------------------------------
 };
};
#endif
