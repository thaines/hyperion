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

#include "eos/ds/voronoi.h"

#include "eos/mem/alloc.h"
#include "eos/mem/functions.h"

#include "eos/file/files.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
Delaunay2DCode::Delaunay2DCode()
:first(null<Site*>()),sites(0),triangles(1),oldTriangles(0)
{
 root.children = 0;
 for (nat32 i=0;i<3;i++)
 {
  root.vert[i] = mem::Malloc<Site>();
  root.partner[i] = null<Triangle*>();
  root.next[i] = null<Triangle*>();

  root.vert[i]->infinite = true;
  root.vert[i]->neighbours = 1;
  root.vert[i]->tri = &root;
 }

 root.vert[0]->x = 1.0;
 root.vert[0]->y = 0.0;
 root.vert[1]->x = -1.0;
 root.vert[1]->y = 1.0;
 root.vert[2]->x = -1.0;
 root.vert[2]->y = -1.0;
}

Delaunay2DCode::~Delaunay2DCode()
{
 for (nat32 i=0;i<3;i++) mem::Free(root.vert[i]);
}

void Delaunay2DCode::Del(void (*Term)(void * ptr))
{
 root.DelChildren();
 root.children = 0;
 for (nat32 i=0;i<3;i++) root.next[i] = null<Triangle*>();

 while (first)
 {
  Site * victim = first;
  first = first->next;

  Term(victim->Data());
  mem::Free(victim);
 }
}

void Delaunay2DCode::Reset(void (*Term)(void * ptr))
{
 Del(Term);

 first = null<Site*>();
 sites = 0;
 triangles = 1;
 oldTriangles = 0;

 root.children = 0;
 for (nat32 i=0;i<3;i++)
 {
  root.vert[i] = mem::Malloc<Site>();
  root.partner[i] = null<Triangle*>();
  root.next[i] = null<Triangle*>();

  root.vert[i]->infinite = true;
  root.vert[i]->neighbours = 1;
  root.vert[i]->tri = &root;
 }

 root.vert[0]->x = 1.0;
 root.vert[0]->y = 0.0;
 root.vert[1]->x = -1.0;
 root.vert[1]->y = 1.0;
 root.vert[2]->x = -1.0;
 root.vert[2]->y = -1.0;
}

Delaunay2DCode::Site * Delaunay2DCode::Add(nat32 elementSize,real64 x,real64 y,const void * ptr)
{
 // Make the new site to be added...
  Site * ns = mem::Malloc<Site>(sizeof(Site) + elementSize);
   ns->next = first; first = ns;
   ns->x = x;
   ns->y = y;
   ns->neighbours = 0;
   ns->tri = null<Triangle*>();
   ns->infinite = false;

   mem::Copy<byte>((byte*)ns->Data(),(byte*)ptr,elementSize);

 // Find the triangle to add it into...
  Triangle * into = root.FindTri(x,y);

 // Add it in by creating 3 more triangles as children...
  into->children = 3;
  into->delChildren = true;
  for (nat32 j=0;j<3;j++) into->child[j] = new Triangle();

  for (nat32 j=0;j<3;j++)
  {
   into->vert[j]->neighbours -= 1;

   for (nat32 i=0;i<3;i++)
   {
    if ((into->partner[j])&&(into->partner[j]->partner[i]==into))
    {
     into->partner[j]->partner[i] = into->child[j];
     break;
    }
   }

   into->child[j]->vert[0] = into->vert[j];
   into->child[j]->vert[1] = into->vert[(j+1)%3];
   into->child[j]->vert[2] = ns;
   into->child[j]->children = 0;
   into->child[j]->delChildren = false;
   into->child[j]->partner[0] = into->partner[j];
   into->child[j]->partner[1] = into->child[(j+1)%3];
   into->child[j]->partner[2] = into->child[(j+2)%3];

   for (nat32 i=0;i<3;i++)
   {
    into->child[j]->next[i] = into->child[j]->vert[i]->tri;
    into->child[j]->vert[i]->tri = into->child[j];
    into->child[j]->vert[i]->neighbours += 1;
   }
  }

  ++sites;
  triangles += 2;
  oldTriangles += 1;


 // Correct the structure by swapping triangles arround, except
 // we create new triangles to maintain the search tree...
  for (nat32 i=0;i<3;i++)
  {
   into->child[i]->MabyFlip(this,2);
  }

 // Testing code, used for finding those dam bugs, prints out the hierachy of the
 // triangle tree...
  //LogAlways("Triangle Hierachy:");
  //root.LogHierachy();

 return ns;
}

