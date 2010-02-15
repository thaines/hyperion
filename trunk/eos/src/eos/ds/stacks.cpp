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
