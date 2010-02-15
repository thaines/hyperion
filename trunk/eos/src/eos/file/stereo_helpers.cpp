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

#include "eos/file/stereo_helpers.h"

#include "eos/filter/image_io.h"
#include "eos/filter/render_segs.h"
#include "eos/svt/file.h"
#include "eos/stereo/warp.h"
#include "eos/rend/visualise.h"

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
EOS_FUNC bit SaveReal(const svt::Field<real32> & map,cstrconst fn)
{
 bs::ColourRGB rgbIni(0.0,0.0,0.0);
 svt::Var temp(map);
 temp.Add("rgb",rgbIni);
 temp.Commit(false);
 svt::Field<bs::ColourRGB> rgb(&temp,"rgb");
 
 real32 max = 0.001;
 for (nat32 y=0;y<map.Size(1);y++)
 {
  for (nat32 x=0;x<map.Size(0);x++)
  {
   if (math::IsFinite(map.Get(x,y))) max = math::Max(max,math::Abs(map.Get(x,y)));
  }
 }
 
 max = 1.0/max;
 for (nat32 y=0;y<map.Size(1);y++)
 {
  for (nat32 x=0;x<map.Size(0);x++)
  {
   if (math::IsFinite(map.Get(x,y)))
   {
    real32 v = math::Abs(map.Get(x,y) * max);
    rgb.Get(x,y) = bs::ColourRGB(v,v,v);
   }
   else rgb.Get(x,y) = bs::ColourRGB(0.0,0.0,1.0);
  }
 }
 
 str::String filename(fn);
 filename << ".bmp";
 
 cstr n = filename.ToStr();
 bit ret = filter::SaveImage(rgb,n,true);
 mem::Free(n);

 return ret;
}

EOS_FUNC bit SaveSignedReal(const svt::Field<real32> & map,cstrconst fn)
{
 bs::ColourRGB rgbIni(0.0,0.0,0.0);
 svt::Var temp(map);
 temp.Add("rgb",rgbIni);
 temp.Commit(false);
 svt::Field<bs::ColourRGB> rgb(&temp,"rgb");
 
 real32 min = math::Infinity<real32>();
 real32 max = -math::Infinity<real32>();
 for (nat32 y=0;y<map.Size(1);y++)
 {
  for (nat32 x=0;x<map.Size(0);x++)
  {
   if (math::IsFinite(map.Get(x,y)))
   {
    max = math::Max(max,map.Get(x,y));
    min = math::Min(min,map.Get(x,y));
   }
  }
 }
 
 if ((max-min)<0.1) max = min+0.1;

 for (nat32 y=0;y<map.Size(1);y++)
 {
  for (nat32 x=0;x<map.Size(0);x++)
  {
   if (math::IsFinite(map.Get(x,y)))
   {
    real32 v = (map.Get(x,y)-min) / (max-min);
    rgb.Get(x,y) = bs::ColourRGB(v,v,v);
   }
   else rgb.Get(x,y) = bs::ColourRGB(0.0,0.0,1.0);
  }
 }
 
 str::String filename(fn);
 filename << ".bmp";
 
 cstr n = filename.ToStr();
 bit ret = filter::SaveImage(rgb,n,true);
 mem::Free(n);

 return ret;
}

EOS_FUNC bit SaveSeg(const svt::Field<nat32> & seg,cstrconst fn)
{
 bs::ColourRGB rgbIni(0.0,0.0,0.0);
 svt::Var temp(seg);
 temp.Add("rgb",rgbIni);
 temp.Commit(false);
 svt::Field<bs::ColourRGB> rgb(&temp,"rgb");
 
 filter::RenderSegsColour(seg,rgb);
 
 str::String filename(fn);
 filename << ".bmp";
 
 cstr n = filename.ToStr();
 bit ret = filter::SaveImage(rgb,n,true);
 mem::Free(n);

 return ret;
}

EOS_FUNC bit SaveDisparity(const svt::Field<real32> & disp,cstrconst fn,const svt::Field<bit> * mask)
{
 // Image...
  if (!SaveSignedReal(disp,fn)) return false;

 // dis...
  real32 dispIni = 0.0;
  bit maskIni = true;
  svt::Var temp(disp);
  temp.Add("disp",dispIni);
  if (mask) temp.Add("mask",maskIni);
  temp.Commit(false);
  
  svt::Field<real32> d(&temp,"disp");
  d.CopyFrom(disp);
  
  if (mask)
  {
   svt::Field<bit> m(&temp,"mask");
   m.CopyFrom(*mask);
  }
 
  str::String filename(fn);
  filename << ".dis";
 
  cstr n = filename.ToStr();
  bit ret = svt::Save(n,&temp,true);
  mem::Free(n);

 return ret;
}