Delaunay2DCode::Site * Delaunay2DCode::Nearest(real64 x,real64 y) const
{
 Triangle * targ = root.FindTri(x,y);

 Site * ret = null<Site*>();
 real64 dist = 0.0;
  for (nat32 i=0;i<3;i++)
  {
   if (targ->vert[i]->infinite) continue;
   real64 nDist = math::Sqr(x-targ->vert[i]->x) + math::Sqr(y-targ->vert[i]->y);
   if ((ret==null<Site*>())||(nDist<dist))
   {
    ret = targ->vert[i];
    dist = nDist;
   }
  }
 return ret;
}

void Delaunay2DCode::Triangle::DelChildren()
{
 if (delChildren)
 {
  for (nat32 i=0;i<children;i++)
  {
   child[i]->DelChildren();
   delete child[i];
  }
 }
}

Delaunay2DCode::Triangle * Delaunay2DCode::Triangle::FindTri(real64 x,real64 y) const
{
 if (children==0) return const_cast<Triangle*>(this);
 if (children==3)
 {
  // 3 children - find the dividing vertex and check the 3 lines comming out of it to the
  // 3 vertexes of the triangle. This code is tightly coupled to the construction code...
   Site * centre = child[0]->vert[2];
   bit check0 = Side(centre,vert[0],x,y);
   bit check1 = Side(centre,vert[1],x,y);
   bit check2 = Side(centre,vert[2],x,y);

   if (check0)
   {
    if (check1) return child[1]->FindTri(x,y);
           else return child[0]->FindTri(x,y);
   }
   else
   {
    if (check2) return child[2]->FindTri(x,y);
           else return child[1]->FindTri(x,y);
   }
 }
 else
 {
  // 2 children (Presumably) - find the dividing vertex and check which side of the line we are on...
  // This code is tightly coupled to the MabyFlip code.
   if (Side(child[0]->vert[0],child[0]->vert[1],x,y)) return child[0]->FindTri(x,y);
                                                 else return child[1]->FindTri(x,y);
 }
}

