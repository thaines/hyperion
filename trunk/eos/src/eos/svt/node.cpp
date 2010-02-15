//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

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

#include "eos/svt/node.h"

namespace eos
{
 namespace svt
 {
//------------------------------------------------------------------------------
Node::Node(Node * victim)
:core(victim->core),parent(victim->parent),child(victim->child),next(victim->next),last(victim->last)
{
 // Correct victim pointers in the sibling list to point to this...
  if (next==victim)
  {
   next = this;
   last = this;
  }
  else
  {
   next->last = this;
   last->next = this;
  }
  
 // If need be correct the parent's pointer...
  if ((parent)&&(parent->child==victim)) parent->child = this;
   
 // Also correct the child list...
 {
  Node * targ = child;
  if (targ)
  {
   do
   {
    targ->parent = this;
    targ = targ->next;
   } while (targ!=child);
  }
 }
 

 // Arrange it so the victim object won't hurt this when it dies...
  victim->parent = null<Node*>();
  victim->child = null<Node*>();
  victim->next = victim;
  victim->last = victim;
}

nat32 Node::ChildCount() const
{
 nat32 ret = 0;
  if (child)
  {
   Node * targ = child;
   do
   {
    ret += 1;
    targ = targ->Next();
   } while (!targ->First());
  } 
 return ret;
}

void Node::Detach()
{
 if (parent)
 {
  if (parent->child==this) parent->child = next;
  if (parent->child==this) parent->child = null<Node*>();
  parent = null<Node*>();
 }
 
 next->last = last;
 last->next = next;

 next = this;
 last = this;
}

void Node::KillChildren()
{
 while (child)
 {
  delete child;
 }
}

void Node::AttachParent(Node * p)
{
 Detach();

 parent = p;
 if (parent->child)
 {
  next = parent->child;
  last = next->last;

  next->last = this;
  last->next = this;
 }
 else parent->child = this;
}

void Node::AttachChild(Node * c)
{
 Detach();

 if (child)
 {
  c->next = child;
  c->last = child->last;
  child = c;
 
  child->next->last = child;
  child->last->next = child;  
 }
 else child = c;
 
 child->parent = this;
}

void Node::AttachBefore(Node * n)
{
 Detach();

 parent = n->parent;

 next = n;
 last = n->last;

 next->last = this;
 last->next = this;

 if (next->First()) parent->child = this;
}

void Node::AttachAfter(Node * l)
{
 Detach();

 parent = l->parent;

 next = l->next;
 last = l;

 next->last = this;
 last->next = this;
}

nat32 Node::TotalMemory() const
{
 nat32 ret = Memory();
 if (child)
 {
  Node * targ = child;
  do
  {
   ret += targ->TotalMemory();
   targ = targ->Next();
  } while (!targ->First());
 }
 return ret;
}

nat32 Node::WriteSize() const
{
 nat32 ret = 16 + 4;
 if (child)
 {
  Node * targ = child;
  do
  {
   ret += targ->TotalWriteSize();
   targ = targ->Next();
  } while (!targ->First());
 }
 return ret;
}

nat32 Node::TotalWriteSize() const
{
 return WriteSize();
}

cstrconst Node::MagicString() const
{
 return "HON";
}

nat32 Node::Write(io::OutVirt<io::Binary> & out) const
{
 nat32 ret = 0;
  // Header...
   nat32 bs = WriteSize();
   nat32 os = TotalWriteSize();
   nat8 zb = 0;

   ret += out.Write("HON",3);
   ret += out.Write(MagicString(),3);
   ret += out.Write(&bs,4);
   ret += out.Write(&zb,1);
   ret += out.Write(&os,4);
   ret += out.Write(&zb,1);
;
  // Child count...
   nat32 childCount = ChildCount();
   ret += out.Write(&childCount,4);

  // Children...
   if (child)
   {
    Node * targ = child;
    do
    {
     ret += targ->Write(out);
     targ = targ->Next();
    } while (!targ->First());
   }   

 if (ret!=bs) out.SetError(true);
 return ret;
}

void Node::ReadBlock(io::InVirt<io::Binary> & in)
{
 struct Head
 {
  cstrchar bm[3];
  cstrchar om[3];
  nat32 bSize;
  nat8 bExt;
  nat32 oSize;
  nat8 oExt;
 } head;
 
 nat32 rsf = 0;
  rsf += in.Read(head.bm,3);
  rsf += in.Read(head.om,3);
  rsf += in.Read(&head.bSize,4);
  rsf += in.Read(&head.bExt,1);
  rsf += in.Read(&head.oSize,4);
  rsf += in.Read(&head.oExt,1);
  
  if ((head.bm[0]!='H')||(head.bm[1]!='O')||(head.bm[2]!='N')) {in.SetError(true); return;}
  if ((head.bExt!=0)||(head.oExt!=0)) {in.SetError(true); return;}

  // Header is loaded, time for the children...
   nat32 children;
   rsf += in.Read(&children,4);
   if (rsf!=20) {in.SetError(true); return;}
   
   for (nat32 i=0;i<children;i++)
   {
    Node * nc = core.LoadObject(in);
    if (nc==null<Node*>()) {in.SetError(true); return;}
    nc->AttachParent(this);
   }

 in.SetError(rsf!=head.bSize);
}

//------------------------------------------------------------------------------
 };
};
