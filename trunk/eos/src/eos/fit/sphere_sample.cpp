//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines
#include "eos/fit/sphere_sample.h"

namespace eos
{
 namespace fit
 {
//------------------------------------------------------------------------------
SubDivSphere::SubDivSphere()
:verts(12,12),tris(20,40)
{
 const real32 nc = math::Sqrt(math::Sqr(1.0+((1.0+math::Sqrt(5.0))/2.0)));
 const real32 innc = 1.0/nc;
 const real32 pnc = math::phi/nc;
 
 verts[0]  = bs::Normal(0.0,innc,pnc);
 verts[1]  = bs::Normal(0.0,-innc,pnc);
 verts[2]  = bs::Normal(0.0,innc,-pnc);
 verts[3]  = bs::Normal(0.0,-innc,-pnc);
 verts[4]  = bs::Normal(innc,pnc,0.0);
 verts[5]  = bs::Normal(-innc,pnc,0.0);
 verts[6]  = bs::Normal(innc,-pnc,0.0);
 verts[7]  = bs::Normal(-innc,-pnc,0.0);
 verts[8]  = bs::Normal(pnc,0.0,innc);
 verts[9]  = bs::Normal(pnc,0.0,-innc);
 verts[10] = bs::Normal(-pnc,0.0,innc);
 verts[11] = bs::Normal(-pnc,0.0,-innc);
 
 for (nat32 i=0;i<tris.Size();i++)
 {
  tris[i].parent = nat32(-1);
  tris[i].child = nat32(-1);
 }
 
 tris[0].vertInd[0]  =  0; tris[0].vertInd[1]  =  1; tris[0].vertInd[2]  =  8;
 tris[1].vertInd[0]  =  0; tris[1].vertInd[1]  = 10; tris[1].vertInd[2]  =  1;
 tris[2].vertInd[0]  =  2; tris[2].vertInd[1]  =  3; tris[2].vertInd[2]  = 11;
 tris[3].vertInd[0]  =  2; tris[3].vertInd[1]  =  9; tris[3].vertInd[2]  =  3;
 tris[4].vertInd[0]  =  4; tris[4].vertInd[1]  =  5; tris[4].vertInd[2]  =  0;
 tris[5].vertInd[0]  =  4; tris[5].vertInd[1]  =  2; tris[5].vertInd[2]  =  5;
 tris[6].vertInd[0]  =  6; tris[6].vertInd[1]  =  7; tris[6].vertInd[2]  =  3;
 tris[7].vertInd[0]  =  6; tris[6].vertInd[1]  =  1; tris[7].vertInd[2]  =  7;
 tris[8].vertInd[0]  =  8; tris[7].vertInd[1]  =  9; tris[8].vertInd[2]  =  4;
 tris[9].vertInd[0]  =  8; tris[8].vertInd[1]  =  6; tris[9].vertInd[2]  =  9;
 tris[10].vertInd[0] = 10; tris[10].vertInd[1] = 11; tris[10].vertInd[2] =  7;
 tris[11].vertInd[0] = 10; tris[11].vertInd[1] =  5; tris[11].vertInd[2] = 11;
 tris[12].vertInd[0] =  8; tris[12].vertInd[1] =  4; tris[12].vertInd[2] =  0;
 tris[13].vertInd[0] =  0; tris[13].vertInd[1] =  5; tris[13].vertInd[2] = 10;
 tris[14].vertInd[0] = 10; tris[14].vertInd[1] =  7; tris[14].vertInd[2] =  1;
 tris[15].vertInd[0] =  1; tris[15].vertInd[1] =  6; tris[15].vertInd[2] =  8;
 tris[16].vertInd[0] =  2; tris[16].vertInd[1] = 11; tris[16].vertInd[2] =  5;
 tris[17].vertInd[0] = 11; tris[17].vertInd[1] =  3; tris[17].vertInd[2] =  7;
 tris[18].vertInd[0] =  3; tris[18].vertInd[1] =  9; tris[18].vertInd[2] =  6;
 tris[19].vertInd[0] =  2; tris[19].vertInd[1] =  4; tris[19].vertInd[2] =  9;

 tris[0].adj[0]  = 15; tris[0].adj[1]  = 12; tris[0].adj[2]  =  1;
 tris[1].adj[0]  = 14; tris[1].adj[1]  =  0; tris[1].adj[2]  = 13;
 tris[2].adj[0]  = 17; tris[2].adj[1]  = 16; tris[2].adj[2]  =  3;
 tris[3].adj[0]  = 18; tris[3].adj[1]  =  2; tris[3].adj[2]  = 19;
 tris[4].adj[0]  = 13; tris[4].adj[1]  = 12; tris[4].adj[2]  =  5;
 tris[5].adj[0]  = 16; tris[5].adj[1]  =  4; tris[5].adj[2]  = 19;
 tris[6].adj[0]  = 17; tris[6].adj[1]  = 18; tris[6].adj[2]  =  7;
 tris[7].adj[0]  = 14; tris[7].adj[1]  =  6; tris[7].adj[2]  = 15;
 tris[8].adj[0]  = 19; tris[8].adj[1]  = 12; tris[8].adj[2]  =  9;
 tris[9].adj[0]  = 18; tris[9].adj[1]  =  8; tris[9].adj[2]  = 15;
 tris[10].adj[0] = 17; tris[10].adj[1] = 14; tris[10].adj[2] = 11;
 tris[11].adj[0] = 16; tris[11].adj[1] = 10; tris[11].adj[2] = 13;
 tris[12].adj[0] =  4; tris[12].adj[1] =  0; tris[12].adj[2] =  8;
 tris[13].adj[0] = 11; tris[13].adj[1] =  1; tris[13].adj[2] =  4;
 tris[14].adj[0] =  7; tris[14].adj[1] =  1; tris[14].adj[2] = 10;
 tris[15].adj[0] =  9; tris[15].adj[1] =  0; tris[15].adj[2] =  7;
 tris[16].adj[0] = 11; tris[16].adj[1] =  5; tris[16].adj[2] =  2;
 tris[17].adj[0] =  6; tris[17].adj[1] = 10; tris[17].adj[2] =  2;
 tris[18].adj[0] =  9; tris[18].adj[1] =  6; tris[18].adj[2] =  3;
 tris[19].adj[0] =  8; tris[19].adj[1] =  3; tris[19].adj[2] =  5;


 // The below code is purly to check the above constructed data structure is
 // correct, as a mistake would be vicous...
 // (To be comented out after first run.)
  for (nat32 i=0;i<tris.Size();i++)
  {
   for (nat32 j=0;j<3;j++)
   {
    nat32 other = tris[i].adj[(j+2)%3];
    bit ok = false;
    for (nat32 k=0;k<3;k++)
    {
     if (tris[other].vertInd[k]==tris[i].vertInd[j])
     {
      ok = tris[other].vertInd[(k+2)%3]==tris[i].vertInd[(j+1)%3];
     }
    }
    log::Assert(ok);
   }
  }
}

SubDivSphere::~SubDivSphere()
{}


void SubDivSphere::SubDivide(Tri tri)
{
 if (tris[tri].child==nat32(-1)) return;

 // First create the 4 relevant triangles - first entry is the middle...
  nat32 base = tris.Size();
  tris.Size(tris.Size()+4);
  for (nat32 i=0;i<4;i++)
  {
   tris[base+i].parent = tri;
   tris[base+i].child = nat32(-1);
  }
  tris[tri].child = base;
 
 
 // Find/create the 3 reused/new vertices - fiddly...
  nat32 midVert[3]; // Indexed 0 for vertex in middle of edge a etc.
  for (nat32 i=0;i<3;i++)
  {
   // Attempt to find and reuse...
    midVert[i] = nat32(-1);
    if (tris[tri].adj[i]!=nat32(-1))
    {
     nat32 adj = tris[tri].adj[i];
     if (tris[adj].child!=nat32(-1))
     {
      nat32 matchAdj = (tris[adj].adj[0]==tri)?0:((tris[adj].adj[1]==tri)?1:2);
      midVert[i] = tris[tris[adj].child].vertInd[matchAdj];
     }
    }
   
   // If not found create new vertex...
    if (midVert[i]==nat32(-1))
    {
     midVert[i] = verts.Size();
     verts.Size(verts.Size()+1);
    
     verts[midVert[i]]  = verts[tris[tri].vertInd[(i+1)%3]];
     verts[midVert[i]] += verts[tris[tri].vertInd[(i+2)%3]];
     verts[midVert[i]].Normalise();
    }
  }


 // Fill in all the vertex indices for the 4 triangles...
  
 
 // Stitch up the simple connections...
 
 
 // Now check the other connections and see if the partners exist and can be
 // connected to - this is the complicated bit...
  
 
 
}

//------------------------------------------------------------------------------
 };
};