void Delaunay2DCode::Triangle::MabyFlip(Delaunay2DCode * parent,nat32 ind)
{
 // If we have children we do nothing - its allready been factored
 // in by a previous call... (I don't think this is actually possible,
 // but better safe than sorry for such a minor test.)
  //log::Assert(children==0); // *************************************************
  if (children!=0) return;


 // Get the second triangle, the one we are considering flipping with,
 // and the partner indexes which point back...
  nat32 thisToOther = (ind+1)%3;
  Triangle * other = partner[thisToOther];
  if (other==null<Triangle*>()) return; // An infinite triangle edge has been found - no flipping here.
  nat32 otherToThis = 0;
  for (nat32 i=0;i<3;i++) {if (other->partner[i]==this) {otherToThis = i; break;}}
  if (other->vert[(otherToThis+2)%3]->infinite) return; // Can't go swapping with infinitly distant points.


 // Check if we actualy need to do a flip, if not we simply return...
 // If either vertex on the line is infinite we have to do a different check.
  if ((!other->vert[otherToThis]->infinite)&&(!other->vert[(otherToThis+1)%3]->infinite))
  {
   math::Mat<4,4> ftm;
    ftm[0][0] = math::Sqr(vert[ind]->x)      + math::Sqr(vert[ind]->y);      ftm[0][1] = vert[ind]->x;      ftm[0][2] = vert[ind]->y;       ftm[0][3] = 1.0;
    ftm[1][0] = math::Sqr(other->vert[0]->x) + math::Sqr(other->vert[0]->y); ftm[1][1] = other->vert[0]->x; ftm[1][2] = other->vert[0]->y;  ftm[1][3] = 1.0;
    ftm[2][0] = math::Sqr(other->vert[1]->x) + math::Sqr(other->vert[1]->y); ftm[2][1] = other->vert[1]->x; ftm[2][2] = other->vert[1]->y;  ftm[2][3] = 1.0;
    ftm[3][0] = math::Sqr(other->vert[2]->x) + math::Sqr(other->vert[2]->y); ftm[3][1] = other->vert[2]->x; ftm[3][2] = other->vert[2]->y;  ftm[3][3] = 1.0;

   if (Determinant(ftm)>=0.0) return;
  }
  else
  {
   // We need to check the side of the line which our point is on, if
   // its the wrong side we return...
    if (vert[thisToOther]->infinite)
    {
     if (Side(other->vert[(otherToThis+2)%3],
              vert[ind],
              other->vert[otherToThis]->x,
              other->vert[otherToThis]->y)) return;

    }
    else
    {
     if (Side(vert[ind],
              other->vert[(otherToThis+2)%3],
              vert[thisToOther]->x,
              vert[thisToOther]->y)) return;
    }
  }


 // Do the flip - 'simply' construct two new triangles and tie them in...
  children = 2;
  delChildren = true;
  other->children = 2;
  child[0] = new Triangle(); other->child[0] = child[0];
  child[1] = new Triangle(); other->child[1] = child[1];

  for (nat32 i=0;i<3;i++)
  {
   vert[i]->neighbours -= 1;
   other->vert[i]->neighbours -= 1;
  }

  child[0]->vert[0] = vert[ind];
  child[0]->vert[1] = other->vert[(otherToThis+2)%3];
  child[0]->vert[2] = vert[(thisToOther+1)%3];
  child[0]->children = 0;
  child[0]->delChildren = false;
  child[0]->partner[0] = child[1];
  child[0]->partner[1] = other->partner[(otherToThis+2)%3];
  child[0]->partner[2] = partner[(thisToOther+1)%3];

  child[1]->vert[0] = other->vert[(otherToThis+2)%3];
  child[1]->vert[1] = vert[ind];
  child[1]->vert[2] = vert[thisToOther];
  child[1]->children = 0;
  child[1]->delChildren = false;
  child[1]->partner[0] = child[0];
  child[1]->partner[1] = partner[ind];
  child[1]->partner[2] = other->partner[(otherToThis+1)%3];

  for (nat32 i=0;i<3;i++)
  {
   if ((child[0]->partner[2])&&(child[0]->partner[2]->partner[i]==this)) child[0]->partner[2]->partner[i] = child[0];
   if ((child[1]->partner[1])&&(child[1]->partner[1]->partner[i]==this)) child[1]->partner[1]->partner[i] = child[1];

   if ((child[0]->partner[1])&&(child[0]->partner[1]->partner[i]==other)) child[0]->partner[1]->partner[i] = child[0];
   if ((child[1]->partner[2])&&(child[1]->partner[2]->partner[i]==other)) child[1]->partner[2]->partner[i] = child[1];
  }

  for (nat32 i=0;i<3;i++)
  {
   child[0]->next[i] = child[0]->vert[i]->tri;
   child[0]->vert[i]->tri = child[0];
   child[0]->vert[i]->neighbours += 1;

   child[1]->next[i] = child[1]->vert[i]->tri;
   child[1]->vert[i]->tri = child[1];
   child[1]->vert[i]->neighbours += 1;
  }

  parent->oldTriangles += 2;


 // Recurse and check the two new potential edges that could need
 // a good hard flipping...
  child[0]->MabyFlip(parent,0);
  child[1]->MabyFlip(parent,1);
}

bit Delaunay2DCode::Triangle::Side(Site * a,Site * b,real64 x,real64 y)
{
 log::Assert(!a->infinite,"eos::ds::Delaunay2DCode::Triangle::Side");
 if (b->infinite) return ((b->x)*(y-a->y) - (b->y)*(x-a->x)) > 0.0;
             else return ((b->x-a->x)*(y-a->y) - (b->y-a->y)*(x-a->x)) > 0.0;
}

//------------------------------------------------------------------------------
 };
};
