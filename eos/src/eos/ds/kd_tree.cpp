//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/ds/kd_tree.h"

#include "eos/math/functions.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
KdTreeCode::Node * KdTreeCode::Node::MakeNode(nat32 elementSize,const void * data)
{
 Node * ret = (Node*)mem::Malloc<byte>(sizeof(Node)+elementSize);
  ret->left = null<Node*>();
  ret->right = null<Node*>();
  mem::Copy<byte>((byte*)ret->Data(),(byte*)data,elementSize);
 return ret;
}

KdTreeCode::Node * KdTreeCode::Node::CloneNode(nat32 elementSize,KdTreeCode::Node * other)
{
 if (other==null<Node*>()) return null<Node*>();

 Node * ret = (Node*)mem::Malloc<byte>(sizeof(Node)+elementSize);
  ret->left = CloneNode(elementSize,other->left);
  ret->right = CloneNode(elementSize,other->right);
  mem::Copy<byte>((byte*)ret->Data(),(byte*)other->Data(),elementSize);
 return ret;
}

void KdTreeCode::Node::Die(void (*Term)(void * ptr))
{
 if (this==null<Node*>()) return;

 left->Die(Term);
 right->Die(Term);

 Term(Data());
 mem::Free(this);
}

void * KdTreeCode::Node::Add(nat32 elementSize,nat32 dimCount,nat32 dim,
                             const void * data,real64 (*DI)(const void * item,nat32 dim))
{
 real64 nodePos = DI(Data(),dim);
 real64 dataPos = DI(data,dim);
 if (dataPos<nodePos)
 {
  if (left) return left->Add(elementSize,dimCount,(dim+1)%dimCount,data,DI);
       else {left = MakeNode(elementSize,data); return left->Data();}
 }
 else
 {
  if (right) return right->Add(elementSize,dimCount,(dim+1)%dimCount,data,DI);
        else {right = MakeNode(elementSize,data); return right->Data();}
 }
}

KdTreeCode::Node * KdTreeCode::Node::MakeMany(nat32 elementSize,nat32 dimCount,nat32 dim,
                                              KdTreeCode::Node ** data,nat32 dataSize,real64 (*DI)(const void * item,nat32 dim))
{
 // Our behaviour depends directly on how many items we are dealing with...
  switch(dataSize)
  {
   case 0: return null<Node*>();

   case 1:
   {
    // Just make the bloody node...
     data[0]->left = null<Node*>();
     data[0]->right = null<Node*>();
     return data[0];
   }

   case 2:
   {
    // Set index 0 as the node to return and then put index 1 into the
    // relevent side...
     real64 nodePos = DI(data[0]->Data(),dim);
     real64 childPos = DI(data[1]->Data(),dim);
      if (childPos<nodePos)
      {
       data[0]->left = data[1];
       data[0]->right = null<Node*>();
      }
      else
      {
       data[0]->left = null<Node*>();
       data[0]->right = data[1];
      }
      data[1]->left = null<Node*>();
      data[1]->right = null<Node*>();
     return data[0];
   }

   case 3:
   {
    // Find the median node, set it as the centre and the other two as its children...
     real64 pos[3];
     nat32 ind[3];
     for (nat32 i=0;i<3;i++)
     {
      pos[i] = DI(data[i]->Data(),dim);
      ind[i] = i;
     }

     if (pos[0]>pos[1])
     {
      Swap(pos[0],pos[1]);
      Swap(ind[0],ind[1]);
     }
     if (pos[1]>pos[2])
     {
      Swap(pos[1],pos[2]);
      Swap(ind[1],ind[2]);
      if (pos[0]>pos[1])
      {
       Swap(pos[0],pos[1]);
       Swap(ind[0],ind[1]);
      }
     }

     data[ind[0]]->left = null<Node*>();
     data[ind[0]]->right = null<Node*>();
     data[ind[1]]->left = data[ind[0]];
     data[ind[1]]->right = data[ind[2]];
     data[ind[2]]->left = null<Node*>();
     data[ind[2]]->right = null<Node*>();

     return data[ind[1]];
   }

   default:
   {
    // Run a partial quicksort to find the median node and stick it in the centre with
    // all other nodes on the relevent sides. Make the pivot the returned node and recurse
    // on the two child ranges...
     int32 left = 0;
     int32 right = dataSize-1;
     int32 median = dataSize/2;
     while (true)
     {
      // Select a median of 3 pivot and stick the pivot into the right-1 position...
       int32 centre = (left+right)/2;
       real64 leftPos = DI(data[left]->Data(),dim);
       real64 centrePos = DI(data[centre]->Data(),dim);
       real64 rightPos = DI(data[right]->Data(),dim);

       if (centrePos<leftPos) {Swap(centrePos,leftPos); Swap(data[centre],data[left]);}
       if (rightPos<leftPos) {Swap(rightPos,leftPos); Swap(data[right],data[left]);}
       if (rightPos<centrePos) {Swap(rightPos,centrePos); Swap(data[right],data[centre]);}

       Swap(data[centre],data[right-1]);

      // Partition...
       int32 pivotIndex = left;
       for (int32 i=left;i<right-1;i++)
       {
	real64 targPos = DI(data[i]->Data(),dim);
	if (targPos < centrePos)
	{
         Swap(data[i],data[pivotIndex]);
	 ++pivotIndex;
	}
       }

       Swap(data[pivotIndex],data[right-1]);

      // Exit the loop if we have the index we want, otherwise continue in a different range...
       if (pivotIndex==median) break;
       if (median<pivotIndex) right = pivotIndex-1;
                         else left = pivotIndex+1;
     }

    // We have the data setup correctly, now recursivly construct the data structure...
     data[median]->left = MakeMany(elementSize,dimCount,(dim+1)%dimCount,data,median,DI);
     data[median]->right = MakeMany(elementSize,dimCount,(dim+1)%dimCount,&data[median+1],dataSize-median-1,DI);
     return data[median];
   }
  }
}

