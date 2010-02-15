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

#include "eos/filter/pyramid.h"

#include "eos/math/constants.h"
#include "eos/math/functions.h"
#include "eos/filter/kernel.h"

namespace eos
{
 namespace filter
 {
//-----------------------------------------------------------------------------
Pyramid::Pyramid()
:octaves(0),scales(3),extras(3),octave(null<svt::Var**>()),
maxOctaves(32),smallestDim(8)
{}

Pyramid::~Pyramid()
{
 for (nat32 i=0;i<octaves;i++)
 {
  delete octave[i];	 
 }
 delete[] octave;	
}

void Pyramid::SetScales(nat32 s,nat32 e)
{
 scales = s;
 extras = e;	
}

void Pyramid::SetStop(nat32 mo,nat32 sd)
{
 maxOctaves = mo;
 smallestDim = sd;	
}

void Pyramid::Construct(const svt::Field<real32> & image)
{
 // Clean up previous...
  if (octave)
  {
   for (nat32 i=0;i<octaves;i++)
   {
    delete octave[i];	 
   }
   delete[] octave;
  }
 
 // First work out how many octaves we will be building...
  octaves = math::Min(maxOctaves,math::TopBit(math::Max(image.Size(0),image.Size(1)))-math::TopBit(smallestDim));
  log::Assert(octaves!=0);
  octave = new svt::Var*[octaves];


 // Declare kernels...
  KernelVect gaussVect(3);
  KernelMat gauss(3);


 // Make a pass buffer to store the data for when moving between octaves,
 // put the base image into it...
  nat32 width = image.Size(0);
  nat32 height = image.Size(1);
  
  svt::Var passBufVar(image);
   real32 nullReal = 0.0;
   passBufVar.Add(str::Token(1),nullReal);
   passBufVar.Commit(false);
  
  svt::Field<real32> passBuf(&passBufVar,str::Token(1));
   passBuf.CopyFrom(image);


 // Loop the octaves and construct the data for each one...  
  for (nat32 i=0;i<octaves;i++)
  {
   // Make the output data structure...   
    octave[i] = new svt::Var(passBuf);    
    octave[i]->Setup2D(width,height);
    for (nat32 j=0;j<scales+extras;j++) octave[i]->Add(str::Token(j+1),nullReal);
    octave[i]->Commit(false);


   // Copy in the first layer then make the rest of the data...
    svt::Field<real32> a(octave[i],str::Token(1));
    svt::Field<real32> b;
    
    for (nat32 y=0;y<height;y++)
    {
     for (nat32 x=0;x<width;x++) a.Get(x,y) = passBuf.Get(x,y);
    }

    for (nat32 j=1;j<scales+extras;j++)
    {
     real32 sigma = math::Sqrt(math::Pow(2.0,2.0/scales) - 1.0) * 1.6;
            sigma *= math::Pow<real32>(math::Pow(2.0,1.0/scales),j-1);
     gaussVect.SetSize(nat32(2.0*sigma));
     gaussVect.MakeGaussian(sigma);    
     gaussVect.ConvertTo(gauss);	    
	    
     octave[i]->ByName(str::Token(j),a);
     octave[i]->ByName(str::Token(j+1),b);
     
     gauss.Apply(a,b);
    }


   // Fill the pass buffer with the data for the next octave, and prep dimensions...	  
    width /= 2;
    height /= 2;
       
    octave[i]->ByName(str::Token(scales+1),a);
    for (nat32 y=0;y<height;y++)
    {
     for (nat32 x=0;x<width;x++) passBuf.Get(x,y) = a.Get(x*2,y*2);
    }
  }
}

nat32 Pyramid::Scales() const
{
 return scales;
}

nat32 Pyramid::Extras() const
{
 return extras;
}

nat32 Pyramid::Octaves() const
{
 return octaves;
}

void Pyramid::Get(nat32 o,nat32 s,svt::Field<real32> & out) const
{
 octave[o]->ByName(str::Token(s+1),out);	
}

//------------------------------------------------------------------------------
 };
};
