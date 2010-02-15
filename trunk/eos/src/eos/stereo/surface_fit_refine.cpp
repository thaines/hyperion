//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/stereo/surface_fit_refine.h"

#include "eos/ds/arrays.h"
#include "eos/alg/mean_shift.h"
#include "eos/filter/segmentation.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
SegmentDSI::SegmentDSI()
:colourSize(4.5),spatialSize(7.0),dispSize(7.0),minMerge(20),minKill(0),
dsi(null<DSI*>()),segments(0)
{}

SegmentDSI::~SegmentDSI()
{}

void SegmentDSI::Set(real32 cS,real32 sS,real32 dS,nat32 mM,nat32 mK)
{
 colourSize = cS;
 spatialSize = sS;
 dispSize = dS;
 minMerge = mM;
 minKill = mK;
}

void SegmentDSI::Set(const svt::Field<bs::ColourLuv> & i)
{
 image = i;
}

void SegmentDSI::Set(const DSI & d)
{
 dsi = &d;
}

void SegmentDSI::Run(time::Progress * prog)
{
 prog->Push();

 // Create tempory svt db, with image duplication and disparity contained...
  bs::ColourLuv luvIni(0.0,0.0,0.0);
  real32 dispIni = 0.0;

  svt::Var temp(image);
  temp.Add("luv",luvIni);
  temp.Add("disp",dispIni);
  temp.Commit(false);

  svt::Field<bs::ColourLuv> luv(&temp,"luv");
  svt::Field<real32> disp(&temp,"disp");

  svt::Field<real32> colL; luv.SubField(0*sizeof(real32),colL);
  svt::Field<real32> colU; luv.SubField(1*sizeof(real32),colU);
  svt::Field<real32> colV; luv.SubField(2*sizeof(real32),colV);

  luv.CopyFrom(image);


 // Fill disparity field with the weighted averages from the dsi...
  for (nat32 y=0;y<dsi->Height();y++)
  {
   for (nat32 x=0;x<dsi->Width();x++)
   {
    real32 min = dsi->Cost(x,y,0);
    for (nat32 i=1;i<dsi->Size(x,y);i++) min = math::Min(min,dsi->Cost(x,y,i));

    real32 mean = 0.0;
    real32 sum = 0.0;
     for (nat32 i=0;i<dsi->Size(x,y);i++)
     {
      real32 mult = math::Exp(min - dsi->Cost(x,y,i));
      mean += mult * dsi->Disp(x,y,i);
      sum += mult;
     }
    disp.Get(x,y) = mean / sum;
   }
  }


 // Mean shift...
  alg::MeanShift meanShift;
  meanShift.AddFeature(0,1.0/spatialSize);
  meanShift.AddFeature(1,1.0/spatialSize);
  meanShift.AddFeature(colL,1.0/colourSize);
  meanShift.AddFeature(colU,1.0/colourSize);
  meanShift.AddFeature(colV,1.0/colourSize);
  //meanShift.AddFeature(disp,1.0/dispSize);
  meanShift.Passover(true);
  meanShift.SetCutoff(0.01,100);

  prog->Report(0,2);
  meanShift.Run(prog);

  meanShift.Get(colL,colL);
  meanShift.Get(colU,colU);
  meanShift.Get(colV,colV);
  //meanShift.Get(disp,disp);

 // Cluster to make segments...
  filter::Segmenter segy;
  segy.SetCutoff(colourSize*0.5);
  segy.SetMinimum(minMerge);
  segy.SetKillMin(minKill);
  segy.SetAverageSteps(2);

  segy.AddField(colL);
  segy.AddField(colU);
  segy.AddField(colV);

  prog->Report(1,2);
  segy.Run(prog);
  segments = segy.Segments();
  segy.GetOutput(seg);

 prog->Pop();
}

nat32 SegmentDSI::Segments() const
{
 return segments;
}

nat32 SegmentDSI::Width() const
{
 return seg.Width();
}

nat32 SegmentDSI::Height() const
{
 return seg.Height();
}

void SegmentDSI::GetSeg(svt::Field<nat32> & out) const
{
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++) out.Get(x,y) = seg.Get(x,y);
 }
}

