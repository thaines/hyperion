//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines
#include "eos/ds/stacks.h"

#include "eos/mem/alloc.h"
#include "eos/mem/functions.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
void StackCode::Del(void (*Term)(void * ptr))
{
 while (top)
 {
  Node * victim = top;
  top = victim->next;
  Term((byte*)victim + sizeof(Node));
  mem::Free((byte*)victim);	 
 }
}

void StackCode::Copy(nat32 elementSize,const StackCode & rhs)
{
 elements = rhs.elements;
 
 Node * tin = rhs.top;
 Node ** tout = &top;
 while (tin)
 {
  *tout = (Node*)mem::Malloc<byte>(sizeof(Node) + elementSize);
  mem::Copy((byte*)(*tout) + sizeof(Node),(byte*)tin + sizeof(Node),elementSize);
  
  tin = tin->next;
  tout = &((*tout)->next);
 }
 *tout = null<Node*>();
}

void StackCode::Push(nat32 elementSize,byte * data)
{
 Node * nn = (Node*)mem::Malloc<byte>(sizeof(Node) + elementSize);
 
 nn->next = top;
 top = nn;
 
 mem::Copy((byte*)nn + sizeof(Node),data,elementSize);
 ++elements;
}

void StackCode::Pop()
{
 Node * victim = top;
 top = victim->next;
 
 mem::Free((byte*)victim);
 --elements;	
}

void StackCode::PopKill(void (*Term)(void * ptr))
{
 Node * victim = top;
 top = victim->next;
 
 Term((byte*)victim + sizeof(Node));
 mem::Free((byte*)victim);
 --elements;	
}

//------------------------------------------------------------------------------
 };
};
