#ifndef EOS_FIT_SPHERE_SAMPLE_H
#define EOS_FIT_SPHERE_SAMPLE_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

/// \file sphere_sample.h
/// Provides a class for getting vertices on a sphere that allows you to 
/// subdivide the sphere where more detail is required.

#include "eos/types.h"
#include "eos/bs/geo3d.h"

namespace eos
{
 namespace fit
 {
//------------------------------------------------------------------------------
/// This is the triangle class used by SubDivSphere for users to access the 
/// structure. Starts with 12 of these, but soon divides down.
class EOS_CLASS SphereTriangle
{
 public:
  

 private:
  friend class SubDivSphere;
  nat32 * nextInd; // Assignment of indices - next new index - points to data in SubDivSphere.
  
  // Normal triangle vertex/edge naming is used below...
   SphereTriangle * parent;
   SphereTriangle * child[4]; // 0=adj to A, 1=adj to B, 2=adj to C, 3=middle. (A on a, B on b, C on c for middle triangle, the rest are oriented the same way as the parent. This means all are anti-clockwise.)
   
   SphereTriangle * adj[3]; // 0 for a, 1 for b, 2 for c.
   bs::Normal dir[3]; // 0 for A, 1 for B, 2 for C. Always calculated.
   nat32 ind[3]; // 0 for A, 1 for B, 2 for C. Unique indices, or -1 if not assigned. (At least in this part of the data structure - has to check others before assigning.)
};

//------------------------------------------------------------------------------
/// This provides a subdividable sphere where the user can selectivly subdivide
/// the bits that matter. Uses as its base an icosahedron and divides the 
/// triangular faces recusivly into 4 triangles using a triangle-based structure.
/// Allows you to get indices for the vertices, as well as there vectors, where
/// indices are allocated on first query so you can tell new once apart as there
/// higher than all previous.
class EOS_CLASS SubDivSphere
{
 public:
 
 
 private:
 
};

//------------------------------------------------------------------------------
 };
};
#endif