KdTreeCode::Node * KdTreeCode::Node::Nearest(nat32 elementSize,nat32 dimCount,nat32 dim,
                                             const void * dummy,real64 (*DI)(const void * item,nat32 dim),real64 & minDist)
{
 // First recurse down if need be, if at the bottom we finally choose it to be this...
  real64 thisPos = DI(Data(),dim);
  real64 dummyPos = DI(dummy,dim);
  bit checkedLeft;
  Node * best;
  if (dummyPos<thisPos)
  {
   checkedLeft = true;
   if (left) best = left->Nearest(elementSize,dimCount,(dim+1)%dimCount,dummy,DI,minDist);
   else
   {
    best = this;
    minDist = 0.0;
    for (nat32 i=0;i<dimCount;i++) minDist += math::Sqr(DI(Data(),i)-DI(dummy,i));
   }
  }
  else
  {
   checkedLeft = false;
   if (right) best = right->Nearest(elementSize,dimCount,(dim+1)%dimCount,dummy,DI,minDist);
   else
   {
    best = this;
    minDist = 0.0;
    for (nat32 i=0;i<dimCount;i++) minDist += math::Sqr(DI(Data(),i)-DI(dummy,i));
   }
  }

 // Secondly, on the way back determine if theres the possibility of a closer node
 // in the left/right branch, whichever one we havn't allready taken.
 // Of course, if that possibility exists we also have to check the current node
 // for being closer...
  if (math::Sqr(DI(Data(),dim)-DI(dummy,dim))<minDist)
  {
   real64 thisDist = 0.0;
   for (nat32 i=0;i<dimCount;i++) {thisDist += math::Sqr(DI(Data(),i)-DI(dummy,i)); if (thisDist>minDist) break;}
   if (thisDist<minDist)
   {
    best = this;
    minDist = thisDist;
   }

   if (checkedLeft) {if (right) best = right->NearestImprove(elementSize,dimCount,(dim+1)%dimCount,dummy,DI,minDist,best);}
               else {if (left) best = left->NearestImprove(elementSize,dimCount,(dim+1)%dimCount,dummy,DI,minDist,best);}
  }

 return best;
}

