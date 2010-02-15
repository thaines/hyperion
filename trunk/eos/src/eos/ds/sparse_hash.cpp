//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines
#include "eos/ds/sparse_hash.h"

#include "eos/mem/alloc.h"
#include "eos/mem/functions.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
SparseHashCode::SparseHashCode(nat32 elementSize,nat32 startSize)
{
 Reset(elementSize,startSize);
}

SparseHashCode::SparseHashCode(nat32 elementSize,const SparseHashCode & rhs)
:size(0),elements(0),nodes(0),data(null<Node*>())
{
 Copy(elementSize,rhs);
}

SparseHashCode::~SparseHashCode()
{}

void SparseHashCode::Del(nat32 elementSize,void (*Term)(void * ptr))
{
 Node * targ = data;
 for (nat32 i=0;i<size;i++)
 {
  Node * t = targ->next;
  while (t)
  {
   if (t->valid) Term(((byte*)t) + sizeof(Node));
   Node * tt = t->next;
   mem::Free(t);
   t = tt;
  }
  
  if (targ->valid) Term(((byte*)targ) + sizeof(Node));
  
  ((byte*&)targ) += sizeof(Node) + elementSize;
 }
 
 mem::Free(data);
}

void SparseHashCode::Reset(nat32 elementSize,nat32 startSize)
{
 size = startSize;
 elements = 0;
 nodes = 0;
 data = (Node*)mem::Malloc<byte>((sizeof(Node) + elementSize) * size);

 Node * targ = data;
 for (nat32 i=0;i<size;i++)
 {
  targ->next = null<Node*>();
  targ->valid = false;
  #ifdef EOS_DEBUG
   targ->index = 0xFFFFFFFF;
  #endif
  ((byte*&)targ) += sizeof(Node) + elementSize;
 }
}

void SparseHashCode::Copy(nat32 elementSize,const SparseHashCode & rhs)
{
 size = rhs.size;
 elements = rhs.elements;
 nodes = rhs.nodes;
 data = (Node*)mem::Malloc<byte>((sizeof(Node) + elementSize) * size);

 mem::Copy<byte>((byte*)data,(byte*)rhs.data,(sizeof(Node) + elementSize) * size);

 Node * targ = data;
 for (nat32 i=0;i<size;i++)
 {
  Node *& t = targ->next;
  while (t)
  {
   Node * n = (Node*)mem::Malloc<byte>(sizeof(Node) + elementSize);
   mem::Copy<byte>((byte*)n,(byte*)t,sizeof(Node) + elementSize);
   t = t->next;
  }

  ((byte*&)targ) += sizeof(Node) + elementSize;
 }
}

void SparseHashCode::Set(nat32 elementSize,nat32 index,void * in)
{
 // First check if the last operation made the hash too fat, if so we apply
 // lip-o-blow-tion...
  if (size<=elements) Inflate(elementSize);

 // Now find where we should be putting the data, and check if its allready
 // there, and if so replace it. Otherwise we add it into any gaps or append it
 // to the chain...
  Node * targ = data;
  ((byte*&)targ) += (sizeof(Node) + elementSize) * (index%size);

  // First we handle the scenario of it allready existing in the chain...
   Node * t = targ;
   while (t)
   {
    #ifdef EOS_DEBUG
    if ((t->valid==true)&&(t->index==index)) // To make valgrind shut up.
    #else
    if ((t->index==index)&&(t->valid==true)) // t->index is not neccesarilly set, but it dosn't matter as valid will be. We test it first as its more likely to fail the test.
    #endif
    {
     mem::Copy((byte*)t + sizeof(Node),(byte*)in,elementSize);
     return;
    }
    t = t->next;
   }

  // If we get this far it dosn't exist and we need to add it in - we
  // loop down the chain looking for a gap, if one isn't found we append it
  // to the chain...
   t = targ;
   while (true)
   {
    if (t->valid==false)
    {
     t->index = index;
     t->valid = true;
     mem::Copy((byte*)t + sizeof(Node),(byte*)in,elementSize);
     elements++;
     return;
    }
    if (t->next) t = t->next;
            else break;
   }

  // We're at the end - add to the chain...
   t->next = (Node*)mem::Malloc<byte>(sizeof(Node) + elementSize); ++nodes;
   t = t->next;
   t->next = null<Node*>();
   t->index = index;
   t->valid = true;
   mem::Copy((byte*)t + sizeof(Node),(byte*)in,elementSize);
   elements++;
}

