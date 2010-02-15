//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines
#include "eos/ds/sort_lists.h"

#include "eos/mem/alloc.h"
#include "eos/mem/functions.h"
#include "eos/math/functions.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
SortListCode::SortListCode(nat32 elementSize,const SortListCode & rhs)
{
 if (rhs.top)
 {
  top = Node::MakeNode(elementSize,*rhs.top);
  top->parent = null<Node*>();
 }
 else top = null<Node*>();
}

void SortListCode::Del(void (*Term)(void * ptr))
{
 if (top)
 {
  top->Del(Term);
  top = null<Node*>();
 }
}

void SortListCode::Copy(nat32 elementSize,const SortListCode & rhs)
{
 if (rhs.top)
 {
  top = Node::MakeNode(elementSize,*rhs.top);
  top->parent = null<Node*>();
 }
 else top = null<Node*>();
}

void SortListCode::Add(nat32 elementSize,byte * data,bit (*LessThan)(void * lhs,void * rhs),void (*Term)(void * ptr))
{
 if (top) top = top->Insert(elementSize,data,LessThan,Term);
 else
 {
  top = Node::MakeNode(elementSize,data);
  top->parent = null<Node*>();
 }
}

void * SortListCode::Find(byte * dummy,bit (*LessThan)(void * lhs,void * rhs)) const
{
 Node * targ = top;
  while (targ)
  {
   if (LessThan(dummy,(byte*)targ + sizeof(Node))) targ = targ->left;
   else if (LessThan((byte*)targ + sizeof(Node),dummy)) targ = targ->right;
   else return (byte*)targ + sizeof(Node);
  }
 return null<void*>();
}

void * SortListCode::Largest(byte * dummy,bit (*LessThan)(void * lhs,void * rhs)) const
{
 void * ret = null<void*>();
 Node * targ = top;
 while (targ)
 {
  if (LessThan(dummy,(byte*)targ + sizeof(Node)))
  {
   targ = targ->left;
  }
  else if (LessThan((byte*)targ + sizeof(Node),dummy))
  {
   ret = (byte*)targ + sizeof(Node);
   targ = targ->right;
  }
  else break;
 }
 return ret;
}

void SortListCode::Rem(byte * data,bit (*LessThan)(void * lhs,void * rhs),void (*Term)(void * ptr))
{
 if (top) top = top->Remove(data,LessThan,Term);
}

void SortListCode::Iter(void (*Func)(void * item,void * ptr),void * ptr) const
{
 if (top) top->Iter(Func,ptr);
}

void * SortListCode::Get(nat32 i) const
{
 if (top) return ((byte*)(void*)top->Get(i,0)) + sizeof(Node);
     else return null<void*>();
}

nat32 SortListCode::FindIndex(byte * dummy,bit (*LessThan)(void * lhs,void * rhs)) const
{
 nat32 offset = 0;
 Node * targ = top;
  while (targ)
  {
   if (LessThan(dummy,(byte*)targ + sizeof(Node)))
   {
    targ = targ->left;
   }
   else
   {
    offset += targ->left->Count();
    if (LessThan((byte*)targ + sizeof(Node),dummy))
    {
     offset += 1;
     targ = targ->right;
    }
    else
    {
     return offset;
    }
   }
  }
 return nat32(-1);
}
 
void * SortListCode::First() const
{
 if (top==null<Node*>()) return null<void*>();
 Node * targ = top;
 while (targ->left) targ = targ->left;
 return (byte*)targ + sizeof(Node);
}

void * SortListCode::Last() const
{
 if (top==null<Node*>()) return null<void*>();
 Node * targ = top;
 while (targ->right) targ = targ->right;
 return (byte*)targ + sizeof(Node);
}

bit SortListCode::Invariant() const
{
 if (top) return top->Invariant();
     else return true;
}

//------------------------------------------------------------------------------
SortListCode::Node * SortListCode::Node::MakeNode(nat32 elementSize,void * data)
{
 log::Assert(data,"SortList: ::MakeNode with null data");
 Node * ret = (Node*)mem::Malloc<byte>(sizeof(Node) + elementSize); log::Assert(ret,"SortList: Failed to malloc in MakeNode");
  ret->height = 0;
  ret->children = 0;
  ret->left = null<Node*>();
  ret->right = null<Node*>();
  mem::Copy<byte>((byte*)ret + sizeof(Node),(byte*)data,elementSize);
 return ret;
}

