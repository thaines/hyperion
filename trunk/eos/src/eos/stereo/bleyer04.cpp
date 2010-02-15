//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/stereo/bleyer04.h"

#include "eos/filter/synergism.h"
#include "eos/stereo/sad_seg_stereo.h"
#include "eos/stereo/plane_seg.h"
#include "eos/stereo/layer_maker.h"
#include "eos/stereo/layer_select.h"

#include "eos/filter/conversion.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
Bleyer04::Bleyer04()
:minDisp(-30),maxDisp(30),
planeRadius(0.6),occCost(0.078),discCost(0.01),
minCluster(20),bailOut(12),
spatialRadius(7.0),rangeRadius(4.5),minSeg(20),
windowSize(2),edgeMix(0.3),edgeLimit(0.9),
out(null<svt::Var*>())
{}

Bleyer04::~Bleyer04()
{
 delete out;
}

void Bleyer04::SetImages(const svt::Field<bs::ColourRGB> & l,const svt::Field<bs::ColourRGB> & r)
{
 left = l;
 right = r;
}

void Bleyer04::BootOverride(const svt::Field<real32> & disp,const svt::Field<bit> & mask)
{
 dispOverride = disp;
 maskOverride = mask;
}

void Bleyer04::SetMasks(const svt::Field<bit> & lm,const svt::Field<bit> & rm)
{
 leftMask = lm;
 rightMask = rm;
}

void Bleyer04::SetRange(int32 minD,int32 maxD)
{
 minDisp = minD;
 maxDisp = maxD;
}

void Bleyer04::SetPlaneRadius(real32 rad)
{
 planeRadius = rad;
}

void Bleyer04::SetWarpCost(real32 occ,real32 disc)
{
 occCost = occ;
 discCost = disc;
}

void Bleyer04::SetMinCluster(nat32 minC)
{
 minCluster = minC;
}

void Bleyer04::SetBailOut(nat32 bo)
{
 bailOut = bo;
}

void Bleyer04::SetSegment(real32 spatial,real32 range,nat32 minS)
{
 spatialRadius = spatial;
 rangeRadius = range;
 minSeg = minS;
}

void Bleyer04::SetSegmentExtra(nat32 rad,real32 mix,real32 edge)
{
 windowSize = rad;
 edgeMix = mix;
 edgeLimit = edge;
}

void Bleyer04::SegmentOverride(const svt::Field<nat32> & segs)
{
 segOverride = segs;
}

