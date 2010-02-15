#ifndef EOS_FIT_SPHERE_SAMPLE_H
#define EOS_FIT_SPHERE_SAMPLE_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

/// \file sphere_sample.h
/// Provides a class for getting vertices on a sphere that allows you to 
/// subdivide the sphere where more detail is required.

#include "eos/types.h"
#include "eos/bs/geo3d.h"
#include "eos/ds/arrays_resize.h"

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


  /// Returns true if the given handle is a root triangle - this indicates
  /// that it has no parent.
   bit IsRoot(Tri tri) const {return tris[tri].parent==nat32(-1);}
   
  /// Returns true if the triangle has children.
   bit HasChildren(Tri tri) const {return tris[tri].child!=nat32(-1);}

  /// Returns true if the triangle at the same subdivision level connected to 
  /// edge a exists.
   bit HasA(Tri tri) const {return tris[tri].adj[0]!=nat32(-1);}
  
  /// Returns true if the triangle at the same subdivision level connected to 
  /// edge b exists.
   bit HasB(Tri tri) const {return tris[tri].adj[1]!=nat32(-1);}

  /// Returns true if the triangle at the same subdivision level connected to 
  /// edge c exists.
   bit HasC(Tri tri) const {return tris[tri].adj[2]!=nat32(-1);}


  /// Returns a unique increasing integer for the vertex at node A, shared by
  /// all triangles.
   nat32 IndA(Tri tri) const {return tris[tri].vertInd[0];}

  /// Returns a unique increasing integer for the vertex at node B, shared by
  /// all triangles.
   nat32 IndB(Tri tri) const {return tris[tri].vertInd[1];}

  /// Returns a unique increasing integer for the vertex at node C, shared by
  /// all triangles.
   nat32 IndC(Tri tri) const {return tris[tri].vertInd[2];}
   
  /// Returns the vertex at node A.
   const bs::Normal & A(Tri tri) const {return verts[tris[tri].vertInd[0]];}

  /// Returns the vertex at node B.
   const bs::Normal & B(Tri tri) const {return verts[tris[tri].vertInd[1]];}

  /// Returns the vertex at node C.
   const bs::Normal & C(Tri tri) const {return verts[tris[tri].vertInd[2]];}


  /// Returns the parent triangle for a given triangle - do not call on root 
  /// triangles.
   Tri Parent(Tri tri) {return tris[tri].parent;}
   
   
   
   
  /// Given a triangle this subdivides that triangle into 4 smaller triangles,
  /// does nothing if the triangle has already been subdivided.
   void SubDivide(Tri tri);


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
    nat32 parent; // -1 for the 20 top level ones.
    nat32 child; // Points to the middle triangle alone - the others are then adjacent to it. -1 if at bottom.
    
    nat32 adj[3]; // Adjacent triangles, can be -1.
    nat32 vertInd[3]; // Indices for the vertices of the triangle, always correct.
   }; // 8 32 bit numbers - 32 bytes.
  
  // Storage for all the triangles...
   ds::ArrayResize<Triangle> tris;
   
  // Shared storage of vertices, indexed from Triangles...
   ds::ArrayResize<bs::Normal> verts;
};

//------------------------------------------------------------------------------
 };
};
#endif
