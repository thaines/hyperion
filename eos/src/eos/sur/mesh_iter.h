#ifndef EOS_SUR_MESH_ITER_H
#define EOS_SUR_MESH_ITER_H
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