KdTreeCode::Node * KdTreeCode::Node::NearestImprove(nat32 elementSize,nat32 dimCount,nat32 dim,
                                                    const void * dummy,real64 (*DI)(const void * item,nat32 dim),
                                                    real64 & minDist,KdTreeCode::Node * bsf)
{
 real64 nodePos = DI(Data(),dim);
 real64 dummyPos = DI(dummy,dim);
 if (math::Sqr(nodePos-dummyPos)<minDist)
 {
  real64 thisDist = 0.0;
  for (nat32 i=0;i<dimCount;i++) {thisDist += math::Sqr(DI(Data(),i)-DI(dummy,i)); if (thisDist>minDist) break;}
  if (thisDist<minDist)
  {
   minDist = thisDist;
   bsf = this;
  }

  if (left) bsf = left->NearestImprove(elementSize,dimCount,(dim+1)%dimCount,dummy,DI,minDist,bsf);
  if (right) bsf = right->NearestImprove(elementSize,dimCount,(dim+1)%dimCount,dummy,DI,minDist,bsf);

  return bsf;
 }
 else
 {
  if (dummyPos<nodePos)
  {
   if (left) return left->NearestImprove(elementSize,dimCount,(dim+1)%dimCount,dummy,DI,minDist,bsf);
        else return bsf;
  }
  else
  {
   if (right) return right->NearestImprove(elementSize,dimCount,(dim+1)%dimCount,dummy,DI,minDist,bsf);
         else return bsf;
  }
 }
}

KdTreeCode::Node * KdTreeCode::Node::ApproxNearest(nat32 elementSize,nat32 dimCount,nat32 dim,
                                                   const void * dummy,real64 (*DI)(const void * item,nat32 dim),
                                                   PriorityQueue<KdTreeCode::NodePtr> & ob)
{
 real64 posDiff = DI(dummy,dim) - DI(Data(),dim);
 nat32 nextDim = (dim+1)%dimCount;
 if (posDiff<0.0)
 {
  if (right)
  {
   NodePtr np;
    np.dimDist = math::Sqr(DI(right->Data(),nextDim)-DI(dummy,nextDim));
    np.dim = nextDim;
    np.node = right;
   ob.Add(np);
  }

  if (left)
  {
   NodePtr np;
    np.dimDist = math::Sqr(posDiff);
    np.dim = dim;
    np.node = this;
   ob.Add(np);

   return left->ApproxNearest(elementSize,dimCount,nextDim,dummy,DI,ob);
  }
  else return this;
 }
 else
 {
  if (left)
  {
   NodePtr np;
    np.dimDist = math::Sqr(DI(left->Data(),nextDim)-DI(dummy,nextDim));
    np.dim = nextDim;
    np.node = left;
   ob.Add(np);
  }

  if (right)
  {
   NodePtr np;
    np.dimDist = math::Sqr(posDiff);
    np.dim = dim;
    np.node = this;
   ob.Add(np);

   return right->ApproxNearest(elementSize,dimCount,nextDim,dummy,DI,ob);
  }
  else return this;
 }
}

void KdTreeCode::Node::RangeGet(nat32 elementSize,nat32 dimCount,nat32 dim,
                                const math::Vector<real64> & min,const math::Vector<real64> & max,
                                ListCode * out,real64 (*DI)(const void * item,nat32 dim))
{
 bit insert = true;
 for (nat32 i=0;i<dimCount;i++)
 {
  real64 dp = DI(Data(),i);
  if ((dp<min[i])||(dp>max[i])) {insert = false; break;}
 }
 if (insert) out->AddBack(elementSize,(byte*)Data());

 real64 thisPos = DI(Data(),dim);
 if ((left)&&(min[dim]<thisPos)) left->RangeGet(elementSize,dimCount,(dim+1)%dimCount,min,max,out,DI);
 if ((right)&&(max[dim]>thisPos)) right->RangeGet(elementSize,dimCount,(dim+1)%dimCount,min,max,out,DI);
}

void KdTreeCode::Node::MakeList(nat32 & index,KdTreeCode::Node ** out)
{
 if (this==null<Node*>()) return;

 left->MakeList(index,out);

 out[index] = this;
 ++index;

 right->MakeList(index,out);
}

//------------------------------------------------------------------------------
KdTreeCode::KdTreeCode()
:size(0),root(null<Node*>())
{}

KdTreeCode::~KdTreeCode()
{}

void KdTreeCode::Del(void (*Term)(void * ptr))
{
 root->Die(Term);
 size = 0;
 root = null<Node*>();
}

void KdTreeCode::Copy(nat32 elementSize,const KdTreeCode & rhs)
{
 size = rhs.size;
 root = Node::CloneNode(elementSize,rhs.root);
}

void * KdTreeCode::Add(nat32 elementSize,nat32 dimCount,const void * data,real64 (*DI)(const void * item,nat32 dim))
{
 ++size;
 if (root)
 {
  return root->Add(elementSize,dimCount,0,data,DI);
 }
 else
 {
  root = Node::MakeNode(elementSize,data);
  return root->Data();
 }
}

