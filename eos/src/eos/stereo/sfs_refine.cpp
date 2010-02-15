//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/stereo/sfs_refine.h"

#include "eos/math/stats.h"
#include "eos/filter/image_io.h"
#include "eos/rend/graphs.h"
#include "eos/inf/gauss_integration.h"
#include "eos/alg/fitting.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
AlbedoEstimate::AlbedoEstimate()
:light(0.0,0.0,0.0,1.0),surSeg(null<SurSeg*>()),debug(false)
{}

AlbedoEstimate::~AlbedoEstimate()
{}

void AlbedoEstimate::SetLight(bs::Vertex & l)
{
 light = l;
}

void AlbedoEstimate::SetImage(const svt::Field<bs::ColourLuv> & im)
{
 image = im;
}

void AlbedoEstimate::SetMask(const svt::Field<bit> & m)
{
 mask = m;
}

void AlbedoEstimate::SetSurSeg(const SurSeg & ss)
{
 surSeg = &ss;
}

void AlbedoEstimate::Run(time::Progress * prog)
{
 LogBlock("eos::stereo::AlbedoEstimate::Run","-");
 prog->Push();

 // Fill in some density estimators, one per segment, entry per pixel...
  prog->Report(0,debug?3:2);

  ds::ArrayDel<math::UniDensityEstimate> est(surSeg->Segments());
  /*ds::Array<nat32> samples(surSeg->Segments());
  ds::Array<real32> mean(surSeg->Segments());
  for (nat32 i=0;i<samples.Size();i++)
  {
   samples[i] = 0;
   mean[i] = 0.0;
  }*/

  prog->Push();
   for (nat32 y=0;y<surSeg->Height();y++)
   {
    prog->Report(y,surSeg->Height());
    for (nat32 x=0;x<surSeg->Width();x++)
    {
     // Skip segment 0 - its where all the bad pixels go...
      if (surSeg->Seg(x,y)==0) continue;

     /// If we have a mask and the mask entry is true skip it...
      if ((mask.Valid())&&(mask.Get(x,y))) continue;

     // Calculate direction to light source from point on surface...
      bs::Vertex pos;
      surSeg->Pos(x,y,pos);

      math::Mat<4> plucker;
      math::Plucker(light,pos,plucker);

     // Dot product with surface orientation, skip those facing away...
      bs::Normal norm;
      surSeg->Norm(x,y,norm);

      math::Vect<3> dir;
      math::PluckerDir(plucker,dir);
      dir.Normalise();
      //if (dir[2]<0.0) continue; // ******************************************** bad def.
      //LogDebug("dir,norm" << LogDiv() << dir << LogDiv() << norm);

      real32 dot = dir * norm;
      if (dot<0.0) dot *= -1.0;

     // Calculate the albedo for this pixel...
      real32 alb = image.Get(x,y).l/dot;
      if (alb>200.0) continue; // A cap on how high it will go, for sanity.
      //LogDebug("dot,alb" << LogDiv() << dot << LogDiv() << alb);

     // Add to estimator...
      //samples[surSeg->Seg(x,y)] += 1;
      //mean[surSeg->Seg(x,y)] += (alb-mean[surSeg->Seg(x,y)])/real32(samples[surSeg->Seg(x,y)]);
      est[surSeg->Seg(x,y)].Add(alb);
    }
   }
  prog->Pop();


 // Go through and extract the maximal modes...
  prog->Report(1,debug?3:2);
  albedo.Size(surSeg->Segments());
  prog->Push();
   albedo[0] = 0.0;
   for (nat32 i=1;i<albedo.Size();i++)
   {
    prog->Report(i,albedo.Size());
    if (est[i].Samples()!=0)
    {
     est[i].Run();
     albedo[i] = est[i].MaxMode();
    }
    else albedo[i] = 0.0;
    //albedo[i] = mean[i];
   }
  prog->Pop();


 // If we are in debug mode then create and save to file a bunch of graphs...
  /*if (debug)
  {
   prog->Report(2,3);

   str::TokenTable tempTT;
   svt::Core tempCore(tempTT);

   for (nat32 i=0;i<est.Size();i++)
   {
    math::Vector<real32> * samples = est[i].GetSamples(400);
    svt::Var * image = rend::RenderSimpleGraph(tempCore,samples);

    str::String s;
    s << "density_graph_" << i << ".bmp";

    cstr fn = s.ToStr();
    filter::SaveImageRGB(image,fn,true);
    mem::Free(fn);

    delete image;
    delete samples;

    LogDebug("Albedo estimate {i,samples,lowest,highest,max,albedo}" << LogDiv()
             << i << LogDiv() << est[i].Samples() << LogDiv()
             << est[i].Lowest() << LogDiv() << est[i].Highest() << LogDiv()
             << est[i].F(albedo[i]) << LogDiv() << albedo[i]);
   }
  }*/

 prog->Pop();
}

