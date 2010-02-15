//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/sur/intersection.h"

#include "eos/math/mat_ops.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace sur
 {
//------------------------------------------------------------------------------
EOS_FUNC bit RayTriIntersect(bs::Ray & ray,
                             const bs::Vert & a,const bs::Vert & b,const bs::Vert & c,
                             real32 & dist,real32 & wa,real32 & wb,real32 & wc)
{
 // First find the distance where the ray intrsects the plane, returning false
 // if its perpendicular or the distance is negative...
  bs::Vert ab = b; ab -= a;
  bs::Vert ac = c; ac -= a;
  bs::Vert n;
  math::CrossProduct(ab,ac,n);

  real32 divisor = n * ray.n;
  if (math::IsZero(divisor)) return false;

  bs::Vert rrs = a; rrs -= ray.s;
  dist = n*rrs;
  if (math::Sign(dist)!=math::Sign(divisor)) return false;
  dist /= divisor;


 // Calculate the position of the point of intersection relative to the first vertex of the triangle...
  bs::Vert is = ray.n;
  is *= dist;
  is += ray.s;
  is -= a;


 // Find the triangular coordinates, check if they are in range for a  collision...
  real32 qab = ab * is;
  real32 qac = ac * is;
  real32 ang = ab * ac;

  wb = ac.LengthSqr() * qab - ang * qac;
  if (wb<0.0) return false;

  wc = ab.LengthSqr() * qac - ang * qab;
  if (wc<0.0) return false;

  real32 scaler = n.LengthSqr();
  if (wb+wc>scaler) return false;


 // Normalise the triangular coordinates for final output (And fill in wa.)...
  wb /= scaler;
  wc /= scaler;
  wa = 1.0 - wb - wc;

 return true;
}

//------------------------------------------------------------------------------
 };
};
