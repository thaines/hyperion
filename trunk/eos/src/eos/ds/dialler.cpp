//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/ds/dialler.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
DiallerCode::DiallerCode()
:indexSize(0),index(indexBlockSize),curr(null<Node*>())
{}

DiallerCode::~DiallerCode()
{}

void DiallerCode::DelAll(void (*Term)(void * ptr))
{
 store.Del(Term);
}

void DiallerCode::HangupCode()
{
 indexSize = 0;
 curr = null<Node*>();
}

void DiallerCode::DialCode(int32 code)
{
 // Make the index array bigger if need be...
  if (indexSize==index.Size())
  {
   index.Size(index.Size()+indexBlockSize);
  }

 // Add number to array...
  index[indexSize] = code;
  indexSize += 1;
  curr = null<Node*>();
}

void * DiallerCode::PickupCode(nat32 elementSize,Node * dummy,void (*MakeFunc)(void * ptr))
{
 // Check if the answer is cached...
  if (curr) return curr->End();


 // Sort the index, remove duplicates...
  if (indexSize!=0)
  {
   index.SortRangeNorm(0,indexSize-1);
   
   nat32 j = 1;
   for (nat32 i=1;i<indexSize;i++)
   {
    if (index[i]!=index[i-1])
    {
     index[j] = index[i];
     j += 1;
    }
   }
   indexSize = j;
  }


 // Try and find an entry that already exists...
  // Create dummy...
   dummy->size = indexSize;
   dummy->index = index.Ptr();

  // Request (and return)...
   curr = (Node*)store.Find((byte*)(void*)dummy,LessThan);
   if (curr) return curr->End();


 // Entry doesn't exist, make it...
  dummy->index = mem::Malloc<int32>(dummy->size);
  for (nat32 i=0;i<indexSize;i++) dummy->index[i] = index[i];
  MakeFunc(dummy->End());

  store.Add(sizeof(Node)+elementSize,(byte*)(void*)dummy,LessThan,0);
  
  curr = (Node*)store.Find((byte*)(void*)dummy,LessThan);
  return curr;
}

bit DiallerCode::ExistsCode(Node * dummy) const
{
 // If a cache entry exists it must exist...
  if (curr) return true;


 // Sort the index, remove duplicates...
  if (indexSize!=0)
  {
   const_cast<DiallerCode*>(this)->index.SortRangeNorm(0,indexSize-1);
   
   nat32 j = 1;
   for (nat32 i=1;i<indexSize;i++)
   {
    if (index[i]!=index[i-1])
    {
     const_cast<DiallerCode*>(this)->index[j] = index[i];
     j += 1;
    }
   }
   const_cast<DiallerCode*>(this)->indexSize = j;
  }


 // Create dummy...
  dummy->size = indexSize;
  dummy->index = const_cast<DiallerCode*>(this)->index.Ptr();

 // Request and return...
  const_cast<DiallerCode*>(this)->curr = (Node*)store.Find((byte*)(void*)dummy,LessThan);
  return curr!=null<Node*>();
}

void DiallerCode::DelCode(Node * dummy,void (*Term)(void * ptr))
{
 // Sort the index, remove duplicates...
  if ((curr==null<Node*>())&&(indexSize!=0))
  {
   index.SortRangeNorm(0,indexSize-1);
   
   nat32 j = 1;
   for (nat32 i=1;i<indexSize;i++)
   {
    if (index[i]!=index[i-1])
    {
     index[j] = index[i];
     j += 1;
    }
   }
   indexSize = j;
  }

 // Create dummy...
  dummy->size = indexSize;
  dummy->index = index.Ptr();

 // Exterminate...
  store.Rem((byte*)(void*)dummy,LessThan,Term);
}

bit DiallerCode::LessThan(void * lhs,void * rhs)
{
 Node * l = (Node*)lhs;
 Node * r = (Node*)rhs;
    
 nat32 minSize = math::Min(l->size,r->size);
 for (nat32 i=0;i<minSize;i++)
 {
  if (l->index[i]==r->index[i]) continue;
  return l->index[i]<r->index[i];
 }
  
 return l->size < r->size;
}

//------------------------------------------------------------------------------
 };
};
