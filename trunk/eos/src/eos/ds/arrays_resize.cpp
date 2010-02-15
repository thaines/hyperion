//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/ds/arrays_resize.h"

#include "eos/math/functions.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
ArrayResizeCode::ArrayResizeCode(nat32 elementSize,nat32 iBlock,nat32 eBlock,
                                 nat32 elements,void (*Make)(void * ptr))
:indexBlocking(iBlock),elementBlocking(eBlock),size(elements)
{
 nat32 usedIndex = (size==0)?0:((size-1)/elementBlocking + 1);
 nat32 uim = usedIndex%indexBlocking;
 if (uim==0) indexSize = math::Max(usedIndex,indexBlocking);
        else indexSize = usedIndex + indexBlocking - uim;
 
 index = new byte*[indexSize];
 for (nat32 i=0;i<usedIndex;i++) index[i] = mem::Malloc<byte>(elementSize * elementBlocking);
 for (nat32 i=usedIndex;i<indexSize;i++) index[i] = null<byte*>();
 
 for (nat32 i=0;i<size;i++) Make(Item(elementSize,i));
}

ArrayResizeCode::~ArrayResizeCode()
{
 nat32 usedIndex = (size==0)?0:((size-1)/elementBlocking + 1);
 for (nat32 i=0;i<usedIndex;i++) mem::Free(index[i]);
 delete[] index;
}

void ArrayResizeCode::Del(nat32 elementSize,void (*Term)(void * ptr))
{
 for (nat32 i=0;i<size;i++) Term(Item(elementSize,i));
}

nat32 ArrayResizeCode::Resize(nat32 elementSize,nat32 newSize,void (*Make)(void * ptr),void (*Term)(void * ptr))
{
 if (newSize==size) return size;
 
 nat32 usedIndex = (size==0)?0:((size-1)/elementBlocking + 1);
 nat32 newUsedIndex = (newSize==0)?0:((newSize-1)/elementBlocking + 1);
 
 if (newSize<size)
 {
  // Call Term...
   for (nat32 i=newSize;i<size;i++) Term(Item(elementSize,i));
   
  // Remove any excess blocks...
   for (nat32 i=newUsedIndex;i<usedIndex;i++)
   {
    mem::Free(index[i]);
    index[i] = null<byte*>(); // Strictly not necesary.
   }
 }
 else
 {
  // Increase the index size as needed...
   if (newUsedIndex>indexSize)
   {
    nat32 uim = newUsedIndex%indexBlocking;
    nat32 newIndexSize;
    if (uim==0) newIndexSize = math::Max(newUsedIndex,indexBlocking);
           else newIndexSize = newUsedIndex + indexBlocking - uim;

    if (newIndexSize>indexSize)
    {
     byte ** newIndex = new byte*[newIndexSize];
     mem::Copy(newIndex,index,sizeof(byte*)*indexSize);

     delete[] index;
     index = newIndex;
     indexSize = newIndexSize;

     // Set to null blocks that will not be touched in the new blocks creation 
     // step below. Strictly not necesary...
      for (nat32 i=newUsedIndex;i<indexSize;i++) index[i] = null<byte*>();
    }
   }

  // Create new blocks...
   for (nat32 i=usedIndex;i<newUsedIndex;i++) index[i] = mem::Malloc<byte>(elementSize * elementBlocking);

  // Call Make...
   for (nat32 i=size;i<newSize;i++) Make(Item(elementSize,i));
 }
 
 size = newSize;
 return size;
}

byte * ArrayResizeCode::Item(nat32 elementSize,nat32 i)
{
 nat32 ind = i/elementBlocking;
 nat32 off = i%elementBlocking;
 
 return index[ind] + off*elementSize;
}

//------------------------------------------------------------------------------
 };
};
