#ifndef EOS_SUR_INTERSECTION_H
#define EOS_SUR_INTERSECTION_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file intersection.h
/// Contains basic intersection algorithms between various primatives.

#include "eos/types.h"
#include "eos/bs/geo3d.h"

namespace eos
{
 namespace sur
 {
//------------------------------------------------------------------------------
/// This intersects a ray with a triangle. Returns true on success, false on
/// failure. On returning true it also supply the distance traveled along the
/// way to the point of collision and the triangular coordinates of the point of
/// collision. (These, rather conveniantly, being the weights for any
/// interpolation from the 3 vertices.)
EOS_FUNC bit RayTriIntersect(bs::Ray & ray,
                             const bs::Vert & a,const bs::Vert & b,const bs::Vert & c,
                             real32 & dist,real32 & wa,real32 & wb,real32 & wc);

//------------------------------------------------------------------------------
 };
};
#endif
