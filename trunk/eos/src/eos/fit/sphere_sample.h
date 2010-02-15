#ifndef EOS_FIT_SPHERE_SAMPLE_H
#define EOS_FIT_SPHERE_SAMPLE_H
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


/// \file sphere_sample.h
/// Provides a class for getting vertices on a sphere that allows you to 
/// subdivide the sphere where more detail is required.

#include "eos/types.h"
#include "eos/bs/geo3d.h"
#include "eos/ds/arrays_resize.h"
#include "eos/math/mat_ops.h"

namespace eos
{
 namespace fit
 {
//------------------------------------------------------------------------------
/// This provides a subdividable sphere where the user can selectivly subdivide
/// the bits that matter. Uses as its base an icosahedron and divides the 
/// triangular faces recusivly into 4 triangles using a triangle-based structure.
/// Allows you to get indices for the vertices, as well as there vectors, where
/// indices are allocated on triangle creation so you can tell new once apart
/// as there higher than all previous.
class EOS_CLASS SubDivSphere
{
 public:
  /// Handle to a triangle. Is just a unique increasing integer - can therefore
  /// be used to index other data structures.
   typedef nat32 Tri;


  /// &nbsp;
   SubDivSphere();

  /// &nbsp;
   ~SubDivSphere();


  /// Returns one of the 20 triangles that make up the icosahedron which is 
  /// subdivided to make the variable resolution sphere.
  /// \param ind 0..19 please.
   Tri GetRoot(nat32 ind) const {return ind;}

  /// Returns the number of triangles currently in use.
   nat32 TriCount() const {return tris.Size();}
   
  /// Returns the number of vertices currently in use.
   nat32 VertCount() const {return verts.Size();}


  /// Returns true if the given handle is a root triangle - this indicates
  /// that it has no parent.
   bit IsRoot(Tri tri) const {return tris[tri].parent==nat32(-1);}
   
  /// Returns true if the triangle has children.
   bit HasChildren(Tri tri) const {return tris[tri].child!=nat32(-1);}

  /// Returns true if the triangle at the same subdivision level connected to 
  /// edge a exists.
   bit HasAdjA(Tri tri) const {return tris[tri].adj[0]!=nat32(-1);}
  
  /// Returns true if the triangle at the same subdivision level connected to 
  /// edge b exists.
   bit HasAdjB(Tri tri) const {return tris[tri].adj[1]!=nat32(-1);}

  /// Returns true if the triangle at the same subdivision level connected to 
  /// edge c exists.
   bit HasAdjC(Tri tri) const {return tris[tri].adj[2]!=nat32(-1);}
   
   
  /// Returns the area of the given triangle. (As a triangle, not on the sphere surface.)
   real32 Area(Tri tri) const;
   
  /// This calls subdivide starting from the top triangle level a given number
  /// of times - 0 means to do nothing, 1 will subdivide each triangle at the top level,
  /// 2 will do the top level and then each of its children, etc.
  /// Don't set to high - this is obviously an exponential process.
   void DivideAll(nat32 levels);


  /// Returns a unique increasing integer for the vertex at the given index,
  /// as shared by all triangles.
   nat32 Ind(Tri tri,nat32 ind) const {return tris[tri].vertInd[ind];}

  /// Returns a unique increasing integer for the vertex at node A, shared by
  /// all triangles.
   nat32 IndA(Tri tri) const {return tris[tri].vertInd[0];}

  /// Returns a unique increasing integer for the vertex at node B, shared by
  /// all triangles.
   nat32 IndB(Tri tri) const {return tris[tri].vertInd[1];}

  /// Returns a unique increasing integer for the vertex at node C, shared by
  /// all triangles.
   nat32 IndC(Tri tri) const {return tris[tri].vertInd[2];}
   
  /// Gets the vertex associated with the given index returned by an Ind method.
  /// Note that these are contiguous - you can go from 0..VertCount() to get 'em all.
   const bs::Normal & Vert(nat32 ind) const {return verts[ind];}
   
  /// Returns the vertex at the vertex indexed by 0..2
   const bs::Normal & Vert(Tri tri,nat32 ind) const {return verts[tris[tri].vertInd[ind]];}
   
  /// Returns the vertex at node A.
   const bs::Normal & A(Tri tri) const {return verts[tris[tri].vertInd[0]];}

  /// Returns the vertex at node B.
   const bs::Normal & B(Tri tri) const {return verts[tris[tri].vertInd[1]];}

  /// Returns the vertex at node C.
   const bs::Normal & C(Tri tri) const {return verts[tris[tri].vertInd[2]];}


  /// Returns the parent triangle for a given triangle - do not call on root 
  /// triangles.
   Tri Parent(Tri tri) const {return tris[tri].parent;}


  /// Returns the middle child triangle of a given triangle, only call if the 
  /// given triangle has children.
   Tri GetM(Tri tri) const {return tris[tri].child;}
   
  /// Returns the child triangle sharing vertex A of a given triangle, only
  /// call if the given triangle has children.
   Tri GetA(Tri tri) const {return tris[tris[tri].child].adj[0];}

  /// Returns the child triangle sharing vertex B of a given triangle, only
  /// call if the given triangle has children.
   Tri GetB(Tri tri) const {return tris[tris[tri].child].adj[1];}

  /// Returns the child triangle sharing vertex C of a given triangle, only
  /// call if the given triangle has children.
   Tri GetC(Tri tri) const {return tris[tris[tri].child].adj[2];}