void * SparseHashCode::Get(nat32 elementSize,nat32 index) const
{
 Node * targ = data;
 ((byte*&)targ) += (sizeof(Node) + elementSize) * (index%size);

 while (targ)
 {
  if ((targ->index==index)&&(targ->valid==true)) return (byte*)targ + sizeof(Node);
  targ = targ->next;
 }
 return null<void*>();
}

void * SparseHashCode::GetCreate(nat32 elementSize,nat32 index)
{
 Node * targ = data;
 ((byte*&)targ) += (sizeof(Node) + elementSize) * (index%size);

 Node * t = targ;
 while (t)
 {
  if ((t->index==index)&&(t->valid==true)) return (byte*)t + sizeof(Node);
  t = t->next;
 }

 // It dosn't exist - we need to create it...
  // First, we might need to re-hash...
   if (size<=elements) Inflate(elementSize);

  // Now find a free slot if it exists...
   t = targ;
   while (1)
   {
    if (t->valid==false)
    {
     t->valid = true;
     t->index = index;
     elements++;
     return (byte*)t + sizeof(Node);
    }
    if (t->next) t = t->next;
            else break;
   }

  // Otherwise we need to add a node...
   t->next = (Node*)mem::Malloc<byte>(sizeof(Node) + elementSize); ++nodes;
   t = t->next;
   t->next = null<Node*>();
   t->index = index;
   t->valid = true;
   elements++;
   return (byte*)t + sizeof(Node);
}

void SparseHashCode::Unset(nat32 elementSize,nat32 index,void (*Term)(void * ptr))
{
 Node * targ = data;
 ((byte*&)targ) += (sizeof(Node) + elementSize) * (index%size);

 while (targ)
 {
  if (targ->index==index)
  {
   if (targ->valid)
   {
    elements--;
    targ->valid = false;
    Term((byte*)targ + sizeof(Node));
   }
   break;
  }
  targ = targ->next;
 }
}

void SparseHashCode::Inflate(nat32 elementSize)
{
 // First prep the data, including the new hash table without any data in...
  nat32 oldSize = size;
  Node * oldData = data;
  nodes = 0;

  size = (size*2)+1;
  data = (Node*)mem::Malloc<byte>((sizeof(Node) + elementSize) * size);

  Node * targ = data;
  for (nat32 i=0;i<size;i++)
  {
   targ->next = null<Node*>();
   targ->valid = false;
   ((byte*&)targ) += sizeof(Node) + elementSize;
  }

 // Now get all the data in the old table and put it into the new one,
 // taking care not to create/delete chain nodes except when neccesary...
  targ = oldData;
  for (nat32 i=0;i<oldSize;i++)
  {
   Node * t = targ;
   while (t)
   {
    if (t->valid==true)
    {
     // We have some data that needs re-inserting - so thats what we do,
     // we have to take into account if this data will be deleted latter
     // or not, and delete it correctly, or reuse it...
     // The only nice thing is we don't have to worry about duplicates.
      Node * loc = data;
      ((byte*&)loc) += (sizeof(Node) + elementSize) * (t->index%size);

      if (loc->valid==false)
      {
       // We insert it into the primary slot - this should be the most common case...
        loc->valid = true;
        loc->index = t->index;
        mem::Copy<byte>((byte*)loc + sizeof(Node),(byte*)t + sizeof(Node),elementSize);

        if (t!=targ)
        {
         Node * tt = t->next;
         mem::Free(t);
         t = tt;
        }
        else t = t->next;
      }
      else
      {
       // We need to overflow into the chain - we insert it for speed, and reuse
       // the node if it allready exists...
        if (t!=targ)
        {
         Node * tt = t->next;
          t->next = loc->next;
          loc->next = t;
         t = tt;
         
         ++nodes;
        }
        else
        {
         Node * tt = (Node*)mem::Malloc<byte>(sizeof(Node) + elementSize); ++nodes;
          tt->next = loc->next;
          loc->next = tt;
          tt->valid = true;
          tt->index = t->index;
          mem::Copy<byte>((byte*)tt + sizeof(Node),(byte*)t + sizeof(Node),elementSize);
         t = t->next;
        }
      }
    } 
    else
    {
     if (t!=targ)
     {
      Node * tt = t->next;
      mem::Free(t);
      t = tt;
     }
     else t = t->next;
    }
   }
   ((byte*&)targ) += sizeof(Node) + elementSize;
  }
  
  mem::Free(oldData);
}

//------------------------------------------------------------------------------
 };
};
