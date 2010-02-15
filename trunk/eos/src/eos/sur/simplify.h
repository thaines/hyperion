#ifndef EOS_SUR_SIMPLIFY_H
#define EOS_SUR_SIMPLIFY_H
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


/// \file simplify.h
/// Mesh simplification algorithm, based on 'Surface Simplification Using 
/// Quadric Error Metrics' by Michael Garland and Paul S. Heckbert.

#include "eos/types.h"
#include "eos/sur/mesh.h"
#include "eos/ds/sorting.h"

namespace eos
{
 namespace sur
 {
//------------------------------------------------------------------------------
/// This merges all vertices that are within a given distance of each other
/// together to remove duplicates.
EOS_FUNC void RemDups(Mesh * mesh,real32 range,time::Progress * prog = null<time::Progress*>());

//------------------------------------------------------------------------------
/// Helper method for below, exported for potential value elsewhere. Given a 
/// distance this checks all vertices in a mesh and makes sure that all vertices
/// within the given range are connected by a egde, adding edges to make this so.
/// Used by the simplification algorithm as a pre-processor so it will consider
/// connecting vertices that arn't actually part of the same mesh chunk.
EOS_FUNC void AddEdges(Mesh * mesh,real32 range,time::Progress * prog = null<time::Progress*>());

//------------------------------------------------------------------------------
/// Mesh simplification algorithm, based on 'Surface Simplification Using 
/// Quadric Error Metrics' by Michael Garland and Paul S. Heckbert.
/// Implimented as a class due to the internal complexity of it, as this makes
/// implimentation neater.
class EOS_CLASS Simplify
{
 public:
  /// &nbsp;
   Simplify();
 
  /// &nbsp;
   ~Simplify();


  /// Sets the mesh to be simplified - it will be editted in-place.
   void Set(Mesh * mesh);
   
  /// Sets the distance for the edge adding algorithm, run before the main course
  /// to add edges between close but unconnected vertices, allowing them to be merged.
  /// Defaults to 0.01, set to 0 to disable.
   void SetMerge(real32 dist);
   
  /// Sets the cost of moving a mesh edge - all edges used by only 1 edge have this
  /// cost applied for moving away from that edge. Defaults to 1.0
  void SetEdge(real32 cost);
   
  /// Runs the algorithm, simplifying to the specified number of vertices.
   void Run(nat32 verts,time::Progress * prog = null<time::Progress*>());
   
  /// Runs the algorithm, simplifying to the spoecified percentage of the starting verts.
   void RunP(real32 num,time::Progress * prog = null<time::Progress*>());


  /// &nbsp;
   static cstrconst TypeString() {return "eos::sur::Simplify";}
 
 
 private:
  Mesh * mesh;
  real32 mergeDist;
  real32 edgeCost;
  
  // Symetric 4x4 matrix, used as a measure of error for each vertex.
   struct ErrorV
   {
    real32 r0[4]; // Variable for each row, indexed by column offset from diagonal.
    real32 r1[3];
    real32 r2[2];
    real32 r3[1];
    
    void Zero()
    {
     for (nat32 i=0;i<4;i++) r0[i] = 0.0;
     for (nat32 i=0;i<3;i++) r1[i] = 0.0;
     for (nat32 i=0;i<2;i++) r2[i] = 0.0;
     for (nat32 i=0;i<1;i++) r3[i] = 0.0;
    }
    
    ErrorV & operator += (const ErrorV & rhs)
    {
     for (nat32 i=0;i<4;i++) r0[i] += rhs.r0[i];
     for (nat32 i=0;i<3;i++) r1[i] += rhs.r1[i];
     for (nat32 i=0;i<2;i++) r2[i] += rhs.r2[i];
     for (nat32 i=0;i<1;i++) r3[i] += rhs.r3[i];     
     return *this;
    }
    
    real32 Cost(const bs::Vert & pos) const
    {
     real32 s0 = r0[0]*pos[0] + r0[1]*pos[1] + r0[2]*pos[2] + r0[3];
     real32 s1 = r0[1]*pos[0] + r1[0]*pos[1] + r1[1]*pos[2] + r1[2];
     real32 s2 = r0[2]*pos[0] + r1[1]*pos[1] + r2[0]*pos[2] + r2[1];
     real32 s3 = r0[3]*pos[0] + r1[2]*pos[1] + r2[1]*pos[2] + r3[0];
     return s0*pos[0] + s1*pos[1] + s2*pos[2] + s3;
    }
    
    static void CostFunc(const math::Vect<3> & pos,math::Vect<1> & err,const ErrorV & ev)
    {
     real32 s0 = ev.r0[0]*pos[0] + ev.r0[1]*pos[1] + ev.r0[2]*pos[2] + ev.r0[3];
     real32 s1 = ev.r0[1]*pos[0] + ev.r1[0]*pos[1] + ev.r1[1]*pos[2] + ev.r1[2];
     real32 s2 = ev.r0[2]*pos[0] + ev.r1[1]*pos[1] + ev.r2[0]*pos[2] + ev.r2[1];
     real32 s3 = ev.r0[3]*pos[0] + ev.r1[2]*pos[1] + ev.r2[1]*pos[2] + ev.r3[0];
     err[0] = s0*pos[0] + s1*pos[1] + s2*pos[2] + s3;
    }
   };
   
  // Stores edge contractions info (Has two sorters, as we need to be able to index it by two systems.)...
   struct ContractE
   {
    Edge edge;

    bs::Vert newPos;
    real32 cost;

    // Calculates newPos and cost given relevant details.
     void FillIn(const bs::Vert & pa,const ErrorV & ea,
                 const bs::Vert & pb,const ErrorV & eb);
                 
    // Returns true if the operation is safe, i.e. doesn't do anything strange to the geometry...
     bit Safe() const;
   };
   
   struct CostSortContractE : public ds::Sort
   {
    static bit LessThan(ContractE * const & lhs,ContractE * const & rhs)
    {
     if (lhs->cost<rhs->cost) return true;
     if (lhs->cost>rhs->cost) return false;
     return lhs->edge.Index()<rhs->edge.Index();
    }
   };

   struct IndexSortContractE : public ds::Sort
   {
    static bit LessThan(const ContractE & lhs,const ContractE & rhs)
    {
     return lhs.edge.Index()<rhs.edge.Index();
    }
   };
};

//------------------------------------------------------------------------------
 };
};
#endif