real32 AlbedoEstimate::Albedo(nat32 seg) const
{
 return albedo[seg];
}

//------------------------------------------------------------------------------
AlbedoEstimate2::AlbedoEstimate2()
:light(0.0,0.0,0.0,1.0),surSeg(null<SurSeg*>()),
surSd(2.0),eqSd(0.1),diffSd(0.25),diffCutoff(5.0),iters(100)
{}

AlbedoEstimate2::~AlbedoEstimate2()
{}

void AlbedoEstimate2::SetLight(bs::Vertex & l)
{
 light = l;
}

void AlbedoEstimate2::SetImage(const svt::Field<bs::ColourLuv> & i)
{
 image = i;
}

void AlbedoEstimate2::SetMask(const svt::Field<bit> & m)
{
 mask = m;
}

void AlbedoEstimate2::SetSurSeg(const SurSeg & ss)
{
 surSeg = &ss;
}

void AlbedoEstimate2::Set(real32 ss,real32 es,real32 ds,real32 dc,nat32 i)
{
 surSd = ss;
 eqSd = es;
 diffSd = ds;
 diffCutoff = dc;
 iters = i;
}

void AlbedoEstimate2::Run(time::Progress * prog)
{
 prog->Push();

 // Create the solver object...
  inf::IntegrateBP ibp;
  ibp.Reset(surSeg->Width(),surSeg->Height());


 // Basics...
  real32 invSurSd = 1.0/surSd;
  real32 invEqSd = 1.0/eqSd;
  real32 invDiffSd = 1.0/diffSd;


 // Use the surface, image and light source to calculate an albedo for every
 // pixel, store in solver...
  prog->Report(0,4);
  prog->Push();
  for (nat32 y=0;y<ibp.Height();y++)
  {
   prog->Report(y,ibp.Height());
   for (nat32 x=0;x<ibp.Width();x++)
   {
    // Skip segment 0 - its where all the bad pixels go...
     if (surSeg->Seg(x,y)==0) continue;

    // If we have a mask and the mask entry is true skip it...
     if ((mask.Valid())&&(mask.Get(x,y))) continue;

    // Calculate direction to light source from point on surface...
     bs::Vertex pos;
     surSeg->Pos(x,y,pos);

     math::Mat<4> plucker;
     math::Plucker(light,pos,plucker);

    // Dot product with surface orientation, skip those facing away...
     bs::Normal norm;
     surSeg->Norm(x,y,norm);

     math::Vect<3> dir;
     math::PluckerDir(plucker,dir);
     dir.Normalise();
     if (dir[2]<0.0) continue;

     real32 dot = dir * norm;
     if (dot<0.0) dot *= -1.0;

    // Calculate the albedo for this pixel, store in integrator...
     real32 alb = image.Get(x,y).l/dot;
     ibp.SetVal(x,y,alb,invSurSd*math::Exp(-alb*0.01));
   }
  }
  prog->Pop();


 // For every difference pair calculate the relevant distribution, either a
 // dosa or a soda, and produce the relevant parameters...
  prog->Report(1,4);
  prog->Push();
  for (nat32 y=0;y<ibp.Height();y++)
  {
   prog->Report(y,ibp.Height());
   for (nat32 x=0;x<ibp.Width();x++)
   {
    // Horizontal...
     if (x+1<ibp.Width())
     {
      bit soda = false;
      if (surSeg->Seg(x,y)!=surSeg->Seg(x+1,y)) soda = true;
      real32 uvDiffSqr = math::Sqr(image.Get(x,y).u-image.Get(x+1,y).u) +
                         math::Sqr(image.Get(x,y).v-image.Get(x+1,y).v);
      if (uvDiffSqr>math::Sqr(diffCutoff)) soda = true;

      if (soda)
      {
       // soda...
        real32 ratio = image.Get(x+1,y).l / image.Get(x,y).l;
        if (math::IsFinite(ratio)&&math::IsFinite(1.0/ratio)&&
            (ratio<max_ratio)&&((1.0/ratio)<max_ratio))
        {
         ibp.SetRel(x,y,0,ratio,0.0,invDiffSd);
         ibp.SetRel(x+1,y,2,1.0/ratio,0.0,invDiffSd);
        }
      }
      else
      {
       // dosa...
        ibp.SetRel(x,y,0,1.0,0.0,invEqSd);
        ibp.SetRel(x+1,y,2,1.0,0.0,invEqSd);
      }
     }

    // Vertical...
     if (y+1<ibp.Height())
     {
      bit soda = false;
      if (surSeg->Seg(x,y)!=surSeg->Seg(x,y+1)) soda = true;
      real32 uvDiffSqr = math::Sqr(image.Get(x,y).u-image.Get(x,y+1).u) +
                         math::Sqr(image.Get(x,y).v-image.Get(x,y+1).v);
      if (uvDiffSqr>math::Sqr(diffCutoff)) soda = true;

      if (soda)
      {
       // soda...
        real32 ratio = image.Get(x,y+1).l / image.Get(x,y).l;
        ibp.SetRel(x,y,1,ratio,0.0,invDiffSd);
        ibp.SetRel(x,y+1,3,1.0/ratio,0.0,invDiffSd);
      }
      else
      {
       // dosa...
        ibp.SetRel(x,y,1,1.0,0.0,invEqSd);
        ibp.SetRel(x,y+1,3,1.0,0.0,invEqSd);
      }
     }
   }
  }
  prog->Pop();


 // Solve...
  prog->Report(2,4);
  ibp.SetIters(iters);
  ibp.Run(prog);


 // Extract the results...
  prog->Report(3,4);
  albedo.Resize(surSeg->Width(),surSeg->Height());
  for (nat32 y=0;y<ibp.Height();y++)
  {
   for (nat32 x=0;x<ibp.Width();x++)
   {
    if (ibp.Defined(x,y)) albedo.Get(x,y) = math::Max(ibp.Expectation(x,y),real32(0.0));
                     else albedo.Get(x,y) = 0.0;
   }
  }


 prog->Pop();
}

