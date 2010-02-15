//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

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

#include "eos/sur/mesh_sup.h"

namespace eos
{
 namespace sur
 {
//------------------------------------------------------------------------------
MeshTransfer::MeshTransfer(const Mesh & from,Mesh & ttoo)
:to(ttoo)
{
 // Create the 3 op arrays...
  Build(from.vertProp,from.vertByName,to.vertProp,vertOp);
  Build(from.edgeProp,from.edgeByName,to.edgeProp,edgeOp);
  Build(from.faceProp,from.faceByName,to.faceProp,faceOp);
 
 // Optimise them...
  Optimise(vertOp);
  Optimise(edgeOp);
  Optimise(faceOp);
 
 // Create the interpolation info...
  str::Token realType = (*to.tt)("real32"); 
  // Count...
   nat32 size = 0;
   for (nat32 i=0;i<to.vertProp.Size();i++)
   {
    if ((to.vertProp[i].type==realType)&&
        (from.vertByName.Exists(to.vertProp[i].name))&&
        (from.vertProp[*from.vertByName.Get(to.vertProp[i].name)].type==realType))
    {
     ++size;
    }
   }
   
  // Construct...
   vertReal.Size(size);
   size = 0;
   for (nat32 i=0;i<to.vertProp.Size();i++)
   {
    if ((to.vertProp[i].type==realType)&&
        (from.vertByName.Exists(to.vertProp[i].name))&&
        (from.vertProp[*from.vertByName.Get(to.vertProp[i].name)].type==realType))
    {
     vertReal[size].fromOffset = from.vertProp[*from.vertByName.Get(to.vertProp[i].name)].offset;
     vertReal[size].toOffset = to.vertProp[i].offset;
     ++size;
    }
   }   
}

MeshTransfer::~MeshTransfer()
{}

void MeshTransfer::Transfer(sur::Vertex to,const sur::Vertex from)
{
 for (nat32 i=0;i<vertOp.Size();i++) vertOp[i].DoTra((byte*)to.vert,(byte*)from.vert);
}

void MeshTransfer::Transfer(sur::Edge to,const sur::Edge from)
{
 for (nat32 i=0;i<edgeOp.Size();i++) edgeOp[i].DoTra((byte*)to.edge,(byte*)from.edge);
}

void MeshTransfer::Transfer(sur::Face to,const sur::Face from)
{
 for (nat32 i=0;i<faceOp.Size();i++) faceOp[i].DoTra((byte*)to.face,(byte*)from.face);
}

sur::Vertex MeshTransfer::Interpolate(nat32 n,sur::Vertex * v,real32 * weight)
{
 // Calculate the position for the new vertex...
  bs::Vert pos(0.0,0.0,0.0);
  real32 weightSum = 0.0;
  for (nat32 i=0;i<n;i++)
  {
   pos[0] += v[i].Pos()[0] * weight[i];
   pos[1] += v[i].Pos()[1] * weight[i];
   pos[2] += v[i].Pos()[2] * weight[i];
   weightSum += weight[i];
  }
  
  pos[0] /= weightSum;
  pos[1] /= weightSum;
  pos[2] /= weightSum;


 // Create it...
  sur::Vertex ret = to.NewVertex(pos);
 
 // Transfer over from the first vertex all normal stuff...
  for (nat32 i=0;i<vertOp.Size();i++) vertOp[i].DoTra((byte*)ret.vert,(byte*)v[0].vert);
 
 // Interpolate real32 fields...
  for (nat32 i=0;i<vertReal.Size();i++)
  {
   real32 val = 0.0;
   for (nat32 j=0;j<n;j++) val += weight[j] * (*(real32*)(void*)(((byte*)v[j].vert)+vertReal[i].fromOffset));
   val /= weightSum;
   
   (*(real32*)(void*)(((byte*)ret.vert)+vertReal[i].toOffset)) = val;
  }
 
 // Return the new vertex...
  return ret;
}

void MeshTransfer::Build(const ds::Array<Mesh::Prop> & from,const ds::SparseHash<nat32> & index,
                         const ds::Array<Mesh::Prop> & to,ds::Array<Op> & out)
{
 out.Size(to.Size());
 for (nat32 i=0;i<out.Size();i++)
 {
  out[i].toOffset = to[i].offset;
  out[i].size = to[i].size;
  if ((index.Exists(to[i].name))&&(from[*index.Get(to[i].name)].size==to[i].size))
  {
   out[i].type = true;
   out[i].fromOffset = from[*index.Get(to[i].name)].offset;
  }
  else
  {
   out[i].type = false;
   out[i].fromPtr = to[i].ini;
  }
 }
}

void MeshTransfer::Optimise(ds::Array<MeshTransfer::Op> & ops)
{
 // Firstly sort the operations by there 'to' offset...
  ops.SortNorm();
 
 // Now check for operations that can be merged...
  nat32 newLength = ops.Size();
  nat32 last = 0;
  for (nat32 i=1;i<ops.Size();i++)
  {
   if ((ops[last].type==true)&&(ops[i].type==true)&& // Check there both the right kind of copys.
      ((ops[last].toOffset+ops[last].size)==ops[i].toOffset)&& // Check they copy to adjacent locations.
      ((ops[last].fromOffset+ops[last].size)==ops[i].fromOffset)) // Check they copy from adjacent locations.
   {
    // They can be merged...
     ops[last].size += ops[i].size;
     --newLength;
   }
   else
   {
    // They can't be merged...
     ++last;
     if (last!=i) ops[last] = ops[i];
   }
  }
  ops.Size(newLength);
}

//------------------------------------------------------------------------------
 };
};
