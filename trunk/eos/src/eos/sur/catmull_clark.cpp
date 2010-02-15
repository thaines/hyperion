//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

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

#include "eos/sur/catmull_clark.h"


namespace eos
{
 namespace sur
 {
//------------------------------------------------------------------------------
EOS_FUNC Mesh * Subdivide(Mesh & old)
{
 // Check if we need to augment the old mesh with the 'sd.vert' field...
 Vertex handIni;
 bit doCommit = false;
 if (!old.ExistsVertProp("sd.vert")) {doCommit = true; old.AddVertProp("sd.vert",handIni);}
 if (!old.ExistsEdgeProp("sd.vert")) {doCommit = true; old.AddEdgeProp("sd.vert",handIni);}
 if (!old.ExistsFaceProp("sd.vert")) {doCommit = true; old.AddFaceProp("sd.vert",handIni);}
 if (doCommit) old.Commit();


 // Create the new mesh, with the same token table as the old...
  Mesh * ret = new Mesh(&old.TT());
  ret->AddVertProp("sd.vert",handIni);
  ret->AddEdgeProp("sd.vert",handIni);
  ret->AddFaceProp("sd.vert",handIni);
  ret->Commit();
  
  data::Property<Vertex,Vertex> propVert = old.GetVertProp<Vertex>("sd.vert");
  data::Property<Edge,Vertex>   propEdge = old.GetEdgeProp<Vertex>("sd.vert");
  data::Property<Face,Vertex>   propFace = old.GetFaceProp<Vertex>("sd.vert");


 // Create the vertices for the new mesh...
  ds::Array<Vertex> verts;
  ds::Array<Edge> edges;
  ds::Array<Face> faces;
  
  old.GetVertices(verts);
  old.GetEdges(edges);
  old.GetFaces(faces);
  
  for (nat32 i=0;i<verts.Size();i++) propVert.Get(verts[i]) = ret->NewVertex(bs::Vert(0.0,0.0,0.0));
  for (nat32 i=0;i<edges.Size();i++) propEdge.Get(edges[i]) = ret->NewVertex(bs::Vert(0.0,0.0,0.0));
  for (nat32 i=0;i<faces.Size();i++) propFace.Get(faces[i]) = ret->NewVertex(bs::Vert(0.0,0.0,0.0));


 // Create all the faces for the new mesh (Work by face), simultaneously 
 // set face vertices to the correct position, as they happen to iterate the 
 // same data...
  ds::Array<Vertex> tempVert;
  ds::Array<Edge> tempEdge;
  for (nat32 i=0;i<faces.Size();i++)
  {
   // Get the face details we need...
    faces[i].GetVertices(tempVert);
    faces[i].GetEdges(tempEdge);
  
   // Create the faces, one per face vertex...
    for (nat32 j=0;j<tempVert.Size();j++)
    {
     ret->NewFace(propFace.Get(faces[i]),
                  propEdge.Get(tempEdge[(j+tempVert.Size()-1)%tempVert.Size()]),
                  propVert.Get(tempVert[j]),
                  propEdge.Get(tempEdge[j]));
    }
    
   // Set the face vertex - simply the average of the faces vertices...
    bs::Vert mean(0.0,0.0,0.0);
    for (nat32 j=0;j<tempVert.Size();j++) mean += tempVert[j].Pos();
    mean /= real32(tempVert.Size());

    propFace.Get(faces[i]).Pos() = mean;
  }


 // Set the edge vertices to the correct position...
  ds::Array<Face> tempFace;
  for (nat32 i=0;i<edges.Size();i++)
  {
   // Get the vertices and faces for the edge...
    edges[i].GetVertices(tempVert);
    edges[i].GetFaces(tempFace);
    
   // Our behaviour depends on how many faces...
    if (tempFace.Size()<2)
    {
     // No faces or 1 face - simply set to half way point...
      bs::Vert pos = tempVert[0].Pos();
      pos += tempVert[1].Pos();
      pos *= 0.5;

      propEdge.Get(edges[i]).Pos() = pos;
    }
    else
    {
     // 2 or more faces - we apply the normal rule extended for further faces.
     // This is a bodge...
      // End points half-average...
       bs::Vert pos = tempVert[0].Pos();
       pos += tempVert[1].Pos();
       pos *= 0.25;
        
      // Face centres half-average...
       bs::Vert pos2(0.0,0.0,0.0);
       for (nat32 j=0;j<tempFace.Size();j++) pos2 += propFace.Get(tempFace[j]).Pos();
       
       pos2 /= real32(2 * tempFace.Size());
      
      // Store...
       pos += pos2;
       propEdge.Get(edges[i]).Pos() = pos;
    }
  }


 // Set the vertex vertices to the correct position...
  for (nat32 i=0;i<verts.Size();i++)
  {
   // Get the faces and edges...
    verts[i].GetEdges(tempEdge);
    verts[i].GetFaces(tempFace);

   // If we have the same number of faces as edges run the standard code, 
   // otherwise consider it to be a border vertex and adjust accordingly...
   // Various bodges exist here for non-manifold geomrty and other strange 
   // stuff.
    if (tempEdge.Size()==0)
    {
     // Special case for lonely vertices...
      propVert.Get(verts[i]).Pos() = verts[i].Pos();
    }
    else
    {
     if (tempEdge.Size()==tempFace.Size())
     {
      // Normal rule...
       bs::Vert pos = verts[i].Pos();
       pos *= real32(tempEdge.Size()-2)/real32(tempEdge.Size());
       
       bs::Vert pos2(0.0,0.0,0.0);
       for (nat32 j=0;j<tempEdge.Size();j++)
       {
        pos2 += propEdge.Get(tempEdge[j]).Pos();
        pos2 += propFace.Get(tempFace[j]).Pos();
       }
       pos2 /= real32(math::Sqr(tempEdge.Size()));
       
       pos += pos2;
       propVert.Get(verts[i]).Pos() = pos;
     }
     else
     {
      // Modified boundary rule - 1/4 of weight comes from all edges that have 
      // less than 2 faces (This is incase a vertex is shared between two 
      // boundarys)...
       bs::Vert pos = verts[i].Pos();
       pos *= 3.0/4.0;

       bs::Vert pos2(0.0,0.0,0.0);
       nat32 divisor = 0;
       for (nat32 j=0;j<tempEdge.Size();j++)
       {
        if (tempEdge[j].FaceCount()<2)
        {
         pos2 += tempEdge[j].Other(verts[i]).Pos();
         divisor += 1;
        }
       }

       if (divisor==0)
       {
        propVert.Get(verts[i]).Pos() = verts[i].Pos();
       }
       else
       {
        pos2 /= real32(4 * divisor);

        pos += pos2;
        propVert.Get(verts[i]).Pos() = pos;
       }
     }
    }
  }


 return ret;
}

//------------------------------------------------------------------------------
 };
};