real32 AlbedoEstimate2::Albedo(nat32 x,nat32 y) const
{
 return albedo.Get(x,y);
}

void AlbedoEstimate2::GetAlbedo(svt::Field<real32> & out) const
{
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++) out.Get(x,y) = albedo.Get(x,y);
 }
}

//------------------------------------------------------------------------------
AlbedoEstimate3::AlbedoEstimate3()
:light(0.0,0.0,0.0,1.0),surSeg(null<SurSeg*>()),
winRad(5)
{}

AlbedoEstimate3::~AlbedoEstimate3()
{}

void AlbedoEstimate3::SetLight(bs::Vertex & l)
{
 light = l;
}

void AlbedoEstimate3::SetImage(const svt::Field<bs::ColourLuv> & i)
{
 image = i;
}

void AlbedoEstimate3::SetMask(const svt::Field<bit> & m)
{
 mask = m;
}

void AlbedoEstimate3::SetSurSeg(const SurSeg & ss)
{
 surSeg = &ss;
}

void AlbedoEstimate3::Set(nat32 wR)
{
 winRad = wR;
}

void AlbedoEstimate3::Run(time::Progress * prog)
{
 prog->Push();

 // Create a counter/running mean pair for each segment...
  ds::Array<nat32> samples(surSeg->Segments());
  ds::Array<real32> mean(samples.Size());
  for (nat32 i=0;i<samples.Size();i++)
  {
   samples[i] = 0;
   mean[i] = 0.0;
  }


 // Iterate all pixels, fit planes and update the running means...
  for (int32 y=0;y<int32(image.Size(1));y++)
  {
   prog->Report(y,image.Size(1));
   for (int32 x=0;x<int32(image.Size(0));x++)
   {
    // Check if the pixel should be considered...
     nat32 curSeg = surSeg->Seg(x,y);
     if (curSeg==0) continue;
     if ((mask.Valid())&&(mask.Get(x,y))) continue;

    // Sum all surrounding pixels into a plane fitter...
     alg::PlaneFit pf;
     for (int32 v=math::Max<int32>(y-winRad,0);v<=math::Min<int32>(y+winRad,int32(image.Size(1)-1));v++)
     {
      for (int32 u=math::Max<int32>(x-winRad,0);u<=math::Min<int32>(x+winRad,int32(image.Size(0)-1));u++)
      {
       if (surSeg->Seg(u,v)!=curSeg) continue;

       bs::Vertex pos;
       surSeg->Pos(u,v,pos);
       pos /= pos[3];

       pf.Add(pos[0],pos[1],pos[2]);
      }
     }

    // Verify enough points have been obtained, if not don't count it...
     if (!pf.Valid()) continue;

    // Calculate the direction to the light source...
     bs::Normal toLight;
     {
      bs::Vertex pos;
      surSeg->Pos(x,y,pos);

      math::Mat<4> plucker;
      math::Plucker(light,pos,plucker);

      math::PluckerDir(plucker,toLight);
      toLight.Normalise();
     }

    // Calculate the albedo estimate...
     bs::PlaneABC abc;
     pf.Get(abc);
     bs::Plane plane(abc);
     plane.Normalise();
     real32 albedo = image.Get(x,y).l / math::Abs(toLight*plane.n);

    // Only let through sane estimates...
     if (!math::IsFinite(albedo)) continue;
     if (albedo>200.0) continue;

    // Add the albedo estimate to the relevant segment estimate...
     samples[curSeg] += 1;
     mean[curSeg] += (albedo-mean[curSeg])/real32(samples[curSeg]);
   }
  }


 // Write out the final means to the albedo map...
  albedo.Resize(surSeg->Width(),surSeg->Height());
  for (nat32 y=0;y<albedo.Height();y++)
  {
   for (nat32 x=0;x<albedo.Width();x++)
   {
    albedo.Get(x,y) = mean[surSeg->Seg(x,y)];
   }
  }


 prog->Pop();
}

