//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines
#include "eos/fit/image_sphere.h"

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

void ImageOfSphere::Run(time::Progress * prog)
{
 prog->Push();

 // Create an array of rays, calculated from the pixel positions...
  prog->Report(0,3);
  // Get the cameras centre...
   bs::Vert camCent;
   {
    math::Vect<4,real64> temp;
    cf.camera.Centre(temp);
    temp /= temp[3];
    for (nat32 i=0;i<3;i++) camCent[i] = temp[i];
   }
   
  // Get the coordinate to 'point on ray' conversion matrix...
   math::Mat<4,3,real64> p2r;
   cf.camera.GetInverse(p2r);
   
  // Actual construction and filling in...
   ds::Array<bs::Ray> rays(edge.Size());
   ds::List<bs::Pnt>::Cursor targ = edge.FrontPtr();
   for (nat32 i=0;i<rays.Size();i++)
   {
    rays[i].s = camCent;
   
    math::Vect<3,real64> point;
    point[0] = (*targ)[0];
    point[1] = (*targ)[1];
    point[2] = 1.0;
   
    math::Vect<4,real64> coord;
    math::MultVect(p2r,point,coord);
    coord /= coord[3];
   
    for (nat32 j=0;j<3;j++) rays[i].n[j] = coord[j] - rays[i].s[j];
    rays[i].n.Normalise();
   
    ++targ;
   }


 // Initialisation is, as always, the hard bit...
  prog->Report(1,3);
  // Find the mean edge coordinate - we initialise on its ray...
   real32 meanX = 0.0;
   real32 meanY = 0.0;
   targ = edge.FrontPtr();
   while (!targ.Bad())
   {
    meanX += (*targ)[0];
    meanY += (*targ)[1];
    ++targ;
   }
   meanX /= real32(edge.Size());
   meanY /= real32(edge.Size());
   
  // Construct a ray for the mean edge coordinate...
   bs::Ray start;
   {
    start.s = camCent;
    math::Vect<3,real64> point;
    point[0] = meanX;
    point[1] = meanY;
    point[2] = 1.0;
   
    math::Vect<4,real64> coord;
    math::MultVect(p2r,point,coord);
    coord /= coord[3];
   
    for (nat32 i=0;i<3;i++) start.n[i] = coord[i] - start.s[i];
    start.n.Normalise();
   }
   
  // Go through all rays and find the largest angle made with the start ray -
  // we arrange the radius to be correct for this largest value...
   real32 largestAng = math::pi/18.0;
   for (nat32 i=0;i<rays.Size();i++)
   {
    largestAng = math::Max(largestAng,math::InvCos(rays[i].n * start.n));
   }
   
  // Set the starting sphere center to a distance such that it is the given radius
  // away from the most distant ray...
   start.Travel(radius/math::Sin(largestAng),centre);


 // Optimisation time - just invoke LM...
  prog->Report(2,3);
  
  math::LM(centre,rays,&ErrorFunc);
  
 
 prog->Pop();
}

const bs::Vert & ImageOfSphere::Centre() const
{
 return centre;
}

void ImageOfSphere::ErrorFunc(const math::Vect<3,real32> & pv,math::Vect<1,real32> & err,const ds::Array<bs::Ray> & oi)
{
 bs::Vert pos;
 for (nat32 i=0;i<3;i++) pos[i] = pv[i];

 err[0] = 0.0;
 for (nat32 i=0;i<oi.Size();i++)
 {
  err[0] += oi[i].Distance(pos);
 }
}

//------------------------------------------------------------------------------
 };
};