SortListCode::Node * SortListCode::Node::MakeNode(nat32 elementSize,const Node & copy)
{
 log::Assert(&copy,"SortList: Null copy given to MakeNode");
 Node * ret = (Node*)mem::Malloc<byte>(sizeof(Node) + elementSize);
 log::Assert(ret,"SortList: Failed to malloc in MakeNode copy");
 mem::Copy<byte>((byte*)ret + sizeof(Node),(byte*)(&copy) + sizeof(Node),elementSize);

 ret->height = copy.height;
 ret->children = copy.children;

 if (copy.left)
 {
  ret->left = MakeNode(elementSize,*copy.left);
  ret->left->parent = ret;
 }
 else ret->left = null<Node*>();
 
 if (copy.right)
 {
  ret->right = MakeNode(elementSize,*copy.right);
  ret->right->parent = ret;
 }
 else ret->right = null<Node*>();

 return ret;
}
void SortListCode::Node::Del(void (*Term)(void * ptr))
{
 log::Assert(this,"SortList: Null this in Del");
 if (left) left->Del(Term);
 if (right) right->Del(Term);

 Term((byte*)this + sizeof(Node));
 mem::Free<byte>((byte*)this);
}

int32 SortListCode::Node::Height() const
{
 if (this) return height;
      else return -1;
}

nat32 SortListCode::Node::Count() const
{
 if (this) return children+1;
      else return 0;
}

SortListCode::Node * SortListCode::Node::RotLeft()
{
 log::Assert(this,"SortList: Null this in RotLeft");
 Node * temp = left; log::Assert(temp,"SortList: Null left pointer in RotLeft");
 left = temp->right;
 if (left) left->parent = this;
 temp->right = this;
 temp->parent = this->parent;
 this->parent = temp;
 
 height = math::Max(left->Height(),right->Height()) + 1;
 children = left->Count() + right->Count();
 temp->height = math::Max(temp->left->Height(),temp->right->Height()) + 1;
 temp->children = temp->left->Count() + temp->right->Count();
 return temp;
}

SortListCode::Node * SortListCode::Node::RotRight()
{
 log::Assert(this,"SortList: Null this in RotRight");
 Node * temp = right; log::Assert(temp,"SortList: Null right pointer in RotRight");
 right = temp->left;
 if (right) right->parent = this;
 temp->left = this;
 temp->parent = this->parent;
 this->parent = temp; 
 
 height = math::Max(left->Height(),right->Height()) + 1;
 children = left->Count() + right->Count();
 temp->height = math::Max(temp->left->Height(),temp->right->Height()) + 1;
 temp->children = temp->left->Count() + temp->right->Count();
 return temp;
}

SortListCode::Node * SortListCode::Node::RotDoubleLeft()
{
 log::Assert(this,"SortList: Null this in RotDoubleLeft");
 left = left->RotRight();
 return RotLeft();
}

SortListCode::Node * SortListCode::Node::RotDoubleRight()
{
 log::Assert(this,"SortList: Null this in RotDoubleRight");
 right = right->RotLeft();
 return RotRight();
}

SortListCode::Node * SortListCode::Node::Insert(nat32 elementSize,byte * data,bit (*LessThan)(void * lhs,void * rhs),void (*Term)(void * ptr))
{
 log::Assert(this,"SortList: Null this in insert");
 Node * ret = this;

 if (LessThan(data,(byte*)this + sizeof(Node)))
 {
  // We go left...
   if (left) left = left->Insert(elementSize,data,LessThan,Term);
   else
   {
    left = (Node*)mem::Malloc<byte>(sizeof(Node) + elementSize);
    left->height = 0;
    left->children = 0;
    left->parent = this;
    left->left = null<Node*>();
    left->right = null<Node*>();
    mem::Copy<byte>((byte*)left + sizeof(Node),data,elementSize);
   }

   if (left->Height()-right->Height()==2)
   {
    if (LessThan(data,(byte*)left + sizeof(Node))) ret = RotLeft();
                                              else ret = RotDoubleLeft();
   }
 }
 else if (LessThan((byte*)this + sizeof(Node),data))
 {
  // We go right...
   if (right) right = right->Insert(elementSize,data,LessThan,Term);
   else
   {
    right = (Node*)mem::Malloc<byte>(sizeof(Node) + elementSize);
    right->height = 0;
    right->children = 0;
    right->parent = this;
    right->left = null<Node*>();
    right->right = null<Node*>();
    mem::Copy((byte*)right + sizeof(Node),data,elementSize);
   }

   if (right->Height()-left->Height()==2)
   {
    if (LessThan((byte*)right + sizeof(Node),data)) ret = RotRight();
                                               else ret = RotDoubleRight();
   }
 }
 else
 {
  // Duplicate...
   Term((byte*)this + sizeof(Node));
   mem::Copy<byte>((byte*)this + sizeof(Node),data,elementSize);
   return this;
 }

 height = math::Max(left->Height(),right->Height()) + 1;
 children = left->Count() + right->Count();
 return ret;
}