//------------------------------------------------------------------------------
void SurSeg::GetSeg(svt::Field<nat32> & out) const
{
 nat32 width = Width();
 nat32 height = Height();
 for (nat32 y=0;y<height;y++)
 {
  for (nat32 x=0;x<width;x++)
  {
   out.Get(x,y) = Seg(x,y);
  }
 }
}

void SurSeg::GetDisp(svt::Field<real32> & out) const
{
 nat32 width = Width();
 nat32 height = Height();
 for (nat32 y=0;y<height;y++)
 {
  for (nat32 x=0;x<width;x++)
  {
   out.Get(x,y) = Disp(x,y);
  }
 }
}

void SurSeg::GetPos(svt::Field<bs::Vertex> & out) const
{
 nat32 width = Width();
 nat32 height = Height();
 for (nat32 y=0;y<height;y++)
 {
  for (nat32 x=0;x<width;x++)
  {
   Pos(x,y,out.Get(x,y));
  }
 }
}

void SurSeg::GetNorm(svt::Field<bs::Normal> & out) const
{
 nat32 width = Width();
 nat32 height = Height();
 for (nat32 y=0;y<height;y++)
 {
  for (nat32 x=0;x<width;x++)
  {
   Norm(x,y,out.Get(x,y));
  }
 }
}

nat32 SurSeg::GetBackground() const
{
 // Create a voting data structure...
  ds::Array<nat32> vote(Segments());
  for (nat32 i=0;i<Segments();i++) vote[i] = 0;


 // Cast votes...
  for (nat32 x=0;x<Width();x++)
  {
   vote[Seg(x,0)] += 1;
   vote[Seg(x,Height()-1)] += 1;
  }
  for (nat32 y=0;y<Height();y++)
  {
   vote[Seg(0,y)] += 1;
   vote[Seg(Width()-1,y)] += 1;
  }


 // Return the winner...
  nat32 best = 0;
  nat32 max = vote[0];
  for (nat32 i=1;i<Segments();i++)
  {
   if (vote[i]>max)
   {
    best = i;
    max = vote[i];
   }
  }

  LogDebug("[seg.bg] Selected background index. {segment}" << LogDiv() << best);
  return best;
}

//------------------------------------------------------------------------------
void SurSegBasic::Set(const svt::Field<real32> & d,const svt::Field<nat32> & s,const cam::CameraPair & pair)
{
 // Setup data structure...
  nat32 segIni = 0;
  real32 dispIni = 0.0;
  bs::Vertex posIni(0.0,0.0,0.0,1.0);
  bs::Normal normIni(0.0,0.0,1.0);

  delete var;
  var = new svt::Var(d);
  var->Add("seg",segIni);
  var->Add("disp",dispIni);
  var->Add("pos",posIni);
  var->Add("norm",normIni);
  var->Commit();

  var->ByName("seg",seg);
  var->ByName("disp",disp);
  var->ByName("pos",pos);
  var->ByName("norm",norm);


 // Transfer over the known...
  segments = 1;
  for (nat32 y=0;y<seg.Size(1);y++)
  {
   for (nat32 x=0;x<seg.Size(0);x++)
   {
    segments = math::Max(segments,s.Get(x,y)+1);
    seg.Get(x,y) = s.Get(x,y);
    disp.Get(x,y) = d.Get(x,y);
   }
  }


 // Use the CameraPair to calculate the unknown...
  pair.Convert(disp,pos);
  pair.Convert(disp,norm);
}

//------------------------------------------------------------------------------
PlaneStereo::PlaneStereo()
:occCost(1.0),vertCost(1.0),vertMult(0.2),errLim(0.1),
radius(1),mult(1.0),cap(3),
colourSize(4.5),spatialSize(7.0),dispSize(7.0),minMerge(20),minKill(0),
out(null<svt::Var*>())
{
 pair.SetDefault(320,240);
}

PlaneStereo::~PlaneStereo()
{
 delete out;
}

void PlaneStereo::Set(real32 occC,real32 vCost,real32 vMult,real32 errL)
{
 occCost = occC;
 vertCost = vCost;
 vertMult = vMult;
 errLim = errL;
}

