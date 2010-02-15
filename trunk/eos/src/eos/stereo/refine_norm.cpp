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

#include "eos/stereo/refine_norm.h"

#include "eos/inf/gauss_integration.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
RefineNorm::RefineNorm()
:iters(100),dsiSd(1.0),costBase(1.0),normVar(0.1),maxD(5.0),
dsc(null<DSC*>()),dsc_threshold(1.0),dsc_width(0.25),
dsi(null<DSI*>())
{}

RefineNorm::~RefineNorm()
{}

void RefineNorm::Set(nat32 i,real32 sd,real32 cB,real32 nV,real32 mD)
{
 iters = i;
 dsiSd = sd;
 costBase = cB;
 normVar = nV;
 maxD = mD;
}

void RefineNorm::Set(const DSC & d,real32 threshold,real32 width)
{
 dsc = &d;
 dsc_threshold = threshold;
 dsc_width = width;
}

void RefineNorm::Set(const DSI & d)
{
 dsi = &d;
}

void RefineNorm::Set(const svt::Field<bs::Normal> & n)
{
 needle = n;
}

void RefineNorm::Set(const svt::Field<nat32> & s)
{
 seg = s;
}

void RefineNorm::Set(const cam::CameraPair & p)
{
 pair = p;
}

void RefineNorm::Run(time::Progress * prog)
{
 prog->Push();

 // Create the solver object, resize the output...
  prog->Report(0,7);
  inf::IntegrateBP ibp;
  ibp.Reset(dsi->Width(),dsi->Height());
  disp.Resize(dsi->Width(),dsi->Height());



 // First fill in the expectation for each pixels disparity, these
 // representing the probability of each disparity without consideration of
 // the needle map. A sumary of the DSI if you will...
  prog->Report(1,7);
  prog->Push();
  for (nat32 y=0;y<dsi->Height();y++)
  {
   prog->Report(y,dsi->Height());
   for (nat32 x=0;x<dsi->Width();x++)
   {
    if (dsi->Size(x,y)!=0)
    {
     // Find the minimum cost assigned to the pixel so we can zero it...
      real32 minCost = dsi->Cost(x,y,0);
      for (nat32 i=1;i<dsi->Size(x,y);i++) minCost = math::Min(minCost,dsi->Cost(x,y,i));

     // Calculate mean...
      real32 mean = 0.0;
      real32 sum = 0.0;
      for (nat32 i=0;i<dsi->Size(x,y);i++)
      {
       real32 weight = math::Exp(-costBase*(dsi->Cost(x,y,i)-minCost));
       mean += dsi->Disp(x,y,i)*weight;
       sum += weight;
      }
      mean /= sum;

     // Calculate sd, have to consider that the disparities represent ranges,
     // which kinda causes a mild headache...
      real32 sd = 0.0;
      for (nat32 i=0;i<dsi->Size(x,y);i++)
      {
       real32 dispSd = 0.5*(math::Abs(dsi->Disp(x,y,i)+dsi->DispWidth(x,y,i) - mean) +
                            math::Abs(dsi->Disp(x,y,i)-dsi->DispWidth(x,y,i) - mean));
       real32 weight = math::Exp(-costBase*(dsi->Cost(x,y,i)-minCost));
       sd += dispSd*weight;
      }
      sd /= sum;

     // Store the relevant output...
      ibp.SetVal(x,y,mean,1.0/sd);
      disp.Get(x,y) = mean;
    }
    else
    {
     ibp.SetVal(x,y,0.0,0.0);
     disp.Get(x,y) = math::Infinity<real32>(); // Used to indicate its unknown - such pixels revert to basic smoothing.
    }
   }
  }
  prog->Pop();



 // The following code needs to know the centre of the left camera...
  math::Vect<4,real64> leftCent;
  pair.lp.Centre(leftCent);

 // ... and the psuedo inverse of the left projection matrix post multiplied by
 // the unRectLeft matrix...
  math::Mat<4,3,real64> inverseLP;
  {
   math::Mat<4,3,real64> temp;
   math::PseudoInverse(pair.lp,temp);
   math::Mult(temp,pair.unRectLeft,inverseLP);
  }

 // ...and the two rectification matrices...
  math::Mat<3,3,real64> rectLeft;
  math::Mat<3,3,real64> rectRight;
  {
   math::Mat<3,3,real64> temp;

   rectLeft = pair.unRectLeft;
   math::Inverse(rectLeft,temp);

   rectRight = pair.unRectRight;
   math::Inverse(rectRight,temp);
  }



 // This array stores a bit for every message to be calculated from a pixel,
 // true to send the message, false to not. Encodes the segmentation and the
 // boundary...
  prog->Report(2,7);
  prog->Push();
  ds::Array2D<PreCalc> preCalc(dsi->Width(),dsi->Height());
  // Initialise to all true...
   prog->Report(0,3);
   for (nat32 y=0;y<preCalc.Height();y++)
   {
    for (nat32 x=0;x<preCalc.Width();x++)
    {
     for (nat32 i=0;i<4;i++) preCalc.Get(x,y).pass[i] = true;
    }
   }

  // Add boundary falses...
   prog->Report(1,3);
   for (nat32 y=0;y<preCalc.Height();y++)
   {
    preCalc.Get(0,y).pass[2] = false;
    preCalc.Get(preCalc.Width()-1,y).pass[0] = false;
   }

   for (nat32 x=0;x<preCalc.Width();x++)
   {
    preCalc.Get(x,0).pass[3] = false;
    preCalc.Get(x,preCalc.Height()-1).pass[1] = false;
   }

  // Add segmentation falses...
   prog->Report(2,3);
   if (seg.Valid())
   {
    for (nat32 y=0;y<preCalc.Height();y++)
    {
     for (nat32 x=0;x<preCalc.Width();x++)
     {
      if (preCalc.Get(x,y).pass[0]) preCalc.Get(x,y).pass[0] = seg.Get(x,y)==seg.Get(x+1,y);
      if (preCalc.Get(x,y).pass[1]) preCalc.Get(x,y).pass[1] = seg.Get(x,y)==seg.Get(x,y+1);
      if (preCalc.Get(x,y).pass[2]) preCalc.Get(x,y).pass[2] = seg.Get(x,y)==seg.Get(x-1,y);
      if (preCalc.Get(x,y).pass[3]) preCalc.Get(x,y).pass[3] = seg.Get(x,y)==seg.Get(x,y-1);
     }
    }
   }

  prog->Pop();



 // Iterate and fill in the invSd parameter to indicate the standard deviation
 // used for the difference when calculating each message...
 prog->Report(3,7);
 prog->Push();
 {
  real32 coMult = 1.0/normVar;
  byte * tempA = mem::Malloc<byte>(dsc?dsc->Bytes():0);
  byte * tempB = mem::Malloc<byte>(dsc?dsc->Bytes():0);
  for (nat32 y=0;y<preCalc.Height();y++)
  {
   prog->Report(y,preCalc.Height());
   for (nat32 x=0;x<preCalc.Width();x++)
   {
    // X direction...
     if (preCalc.Get(x,y).pass[0])
     {
      real32 val = coMult;
       if (dsc)
       {
        dsc->Left(x,y,tempA);
        dsc->Left(x+1,y,tempB);
        val *= math::SigmoidCutoff(dsc->Cost(tempA,tempB),dsc_threshold,dsc_width);
       }
      preCalc.Get(x,y).invSd[0] = val;
      preCalc.Get(x+1,y).invSd[2] = val;
     }

    // Y direction...
     if (preCalc.Get(x,y).pass[1])
     {
      real32 val = coMult;
       if (dsc)
       {
        dsc->Left(x,y,tempA);
        dsc->Left(x,y+1,tempB);
        val *= math::SigmoidCutoff(dsc->Cost(tempA,tempB),dsc_threshold,dsc_width);
       }
      preCalc.Get(x,y).invSd[1] = val;
      preCalc.Get(x,y+1).invSd[3] = val;
     }
   }
  }
  mem::Free(tempA);
  mem::Free(tempB);
 }
 prog->Pop();



 // Fill in the differences between disparities, and there confidence,
 // using the needle map and the initialisation disparities...
  prog->Report(4,7);
  prog->Push();
  for (nat32 y=0;y<dsi->Height();y++)
  {
   prog->Report(y,dsi->Height());
   for (nat32 x=0;x<dsi->Width();x++)
   {
    if (math::IsFinite(disp.Get(x,y)))
    {
     // Triangulate the current point, then re-project it back and update the
     // disparity. This is needed as there can be quite a lot of numerical
     // error, and the below code is very sensitive...
      math::Vect<4,real64> pos;
      pair.Triangulate(x,y,disp.Get(x,y),pos);
      pos /= pos[3];
      real32 oX,oY;
      pair.Project(pos,oX,oY,disp.Get(x,y),&rectLeft,&rectRight);

     // Create a plane going through the current point with the normal of the needle map...
      bs::Plane plane;
      plane.n = needle.Get(x,y);
      plane.d = -(pos[0]*plane.n[0] + pos[1]*plane.n[1] + pos[2]*plane.n[2]);

     // Iterate the neighbours...
      for (nat32 i=0;i<4;i++)
      {
       if (preCalc.Get(x,y).pass[i])
       {
        // Calculate the neighbours coordinates...
         int32 nx = int32(x);
         int32 ny = int32(y);
         switch (i)
         {
          case 0: nx += 1; break;
          case 1: ny += 1; break;
          case 2: nx -= 1; break;
          case 3: ny -= 1; break;
         }

        // Calculate a point on the neighbours line other than the centre...
         math::Vect<4,real64> nPos;
         {
          math::Vect<3,real64> loc;
          loc[0] = nx;
          loc[1] = ny;
          loc[2] = 1.0;

          math::MultVect(inverseLP,loc,nPos);
         }

        // Intercept the plane with the neighbours line...
         math::Vect<4,real64> inter;
         plane.LineIntercept(leftCent,nPos,inter);

        // Project the point of intersection back to the left image, take the
        // difference in x to get the disparity difference we desire...
         real32 prX,prY,prD;
         pair.Project(inter,prX,prY,prD,&rectLeft,&rectRight);
         real32 mean = prD - disp.Get(x,y);

         if (!math::IsFinite(mean)) ibp.SetRel(x,y,i,1.0,0.0,0.0);
         else
         {
          mean = math::Clamp(mean,-maxD,maxD);
          ibp.SetRel(x,y,i,1.0,mean,preCalc.Get(x,y).invSd[i]);
         }
       }
       else
       {
        ibp.SetRel(x,y,i,1.0,0.0,0.0);
       }
      }
    }
    else
    {
     // We don't know the disparity, we therefore can't work out the details -
     // simply set the changes to zero as a basic smoothing operation...
      for (nat32 i=0;i<4;i++) ibp.SetRel(x,y,i,1.0,0.0,preCalc.Get(x,y).invSd[i]);
    }
   }
  }
  prog->Pop();



 // Run...
  prog->Report(5,7);
  ibp.SetIters(iters);
  ibp.Run(prog);



 // Extract the final beliefs...
  prog->Report(6,7);
  prog->Push();
  for (nat32 y=0;y<disp.Height();y++)
  {
   prog->Report(y,disp.Height());
   for (nat32 x=0;x<disp.Width();x++)
   {
    if (ibp.Defined(x,y))
    {
     disp.Get(x,y) = ibp.Expectation(x,y);
    }
    else
    {
     disp.Get(x,y) = 0.0;
    }
   }
  }
  prog->Pop();

 prog->Pop();
}

