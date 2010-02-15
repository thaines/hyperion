#ifndef EOS_ALG_SHAPES_H
#define EOS_ALG_SHAPES_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file shapes.h
/// Contains code for generating various shapes, and weird variations of this
/// activity.

#include "eos/types.h"
#include "eos/bs/geo3d.h"

namespace eos
{
 namespace alg
 {
//------------------------------------------------------------------------------
/// Set of constants that define an icosahedron in terms of normals. To generate
/// an actual icosahedron you simply take the centre and add to it each normal 
/// multiplied by the desired radius to get the final coordinate positions. This
/// structure also specifies which normals are connected to each other as 
/// triangles to generate an actual icosahedron rendering.
struct EOS_CLASS Icosahedron
{
 /// Vertices of the shape as unit normals from the centre; [vertex number][0=x;1=y;2=z]
  static const real32 vert[12][3];

 /// Edges of the shape, as pairs of indexes.
  static const nat32 edge[30][2];

 /// The indexes of the sets you need to form the triangles that construct it.
 /// All given to go anti-clockwise when looking at the facing out side.
  static const nat32 tri[20][3];
};

//------------------------------------------------------------------------------
/// A triangulation of a sphere that produces an even distribution of vertices.
/// An icosahedron subdivided into smaller triangles which are remaped back onto
/// a sphere. A method of specifying a geodesic dome. Refered to here as an 
/// icosphere, in accordance with Blender (blender.org) termanology. You specify
/// the subdivision count on construction. Note that unlike Blender we use a 
/// single step method where the subdivisions are the number of splits on each
/// side of the original icosahedron edges. This allows for more control.
class EOS_CLASS Icosphere
{
 public:
  /// Converts subdivisions into a vertex count.
   static nat32 SubToVert(nat32 subdivs);

  /// Converts subdivisions into an edge count.
   static nat32 SubToEdge(nat32 subdivs);
   
  /// Converts subdivisions into a triangle count.
   static nat32 SubToTri(nat32 subdivs);
   
  /// Slightly unushall really, converts subdivisions into the closest angle 
  /// between any two normals on such an Icosphere. An indication of how 
  /// accuratly such a sphere represents the real thing.
   static real32 SubToAng(nat32 subdivs);

    
  /// subdivs is the number of splits in the edges of the original Icosahedron.
  /// The static member function SubToVert will tell you how many vertices
  /// a given subdivision level will produce prior to construction.
  /// subdivs==0 will produce an Icosahedron.
   Icosphere(nat32 subdivs);
 
  /// &nbsp;
   ~Icosphere();


  /// Returns the number of vertices.
   nat32 Verts() const {return verts;}
   
  /// Returns the number of edges.
   nat32 Edges() const {return edges;}
   
  /// Returns the number of triangles.
   nat32 Tris() const {return tris;}
   
   
  /// Indexes the vertices, there stored as normals so you can take
  /// the centre of the object and add the normal multiplied by the radius
  /// to get actual vertices. Returns a pointer to 3 real32's.
   const real32 * Vert(nat32 i) const {return &vert[i*3];}
   
  /// Indexes the edges, returns pairs of indexes to vertices.  
   const nat32 * Edge(nat32 i) const {return &edge[i*2];}
   
  /// Indexes the triangles, returns triplets of indexes to vertices.
   const nat32 * Tri(nat32 i) const {return &tri[i*3];}


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::alg::Icosphere";}


 private:
  nat32 verts;
  nat32 edges;
  nat32 tris;
  
  real32 * vert;
  nat32 * edge;
  nat32 * tri;
};

//------------------------------------------------------------------------------
/// This class provides a number of normals evenly distributed over a 
/// hemisphere, you specify the number of subdivisions of an icosphere, it then 
/// acts as an indexable array of normals. You find out how many post-construction.
/// All the normals have z>=0.
class EOS_CLASS HemiOfNorm
{
 public:
  /// The more subdivisions the more normals. Well, duh.
   HemiOfNorm(nat32 subdivs);
   
  /// &nbsp;
   HemiOfNorm(const HemiOfNorm & rhs);
   
  /// &nbsp;
   ~HemiOfNorm();

 
  /// &nbsp;
   HemiOfNorm & operator = (const HemiOfNorm & rhs);


  /// Returns the number of normals distributed over the sphere.
   nat32 Norms() const {return norms;}   
  
  /// Returns a const reference to the indexed normal.
   const bs::Normal & Norm(nat32 ind) const {return norm[ind];}
   
  /// Returns the angle between 'adjacent' vertices, taken through the centre of
  /// a sphere.
   real32 Spacing() const {return spacing;};


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::alg::HemiOfNorm";}


 private:
  nat32 norms;
  bs::Normal * norm;
  real32 spacing;
};

//------------------------------------------------------------------------------
 };
};
#endif
