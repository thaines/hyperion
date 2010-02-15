#ifndef EOS_SUR_SUBDIVIDE_H
#define EOS_SUR_SUBDIVIDE_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file subdivide.h
/// Subdivides a mesh.

#include "eos/types.h"

#include "eos/sur/mesh.h"

namespace eos
{
 namespace sur
 {
//------------------------------------------------------------------------------
/// Given an input mesh that must be triangulated this returns a new mesh that 
/// has had each triangle subdivided along each edge the given number of times,
/// with the centres filled in accordingly.
/// (divs==0 will result in the original input.)
/// The subdivisions are flat, so the mesh remains the same shape.
/// Remember to delete the returned mesh.
EOS_FUNC Mesh * Subdivide(Mesh & in,nat32 divs);

//------------------------------------------------------------------------------
 };
};
#endif
