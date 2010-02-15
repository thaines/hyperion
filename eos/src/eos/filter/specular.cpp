//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/filter/specular.h"

#include "eos/ds/arrays.h"
#include "eos/math/stats.h"
#include "eos/math/mat_ops.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
EOS_FUNC void SimpleSpecMask(const svt::Field<bs::ColourRGB> & image,
                             const svt::Field<nat32> & segs,                             
                             svt::Field<bit> & mask,const bs::ColourRGB & light)
{
 // Count how many segments we have...
  nat32 segCount = 1;
  for (nat32 y=0;y<segs.Size(1);y++)
  {
   for (nat32 x=0;x<segs.Size(0);x++)
   {
    segCount = math::Max(segCount,segs.Get(x,y)+1);
   }
  }


 // Create a data structure to contain all our working...
  real32 realIni;
 
  svt::Var temp(image);
  temp.Add("rho",realIni);
  temp.Add("theta",realIni);
  temp.Add("phi",realIni);
  temp.Commit();
  
  svt::Field<real32> rho(&temp,"rho");
  svt::Field<real32> theta(&temp,"theta");
  svt::Field<real32> phi(&temp,"phi");


 // Generate matrices to convert to/from suv space...
  math::Mat<3> toSuv;
  math::Mat<3> fromSuv;
  {
   math::Vect<3> xAxis;
   xAxis[0] = 1.0; xAxis[1] = 0.0; xAxis[2] = 0.0;
   
   math::Vect<3> lightAxis;
   lightAxis[0] = light.r; lightAxis[1] = light.g; lightAxis[2] = light.b;
   lightAxis.Normalise();
   
   math::Vect<3> rotAxis;
   math::CrossProduct(lightAxis,xAxis,rotAxis);
   rotAxis.Normalise();
   
   math::AngAxisToRotMat(rotAxis,xAxis*lightAxis,toSuv);
   
   fromSuv = toSuv;
   math::Transpose(fromSuv);
  }


 // Convert from rgb to polar/cylindrical suv coordinates...
  for (nat32 y=0;y<image.Size(1);y++)
  {
   for (nat32 x=0;x<image.Size(0);x++)
   {
    // rgb to suv...
     math::Vect<3> rgb;
     rgb[0] = image.Get(x,y).r;
     rgb[1] = image.Get(x,y).g;
     rgb[2] = image.Get(x,y).b;
     
     math::Vect<3> suv;
     math::MultVect(toSuv,rgb,suv);
    
    // suv to polar/cylindrical form...
     rho.Get(x,y) = math::Sqrt(math::Sqr(suv[1]) + math::Sqr(suv[2]));
     theta.Get(x,y) = math::InvTan2(suv[2],suv[1]);
     phi.Get(x,y) = math::InvTan2(suv[0],rho.Get(x,y));
   }
  }


 // Find the average phi value for each segment...
  ds::Array<nat32> segSize(segCount);
  ds::Array<real32> meanPhi(segCount);
  for (nat32 i=0;i<segCount;i++)
  {
   segSize[i] = 0;
   meanPhi[i] = 0.0;
  }
  
  for (nat32 y=0;y<phi.Size(1);y++)
  {
   for (nat32 x=0;x<phi.Size(0);x++)
   {
    nat32 seg = segs.Get(x,y);
    meanPhi[seg] += phi.Get(x,y);
    segSize[seg] += 1;
   }
  }
  
  for (nat32 i=0;i<segCount;i++)
  {
   if (segSize[i]!=0) meanPhi[i] /= real32(segSize[i]);
  }


 // Threshold all pixels in each segment, such that any larger than the average
 // plus the threshold will be classed as specular...
  static const real32 threshold = 0.2;
  for (nat32 i=0;i<meanPhi.Size();i++) meanPhi[i] += threshold;
  
  for (nat32 y=0;y<mask.Size(1);y++)
  {
   for (nat32 x=0;x<mask.Size(0);x++)
   {
    mask.Get(x,y) = phi.Get(x,y)>meanPhi[segs.Get(x,y)];
   }
  }
}

//------------------------------------------------------------------------------
SpecRemoval::SpecRemoval()
:light(1.0,1.0,1.0)
{}

SpecRemoval::~SpecRemoval()
{}

void SpecRemoval::Set(const svt::Field<bs::ColourRGB> & i,svt::Field<bs::ColourRGB> & o)
{
 in = i;
 out = o;	
}

