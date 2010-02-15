//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/alg/nearest.h"

#include "eos/file/csv.h"

#include "eos/math/mat_ops.h"
#include "eos/math/eigen.h"
#include "eos/math/complex.h"

namespace eos
{
 namespace alg
 {
//------------------------------------------------------------------------------
EOS_FUNC void PointEllipsoid(const math::Vect<3> & ell,const math::Vect<3> & point,math::Vect<3> & out,
                             real32 tol,nat32 limit)
{
 // Scale the problem to make it numerically reasonable...
  real32 scaler = math::Max(ell[0],ell[1],ell[2]);

  real32 a = ell[0]/scaler;
  real32 b = ell[1]/scaler;
  real32 c = ell[2]/scaler;

  real32 x = point[0]/scaler;
  real32 y = point[1]/scaler;
  real32 z = point[2]/scaler;


 // Calculate the terms of the polynomial we need to solve...
  real32 x2 = math::Sqr(x);
  real32 y2 = math::Sqr(y);
  real32 z2 = math::Sqr(z);
   
  real32 a2 = math::Sqr(a);
  real32 b2 = math::Sqr(b);
  real32 c2 = math::Sqr(c);
 
  real32 a4 = math::Sqr(a2);
  real32 b4 = math::Sqr(b2);
  real32 c4 = math::Sqr(c2);
  
  real32 t6 = -1.0;
  real32 t5 = -2.0 * (a2 + b2 + c2);
  real32 t4 = a2*x2 + b2*y2 + c2*z2 - a4 - b4 - c4 - 4.0*(b2*c2 + a2*c2 + a2*b2);
  real32 t3 = 2.0 * ((b2+c2)*a2*x2 + (a2+c2)*b2*y2 + (a2+b2)*c2*z2)
              - 2.0 * (b2*c4 + b4*c2 + a2*c4 + a4*c2 + a2*b4 + a4*b2)
              - 8.0*a2*b2*c2;
  real32 t2 = (b4 + c4 + 4.0*b2*c2)*a2*x2 + (a4 + c4 + 4.0*a2*c2)*b2*y2
              + (a4 + b4 + 4.0*a2*b2)*c2*z2 - b4*c4 - a4*c4 - a4*b4
              - 4.0*(a4*b2*c2 + a2*b4*c2 + a2*b2*c4);
  real32 t1 = 2.0*((b2*c4 + b4*c2)*a2*x2 + (a2*c4 + a4*c2)*b2*y2 + (a2*b4 + a4*b2)*c2*z2
                   - a2*b4*c4 - a4*b2*c4 - a4*b4*c2);
  real32 t0 = a2*b4*c4*x2 + a4*b2*c4*y2 + a4*b4*c2*z2 - a4*b4*c4;


 // Calculate the starting position for newton iterations, done such that we will always 
 // converge to the best solution...
  real32 alpha;
  if (math::Sqr(x/a)+math::Sqr(y/b)+math::Sqr(z/c)<1.0) alpha = 0.0;
  else
  {
   alpha = math::Sqrt(x2+y2+z2); // * math::Max(a,b,c); - due to scaler will always be 1.
  }


 // Use newton iterations to find alpha...  
  for (nat32 i=0;i<limit;i++)
  {
   // Calculate the function and its differential...
    real32 f = (((((t6*alpha + t5)*alpha + t4)*alpha + t3)*alpha + t2)*alpha + t1)*alpha + t0;    
    real32 df = ((((6.0*t6*alpha + 5.0*t5)*alpha + 4.0*t4)*alpha + 3.0*t3)*alpha + 2.0*t2)*alpha + t1;
   
   // Update...
    real32 offset = f/df;
    alpha -= offset;

   // Break if offset was small enough...
    if (math::Abs(offset)<tol) break;    
  }


 // From the final alpha calculate the output coordinate...
  out[0] = (a2*x)/(alpha + a2);
  out[1] = (b2*y)/(alpha + b2);
  out[2] = (c2*z)/(alpha + c2);

 // Unscale the answer...
  out *= scaler;
}

EOS_FUNC nat32 PointEllipsoidDir(const math::Vect<3> & ell,const math::Vect<3> & point,math::Vect<3> out[6],
                                 real32 tol,nat32 limit)
{
 // Scale the problem to make it numerically reasonable...
  real32 scaler = math::Max(ell[0],ell[1],ell[2]);

  real32 a = ell[0]/scaler;
  real32 b = ell[1]/scaler;
  real32 c = ell[2]/scaler;

  real32 x = point[0]/scaler;
  real32 y = point[1]/scaler;
  real32 z = point[2]/scaler;


 // Calculate the terms of the polynomial we need to solve...
  real32 x2 = math::Sqr(x);
  real32 y2 = math::Sqr(y);
  real32 z2 = math::Sqr(z);
   
  real32 a2 = math::Sqr(a);
  real32 b2 = math::Sqr(b);
  real32 c2 = math::Sqr(c);
 
  real32 a4 = math::Sqr(a2);
  real32 b4 = math::Sqr(b2);
  real32 c4 = math::Sqr(c2);
  
  math::Vect<7> poly;
  poly[6] = -1.0;
  poly[5] = -2.0 * (a2 + b2 + c2);
  poly[4] = a2*x2 + b2*y2 + c2*z2 - a4 - b4 - c4 - 4.0*(b2*c2 + a2*c2 + a2*b2);
  poly[3] = 2.0 * ((b2+c2)*a2*x2 + (a2+c2)*b2*y2 + (a2+b2)*c2*z2)
            - 2.0 * (b2*c4 + b4*c2 + a2*c4 + a4*c2 + a2*b4 + a4*b2)
            - 8.0*a2*b2*c2;
  poly[2] = (b4 + c4 + 4.0*b2*c2)*a2*x2 + (a4 + c4 + 4.0*a2*c2)*b2*y2
            + (a4 + b4 + 4.0*a2*b2)*c2*z2 - b4*c4 - a4*c4 - a4*b4
            - 4.0*(a4*b2*c2 + a2*b4*c2 + a2*b2*c4);
  poly[1] = 2.0*((b2*c4 + b4*c2)*a2*x2 + (a2*c4 + a4*c2)*b2*y2 + (a2*b4 + a4*b2)*c2*z2
            - a2*b4*c4 - a4*b2*c4 - a4*b4*c2);
  poly[0] = a2*b4*c4*x2 + a4*b2*c4*y2 + a4*b4*c2*z2 - a4*b4*c4;
  
  
 // Adjust behaviour of algorithm depending on whether the point to be 
 // perpendicular to is inside or outside the ellipsoid.
 // If inside we have to do a full root solving as all 6 roots could be 
 // possible, if its outside however then there can be only two answers and
 // these are the closest and furthest points on the iterations, i.e. 
 // convergence to them is easily done by two Newton itertions.
 // This approach is far more stable than allways finding all roots, as that
 // case can cause problems for finding the roots by eigenvalues...
  if ((math::Sqr(x/a) + math::Sqr(y/b) + math::Sqr(z/c))<1.0)
  {
   // Inside - solve via eigen method for all 6 roots...
   // Solve for all roots...
    math::Vect<6,math::Complex<real32> > roots;
    nat32 rootCount = 0;
    {
     math::Mat<6,6> temp;
     rootCount = math::RobustPolyRoot(poly,roots,temp,tol,limit);
    }
 
   // Turn the roots into locations on the ellipsoid, as possible - complex and 
   // dodgy roots are possible afterall...
    nat32 ret = 0;
    for (nat32 i=0;i<rootCount;i++)
    {
     // Calculate location based on real component alone...
      out[ret][0] = (a2*x)/(roots[i].x + a2);
      out[ret][1] = (b2*y)/(roots[i].x + b2);
      out[ret][2] = (c2*z)/(roots[i].x + c2);
    
     // Keep if its on the ellipsoid, accounting for numerical error...
      real32 one = math::Sqrt(math::Sqr(out[ret][0]/a) + 
                              math::Sqr(out[ret][1]/b) + 
                              math::Sqr(out[ret][2]/c));
      if (math::Abs(one-1.0)<10.0*math::Sqrt(tol))
      {
       out[ret] *= scaler; // Convert back to the input coordinate system.
       ++ret;
      }
    }
    if (ret!=0) return ret;
  }
  // Else has been dropped, and return adjusted, so it falls back to below if no points found, so it at least gives something.
  {
   // Outside - solve via two Newton iterations...
   
   // Find the closest point...
   // Calculate the starting position for newton iterations, done such that we will always 
   // converge to the closest solution...
   // Don't have to consider the case of being inside the ellipsoid as thats handled above.
    real32 alpha = math::Sqrt(x2+y2+z2); // * math::Max(a,b,c); - due to scaler will always be 1.

   // Use newton iterations to find alpha...  
    for (nat32 i=0;i<limit;i++)
    {
     // Calculate the function and its differential...
      real32 f = (((((poly[6]*alpha + poly[5])*alpha + poly[4])*alpha + poly[3])*alpha + poly[2])*alpha + poly[1])*alpha + poly[0];
      real32 df = ((((6.0*poly[6]*alpha + 5.0*poly[5])*alpha + 4.0*poly[4])*alpha + 3.0*poly[3])*alpha + 2.0*poly[2])*alpha + poly[1];
   
     // Update...
      real32 offset = f/df;
      alpha -= offset;

     // Break if offset was small enough...
      if (math::Abs(offset)<tol) break;    
    }

   // From the final alpha calculate the output coordinate, and remove the scaler affect...
    out[0][0] = (a2*x)/(alpha + a2);
    out[0][1] = (b2*y)/(alpha + b2);
    out[0][2] = (c2*z)/(alpha + c2);
    out[0] *= scaler;
  

   // Find the furthest point...
   // Calculate the starting position for newton iterations, done such that we will always 
   // converge to the closest solution...
   // Don't have to consider the case of being inside the ellipsoid as thats handled above.
    alpha = -math::Sqrt(x2+y2+z2) - 1.0; // - math::Max(a,b,c); - due to scaler will always be 1.

   // Use newton iterations to find alpha...  
    for (nat32 i=0;i<limit;i++)
    {
     // Calculate the function and its differential...
      real32 f = (((((poly[6]*alpha + poly[5])*alpha + poly[4])*alpha + poly[3])*alpha + poly[2])*alpha + poly[1])*alpha + poly[0];
      real32 df = ((((6.0*poly[6]*alpha + 5.0*poly[5])*alpha + 4.0*poly[4])*alpha + 3.0*poly[3])*alpha + 2.0*poly[2])*alpha + poly[1];
   
     // Update...
      real32 offset = f/df;
      alpha -= offset;

     // Break if offset was small enough...
      if (math::Abs(offset)<tol) break;    
    }

   // From the final alpha calculate the output coordinate, and remove the scaler affect...
    out[1][0] = (a2*x)/(alpha + a2);
    out[1][1] = (b2*y)/(alpha + b2);
    out[1][2] = (c2*z)/(alpha + c2);
    out[1] *= scaler;
  
  
   return 2;
  }
}
//------------------------------------------------------------------------------
 };
};