void PlaneStereo::Set(nat32 r,real32 m,nat32 c)
{
 radius = r;
 mult = m;
 cap = c;
}

void PlaneStereo::Set(real32 colourS,real32 spatialS,real32 dispS,nat32 minM,nat32 minK)
{
 colourSize = colourS;
 spatialSize = spatialS;
 dispSize = dispS;
 minMerge = minM;
 minKill = minK;
}

void PlaneStereo::Set(const svt::Field<bs::ColourLuv> & l,const svt::Field<bs::ColourLuv> & r)
{
 left = l;
 right = r;
}

void PlaneStereo::Set(const cam::CameraPair & p)
{
 pair = p;
}

void PlaneStereo::Run(time::Progress * prog)
{
 prog->Push();

 // Create the output data structure...
  nat32 segIni = 0;
  real32 dispIni = 0.0;
  bs::Vertex posIni(0.0,0.0,0.0,1.0);

  delete out;
  out = new svt::Var(left);
  out->Add("seg",segIni);
  out->Add("disp",dispIni);
  out->Add("pos",posIni);
  out->Commit();

  out->ByName("seg",seg);
  out->ByName("disp",disp);
  out->ByName("pos",pos);


 // First calculate the sparse DSI...
  prog->Report(0,7);
  SparseDSI sdsi;
  sdsi.Set(occCost,vertCost,vertMult,errLim);
  BoundLuvDSC dsc(left,right);
  sdsi.Set(&dsc);
  sdsi.Run(prog);


 // Use the sparse DSI to generate a suitable image and disparity map for
 // segmentation...
  prog->Report(1,7);
  bs::ColourLuv luvIni(0.0,0.0,0.0);

  svt::Var merge(left);
  merge.Add("luv",luvIni);
  merge.Add("disp",dispIni);
  merge.Commit();

  svt::Field<bs::ColourLuv> segCol(&merge,"luv");
  svt::Field<real32> segDisp(&merge,"disp");

  // Disparity...
   DispSelect dispSelect;
   dispSelect.Set(radius,mult);
   dispSelect.Set(sdsi);
   dispSelect.Run(prog);
   dispSelect.Get(segDisp);

  // Colour...
   for (int32 y=0;y<int32(left.Size(1));y++)
   {
    for (int32 x=0;x<int32(left.Size(0));x++)
    {
     int32 lowX = math::Clamp<int32>(x+int32(math::RoundDown(segDisp.Get(x,y))),0,int32(right.Size(0)-1));
     int32 highX = math::Min(lowX+1,int32(right.Size(0)-1));
     real32 t = math::Clamp<real32>(segDisp.Get(x,y)-lowX,0.0,1.0);

     segCol.Get(x,y).l = 0.5*(left.Get(x,y).l + ((1.0-t)*right.Get(lowX,y).l + t*right.Get(highX,y).l));
     segCol.Get(x,y).u = 0.5*(left.Get(x,y).u + ((1.0-t)*right.Get(lowX,y).u + t*right.Get(highX,y).u));
     segCol.Get(x,y).v = 0.5*(left.Get(x,y).v + ((1.0-t)*right.Get(lowX,y).v + t*right.Get(highX,y).v));
    }
   }


 // Segment using mean shift...
  // Get individual fields of colour...
   svt::Field<real32> segColL; segCol.SubField(0*sizeof(real32),segColL);
   svt::Field<real32> segColU; segCol.SubField(1*sizeof(real32),segColU);
   svt::Field<real32> segColV; segCol.SubField(2*sizeof(real32),segColV);

  // Mean shift...
   prog->Report(2,7);
   alg::MeanShift meanShift;
   meanShift.AddFeature(0,1.0/spatialSize);
   meanShift.AddFeature(1,1.0/spatialSize);
   meanShift.AddFeature(segColL,1.0/colourSize);
   meanShift.AddFeature(segColU,1.0/colourSize);
   meanShift.AddFeature(segColV,1.0/colourSize);
   meanShift.AddFeature(segDisp,1.0/dispSize);
   meanShift.Passover(true);
   meanShift.SetCutoff(0.01,100);

   meanShift.Run(prog);

   meanShift.Get(segColL,segColL);
   meanShift.Get(segColU,segColU);
   meanShift.Get(segColV,segColV);
   meanShift.Get(segDisp,segDisp);

  // Cluster to make segments...
   prog->Report(3,7);
   filter::Segmenter segy;
   segy.SetCutoff(colourSize*0.5);
   segy.SetMinimum(minMerge);
   segy.SetKillMin(minKill);
   segy.SetAverageSteps(2);

   segy.AddField(segColL);
   segy.AddField(segColU);
   segy.AddField(segColV);

   segy.Run(prog);
   segy.GetOutput(seg);

 // Create 3D positions from the DSI...
  prog->Report(4,7);
  prog->Push();
   prog->Report(0,2);
   sdsi.SortByCost(prog);
   prog->Report(1,2);
   sparsePos.Set(cap);
   sparsePos.Set(sdsi,pair,prog);
  prog->Pop();


 // Feed the segments and DSI into a plane fitter...
  prog->Report(5,7);
  SparsePlane sparsePlane;
  sparsePlane.Set(sparsePos);
  sparsePlane.Set(seg);
  if (minKill>minMerge) sparsePlane.Duds(true);

  sparsePlane.Run(prog);

  plane.Size(sparsePlane.Segments());
  for (nat32 i=0;i<sparsePlane.Segments();i++) plane[i] = sparsePlane.Seg(i);


 // Extract the results into our own data structure...
  prog->Report(6,7);
  prog->Push();

   prog->Report(0,2);
   sparsePlane.PosMap(pair,pos);

   prog->Report(1,2);
   sparsePlane.DispMap(pair,disp);

  prog->Pop();


 prog->Pop();
}

