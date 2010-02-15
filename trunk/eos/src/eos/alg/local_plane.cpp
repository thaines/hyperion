//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/alg/local_plane.h"


namespace eos
{
 namespace alg
 {
//------------------------------------------------------------------------------
SphereRegion::SphereRegion(nat32 ini)
{
 log::Assert(ini<24);

 // Set flags and reduce the ini range, so we only have to manual enter
 // cooridnates for a single corner...
  bit switchZ = ini>=12; if (switchZ) ini -= 12;
  bit switchY = ini>=6;  if (switchY) ini -= 6;
  bit switchX = ini>=3;  if (switchX) ini -= 3;


 // Do a basic corner...
  switch (ini)
  {
   case 0:
    norm[0] = bs::Normal(1.0,1.0,1.0);
    norm[1] = bs::Normal(0.0,1.0,1.0);
    norm[2] = bs::Normal(0.0,0.0,1.0);
    norm[3] = bs::Normal(1.0,0.0,1.0);
   break;
   case 1:
    norm[0] = bs::Normal(1.0,1.0,1.0);
    norm[1] = bs::Normal(1.0,1.0,0.0);
    norm[2] = bs::Normal(0.0,1.0,0.0);
    norm[3] = bs::Normal(0.0,1.0,1.0);
   break;
   case 2:
    norm[0] = bs::Normal(1.0,1.0,1.0);
    norm[1] = bs::Normal(1.0,0.0,1.0);
    norm[2] = bs::Normal(1.0,0.0,0.0);
    norm[3] = bs::Normal(1.0,1.0,0.0);  
   break;
  }


 // Normalise...
  for (nat32 i=0;i<4;i++) norm[i].Normalise();


 // Depending on the flags set mirror the corner along any of the 3 axis...
  if (switchX)
  {
   for (nat32 i=0;i<4;i++) norm[i].X() = -norm[i].X();
   math::Swap(norm[1],norm[2]);
  }
  
  if (switchY)
  {
   for (nat32 i=0;i<4;i++) norm[i].Y() = -norm[i].Y();
   math::Swap(norm[1],norm[2]);
  }
  
  if (switchZ)
  {
   for (nat32 i=0;i<4;i++) norm[i].Z() = -norm[i].Z();
   math::Swap(norm[1],norm[2]);
  }
}

SphereRegion::~SphereRegion()
{}

void SphereRegion::Subdivide(SphereRegion & a,SphereRegion & b,SphereRegion & c,SphereRegion & d) const
{
 // Create the interpolating normals...
  // Sides...
   bs::Normal side[4];
   for (nat32 i=0;i<4;i++)
   {
    math::Vect<3> tempVect;
    math::Mat<3> tempMat;
   
    math::CrossProduct(norm[i],norm[(i+1)%4],tempVect);
    tempVect.Normalise();
    math::AngAxisToRotMat(tempVect,0.5 * math::InvCos(norm[i]*norm[(i+1)%4]),tempMat);
    math::MultVect(tempMat,norm[i],side[i]);
    side[i].Normalise();
   }

  // Soft, juicy centre...
   bs::Normal centre(0.0,0.0,0.0);
   for (nat32 i=0;i<4;i++) centre += side[i];
   centre.Normalise();


 // Assign to the outputs...
  a.norm[0] = norm[0];
  a.norm[1] = side[0];
  a.norm[2] = centre;
  a.norm[3] = side[3];

  b.norm[0] = side[0];
  b.norm[1] = norm[1];
  b.norm[2] = side[1];
  b.norm[3] = centre;
  
  c.norm[0] = side[3];
  c.norm[1] = centre;
  c.norm[2] = side[2];
  c.norm[3] = norm[3];

  d.norm[0] = centre;
  d.norm[1] = side[1];
  d.norm[2] = norm[2];
  d.norm[3] = side[2];
}

void SphereRegion::PerpNormRange(const bs::Normal & n,math::Range<real32> & out) const
{
 // Calculate everything needed below, all the real work is done here...
  real32 sign[4];
  for (nat32 i=0;i<4;i++) sign[i] = n * norm[i];

  real32 ang[4];
  for (nat32 i=0;i<4;i++) ang[i] = math::InvCos(-math::Abs(sign[i])) - math::pi*0.5;

  bit outside = true;
  for (nat32 i=0;i<4;i++) outside &= math::SameSign(sign[i],sign[(i+1)%4]);
  

 // Setup the range...
  out.min = outside?math::Min(ang[0],ang[1],ang[2],ang[3]):0.0;
  out.minInc = true;
  out.max = math::Max(ang[0],ang[1],ang[2],ang[3]);
  out.maxInc = true;
}

void SphereRegion::PerpNormRange(const bs::Normal & start,const bs::Normal & end,math::Range<real32> & out) const
{
 // Calculate everything needed below, all the real work is done here...
  real32 sign[2][4];
  for (nat32 i=0;i<4;i++) sign[0][i] = start * norm[i];
  for (nat32 i=0;i<4;i++) sign[1][i] = end * norm[i];

  real32 ang[2][4];
  for (nat32 j=0;j<2;j++)
  {
   for (nat32 i=0;i<4;i++)
   {
    ang[j][i] = math::InvCos(-math::Abs(sign[j][i])) - math::pi*0.5;
   }
  }

  bit outside = true;
  for (nat32 i=0;i<4;i++)
  {
   outside &= math::SameSign(sign[0][i],sign[0][(i+1)%4]) && math::SameSign(sign[0][i],sign[1][i]);
  }


 // Setup the range...
  out.min = outside?math::Min(ang[0][0],ang[0][1],ang[0][2],ang[0][3],ang[1][0],ang[1][1],ang[1][2],ang[1][3]):0.0;
  out.minInc = true;
  out.max = math::Max(ang[0][0],ang[0][1],ang[0][2],ang[0][3],ang[1][0],ang[1][1],ang[1][2],ang[1][3]);
  out.maxInc = true;
}

//------------------------------------------------------------------------------
LocalPlane::LocalPlane()
:k(6.0)
{}

LocalPlane::~LocalPlane()
{}

void LocalPlane::Set(real32 kk)
{
 k = kk;
}

void LocalPlane::Set(const bs::Vertex & s,const bs::Normal & d)
{
 start = s;
 dir = d;
}

void LocalPlane::Add(const bs::Vertex & point,real32 weight)
{
 PointSample s;
  s.point = point;
  s.weight = weight;
 psl.AddBack(s);
}

void LocalPlane::Add(const bs::Vertex & start,const bs::Vertex & end,real32 weight)
{
 LineSample s;
  s.start = start;
  s.end = end;
  s.weight = weight;
 lsl.AddBack(s);
}

real64 LocalPlane::F(const bs::Vertex & pos,const bs::Normal & orient) const
{
 real64 ret = 0.0;


 // Sum in the influence of each point...
 {
  ds::List<PointSample>::Cursor targ = psl.FrontPtr();
  math::Mat<4> plucker;
  math::Vect<3> norm;
  while (!targ.Bad())
  {
   math::Plucker(targ->point,pos,plucker);
   math::PluckerDir(plucker,norm);
   
   real32 len = norm.Length();
   if (!math::IsZero(len)) norm /= len;
   
   real32 ang = math::InvCos(-math::Abs(norm * orient)) - math::pi*0.5;

   ret += targ->weight * math::Exp(k * math::Cos(ang));
  
   ++targ;
  }
 }


 // Sum in the influence of each line...
 {
  ds::List<LineSample>::Cursor targ = lsl.FrontPtr();
  math::Mat<4> plucker;
  math::Vect<3> norm[2];  
  while (!targ.Bad())
  {
   math::Plucker(targ->start,pos,plucker);
   math::PluckerDir(plucker,norm[0]);
   math::Plucker(targ->end,pos,plucker);
   math::PluckerDir(plucker,norm[1]);

   real32 len = norm[0].Length();
   if (!math::IsZero(len)) norm[0] /= len;
   len = norm[1].Length();
   if (!math::IsZero(len)) norm[1] /= len;

   real32 dot[2];
   dot[0] = norm[0] * orient;
   dot[1] = norm[1] * orient;

   real32 best;
   if (dot[0]*dot[1] < 0.0) best = 0.0;
                       else best = math::Min(math::Abs(dot[0]),math::Abs(dot[1]));
   if (math::IsZero((norm[0]*norm[1]) - 1.0))
   {
    best = 0.0; // Detect and handle the scenario that pos is between the two points.
   }

   real32 ang = math::InvCos(-best) - math::pi*0.5;

   ret += targ->weight * math::Exp(k * math::Cos(ang));

   ++targ;
  }
 }


 return ret;
}

void LocalPlane::RegionRange(const bs::Vertex & start,const bs::Vertex & end,
                             const SphereRegion & orient,math::Range<real32> & out) const
{
 // Setup the range object, we then iterate everything and sum in its influence...
  out.min = 0.0;
  out.minInc = true;
  out.max = 0.0;
  out.maxInc = true;


 // Sum in the maximum/minimum influence of each point...
 {
  ds::List<PointSample>::Cursor targ = psl.FrontPtr();
  math::Mat<4> plucker;
  bs::Normal sn;
  bs::Normal en;
  math::Range<real32> range;
  while (!targ.Bad())
  {
   math::Plucker(targ->point,start,plucker);
   math::PluckerDir(plucker,sn);
   math::Plucker(targ->point,end,plucker);
   math::PluckerDir(plucker,en);
  
   real32 len = sn.Length();
   if (!math::IsZero(len)) sn /= len;
   len = en.Length();
   if (!math::IsZero(len)) en /= len;

   orient.PerpNormRange(sn,en,range);

   out.max += targ->weight * math::Exp(k * math::Cos(range.min));
   out.min += targ->weight * math::Exp(k * math::Cos(range.max));
  
   ++targ;
  }
 }


 // Sum in the maximum/minimum influence of each line...
 // (We approxmate this, as its enough of a bitch allready.)
 {
  ds::List<LineSample>::Cursor targ = lsl.FrontPtr();
  math::Mat<4> plucker;
  bs::Normal sn;
  bs::Normal en;
  math::Range<real32> range[2];
  while (!targ.Bad())
  {
   // Start of line...
    math::Plucker(targ->start,start,plucker);
    math::PluckerDir(plucker,sn);
    math::Plucker(targ->start,end,plucker);
    math::PluckerDir(plucker,en);
  
    real32 len = sn.Length();
    if (!math::IsZero(len)) sn /= len;
    len = en.Length();
    if (!math::IsZero(len)) en /= len;

    orient.PerpNormRange(sn,en,range[0]);
    
   // End of line...
    math::Plucker(targ->end,start,plucker);
    math::PluckerDir(plucker,sn);
    math::Plucker(targ->end,end,plucker);
    math::PluckerDir(plucker,en);
  
    len = sn.Length();
    if (!math::IsZero(len)) sn /= len;
    len = en.Length();
    if (!math::IsZero(len)) en /= len;

    orient.PerpNormRange(sn,en,range[1]);

   // Min and max...
    out.max += targ->weight * math::Exp(k * math::Cos(math::Min(range[0].min,range[1].min)));
    out.min += targ->weight * math::Exp(k * math::Cos(math::Max(range[0].max,range[1].max)));

   ++targ;
  }
 }
}

void LocalPlane::RegionPosRange(const bs::Vertex & start,const bs::Vertex & end,
                                const bs::Normal & orient,math::Range<real32> & out) const
{
 // Setup the range object, we then iterate everything and sum in its influence...
  out.min = 0.0;
  out.minInc = true;
  out.max = 0.0;
  out.maxInc = true;


 // Sum in the maximum/minimum influence of each point...
 {
  ds::List<PointSample>::Cursor targ = psl.FrontPtr();
  math::Mat<4> plucker;
  bs::Normal sn;
  bs::Normal en;
  real32 sign[2];  
  real32 min;
  real32 max;
  real32 ang[2];

  while (!targ.Bad())
  {
   math::Plucker(targ->point,start,plucker);
   math::PluckerDir(plucker,sn);
   math::Plucker(targ->point,end,plucker);
   math::PluckerDir(plucker,en);
  
   real32 len = sn.Length();
   if (!math::IsZero(len)) sn /= len;
   len = en.Length();
   if (!math::IsZero(len)) en /= len;

   sign[0] = sn * orient;
   sign[1] = en * orient;

   ang[0] = math::InvCos(-math::Abs(sign[0])) - math::pi*0.5;
   ang[1] = math::InvCos(-math::Abs(sign[1])) - math::pi*0.5;
   
   if (math::SameSign(sign[0],sign[1])) min = math::Min(ang[0],ang[1]);
                                   else min = 0.0;
   max = math::Max(ang[0],ang[1]);

   out.max += targ->weight * math::Exp(k * math::Cos(min));
   out.min += targ->weight * math::Exp(k * math::Cos(max));
  
   ++targ;
  }
 }


 // Sum in the maximum/minimum influence of each line...
 // (We approxmate this, as its enough of a bitch allready.)
 {
  ds::List<LineSample>::Cursor targ = lsl.FrontPtr();
  math::Mat<4> plucker;
  bs::Normal sn;
  bs::Normal en;
  real32 sign[2];
  real32 min;
  real32 max;
  real32 ang[2];
  while (!targ.Bad())
  {
   // Start of line...
    math::Plucker(targ->start,start,plucker);
    math::PluckerDir(plucker,sn);
    math::Plucker(targ->start,end,plucker);
    math::PluckerDir(plucker,en);
  
    real32 len = sn.Length();
    if (!math::IsZero(len)) sn /= len;
    len = en.Length();
    if (!math::IsZero(len)) en /= len;

    sign[0] = sn * orient;
    sign[1] = en * orient;
    
    ang[0] = math::InvCos(-math::Abs(sign[0])) - math::pi*0.5;
    ang[1] = math::InvCos(-math::Abs(sign[1])) - math::pi*0.5;
    
    if (math::SameSign(sign[0],sign[1])) min = math::Min(ang[0],ang[1]);
                                    else min = 0.0;
    max = math::Max(ang[0],ang[1]);
   
   // End of line...
    math::Plucker(targ->end,start,plucker);
    math::PluckerDir(plucker,sn);
    math::Plucker(targ->end,end,plucker);
    math::PluckerDir(plucker,en);
  
    len = sn.Length();
    if (!math::IsZero(len)) sn /= len;
    len = en.Length();
    if (!math::IsZero(len)) en /= len;

    sign[0] = sn * orient;
    sign[1] = en * orient;
    
    ang[0] = math::InvCos(-math::Abs(sign[0])) - math::pi*0.5;
    ang[1] = math::InvCos(-math::Abs(sign[1])) - math::pi*0.5;
    
    if (math::SameSign(sign[0],sign[1])) min = math::Min(ang[0],ang[1],min);
                                    else min = 0.0;
    max = math::Max(ang[0],ang[1],max);

   // Min and max...
    out.max += targ->weight * math::Exp(k * math::Cos(min));
    out.min += targ->weight * math::Exp(k * math::Cos(max));

   ++targ;
  }
 }
}

void LocalPlane::RegionOrientRange(const bs::Vertex & pos,const SphereRegion & orient,math::Range<real32> & out) const
{
 // Setup the range object, we then iterate everything and sum in its influence...
  out.min = 0.0;
  out.minInc = true;
  out.max = 0.0;
  out.maxInc = true;


 // Sum in the maximum/minimum influence of each point...
 {
  ds::List<PointSample>::Cursor targ = psl.FrontPtr();
  math::Mat<4> plucker;
  bs::Normal norm;
  math::Range<real32> range;
  while (!targ.Bad())
  {
   math::Plucker(targ->point,pos,plucker);
   math::PluckerDir(plucker,norm);
  
   real32 len = norm.Length();
   if (!math::IsZero(len)) norm /= len;

   orient.PerpNormRange(norm,range);

   out.max += targ->weight * math::Exp(k * math::Cos(range.min));
   out.min += targ->weight * math::Exp(k * math::Cos(range.max));
  
   ++targ;
  }
 }


 // Sum in the maximum/minimum influence of each line...
 // (We approxmate this, as its enough of a bitch allready.)
 {
  ds::List<LineSample>::Cursor targ = lsl.FrontPtr();
  math::Mat<4> plucker;
  bs::Normal sn;
  bs::Normal en;
  math::Range<real32> range;
  while (!targ.Bad())
  {
   math::Plucker(targ->start,pos,plucker);
   math::PluckerDir(plucker,sn);
   math::Plucker(targ->end,pos,plucker);
   math::PluckerDir(plucker,en);
  
   real32 len = sn.Length();
   if (!math::IsZero(len)) sn /= len;
   len = en.Length();
   if (!math::IsZero(len)) en /= len;

   orient.PerpNormRange(sn,en,range);

   out.max += targ->weight * math::Exp(k * math::Cos(range.min));
   out.min += targ->weight * math::Exp(k * math::Cos(range.max));

   ++targ;
  }
 }
}

void LocalPlane::Mode(bs::Vertex & pos,bs::Normal & orient,time::Progress * prog) const
{
 // This does divide and conquer, we use a priority queue to always process the
 // item with the maximum possible score, we also keep track of the highest 
 // minimum found so we don't put items that could never prove productive onto
 // the queue. When we reach the subdivison depth for each node we initialise
 // Levenberg-Marquardt at the regions centre, iterate and take the best out of
 // every one of these. Despite all this it is still going to be slow, 
 // sub-dividing 6 way each level is not good.
  real32 best = 0.0; // Best value found so far, is stored in the outputs.
  real32 maxMin = 0.0; // Maximum of the minimums, to save storing useless tasks. Will never be less than best.


 // Find a suitable search range along the line - project all coordinates to 
 // the line then widen by 50% either side. Cap to take into account the half 
 // line nature...
  bs::Vertex start;
  bs::Vertex end;
  bit first = true;

  // Points...
  {
   ds::List<PointSample>::Cursor targ = psl.FrontPtr();
   while (!targ.Bad())
   {
    
   
   
    ++targ;
   }
  }
  
  // Lines...
  {
  
  }
  

 // Create the work priority queue, fill it with all the initial tasks...
  
  
 // Whilst there is work to do, do it - that means sub-dividing till the max 
 // depth is reached, and then using LM to find a precise point...
 
 
}

/*void LocalPlane::ModePos(bs::Vertex & pos,const bs::Normal & orient,time::Progress * prog) const;
void LocalPlane::ModeOrient(const bs::Vertex & pos,bs::Normal & orient,time::Progress * prog) const;*/

//------------------------------------------------------------------------------
 };
};
