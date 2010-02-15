//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

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