nat32 PlaneStereo::Segments() const
{
 return plane.Size();
}

const bs::Plane & PlaneStereo::Surface(nat32 i) const
{
 return plane[i];
}

nat32 PlaneStereo::Seg(nat32 x,nat32 y) const
{
 return seg.Get(x,y);
}

nat32 PlaneStereo::Width() const
{
 return left.Size(0);
}

nat32 PlaneStereo::Height() const
{
 return left.Size(1);
}

real32 PlaneStereo::Disp(nat32 x,nat32 y) const
{
 return disp.Get(x,y);
}

void PlaneStereo::Pos(nat32 x,nat32 y,bs::Vertex & out) const
{
 out = pos.Get(x,y);
}

void PlaneStereo::Norm(nat32 x,nat32 y,bs::Normal & out) const
{
 out = plane[seg.Get(x,y)].n;
}

void PlaneStereo::GetSeg(svt::Field<nat32> & o) const
{
 o.CopyFrom(seg);
}

void PlaneStereo::GetDisp(svt::Field<real32> & o) const
{
 o.CopyFrom(disp);
}

void PlaneStereo::GetPos(svt::Field<bs::Vertex> & o) const
{
 o.CopyFrom(pos);
}

void PlaneStereo::GetNorm(svt::Field<bs::Normal> & out) const
{
 for (nat32 y=0;y<left.Size(1);y++)
 {
  for (nat32 x=0;x<left.Size(0);x++)
  {
   out.Get(x,y) = plane[seg.Get(x,y)].n;
  }
 }
}

//------------------------------------------------------------------------------
LocalPlaneStereo::LocalPlaneStereo()
:occCost(1.0),vertCost(1.0),vertMult(0.2),errLim(0.1),
radius(1),mult(1.0),cap(3),
colourSize(4.5),spatialSize(7.0),dispSize(7.0),minMerge(20),minKill(0),
out(null<svt::Var*>())
{
 pair.SetDefault(320,240);
}

LocalPlaneStereo::~LocalPlaneStereo()
{
 delete out;
}

void LocalPlaneStereo::Set(real32 occC,real32 vCost,real32 vMult,real32 errL)
{
 occCost = occC;
 vertCost = vCost;
 vertMult = vMult;
 errLim = errL;
}

