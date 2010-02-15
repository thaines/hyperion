//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/math/func.h"

#include "eos/file/xml.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
Func::Func()
{}

Func::~Func()
{}

real64 Func::operator () (real64 x) const
{
 // Fill in the array of polynomial terms...
  power[0] = 1.0;
  for (nat32 i=1;i<power.Size();i++) power[i] = power[i-1] * x;
  
 // Iterate the terms and sum them into the result, return...
  real64 ret = 0.0;
  
  ds::List<Poly>::Cursor targ = terms.FrontPtr();
  while (!targ.Bad())
  {
   ret += targ->mult * power[targ->power];
   ++targ;
  }

 return ret;
}

real64 Func::Diff1(real64 x) const
{
 // Fill in the array of polynomial terms, sans last entry which we don't need...
  power[0] = 1.0;
  for (nat32 i=1;i<power.Size()-1;i++) power[i] = power[i-1] * x;
  
 // Iterate the terms and sum there differential into the result, return...
  real64 ret = 0.0;
  
  ds::List<Poly>::Cursor targ = terms.FrontPtr();
  while (!targ.Bad())
  {
   if (targ->power!=0)
   {
    ret += real64(targ->power) * targ->mult * power[targ->power-1];
   }
   ++targ;
  }

 return ret;
}

real64 Func::Inverse(real64 y,real64 tol,nat32 maxIters) const
{
 real64 x = 0.0;
 
 for (nat32 iter=0;iter<maxIters;iter++)
 {
  real64 f = (*this)(x) - y;
  real64 df = Diff1(x);
  real64 offset = f/df;
  if (math::Abs(offset)<tol) break;
  x -= offset;
 }
 
 return x;
}

void Func::SetMult(real32 k)
{
 terms.Reset();
 power.Size(2);
 
 Poly p;
 p.power = 1;
 p.mult = k;
 terms.AddBack(p);
}

void Func::Load(const bs::Element & root)
{
 LogBlock("eos::math::Func::Load","");
 
 terms.Reset();
 
 nat32 max_power = 0;
 nat32 total = root.ElementCount("poly");
 for (nat32 i=0;i<total;i++)
 {
  bs::Element * targ = root.GetElement("poly",i);

  Poly p;
  p.power = targ->GetInt("power",0);
  p.mult = targ->GetReal("mult",0.0);
  LogDebug("[Func::load] Found poly term" << LogDiv() << p.mult << " * x^" << p.power);
  
  terms.AddBack(p);
  max_power = math::Max(max_power,p.power);
 }

 power.Size(max_power+1);
}

bit Func::Load(cstrconst fn)
{
 str::TokenTable tt;
 bs::Element * root = file::LoadXML(tt,fn);
 if (root==null<bs::Element*>()) return false;

 Load(*root);

 delete root;
 return true;
}

bit Func::Load(const str::String & fn)
{
 str::TokenTable tt;
 bs::Element * root = file::LoadXML(tt,fn);
 if (root==null<bs::Element*>()) return false;

 Load(*root);

 delete root;
 return true;
}

void Func::Save(bs::Element & root) const
{
 ds::List<Poly>::Cursor targ = terms.FrontPtr();
 while (!targ.Bad())
 {
  bs::Element * np = root.NewChild("poly");
  np->SetAttribute("power",targ->power);
  np->SetAttribute("mult",targ->mult);
  
  ++targ;
 }
}

bit Func::Save(cstrconst fn,bit overwrite) const
{
 // Construct the dom...
  str::TokenTable tt;
  bs::Element root(tt,"function");
  Save(root);

 // Save the dom to the user selected filename...
  return file::SaveXML(&root,fn,overwrite);
}

bit Func::Save(const str::String & fn,bit overwrite) const
{
 // Construct the dom...
  str::TokenTable tt;
  bs::Element root(tt,"function");
  Save(root);

 // Save the dom to the user selected filename...
  return file::SaveXML(&root,fn,overwrite);
}

//------------------------------------------------------------------------------
 };
};
