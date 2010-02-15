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

#include "eos/ds/lists.h"

#include "eos/mem/alloc.h"
#include "eos/mem/functions.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
void ListCode::Del(void (*Term)(void * ptr))
{
 Node * targ = dummy.next;
 while (targ!=&dummy)
 {
  Node * victim = targ;
  targ = targ->next;
  
  Term((byte*)victim + sizeof(Node));
  mem::Free((byte*)victim);
 }	
}

void ListCode::Copy(nat32 elementSize,const ListCode & rhs)
{
 elements = 0;
 dummy.next = &dummy;
 dummy.last = &dummy;
 
 Node * targ = rhs.dummy.next;
 while (targ!=&rhs.dummy)
 {
  AddBack(elementSize,(byte*)targ + sizeof(Node));
  targ = targ->next;	 
 }
}

void ListCode::AddFront(nat32 elementSize,byte * data)
{
 Node * nn = (Node*)mem::Malloc<byte>(sizeof(Node) + elementSize);
 mem::Copy((byte*)nn + sizeof(Node),data,elementSize);
 
 nn->last = &dummy;
 nn->next = dummy.next;
 
 nn->next->last = nn;
 nn->last->next = nn;
 ++elements;	
}

void ListCode::AddBack(nat32 elementSize,byte * data)
{
 Node * nn = (Node*)mem::Malloc<byte>(sizeof(Node) + elementSize);
 mem::Copy((byte*)nn + sizeof(Node),data,elementSize);
 
 nn->last = dummy.last;
 nn->next = &dummy;
 
 nn->next->last = nn;
 nn->last->next = nn;
 ++elements;	
}

void ListCode::RemFront()
{
 Node * victim = dummy.next;
 
 victim->next->last = victim->last;	
 victim->last->next = victim->next;
 
 mem::Free(victim);
 --elements;
}

void ListCode::RemBack()
{
 Node * victim = dummy.last;
 
 victim->next->last = victim->last;	
 victim->last->next = victim->next;
 
 mem::Free(victim);
 --elements;
}

void ListCode::RemFrontKill(void (*Term)(void * ptr))
{
 Node * victim = dummy.next;
 
 victim->next->last = victim->last;	
 victim->last->next = victim->next;
 
 Term((byte*)victim + sizeof(Node));
 mem::Free(victim);
 --elements;
}

void ListCode::RemBackKill(void (*Term)(void * ptr))
{
 Node * victim = dummy.last;
 
 victim->next->last = victim->last;	
 victim->last->next = victim->next;
 
 Term((byte*)victim + sizeof(Node));
 mem::Free(victim);
 --elements;
}

void ListCode::Rem(ListCode::Node * victim)
{
 victim->next->last = victim->last;	
 victim->last->next = victim->next;
 
 mem::Free(victim);
 --elements;	
}

void ListCode::RemKill(ListCode::Node * victim,void (*Term)(void * ptr))
{
 victim->next->last = victim->last;	
 victim->last->next = victim->next;
 
 Term((byte*)victim + sizeof(Node));
 mem::Free(victim);
 --elements;
}

ListCode::Node * ListCode::AddBefore(nat32 elementSize,byte * data,ListCode::Node * targ)
{
 Node * nn = (Node*)mem::Malloc<byte>(sizeof(Node) + elementSize);
 mem::Copy((byte*)nn + sizeof(Node),data,elementSize);

 nn->next = targ;
 nn->last = targ->last;
 
 nn->next->last = nn;
 nn->last->next = nn;

 ++elements;
 return nn;
}

ListCode::Node * ListCode::AddAfter(nat32 elementSize,byte * data,ListCode::Node * targ)
{
 Node * nn = (Node*)mem::Malloc<byte>(sizeof(Node) + elementSize);
 mem::Copy((byte*)nn + sizeof(Node),data,elementSize);
 
 nn->last = targ;
 nn->next = targ->next;
 
 nn->next->last = nn;
 nn->last->next = nn;

 ++elements;
 return nn;
}

//------------------------------------------------------------------------------
 };
};
