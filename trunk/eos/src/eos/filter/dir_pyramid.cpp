//-----------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/filter/dir_pyramid.h"

#include "eos/math/constants.h"
#include "eos/math/functions.h"
#include "eos/math/mat_ops.h"

namespace eos
{
 namespace filter
 {
//-----------------------------------------------------------------------------
DirPyramid::DirPyramid()
:octaves(0),scales(0),extras(0),magOctave(null<svt::Var**>()),dirOctave(null<svt::Var**>())
{}

DirPyramid::~DirPyramid()
{
 for (nat32 i=0;i<octaves;i++)
 {
  delete magOctave[i];
  delete dirOctave[i];
 }
 delete[] magOctave;	
 delete[] dirOctave;
}

void DirPyramid::Construct(const Pyramid & pyramid)
{
 // Copy in the pyramid stuff...
  octaves = pyramid.Octaves();
  magOctave = new svt::Var*[octaves];
  dirOctave = new svt::Var*[octaves];
  scales = pyramid.Scales();
  extras = pyramid.Extras();

 // Get details of the base image size...
  svt::Field<real32> details;
  pyramid.Get(0,0,details);

  nat32 width = details.Size(0);
  nat32 height = details.Size(1);

  real32 nullReal = 0.0;


 // Loop the octaves and construct the data for each one...  
  for (nat32 i=0;i<octaves;i++)
  {
   // Make the output data structure...
    magOctave[i] = new svt::Var(details);    
    magOctave[i]->Setup2D(width,height);
    for (nat32 j=0;j<scales+extras;j++) magOctave[i]->Add(str::Token(j+1),nullReal);
    magOctave[i]->Commit(true);
   
    dirOctave[i] = new svt::Var(details);    
    dirOctave[i]->Setup2D(width,height);
    for (nat32 j=0;j<scales+extras;j++) dirOctave[i]->Add(str::Token(j+1),nullReal);
    dirOctave[i]->Commit(true);
        
   // Calculate all the values...
    for (nat32 j=0;j<scales+extras;j++)
    {
     svt::Field<real32> targ;
     svt::Field<real32> mag;
     svt::Field<real32> dir;
	    
     pyramid.Get(i,j,targ);
     magOctave[i]->ByName(str::Token(j+1),mag);
     dirOctave[i]->ByName(str::Token(j+1),dir);
     
     for (nat32 y=1;y<height-1;y++)
     {
      for (nat32 x=1;x<width-1;x++)
      {
       real32 dx = 0.5*(targ.Get(x+1,y)-targ.Get(x-1,y));
       real32 dy = 0.5*(targ.Get(x,y+1)-targ.Get(x,y-1));
       mag.Get(x,y) = math::Sqrt(math::Sqr(dx) + math::Sqr(dy));
       dir.Get(x,y) = math::InvTan2(dy,dx);
      }
     }     
    }
    
    width = width/2;
    height = height/2;    
  }
}

nat32 DirPyramid::Scales() const
{
 return scales;	
}

nat32 DirPyramid::Extras() const
{
 return extras;	
}

nat32 DirPyramid::Octaves() const
{
 return octaves;	
}

void DirPyramid::GetMag(nat32 o,nat32 s,svt::Field<real32> & mag) const
{
 magOctave[o]->ByName(str::Token(s+1),mag);
}

void DirPyramid::GetDir(nat32 o,nat32 s,svt::Field<real32> & dir) const
{
 dirOctave[o]->ByName(str::Token(s+1),dir);
}

real32 DirPyramid::SampleMag(nat32 octave,real32 scale,real32 x,real32 y) const
{
 nat32 baseX = nat32(math::RoundDown(x));     x -= baseX;
 nat32 baseY = nat32(math::RoundDown(y));     y -= baseY;
 nat32 baseS = nat32(math::RoundDown(scale)); scale -= baseS;
 
 svt::Field<real32> low(magOctave[octave],str::Token(baseS+1));
 svt::Field<real32> high(magOctave[octave],str::Token(baseS+2));
 
 real32 ret = 0.0;
  ret += (1.0-x)*(1.0-y)*(1.0-scale)*low.Get(baseX,baseY);
  ret += (1.0-x)*(1.0-y)*scale*high.Get(baseX,baseY);
  ret += (1.0-x)*y*(1.0-scale)*low.Get(baseX,baseY+1);
  ret += (1.0-x)*y*scale*high.Get(baseX,baseY+1);
  ret += x*(1.0-y)*(1.0-scale)*low.Get(baseX+1,baseY);
  ret += x*(1.0-y)*scale*high.Get(baseX+1,baseY);
  ret += x*y*(1.0-scale)*low.Get(baseX+1,baseY+1);
  ret += x*y*scale*high.Get(baseX+1,baseY+1);
 return ret;
}

real32 DirPyramid::SampleRot(nat32 octave,real32 scale,real32 x,real32 y) const
{
 nat32 baseX = nat32(math::RoundDown(x));     x -= baseX;
 nat32 baseY = nat32(math::RoundDown(y));     y -= baseY;
 nat32 baseS = nat32(math::RoundDown(scale)); scale -= baseS;
 
 svt::Field<real32> low(dirOctave[octave],str::Token(baseS+1));
 svt::Field<real32> high(dirOctave[octave],str::Token(baseS+2));
 
 real32 ret = 0.0;
  ret += (1.0-x)*(1.0-y)*(1.0-scale)*low.Get(baseX,baseY);
  ret += (1.0-x)*(1.0-y)*scale*high.Get(baseX,baseY);
  ret += (1.0-x)*y*(1.0-scale)*low.Get(baseX,baseY+1);
  ret += (1.0-x)*y*scale*high.Get(baseX,baseY+1);
  ret += x*(1.0-y)*(1.0-scale)*low.Get(baseX+1,baseY);
  ret += x*(1.0-y)*scale*high.Get(baseX+1,baseY);
  ret += x*y*(1.0-scale)*low.Get(baseX+1,baseY+1);
  ret += x*y*scale*high.Get(baseX+1,baseY+1);
 return ret;
}

//-----------------------------------------------------------------------------
 };
};