void RefineNorm::Disp(svt::Field<real32> & out) const
{
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++) out.Get(x,y) = disp.Get(x,y);
 }
}

//------------------------------------------------------------------------------
GaussianDisp::~GaussianDisp()
{}

void GaussianDisp::SetDSI(const DSI & d)
{
 dsi = &d;
}

//------------------------------------------------------------------------------
GaussianDispDiff::GaussianDispDiff()
:cacheX(0xFFFFFFFF),cacheY(0xFFFFFFFF),cacheD(0.0)
{}

GaussianDispDiff::~GaussianDispDiff()
{}

void GaussianDispDiff::SetPair(const cam::CameraPair & p)
{
 pair = p;
 
 // Create all the suport information derived from the pair, to save on
 // repeated calculation...
  // Centre of left camera...
   pair.lp.Centre(leftCentre);
   
  // Psuedo inverse of left camera projection matrix (With unRectLeft factored in)...
   math::Mat<4,3,real64> temp43;
   pair.lp.GetInverse(temp43);   
   math::Mult(temp43,pair.unRectLeft,inverseLP);

  // Rectification matrices...
   math::Mat<3,3,real64> temp33;

   rectLeft = pair.unRectLeft;
   math::Inverse(rectLeft,temp33);

   rectRight = pair.unRectRight;
   math::Inverse(rectRight,temp33);
   
  // Sillify the cache...
   cacheX = 0xFFFFFFFF;
   cacheY = 0xFFFFFFFF;
   cacheD = math::Infinity<real32>();
   cachePos[0] = 0.0;
   cachePos[1] = 0.0;
   cachePos[2] = 0.0;
   cachePos[3] = 1.0;
}