void LocalPlaneStereo::Set(nat32 r,real32 m,nat32 c)
{
 radius = r;
 mult = m;
 cap = c;
}

void LocalPlaneStereo::Set(real32 colourS,real32 spatialS,real32 dispS,nat32 minM,nat32 minK)
{
 colourSize = colourS;
 spatialSize = spatialS;
 dispSize = dispS;
 minMerge = minM;
 minKill = minK;
}

void LocalPlaneStereo::Set(nat32 r)
{
 planeRadius = r;
}

void LocalPlaneStereo::Set(const svt::Field<bs::ColourLuv> & l,const svt::Field<bs::ColourLuv> & r)
{
 left = l;
 right = r;
}

void LocalPlaneStereo::Set(const cam::CameraPair & p)
{
 pair = p;
}

void LocalPlaneStereo::Run(time::Progress * prog)
{
 prog->Push();

 // Create the output data structure...
  nat32 segIni = 0;
  real32 dispIni = 0.0;
  bs::Vertex posIni(0.0,0.0,0.0,1.0);
  bs::Normal normIni(0.0,0.0,1.0);

  delete out;
  out = new svt::Var(left);
  out->Add("seg",segIni);
  out->Add("disp",dispIni);
  out->Add("pos",posIni);
  out->Add("norm",normIni);
  out->Commit();

  out->ByName("seg",seg);
  out->ByName("disp",disp);
  out->ByName("pos",pos);
  out->ByName("norm",norm);


 // First calculate the sparse DSI...
  prog->Report(0,7);
  SparseDSI sdsi;
  sdsi.Set(occCost,vertCost,vertMult,errLim);
  BoundLuvDSC dsc(left,right);
  sdsi.Set(&dsc);
  sdsi.Run(prog);


 // Use the sparse DSI to generate a suitable image and disparity map for
 // segmentation...
  prog->Report(1,7);
  bs::ColourLuv luvIni(0.0,0.0,0.0);

  svt::Var merge(left);
  merge.Add("luv",luvIni);
  merge.Add("disp",dispIni);
  merge.Commit();

  svt::Field<bs::ColourLuv> segCol(&merge,"luv");
  svt::Field<real32> segDisp(&merge,"disp");

  // Disparity...
   DispSelect dispSelect;
   dispSelect.Set(radius,mult);
   dispSelect.Set(sdsi);
   dispSelect.Run(prog);
   dispSelect.Get(segDisp);

  // Colour...
   for (int32 y=0;y<int32(left.Size(1));y++)
   {
    for (int32 x=0;x<int32(left.Size(0));x++)
    {
     int32 lowX = math::Clamp<int32>(x+int32(math::RoundDown(segDisp.Get(x,y))),0,int32(right.Size(0)-1));
     int32 highX = math::Min(lowX+1,int32(right.Size(0)-1));
     real32 t = math::Clamp<real32>(segDisp.Get(x,y)-lowX,0.0,1.0);

     segCol.Get(x,y).l = 0.5*(left.Get(x,y).l + ((1.0-t)*right.Get(lowX,y).l + t*right.Get(highX,y).l));
     segCol.Get(x,y).u = 0.5*(left.Get(x,y).u + ((1.0-t)*right.Get(lowX,y).u + t*right.Get(highX,y).u));
     segCol.Get(x,y).v = 0.5*(left.Get(x,y).v + ((1.0-t)*right.Get(lowX,y).v + t*right.Get(highX,y).v));
    }
   }


 // Segment using mean shift...
  // Get individual fields of colour...
   svt::Field<real32> segColL; segCol.SubField(0*sizeof(real32),segColL);
   svt::Field<real32> segColU; segCol.SubField(1*sizeof(real32),segColU);
   svt::Field<real32> segColV; segCol.SubField(2*sizeof(real32),segColV);

  // Mean shift...
   prog->Report(2,7);
   alg::MeanShift meanShift;
   meanShift.AddFeature(0,1.0/spatialSize);
   meanShift.AddFeature(1,1.0/spatialSize);
   meanShift.AddFeature(segColL,1.0/colourSize);
   meanShift.AddFeature(segColU,1.0/colourSize);
   meanShift.AddFeature(segColV,1.0/colourSize);
   meanShift.AddFeature(segDisp,1.0/dispSize);
   meanShift.Passover(true);
   meanShift.SetCutoff(0.01,100);

   meanShift.Run(prog);

   meanShift.Get(segColL,segColL);
   meanShift.Get(segColU,segColU);
   meanShift.Get(segColV,segColV);
   meanShift.Get(segDisp,segDisp);

  // Cluster to make segments...
   prog->Report(3,7);
   filter::Segmenter segy;
   segy.SetCutoff(colourSize*0.5);
   segy.SetMinimum(minMerge);
   segy.SetKillMin(minKill);
   segy.SetAverageSteps(2);

   segy.AddField(segColL);
   segy.AddField(segColU);
   segy.AddField(segColV);

   segy.Run(prog);
   segy.GetOutput(seg);
   segCount = segy.Segments();

 // Create 3D positions from the DSI...
  prog->Report(4,7);
  prog->Push();
   prog->Report(0,2);
   sdsi.SortByCost(prog);
   prog->Report(1,2);
   sparsePos.Set(cap);
   sparsePos.Set(sdsi,pair,prog);
  prog->Pop();


 // Feed the segments and DSI into a plane fitter...
  prog->Report(5,7);
  LocalSparsePlane sparsePlane;
  sparsePlane.Set(planeRadius);
  sparsePlane.Set(sparsePos);
  sparsePlane.Set(seg);
  sparsePlane.Set(pair);
  if (minKill>minMerge) sparsePlane.Duds(true);

  sparsePlane.Run(prog);


 // Extract the results into our own data structure...
  prog->Report(6,7);
  sparsePlane.PosMap(pos);
  sparsePlane.NeedleMap(norm);
  sparsePlane.DispMap(disp);


 prog->Pop();
}

