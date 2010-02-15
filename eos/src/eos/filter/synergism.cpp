//-----------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/filter/synergism.h"

#include "eos/filter/conversion.h"
#include "eos/filter/kernel.h"
#include "eos/filter/edge_confidence.h"
#include "eos/filter/segmentation.h"
#include "eos/alg/mean_shift.h"

namespace eos
{
 namespace filter
 {
//-----------------------------------------------------------------------------
void Synergism::Run(time::Progress * prog)
{
 prog->Push();
 prog->Report(0,3);


 // Create the extra storage we require, all of it...
  out = new svt::Var(image);
   real32 greekIni = 0.0;
   out->Add("eta",greekIni);
   out->Add("rho",greekIni);
   out->Add("weight",greekIni);
   bs::ColourLuv smIni(0.0,0.0,0.0);
   out->Add("smooth",smIni);
   nat32 segsIni = 0;
   out->Add("segs",segsIni);
  out->Commit();


 // Make fields for all of the newly created data, ready for use...
  svt::Field<real32> greyReal; grey.SubField(0,greyReal);
  svt::Field<real32> eta; out->ByName("eta",eta);
  svt::Field<real32> rho; out->ByName("rho",rho);  
  svt::Field<real32> weight; out->ByName("weight",weight);
  svt::Field<bs::ColourLuv> smooth; out->ByName("smooth",smooth);  
  svt::Field<nat32> segs; out->ByName("segs",segs);


 // Calculate the two edge parameters from the greyscale...
  filter::KernelVect kernel(diffRad);
  filter::EdgeConfidenceKernel(kernel,diffRad);
  filter::EdgeConfidence(greyReal,kernel,eta,rho);


 // Using the edge parameters calculate the weight map...
  for (nat32 y=0;y<weight.Size(1);y++)
  {
   for (nat32 x=0;x<weight.Size(0);x++)
   {
    real32 r = rho.Get(x,y);
    real32 e = eta.Get(x,y);
    if (r <= 0.02) weight.Get(x,y) = 1.0;
              else weight.Get(x,y) = 1.0 - (edgeMix*r + (1.0-edgeMix)*e);
   }
  }


 // Do the mean shift to generate the smoothed image, the most time consuming part of the proccess...
  prog->Report(1,3);

  svt::Field<real32> lImg;
  svt::Field<real32> uImg;
  svt::Field<real32> vImg;
  image.SubField(0,lImg);
  image.SubField(sizeof(real32),uImg);
  image.SubField(sizeof(real32)*2,vImg);

  svt::Field<real32> lSmo;
  svt::Field<real32> uSmo;
  svt::Field<real32> vSmo;
  smooth.SubField(0,lSmo);
  smooth.SubField(sizeof(real32),uSmo);
  smooth.SubField(sizeof(real32)*2,vSmo);

  alg::MeanShift meanShift;
   meanShift.SetWeight(weight);
   meanShift.AddFeature(0,1.0/spatialSize);
   meanShift.AddFeature(1,1.0/spatialSize);   
   meanShift.AddFeature(lImg,1.0/rangeSize);
   meanShift.AddFeature(uImg,1.0/rangeSize);
   meanShift.AddFeature(vImg,1.0/rangeSize);
   meanShift.SetDistance(&Distance,0.0);
   meanShift.SetCutoff(0.01,100); // *0.01
   meanShift.Passover(true);

  meanShift.Run(prog);

  meanShift.Get(lImg,lSmo);
  meanShift.Get(uImg,uSmo);
  meanShift.Get(vImg,vSmo);
  
  
 // Generate the segmentation...
  prog->Report(2,3);

  filter::Segmenter segy;
   segy.SetCutoff(mergeCutoff);
   segy.SetMinimum(minSegment);
   segy.SetAverageSteps(averageSteps);
   //segy.SetMergeWeight(weight,1.0-mergeMax); // Too slow!
   
   segy.AddField(lSmo);
   segy.AddField(uSmo);
   segy.AddField(vSmo);      

  segy.Run(prog);

  segy.GetOutput(segs);
  segments = segy.Segments();


 prog->Pop();
}

void Synergism::GetSegments(svt::Field<nat32> & o) const
{
 svt::Field<nat32> temp;
 if (out->ByName("segs",temp)) o.CopyFrom(temp);
}

void Synergism::GetSmoothed(svt::Field<bs::ColourLuv> & o) const
{
 svt::Field<bs::ColourLuv> temp;
 if (out->ByName("smooth",temp)) o.CopyFrom(temp);
}

void Synergism::GetWeights(svt::Field<real32> & o) const
{
 svt::Field<real32> temp;
 if (out->ByName("weight",temp)) o.CopyFrom(temp);
}

void Synergism::GetEta(svt::Field<real32> & o) const
{
 svt::Field<real32> temp;
 if (out->ByName("eta",temp)) o.CopyFrom(temp);
}

void Synergism::GetRho(svt::Field<real32> & o) const
{
 svt::Field<real32> temp;
 if (out->ByName("rho",temp)) o.CopyFrom(temp);
}

bit Synergism::Distance(nat32,real32 * fv1,real32 * fv2,real32)
{
 real32 dis = math::Sqr(fv1[0]-fv2[0]) + math::Sqr(fv1[1]-fv2[1]);
 if (dis>1.0) return false;
 
 real32 col = math::Sqr(fv1[2]-fv2[2]) + math::Sqr(fv1[3]-fv2[3]) + math::Sqr(fv1[4]-fv2[4]);
 if (col>1.0) return false;

 return true;
}

//-----------------------------------------------------------------------------
 };
};
