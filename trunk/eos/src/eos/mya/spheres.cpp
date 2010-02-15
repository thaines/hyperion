//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/mya/spheres.h"

#include "eos/math/mat_ops.h"
#include "eos/math/iter_min.h"

namespace eos
{
 namespace mya
 {
//------------------------------------------------------------------------------
SphereSurface::SphereSurface()
{}

SphereSurface::~SphereSurface()
{}

SphereSurface * SphereSurface::Clone() const
{
 SphereSurface * ret = new SphereSurface();
  ret->loc = loc;
  ret->rad = rad;
  ret->scale = scale;
 return ret;
}

bit SphereSurface::operator == (const Surface & rhs) const
{
 math::Vect<3> temp = loc;
 temp -= dynamic_cast<const SphereSurface&>(rhs).loc;
 
 real32 radD = math::Abs(dynamic_cast<const SphereSurface&>(rhs).rad-rad);
 
 return (temp.LengthSqr()<1e-5) && (radD<1e-5);
}

void SphereSurface::Get(real32 x,real32 y,math::Vect<2> & z) const
{
 z[0] = loc[2];
 z[1] = 1.0;
  
 real32 distSqr = math::Sqr(x-loc[0]) + math::Sqr(y-loc[1]);
 real32 radSqr = math::Sqr(rad);
 
 if (distSqr<radSqr)
 {
  z[0] = loc[2] + scale*math::Sqrt(radSqr-distSqr);
 }
}

void SphereSurface::GetDelta(real32 x,real32 y,math::Vect<2> & dx,math::Vect<2> & dy) const
{
 x -= loc[0];
 y -= loc[1];

 real32 distSqr = math::Sqr(x) + math::Sqr(y);
 real32 radSqr = math::Sqr(rad);
 
 if (distSqr<radSqr)
 {
  real32 div = math::Sqrt(radSqr - distSqr);

  dx[0] = -scale*x; dx[1] = div;
  dy[0] = -scale*y; dy[1] = div;
 }
 else
 {
  dx[0] = 0.0; dx[1] = 1.0;
  dy[0] = 0.0; dy[1] = 1.0; 
 }
}

math::Vect<3> & SphereSurface::Location()
{
 return loc;
}

real32 & SphereSurface::Radius()
{
 return rad;
}

real32 & SphereSurface::Scale()
{
 return scale;
}

cstrconst SphereSurface::TypeString() const
{
 return "eos::mya::SphereSurface";
}

//------------------------------------------------------------------------------
SphereFitter::SphereFitter()
:dispCount(0),needleCount(0),sfsCount(0)
{}

SphereFitter::~SphereFitter()
{}

void SphereFitter::Reset()
{
 dispCount = 0;
 needleCount = 0;
 sfsCount = 0;
 data.Reset();
}

bit SphereFitter::Add(nat32 type,real32 x,real32 y,const math::Vector<real32> & d)
{
 Node n;
  switch (type)
  {
   case 0:
    if (!math::Equal(d[1],real32(0.0)))
    {
     n.type = Node::disp;
     n.v[0] = x;
     n.v[1] = y;
     n.v[2] = d[0]/d[1];
     n.v[3] = 1.0;
     n.v[4] = 0.0;
     n.v[5] = 0.0;
     ++dispCount;
     data.AddBack(n);
    }
   break;
   case 1:
    n.type = Node::needle;
    n.v[0] = x;
    n.v[1] = y;    
    n.v[2] = d[0];
    n.v[3] = d[1];
    n.v[4] = d[2];
    n.v[5] = 1.0;
    ++needleCount; 
    data.AddBack(n);
   break;
   case 2:
    n.type = Node::sfs;
    n.v[0] = x;
    n.v[1] = y;    
    n.v[2] = d[0];
    n.v[3] = d[1];
    n.v[4] = d[2];
    n.v[5] = d[3];
    ++sfsCount;   
    data.AddBack(n);
   break;
  }
 
 nat32 dc = dispCount + 2*math::Min(needleCount,nat32(2)) + math::Min(sfsCount,nat32(3));
 return dc>=4;
}

bit SphereFitter::Extract(Surface & out) const
{
 SphereSurface & o = dynamic_cast<SphereSurface&>(out);

 // Construct an initialisation condition, simply a centre as the average in x/y and the
 // deapest point in z of all disp data, then radius to include the entire data set...
  math::Vector<real32> sr(5,0.0);
  ds::List<Node>::Cursor targ = data.FrontPtr();
  nat32 zDiv = 0;
  while (!targ.Bad())
  {
   sr[0] += targ->v[0];
   sr[1] += targ->v[1];   
   if (targ->type==Node::disp)
   {
    sr[2] += targ->v[2];
    ++zDiv;
   }
   ++targ;
  }
  sr[0] /= real32(data.Size());
  sr[1] /= real32(data.Size());
  if (zDiv!=0) sr[2] /= real32(zDiv); 
  
  targ = data.FrontPtr();
  while (!targ.Bad())
  {
   if (targ->type==Node::disp)
   {
    sr[3] = math::Max(sr[3],math::Sqr(sr[0] - targ->v[0]) + math::Sqr(sr[1] - targ->v[1]) + math::Sqr(sr[2] - targ->v[2]));
   }
   else
   {
    sr[3] = math::Max(sr[3],math::Sqr(sr[0] - targ->v[0]) + math::Sqr(sr[1] - targ->v[1]));
   }
   ++targ;
  }
  sr[3] = math::Sqrt(sr[3]);


 // Now apply LM, optimise with a centre - radius representation, with the 
 // position error as the x-y distance to the nearest point on the sphere with the same z,
 // or the distance to the centre x-y if outside the range; and angle error as the arc
 // distance in x-y from where the point projects to where it should project concidering
 // its normal.
  math::LM(dispCount+needleCount+sfsCount,sr,*this,LMfunc);

  o.Location()[0] = sr[0];
  o.Location()[1] = sr[1];
  o.Location()[2] = sr[2];
  o.Radius() = math::Abs(sr[3]);
  o.Scale() = sr[4];

 return true;
}

cstrconst SphereFitter::TypeString() const
{
 return "eos::mya::SphereFitter";
}

void SphereFitter::LMfunc(const math::Vector<real32> & pv,math::Vector<real32> & err,const SphereFitter & self)
{
 nat32 ewp = 0;
 ds::List<Node>::Cursor targ = self.data.FrontPtr();
 while (!targ.Bad())
 {
  switch (targ->type)
  {
   case Node::disp:
   {
    real32 xD = targ->v[0] - pv[0];
    real32 yD = targ->v[1] - pv[1];    
    real32 distSqr = math::Sqr(xD) + math::Sqr(yD);
    real32 radSqr = math::Sqr(pv[3]);
    if (distSqr<radSqr)
    {
     err[ewp] = targ->v[2] - (pv[2] + pv[4]*math::Sqrt(radSqr - distSqr));
    }
    else
    {
     err[ewp] = targ->v[2] - pv[2];
     if (err[ewp]<0.0) err[ewp] -= math::Sqrt(distSqr) - pv[3];
                  else err[ewp] += math::Sqrt(distSqr) - pv[3];
    }
    err[ewp] = 0.5*(math::Sqrt(1.0 + math::Sqr(err[ewp]/0.5)) - 1.0);
   }
   break;

   case Node::needle:
   {
    real32 xD = targ->v[0] - pv[0];
    real32 yD = targ->v[1] - pv[1];    
    real32 distSqr = math::Sqr(xD) + math::Sqr(yD);
    real32 radSqr = math::Sqr(pv[3]);
    if (distSqr<radSqr)
    {
     math::Vect<3> dir;
      dir[0] = xD;
      dir[1] = yD;
      dir[2] = pv[4]*math::Sqrt(radSqr-distSqr);
     math::Vect<3> nor;
      nor[0] = targ->v[2];
      nor[1] = targ->v[3];
      nor[2] = targ->v[4];
     real32 mult = 1.0/(dir.Length()*nor.Length());
     err[ewp] = math::InvCos(mult * (dir*nor));
    }
    else
    {
     math::Vect<3> dir;
      dir[0] = xD;
      dir[1] = yD;
      dir[2] = 0.0;
     math::Vect<3> nor;
      nor[0] = targ->v[2];
      nor[1] = targ->v[3];
      nor[2] = targ->v[4];
     real32 mult = 1.0/(dir.Length()*nor.Length());
     err[ewp] = math::InvCos(mult * (dir*nor))*self.angMult;
    }
    err[ewp] = 0.5*(math::Sqrt(1.0 + math::Sqr(err[ewp]/0.5)) - 1.0);
   }
   break;

   case Node::sfs:
    err[ewp] = 0.0; // ? ******************************************************************
   break;
  }
  
  ++targ;
  ++ewp;
 }
}

//------------------------------------------------------------------------------
SphereSurfaceType::SphereSurfaceType()
{}

SphereSurfaceType::~SphereSurfaceType()
{}

SphereSurface * SphereSurfaceType::NewSurface() const
{
 return new SphereSurface();
}

SphereFitter * SphereSurfaceType::NewFitter() const
{
 return new SphereFitter();
}

bit SphereSurfaceType::IsMember(Surface * s) const
{
 return str::Compare("eos::mya::SphereSurface",s->TypeString())==0;
}

bit SphereSurfaceType::Supports(str::TokenTable & tt,str::Token tok,nat32 & outType) const
{
 if (tok==tt("disp")) {outType = 0; return true;}
 if (tok==tt("needle")) {outType = 1; return true;}
 if (tok==tt("sfs")) {outType = 2; return true;}
 return false;
}

nat32 SphereSurfaceType::Degrees() const
{
 return 5;
}

nat32 SphereSurfaceType::TypeDegrees(nat32 type) const
{
 switch (type)
 {
  case 0: return 1;
  case 1: return 1;
  case 2: return 1;
  default: return 0;
 }
}

cstrconst SphereSurfaceType::TypeString() const
{
 return "eos::mya::SphereSurfaceType";
}

//------------------------------------------------------------------------------
 };
};