void LocalPlaneStereo::ReRun(const svt::Field<bs::Normal> & needle,time::Progress * prog)
{
 prog->Push();

 // Refit...
 prog->Report(0,2);
  OrientSparsePlane sparsePlane;
  sparsePlane.Set(planeRadius);
  sparsePlane.Set(sparsePos);
  sparsePlane.Set(seg);
  sparsePlane.Set(needle);
  sparsePlane.Set(pair);
  if (minKill>minMerge) sparsePlane.Duds(true);

  sparsePlane.Run(prog);


 // Reextract...
  prog->Report(1,2);
  sparsePlane.PosMap(pos);
  sparsePlane.NeedleMap(norm);
  sparsePlane.DispMap(disp);

 prog->Pop();
}

nat32 LocalPlaneStereo::Segments() const
{
 return segCount;
}

nat32 LocalPlaneStereo::Width() const
{
 return left.Size(0);
}

nat32 LocalPlaneStereo::Height() const
{
 return left.Size(1);
}

nat32 LocalPlaneStereo::Seg(nat32 x,nat32 y) const
{
 return seg.Get(x,y);
}

real32 LocalPlaneStereo::Disp(nat32 x,nat32 y) const
{
 return disp.Get(x,y);
}

void LocalPlaneStereo::Pos(nat32 x,nat32 y,bs::Vertex & out) const
{
 out = pos.Get(x,y);
}

void LocalPlaneStereo::Norm(nat32 x,nat32 y,bs::Normal & out) const
{
 out = norm.Get(x,y);
}

void LocalPlaneStereo::GetSeg(svt::Field<nat32> & o) const
{
 o.CopyFrom(seg);
}

void LocalPlaneStereo::GetDisp(svt::Field<real32> & o) const
{
 o.CopyFrom(disp);
}

void LocalPlaneStereo::GetPos(svt::Field<bs::Vertex> & o) const
{
 o.CopyFrom(pos);
}

void LocalPlaneStereo::GetNorm(svt::Field<bs::Normal> & o) const
{
 o.CopyFrom(norm);
}

