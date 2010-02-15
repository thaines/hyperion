//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines
#include "eos/fit/sphere_sample.h"

namespace eos
{
 namespace fit
 {
//------------------------------------------------------------------------------
SubDivSphere::SubDivSphere()
:tris(20,40),verts(12,12)
{
 const real32 nc = math::Sqrt(1.0 + math::Sqr((1.0 + math::Sqrt(5.0))/2.0));
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
 tris[7].vertInd[0]  =  6; tris[7].vertInd[1]  =  1; tris[7].vertInd[2]  =  7;
 tris[8].vertInd[0]  =  8; tris[8].vertInd[1]  =  9; tris[8].vertInd[2]  =  4;
 tris[9].vertInd[0]  =  8; tris[9].vertInd[1]  =  6; tris[9].vertInd[2]  =  9;
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
  /*for (nat32 i=0;i<tris.Size();i++)
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
  }*/
}

SubDivSphere::~SubDivSphere()
{}

real32 SubDivSphere::Area(Tri tri) const
{
 bs::Normal dirA = verts[tris[tri].vertInd[1]]; dirA -= verts[tris[tri].vertInd[0]];
 bs::Normal dirB = verts[tris[tri].vertInd[2]]; dirB -= verts[tris[tri].vertInd[0]];
 
 bs::Normal cp;
 math::CrossProduct(dirA,dirB,cp);
 return cp.Length() * 0.5;
}

void SubDivSphere::DivideAll(nat32 levels)
{
 if (levels==0) return;
 for (nat32 i=0;i<20;i++) RecSubDivide(i,levels);
}

SubDivSphere::Tri SubDivSphere::Collide(const bs::Normal & dir) const
{
 // Helper arrays that tie in with the first 20 face - gives there x/y/z
 // assignment as a quick way of excluding certain faces from collision
 // detection.
 // Each one consists of -1,0 and 1 values for each axis. 0 indicates the face
 // has both directions, -1 only negative values of that axis, 1 only positive
 // values of that axis.
  static const int8 xMask[20] = {1,-1,-1, 1, 0, 0, 0, 0, 1, 1,-1,-1, 1,-1,-1, 1,-1,-1, 1, 1};
  static const int8 yMask[20] = {0, 0, 0, 0, 1, 1,-1,-1, 1,-1,-1, 1, 1, 1,-1,-1, 1,-1,-1, 1};
  static const int8 zMask[20] = {1, 1,-1,-1, 1,-1,-1, 1, 0, 0, 0, 0, 1, 1, 1, 1,-1,-1,-1,-1};


 // Find the top level face that this direction collides with...
  // Calculate the flags to use with the above data structure...
   int8 dX = int8(math::Sign(dir[0]));
   int8 dY = int8(math::Sign(dir[1]));
   int8 dZ = int8(math::Sign(dir[2]));
  
  // Use flags to ignore most of the test faces, only checking against valid
  // once. This code is designed to handle boundary cases so samples can't fall
  // between the cracks...
   Tri ret = nat32(-1);
   real32 bestDist = math::Infinity<real32>();
   for (nat32 i=0;i<20;i++)
   {
    // Test flags...
     if (dX*xMask[i]<0) continue;
     if (dY*yMask[i]<0) continue;
     if (dZ*zMask[i]<0) continue;
     
    // Its a possible match - do a full check, but produce a distance that 
    // should be zero on a match and select the smallest - this makes cracks a 
    // non-issue (With early break out if all are positive)...
     real32 distA = UnnormDist(dir,verts[tris[i].vertInd[1]],verts[tris[i].vertInd[2]]);
     real32 distB = UnnormDist(dir,verts[tris[i].vertInd[2]],verts[tris[i].vertInd[0]]);
     real32 distC = UnnormDist(dir,verts[tris[i].vertInd[3]],verts[tris[i].vertInd[1]]);
     
     if ((distA>0.0)&&(distB>0.0)&&(distC>0.0))
     {
      ret = i;
      break;
     }
     else
     {
      real32 dist = math::Max<real32>(-distA,0.0) + 
                    math::Max<real32>(-distB,0.0) + 
                    math::Max<real32>(-distC,0.0);
      if (dist<bestDist)
      {
       bestDist = dist;
       ret = i;
      }
     }
   }


 // Recurse down until we reach a childless face...
  while (tris[ret].child!=nat32(-1))
  {
   Tri c = tris[ret].child;
   real32 distA = UnnormDist(dir,verts[tris[c].vertInd[1]],verts[tris[c].vertInd[2]]);
   real32 distB = UnnormDist(dir,verts[tris[c].vertInd[2]],verts[tris[c].vertInd[0]]);
   real32 distC = UnnormDist(dir,verts[tris[c].vertInd[3]],verts[tris[c].vertInd[1]]);
   
   if (distA<0.0) ret = tris[c].adj[0];
   else
   {
    if (distB<0.0) ret = tris[c].adj[1];
    else
    {
     if (distC<0.0) ret = tris[c].adj[2];
               else ret = c;
    }
   }
  }
 
 
 return ret;
}

void SubDivSphere::Trilinear(Tri tri,const bs::Normal & dir,real32 & a,real32 & b,real32 & c) const
{
 a = NormDist(dir,verts[tris[tri].vertInd[1]],verts[tris[tri].vertInd[2]]);
 b = NormDist(dir,verts[tris[tri].vertInd[2]],verts[tris[tri].vertInd[0]]);
 c = NormDist(dir,verts[tris[tri].vertInd[3]],verts[tris[tri].vertInd[1]]);
}

void SubDivSphere::MakeExist(Tri tri,nat32 a)
{
 if (tris[tri].adj[a]!=nat32(-1)) return;
 
 if (tris[tris[tri].parent].adj[a]==nat32(-1)) MakeExist(tris[tri].parent,a);
 SubDivide(tris[tris[tri].parent].adj[a]);
 
 log::Assert(tris[tri].adj[a]!=nat32(-1));
}

void SubDivSphere::SubDivide(Tri tri)
{
 if (tris[tri].child!=nat32(-1)) return;

 // First create the 4 relevant triangles - first entry is the middle, then adj to A,B,C...
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
  for (nat32 i=0;i<3;i++) tris[base].vertInd[i] = midVert[i];

  tris[base+1].vertInd[0] = tris[tri].vertInd[0];
  tris[base+1].vertInd[1] = midVert[2];
  tris[base+1].vertInd[2] = midVert[1];
  
  tris[base+2].vertInd[0] = midVert[2];
  tris[base+2].vertInd[1] = tris[tri].vertInd[1];
  tris[base+2].vertInd[2] = midVert[0];
  
  tris[base+3].vertInd[0] = midVert[1];
  tris[base+3].vertInd[1] = midVert[0];
  tris[base+3].vertInd[2] = tris[tri].vertInd[2];

   
 // Stitch up the simple connections...
  for (nat32 i=0;i<3;i++)
  {
   tris[base].adj[i] = base+1+i;
   tris[base+1+i].adj[i] = base;
  }


 // Now check the other connections and see if the partners exist and can be
 // connected to - this is the complicated bit...
  for (nat32 i=0;i<3;i++) // i is the edge of tri we are currently processing.
  {
   if ((tris[tri].adj[i]!=nat32(-1))&&(tris[tris[tri].adj[i]].child!=nat32(-1)))
   {
    // We have an adjacent subdivision - we need to link up...
     Tri adj = tris[tri].adj[i];
     nat32 adjI = (tris[adj].adj[0]==tri)?0:((tris[adj].adj[1]==tri)?1:2);

     Tri thisU = tris[tris[tri].child].adj[(i+1)%3];
     Tri thisV = tris[tris[tri].child].adj[(i+2)%3];
     
     Tri otherU = tris[tris[adj].child].adj[(adjI+1)%3];
     Tri otherV = tris[tris[adj].child].adj[(adjI+2)%3];
     
     tris[thisU].adj[i] = otherV;
     tris[thisV].adj[i] = otherU;

     tris[otherU].adj[adjI] = thisV;
     tris[otherV].adj[adjI] = thisU;    
   }
   else
   {
    // No subdivision - simply set 'em to be null pointers...
     for (nat32 j=0;j<3;j++)
     {
      if (j!=i) tris[tris[tris[tri].child].adj[j]].adj[i] = nat32(-1);
     }
   }
  }
}

void SubDivSphere::RecSubDivide(Tri tri,nat32 levels)
{
 if (levels==0) return;
 
 SubDivide(tri);
 if (levels==1) return;

 RecSubDivide(tris[tri].child,levels-1);
 RecSubDivide(tris[tris[tri].child].adj[0],levels-1);
 RecSubDivide(tris[tris[tri].child].adj[1],levels-1);
 RecSubDivide(tris[tris[tri].child].adj[2],levels-1);
}

//------------------------------------------------------------------------------
 };
};