void KdTreeCode::MassFill(nat32 elementSize,nat32 dimCount,const ArrayCode * ac,real64 (*DI)(const void * item,nat32 dim))
{
 // Create all the nodes...
  size = ac->elements;
  Node ** sa = new Node*[size];
  for (nat32 i=0;i<size;i++)
  {
   sa[i] = Node::MakeNode(elementSize,ac->Item(elementSize,i));
  }

 // Build it...
  root = Node::MakeMany(elementSize,dimCount,0,sa,size,DI);

 // Clean up...
  delete[] sa;
}

void KdTreeCode::Rebalance(nat32 elementSize,nat32 dimCount,real64 (*DI)(const void * item,nat32 dim))
{
 if (size!=0)
 {
  Node ** sa = new Node*[size];
   nat32 index = 0;
   root->MakeList(index,sa);
   root = Node::MakeMany(elementSize,dimCount,0,sa,size,DI);
  delete[] sa;
 }
}

KdTreeCode::Node * KdTreeCode::Nearest(nat32 elementSize,nat32 dimCount,
                                       const void * dummy,real64 (*DI)(const void * item,nat32 dim))
{
 real64 minDist;
 if (root) return root->Nearest(elementSize,dimCount,0,dummy,DI,minDist);
      else return null<Node*>();
}

KdTreeCode::Node * KdTreeCode::ApproxNearest(nat32 elementSize,nat32 dimCount,
                                             const void * dummy,real64 (*DI)(const void * item,nat32 dim),
                                             nat32 nodeLimit)
{
 if (root==null<Node*>()) return null<Node*>();

 // First get the node containning the dummy and a heap of possible
 // places to search...
  ann.MakeEmpty();
  Node * best = root->ApproxNearest(elementSize,dimCount,0,dummy,DI,ann);
  real64 bestDist = 0.0;
  for (nat32 i=0;i<dimCount;i++) bestDist += math::Sqr(DI(dummy,i)-DI(best->Data(),i));

 // Now iterate nodeLimit times, each time checking the best bin from the heap...
  for (nat32 i=0;i<nodeLimit;i++)
  {
   if (ann.Size()==0) break;

   NodePtr targ = ann.Peek(); ann.Rem();
   nat32 nextDim = (targ.dim+1)%dimCount;
   if (targ.dimDist<bestDist)
   {
    // Check the targetted node and add its two children nodes...
     real64 targDist = 0.0;
     for (nat32 i=0;i<dimCount;i++)
     {targDist += math::Sqr(DI(targ.node->Data(),i)-DI(dummy,i)); if (targDist>bestDist) break;}
     if (targDist<bestDist)
     {
      bestDist = targDist;
      best = targ.node;
     }

     if (targ.node->left)
     {
      NodePtr np;
       np.dimDist = math::Sqr(DI(targ.node->left->Data(),nextDim)-DI(dummy,nextDim));
       np.dim = nextDim;
       np.node = targ.node->left;
      ann.Add(np);
     }

     if (targ.node->right)
     {
      NodePtr np;
       np.dimDist = math::Sqr(DI(targ.node->right->Data(),nextDim)-DI(dummy,nextDim));
       np.dim = nextDim;
       np.node = targ.node->right;
      ann.Add(np);
     }
   }
   else
   {
    // Add to the heap only the relevent child of the targetted node...
     if (DI(dummy,targ.dim)<DI(targ.node->Data(),targ.dim))
     {
      if (targ.node->left)
      {
       NodePtr np;
        np.dimDist = math::Sqr(DI(targ.node->left->Data(),nextDim)-DI(dummy,nextDim));
        np.dim = nextDim;
        np.node = targ.node->left;
       ann.Add(np);
      }
     }
     else
     {
      if (targ.node->right)
      {
       NodePtr np;
        np.dimDist = math::Sqr(DI(targ.node->right->Data(),nextDim)-DI(dummy,nextDim));
        np.dim = nextDim;
        np.node = targ.node->right;
       ann.Add(np);
      }
     }
   }
  }

 return best;
}

void KdTreeCode::RangeGet(nat32 elementSize,nat32 dimCount,
                          const math::Vector<real64> & min,const math::Vector<real64> & max,
                          ListCode * out,real64 (*DI)(const void * item,nat32 dim))
{
 if (root) root->RangeGet(elementSize,dimCount,0,min,max,out,DI);
}