SortListCode::Node * SortListCode::Node::Remove(byte * data,bit (*LessThan)(void * lhs,void * rhs),void (*Term)(void * ptr))
{
 // Attempt to find the node to die...
  Node * targ = this;
  while (targ)
  {
   if (LessThan(data,(byte*)targ + sizeof(Node))) targ = targ->left;
   else
   {
    if (LessThan((byte*)targ + sizeof(Node),data)) targ = targ->right;
    else break;
   }
  }
  if (targ==null<Node*>()) return this; // Can't be found - nothing to do.

 // Check for a quick way out - if the node has a null child our job is easy,
 // otherwise we go down the hard road...
  if (targ->left==null<Node*>())
  {
   if (targ->parent==null<Node*>()) // Its the root node, just replace it with the relevant child.
   {
    log::Assert(targ==this);
    Node * ret = targ->right;
     Term((byte*)targ + sizeof(Node));
     mem::Free<byte>((byte*)targ);
    if (ret) ret->parent = null<Node*>();
    return ret;
   }
   else
   {
    // Put child node in its position then rebalance from parent...     
     if (targ->parent->left==targ) targ->parent->left  = targ->right;
                              else targ->parent->right = targ->right;
     if (targ->right) targ->right->parent = targ->parent;
    
    // Delete, and rebalance from parent...
     Node * p = targ->parent;
     Term((byte*)targ + sizeof(Node));
     mem::Free<byte>((byte*)targ);
     targ = p;
   }
  }
  else
  {
   if (targ->right==null<Node*>())
   {
    if (targ->parent==null<Node*>()) // Its the root node, just replace it with the relevant child.
    {
     log::Assert(targ==this);
     Node * ret = targ->left;
      Term((byte*)targ + sizeof(Node));
      mem::Free<byte>((byte*)targ);
     if (ret) ret->parent = null<Node*>();
     return ret;
    }
    else
    {
     // Move child node to bypass the targ...
      targ->left->parent = targ->parent;
      if (targ->parent->left==targ) targ->parent->left  = targ->left;
                               else targ->parent->right = targ->left;
    
     // Delete, and rebalance from parent...
      Node * p = targ->parent;
      Term((byte*)targ + sizeof(Node));
      mem::Free<byte>((byte*)targ);
      targ = p;
    }
   }
   else
   {
    // Problem - it has two children.
    // Swap out the node to be deleted, swap into its place an adjacent node which
    // will, by definition, have free pointers we can use to acchieve this 
    // operation...
     Node * toDie = targ;
     Node * toSwap;
     // Go down the path with the greatest height to get our node to swap, either
     // left then right till null or right then left till null.
     // If the node-to-wap has any children do a bypass, as it by definition can 
     // only have one, so both its children are free...
      if (right->height>left->height)
      {
       // Right, then lots of left...
       	targ = targ->right;
       	while (targ->left) targ = targ->left;

       	toSwap = targ;
       	targ = targ->parent;
       	if (targ->left==toSwap) targ->left  = toSwap->right;
	                   else targ->right = toSwap->right;
       	if (toSwap->right) toSwap->right->parent = targ;
      }
      else
      {
       // Left, then lots of right...
       	targ = targ->left;
       	while (targ->right) targ = targ->right;

       	toSwap = targ;
       	targ = targ->parent;
 	if (targ->right==toSwap) targ->right = toSwap->left;
	                    else targ->left  = toSwap->left;
       	if (toSwap->left) toSwap->left->parent = targ;
      }
      
     // Do the swap, fiddle with the pointers, then finally delete...            
      // Copy & fiddle...
       toSwap->height = toDie->height;
       toSwap->children = toDie->children;
       toSwap->parent = toDie->parent;
       toSwap->left = toDie->left;
       toSwap->right = toDie->right;
       
       if (toSwap->left) toSwap->left->parent = toSwap;
       if (toSwap->right) toSwap->right->parent = toSwap;
       
       if (toDie->parent)
       {
        if (toDie->parent->left==toDie) toDie->parent->left = toSwap;
                                   else toDie->parent->right = toSwap;
       }
       if (targ==toDie) targ = toSwap;

      // Deletion...
       Term((byte*)toDie + sizeof(Node));
       mem::Free<byte>((byte*)toDie);
   }  
  }

 
 // Work back upwards from the lowest edit point until we make no change in 
 // height, and rebalance the tree, also update children counts the whole way
 // up...
  // Complex pass, call this whilst its needed...
   Node * ret = targ;
   while (targ)
   {
    int32 nodeHeight = targ->Height();
    
    // Check for rotation requirements, rotate as needed...
     int32 leftHeight = targ->left->Height();
     int32 rightHeight = targ->right->Height();
     int32 diff = leftHeight - rightHeight;
     if (math::Abs(diff)>1)
     {
      Node ** update = null<Node**>();
      if (targ->parent)
      {
       if (targ->parent->left==targ) update = &targ->parent->left;
                                else update = &targ->parent->right;	      
      }

      if (diff>1)
      {
       // left hand side is deeper...
        int32 leftLeftHeight = targ->left->left->Height();
        int32 leftRightHeight = targ->left->right->Height();
        int32 leftDiff = leftLeftHeight - leftRightHeight;

        if (leftDiff<0) targ = targ->RotDoubleLeft();
                   else targ = targ->RotLeft();
      }
      else
      {
       // right hand side is deeper...
        int32 rightLeftHeight = targ->right->left->Height();
        int32 rightRightHeight = targ->right->right->Height();
        int32 rightDiff = rightLeftHeight - rightRightHeight;

        if (rightDiff>0) targ = targ->RotDoubleRight();
                    else targ = targ->RotRight(); 	      
      }
      if (update) *update = targ;	     
     }
     else
     {
      // Update height and child count (Rotations do this anyway, hence encapsulating this in the else.)...
       targ->height = math::Max(leftHeight,rightHeight) + 1;
       targ->children = targ->left->Count() + targ->right->Count();
     }


    // If this nodes height doesn't change then we can break out and just update
    // the child count from here on in...
     if (targ->height==nodeHeight)
     {
      ret = targ;
      targ = targ->parent;
      break;	     
     }

    ret = targ;
    targ = targ->parent;	   
   }
  
  // Once we no we no longer need the complex pass complete with a simple pass
  // that re-calculates children counts.
   while (targ)
   {
    targ->children = targ->left->Count() + targ->right->Count();
    ret = targ;

    targ = targ->parent;	   
   }

 
 log::Assert((ret==null<Node*>())||(ret->parent==null<Node*>()));
 return ret;
}