real32 GaussianDispDiff::NormToDiff(nat32 x,nat32 y,nat32 dir,
                                  real32 disp,const bs::Normal & norm) const
{
 // Calculate the pos, unless its cached...
  math::Vect<4,real64> pos;
  if ((x==cacheX)&&(y==cacheY)&&math::Equal(cacheD,disp))
  {
   pos = cachePos;
  }
  else
  {
   pair.Triangulate(x,y,disp,pos);   pos /= pos[3];
   
   cacheX = x;
   cacheY = y;
   cacheD = disp;
   cachePos = pos;
  }


 // Create a plane going through the current point with the given normal...
  bs::Plane plane;
  plane.n = norm;
  plane.d = -(pos[0]*plane.n[0] + pos[1]*plane.n[1] + pos[2]*plane.n[2]);


 // Calculate the neighbours coordinates...
  int32 nx = int32(x);
  int32 ny = int32(y);
  switch (dir)
  {
   case 0: nx += 1; break;
   case 1: ny += 1; break;
   case 2: nx -= 1; break;
   case 3: ny -= 1; break;
  }


 // Calculate a point on the neighbours line other than the centre...
  math::Vect<4,real64> nPos;
  {
   math::Vect<3,real64> loc;
   loc[0] = nx;
   loc[1] = ny;
   loc[2] = 1.0;

   math::MultVect(inverseLP,loc,nPos);
  }


 // Intercept the plane with the neighbours line...
  math::Vect<4,real64> inter;
  plane.LineIntercept(leftCentre,nPos,inter);


 // Project the point of intersection back to the left image, take the
 // difference in x to get the disparity difference we desire...
  real32 prX,prY,prD;
  pair.Project(inter,prX,prY,prD,&rectLeft,&rectRight);
  return prD - disp;
}