  /// Returns the middle child triangle of a given triangle.
  /// Will create it if it doesn't exist.
   Tri MakeM(Tri tri) {SubDivide(tri); return tris[tri].child;}
   
  /// Returns the child triangle sharing vertex A of a given triangle.
  /// Will create it if it doesn't exist.
   Tri MakeA(Tri tri) {SubDivide(tri); return tris[tris[tri].child].adj[0];}

  /// Returns the child triangle sharing vertex B of a given triangle.
  /// Will create it if it doesn't exist.
   Tri MakeB(Tri tri) {SubDivide(tri); return tris[tris[tri].child].adj[1];}

  /// Returns the child triangle sharing vertex C of a given triangle.
  /// Will create it if it doesn't exist.
   Tri MakeC(Tri tri) {SubDivide(tri); return tris[tris[tri].child].adj[2];}
   
   
  /// Returns the given triangles adjacent triangle connected to edge A.
  /// Only call if HasAdjA returns true.
   Tri AdjA(Tri tri) const {return tris[tri].adj[0];}

  /// Returns the given triangles adjacent triangle connected to edge B.
  /// Only call if HasAdjB returns true.
   Tri AdjB(Tri tri) const {return tris[tri].adj[1];}

  /// Returns the given triangles adjacent triangle connected to edge C.
  /// Only call if HasAdjC returns true.
   Tri AdjC(Tri tri) const {return tris[tri].adj[2];}

  /// Returns the given triangles adjacent triangle connected to edge A.
  /// Will create it if it doesn't exist.
   Tri MakeAdjA(Tri tri) {MakeExist(tri,0); return tris[tri].adj[0];}

  /// Returns the given triangles adjacent triangle connected to edge B.
  /// Will create it if it doesn't exist.
   Tri MakeAdjB(Tri tri) {MakeExist(tri,1); return tris[tri].adj[1];}

  /// Returns the given triangles adjacent triangle connected to edge C.
  /// Will create it if it doesn't exist.
   Tri MakeAdjC(Tri tri) {MakeExist(tri,2); return tris[tri].adj[2];}


  /// Returns the smallest triangle which collides with the given direction.
  /// Does not require that the given direction be normalised.
   Tri Collide(const bs::Normal & dir) const;
   
  /// Given a triangle and a direction outputs unnormalised trilinear coordinates
  /// for that direction in the triangle - will handle directions outside the
  /// triangle but note that its done in 3D space rather than on the surface of
  /// the sphere, so thats rather dubious.
  /// The values given are actual distances to edge planes - the user can
  /// normalise if need be.
  /// In use with collide allows for linear interpolation of any value to any
  /// given direction.
  /// (The coordinates do not factor in the sphere.)
   void Trilinear(Tri tri,const bs::Normal & dir,real32 & a,real32 & b,real32 & c) const;
  
  
  /// Given a triangle and an index indicating 0 for a, 1 for b and 2 for c this
  /// arranges for that adjacent triangle at the triangles level to exist if it
  /// doesn't already. Simply recursivly calls itself with subdivide calls until
  /// its down to the correct level.
   void MakeExist(Tri tri,nat32 adj);
  
  /// Given a triangle this subdivides that triangle into 4 smaller triangles,
  /// does nothing if the triangle has already been subdivided.
   void SubDivide(Tri tri);
   
  /// Subdivides the given triangle recursivly, taking it down to a fine 
  /// resolution if need be. For levels=0 nothing happens, for 1 its the same as
  /// SubDivide, for 2 its subdivide then subdivide on each of the children, etc.
  /// This is addative to any subdivisions so far.
   void RecSubDivide(Tri tri,nat32 levels);


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::alg::SubDivSphere";}


 private:
  // We construct this data structure using triangles. Use the standard
  // triangle naming scheme, anti-clockwise, with lower case for edges opposite
  // the similarly named upper case vertices...
  // Each triangle can have no child triangles or 4 child triangles. The first 3
  // share corners and have the same orientation as the parent, whilst the last
  // middle one has A on a of the parent, B on b and C on c, making it upside 
  // down but with the same rotation order.
  // Instead of pointers indices into the tris structure are used, so handles
  // can be passed out to the user. -1 is used as the 'null pointer'.
   struct Triangle
   {
    Tri parent; // -1 for the 20 top level ones.
    Tri child; // Points to the middle triangle alone - the others are then adjacent to it. -1 if at bottom.
    
    Tri adj[3]; // Adjacent triangles, can be -1.
    nat32 vertInd[3]; // Indices for the vertices of the triangle, always correct.
   }; // 8 32 bit numbers - 32 bytes.
  
  // Storage for all the triangles...
   ds::ArrayResize<Triangle> tris;
   
  // Shared storage of vertices, indexed from Triangles...
   ds::ArrayResize<bs::Normal> verts;
   
  
  // Helper functions used by the collision/interpolation code...
   real32 UnnormDist(const bs::Normal & dir,const bs::Normal & a,const bs::Normal & b) const
   {
    bs::Normal perp;
    math::CrossProduct(a,b,perp);
    return perp*dir;
   }
   
   real32 NormDist(const bs::Normal & dir,const bs::Normal & a,const bs::Normal & b) const
   {
    bs::Normal perp;
    math::CrossProduct(a,b,perp);
    perp.Normalise();
    return perp*dir;
   }
};

//------------------------------------------------------------------------------
 };
};
#endif
