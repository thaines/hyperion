//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/filter/dog_pyramid.h"

#include "eos/math/constants.h"
#include "eos/math/functions.h"
#include "eos/math/mat_ops.h"

#include <stdio.h>

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
DogPyramid::DogPyramid()
:octaves(0),scales(3),octave(null<svt::Var**>())
{}

DogPyramid::~DogPyramid()
{
 for (nat32 i=0;i<octaves;i++)
 {
  delete octave[i];	 
 }
 delete[] octave;
}

void DogPyramid::Construct(const Pyramid & pyramid)
{
 // Copy in the pyramid stuff...
  octaves = pyramid.Octaves();
  octave = new svt::Var*[octaves];
  scales = pyramid.Scales();

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
    octave[i] = new svt::Var(details);    
    octave[i]->Setup2D(width,height);
    for (nat32 j=0;j<scales+2;j++) octave[i]->Add(str::Token(j+1),nullReal); // Extra 2 for 'end peices'.
    octave[i]->Commit(false);
   
   // Difference the gaussians to generate the actual output...
    for (nat32 j=1;j<scales+3;j++)
    {
     svt::Field<real32> a;
     svt::Field<real32> b;
     svt::Field<real32> c;
	    
     pyramid.Get(i,j-1,a);
     pyramid.Get(i,j,b);
     octave[i]->ByName(str::Token(j),c);

     for (nat32 y=0;y<height;y++)
     {
      for (nat32 x=0;x<width;x++) c.Get(x,y) = b.Get(x,y) - a.Get(x,y);
     }     
    }
    
    width = width/2;
    height = height/2;    
  }
}

nat32 DogPyramid::Scales() const
{
 return scales;	
}

nat32 DogPyramid::Octaves() const
{
 return octaves;	
}

void DogPyramid::Get(nat32 o,nat32 s,svt::Field<real32> & out) const
{
 octave[o]->ByName(str::Token(s+1),out);
}


void DogPyramid::GetExtrema(ds::List<Pos> & out) const
{
 static const int32 offX[8] = {1,0,-1,-1,1, 1, 0,-1};
 static const int32 offY[8] = {1,1, 1, 0,0,-1,-1,-1};

 // Iterate all posible places for extrema...
  for (nat32 i=0;i<octaves;i++)
  {
   for (nat32 j=2;j<scales+2;j++)
   {
    svt::Field<real32> below;
    svt::Field<real32> level;
    svt::Field<real32> above;        
	   
    octave[i]->ByName(str::Token(j-1),below);
    octave[i]->ByName(str::Token(j),level);
    octave[i]->ByName(str::Token(j+1),above);
    
    for (nat32 y=2;y<level.Size(1)-2;y++)
    {
     for (nat32 x=2;x<level.Size(0)-2;x++)
     {
      real32 value = level.Get(x,y);
      bit type = below.Get(x,y)<value;
      
      if (type)
      {
       // Testing for a maximum...
        if (above.Get(x,y)>=value) continue;
            
        bit problem;
        for (nat32 k=0;k<8;k++) {problem = below.Get(x+offX[k],y+offY[k])>=value; if (problem) break;}
        if (problem) continue;

        for (nat32 k=0;k<8;k++) {problem = level.Get(x+offX[k],y+offY[k])>=value; if (problem) break;}
        if (problem) continue;

        for (nat32 k=0;k<8;k++) {problem = above.Get(x+offX[k],y+offY[k])>=value; if (problem) break;}
        if (problem) continue;      
      }
      else
      {
       // Testing for a minimum...
        if (above.Get(x,y)<=value) continue;
            
        bit problem;
        for (nat32 k=0;k<8;k++) {problem = below.Get(x+offX[k],y+offY[k])<=value; if (problem) break;}
        if (problem) continue;

        for (nat32 k=0;k<8;k++) {problem = level.Get(x+offX[k],y+offY[k])<=value; if (problem) break;}
        if (problem) continue;

        for (nat32 k=0;k<8;k++) {problem = above.Get(x+offX[k],y+offY[k])<=value; if (problem) break;}
        if (problem) continue;       
      }
      
      

      Pos np;
       np.x = x;     
       np.y = y;
       np.oct = i;
       np.scale = j-1;
       np.xOff = 0.0;
       np.yOff = 0.0;
       np.sOff = 0.0;
       np.value = value;
      out.AddBack(np);
     }
    }
   }
  }
}

