//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

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