EOS_FUNC bit SaveWarp(const svt::Field<bs::ColourRGB> & left,const svt::Field<bs::ColourRGB> & right,
                      const svt::Field<real32> & disp,cstrconst fn,const svt::Field<bit> * mask)
{
 bs::ColourRGB rgbIni(0.0,0.0,0.0);
 svt::Var temp(right);
 temp.Add("rgb",rgbIni);
 temp.Commit(false);
 svt::Field<bs::ColourRGB> rgb(&temp,"rgb");
 
 stereo::ForwardWarp(left,disp,rgb,2.5,1.0,mask);

 str::String filename(fn);
 filename << ".bmp";
 
 cstr n = filename.ToStr();
 bit ret = filter::SaveImage(rgb,n,true);
 mem::Free(n);

 return ret;
}

EOS_FUNC bit SaveNeedle(const svt::Field<bs::Normal> & needle,cstrconst fn,bit ext)
{
 // First save a colour image...
 {
  bs::ColourRGB rgbIni(0.0,0.0,0.0);
  svt::Var temp(needle);
  temp.Add("rgb",rgbIni);
  temp.Commit(false);
  svt::Field<bs::ColourRGB> rgb(&temp,"rgb");
 
  rend::VisNeedleMap(needle,rgb);

  str::String filename(fn);
  filename << ".bmp";
 
  cstr n = filename.ToStr();
  bit ret = filter::SaveImage(rgb,n,true);
  mem::Free(n);

  if (ret==false) return false;
 }
 
 // Now save a negative version
 {
  bs::ColourRGB rgbIni(0.0,0.0,0.0);
  svt::Var temp(needle);
  temp.Add("rgb",rgbIni);
  temp.Commit(false);
  svt::Field<bs::ColourRGB> rgb(&temp,"rgb");
 
  rend::VisNeedleMap(needle,rgb,true);

  str::String filename(fn);
  filename << "_neg.bmp";
 
  cstr n = filename.ToStr();
  bit ret = filter::SaveImage(rgb,n,true);
  mem::Free(n);

  if (ret==false) return false;
 }

 // Then save a 3d model with planes representing normal orientation...
  if (ext)
  {
   file::Wavefront mod;
   rend::NeedleMapToModel(needle,mod);
   
   str::String filename(fn);
   filename << ".obj";
  
   cstr n = filename.ToStr();
   bit ret = mod.Save(n,true);
   mem::Free(n);
   
   return ret;
  }
  
 return true;
}

EOS_FUNC bit SaveGradient(const svt::Field<real32> & dx,const svt::Field<real32> & dy,cstrconst fn)
{
 log::Assert((dx.Size(0)==dy.Size(0))&&(dx.Size(1)==dy.Size(1)));
 
 // Create the storage for the image...
  bs::ColourRGB rgbIni(0.0,0.0,0.0);
  svt::Var temp(dx);
  temp.Add("rgb",rgbIni);
  temp.Commit(false);
  svt::Field<bs::ColourRGB> rgb(&temp,"rgb");
  
 // Do a pass over the gradient to find the scaling of the channels...
  real32 maxDelta = 0.1;
  real32 maxMag = 0.1;
  for (nat32 y=0;y<dx.Size(1);y++)
  {
   for (nat32 x=0;x<dx.Size(0);x++)
   {
    maxDelta = math::Max(maxDelta,math::Abs(dx.Get(x,y)),math::Abs(dy.Get(x,y)));
    maxMag = math::Max(maxMag,math::Sqrt(math::Sqr(dx.Get(x,y)) + math::Sqr(dy.Get(x,y))));
   }
  }
  
 // Fill in the image...
  for (nat32 y=0;y<dx.Size(1);y++)
  {
   for (nat32 x=0;x<dx.Size(0);x++)
   {
    rgb.Get(x,y).r = (0.5*dx.Get(x,y)/maxDelta) + 0.5;
    rgb.Get(x,y).g = (0.5*dy.Get(x,y)/maxDelta) + 0.5;
    rgb.Get(x,y).b = math::Sqrt(math::Sqr(dx.Get(x,y)) + math::Sqr(dy.Get(x,y)))/maxMag;
   }
  }

 // Save it...
  str::String filename(fn);
  filename << ".bmp";
 
  cstr n = filename.ToStr();
  bit ret = filter::SaveImage(rgb,n,true);
  mem::Free(n);

 return ret;
}

//------------------------------------------------------------------------------
 };
};
