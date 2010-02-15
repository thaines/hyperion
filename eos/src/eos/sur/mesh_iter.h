#ifndef EOS_SUR_MESH_ITER_H
#define EOS_SUR_MESH_ITER_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

/// \file mesh_iter.h
/// Classes for iterating things in a mesh, to save on the creation of linked 
/// lists/arrays as that obviously slow.

#include "eos/types.h"
#include "eos/sur/mesh.h"

namespace eos
{
 namespace sur
 {
//------------------------------------------------------------------------------
/// Given a vertex this iterates all the edges that it is a member of.
class EOS_CLASS IterVertexEdges
{
 public:
  /// &nbsp;
   IterVertexEdges(const Vertex & vert);
   
  /// &nbsp;
   ~IterVertexEdges();

 
  /// Returns true if Targ() is safe to call.
   bit Valid() const;

  /// Returns current target.
   Edge Targ() const;

  /// Moves to next target, or makes valid false if its out of targets.
  /// If valid is already false does nothing.
   void Next();


  /// &nbsp;
   static cstrconst TypeString() {return "eos::sur::IterVertexEdges";}


 private:
  Mesh::Vertex * vert;
  Mesh::DirEdge * targ;
};

//------------------------------------------------------------------------------
/// Given a vertex this iterates all its adjacent faces.
class EOS_CLASS IterVertexFaces
{
 public:
  /// &nbsp;
   IterVertexFaces(const Vertex & vert);
   
  /// &nbsp;
   ~IterVertexFaces();


  /// Returns true if Targ() is safe to call.
   bit Valid() const;

  /// Returns current target.
   Face Targ() const;

  /// Moves to next target, or makes valid false if its out of targets.
  /// If valid is already false does nothing.
   void Next();


  /// &nbsp;
   static cstrconst TypeString() {return "eos::sur::IterVertexFaces";}


 private:
  Mesh::Vertex * vert;
  Mesh::DirEdge * targ1;
  Mesh::HalfEdge * targ2;
};

//------------------------------------------------------------------------------
 };
};
#endif
