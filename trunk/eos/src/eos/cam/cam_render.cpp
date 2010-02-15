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

#include "eos/cam/cam_render.h"

#include "eos/math/mat_ops.h"
#include "eos/cam/triangulation.h"
#include "eos/sur/intersection.h"

namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------
void EOS_FUNC CamToRays(const CameraFull & cam,ds::Array2D<bs::Ray> & rays,time::Progress * prog)
{
 rays.Resize(nat32(cam.dim[0]),nat32(cam.dim[1]));

 math::Vect<4,real64> centre;
 cam.camera.Centre(centre);
 centre /= centre[3];

 math::Mat<4,3,real64> invCam;
 cam.camera.GetInverse(invCam);

 for (nat32 y=0;y<rays.Height();y++)
 {
  prog->Report(y,rays.Height());
  for (nat32 x=0;x<rays.Width();x++)
  {
   // Centre is constant...
    for (nat32 i=0;i<3;i++) rays.Get(x,y).s[i] = centre[i];

   // Undistort the position...
    math::Vect<3,real64> dis;
    dis[0] = real32(x);
    dis[1] = real32(y);
    dis[2] = 1.0;

    math::Vect<3,real64> undis;
    cam.radial.UnDis(dis,undis);

   // Multiply by the inverse projection matrix...
    math::Vect<4,real64> loc;
    math::MultVect(invCam,undis,loc);
    loc /= loc[3];

   // Change into a direction and store, then normalise...
    for (nat32 i=0;i<3;i++) rays.Get(x,y).n[i] = loc[i] - centre[i];
    rays.Get(x,y).n.Normalise();
  }
 }
}

void EOS_FUNC RenderTri(const CameraFull & cam,const ds::Array2D<bs::Ray> & rays,
                        svt::Field<bs::ColourRGB> & image,
                        ds::Array2D<real32> & depth,
                        const bs::Vert & a,const bs::ColourRGB & ac,
                        const bs::Vert & b,const bs::ColourRGB & bc,
                        const bs::Vert & c,const bs::ColourRGB & cc)
{
 // Calculate the projected positions of the 3 corners, distort accordingly...
  math::Vect<2,real64> pos[3];
  math::MultVectEH(cam.camera,a,pos[0]); cam.radial.Dis(pos[0],pos[0]);
  math::MultVectEH(cam.camera,b,pos[1]); cam.radial.Dis(pos[1],pos[1]);
  math::MultVectEH(cam.camera,c,pos[2]); cam.radial.Dis(pos[2],pos[2]);


 // Calculate the region of space covered by the corners...
  int32 minX = int32(math::RoundDown(math::Min(pos[0][0],pos[1][0],pos[2][0])));
  int32 maxX = int32(math::RoundUp(math::Max(pos[0][0],pos[1][0],pos[2][0])));
  int32 minY = int32(math::RoundDown(math::Min(pos[0][1],pos[1][1],pos[2][1])));
  int32 maxY = int32(math::RoundUp(math::Max(pos[0][1],pos[1][1],pos[2][1])));

  int32 startX = math::Max(minX,int32(0));
  int32 endX   = math::Min(maxX,int32(image.Size(0)-1));
  int32 startY = math::Max(minY,int32(0));
  int32 endY   = math::Min(maxY,int32(image.Size(1)-1));


 // Iterate the region of space covered by the corners, intercepting rays with
 // the triangle. Record when edges have a collision, as that means we have to
 // extend such edges...
  for (int32 y=startY;y<=endY;y++)
  {
   for (int32 x=startX;x<=endX;x++)
   {
    real32 iDist,iA,iB,iC;
    if (sur::RayTriIntersect(rays.Get(x,y),a,b,c,iDist,iA,iB,iC))
    {
     if (-iDist > depth.Get(x,y))
     {
      depth.Get(x,y) = -iDist;

      image.Get(x,y).r = ac.r*iA + bc.r*iB + cc.r*iC;
      image.Get(x,y).g = ac.g*iA + bc.g*iB + cc.g*iC;
      image.Get(x,y).b = ac.b*iA + bc.b*iB + cc.b*iC;
     }

     minX = math::Min(minX,x-1);
     maxX = math::Max(maxX,x+1);
     minY = math::Min(minY,y-1);
     maxY = math::Max(maxY,y+1);
    }
   }
  }


 // Where edges of the region have rays hitting the triangle extend the region,
 // this is needed as the distortion means the triangle is not necesarilly that
 // triangular in the render...
  while (true)
  {
   int32 lowX  = math::Max(minX,int32(0));
   int32 highX = math::Min(maxX,int32(image.Size(0)-1));
   int32 lowY  = math::Max(minY,int32(0));
   int32 highY = math::Min(maxY,int32(image.Size(1)-1));

   if ((lowX==startX)&&(highX==endX)&&(lowY==startY)&&(highY==endY)) break;

   for (int32 y=lowY;y<=highY;y++)
   {
    for (int32 x=lowX;x<=highX;x++)
    {
     if (((y<startY)||(y>endY))&&((x<startX)||(x>endX)))
     {
      real32 iDist,iA,iB,iC;
      if (sur::RayTriIntersect(rays.Get(x,y),a,b,c,iDist,iA,iB,iC))
      {
       if (-iDist > depth.Get(x,y))
       {
        depth.Get(x,y) = -iDist;

        image.Get(x,y).r = ac.r*iA + bc.r*iB + cc.r*iC;
        image.Get(x,y).g = ac.g*iA + bc.g*iB + cc.g*iC;
        image.Get(x,y).b = ac.b*iA + bc.b*iB + cc.b*iC;
       }

       minX = math::Min(minX,x-1);
       maxX = math::Max(maxX,x+1);
       minY = math::Min(minY,y-1);
       maxY = math::Max(maxY,y+1);
      }
     }
    }
   }

   startX = lowX;
   endX = highX;
   startY = lowY;
   endY = highY;
  }
}

//------------------------------------------------------------------------------
 };
};