real32 AlbedoEstimate3::Albedo(nat32 x,nat32 y) const
{
 return albedo.Get(x,y);
}

void AlbedoEstimate3::GetAlbedo(svt::Field<real32> & out) const
{
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++) out.Get(x,y) = albedo.Get(x,y);
 }
}

//------------------------------------------------------------------------------
SurfaceSfS::SurfaceSfS()
:iters(200),ss(null<SurSeg*>()),bgSeg(nat32(-1))
{}

SurfaceSfS::~SurfaceSfS()
{}

void SurfaceSfS::Set(nat32 i)
{
 iters = i;
}

void SurfaceSfS::Set(const svt::Field<bs::ColourLuv> & i)
{
 image = i;
}

void SurfaceSfS::Set(const SurSeg & s)
{
 ss = &s;
}

void SurfaceSfS::SetBG(nat32 index)
{
 bgSeg = index;
}

void SurfaceSfS::Set(const bs::Vertex & l)
{
 light = l;
}

void SurfaceSfS::Set(const svt::Field<real32> a)
{
 ae = a;
}

void SurfaceSfS::Run(time::Progress * prog)
{
 prog->Push();

 // Setup data correctly, initialise the surface normals from the fitted surface
 // normals...
  prog->Report(0,iters*2+4);
  data.Resize(ss->Width(),ss->Height());
  ds::Array2D<bs::Normal> temp(ss->Width(),ss->Height());
  for (nat32 y=0;y<ss->Height();y++)
  {
   for (nat32 x=0;x<ss->Width();x++) ss->Norm(x,y,temp.Get(x,y));
  }


 // Generate a mask of pixels whose surface normals must not be editted...
  prog->Report(1,iters*2+4);
  ds::Array2D<bit> mask(ss->Width(),ss->Height());
  for (nat32 y=0;y<ss->Height();y++)
  {
   for (nat32 x=0;x<ss->Width();x++) mask.Get(x,y) = false;
  }

  for (nat32 y=0;y<ss->Height();y++)
  {
   for (nat32 x=0;x<ss->Width();x++)
   {
    if (ss->Seg(x,y)==bgSeg)
    {
     mask.Get(x,y) = true;

     if (x!=0) mask.Get(x-1,y) = true;
     if (x+1!=ss->Width()) mask.Get(x+1,y) = true;

     if (y!=0) mask.Get(x,y-1) = true;
     if (y+1!=ss->Height()) mask.Get(x,y+1) = true;
    }
    else
    {
     if (ss->Seg(x,y)==0)
     {
      mask.Get(x,y) = true;
     }
    }
   }
  }


 // Set boundary pixels to point perpendicular to the camera in the edge
 // direction and background pixels to point directly at the camera - they
 // shall then remain like this...
  prog->Report(2,iters*2+4);
  static const int32 winSize = 2;
  for (nat32 y=0;y<ss->Height();y++)
  {
   for (nat32 x=0;x<ss->Width();x++)
   {
    if (mask.Get(x,y))
    {
     if ((ss->Seg(x,y)==bgSeg)||(ss->Seg(x,y)==0))
     {
      // Background pixel - set to point straight at the camera...
       temp.Get(x,y) = bs::Normal(0.0,0.0,1.0);
     }
     else
     {
      // Find mean of segment pixels in window, set normal as opposite direction...
       math::Vect<2> meanSeg;
       meanSeg[0] = 0.0;
       meanSeg[1] = 0.0;

       for (int32 v=math::Max<int32>(int32(y)-winSize,0);v<=math::Min(int32(y)+winSize,int32(ss->Height()-1));v++)
       {
        for (int32 u=math::Max<int32>(int32(x)-winSize,0);u<=math::Min(int32(x)+winSize,int32(ss->Width()-1));u++)
        {
         if (ss->Seg(u,v)==ss->Seg(x,y))
         {
          meanSeg[0] += u-real32(x);
          meanSeg[1] += v-real32(y);
         }
        }
       }

       temp.Get(x,y)[0] = -meanSeg[0];
       temp.Get(x,y)[1] = -meanSeg[1];
       temp.Get(x,y)[2] = 0.0;
       if (math::IsZero(temp.Get(x,y).LengthSqr()))
       {
        temp.Get(x,y) = bs::Normal(0.0,0.0,1.0);
        mask.Get(x,y) = false;
       }
       else temp.Get(x,y).Normalise();
     }
    }
   }
  }


 // Calculate the toLight vector for every pixel and store in an array...
  ds::Array2D<bs::Normal> toLight(ss->Width(),ss->Height());
  for (nat32 y=0;y<ss->Height();y++)
  {
   for (nat32 x=0;x<ss->Width();x++)
   {
    if (!mask.Get(x,y))
    {
     bs::Vertex pos;
     ss->Pos(x,y,pos);

     math::Mat<4> plucker;
     math::Plucker(light,pos,plucker);

     math::PluckerDir(plucker,toLight.Get(x,y));
     toLight.Get(x,y).Normalise();
     if (toLight.Get(x,y)[2]<0.0) toLight.Get(x,y) *= -1.0;
    }
    else
    {
     toLight.Get(x,y) = bs::Normal(0.0,0.0,1.0);
    }
   }
  }


 // Iterate through all the iterations - we need an extra correction step before
 // the first smoothing step...
  prog->Report(3,iters*2+4);
  CorrectStep(temp,data,mask,toLight);

  for (nat32 i=0;i<iters;i++)
  {
   prog->Report(4+i*2,iters*2+4);
   SmoothStep(data,temp,mask);

   prog->Report(5+i*2,iters*2+4);
   CorrectStep(temp,data,mask,toLight);
  }


 prog->Pop();
}

