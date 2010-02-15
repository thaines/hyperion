//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/ds/falloff.h"

#include "eos/math/functions.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
FalloffCode::FalloffCode(nat32 dim)
:data(sizeof(Node) + dim * sizeof(real32))
{}

FalloffCode::~FalloffCode()
{}

void FalloffCode::SetSize(nat32 sz)
{
 data.Size(sz);
}

void FalloffCode::Set(nat32 ind,real32 cost)
{
 data[ind].cost = cost;
}

void FalloffCode::Set(nat32 dim,nat32 ind,const real32 * pos)
{
 for (nat32 i=0;i<dim;i++) data[ind].Pos()[i] = pos[i];
}

void FalloffCode::Set(nat32 ind,nat32 ord,real32 val)
{
 data[ind].Pos()[ord] = val;
}

void FalloffCode::Build(nat32 dim,const math::Distance & dist)
{
 // First sort the nodes in the big array, lowest to highest by cost...
  data.Sort< SortOp<Node> >();


 // Now construct the kd tree, starting with the lowest cost node and so on.
 // This allows us to check for and delete nodes that have no affect on the cost
 // terrain...
  data[0].left = null<Node*>();
  data[0].right = null<Node*>();
  
  for (nat32 i=1;i<data.Size();i++)
  {
   // Check if the node in question is of sufficiently low cost to be allowed
   // in...
    if (Cost(dim,dist,data[i].Pos())<data[i].cost) continue;
   
   // Add the node to the kd tree...
    data[i].left = null<Node*>();
    data[i].right = null<Node*>();
   
    nat32 depth = 0;
    Node * targ = &(data[0]);
    while (true)
    {
     nat32 ord = depth%dim;
     if (data[i].Pos()[ord]<targ->Pos()[ord])
     {
      // Left...
       if (targ->left)
       {
        targ = targ->left;
        depth += 1;
       }
       else
       {
        targ->left = &(data[i]);
        break;
       }
     }
     else
     {
      // Right...
       if (targ->right)
       {
        targ = targ->right;
        depth += 1;
       }
       else
       {
        targ->right = &(data[i]);
        break;
       }
     }
    }
  }
}

real32 FalloffCode::Cost(nat32 dim,const math::Distance & dist,const real32 * pos) const
{
 real32 ret = math::Infinity<real32>();
 CostRec(dim,dist,&(data[0]),0,pos,ret);
 return ret; 
}

void FalloffCode::CostRec(nat32 dim,const math::Distance & dist,Node * targ,nat32 depth,const real32 * pos,real32 & out)
{
 nat32 ord = depth%dim;
 if (pos[ord]<targ->Pos()[ord])
 {
  // Left...
   if (targ->left) CostRec(dim,dist,targ->left,depth+1,pos,out);
   
  // Self...
   out = math::Min(out,targ->cost + dist(dim,targ->Pos(),pos));
  
  // Right...
   real32 edgeDist = dist(1,&(targ->Pos()[ord]),&(pos[ord]));
   if ((targ->right)&&(edgeDist<out)) CostRec(dim,dist,targ->right,depth+1,pos,out);
 }
 else
 {
  // Right...
   if (targ->right) CostRec(dim,dist,targ->right,depth+1,pos,out);

  // Self...
   out = math::Min(out,targ->cost + dist(dim,targ->Pos(),pos));

  // Left...
   real32 edgeDist = dist(1,&(pos[ord]),&(targ->Pos()[ord]));
   if ((targ->left)&&(edgeDist<out)) CostRec(dim,dist,targ->left,depth+1,pos,out);
 }
}

//------------------------------------------------------------------------------
 };
};