void SortListCode::Node::Iter(void (*Func)(void * item,void * ptr),void * ptr)
{
 if (left) left->Iter(Func,ptr);
 Func((byte*)this + sizeof(Node),ptr);
 if (right) right->Iter(Func,ptr);
}

SortListCode::Node * SortListCode::Node::Get(nat32 i,nat32 offset)
{
 log::Assert(this,"SortList: Null this in Get");
 nat32 num = offset + left->Count();
 if (i<num) return left->Get(i,offset);
 if (i>num) return right->Get(i,num+1);
 return this;
}

bit SortListCode::Node::Invariant() const
{
 if (this==null<SortListCode::Node*>()) return true;
 int32 diff = left->Height() - right->Height();
 if (math::Abs(diff)>1) return false;
 return left->Invariant() && right->Invariant();
}

//------------------------------------------------------------------------------
void SortListCode::CursorCode::ToStart()
{
 // Up...
  while (targ->parent) targ = targ->parent;
 
 // Go left...
  while (targ->left) targ = targ->left;
}

void SortListCode::CursorCode::ToEnd()
{
 // Up...
  while (targ->parent) targ = targ->parent;
 
 // Go right...
  while (targ->right) targ = targ->right;
}
     
SortListCode::CursorCode & SortListCode::CursorCode::operator ++ ()
{
 // Whilst we emerge from the right branch keep heading up,
 // the moment, including the first case, we can go right 
 // when we didn't just come from the right we break. 
 // The moment we come up from the left we have an answer.
  Node * last = null<Node*>();
  while (true)
  {
   if (targ==null<Node*>()) return *this;

   // If we have just come from the left we have an answer...
    if ((last)&&(last==targ->left)) return *this;
       
   // If we havn't just come from the right and right is avaliable
   // head right and break...
    if ((last!=targ->right)&&(targ->right))
    {
     targ = targ->right;
     break;
    }
   
   // Head up...
    last = targ;
    targ = targ->parent;
  }
  
  
 // We have gone down a right branch - we must now head left
 // as far as possible...
  while (targ->left) targ = targ->left;
  
 return *this;
}

SortListCode::CursorCode & SortListCode::CursorCode::operator -- ()
{
 // Whilst we emerge from the left branch keep heading up,
 // the moment, including the first case, we can go left 
 // when we didn't just come from the left we break. 
 // The moment we come up from the right we have an answer.
  Node * last = null<Node*>();
  while (true)
  {
   if (targ==null<Node*>()) return *this;

   // If we have just come from the right we have an answer...
    if ((last)&&(last==targ->right)) return *this;
       
   // If we havn't just come from the left and left is avaliable
   // head left and break...
    if ((last!=targ->left)&&(targ->left))
    {
     targ = targ->left;
     break;
    }
   
   // Head up...
    last = targ;
    targ = targ->parent;
  }
  
  
 // We have gone down a left branch - we must now head right
 // as far as possible...
  while (targ->right) targ = targ->right;

 return *this;
}

//------------------------------------------------------------------------------
 };
};