//------------------------------------------------------------------------------
GaussRefine::GaussRefine()
:gd(null<GaussianDisp*>()),gdd(null<GaussianDispDiff*>()),iters(100)
{}

GaussRefine::~GaussRefine()
{}

void GaussRefine::Set(const GaussianDisp & g)
{
 gd = &g;
}

void GaussRefine::Set(const GaussianDispDiff & g)
{
 gdd = &g;
}

void GaussRefine::Set(nat32 i)
{
 iters = i;
}

void GaussRefine::Run(time::Progress * prog)
{
 // Prep the solver...
  inf::IntegrateBP ibp(gd->Width(),gd->Height());
  for (nat32 y=0;y<gd->Height();y++)
  {
   for (nat32 x=0;x<gd->Width();x++)
   {
    // Disparity estimate...
     real32 mean;
     real32 invSd;
     if (gd->GetDisp(x,y,mean,invSd)) ibp.SetVal(x,y,mean,invSd);

    // Differences of disparity estiamtes...
     for (nat32 d=0;d<4;d++)
     {
      real32 diff;
      real32 invSd;
      if (gdd->GetDispDiff(x,y,d,mean,diff,invSd)) ibp.SetRel(x,y,d,1.0,diff,invSd);
     }
   }
  }
  
  ibp.SetIters(iters);
  
 
 // Solve...
  ibp.Run(prog);


 // Extract the results...
  mask.Resize(gd->Width(),gd->Height());
  disp.Resize(gd->Width(),gd->Height());

  for (nat32 y=0;y<gd->Height();y++)
  {
   for (nat32 x=0;x<gd->Width();x++)
   {
    if (ibp.Defined(x,y))
    {
     disp.Get(x,y) = ibp.Expectation(x,y);
     if (math::IsFinite(disp.Get(x,y)))
     {
      mask.Get(x,y) = true;
     }
     else
     {
      mask.Get(x,y) = false;
      disp.Get(x,y) = 0.0;
     }
    }
    else
    {
     mask.Get(x,y) = false;
     disp.Get(x,y) = 0.0;
    }
   }
  }
}

void GaussRefine::MaskMap(svt::Field<bit> & out) const
{
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++)
  {
   out.Get(x,y) = mask.Get(x,y);
  }
 }
}

bit GaussRefine::Mask(nat32 x,nat32 y) const
{
 return mask.Get(x,y);
}

void GaussRefine::DispMap(svt::Field<real32> & out) const
{
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++)
  {
   out.Get(x,y) = disp.Get(x,y);
  }
 }
}

