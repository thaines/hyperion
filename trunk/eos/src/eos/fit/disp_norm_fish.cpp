//------------------------------------------------------------------------------
// Copyright 2009 Tom Haines

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

#include "eos/fit/disp_norm_fish.h"

#include "eos/math/eigen.h"
#include "eos/math/gaussian_mix.h"
#include "eos/sfs/sfs_bp.h"

namespace eos
{
 namespace fit
 {
//------------------------------------------------------------------------------
DispNormFish::DispNormFish()
:prob(0.1),minK(0.1),maxK(10.0)
{}

DispNormFish::~DispNormFish()
{}

void DispNormFish::Set(const svt::Field<real32> & d,const svt::Field<real32> & s)
{
 disp = d;
 sd = s;
}

void DispNormFish::SetMask(const svt::Field<bit> & m)
{
 mask = m;
}

void DispNormFish::SetPair(const cam::CameraPair & p)
{
 pair = p;
}

void DispNormFish::SetRegion(real32 p)
{
 prob = p;
}

void DispNormFish::SetRange(real32 nK,real32 xK)
{
 minK = nK;
 maxK = xK;
}
 
void DispNormFish::Run(time::Progress * prog)
{
 prog->Push();
 
 // Calculate the standard deviation multiplier for a bivariate gaussian needed
 // to obtain a region containing a probability mass of prob around the mean...
 // (Also resize the output array.)
  prog->Report(0,2+disp.Size(1));
  real32 mahDist = math::ChiSquareCulmInv(prob,2);
  LogDebug("test chi square " << math::ChiSquareCulmInv(0.95,10)); // Should output approx 18.3
  
  out.Resize(disp.Size(0),disp.Size(1));

 
 // Setup ready to calculate a Fisher distribution concentration given an
 // angular range that should contain prob around the concentration direction...
  prog->Report(1,2+disp.Size(1));
  prog->Push();
  sfs::FisherAngProb fap;
  fap.Make(prob,minK,maxK,1000,180,prog);
  prog->Pop();


 // Iterate every pixel and do the maths...
  for (nat32 y=0;y<disp.Size(1);y++)
  {
   prog->Report(2+y,2+disp.Size(1));
   for (nat32 x=0;x<disp.Size(0);x++)
   {
    // Set the concentration to zero where we don't have enough information...
     if (mask.Valid()&&(mask.Get(x,y)==false))
     {
      out.Get(x,y) = bs::Vert(0.0,0.0,0.0);
      continue;
     }
     
     bit safeXN = (x>0) && (!mask.Valid() || mask.Get(x-1,y));
     bit safeXP = (x+1<disp.Size(0)) && (!mask.Valid() || mask.Get(x+1,y));
     bit safeYN = (y>0) && (!mask.Valid() || mask.Get(x,y-1));
     bit safeYP = (y+1<disp.Size(1)) && (!mask.Valid() || mask.Get(x,y+1));
     
     if ((!safeXN && !safeXP) || (!safeYN && !safeYP))
     {
      out.Get(x,y) = bs::Vert(0.0,0.0,0.0);
      continue;
     }


    // First calculate the bivariate Gaussian on disparity difference from the 
    // input disparity map, handling the boundary cases...
     real32 muA = disp.Get(x,y);
     real32 sigmaA = sd.Get(x,y);
     
     real32 muB = safeXP?disp.Get(x+1,y):(2.0*muA - disp.Get(x-1,y));
     real32 sigmaB = safeXP?sd.Get(x+1,y):sd.Get(x-1,y);
    
     real32 muC = safeYP?disp.Get(x,y+1):(2.0*muA - disp.Get(x,y-1));
     real32 sigmaC = safeYP?sd.Get(x,y+1):sd.Get(x,y-1);


     real32 rhoA = 1.0/(2.0*sigmaA*sigmaA);
     real32 rhoB = 1.0/(2.0*sigmaB*sigmaB);
     real32 rhoC = 1.0/(2.0*sigmaC*sigmaC);
     real32 rhoS = 1.0/(rhoA + rhoB + rhoC);


     math::Vect<2> mean;
     mean[0] = muB - muA;
     mean[1] = muC - muA;
     
     math::Mat<2,2> prec;
     prec[0][0] = 2.0 * rhoB * (rhoS*rhoB + 1.0);
     prec[0][1] = 2.0 * rhoS * rhoB * rhoC;
     prec[1][0] = prec[0][1];
     prec[1][1] = 2.0 * rhoC * (rhoS*rhoC + 1.0);


     math::Mat<2,2> covar = prec;
     math::Inverse22(covar);
     real32 det = Determinant(covar);
     if ((!math::IsFinite(det))||math::IsZero(det)||(det<0.0))
     {
      out.Get(x,y) = bs::Vert(0.0,0.0,0.0);
      continue;
     }


    // Get the 5 disparity difference values - the mean value and 4 more,
    // offsetted from the mean in both directions along each of the major and 
    // minor axis of the ellipsoid, so they are at the edge of the region 
    // containing the given probability...
     math::Vect<2> ddp[5];
     for (nat32 i=0;i<5;i++) ddp[i] = mean;
     
     math::Mat<2,2> eigenVec;
     math::Vect<2> eigenVal;
     if (math::SymEigen(covar,eigenVec,eigenVal)==false)
     {
      out.Get(x,y) = bs::Vert(0.0,0.0,0.0);
      continue;
     }
     
     for (nat32 r=0;r<2;r++)
     {
      for (nat32 c=0;c<2;c++) eigenVec[r][c] *= math::Sqrt(eigenVal[c]) * mahDist;
     }
     
     ddp[1][0] += eigenVec[0][0]; ddp[1][1] += eigenVec[1][0];
     ddp[2][0] -= eigenVec[0][0]; ddp[2][1] -= eigenVec[1][0];
     ddp[3][0] += eigenVec[0][1]; ddp[3][1] += eigenVec[1][1];
     ddp[4][0] -= eigenVec[0][1]; ddp[4][1] -= eigenVec[1][1];    


    // Convert each disparity difference into a surface orientaiton direction,
    // using the pixels disparity to ground the differences...
     bs::Normal dir[5];
     {
      math::Vect<3> base;
      pair.Triangulate(x,y,muA,base);
      
      for (nat32 i=0;i<5;i++)
      {	
       math::Vect<3> incX, incY;
       pair.Triangulate(x+1,y,muA+ddp[i][0],incX);
       pair.Triangulate(x,y+1,muA+ddp[i][1],incY);
       
       incX -= base;
       incY -= base;
       
       math::CrossProduct(incX,incY,dir[i]);
       dir[i].Normalise();
      }
     }


    // Construct a Fisher distribution from the angles - direction of the centre
    // disparity difference, concentration to match the probability consumed in
    // the angular range...
    // (We take the maximum angle between the edge surface normals and centre
    // surface normal, to take a pesimistic view of certainty.)
     real32 maxAng = 0.0;
     for (nat32 i=1;i<5;i++)
     {
      maxAng = math::Max(maxAng,math::InvCos(dir[0] * dir[1]));
     }
     
     out.Get(x,y) = dir[0];
     out.Get(x,y) *= fap.Concentration(maxAng);
   }
  }

 prog->Pop();
}

void DispNormFish::Get(svt::Field<bs::Vert> & fish)
{
 for (nat32 y=0;y<out.Height();y++)
 {
  for (nat32 x=0;x<out.Width();x++) fish.Get(x,y) = out.Get(x,y);
 }
}

void DispNormFish::Get(svt::Field<math::Fisher> & fish)
{
 for (nat32 y=0;y<out.Height();y++)
 {
  for (nat32 x=0;x<out.Width();x++)
  {
   for (nat32 i=0;i<3;i++) fish.Get(x,y)[i] = out.Get(x,y)[i];
  }
 }
}

//------------------------------------------------------------------------------
 };
};
