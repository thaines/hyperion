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

#include "eos/filter/colour_matching.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
// Returns the mean value of all pixels in a given range...
real32 SimpleImageMatchHelperMean(svt::Field<real32> & im,real32 min,real32 max)
{
 real32 mean = 0.0;
 real32 sum = 0.0;

 for (nat32 y=0;y<im.Size(1);y++)
 {
  for (nat32 x=0;x<im.Size(0);x++)
  {
   real32 val = im.Get(x,y);
   if ((val>=min)&&(val<=max))
   {
    sum += 1.0;
    mean += (val-mean)/sum;
   }
  }
 }

 return mean;
}

// This returns the weighted centre of an image. Recursive with height the never
// of levels to go, weighted with higher means being squared so it puts more
// weight on matching brighter colours than darker colours.
real32 SimpleImageMatchMult(svt::Field<real32> & im,real32 min,real32 max,nat32 height,real32 weight)
{
 real32 centre = SimpleImageMatchHelperMean(im,min,max);

 if (height!=0)
 {
  real32 centreLow  = SimpleImageMatchMult(im,min,centre,height-1,weight);
  real32 centreHigh = SimpleImageMatchMult(im,centre,max,height-1,weight);
  centre = math::Pow((math::Pow(centreLow,weight) + math::Pow(centreHigh,weight))/2.0,1.0/weight);
 }

 return centre;
}

EOS_FUNC void SimpleImageMatch(svt::Field<bs::ColourRGB> & a,svt::Field<bs::ColourRGB> & b,nat32 depth,real32 weight)
{
 LogTime("eos::filter::SimpleImageMatch");
 // Do each colour channel seperatly...
  // Red...
  {
   svt::Field<real32> ar;
   svt::Field<real32> br;
   a.SubField(sizeof(real32)*0,ar);
   b.SubField(sizeof(real32)*0,br);

   real32 redA = SimpleImageMatchMult(ar,-math::Infinity<real32>(),math::Infinity<real32>(),depth,weight);
   real32 redB = SimpleImageMatchMult(br,-math::Infinity<real32>(),math::Infinity<real32>(),depth,weight);

   if (redA>redB)
   {
    real32 mult = redB/redA;
    for (nat32 y=0;y<a.Size(1);y++)
    {
     for (nat32 x=0;x<a.Size(0);x++) a.Get(x,y).r *= mult;
    }
   }
   else
   {
    real32 mult = redA/redB;
    for (nat32 y=0;y<b.Size(1);y++)
    {
     for (nat32 x=0;x<b.Size(0);x++) b.Get(x,y).r *= mult;
    }
   }
  }


  // Green...
  {
   svt::Field<real32> ag;
   svt::Field<real32> bg;
   a.SubField(sizeof(real32)*1,ag);
   b.SubField(sizeof(real32)*1,bg);

   real32 greenA = SimpleImageMatchMult(ag,-math::Infinity<real32>(),math::Infinity<real32>(),depth,weight);
   real32 greenB = SimpleImageMatchMult(bg,-math::Infinity<real32>(),math::Infinity<real32>(),depth,weight);

   if (greenA>greenB)
   {
    real32 mult = greenB/greenA;
    for (nat32 y=0;y<a.Size(1);y++)
    {
     for (nat32 x=0;x<a.Size(0);x++) a.Get(x,y).g *= mult;
    }
   }
   else
   {
    real32 mult = greenA/greenB;
    for (nat32 y=0;y<b.Size(1);y++)
    {
     for (nat32 x=0;x<b.Size(0);x++) b.Get(x,y).g *= mult;
    }
   }
  }


  // Blue...
  {
   svt::Field<real32> ab;
   svt::Field<real32> bb;
   a.SubField(sizeof(real32)*2,ab);
   b.SubField(sizeof(real32)*2,bb);

   real32 blueA = SimpleImageMatchMult(ab,-math::Infinity<real32>(),math::Infinity<real32>(),depth,weight);
   real32 blueB = SimpleImageMatchMult(bb,-math::Infinity<real32>(),math::Infinity<real32>(),depth,weight);

   if (blueA>blueB)
   {
    real32 mult = blueB/blueA;
    for (nat32 y=0;y<a.Size(1);y++)
    {
     for (nat32 x=0;x<a.Size(0);x++) a.Get(x,y).b *= mult;
    }
   }
   else
   {
    real32 mult = blueA/blueB;
    for (nat32 y=0;y<b.Size(1);y++)
    {
     for (nat32 x=0;x<b.Size(0);x++) b.Get(x,y).b *= mult;
    }
   }
  }
}

//------------------------------------------------------------------------------
 };
};