real32 GaussRefine::Disp(nat32 x,nat32 y) const
{
 return disp.Get(x,y);
}

//------------------------------------------------------------------------------
GaussianDispSimple::GaussianDispSimple()
{}

GaussianDispSimple::~GaussianDispSimple()
{}

void GaussianDispSimple::SetMode(bit m)
{
 mode = m;
}

bit GaussianDispSimple::GetDisp(nat32 x,nat32 y,real32 & mean,real32 & invSd) const
{
 // Check there is information to be used...
  if (dsi->Size(x,y)==0) return false;


 // First calculate the minimum cost, needed for numerical stability, used for 
 // either just the sd or both the sd and mean calculation...
  real32 minCost = dsi->Cost(x,y,0);
  for (nat32 i=0;i<dsi->Size(x,y);i++) minCost = math::Min(minCost,dsi->Cost(x,y,i));


 // Calculate the mean...
  if (mode)
  {
   mean = dsi->Disp(x,y,0);
   real32 bestCost = dsi->Cost(x,y,0);
   for (nat32 i=1;i<dsi->Size(x,y);i++)
   {
    if (dsi->Cost(x,y,i)<bestCost)
    {
     mean = dsi->Disp(x,y,i);
     bestCost = dsi->Cost(x,y,i);
    }
   }
  }
  else
  {
   // Calculate the mean, weighting by cost, using the minimum cost for stability...
    mean = 0.0;
    real32 sum = 0.0;
    for (nat32 i=0;i<dsi->Size(x,y);i++)
    {
     real32 weight = math::Exp(-(dsi->Cost(x,y,i)-minCost));
     mean += weight * dsi->Disp(x,y,i);
     sum += weight;
    }
    mean /= sum;
  }


 // Calculate the inverse standard deviation...
  invSd = 0.0;
  real32 sum = 0.0;
  for (nat32 i=0;i<dsi->Size(x,y);i++)
  {
   real32 weight = math::Exp(-(dsi->Cost(x,y,i)-minCost));
   real32 dispSd = 0.5*(math::Abs(dsi->Disp(x,y,i)+dsi->DispWidth(x,y,i) - mean) +
                        math::Abs(dsi->Disp(x,y,i)-dsi->DispWidth(x,y,i) - mean));
   invSd += dispSd*weight;
   sum += weight;
  }
  invSd = sum/invSd;
 
 return true;
}

//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
GaussianDispDiffSimple::GaussianDispDiffSimple()
:baseInvSd(1.0),threshold(40.0),halflife(4.0)
{}

GaussianDispDiffSimple::~GaussianDispDiffSimple()
{}

void GaussianDispDiffSimple::SetNeedle(const svt::Field<bs::Normal> & n)
{
 needle = n;
}

void GaussianDispDiffSimple::SetInvSd(real32 invSd)
{
 baseInvSd = invSd;
}

void GaussianDispDiffSimple::SetImageMod(const svt::Field<bs::ColourLuv> & i,real32 t,real32 h)
{
 image = i;
 threshold = t;
 halflife = h;
}

bit GaussianDispDiffSimple::GetDispDiff(nat32 x,nat32 y,nat32 dir,real32 disp,
                                        real32 & diff,real32 & invSd) const
{
 bs::Normal targ = needle.Get(x,y);
 real32 length = targ.Length();
 if (math::IsZero(length)) return false;
 targ /= length;
 
 diff = NormToDiff(x,y,dir,disp,targ);
 
 invSd = baseInvSd * length;
 
 if (image.Valid())
 {
  bit cont = true;
  nat32 nx = x;
  nat32 ny = y;
  switch (dir)
  {
   case 0: cont = x<(image.Size(0)-1); ++nx; break;
   case 1: cont = y<(image.Size(1)-1); ++ny; break;
   case 2: cont = x>0; --nx; break;
   case 3: cont = y>0; --ny; break;
  }

  if (cont)
  {
   real32 dist  = math::Abs(image.Get(x,y).l - image.Get(nx,ny).l);
          dist += math::Abs(image.Get(x,y).u - image.Get(nx,ny).u);
          dist += math::Abs(image.Get(x,y).v - image.Get(nx,ny).v); 
   
   invSd *= math::SigmoidCutoff(dist,threshold,halflife);
  }
 }

 return true;
}

//------------------------------------------------------------------------------
 };
};
