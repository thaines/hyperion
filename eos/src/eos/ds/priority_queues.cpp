//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines
#include "eos/ds/priority_queues.h"

#include "eos/mem/alloc.h"
#include "eos/mem/functions.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
PriorityQueueCode::PriorityQueueCode(nat32 elementSize,nat32 startSize)
:size(startSize),elements(0)
{
 data = mem::Malloc<byte>(elementSize * size);
}

PriorityQueueCode::PriorityQueueCode(nat32 elementSize,const PriorityQueueCode & rhs)
:size(rhs.elements),elements(rhs.elements)
{
 data = mem::Malloc<byte>(elementSize * size);
 mem::Copy(data,rhs.data,elementSize * elements);
}

PriorityQueueCode::~PriorityQueueCode()
{
 mem::Free(data);
}

void PriorityQueueCode::Del(nat32 elementSize,void (*Term)(void * ptr))
{
 byte * targ = data;
 for (nat32 i=0;i<elements;i++)
 {
  Term(targ);
  targ += elementSize;       
 }
}

void PriorityQueueCode::Copy(nat32 elementSize,const PriorityQueueCode & rhs)
{
 if (rhs.elements>size)
 {
  mem::Free(data);
  data = mem::Malloc<byte>(elementSize * rhs.elements);
  size = rhs.elements;
 }

 elements = rhs.elements;
 mem::Copy(data,rhs.data,elementSize * elements);
}

void PriorityQueueCode::Add(nat32 elementSize,void * in,bit (*LessThan)(void * lhs,void * rhs))
{
 // On overflow, get larger...
  if (elements==size)
  {
   size += growthFactor;
   byte * newData = mem::Malloc<byte>(size * elementSize);
   mem::Copy(newData,data,elementSize * elements);
   mem::Free(data);
   data = newData;
  }

 // Now increase the size of the array and do the floating upwards that
 // will be required...
  nat32 target = elements;
  elements++;

  while (target!=0)
  {
   nat32 parent = (target-1)>>1;
   if (LessThan(in,data + elementSize*parent)) 
   {
    mem::Copy(data + elementSize*target,data + elementSize*parent,elementSize);
    target = parent;
   } else break;
  }

  mem::Copy(data + elementSize*target,(byte*)in,elementSize);
}

void PriorityQueueCode::Rem(nat32 elementSize,bit (*LessThan)(void * lhs,void * rhs))
{
 --elements;
 byte * temp = data + elementSize*elements;

 nat32 target = 0;
 while (1)
 {
  nat32 child = (target<<1)+1;
  if (child>=elements) break;

  byte * pos = data + elementSize*child;
  if ((child+1)!=elements)
  {
   if (LessThan(pos+elementSize,pos))
   {
    pos += elementSize;
    ++child;
   }
  }

  if (LessThan(temp,pos)) break;
  
  mem::Copy(data + elementSize*target,pos,elementSize);
  target = child;
 }
 
 byte * targ = data + elementSize*target;
 if (targ!=temp) mem::Copy(targ,temp,elementSize);
}

//------------------------------------------------------------------------------
 };
};