//------------------------------------------------------------------------------
InplaceKdTreeCode::InplaceKdTreeCode()
:dataSize(0),shrink(true),size(0),data(null<byte*>()),top(null<Node*>())
{}

InplaceKdTreeCode::~InplaceKdTreeCode()
{
 mem::Free(data);
}

void InplaceKdTreeCode::Node::NearestMan(nat32 dims,nat32 elementSize,const real32 * pos,
                                      nat32 depth,real32 & dist,Node *& bsf)
{
 depth = depth%dims;

 // Go down the relevant side first, before trying the other if needed...
  if (pos[depth]<Pos(depth))
  {
   // Left...
    if (left) left->NearestMan(dims,elementSize,pos,depth+1,dist,bsf);

   // If relevant now try self and the other side...
    if ((pos[depth]+dist)>=Pos(depth))
    {
     // Self...
      real32 selfDist = 0.0;
      for (nat32 i=0;i<dims;i++) selfDist += math::Abs(Pos(i)-pos[i]);
      if (selfDist<dist)
      {
       dist = selfDist;
       bsf = this;
      }

     // Right...
      if (right) right->NearestMan(dims,elementSize,pos,depth+1,dist,bsf);
    }
  }
  else
  {
   // Right...
    if (right) right->NearestMan(dims,elementSize,pos,depth+1,dist,bsf);

   // If relevant now try self and the other side...
    if ((pos[depth]-dist)<=Pos(depth))
    {
     // Self...
      real32 selfDist = 0.0;
      for (nat32 i=0;i<dims;i++) selfDist += math::Abs(Pos(i)-pos[i]);
      if (selfDist<dist)
      {
       dist = selfDist;
       bsf = this;
      }

     // Left...
      if (left) left->NearestMan(dims,elementSize,pos,depth+1,dist,bsf);
    }
  }
}

void InplaceKdTreeCode::Node::NearestEuc(nat32 dims,nat32 elementSize,const real32 * pos,
                                         nat32 depth,real32 & dist,Node *& bsf)
{
 depth = depth%dims;

 // Go down the relevant side first, before trying the other if needed...
  if (pos[depth]<Pos(depth))
  {
   // Left...
    if (left) left->NearestEuc(dims,elementSize,pos,depth+1,dist,bsf);

   // If relevant now try self and the other side...
    if ((pos[depth]+dist)>=Pos(depth))
    {
     // Self...
      real32 selfDist = 0.0;
      for (nat32 i=0;i<dims;i++) selfDist += math::Sqr(Pos(i)-pos[i]);
      if (selfDist<dist)
      {
       dist = selfDist;
       bsf = this;
      }

     // Right...
      if (right) right->NearestEuc(dims,elementSize,pos,depth+1,dist,bsf);
    }
  }
  else
  {
   // Right...
    if (right) right->NearestEuc(dims,elementSize,pos,depth+1,dist,bsf);

   // If relevant now try self and the other side...
    if ((pos[depth]-dist)<=Pos(depth))
    {
     // Self...
      real32 selfDist = 0.0;
      for (nat32 i=0;i<dims;i++) selfDist += math::Sqr(Pos(i)-pos[i]);
      if (selfDist<dist)
      {
       dist = selfDist;
       bsf = this;
      }

     // Left...
      if (left) left->NearestEuc(dims,elementSize,pos,depth+1,dist,bsf);
    }
  }
}

void InplaceKdTreeCode::Resize(nat32 dims,nat32 elementSize,nat32 newSize)
{
 top = null<Node*>();

 if (newSize==size) return;
 if ((newSize<dataSize)&&(shrink==false))
 {
  size = newSize;
  return;
 }

 nat32 nodeSize = sizeof(Node) + dims*sizeof(real32) + elementSize;
 byte * newData = mem::Malloc<byte>(nodeSize * newSize);

 mem::Copy(newData,data,nodeSize * math::Min(size,newSize));

 mem::Free(data);
 data = newData;
 dataSize = newSize;
 size = newSize;
}

