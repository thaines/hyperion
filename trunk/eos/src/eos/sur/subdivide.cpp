//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/sur/subdivide.h"

#include "eos/ds/arrays2d.h"

namespace eos
{
 namespace sur
 {
//------------------------------------------------------------------------------
EOS_FUNC Mesh * Subdivide(Mesh & in,nat32 divs)
{
 Mesh * ret = new Mesh(&in.TT());

 // Get arrays containing all the faces of the input mesh...
  ds::Array<sur::Vertex> vert;
  ds::Array<sur::Edge> edge;
  ds::Array<sur::Face> face;

  in.GetVertices(vert);
  in.GetEdges(edge);
  in.GetFaces(face);


 // Transfer over the corner vertices from the old mesh, keeping an array of
 // them...
  ds::Array<sur::Vertex> newVert(vert.Size());
  for (nat32 i=0;i<newVert.Size();i++) newVert[i] = ret->NewVertex(vert[i].Pos());


 // Create the divided edge of each edge in the input mesh, with the relevant
 // vertices... (Again, store copys ready for further screwing around with.)
  ds::Array2D<sur::Vertex> newEdge(divs,edge.Size());
  for (nat32 e=0;e<newEdge.Height();e++)
  {
   // Get the end locations...
    bs::Vert a = edge[e].VertexA().Pos();
    bs::Vert b = edge[e].VertexB().Pos();

   // Iterate the edge dividing vertices and create each one...
    for (nat32 i=0;i<divs;i++)
    {
     real32 t = real32(i+1)/real32(divs+1);
     bs::Vert ip;
     for (nat32 j=0;j<3;j++) ip[j] = t*a[j] + (1.0-t)*b[j];
     newEdge.Get(i,e) = ret->NewVertex(ip);
    }
  }


 // Iterate all faces of the input mesh, for each one create the relevent
 // not-shared output structure...
 // (This is where is gets hard, but at least we don't have to store the output.)
  nat32 totalDivs = divs+2;
  ds::Array<sur::Vertex> cur(math::Triangular(totalDivs));
  ds::Array<sur::Vertex> corner(3);
  ds::Array<sur::Edge> side(3);
  for (nat32 i=0;i<face.Size();i++)
  {
   // Fill in the 3 corners from the previous data structures...
    face[i].GetVertices(corner);
    log::Assert(corner.Size()==3);
    cur[0] = newVert[corner[0].Index()];
    cur[cur.Size()-totalDivs] = newVert[corner[0].Index()];
    cur[cur.Size()-1] = newVert[corner[0].Index()];


   // Fill in the edges from the previous data structures, making sure to get
   // them the right way arround...
    face[i].GetEdges(side);
    // Side 0...
     nat32 ind = 1;
     nat32 e = side[0].Index();
     int32 pos, step;
     if (edge[e].VertexA()==corner[0])
     {pos = 0; step = 1;}
     else
     {pos = divs-1; step = -1;}

     for (nat32 j=1;j<=divs;j++)
     {
      cur[ind] = newEdge.Get(pos,e);
      pos += step;
      ind += j+1;
     }

    // Side 1...
     e = side[1].Index();
     if (edge[e].VertexA()==corner[1])
     {pos = 0; step = 1;}
     else
     {pos = divs-1; step = -1;}

     for (nat32 j=1;j<=divs;j++)
     {
      cur[cur.Size()-totalDivs+j] = newEdge.Get(pos,e);
      pos += step;
     }

    // Side 2...
     ind = cur.Size()-totalDivs-1;
     e = side[2].Index();
     if (edge[e].VertexA()==corner[2])
     {pos = 0; step = 1;}
     else
     {pos = divs-1; step = -1;}

     for (nat32 j=divs;j>0;j--)
     {
      cur[ind] = newEdge.Get(pos,e);
      pos += step;
      ind -= j+1;
     }


   // Create new vertices to fill in the soft juicy centre...
    ind = 1;
    for (nat32 j=1;j<=divs;j++)
    {
     ++ind;
     for (nat32 k=1;k<j;k++)
     {
      real32 t0 = totalDivs - j;
      real32 t1 = j - k;
      real32 t2 = k;

      real32 mult = math::InvSqrt(math::Sqr(t0) + math::Sqr(t1) + math::Sqr(t2));
      t0 *= mult; t1 *= mult; t2 *= mult;

      bs::Vert ip;
      for (nat32 l=0;l<3;l++) ip[l] = t0*corner[0].Pos()[l] + t1*corner[1].Pos()[l] + t2*corner[2].Pos()[l];
      cur[ind] = ret->NewVertex(ip);
      ++ind;
     }
     ++ind;
    }


   // Iterate the triangular structure and create all the faces...
    ind = 0;
    for (nat32 j=0;j<totalDivs;j++)
    {
     ind += j;
     // Do the down triangles from this row...
      if (j!=(totalDivs-1))
      {
       for (nat32 k=0;k<=j;k++)
       {
        ret->NewFace(cur[ind+k],cur[ind+k+j+1],cur[ind+k+j+2]);
       }
      }

     // Do the up triangles from this row...
      if (j!=0)
      {
       for (nat32 k=1;k<j;k++)
       {
        ret->NewFace(cur[ind+k],cur[ind+k-j],cur[ind+k-j-1]);
       }
      }
    }
  }


 return ret;
}

//------------------------------------------------------------------------------
 };
};
