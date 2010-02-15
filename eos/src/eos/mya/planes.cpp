//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/mya/planes.h"

#include "eos/alg/fitting.h"
#include "eos/math/iter_min.h"

namespace eos
{
 namespace mya
 {
//------------------------------------------------------------------------------
PlaneSurface::PlaneSurface()
{}

PlaneSurface::~PlaneSurface()
{}

PlaneSurface * PlaneSurface::Clone() const
{
 PlaneSurface * ret = new PlaneSurface();
  ret->Data() = plane;  
 return ret;
}

bit PlaneSurface::operator == (const Surface & rhs) const
{
 math::Vect<4> temp = plane;
 temp -= dynamic_cast<const PlaneSurface&>(rhs).plane;
 return temp.LengthSqr()<1e-5;
}

void PlaneSurface::Get(real32 x,real32 y,math::Vect<2> & z) const
{
 z[0] = plane[0]*x + plane[1]*y + plane[3];
 z[1] = -plane[2];
}

void PlaneSurface::GetDelta(real32,real32,math::Vect<2> & dx,math::Vect<2> & dy) const
{
 dx[0] = plane[0];
 dx[1] = plane[2];
 
 dy[0] = plane[1];
 dy[1] = plane[2];
}

math::Vect<4> & PlaneSurface::Data()
{
 return plane;
}

cstrconst PlaneSurface::TypeString() const
{
 return "eos::mya::PlaneSurface";
}

//------------------------------------------------------------------------------
PlaneFitter::PlaneFitter()
:dispCount(0),needleCount(0),sfsCount(0)
{}

PlaneFitter::~PlaneFitter()
{}

void PlaneFitter::Reset()
{
 dispCount = 0;
 needleCount = 0;
 sfsCount = 0;
 data.Reset();
}

bit PlaneFitter::Add(nat32 type,real32 x,real32 y,const math::Vector<real32> & d)
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
     ++dispCount;
     data.AddBack(n);
    }
   break;
   case 1:
    n.type = Node::needle;
    n.v[0] = d[0];
    n.v[1] = d[1];
    n.v[2] = d[2];
    n.v[3] = 1.0;
    ++needleCount; 
    data.AddBack(n);
   break;
   case 2:
    n.type = Node::sfs;
    n.v[0] = d[0]; 
    n.v[1] = d[1];
    n.v[2] = d[2];
    n.v[3] = d[3];
    ++sfsCount;   
    data.AddBack(n);
   break;
  }
 
 nat32 dc = dispCount + 2*math::Min(needleCount,nat32(1)) + math::Min(sfsCount,nat32(2));
 return dc>=3;
}

bit PlaneFitter::Extract(Surface & out) const
{
 PlaneSurface & o = dynamic_cast<PlaneSurface&>(out);

 // Fit an initial plane using only position data, if that fails we simply start with a plane
 // orthogonal to the viewing direction...
 {
  alg::PlaneFit pf;
   ds::List<Node>::Cursor targ = data.FrontPtr();
   while (!targ.Bad())
   {
    if (targ->type==Node::disp) pf.Add(targ->v[0],targ->v[1],targ->v[2]);    
    ++targ;
   }
    
  if (pf.Valid())
  {
   bs::PlaneABC plane;
   pf.Get(plane);
   bs::Plane p = plane;
    o.Data()[0] = p.n[0];
    o.Data()[1] = p.n[1];
    o.Data()[2] = p.n[2];        
    o.Data()[3] = p.d;    
  }
  else
  {
   o.Data()[0] = 0.0;
   o.Data()[1] = 0.0;
   o.Data()[2] = 1.0;        
   o.Data()[3] = 0.0;    
  }
 }


 // We now have a solution using algebraic error and partial data, use levenberg marquedt
 // with geometric error metrics to refine it with all data...
  math::Vector<real32> pl(3);
   pl[0] = math::InvTan2(o.Data()[1],o.Data()[0]);
   pl[1] = math::InvCos(o.Data()[2]*math::InvSqrt(math::Sqr(o.Data()[0])+math::Sqr(o.Data()[1])+math::Sqr(o.Data()[2])));
   pl[2] = o.Data()[3];

  math::LM(dispCount+needleCount+sfsCount,pl,*this,&LMfunc); 

  o.Data()[0] = math::Cos(pl[0])*math::Sin(pl[1]);
  o.Data()[1] = math::Sin(pl[0])*math::Sin(pl[1]);
  o.Data()[2] = math::Cos(pl[1]);
  o.Data()[3] = pl[2];

 return true;
}

cstrconst PlaneFitter::TypeString() const
{
 return "eos::mya::PlaneFitter";
}

void PlaneFitter::LMfunc(const math::Vector<real32> & pv, math::Vector<real32> & err, const PlaneFitter & self)
{
 real32 nx = math::Cos(pv[0])*math::Sin(pv[1]);
 real32 ny = math::Sin(pv[0])*math::Sin(pv[1]);
 real32 nz = math::Cos(pv[1]);

 nat32 ewp = 0;
 ds::List<Node>::Cursor targ = self.data.FrontPtr();
 while (!targ.Bad())
 {
  switch (targ->type)
  {
   case Node::disp:
    err[ewp] = nx*targ->v[0] + ny*targ->v[1] + nz*targ->v[2] + pv[2];    
   break;

   case Node::needle:
    err[ewp] = angMult*math::Sin(math::InvCos(nx*targ->v[0] + ny*targ->v[1] + nz*targ->v[2]));
   break;

   case Node::sfs:
    err[ewp] = angMult*math::Sin(math::InvCos(nx*targ->v[0] + ny*targ->v[1] + nz*targ->v[2]) - math::InvCos(targ->v[3]));
   break;
  }
  
  ++targ;
  ++ewp;
 }
}

//------------------------------------------------------------------------------
PlaneSurfaceType::PlaneSurfaceType()
{}

PlaneSurfaceType::~PlaneSurfaceType()
{}

PlaneSurface * PlaneSurfaceType::NewSurface() const
{
 return new PlaneSurface();
}

PlaneFitter * PlaneSurfaceType::NewFitter() const
{
 return new PlaneFitter();
}

bit PlaneSurfaceType::IsMember(Surface * s) const
{
 return str::Compare("eos::mya::PlaneSurface",s->TypeString())==0;
}

bit PlaneSurfaceType::Supports(str::TokenTable & tt,str::Token tok,nat32 & outType) const
{
 if (tok==tt("disp")) {outType = 0; return true;}
 if (tok==tt("needle")) {outType = 1; return true;}
 if (tok==tt("sfs")) {outType = 2; return true;}
 return false;
}

nat32 PlaneSurfaceType::Degrees() const
{
 return 3;
}

nat32 PlaneSurfaceType::TypeDegrees(nat32 type) const
{
 switch (type)
 {
  case 0: return 1;
  case 1: return 2;
  case 2: return 1;
  default: return 0;
 }
}

cstrconst PlaneSurfaceType::TypeString() const
{
 return "eos::mya::PlaneSurfaceType";
}

//------------------------------------------------------------------------------
 };
};
