//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/ds/collectors.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
CollectorCode::CollectorCode(nat32 elementSize,nat32 blockSize)
:size(0),packer(sizeof(void*) + (sizeof(Node)+elementSize)*blockSize),last(null<Node*>())
{}

CollectorCode::~CollectorCode()
{}

void CollectorCode::Add(nat32 elementSize,const void * data)
{
 Node * nn = (Node*)(void*)packer.Malloc<byte>(sizeof(Node)+elementSize);
 
 mem::Copy((byte*)nn->Data(),(byte*)data,elementSize);
 
 nn->next = last;
 last = nn;
 
 ++size;
}

void CollectorCode::Fill(nat32 elementSize,ArrayCode & out) const
{
 out.Resize(elementSize,size,NoOp,NoOp);
 Node * targ = last;
 for (nat32 i=0;i<size;i++)
 {
  mem::Copy(out.Item(elementSize,size-1-i),(byte*)targ->Data(),elementSize);
  targ = targ->next;
 }
}

void CollectorCode::Reverse()
{
 Node * newLast = null<Node*>();
 while (last)
 {
  Node * temp = last;
  last = temp->next;
  temp->next = newLast;
  newLast = temp;
 }
 last = newLast;
}

//------------------------------------------------------------------------------
 };
};
