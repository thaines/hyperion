//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines
#include "eos/fit/image_sphere.h"
#include "eos/data/randoms.h"

namespace eos
{
 namespace fit
 {
//------------------------------------------------------------------------------
ImageOfSphere::ImageOfSphere()
:radius(1.0),centre(0.0,0.0,0.0)
{}

ImageOfSphere::~ImageOfSphere()
{}

void ImageOfSphere::SetCamera(const cam::CameraFull & c)
{
 cf = c;
}

void ImageOfSphere::SetRadius(real32 rad)
{
 radius = rad;
}

void ImageOfSphere::AddPixel(real32 x,real32 y)
{
 bs::Pnt pnt(x,y);
 edge.AddBack(pnt);
}

bit ImageOfSphere::Run(time::Progress * prog)
{
 prog->Push();

 // Create an array of rays, calculated from the pixel positions...
  prog->Report(0,3);
  // Get the cameras centre...
   bs::Vert camCent;
   math::Vect<4,real64> camCentHomo;
   cf.camera.Centre(camCentHomo);
   camCentHomo /= camCentHomo[3];
   for (nat32 i=0;i<3;i++) camCent[i] = camCentHomo[i];
   
   
  // Get the coordinate to 'point on ray' conversion matrix...
   math::Mat<4,3,real64> p2r;
   cf.camera.GetInverse(p2r);
   p2r /= math::FrobNorm(p2r);
   
  // Get the cameras direction...
   math::Vect<3,real64> camDir;
   cf.camera.Dir(camDir);
   LogDebug("camDir" << LogDiv() << camDir);
   
  // Actual construction and filling in...
   ErrData ed;
   ed.radius = radius;
   ed.ray.Size(edge.Size());
   ds::List<bs::Pnt>::Cursor targ = edge.FrontPtr();
   for (nat32 i=0;i<ed.ray.Size();i++)
   {
    cf.GetRay((*targ)[0],(*targ)[1],ed.ray[i],&camCentHomo,&p2r,&camDir);
    
    LogDebug("ray {i,start,dir}" << LogDiv() << i << LogDiv() << ed.ray[i].s << LogDiv() << ed.ray[i].n);
   
    ++targ;
   }


 // Initialisation is, as always, the hard bit...
 // We use a scheme of selecting a bunch of random ray triplets, from each 
 // triplet calculating the exact fit and then finding its error - we ultimatly
 // select the lowest error result...
 // (This is a bit like ransac, but without a rigourous stopping criterion.)
  prog->Report(1,3);
  real32 lowestError = math::Infinity<real32>();
  data::Random rand;
  for (nat32 i=0;i<ed.ray.Size()-2;i++)
  {
   // Select 3 rays at random, without duplication...
    nat32 indA = rand.Int(0,ed.ray.Size()-1);
    nat32 indB = rand.Int(0,ed.ray.Size()-2);
    nat32 indC = rand.Int(0,ed.ray.Size()-3);
    if (indB>=indA) ++indB;
    if (indC>=math::Min(indA,indB)) ++indC;
    if (indC>=math::Max(indA,indB)) ++indC;
  
   // Find the associated centre...
    bs::Vert cent;
    SphereFromRays(radius,ed.ray[indA].s,ed.ray[indA].n,ed.ray[indB].n,ed.ray[indC].n,cent);
   
   // Find the error...
    math::Vect<1> err;
    ErrorFunc(cent,err,ed);
   
   // If the best keep it...
    if (math::IsFinite(err[0])&&(err[0]<lowestError))
    {
     centre = cent;
     lowestError = err[0];
    }
  }
  
  if (!math::IsFinite(lowestError))
  {
   prog->Pop();
   return false;
  }


 // Optimisation time - just invoke LM...
  prog->Report(2,3);
  
  math::LM(centre,ed,&ErrorFunc);
  
 
 prog->Pop();
 return true;
}

const bs::Vert & ImageOfSphere::Centre() const
{
 return centre;
}

void ImageOfSphere::ErrorFunc(const math::Vect<3,real32> & pv,math::Vect<1,real32> & err,
                              const ImageOfSphere::ErrData & oi)
{
 bs::Vert pos;
 for (nat32 i=0;i<3;i++) pos[i] = pv[i];

 err[0] = 0.0;
 for (nat32 i=0;i<oi.ray.Size();i++)
 {
  err[0] += math::Sqr(oi.ray[i].Distance(pos)-oi.radius);
 }
}

void ImageOfSphere::SphereFromRays(real32 radius,const bs::Vert & start,
                                   const bs::Normal & rayA,const bs::Normal & rayB,const bs::Normal & rayC,
                                   bs::Vert & out)
{
 // First calculate the direction that the center must be from start - i.e the
 // direction that makes the same angle with all 3 of the input directions...
  math::Mat<4,4> mat;
  mat[0][0] = rayA[0]; mat[0][1] = rayA[1]; mat[0][2] = rayA[2]; mat[0][3] = 1.0;
  mat[1][0] = rayB[0]; mat[1][1] = rayB[1]; mat[1][2] = rayB[2]; mat[1][3] = 1.0;
  mat[2][0] = rayC[0]; mat[2][1] = rayC[1]; mat[2][2] = rayC[2]; mat[2][3] = 1.0;
  mat[3][0] =     0.0; mat[3][1] =     0.0; mat[3][2] =     0.0; mat[3][3] = 0.0;
  
  math::Vect<4> dirAug;
  math::RightNullSpace(mat,dirAug);
  
  bs::Normal rayCent;
  for (nat32 i=0;i<3;i++) rayCent[i] = dirAug[i];
  rayCent.Normalise();
  
  
 // Now we have the direction to the centre we need to know how far we have to 
 // travel - simple trig...
  real32 dist = radius / math::Sin(math::InvCos(rayA*rayCent));
 
 
 // Now construct the centre from the information avaliable...
  for (nat32 i=0;i<3;i++) out[i] = start[i] + rayCent[i]*dist;
}

//------------------------------------------------------------------------------
 };
};
