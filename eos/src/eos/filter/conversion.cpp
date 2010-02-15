//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/filter/conversion.h"

#include "eos/str/tokens.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
EOS_FUNC void LtoRGB(const svt::Field<bs::ColourL> & l,svt::Field<bs::ColourRGB> & rgb)
{
 for (nat32 y=0;y<rgb.Size(1);y++)
 {
  for (nat32 x=0;x<rgb.Size(0);x++)
  {
   rgb.Get(x,y) = l.Get(x,y);
  }
 }
}

EOS_FUNC void RGBtoL(const svt::Field<bs::ColourRGB> & rgb,svt::Field<bs::ColourL> & l)
{
 for (nat32 y=0;y<l.Size(1);y++)
 {
  for (nat32 x=0;x<l.Size(0);x++)
  {
   l.Get(x,y) = rgb.Get(x,y);
  }
 }
}

EOS_FUNC void RGBtoLuv(const svt::Field<bs::ColourRGB> & rgb,svt::Field<bs::ColourLuv> & luv)
{
 for (nat32 y=0;y<luv.Size(1);y++)
 {
  for (nat32 x=0;x<luv.Size(0);x++)
  {
   luv.Get(x,y) = rgb.Get(x,y);
  }
 }
}

EOS_FUNC void LuvtoL(const svt::Field<bs::ColourLuv> & luv,svt::Field<bs::ColourL> & l)
{
 for (nat32 y=0;y<l.Size(1);y++)
 {
  for (nat32 x=0;x<l.Size(0);x++)
  {
   l.Get(x,y) = luv.Get(x,y);
  }
 }
}

EOS_FUNC void LuvtoRGB(const svt::Field<bs::ColourLuv> & luv,svt::Field<bs::ColourRGB> & rgb)
{
 for (nat32 y=0;y<rgb.Size(1);y++)
 {
  for (nat32 x=0;x<rgb.Size(0);x++)
  {
   rgb.Get(x,y) = luv.Get(x,y);
  }
 }
}

//------------------------------------------------------------------------------
EOS_FUNC void LtoRGB(svt::Var * var,cstrconst l,cstrconst rgb)
{
 str::Token tokFrom = var->GetCore().GetTT()(l);
 str::Token tokTo = var->GetCore().GetTT()(rgb);

 svt::Field<bs::ColourL> from;
 svt::Field<bs::ColourRGB> to;

 if (var->ByName(tokTo,to)==false)
 {
  bs::ColourRGB ini(0.0,0.0,0.0);
  var->Add(tokTo,ini);
  var->Commit(false);
  var->ByName(tokTo,to);
 }
 var->ByName(tokFrom,from);

 LtoRGB(from,to);
}

EOS_FUNC void RGBtoL(svt::Var * var,cstrconst rgb,cstrconst l)
{
 str::Token tokFrom = var->GetCore().GetTT()(rgb);
 str::Token tokTo = var->GetCore().GetTT()(l);

 svt::Field<bs::ColourRGB> from;
 svt::Field<bs::ColourL> to;

 if (var->ByName(tokTo,to)==false)
 {
  bs::ColourL ini(0.0);
  var->Add(tokTo,ini);
  var->Commit(false);
  var->ByName(tokTo,to);
 }
 var->ByName(tokFrom,from);

 RGBtoL(from,to);
}

EOS_FUNC void RGBtoLuv(svt::Var * var,cstrconst rgb,cstrconst luv)
{
 str::Token tokFrom = var->GetCore().GetTT()(rgb);
 str::Token tokTo = var->GetCore().GetTT()(luv);

 svt::Field<bs::ColourRGB> from;
 svt::Field<bs::ColourLuv> to;

 if (var->ByName(tokTo,to)==false)
 {
  bs::ColourLuv ini(0.0,0.0,0.0);
  var->Add(tokTo,ini);
  var->Commit(false);
  var->ByName(tokTo,to);
 }
 var->ByName(tokFrom,from);

 RGBtoLuv(from,to);
}

EOS_FUNC void LuvtoL(svt::Var * var,cstrconst luv,cstrconst l)
{
 str::Token tokFrom = var->GetCore().GetTT()(luv);
 str::Token tokTo = var->GetCore().GetTT()(l);

 svt::Field<bs::ColourLuv> from;
 svt::Field<bs::ColourL> to;

 if (var->ByName(tokTo,to)==false)
 {
  bs::ColourL ini(0.0);
  var->Add(tokTo,ini);
  var->Commit(false);
  var->ByName(tokTo,to);
 }
 var->ByName(tokFrom,from);

 LuvtoL(from,to);
}

EOS_FUNC void LuvtoRGB(svt::Var * var,cstrconst luv,cstrconst rgb)
{
 str::Token tokFrom = var->GetCore().GetTT()(luv);
 str::Token tokTo = var->GetCore().GetTT()(rgb);

 svt::Field<bs::ColourLuv> from;
 svt::Field<bs::ColourRGB> to;

 if (var->ByName(tokTo,to)==false)
 {
  bs::ColourRGB ini(0.0,0.0,0.0);
  var->Add(tokTo,ini);
  var->Commit(false);
  var->ByName(tokTo,to);
 }
 var->ByName(tokFrom,from);

 LuvtoRGB(from,to);
}

//------------------------------------------------------------------------------
EOS_FUNC svt::Var * MakeByteRGB(svt::Var * var,cstrconst rgb)
{
 svt::Var * ret = new svt::Var(var->GetCore());
  ret->Setup2D(var->Size(0),var->Size(1));
  bs::ColRGB ini(0,0,0);
  ret->Add(rgb,ini);
 ret->Commit(false);
 
 svt::Field<bs::ColourRGB> from(var,"rgb");
 svt::Field<bs::ColRGB> to(ret,"rgb");
 
 for (nat32 y=0;y<to.Size(1);y++)
 {
  for (nat32 x=0;x<to.Size(0);x++)
  {
   to.Get(x,y) = from.Get(x,y);
  }	 
 }
 
 return ret;
}

//------------------------------------------------------------------------------
EOS_FUNC void Quant(const svt::Field<real32> & in,svt::Field<real32> & out,nat32 steps)
{ 
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++)
  {
   out.Get(x,y) = math::RoundDown(in.Get(x,y)*steps)/steps;
  }
 }
}

//------------------------------------------------------------------------------
EOS_FUNC void MaskToRGB(const svt::Field<bit> & mask,svt::Field<bs::ColourRGB> & rgb,
                        const bs::ColourRGB & colF,const bs::ColourRGB & colT)
{
 for (nat32 y=0;y<mask.Size(1);y++)
 {
  for (nat32 x=0;x<mask.Size(0);x++)
  {
   if (mask.Get(x,y)) rgb.Get(x,y) = colT;
                 else rgb.Get(x,y) = colF;
  }
 }
}

//------------------------------------------------------------------------------
 };
};