void Bleyer04::Run(time::Progress * prog)
{
 prog->Push();
 nat32 step = 0;
 nat32 steps = 6;
 if (dispOverride.Valid()) --steps;
 if (segOverride.Valid()) --steps;
 prog->Report(step++,steps);

 // Setup the output, in anticipation of getting that far...
  out = new svt::Var(left.GetVar()->GetCore());
   out->Setup2D(left.Size(0),left.Size(1));
   real32 dispIni = 0.0;
   out->Add("disp",dispIni);
   nat32 segIni = 0;
   out->Add("segs",segIni);
   out->Add("layers",segIni);
  out->Commit();

 // Setup a tempory Var we use through the proccess to store working data...
  svt::Var * temp = new svt::Var(out->GetCore());
   temp->Setup2D(left.Size(0),left.Size(1));
   bs::ColourLuv colIni(0.0,0.0,0.0);
   temp->Add("luv-left",colIni);
   temp->Add("luv-right",colIni);
   bs::ColourL collIni(0.0);
   temp->Add("l-left",collIni);
   bit maskIni;
   temp->Add("mask",maskIni);
  temp->Commit(false);

 // Prep a whole load of fields pointing to all the data we are dealing with...
  svt::Field<real32> disp; out->ByName("disp",disp);
  svt::Field<nat32> segs; out->ByName("segs",segs);
  svt::Field<nat32> layers; out->ByName("layers",layers);

  svt::Field<real32> leftR; left.SubField(0,leftR);
  svt::Field<real32> leftG; left.SubField(sizeof(real32),leftG);
  svt::Field<real32> leftB; left.SubField(2*sizeof(real32),leftB);

  svt::Field<real32> rightR; right.SubField(0,rightR);
  svt::Field<real32> rightG; right.SubField(sizeof(real32),rightG);
  svt::Field<real32> rightB; right.SubField(2*sizeof(real32),rightB);

  svt::Field<bs::ColourLuv> leftLuv; temp->ByName("luv-left",leftLuv);
  svt::Field<bs::ColourLuv> rightLuv; temp->ByName("luv-right",rightLuv);

  svt::Field<bs::ColourL> leftL; temp->ByName("l-left",leftL);

  svt::Field<bit> mask; temp->ByName("mask",mask);


 // First step, we need the images in Luv format as well, so convert (Starts easy)...
  filter::RGBtoL(left,leftL);
  filter::RGBtoLuv(left,leftLuv);
  filter::RGBtoLuv(right,rightLuv);

 // Now segment the left image...
 if (!segOverride.Valid())
 {
  prog->Report(step++,steps);
  filter::Synergism syn;
   syn.SetImage(leftL,leftLuv);
   syn.DiffWindow(windowSize);
   syn.SetMix(edgeMix);
   syn.SetSpatial(spatialRadius);
   syn.SetRange(rangeRadius);
   syn.SetSegmentMin(minSeg);
   syn.SetMergeMax(edgeLimit);

   syn.Run(prog);

   segCount = syn.Segments();
   syn.GetSegments(segs);
 }
 else
 {
  segs = segOverride;
  segCount = 1;
  for (nat32 y=0;y<segs.Size(1);y++)
  {
   for (nat32 x=0;x<segs.Size(0);x++) segCount = math::Max(segCount,segs.Get(x,y)+1);
  }
 }

 // Generate an initial disparity map for the left image, or copy in the override...
 if (!dispOverride.Valid())
 {
  prog->Report(step++,steps);
  SadSegStereo sadSeg;
   sadSeg.SetSegments(segCount,segs);
   sadSeg.AddField(leftR,rightR);
   sadSeg.AddField(leftG,rightG);
   sadSeg.AddField(leftB,rightB);
   sadSeg.SetRange(minDisp,maxDisp);
   sadSeg.SetMinCluster(minCluster);

   sadSeg.Run(prog);

   sadSeg.GetDisparity(disp);
   sadSeg.GetMask(mask);
 }
 else
 {
  disp.CopyFrom(dispOverride);
  mask.CopyFrom(maskOverride);
  if (leftMask.Valid())
  {
   for (nat32 y=0;y<mask.Size(1);y++)
   {
    for (nat32 x=0;x<mask.Size(0);x++) mask.Get(x,y) &= leftMask.Get(x,y);
   }
  }
 }


 // Fit planes to each segment...
  prog->Report(step++,steps);
  PlaneSeg planeSeg;
   planeSeg.SetDisparity(disp);
   planeSeg.SetValidity(mask);
   planeSeg.SetSegs(segCount,segs);

   planeSeg.MakePlanes();

 // Fit layers to sets of segments using mean shift...
  prog->Report(step++,steps);
  LayerMaker layerMaker;
   layerMaker.Setup(out->GetCore(),planeSeg,planeRadius,prog);


 // Iterate till changes stop, re-assign segments to different layers to improve
 // the warp image then re-fit the layers to better match the new segments...
  LayerSelect layerSelect;
   layerSelect.SetMaker(&layerMaker);
   layerSelect.SetSegs(segCount,segs);
   layerSelect.SetImages(left,right);
   layerSelect.SetMasks(leftMask,rightMask);
   layerSelect.SetCosts(occCost,discCost);
   layerSelect.SetBailOut(bailOut);

  for (nat32 i=0;i<maxIter;i++)
  {
   prog->Report(step++,steps);
   if (layerSelect.Run(prog)==false) break;
   steps += 3;
   prog->Report(step++,steps);
   layerMaker.Rebuild(planeSeg,prog);
   prog->Report(step++,steps);
   layerMaker.LayerMerge(out->GetCore(),segs,planeSeg,planeRadius,prog);
  }


 // Extract the disparity map...
  for (nat32 i=0;i<segCount;i++)
  {
   planeSeg.SetPlane(i,layerMaker.Plane(layerMaker.SegToLayer(i)));
  }
  planeSeg.Extract(disp);

 // Extract diagnostic information...
  layerCount = layerMaker.Layers();
  layerMaker.GetLayerSeg(segs,layers);


 // Clean up...
  delete temp;

 prog->Pop();
}

void Bleyer04::GetDisparity(svt::Field<real32> & disp)
{
 svt::Field<real32> temp;
 out->ByName("disp",temp);
 disp.CopyFrom(temp);
}

nat32 Bleyer04::GetSegCount()
{
 return segCount;
}

void Bleyer04::GetSegs(svt::Field<nat32> & segs)
{
 svt::Field<nat32> temp;
 out->ByName("segs",temp);
 segs.CopyFrom(temp);
}

nat32 Bleyer04::GetLayerCount()
{
 return layerCount;
}

void Bleyer04::GetLayers(svt::Field<nat32> & layers)
{
 svt::Field<nat32> temp;
 out->ByName("layers",temp);
 layers.CopyFrom(temp);
}

//------------------------------------------------------------------------------
 };
};