const bs::Normal & SurfaceSfS::Orient(nat32 x,nat32 y)
{
 return data.Get(x,y);
}

void SurfaceSfS::CorrectStep(ds::Array2D<bs::Normal> & from,ds::Array2D<bs::Normal> & to,
                             ds::Array2D<bit> & mask,const ds::Array2D<bs::Normal> & toLight)
{
 for (nat32 y=0;y<from.Height();y++)
 {
  for (nat32 x=0;x<from.Width();x++)
  {
   // Skip masked segments - its where all the bad pixels go...
    if (mask.Get(x,y))
    {
     to.Get(x,y) = from.Get(x,y);
     continue;
    }

   // Calculate the sfs derived correct angle between toLight and
   // the surface normal...
    real32 alb = ae.Get(x,y);
    if (math::IsZero(alb))
    {
     to.Get(x,y) = from.Get(x,y);
     continue;
    }

    real32 cosAng = math::Min(image.Get(x,y).l / alb,real32(1.0));
    real32 coneAng = math::InvCos(cosAng);

   // Calculate an axis-of-rotation that will re-project as needed...
    bs::Normal axis;
    CrossProduct(from.Get(x,y),toLight.Get(x,y),axis);
    if (math::IsZero(axis.LengthSqr()))
    {
     to.Get(x,y) = toLight.Get(x,y);
    }
    else
    {
     axis.Normalise();

     // Apply the axis-of-rotation. Gain +1 mana...
      math::Mat<3,3> rotMat;
      AngAxisToRotMat(axis,-coneAng,rotMat);
      MultVect(rotMat,toLight.Get(x,y),to.Get(x,y));
      to.Get(x,y).Normalise();
    }

   // Check if the normal is pointing away from the camera - if so this is
   // impossible and needs to be rectified...
    if (to.Get(x,y)[2]<0.0)
    {
     // A rather primative approach, but good enough as further iterations
     // should then fix it...
      to.Get(x,y)[2] = 0.0;
      to.Get(x,y).Normalise();
    }
  }
 }
}