//------------------------------------------------------------------------------
CloudStereo::CloudStereo()
:occCost(1.0),vertCost(1.0),vertMult(0.2),errLim(0.1),
radius(1),mult(1.0),cap(3),
colourSize(4.5),spatialSize(7.0),dispSize(7.0),minMerge(20),minKill(0),
surRadius(2),surDamping(1.0),surStepCost(0.5),surIters(100),
out(null<svt::Var*>())
{
 pair.SetDefault(320,240);
}

CloudStereo::~CloudStereo()
{
 delete out;
}

void CloudStereo::Set(real32 occC,real32 vCost,real32 vMult,real32 errL)
{
 occCost = occC;
 vertCost = vCost;
 vertMult = vMult;
 errLim = errL;
}

void CloudStereo::Set(nat32 r,real32 m,nat32 c)
{
 radius = r;
 mult = m;
 cap = c;
}

void CloudStereo::Set(real32 colourS,real32 spatialS,real32 dispS,nat32 minM,nat32 minK)
{
 colourSize = colourS;
 spatialSize = spatialS;
 dispSize = dispS;
 minMerge = minM;
 minKill = minK;
}

void CloudStereo::Set(nat32 radius,real32 damping,real32 stepCost,nat32 iters)
{
 surRadius = radius;
 surDamping = damping;
 surStepCost = stepCost;
 surIters = iters;
}

void CloudStereo::Set(const svt::Field<bs::ColourLuv> & l,const svt::Field<bs::ColourLuv> & r)
{
 left = l;
 right = r;
}

void CloudStereo::Set(const cam::CameraPair & p)
{
 pair = p;
}

void CloudStereo::Run(time::Progress * prog)
{
 prog->Push();

 // Create the output data structure...
  nat32 segIni = 0;
  real32 dispIni = 0.0;
  bs::Vertex posIni(0.0,0.0,0.0,1.0);
  bs::Normal normIni(0.0,0.0,1.0);

  delete out;
  out = new svt::Var(left);
  out->Add("seg",segIni);
  out->Add("disp",dispIni);
  out->Add("pos",posIni);
  out->Add("norm",normIni);
  out->Commit();

  out->ByName("seg",seg);
  out->ByName("disp",disp);
  out->ByName("pos",pos);
  out->ByName("norm",norm);


 // First calculate the sparse DSI...
  prog->Report(0,7);
  sdsi.Set(occCost,vertCost,vertMult,errLim);
  BoundLuvDSC dsc(left,right);
  sdsi.Set(&dsc);
  sdsi.Run(prog);


 // Use the sparse DSI to generate a suitable image and disparity map for
 // segmentation...
  prog->Report(1,7);
  bs::ColourLuv luvIni(0.0,0.0,0.0);

  svt::Var merge(left);
  merge.Add("luv",luvIni);
  merge.Add("disp",dispIni);
  merge.Commit();

  svt::Field<bs::ColourLuv> segCol(&merge,"luv");
  svt::Field<real32> segDisp(&merge,"disp");

  // Disparity...
   DispSelect dispSelect;
   dispSelect.Set(radius,mult);
   dispSelect.Set(sdsi);
   dispSelect.Run(prog);
   dispSelect.Get(segDisp);

  // Colour...
   for (int32 y=0;y<int32(left.Size(1));y++)
   {
    for (int32 x=0;x<int32(left.Size(0));x++)
    {
     int32 lowX = math::Clamp<int32>(x+int32(math::RoundDown(segDisp.Get(x,y))),0,int32(right.Size(0)-1));
     int32 highX = math::Min(lowX+1,int32(right.Size(0)-1));
     real32 t = math::Clamp<real32>(segDisp.Get(x,y)-lowX,0.0,1.0);

     segCol.Get(x,y).l = 0.5*(left.Get(x,y).l + ((1.0-t)*right.Get(lowX,y).l + t*right.Get(highX,y).l));
     segCol.Get(x,y).u = 0.5*(left.Get(x,y).u + ((1.0-t)*right.Get(lowX,y).u + t*right.Get(highX,y).u));
     segCol.Get(x,y).v = 0.5*(left.Get(x,y).v + ((1.0-t)*right.Get(lowX,y).v + t*right.Get(highX,y).v));
    }
   }


 // Segment using mean shift...
  // Get individual fields of colour...
   svt::Field<real32> segColL; segCol.SubField(0*sizeof(real32),segColL);
   svt::Field<real32> segColU; segCol.SubField(1*sizeof(real32),segColU);
   svt::Field<real32> segColV; segCol.SubField(2*sizeof(real32),segColV);

  // Mean shift...
   prog->Report(2,7);
   alg::MeanShift meanShift;
   meanShift.AddFeature(0,1.0/spatialSize);
   meanShift.AddFeature(1,1.0/spatialSize);
   meanShift.AddFeature(segColL,1.0/colourSize);
   meanShift.AddFeature(segColU,1.0/colourSize);
   meanShift.AddFeature(segColV,1.0/colourSize);
   meanShift.AddFeature(segDisp,1.0/dispSize);
   meanShift.Passover(true);
   meanShift.SetCutoff(0.01,100);

   meanShift.Run(prog);

   meanShift.Get(segColL,segColL);
   meanShift.Get(segColU,segColU);
   meanShift.Get(segColV,segColV);
   meanShift.Get(segDisp,segDisp);

  // Cluster to make segments...
   prog->Report(3,7);
   filter::Segmenter segy;
   segy.SetCutoff(colourSize*0.5);
   segy.SetMinimum(minMerge);
   segy.SetKillMin(minKill);
   segy.SetAverageSteps(2);

   segy.AddField(segColL);
   segy.AddField(segColU);
   segy.AddField(segColV);

   segy.Run(prog);
   segy.GetOutput(seg);
   segCount = segy.Segments();

 // Create 3D positions from the DSI...
  prog->Report(4,7);
  prog->Push();
   prog->Report(0,2);
   sdsi.SortByCost(prog);
   prog->Report(1,2);
   sparsePos.Set(cap);
   sparsePos.Set(sdsi,pair,prog);
  prog->Pop();


 // Feed the segments and DSI into a surface fitter...
  prog->Report(5,7);
  refOrient.Set(surRadius,surDamping,surStepCost,surIters);
  refOrient.Set(sdsi,sparsePos);
  refOrient.Set(seg);
  refOrient.Set(pair);
  if (minKill>minMerge) refOrient.Duds(true);

  refOrient.Run(prog);


 // Extract the results into our own data structure...
  prog->Report(6,7);
  refOrient.PosMap(pos);
  refOrient.NeedleMap(norm,0.0);
  refOrient.DispMap(disp);


 prog->Pop();
}

