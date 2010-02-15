//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

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

#include "eos/alg/shapes.h"

#include "eos/mem/safety.h"
#include "eos/math/mat_ops.h"
#include "eos/sur/subdivide.h"

namespace eos
{
 namespace alg
 {
//------------------------------------------------------------------------------
static const real32 nc = 1.902113032590307144232878666759; // sqrt(1+((1+sqrt(5))/2)^2)

const real32 Icosahedron::vert[12][3] = {{0.0,1.0/nc,math::phi/nc},
                                         {0.0,-1.0/nc,math::phi/nc},
                                         {0.0,1.0/nc,-math::phi/nc},
                                         {0.0,-1.0/nc,-math::phi/nc},
                                         {1.0/nc,math::phi/nc,0.0},
                                         {-1.0/nc,math::phi/nc,0.0},
                                         {1.0/nc,-math::phi/nc,0.0},
                                         {-1.0/nc,-math::phi/nc,0.0},
                                         {math::phi/nc,0.0,1.0/nc},
                                         {math::phi/nc,0.0,-1.0/nc},
                                         {-math::phi/nc,0.0,1.0/nc},
                                         {-math::phi/nc,0.0,-1.0/nc}
                                        };

const nat32 Icosahedron::edge[30][2] = {{0,1},{0,4},{0,5},{0,8},{0,10},
                                        {1,6},{1,7},{1,8},{1,10},
                                        {2,3},{2,4},{2,5},{2,9},{2,11},
                                        {3,6},{3,7},{3,9},{3,11},
                                        {4,5},{4,8},{4,9},
                                        {5,10},{5,11},
                                        {6,7},{6,8},{6,9},
                                        {7,10},{7,11},
                                        {8,9},
                                        {10,11},
                                       };

const nat32 Icosahedron::tri[20][3] = {{0,1,8},
                                       {0,10,1},
                                       {2,3,11},
                                       {2,9,3},
                                       {4,5,0},
                                       {4,2,5},
                                       {6,7,3},
                                       {6,1,7},
                                       {8,9,4},
                                       {8,6,9},
                                       {10,11,7},
                                       {10,5,11},
                                       {8,4,0},
                                       {0,5,10},
                                       {10,7,1},
                                       {1,6,8},
                                       {2,11,5},
                                       {11,3,7},
                                       {3,9,6},
                                       {2,4,9}
                                      };

//------------------------------------------------------------------------------
nat32 Icosphere::SubToVert(nat32 subdivs)
{
 nat32 perFace = math::Triangular(subdivs+2);
 nat32 ret = perFace * 20;
  ret -= subdivs*30;
  ret -= 12*4;
 return ret;
}

nat32 Icosphere::SubToEdge(nat32 subdivs)
{
 nat32 perFace = math::Triangular(subdivs+1)*3;
 nat32 ret = perFace * 20;
  ret -= 30*(subdivs+1);
 return ret;
}
   
nat32 Icosphere::SubToTri(nat32 subdivs)
{
 nat32 perFace = math::Sqr(subdivs+1);
 return perFace * 20;
}

real32 Icosphere::SubToAng(nat32 subdivs)
{
 real32 baseAng = math::InvCos((1.0+math::Sqrt(5.0))/(5+math::Sqrt(5.0)));
 return baseAng/real32(subdivs+1);
}

Icosphere::Icosphere(nat32 subdivs)
{
 // First create a mesh object that contains an icosahedron...
  sur::Mesh base;
  sur::Vertex baseVert[12];
  for (nat32 i=0;i<12;i++) baseVert[i] = base.NewVertex(bs::Vert(Icosahedron::vert[i][0],
                                                                 Icosahedron::vert[i][1],
                                                                 Icosahedron::vert[i][2]));
  
  for (nat32 i=0;i<20;i++) base.NewFace(baseVert[Icosahedron::tri[i][0]],
                                        baseVert[Icosahedron::tri[i][1]],
                                        baseVert[Icosahedron::tri[i][2]]);


 // Subdivide the mesh the requisit number of times...
  mem::StackPtr<sur::Mesh> sdm(sur::Subdivide(base,subdivs));


 // Create storage for this final mesh in the stored form of this object...
  ds::Array<sur::Vertex> tempVert;
  ds::Array<sur::Edge> tempEdge;
  ds::Array<sur::Face> tempTri;

  sdm->GetVertices(tempVert);
  sdm->GetEdges(tempEdge);
  sdm->GetFaces(tempTri);
  
  verts = tempVert.Size();
  edges = tempEdge.Size();
  tris = tempTri.Size();

  vert = new real32[3*verts];
  edge = new nat32[2*edges];
  tri = new nat32[3*tris];


 // Extract the subdivided icosphere, pushing the vertices back to there unit
 // sphere positions...
  for (nat32 i=0;i<verts;i++)
  {
   tempVert[i].Pos().Normalise();
   vert[i*3+0] = tempVert[i].Pos()[0];
   vert[i*3+1] = tempVert[i].Pos()[1];
   vert[i*3+2] = tempVert[i].Pos()[2];
  }

  for (nat32 i=0;i<edges;i++)
  {
   edge[i*2+0] = tempEdge[i].VertexA().Index();
   edge[i*2+1] = tempEdge[i].VertexB().Index();
  }
  
  ds::Array<sur::Vertex> corner(3);
  for (nat32 i=0;i<tris;i++)
  {
   tempTri[i].GetVertices(corner);
   tri[i*3+0] = corner[0].Index();
   tri[i*3+1] = corner[1].Index();
   tri[i*3+2] = corner[2].Index();
  }
}

Icosphere::~Icosphere()
{
 delete[] vert;
 delete[] edge;
 delete[] tri;
}

//------------------------------------------------------------------------------
HemiOfNorm::HemiOfNorm(nat32 subdivs)
:norms(0),norm(null<bs::Normal*>()),spacing(Icosphere::SubToAng(subdivs))
{
 Icosphere ico(subdivs);
 for (nat32 i=0;i<ico.Verts();i++)
 {
  if (ico.Vert(i)[2]>=0.0) ++norms;
 }
 
 norm = new bs::Normal[norms];
 norms = 0;
 for (nat32 i=0;i<ico.Verts();i++)
 {
  if (ico.Vert(i)[2]>=0.0)
  {
   norm[norms] = bs::Normal(ico.Vert(i)[0],ico.Vert(i)[1],ico.Vert(i)[2]);
   ++norms;
  }
 }
}

HemiOfNorm::HemiOfNorm(const HemiOfNorm & rhs)
{
 norms = rhs.norms;
 norm = new bs::Normal[norms];
 for (nat32 i=0;i<norms;i++) norm[i] = rhs.norm[i];
 spacing = rhs.spacing;
}

HemiOfNorm::~HemiOfNorm()
{
 delete[] norm;
}

HemiOfNorm & HemiOfNorm::operator = (const HemiOfNorm & rhs)
{
 norms = rhs.norms;
 
 delete[] norm;
 norm = new bs::Normal[norms];

 for (nat32 i=0;i<norms;i++) norm[i] = rhs.norm[i];
 
 spacing = rhs.spacing;

 return *this;
}

//------------------------------------------------------------------------------
 };
};