void SpecRemoval::Set(const bs::ColourRGB & l)
{
 light = l;	
}

void SpecRemoval::Set(svt::Field<bit> & m)
{
 mask = m;	
}

void SpecRemoval::Run(time::Progress * prog)
{
 prog->Push();
 
 // Create a data structure to contain all our working...
  real32 realIni;
 
  svt::Var temp(in);
  temp.Add("rho",realIni);
  temp.Add("theta",realIni);
  temp.Add("phi",realIni);
  temp.Commit();
  
  svt::Field<real32> rho(&temp,"rho");
  svt::Field<real32> theta(&temp,"theta");
  svt::Field<real32> phi(&temp,"phi");


 // Generate matrices to convert to/from suv space...
  math::Mat<3> toSuv;
  math::Mat<3> fromSuv;
  {
   math::Vect<3> xAxis;
   xAxis[0] = 1.0; xAxis[1] = 0.0; xAxis[2] = 0.0;
   
   math::Vect<3> lightAxis;
   lightAxis[0] = light.r; lightAxis[1] = light.g; lightAxis[2] = light.b;
   lightAxis.Normalise();
   
   math::Vect<3> rotAxis;
   math::CrossProduct(lightAxis,xAxis,rotAxis);
   rotAxis.Normalise();
   
   math::AngAxisToRotMat(rotAxis,xAxis*lightAxis,toSuv);
   
   fromSuv = toSuv;
   math::Transpose(fromSuv);
  }


 // Convert from rgb to the papers polar/cylindrical suv coordinates...
  for (nat32 y=0;y<in.Size(1);y++)
  {
   for (nat32 x=0;x<in.Size(0);x++)
   {
    // rgb to suv...
     math::Vect<3> rgb;
     rgb[0] = in.Get(x,y).r; rgb[1] = in.Get(x,y).g; rgb[2] = in.Get(x,y).b;
     
     math::Vect<3> suv;
     math::MultVect(toSuv,rgb,suv);
    
    // suv to polar/cylindrical form...
     rho.Get(x,y) = math::Sqrt(math::Sqr(suv[1]) + math::Sqr(suv[2]));
     theta.Get(x,y) = math::InvTan2(suv[2],suv[1]);
     phi.Get(x,y) = math::InvTan2(suv[0],rho.Get(x,y));
   }
  }


 // Specularity is only evident under our assumptions as an addative affect in
 // the phi channel, so we need to reduce this channel an appropriate amount for
 // each pixel. rho represents the diffuse component, whilst theta is
 // a 'generalised hue'. Basically we need to identify pixels that have the same
 // albedo and set there phi to the minimum evident.
 // The likelyhood of pixels being identical is related to theta directly, with
 // sudden changes of rho, and even phi, being indicative of edges through which
 // info should not pass.
 
 for (nat32 y=0;y<phi.Size(1);y++)
 {
  for (nat32 x=0;x<phi.Size(0);x++)
  {
   if (x!=0) phi.Get(x,y) = math::Min(phi.Get(x,y),phi.Get(x-1,y));
   if (y!=0) phi.Get(x,y) = math::Min(phi.Get(x,y),phi.Get(x,y-1));
  }
 }
 
 // *******************************************************
 

 // Convert from the polar/cylindrical suv representation back to rgb...
  for (nat32 y=0;y<out.Size(1);y++)
  {
   for (nat32 x=0;x<out.Size(0);x++)
   {
    // polar/cylindrical form to suv...
     math::Vect<3> suv;
     suv[0] = rho.Get(x,y) * math::Tan(phi.Get(x,y));
     suv[1] = rho.Get(x,y) * math::Cos(theta.Get(x,y));
     suv[2] = rho.Get(x,y) * math::Sin(theta.Get(x,y));


    // suv to rgb, and save out...
     math::Vect<3> rgb;
     math::MultVect(fromSuv,suv,rgb);
     
     out.Get(x,y).r = rgb[0];
     out.Get(x,y).g = rgb[1];
     out.Get(x,y).b = rgb[2];
   }
  }


 // Fill in the mask if requested, just anywhere where rho is zero, indicating
 // no variation from the light source colour...
  if (mask.Valid())
  {
   for (nat32 y=0;y<mask.Size(1);y++)
   {
    for (nat32 x=0;x<mask.Size(0);x++)
    {
     mask.Get(x,y) = math::IsZero(rho.Get(x,y));
    }
   }
  }
 
 
 prog->Pop();	
}

//------------------------------------------------------------------------------
 };
};
