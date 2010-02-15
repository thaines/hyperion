//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/filter/shape_index.h"

#include "eos/math/functions.h"
#include "eos/math/matrices.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
EOS_FUNC void ShapeIndex(const svt::Field<bs::Normal> & needle,const svt::Field<bit> & mask,svt::Field<real32> & index)
{
 for (nat32 y=0;y<index.Size(1);y++)
 {
  for (nat32 x=0;x<index.Size(0);x++)
  {
   index.Get(x,y) = -2.0;
  } 
 }

 for (nat32 y=1;y<index.Size(1)-1;y++)
 {
  for (nat32 x=1;x<index.Size(0)-1;x++)
  {
   if (mask.Get(x,y))
   {
    // Calculate the hessian...
     math::Mat<2,2> h;
      h[0][0] = -0.5*(needle.Get(x+1,y)[0]/needle.Get(x+1,y)[2] - needle.Get(x-1,y)[0]/needle.Get(x-1,y)[2]);
      h[0][1] = -0.5*(needle.Get(x+1,y)[1]/needle.Get(x+1,y)[2] - needle.Get(x-1,y)[1]/needle.Get(x-1,y)[2]);
      h[1][0] = -0.5*(needle.Get(x,y+1)[0]/needle.Get(x,y+1)[2] - needle.Get(x,y-1)[0]/needle.Get(x,y-1)[2]);
      h[1][1] = -0.5*(needle.Get(x,y+1)[1]/needle.Get(x,y+1)[2] - needle.Get(x,y-1)[1]/needle.Get(x,y-1)[2]);
     
    // Calculate the curvatures, with k1 >= k2...
     real32 s = math::Sqr(h[0][0] - h[1][1]) + 4.0*h[0][1]*h[1][0];
     if (s<0.0) continue;
     s = math::Sqrt(s);
     
    // Calculate the shape index, checking for degeneracy...
     if (math::Equal(h[0][0]+h[1][1],real32(0.0))&&math::Equal(s,real32(0.0)))
     {
      index.Get(x,y) = 2.0;
      continue;
     }
     
     index.Get(x,y) = (2.0/math::pi)*math::InvTan2(h[0][0]+h[1][1],s);
   }
  }
 }
}

//------------------------------------------------------------------------------
EOS_FUNC void ColourShapeIndex(const svt::Field<real32> & index,svt::Field<bs::ColourRGB> & colour)
{
 for (nat32 y=0;y<colour.Size(1);y++)
 {
  for (nat32 x=0;x<colour.Size(0);x++)
  {
   if (index.Get(x,y)>1.0)
   {
    colour.Get(x,y) = bs::ColourRGB(0.0,0.0,1.0);
   }
   else if (index.Get(x,y)<-1.0)
   {
    colour.Get(x,y) = bs::ColourRGB(0.0,0.0,0.0);
   }
   else
   {
    real32 ind = index.Get(x,y);
    if (ind>0.0)
    {
     if (ind>0.5) colour.Get(x,y) = bs::ColourRGB(1.0,1.0 - 2.0*(ind-0.5),0.0);
             else colour.Get(x,y) = bs::ColourRGB(1.0,1.0,1.0 - 2.0*ind);
    }
    else
    {
     if (ind<-0.5) colour.Get(x,y) = bs::ColourRGB(0.0,1.0,1.0 + 2.0*(ind+0.5));
              else colour.Get(x,y) = bs::ColourRGB(1.0 + 2.0*ind,1.0,1.0);  
    } 
   }
  }
 }
}

//------------------------------------------------------------------------------
 };
};