void SurfaceSfS::SmoothStep(ds::Array2D<bs::Normal> & from,ds::Array2D<bs::Normal> & to,ds::Array2D<bit> & mask)
{
 for (nat32 y=0;y<from.Height();y++)
 {
  for (nat32 x=0;x<from.Width();x++)
  {
   // Ignore masked pixels...
    if (mask.Get(x,y))
    {
     to.Get(x,y) = from.Get(x,y);
     continue;
    }

   // Get the segment...
    nat32 seg = ss->Seg(x,y);

   // The smoothing proccess...
    to.Get(x,y) = from.Get(x,y); //bs::Normal(0.0,0.0,0.0);
    to.Get(x,y) *= 3.0; // Momentum term. **************************************************

    if ((x!=0)&&(ss->Seg(x-1,y)==seg)) to.Get(x,y) += from.Get(x-1,y);
    if ((x!=from.Width()-1)&&(ss->Seg(x+1,y)==seg)) to.Get(x,y) += from.Get(x+1,y);
    if ((y!=0)&&(ss->Seg(x,y-1)==seg)) to.Get(x,y) += from.Get(x,y-1);
    if ((y!=from.Height()-1)&&(ss->Seg(x,y+1)==seg)) to.Get(x,y) += from.Get(x,y+1);

    if (math::IsZero(to.Get(x,y).LengthSqr())) to.Get(x,y) = bs::Normal(0.0,0.0,1.0);
                                          else to.Get(x,y).Normalise();
  }
 }
}

//------------------------------------------------------------------------------
 };
};