void CloudStereo::ReRun(const svt::Field<bs::Normal> & needle,time::Progress * prog)
{
 prog->Push();

 // Refit...
 prog->Report(0,2);
  refOrient.Set(needle);
  refOrient.Run(prog);

 // Reextract...
  prog->Report(1,2);
  refOrient.PosMap(pos);
  refOrient.NeedleMap(norm);
  refOrient.DispMap(disp);

 prog->Pop();
}

nat32 CloudStereo::Segments() const
{
 return segCount;
}

nat32 CloudStereo::Width() const
{
 return left.Size(0);
}

nat32 CloudStereo::Height() const
{
 return left.Size(1);
}

nat32 CloudStereo::Seg(nat32 x,nat32 y) const
{
 return seg.Get(x,y);
}

real32 CloudStereo::Disp(nat32 x,nat32 y) const
{
 return disp.Get(x,y);
}

void CloudStereo::Pos(nat32 x,nat32 y,bs::Vertex & out) const
{
 out = pos.Get(x,y);
}

void CloudStereo::Norm(nat32 x,nat32 y,bs::Normal & out) const
{
 out = norm.Get(x,y);
}

void CloudStereo::GetSeg(svt::Field<nat32> & o) const
{
 o.CopyFrom(seg);
}

void CloudStereo::GetDisp(svt::Field<real32> & o) const
{
 o.CopyFrom(disp);
}

void CloudStereo::GetPos(svt::Field<bs::Vertex> & o) const
{
 o.CopyFrom(pos);
}

void CloudStereo::GetNorm(svt::Field<bs::Normal> & o) const
{
 o.CopyFrom(norm);
}

//------------------------------------------------------------------------------
 };
};
