#ifdef EOS_INC_PATENTED
//-----------------------------------------------------------------------------
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

#include "eos/filter/sift.h"

#include "eos/math/functions.h"
#include "eos/math/mat_ops.h"
#include "eos/math/gaussian_mix.h"
#include "eos/filter/kernel.h"
#include "eos/filter/pyramid.h"
#include "eos/filter/dog_pyramid.h"
#include "eos/filter/dir_pyramid.h"
#include "eos/ds/lists.h"
#include "eos/rend/functions.h"
#include "eos/file/csv.h"

#include <stdio.h>

namespace eos
{
 namespace filter
 {
//-----------------------------------------------------------------------------
SiftKeypoint::SiftKeypoint()
{}

SiftKeypoint::~SiftKeypoint()
{}

void SiftKeypoint::Run(const svt::Field<real32> & image,time::Progress * prog)
{
 prog->Push();
 
 // Double the image size and apply an initial blur...
  // Make storage...
   svt::Var bic(image);
   bic.Setup2D(image.Size(0)*2,image.Size(1)*2);
    real32 nullReal = 0.0;
    bic.Add("l",nullReal);
    bic.Commit();
    
   svt::Field<real32> baseImage(&bic,"l");
   
  // Fill with doubled image...
   for (nat32 y=0;y<image.Size(1);y++)
   {
    for (nat32 x=0;x<image.Size(0);x++)
    {
     baseImage.Get(x*2,y*2) = image.Get(x,y);	    
     if ((x+1)!=image.Size(0)) baseImage.Get(x*2+1,y*2) = 0.5*(image.Get(x,y) + image.Get(x+1,y));
                          else baseImage.Get(x*2+1,y*2) = image.Get(x,y);
     if ((y+1)!=image.Size(1)) baseImage.Get(x*2,y*2+1) = 0.5*(image.Get(x,y) + image.Get(x,y+1));
                          else baseImage.Get(x*2,y*2+1) = image.Get(x,y);
     if (((x+1)!=image.Size(0))&&((y+1)!=image.Size(1)))
     {
      baseImage.Get(x*2+1,y*2+1) = 0.25*(image.Get(x,y)+image.Get(x+1,y)+image.Get(x,y+1)+image.Get(x+1,y+1));
     }
     else baseImage.Get(x*2+1,y*2+1) = image.Get(x,y);
    }
   }
  
  // Blur...
   filter::KernelVect gauss(nat32(initialSmooth*2.0));
   gauss.MakeGaussian(initialSmooth);
   gauss.Apply(baseImage,baseImage);


 // Build the 3 pyramids...
  DogPyramid dogPyramid;
  DirPyramid dirPyramid;
  
  prog->Report(0,7);
  pyramid.Construct(baseImage); prog->Report(1,7);
  dogPyramid.Construct(pyramid); prog->Report(2,7);
  dirPyramid.Construct(pyramid); prog->Report(3,7);


 // Now find all possible keypoints...
  ds::List<DogPyramid::Pos> listA;
  dogPyramid.GetExtrema(listA); prog->Report(4,7);


 // Refine all points then prune those that are unsuitable for the task at hand...
 {
  ds::List<DogPyramid::Pos>::Cursor targ = listA.FrontPtr();
  while (!targ.Bad())
  {	  
   // First refine it...
    if (dogPyramid.RefineExtrema(*targ)==false)
    {
     targ.RemKillNext();
     continue;
    }

   // Check if the value is high enough...
    if (math::Abs(targ->value)<minContrast)
    {
     targ.RemKillNext();
     continue;	    
    }
   
   // Do the curvature ratio check...
    real32 curveRatio = dogPyramid.CurveRatio(*targ);
    if ((curveRatio<0.0)||(curveRatio>(math::Sqr(maxCurveRatio+1)/maxCurveRatio)))
    {
     targ.RemKillNext();
     continue;	    
    }

   ++targ;	  
  }
  prog->Report(5,7);
 }
 
 
 // Assign rotation to each keypoint, splitting them as needed...
  ds::List<Keypoint> listB;
  {
   ds::List<DogPyramid::Pos>::Cursor targ = listA.FrontPtr();
   while (!targ.Bad())
   {
    // Construct a weighted rotation histogram...
     // Build the histogram structure...
      real32 histo[rotBins];
      for (nat32 i=0;i<rotBins;i++) histo[i] = 0.0;
     
     // Get the rotations and magnitudes we are going to be playing with...
      svt::Field<real32> mag;
      svt::Field<real32> dir;
      
      dirPyramid.GetMag(targ->oct,targ->scale,mag);
      dirPyramid.GetDir(targ->oct,targ->scale,dir);
      
     // Calculate the range of pixels to sample...
      real32 sd = pyramid.Sd(targ->oct,targ->scale) * sdScale;
      real32 range = sd * sdMult;
      
      real32 centX = targ->x + targ->xOff;
      real32 centY = targ->y + targ->yOff;
            
      int32 minX = int32(math::RoundDown(centX - range));
      int32 maxX = int32(math::RoundUp(centX + range));
      int32 minY = int32(math::RoundDown(centY - range));
      int32 maxY = int32(math::RoundUp(centY + range));
      
     // Ignore points that are too close to the border to be analysed reasonably...
      if ((minX<0)||(maxX>=int32(mag.Size(0)))||(minY<0)||(maxY>=int32(mag.Size(1))))
      {
       ++targ;
       continue;	      
      }
      
     // Iterate...
      for (nat32 y=minY;int32(y)<=maxY;y++)
      {
       for (nat32 x=minX;int32(x)<=maxX;x++)
       {
	histo[nat32(math::Round((rotBins*(dir.Get(x,y)+math::pi))/(2.0*math::pi)))%rotBins] 
	     += math::Gaussian(sd,math::Sqrt(math::Sqr(x-centX)+math::Sqr(y-centY))) * mag.Get(x,y);
       }	      
      }
    
    // Find the maximum...
     real32 max = histo[0];
     for (nat32 i=1;i<rotBins;i++) max = math::Max(max,histo[i]);
     max *= peekMult;

    // For all points within the given ratio of the maximum make an
    // output keypoint, interpolate to get accurate positions...
     for (nat32 i=0;i<rotBins;i++)
     {
      if (histo[i]>=max)
      {
       nat32 bi	= (i+rotBins-1)%rotBins;
       nat32 ai = (i+1)%rotBins;
       if ((histo[bi]<histo[i])&&(histo[ai]<histo[i]))
       {
	// Interpolate the orientation...
	 math::Mat<3> mat;
	 math::Vect<3> vect;
	 mat[0][0] = 1.0; mat[0][1] = -1.0; mat[0][1] = 1.0; vect[0] = histo[bi];
	 mat[1][0] = 1.0; mat[1][1] =  1.0; mat[1][1] = 1.0; vect[1] = histo[ai];
	 mat[2][0] = 1.0; mat[2][1] =  0.0; mat[2][1] = 0.0; vect[2] = histo[i];	 
	 
	 SolveLinear(mat,vect);
	 if (!math::Equal(vect[2],real32(0.0)))
	 {
	  real32 iRot = (((-vect[1]/(2.0*vect[2]))+i+0.5)*2.0*math::pi)/real32(rotBins) - math::pi;
	 
	  // Store it...
	   Keypoint kp;
	    kp.octave = targ->oct;
	    kp.scale = targ->scale;
	    kp.x = targ->x;
	    kp.y = targ->y;
	    kp.xOff = targ->xOff;
	    kp.yOff = targ->yOff;
	    kp.sOff = targ->sOff;
	    kp.rot = iRot;	 
	   listB.AddBack(kp);
         }
       }
      }   
     }

    ++targ;	   
   }
   prog->Report(6,7);
  }  
  
  
 // Transfer from the linked list into the array where the final results will live...
 {
  data.Size(listB.Size());
  ds::List<Keypoint>::Cursor targ = listB.FrontPtr();
  nat32 i = 0;
  while (!targ.Bad())
  {
   data[i] = *targ;
   ++targ;
   ++i;	  
  }
  prog->Report(7,7);
 }

 prog->Pop();	
}

nat32 SiftKeypoint::Keypoints() const
{
 return data.Size();	
}

const SiftKeypoint::Keypoint & SiftKeypoint::operator [] (nat32 i) const
{
 return data[i];	
}

void SiftKeypoint::Render(svt::Field<bs::ColourRGB> & image,const bs::ColourRGB & pcol,const bs::ColourRGB & lcol)
{
 for (nat32 i=0;i<data.Size();i++)
 {
  bs::Pnt start = bs::Pnt((data[i].x+data[i].xOff)*math::Pow<real32>(2,data[i].octave)*0.5,(data[i].y+data[i].yOff)*math::Pow<real32>(2,data[i].octave)*0.5);
  bs::Pnt end;
   end[0] = math::Cos(data[i].rot);
   end[1] = math::Sin(data[i].rot);
   end *= pyramid.Sd(data[i].octave,data[i].scale+data[i].sOff)*3.0;
   end += start;
  
  rend::Arrow(image,start,end,lcol,0.15);
  rend::SetPixel(image,start,pcol);
 }
}

//-----------------------------------------------------------------------------
SiftFeature::SiftFeature()
{}

SiftFeature::~SiftFeature()
{}

void SiftFeature::Run(const SiftKeypoint & kps,time::Progress * prog)
{
 prog->Push();
 
 // Yes, we make this twice. But the codes so slow anyway I really couldn't care...
  DirPyramid dirPyramid; 
  dirPyramid.Construct(kps.GetPyramid());
 
 
 data.Size(kps.Keypoints());
 for (nat32 i=0;i<kps.Keypoints();i++)
 {
  prog->Report(i,kps.Keypoints());
  
  // Null the feature vector...
   for (nat32 j=0;j<fvSize;j++) data[i][j] = 0.0;
  
  // Iterate all sample points and distribute there component into the feature
  // vector...
   svt::Field<real32> strength; dirPyramid.GetMag(kps[i].octave,kps[i].scale,strength);
   svt::Field<real32> direction; dirPyramid.GetDir(kps[i].octave,kps[i].scale,direction);
   
   real32 sAng = math::Sin(kps[i].rot);
   real32 cAng = math::Cos(kps[i].rot);
   
   nat32 startX = nat32(math::RoundDown(kps[i].x+kps[i].xOff));
   nat32 startY = nat32(math::RoundDown(kps[i].y+kps[i].yOff));
   if ((startX<(dimRes/2))||(startY<(dimRes/2))) continue;
   startX -= dimRes/2;
   startY -= dimRes/2;
   if ((startX+dimRes)>=strength.Size(0)) continue;
   if ((startY+dimRes)>=strength.Size(1)) continue;
   
   //LogAlways("sift feature (" << kps[i].x+kps[i].xOff << "," << kps[i].y+kps[i].yOff << ") at [" << kps[i].octave << "," << kps[i].scale << "]");
   //LogAlways("start = (" << startX << "," << startY << ")");
   
   for (nat32 y=0;y<dimRes;y++)
   {
    for (nat32 x=0;x<dimRes;x++)
    {
     // Convert from world coordinates to local coordinates...
      real32 relX = (startX+x) - (kps[i].x+kps[i].xOff);
      real32 relY = (startY+y) - (kps[i].y+kps[i].yOff);
      
      real32 locX = cAng*relX - sAng*relY;
      real32 locY = sAng*relX + cAng*relY;
       
     // Get the rotation and magnitude, weight the magnitude...
      real32 rot = direction.Get(x+startX,y+startY);
      real32 mag = strength.Get(x+startX,y+startY);
      mag *= math::Gaussian(real32(dimRes*0.5),math::Sqrt(math::Sqr(locX)+math::Sqr(locY)));
     
      //LogAlways("rel = (" << relX << "," << relY << "); loc = (" << locX << "," << locY << ")");
      //LogAlways("rot = " << rot << "; mag = " << mag);
     
     // Move into the feature vector 'coordinate system'...
      rot = math::Mod(rot - kps[i].rot + 4.0*math::pi*2.0,math::pi*2.0)*(real32(rotSamp)/(2.0*math::pi));
      int32 lowRot = int32(math::RoundDown(rot));
      nat32 highRot = (lowRot+1)%rotSamp;
      rot -= lowRot;
      
      locX = locX*(real32(dimSamp+1)/real32(2*dimRes)) + dimSamp/2;
      int32 lowX = int32(math::RoundDown(locX)); locX -= lowX;
      
      locY = locY*(real32(dimSamp+1)/real32(2*dimRes)) + dimSamp/2;
      int32 lowY = int32(math::RoundDown(locY)); locY -= lowY;
           
      if ((lowX<-1)||(lowX>=int32(dimSamp))) continue;
      if ((lowY<-1)||(lowY>=int32(dimSamp))) continue;
      
      //LogAlways("lowX = " << lowX << "; lowY = " << lowY << "; lowRot = " << lowRot);
      //LogAlways("locX = " << locX << "; locY = " << locY << "; rot = " << rot);

            
     // Add it to the feature vector, will influence 8 positions
     // due to its 3d nature... (2 normal dimensions and orientation.)     
      if ((lowX>=0)&&(lowY>=0))
      {
       Entry(data[i],lowX,lowY,lowRot)  += mag * (1.0-rot) * (1.0-locX) * (1.0-locY);
       Entry(data[i],lowX,lowY,highRot) += mag *       rot * (1.0-locX) * (1.0-locY);
      }
      
      if ((lowX>=0)&&(lowY+1<int32(dimSamp)))
      {
       Entry(data[i],lowX,lowY+1,lowRot)  += mag * (1.0-rot) * (1.0-locX) * locY;
       Entry(data[i],lowX,lowY+1,highRot) += mag *       rot * (1.0-locX) * locY;
      }

      if ((lowX+1<int32(dimSamp))&&(lowY>=0))
      {
       Entry(data[i],lowX+1,lowY,lowRot)  += mag * (1.0-rot) * locX * (1.0-locY);
       Entry(data[i],lowX+1,lowY,highRot) += mag *       rot * locX * (1.0-locY);
      }
      
      if ((lowX+1<int32(dimSamp))&&(lowY+1<int32(dimSamp)))
      {
       Entry(data[i],lowX+1,lowY+1,lowRot)  += mag * (1.0-rot) * locX * locY;
       Entry(data[i],lowX+1,lowY+1,highRot) += mag *       rot * locX * locY;
      }    
    }	   
   }
  
  // Normalise, threshold, normalise...
   real32 len = 0.0;
   for (nat32 j=0;j<fvSize;j++) len += math::Sqr(data[i][j]);
   len = math::InvSqrt(len);
   
   for (nat32 j=0;j<fvSize;j++)
   {
    data[i][j] *= len;
    if (data[i][j]>maxFeat) data[i][j] = maxFeat;
   }
  
   len = 0.0;
   for (nat32 j=0;j<fvSize;j++) len += math::Sqr(data[i][j]);   
   len = math::InvSqrt(len);
   
   for (nat32 j=0;j<fvSize;j++) data[i][j] *= len;	 
 }
 
 prog->Pop();	
}

const math::Vect<128> & SiftFeature::operator[] (nat32 kp) const
{
 return data[kp];	
}

//-----------------------------------------------------------------------------
 };
};#endif