bit DogPyramid::RefineExtrema(Pos & pos) const
{
 // Variables needed...
  svt::Field<real32> level;
  
  math::Vect<3> der;
  math::Mat<3> hess;
  math::Vect<3> res;
  
  bit tts;
  
 // Calculate the new position, iterativly...
  for (nat32 i=0;i<4;i++) // We have a limit, otherwise we get into infinite loops.
  {
   octave[pos.oct]->ByName(str::Token(pos.scale+1),level);

   // Get derivatives and hessian...
    Deriv(pos,der);
    Hessian(pos,hess);
  
   // Calcualte the adjustment...
    res = der;
    res *= -1.0;
    SolveLinear(hess,res);
 
   // Produce the adjustment, if out of range we move to the new
   // position, assuming it dosn't take us past a border...
    pos.xOff = res[0];
    pos.yOff = res[1];
    pos.sOff = res[2];
  
    tts = true;
     if (pos.xOff<-0.5) {tts = false; pos.x--;}
     if (pos.xOff>0.5) {tts = false; pos.x++;}
     if (pos.yOff<-0.5) {tts = false; pos.y--;}
     if (pos.yOff>0.5) {tts = false; pos.y++;}
     if (pos.sOff<-0.5) {tts = false; pos.scale--;}
     if (pos.sOff>0.5) {tts = false; pos.scale++;}        
    if (tts) break;
    
    if ((pos.x<=1)||(pos.x>=(level.Size(0)-2))) return false;
    if ((pos.y<=1)||(pos.y>=(level.Size(1)-2))) return false;
    if ((pos.scale<1)||(pos.scale>scales)) return false;
  }
  if (tts==false) return false;
   
   
 // Update the value to reflect the new location...
  pos.value = level.Get(pos.x,pos.y) + 0.5 * (res * der);
 
 return true;
}

real32 DogPyramid::CurveRatio(const Pos & pos) const
{
 svt::Field<real32> level;
 octave[pos.oct]->ByName(str::Token(pos.scale+1),level);

 math::Mat<2> hess;
 
 hess[0][0] = level.Get(pos.x-1,pos.y) + level.Get(pos.x+1,pos.y) - 2.0*level.Get(pos.x,pos.y);
 hess[0][1] = 0.25*((level.Get(pos.x+1,pos.y+1)-level.Get(pos.x-1,pos.y+1)) - (level.Get(pos.x+1,pos.y-1)-level.Get(pos.x-1,pos.y-1)));
 
 //hess[1][0] = hess[0][1];
 hess[1][1] = level.Get(pos.x,pos.y-1) + level.Get(pos.x,pos.y-1) - 2.0*level.Get(pos.x,pos.y);
 
 
 real32 tr = hess[0][0] + hess[1][1];
 real32 det = hess[0][0]*hess[1][1] - math::Sqr(hess[0][1]);
 
 if (math::Equal(det,real32(0.0))) return 1e100; 
                              else return math::Sqr(tr)/det;
}

void DogPyramid::Deriv(const Pos & pos,math::Vect<3> & out) const
{
 svt::Field<real32> below;	
 svt::Field<real32> level;
 svt::Field<real32> above;
 
 octave[pos.oct]->ByName(str::Token(pos.scale),below);
 octave[pos.oct]->ByName(str::Token(pos.scale+1),level);
 octave[pos.oct]->ByName(str::Token(pos.scale+2),above);
    
 out[0] = 0.5*(level.Get(pos.x+1,pos.y) - level.Get(pos.x-1,pos.y));
 out[1] = 0.5*(level.Get(pos.x,pos.y+1) - level.Get(pos.x,pos.y-1));
 out[2] = 0.5*(above.Get(pos.x,pos.y) - below.Get(pos.x,pos.y));
}

void DogPyramid::Hessian(const Pos & pos,math::Mat<3> & out) const
{
 // Not sure below code is correct - Brown talks about 3D quadratic surface fitting,
 // which I'm ignoring as that would be rather complicated. So taking differences
 // instead as thats what Lowe sugests. He also says it should be easy, and surface
 // fitting certainly would not be.
  svt::Field<real32> below;	
  svt::Field<real32> level;
  svt::Field<real32> above;
 
  octave[pos.oct]->ByName(str::Token(pos.scale),below);
  octave[pos.oct]->ByName(str::Token(pos.scale+1),level);
  octave[pos.oct]->ByName(str::Token(pos.scale+2),above); 
 
  out[0][0] = level.Get(pos.x-1,pos.y)+level.Get(pos.x+1,pos.y) - 2.0*level.Get(pos.x,pos.y);
  out[0][1] = 0.25*((level.Get(pos.x+1,pos.y+1)-level.Get(pos.x-1,pos.y+1)) - (level.Get(pos.x+1,pos.y-1)-level.Get(pos.x-1,pos.y-1)));
  out[0][2] = 0.25*((above.Get(pos.x+1,pos.y)-above.Get(pos.x-1,pos.y)) - (below.Get(pos.x+1,pos.y)-below.Get(pos.x-1,pos.y)));
  
  out[1][0] = out[0][1];
  out[1][1] = level.Get(pos.x,pos.y-1)+level.Get(pos.x,pos.y+1) - 2.0*level.Get(pos.x,pos.y);
  out[1][2] = 0.25*((above.Get(pos.x,pos.y-1)-above.Get(pos.x,pos.y-1)) - (below.Get(pos.x,pos.y-1)-below.Get(pos.x,pos.y-1)));
  
  out[2][0] = out[0][2]; 
  out[2][1] = out[1][2]; 
  out[2][2] = below.Get(pos.x,pos.y)+above.Get(pos.x,pos.y) - 2.0*level.Get(pos.x,pos.y);
}

//------------------------------------------------------------------------------
 };
};
