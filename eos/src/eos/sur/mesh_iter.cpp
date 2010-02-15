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

#include "eos/sur/mesh_iter.h"

namespace eos
{
 namespace sur
 {
//------------------------------------------------------------------------------
IterVertexEdges::IterVertexEdges(const Vertex & v)
:vert(v.vert)
{
 if (vert->edge!=null<Mesh::DirEdge*>())
 {
  targ = vert->edge->next;
 }
 else vert = null<Mesh::Vertex*>();
}

IterVertexEdges::~IterVertexEdges()
{}

bit IterVertexEdges::Valid() const
{
 return vert!=null<Mesh::Vertex*>();
}

Edge IterVertexEdges::Targ() const
{
 return Edge(targ);
}

void IterVertexEdges::Next()
{
 if (vert==null<Mesh::Vertex*>()) return;
 if (targ==vert->edge) vert = null<Mesh::Vertex*>();
                  else targ = targ->next;
}

//------------------------------------------------------------------------------
IterVertexFaces::IterVertexFaces(const Vertex & v)
:vert(v.vert)
{
 // Select the first face in the vertices set, have to consider that there 
 // might be no faces for this vertex...
  if (vert->edge==null<Mesh::DirEdge*>())
  {
   vert = null<Mesh::Vertex*>();
   return;
  }
 
  targ1 = vert->edge->next;
  while (true)
  {
   if (targ1->user)
   {
    targ2 = targ1->user->next;
    break;
   }
  
   if (targ1==vert->edge)
   {
    targ1 = null<Mesh::DirEdge*>();
    targ2 = null<Mesh::HalfEdge*>();
    vert = null<Mesh::Vertex*>();
    break;
   }
   else targ1 = targ1->next;
  }
}

IterVertexFaces::~IterVertexFaces()
{}

bit IterVertexFaces::Valid() const
{
 return vert!=null<Mesh::Vertex*>();
}

Face IterVertexFaces::Targ() const
{
 return Face(targ2->face);
}

void IterVertexFaces::Next()
{
 if (vert==null<Mesh::Vertex*>()) return;

 if (targ2!=targ1->user) targ2 = targ2->next;
 else
 {
  while (true)
  {
   if (targ1==vert->edge)
   {
    vert = null<Mesh::Vertex*>();
    break;
   }
   else
   {
    targ1 = targ1->next;
    if (targ1->user!=null<Mesh::HalfEdge*>())
    {
     targ2 = targ1->user->next;
     break;
    }
   }
  }
 }
}

//------------------------------------------------------------------------------
 };
};
