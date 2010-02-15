//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines
#include "eos/ds/queues.h"

#include "eos/mem/alloc.h"
#include "eos/mem/functions.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
void QueueCode::Del(void (*Term)(void * ptr))
{
 while (top)
 {
  Node * victim = top;
  top = victim->next;
  Term((byte*)victim + sizeof(Node));
  mem::Free((byte*)victim);	 
 }	
}

void QueueCode::Copy(nat32 elementSize,const QueueCode & rhs)
{
 elements = rhs.elements;
 end = &top;
  
 Node * tin = rhs.top;
 while (tin)
 {
  *end = (Node*)mem::Malloc<byte>(sizeof(Node) + elementSize);
  mem::Copy((byte*)(*end) + sizeof(Node),(byte*)tin + sizeof(Node),elementSize);
  
  tin = tin->next;
  end = &((*end)->next);
 }
 *end = null<Node*>();	
}

void QueueCode::Add(nat32 elementSize,byte * data)
{
 Node * nn = (Node*)mem::Malloc<byte>(sizeof(Node) + elementSize);
 
 nn->next = null<Node*>();
 *end = nn;
 end = &nn->next;
 
 mem::Copy((byte*)nn + sizeof(Node),data,elementSize);
 ++elements;
}

void QueueCode::Rem()
{
 Node * victim = top;
 top = victim->next;
 if (top==null<Node*>()) end = &top;
 
 mem::Free((byte*)victim);
 --elements;	
}

void QueueCode::RemKill(void (*Term)(void * ptr))
{
 Node * victim = top;
 top = victim->next;
 if (top==null<Node*>()) end = &top;

 Term((byte*)victim + sizeof(Node));
 mem::Free((byte*)victim);
 --elements;
}

//------------------------------------------------------------------------------
 };
};
