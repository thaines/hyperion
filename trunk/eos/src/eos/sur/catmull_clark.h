#ifndef EOS_SUR_CATMULL_CLARK_H
#define EOS_SUR_CATMULL_CLARK_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file catmull_clark.h
/// Provides an implimentation of subdivision surfaces, of the catmull clark 
/// variety.

#include "eos/types.h"
#include "eos/sur/mesh.h"

namespace eos
{
 namespace sur
 {
//------------------------------------------------------------------------------
/// Simple function - you give it an input mesh and it returns a new mesh,
/// identical except a single catmull-clark subdivision has been applied.
/// The old mesh is augmented with the field 'sd.vert' on every vertex, edge and
/// face. It contains Vertex handles to the vertices in the new mesh, allowing
/// the two meshes to be synchronised. The new mesh will also be augmented with
/// the sd.vert field, so if you sub-divide again it will not have to commit and 
/// break the handles from the layer above the layer being sub-divided.
/// Don't forget to delete the returned mesh when you are done with it.
EOS_FUNC Mesh * Subdivide(Mesh & mesh);

//------------------------------------------------------------------------------
 };
};
#endif