InplaceKdTreeCode::Node * InplaceKdTreeCode::Build(nat32 dims,nat32 elementSize,nat32 depth,nat32 min,nat32 max)
{
 depth = depth%dims;
 nat32 medianCentre = (min+max)/2;
 nat32 nodeSize = sizeof(Node) + dims*sizeof(real32) + elementSize;


 // Find the mode of the given range, putting it in the centre of the range with
 // all less than on the left and all greater on the right...
 // (This is directly from quick sort, with median of 3 pivoting.)
  int32 left = int32(min);
  int32 right = int32(max);
  while (true)
  {
   if (right-left>1)
   {
    int32 centre = (left+right)/2;

    // First setup the pivot so it is in position right-1...
     real32 leftPos   = ((Node*)(void*)(data + nodeSize*left))->self->Pos(depth);
     real32 centrePos = ((Node*)(void*)(data + nodeSize*centre))->self->Pos(depth);
     real32 rightPos  = ((Node*)(void*)(data + nodeSize*right))->self->Pos(depth);

     if (centrePos<leftPos) {SelfSwap(nodeSize,centre,left); math::Swap(centrePos,leftPos);}
     if (rightPos<leftPos) {SelfSwap(nodeSize,right,left); math::Swap(rightPos,leftPos);}
     if (rightPos<centrePos) {SelfSwap(nodeSize,right,centre); math::Swap(rightPos,centrePos);}

     SelfSwap(nodeSize,right-1,centre);
    // Now do the actual swaping of elements to put everything correct relative to the centre...
     int32 i = int32(left);
     int32 j = int32(right) - 1;
     while (true)
     {
      while (true)
      {
       i += 1;
       real32 targPos = ((Node*)(void*)(data + nodeSize*i))->self->Pos(depth);
       if (targPos>=centrePos) break;
      }

      while (true)
      {
       j -= 1;
       real32 targPos = ((Node*)(void*)(data + nodeSize*j))->self->Pos(depth);
       if (targPos<=centrePos) break;
      }

      if (i<j) SelfSwap(nodeSize,i,j);
          else break;
     }

    // Return the pivot to the center...
     SelfSwap(nodeSize,i,right-1);

    // If the centre we have just set is the median break and use it,
    // otherwise go down the correct side to find it...
     if (i==int32(medianCentre)) break;
     else
     {
      if (i<int32(medianCentre)) left = i+1;
                            else right = i-1;
     }
   }
   else
   {
    if (left!=right)
    {
     real32 leftPos  = ((Node*)(void*)(data + nodeSize*left))->self->Pos(depth);
     real32 rightPos = ((Node*)(void*)(data + nodeSize*right))->self->Pos(depth);
     if (rightPos<leftPos) SelfSwap(nodeSize,left,right);
    }
    break;
   }
  }


 // Recurse down both sides, setting the returned node into the medians
 // left/right pointer...
  Node * ret = ((Node*)(void*)(data + nodeSize*medianCentre))->self;
  if (medianCentre!=min) ret->left = Build(dims,elementSize,depth+1,min,medianCentre-1);
                    else ret->left = null<Node*>();
  if (medianCentre!=max) ret->right = Build(dims,elementSize,depth+1,medianCentre+1,max);
                    else ret->right = null<Node*>();


 // Return the median...
  return ret;
}

void InplaceKdTreeCode::BuildAll(nat32 dims,nat32 elementSize)
{
 if (size==0) return;
 nat32 nodeSize = sizeof(Node) + dims*sizeof(real32) + elementSize;

 // Setup the self pointers...
  for (nat32 i=0;i<size;i++)
  {
   Node * targ = (Node*)(void*)(data + nodeSize*i);
   targ->self = targ;
  }

 // Get recursive...
  top = Build(dims,elementSize,0,0,size-1);
}

InplaceKdTreeCode::Node * InplaceKdTreeCode::NearestMan(nat32 dims,nat32 elementSize,const real32 * pos,real32 * d)
{
 Node * ret = null<Node*>();
 real32 dist = math::Infinity<real32>();
 top->NearestMan(dims,elementSize,pos,0,dist,ret);
 if (d) *d = dist;
 return ret;
}

InplaceKdTreeCode::Node * InplaceKdTreeCode::NearestEuc(nat32 dims,nat32 elementSize,const real32 * pos,real32 * d)
{
 Node * ret = null<Node*>();
 real32 dist = math::Infinity<real32>();
 top->NearestEuc(dims,elementSize,pos,0,dist,ret);
 if (d) *d = dist;
 return ret;
}

InplaceKdTreeCode::Node * InplaceKdTreeCode::Index(nat32 dims,nat32 elementSize,nat32 index)
{
 nat32 nodeSize = sizeof(Node) + dims*sizeof(real32) + elementSize;
 return (Node*)(void*)(data + nodeSize*index);
}

//------------------------------------------------------------------------------
 };
};
