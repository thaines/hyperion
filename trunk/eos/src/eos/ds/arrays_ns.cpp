//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/ds/arrays_ns.h"

#include "eos/mem/functions.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
ArrayNSCode::ArrayNSCode(nat32 elementSize,const ArrayNSCode & rhs)
:elements(rhs.elements),dataSize(rhs.elements),data(mem::Malloc<byte>(elementSize*rhs.elements))
{
 mem::Copy(data,rhs.data,elementSize*rhs.elements);
}

void ArrayNSCode::Copy(nat32 elementSize,const ArrayNSCode & rhs)
{
 elements = rhs.elements;
 if (elements>dataSize)
 {
  mem::Free(data);
  data = mem::Malloc<byte>(elementSize*elements);
 }

 mem::Copy(data,rhs.data,elementSize*elements);
}

nat32 ArrayNSCode::Resize(nat32 elementSize,nat32 newSize)
{
 if (newSize>dataSize)
 {
  byte * newData = mem::Malloc<byte>(elementSize*newSize);
  mem::Copy(newData,data,elementSize*elements);
 
  mem::Free(data);
  data = newData;
  
  dataSize = newSize;  
 }

 elements = newSize;
 return newSize;
}

//------------------------------------------------------------------------------
 };
};
