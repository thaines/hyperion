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

#include "eos/stereo/dsi.h"

#include "eos/math/functions.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
real32 DSC::Cost(nat32 leftX,nat32 rightX,nat32 y) const
{
 nat32 size = Bytes();
 byte * mem = mem::Malloc<byte>(size*2);
 Left(leftX,y,mem);
 Right(rightX,y,mem+size);
 real32 ret = Cost(mem,mem+size);
 mem::Free(mem); 
 return ret;
}

//------------------------------------------------------------------------------
DifferenceDSC::DifferenceDSC(const svt::Field<real32> & l,const svt::Field<real32> & r,real32 m)
:left(l),right(r),mult(m)
{}

DifferenceDSC::~DifferenceDSC()
{}

DSC * DifferenceDSC::Clone() const
{
 return new DifferenceDSC(left,right,mult);
}

nat32 DifferenceDSC::Bytes() const
{
 return sizeof(real32);
}

real32 DifferenceDSC::Cost(const byte * left,const byte * right) const
{
 real32 a = *(real32*)(void*)left;
 real32 b = *(real32*)(void*)right; 
 return math::Abs(a - b);
}

void DifferenceDSC::Join(const byte * left,const byte * right,byte * out) const
{
 real32 a = *(real32*)(void*)left;
 real32 b = *(real32*)(void*)right; 
 real32 & o = *(real32*)(void*)out;
 o = 0.5 * (a + b);
}

void DifferenceDSC::Join(nat32 n,const byte ** in,byte * out) const
{
 real32 & o = *(real32*)(void*)out;
 o = 0.0;
 
 nat32 num = 0;
 for (nat32 i=0;i<n;i++)
 {
  if (in[i])
  {
   real32 t = *(real32*)(void*)in[i];
   ++num;
   o += (t-o)/real32(num);
  }
 }
}

nat32 DifferenceDSC::WidthLeft() const
{
 return left.Size(0);
}

nat32 DifferenceDSC::HeightLeft() const
{
 return left.Size(1);
}

void DifferenceDSC::Left(nat32 x,nat32 y,byte * out) const
{
 real32 & o = *(real32*)(void*)out;
 o = left.Get(x,y) * mult;
}

nat32 DifferenceDSC::WidthRight() const
{
 return right.Size(0);
}

nat32 DifferenceDSC::HeightRight() const
{
 return right.Size(1);
}

void DifferenceDSC::Right(nat32 x,nat32 y,byte * out) const
{
 real32 & o = *(real32*)(void*)out;
 o = right.Get(x,y) * mult;
}

real32 DifferenceDSC::Cost(nat32 leftX,nat32 rightX,nat32 y) const
{
 return math::Abs(left.Get(leftX,y) - right.Get(rightX,y)) * mult;
}

cstrconst DifferenceDSC::TypeString() const
{
 return "eos::stereo::DifferenceDSC";
}

//------------------------------------------------------------------------------
BoundDifferenceDSC::BoundDifferenceDSC(const svt::Field<real32> & l,const svt::Field<real32> & r,real32 m)
:left(l),right(r),mult(m)
{}

BoundDifferenceDSC::~BoundDifferenceDSC()
{}

DSC * BoundDifferenceDSC::Clone() const
{
 return new BoundDifferenceDSC(left,right,mult);
}

nat32 BoundDifferenceDSC::Bytes() const
{
 return sizeof(real32) * 2;
}

real32 BoundDifferenceDSC::Cost(const byte * left,const byte * right) const
{
 real32 lMin = *(real32*)(void*)(left);
 real32 lMax = *(real32*)(void*)(left+sizeof(real32));
 real32 rMin = *(real32*)(void*)(right);
 real32 rMax = *(real32*)(void*)(right+sizeof(real32));

 return math::Max<real32>(0.0,lMin-rMax,rMin-lMax);
}

void BoundDifferenceDSC::Join(const byte * left,const byte * right,byte * out) const
{
 real32 lMin = *(real32*)(void*)(left);
 real32 lMax = *(real32*)(void*)(left+sizeof(real32));
 real32 rMin = *(real32*)(void*)(right);
 real32 rMax = *(real32*)(void*)(right+sizeof(real32));
 
 real32 & oMin = *(real32*)(void*)(out);
 real32 & oMax = *(real32*)(void*)(out+sizeof(real32));

 oMin = math::Min(lMin,rMin);
 oMax = math::Max(lMax,rMax);
}

void BoundDifferenceDSC::Join(nat32 n,const byte ** in,byte * out) const
{
 real32 & oMin = *(real32*)(void*)(out);
 real32 & oMax = *(real32*)(void*)(out+sizeof(real32));
 
 oMin = 0.0;
 oMax = 0.0;

 for (nat32 i=0;i<n;i++)
 {
  if (in[i])
  {
   real32 tMin = *(real32*)(void*)(in[i]);
   real32 tMax = *(real32*)(void*)(in[i]+sizeof(real32));
   
   tMin = math::Min(tMin,oMin);
   tMax = math::Max(tMax,oMax);
  }
 }
}

nat32 BoundDifferenceDSC::WidthLeft() const
{
 return left.Size(0);
}

nat32 BoundDifferenceDSC::HeightLeft() const
{
 return left.Size(1);
}

void BoundDifferenceDSC::Left(nat32 x,nat32 y,byte * out) const
{
 real32 & oMin = *(real32*)(void*)(out);
 real32 & oMax = *(real32*)(void*)(out+sizeof(real32));
 
 oMin = left.Get(x,y);
 oMax = left.Get(x,y);
 
 if (x!=0)
 {
  real32 val = 0.5*(left.Get(x,y) + left.Get(x-1,y));
  oMin = math::Min(oMin,val);
  oMax = math::Max(oMax,val);
 }

 if (x+1!=left.Size(0))
 {
  real32 val = 0.5*(left.Get(x,y) + left.Get(x+1,y));
  oMin = math::Min(oMin,val);
  oMax = math::Max(oMax,val);
 }

 if (y!=0)
 {
  real32 val = 0.5*(left.Get(x,y) + left.Get(x,y-1));
  oMin = math::Min(oMin,val);
  oMax = math::Max(oMax,val);
 }

 if (y+1!=left.Size(1))
 {
  real32 val = 0.5*(left.Get(x,y) + left.Get(x,y+1));
  oMin = math::Min(oMin,val);
  oMax = math::Max(oMax,val);
 }
 
 oMin *= mult;
 oMax *= mult;
}

nat32 BoundDifferenceDSC::WidthRight() const
{
 return right.Size(0);
}

nat32 BoundDifferenceDSC::HeightRight() const
{
 return right.Size(1);
}

void BoundDifferenceDSC::Right(nat32 x,nat32 y,byte * out) const
{
 real32 & oMin = *(real32*)(void*)(out);
 real32 & oMax = *(real32*)(void*)(out+sizeof(real32));
 
 oMin = right.Get(x,y);
 oMax = right.Get(x,y);
 
 if (x!=0)
 {
  real32 val = 0.5*(right.Get(x,y) + right.Get(x-1,y));
  oMin = math::Min(oMin,val);
  oMax = math::Max(oMax,val);
 }

 if (x+1!=right.Size(0))
 {
  real32 val = 0.5*(right.Get(x,y) + right.Get(x+1,y));
  oMin = math::Min(oMin,val);
  oMax = math::Max(oMax,val);
 }

 if (y!=0)
 {
  real32 val = 0.5*(right.Get(x,y) + right.Get(x,y-1));
  oMin = math::Min(oMin,val);
  oMax = math::Max(oMax,val);
 }

 if (y+1!=right.Size(1))
 {
  real32 val = 0.5*(right.Get(x,y) + right.Get(x,y+1));
  oMin = math::Min(oMin,val);
  oMax = math::Max(oMax,val);
 }
 
 oMin *= mult;
 oMax *= mult; 
}

real32 BoundDifferenceDSC::Cost(nat32 leftX,nat32 rightX,nat32 y) const
{
 byte temp[sizeof(real32)*4];
 Left(leftX,y,temp);
 Right(rightX,y,temp+sizeof(real32)*2);
 return Cost(temp,temp+sizeof(real32)*2);
}

cstrconst BoundDifferenceDSC::TypeString() const
{
 return "eos::stereo::BoundDifferenceDSC";
}

//------------------------------------------------------------------------------
LuvDSC::LuvDSC(const svt::Field<bs::ColourLuv> & l,const svt::Field<bs::ColourLuv> & r,real32 m,real32 c)
:left(l),right(r),mult(m),cap(c)
{}

LuvDSC::~LuvDSC()
{}

DSC * LuvDSC::Clone() const
{
 return new LuvDSC(left,right,mult,cap);
}

nat32 LuvDSC::Bytes() const
{
 return sizeof(real32) * 3;
}

real32 LuvDSC::Cost(const byte * left,const byte * right) const
{
 real32 aL = *(real32*)(void*)(left);
 real32 aU = *(real32*)(void*)(left+sizeof(real32));
 real32 aV = *(real32*)(void*)(left+sizeof(real32)*2);

 real32 bL = *(real32*)(void*)(right);
 real32 bU = *(real32*)(void*)(right+sizeof(real32));
 real32 bV = *(real32*)(void*)(right+sizeof(real32)*2);

 return math::Min(math::Sqrt(math::Sqr(aL-bL) + math::Sqr(aU-bU) + math::Sqr(aV-bV)),cap);
}

void LuvDSC::Join(const byte * left,const byte * right,byte * out) const
{
 real32 aL = *(real32*)(void*)(left);
 real32 aU = *(real32*)(void*)(left+sizeof(real32));
 real32 aV = *(real32*)(void*)(left+sizeof(real32)*2);

 real32 bL = *(real32*)(void*)(right);
 real32 bU = *(real32*)(void*)(right+sizeof(real32));
 real32 bV = *(real32*)(void*)(right+sizeof(real32)*2);

 real32 & oL = *(real32*)(void*)(out);
 real32 & oU = *(real32*)(void*)(out+sizeof(real32));
 real32 & oV = *(real32*)(void*)(out+sizeof(real32)*2);

 oL = 0.5 * (aL + bL);
 oU = 0.5 * (aU + bU);
 oV = 0.5 * (aV + bV);
}

void LuvDSC::Join(nat32 n,const byte ** in,byte * out) const
{
 real32 & oL = *(real32*)(void*)(out);
 real32 & oU = *(real32*)(void*)(out+sizeof(real32));
 real32 & oV = *(real32*)(void*)(out+sizeof(real32)*2);
 
 oL = 0.0;
 oU = 0.0;
 oV = 0.0;
 
 nat32 num = 0;
 for (nat32 i=0;i<n;i++)
 {
  if (in[i])
  {
   real32 tL = *(real32*)(void*)(in[i]);
   real32 tU = *(real32*)(void*)(in[i]+sizeof(real32));
   real32 tV = *(real32*)(void*)(in[i]+sizeof(real32)*2);
   
   ++num;
   
   oL += (tL-oL)/real32(num);
   oU += (tU-oU)/real32(num);
   oV += (tV-oV)/real32(num);
  }
 }
}

nat32 LuvDSC::WidthLeft() const
{
 return left.Size(0);
}

nat32 LuvDSC::HeightLeft() const
{
 return left.Size(1);
}

void LuvDSC::Left(nat32 x,nat32 y,byte * out) const
{
 real32 & oL = *(real32*)(void*)(out);
 real32 & oU = *(real32*)(void*)(out+sizeof(real32));
 real32 & oV = *(real32*)(void*)(out+sizeof(real32)*2);
 
 oL = left.Get(x,y).l * mult;
 oU = left.Get(x,y).u * mult;
 oV = left.Get(x,y).v * mult;
}

nat32 LuvDSC::WidthRight() const
{
 return right.Size(0);
}

nat32 LuvDSC::HeightRight() const
{
 return right.Size(1);
}

void LuvDSC::Right(nat32 x,nat32 y,byte * out) const
{
 real32 & oL = *(real32*)(void*)(out);
 real32 & oU = *(real32*)(void*)(out+sizeof(real32));
 real32 & oV = *(real32*)(void*)(out+sizeof(real32)*2);
 
 oL = right.Get(x,y).l * mult;
 oU = right.Get(x,y).u * mult;
 oV = right.Get(x,y).v * mult;
}

real32 LuvDSC::Cost(nat32 leftX,nat32 rightX,nat32 y) const
{
 byte temp[sizeof(real32) * 6];
 Left(leftX,y,temp);
 Right(rightX,y,temp + sizeof(real32)*3);
 return Cost(temp,temp + sizeof(real32)*3);
}

cstrconst LuvDSC::TypeString() const
{
 return "eos::stereo::LuvDSC";
}

//------------------------------------------------------------------------------
SqrLuvDSC::SqrLuvDSC(const svt::Field<bs::ColourLuv> & l,const svt::Field<bs::ColourLuv> & r,real32 m,real32 c)
:left(l),right(r),mult(m),cap(c)
{}

SqrLuvDSC::~SqrLuvDSC()
{}

DSC * SqrLuvDSC::Clone() const
{
 return new SqrLuvDSC(left,right,mult,cap);
}

nat32 SqrLuvDSC::Bytes() const
{
 return sizeof(real32) * 3;
}

real32 SqrLuvDSC::Cost(const byte * left,const byte * right) const
{
 real32 aL = *(real32*)(void*)(left);
 real32 aU = *(real32*)(void*)(left+sizeof(real32));
 real32 aV = *(real32*)(void*)(left+sizeof(real32)*2);

 real32 bL = *(real32*)(void*)(right);
 real32 bU = *(real32*)(void*)(right+sizeof(real32));
 real32 bV = *(real32*)(void*)(right+sizeof(real32)*2);

 return math::Min(math::Sqr(aL-bL) + math::Sqr(aU-bU) + math::Sqr(aV-bV),cap);
}

void SqrLuvDSC::Join(const byte * left,const byte * right,byte * out) const
{
 real32 aL = *(real32*)(void*)(left);
 real32 aU = *(real32*)(void*)(left+sizeof(real32));
 real32 aV = *(real32*)(void*)(left+sizeof(real32)*2);

 real32 bL = *(real32*)(void*)(right);
 real32 bU = *(real32*)(void*)(right+sizeof(real32));
 real32 bV = *(real32*)(void*)(right+sizeof(real32)*2);

 real32 & oL = *(real32*)(void*)(out);
 real32 & oU = *(real32*)(void*)(out+sizeof(real32));
 real32 & oV = *(real32*)(void*)(out+sizeof(real32)*2);

 oL = 0.5 * (aL + bL);
 oU = 0.5 * (aU + bU);
 oV = 0.5 * (aV + bV);
}

void SqrLuvDSC::Join(nat32 n,const byte ** in,byte * out) const
{
 real32 & oL = *(real32*)(void*)(out);
 real32 & oU = *(real32*)(void*)(out+sizeof(real32));
 real32 & oV = *(real32*)(void*)(out+sizeof(real32)*2);
 
 oL = 0.0;
 oU = 0.0;
 oV = 0.0;
 
 nat32 num = 0;
 for (nat32 i=0;i<n;i++)
 {
  if (in[i])
  {
   real32 tL = *(real32*)(void*)(in[i]);
   real32 tU = *(real32*)(void*)(in[i]+sizeof(real32));
   real32 tV = *(real32*)(void*)(in[i]+sizeof(real32)*2);
   
   ++num;
   
   oL += (tL-oL)/real32(num);
   oU += (tU-oU)/real32(num);
   oV += (tV-oV)/real32(num);
  }
 }
}

nat32 SqrLuvDSC::WidthLeft() const
{
 return left.Size(0);
}

nat32 SqrLuvDSC::HeightLeft() const
{
 return left.Size(1);
}

void SqrLuvDSC::Left(nat32 x,nat32 y,byte * out) const
{
 real32 & oL = *(real32*)(void*)(out);
 real32 & oU = *(real32*)(void*)(out+sizeof(real32));
 real32 & oV = *(real32*)(void*)(out+sizeof(real32)*2);
 
 oL = left.Get(x,y).l * mult;
 oU = left.Get(x,y).u * mult;
 oV = left.Get(x,y).v * mult;
}

nat32 SqrLuvDSC::WidthRight() const
{
 return right.Size(0);
}

nat32 SqrLuvDSC::HeightRight() const
{
 return right.Size(1);
}

void SqrLuvDSC::Right(nat32 x,nat32 y,byte * out) const
{
 real32 & oL = *(real32*)(void*)(out);
 real32 & oU = *(real32*)(void*)(out+sizeof(real32));
 real32 & oV = *(real32*)(void*)(out+sizeof(real32)*2);
 
 oL = right.Get(x,y).l * mult;
 oU = right.Get(x,y).u * mult;
 oV = right.Get(x,y).v * mult;
}

real32 SqrLuvDSC::Cost(nat32 leftX,nat32 rightX,nat32 y) const
{
 byte temp[sizeof(real32) * 6];
 Left(leftX,y,temp);
 Right(rightX,y,temp + sizeof(real32)*3);
 return Cost(temp,temp + sizeof(real32)*3);
}

cstrconst SqrLuvDSC::TypeString() const
{
 return "eos::stereo::SqrLuvDSC";
}

//------------------------------------------------------------------------------
ManLuvDSC::ManLuvDSC(const svt::Field<bs::ColourLuv> & l,const svt::Field<bs::ColourLuv> & r,real32 m,real32 c)
:left(l),right(r),mult(m),cap(c)
{}

ManLuvDSC::~ManLuvDSC()
{}

DSC * ManLuvDSC::Clone() const
{
 return new ManLuvDSC(left,right,mult,cap);
}

nat32 ManLuvDSC::Bytes() const
{
 return sizeof(real32) * 3;
}

real32 ManLuvDSC::Cost(const byte * left,const byte * right) const
{
 real32 aL = *(real32*)(void*)(left);
 real32 aU = *(real32*)(void*)(left+sizeof(real32));
 real32 aV = *(real32*)(void*)(left+sizeof(real32)*2);

 real32 bL = *(real32*)(void*)(right);
 real32 bU = *(real32*)(void*)(right+sizeof(real32));
 real32 bV = *(real32*)(void*)(right+sizeof(real32)*2);

 return math::Min(math::Abs(aL-bL) + math::Abs(aU-bU) + math::Abs(aV-bV),cap);
}

void ManLuvDSC::Join(const byte * left,const byte * right,byte * out) const
{
 real32 aL = *(real32*)(void*)(left);
 real32 aU = *(real32*)(void*)(left+sizeof(real32));
 real32 aV = *(real32*)(void*)(left+sizeof(real32)*2);

 real32 bL = *(real32*)(void*)(right);
 real32 bU = *(real32*)(void*)(right+sizeof(real32));
 real32 bV = *(real32*)(void*)(right+sizeof(real32)*2);

 real32 & oL = *(real32*)(void*)(out);
 real32 & oU = *(real32*)(void*)(out+sizeof(real32));
 real32 & oV = *(real32*)(void*)(out+sizeof(real32)*2);

 oL = 0.5 * (aL + bL);
 oU = 0.5 * (aU + bU);
 oV = 0.5 * (aV + bV);
}

void ManLuvDSC::Join(nat32 n,const byte ** in,byte * out) const
{
 real32 & oL = *(real32*)(void*)(out);
 real32 & oU = *(real32*)(void*)(out+sizeof(real32));
 real32 & oV = *(real32*)(void*)(out+sizeof(real32)*2);
 
 oL = 0.0;
 oU = 0.0;
 oV = 0.0;
 
 nat32 num = 0;
 for (nat32 i=0;i<n;i++)
 {
  if (in[i])
  {
   real32 tL = *(real32*)(void*)(in[i]);
   real32 tU = *(real32*)(void*)(in[i]+sizeof(real32));
   real32 tV = *(real32*)(void*)(in[i]+sizeof(real32)*2);
   
   ++num;
   
   oL += (tL-oL)/real32(num);
   oU += (tU-oU)/real32(num);
   oV += (tV-oV)/real32(num);
  }
 }
}

nat32 ManLuvDSC::WidthLeft() const
{
 return left.Size(0);
}

nat32 ManLuvDSC::HeightLeft() const
{
 return left.Size(1);
}

void ManLuvDSC::Left(nat32 x,nat32 y,byte * out) const
{
 real32 & oL = *(real32*)(void*)(out);
 real32 & oU = *(real32*)(void*)(out+sizeof(real32));
 real32 & oV = *(real32*)(void*)(out+sizeof(real32)*2);
 
 oL = left.Get(x,y).l * mult;
 oU = left.Get(x,y).u * mult;
 oV = left.Get(x,y).v * mult;
}

nat32 ManLuvDSC::WidthRight() const
{
 return right.Size(0);
}

nat32 ManLuvDSC::HeightRight() const
{
 return right.Size(1);
}

void ManLuvDSC::Right(nat32 x,nat32 y,byte * out) const
{
 real32 & oL = *(real32*)(void*)(out);
 real32 & oU = *(real32*)(void*)(out+sizeof(real32));
 real32 & oV = *(real32*)(void*)(out+sizeof(real32)*2);
 
 oL = right.Get(x,y).l * mult;
 oU = right.Get(x,y).u * mult;
 oV = right.Get(x,y).v * mult;
}

real32 ManLuvDSC::Cost(nat32 leftX,nat32 rightX,nat32 y) const
{
 byte temp[sizeof(real32) * 6];
 Left(leftX,y,temp);
 Right(rightX,y,temp + sizeof(real32)*3);
 return Cost(temp,temp + sizeof(real32)*3);
}

cstrconst ManLuvDSC::TypeString() const
{
 return "eos::stereo::ManLuvDSC";
}

//------------------------------------------------------------------------------
HalfBoundLuvDSC::HalfBoundLuvDSC(const svt::Field<bs::ColourLuv> & l,const svt::Field<bs::ColourLuv> & r,real32 m,real32 c)
:left(l),right(r),mult(m),cap(c)
{}

HalfBoundLuvDSC::~HalfBoundLuvDSC()
{}

DSC * HalfBoundLuvDSC::Clone() const
{
 return new BoundLuvDSC(left,right,mult,cap);
}

void HalfBoundLuvDSC::SetMult(real32 m)
{
 mult = m;
}

void HalfBoundLuvDSC::SetCap(real32 c)
{
 cap = c;
}

nat32 HalfBoundLuvDSC::Bytes() const
{
 return sizeof(real32) * 6;
}

real32 HalfBoundLuvDSC::Cost(const byte * left,const byte * right) const
{
 real32 aMinL = *(real32*)(void*)(left);
 real32 aMaxL = *(real32*)(void*)(left+sizeof(real32));
 real32 aMinU = *(real32*)(void*)(left+sizeof(real32)*2);
 real32 aMaxU = *(real32*)(void*)(left+sizeof(real32)*3);
 real32 aMinV = *(real32*)(void*)(left+sizeof(real32)*4);
 real32 aMaxV = *(real32*)(void*)(left+sizeof(real32)*5);

 real32 bMinL = *(real32*)(void*)(right);
 real32 bMaxL = *(real32*)(void*)(right+sizeof(real32));
 real32 bMinU = *(real32*)(void*)(right+sizeof(real32)*2);
 real32 bMaxU = *(real32*)(void*)(right+sizeof(real32)*3);
 real32 bMinV = *(real32*)(void*)(right+sizeof(real32)*4);
 real32 bMaxV = *(real32*)(void*)(right+sizeof(real32)*5);

 real32 dL = math::Max<real32>(0.0,aMinL-bMaxL,bMinL-aMaxL);
 real32 dU = math::Max<real32>(0.0,aMinU-bMaxU,bMinU-aMaxU);
 real32 dV = math::Max<real32>(0.0,aMinV-bMaxV,bMinV-aMaxV);

 return math::Min(math::Sqrt(math::Sqr(dL) + math::Sqr(dU) + math::Sqr(dV))*mult,cap);
}

void HalfBoundLuvDSC::Join(const byte * left,const byte * right,byte * out) const
{
 real32 aMinL = *(real32*)(void*)(left);
 real32 aMaxL = *(real32*)(void*)(left+sizeof(real32));
 real32 aMinU = *(real32*)(void*)(left+sizeof(real32)*2);
 real32 aMaxU = *(real32*)(void*)(left+sizeof(real32)*3);
 real32 aMinV = *(real32*)(void*)(left+sizeof(real32)*4);
 real32 aMaxV = *(real32*)(void*)(left+sizeof(real32)*5);

 real32 bMinL = *(real32*)(void*)(right);
 real32 bMaxL = *(real32*)(void*)(right+sizeof(real32));
 real32 bMinU = *(real32*)(void*)(right+sizeof(real32)*2);
 real32 bMaxU = *(real32*)(void*)(right+sizeof(real32)*3);
 real32 bMinV = *(real32*)(void*)(right+sizeof(real32)*4);
 real32 bMaxV = *(real32*)(void*)(right+sizeof(real32)*5);
 
 real32 & oMinL = *(real32*)(void*)(out);
 real32 & oMaxL = *(real32*)(void*)(out+sizeof(real32));
 real32 & oMinU = *(real32*)(void*)(out+sizeof(real32)*2);
 real32 & oMaxU = *(real32*)(void*)(out+sizeof(real32)*3);
 real32 & oMinV = *(real32*)(void*)(out+sizeof(real32)*4);
 real32 & oMaxV = *(real32*)(void*)(out+sizeof(real32)*5);
 
 oMinL = math::Min(aMinL,bMinL);
 oMaxL = math::Max(aMaxL,bMaxL);
 oMinU = math::Min(aMinU,bMinU);
 oMaxU = math::Max(aMaxU,bMaxU);
 oMinV = math::Min(aMinV,bMinV);
 oMaxV = math::Max(aMaxV,bMaxV);  
}

void HalfBoundLuvDSC::Join(nat32 n,const byte ** in,byte * out) const
{
 real32 & oMinL = *(real32*)(void*)(out);
 real32 & oMaxL = *(real32*)(void*)(out+sizeof(real32));
 real32 & oMinU = *(real32*)(void*)(out+sizeof(real32)*2);
 real32 & oMaxU = *(real32*)(void*)(out+sizeof(real32)*3);
 real32 & oMinV = *(real32*)(void*)(out+sizeof(real32)*4);
 real32 & oMaxV = *(real32*)(void*)(out+sizeof(real32)*5);

 bit first = true;
 for (nat32 i=0;i<n;i++)
 {
  if (in[i])
  {
   if (first) mem::Copy(out,in[i],Bytes());
   else
   {
    real32 tMinL = *(real32*)(void*)(in[i]);
    real32 tMaxL = *(real32*)(void*)(in[i]+sizeof(real32));
    real32 tMinU = *(real32*)(void*)(in[i]+sizeof(real32)*2);
    real32 tMaxU = *(real32*)(void*)(in[i]+sizeof(real32)*3);
    real32 tMinV = *(real32*)(void*)(in[i]+sizeof(real32)*4);
    real32 tMaxV = *(real32*)(void*)(in[i]+sizeof(real32)*5);
    
    oMinL = math::Min(oMinL,tMinL);
    oMaxL = math::Max(oMaxL,tMaxL);
    oMinU = math::Min(oMinU,tMinU);
    oMaxU = math::Max(oMaxU,tMaxU);
    oMinV = math::Min(oMinV,tMinV);
    oMaxV = math::Max(oMaxV,tMaxV);  
   }
  }
 }
}

nat32 HalfBoundLuvDSC::WidthLeft() const
{
 return left.Size(0);
}

nat32 HalfBoundLuvDSC::HeightLeft() const
{
 return left.Size(1);
}

void HalfBoundLuvDSC::Left(nat32 x,nat32 y,byte * out) const
{
 real32 & oMinL = *(real32*)(void*)(out);
 real32 & oMaxL = *(real32*)(void*)(out+sizeof(real32));
 real32 & oMinU = *(real32*)(void*)(out+sizeof(real32)*2);
 real32 & oMaxU = *(real32*)(void*)(out+sizeof(real32)*3);
 real32 & oMinV = *(real32*)(void*)(out+sizeof(real32)*4);
 real32 & oMaxV = *(real32*)(void*)(out+sizeof(real32)*5);
 
 oMinL = left.Get(x,y).l;
 oMaxL = left.Get(x,y).l;
 oMinU = left.Get(x,y).u;
 oMaxU = left.Get(x,y).u;
 oMinV = left.Get(x,y).v;
 oMaxV = left.Get(x,y).v; 
 
 if (x!=0)
 {
  real32 sL = 0.5 * (left.Get(x,y).l + left.Get(x-1,y).l);
  real32 sU = 0.5 * (left.Get(x,y).u + left.Get(x-1,y).u);
  real32 sV = 0.5 * (left.Get(x,y).v + left.Get(x-1,y).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }
 
 if (x+1!=left.Size(0))
 {
  real32 sL = 0.5 * (left.Get(x,y).l + left.Get(x+1,y).l);
  real32 sU = 0.5 * (left.Get(x,y).u + left.Get(x+1,y).u);
  real32 sV = 0.5 * (left.Get(x,y).v + left.Get(x+1,y).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }

 oMinL *= mult;
 oMaxL *= mult;
 oMinU *= mult;
 oMaxU *= mult;
 oMinV *= mult;
 oMaxV *= mult;
}

nat32 HalfBoundLuvDSC::WidthRight() const
{
 return right.Size(0);
}

nat32 HalfBoundLuvDSC::HeightRight() const
{
 return right.Size(1);
}

void HalfBoundLuvDSC::Right(nat32 x,nat32 y,byte * out) const
{
 real32 & oMinL = *(real32*)(void*)(out);
 real32 & oMaxL = *(real32*)(void*)(out+sizeof(real32));
 real32 & oMinU = *(real32*)(void*)(out+sizeof(real32)*2);
 real32 & oMaxU = *(real32*)(void*)(out+sizeof(real32)*3);
 real32 & oMinV = *(real32*)(void*)(out+sizeof(real32)*4);
 real32 & oMaxV = *(real32*)(void*)(out+sizeof(real32)*5);
 
 oMinL = right.Get(x,y).l;
 oMaxL = right.Get(x,y).l;
 oMinU = right.Get(x,y).u;
 oMaxU = right.Get(x,y).u;
 oMinV = right.Get(x,y).v;
 oMaxV = right.Get(x,y).v;
 
 if (x!=0)
 {
  real32 sL = 0.5 * (right.Get(x,y).l + right.Get(x-1,y).l);
  real32 sU = 0.5 * (right.Get(x,y).u + right.Get(x-1,y).u);
  real32 sV = 0.5 * (right.Get(x,y).v + right.Get(x-1,y).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }
 
 if (x+1!=right.Size(0))
 {
  real32 sL = 0.5 * (right.Get(x,y).l + right.Get(x+1,y).l);
  real32 sU = 0.5 * (right.Get(x,y).u + right.Get(x+1,y).u);
  real32 sV = 0.5 * (right.Get(x,y).v + right.Get(x+1,y).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 } 

 oMinL *= mult;
 oMaxL *= mult;
 oMinU *= mult;
 oMaxU *= mult;
 oMinV *= mult;
 oMaxV *= mult;
}

real32 HalfBoundLuvDSC::Cost(nat32 leftX,nat32 rightX,nat32 y) const
{
 byte temp[sizeof(real32) * 12];
 Left(leftX,y,temp);
 Right(rightX,y,temp + sizeof(real32)*6);
 return Cost(temp,temp + sizeof(real32)*6);
}

cstrconst HalfBoundLuvDSC::TypeString() const
{
 return "eos::stereo::HalfBoundLuvDSC";
}

//------------------------------------------------------------------------------
BoundLuvDSC::BoundLuvDSC(const svt::Field<bs::ColourLuv> & l,const svt::Field<bs::ColourLuv> & r,real32 m,real32 c)
:left(l),right(r),mult(m),cap(c)
{}

BoundLuvDSC::~BoundLuvDSC()
{}

DSC * BoundLuvDSC::Clone() const
{
 return new BoundLuvDSC(left,right,mult,cap);
}

void BoundLuvDSC::SetMult(real32 m)
{
 mult = m;
}

void BoundLuvDSC::SetCap(real32 c)
{
 cap = c;
}

nat32 BoundLuvDSC::Bytes() const
{
 return sizeof(real32) * 6;
}

real32 BoundLuvDSC::Cost(const byte * left,const byte * right) const
{
 real32 aMinL = *(real32*)(void*)(left);
 real32 aMaxL = *(real32*)(void*)(left+sizeof(real32));
 real32 aMinU = *(real32*)(void*)(left+sizeof(real32)*2);
 real32 aMaxU = *(real32*)(void*)(left+sizeof(real32)*3);
 real32 aMinV = *(real32*)(void*)(left+sizeof(real32)*4);
 real32 aMaxV = *(real32*)(void*)(left+sizeof(real32)*5);

 real32 bMinL = *(real32*)(void*)(right);
 real32 bMaxL = *(real32*)(void*)(right+sizeof(real32));
 real32 bMinU = *(real32*)(void*)(right+sizeof(real32)*2);
 real32 bMaxU = *(real32*)(void*)(right+sizeof(real32)*3);
 real32 bMinV = *(real32*)(void*)(right+sizeof(real32)*4);
 real32 bMaxV = *(real32*)(void*)(right+sizeof(real32)*5);

 real32 dL = math::Max<real32>(0.0,aMinL-bMaxL,bMinL-aMaxL);
 real32 dU = math::Max<real32>(0.0,aMinU-bMaxU,bMinU-aMaxU);
 real32 dV = math::Max<real32>(0.0,aMinV-bMaxV,bMinV-aMaxV);

 return math::Min(math::Sqrt(math::Sqr(dL) + math::Sqr(dU) + math::Sqr(dV))*mult,cap);
}

void BoundLuvDSC::Join(const byte * left,const byte * right,byte * out) const
{
 real32 aMinL = *(real32*)(void*)(left);
 real32 aMaxL = *(real32*)(void*)(left+sizeof(real32));
 real32 aMinU = *(real32*)(void*)(left+sizeof(real32)*2);
 real32 aMaxU = *(real32*)(void*)(left+sizeof(real32)*3);
 real32 aMinV = *(real32*)(void*)(left+sizeof(real32)*4);
 real32 aMaxV = *(real32*)(void*)(left+sizeof(real32)*5);

 real32 bMinL = *(real32*)(void*)(right);
 real32 bMaxL = *(real32*)(void*)(right+sizeof(real32));
 real32 bMinU = *(real32*)(void*)(right+sizeof(real32)*2);
 real32 bMaxU = *(real32*)(void*)(right+sizeof(real32)*3);
 real32 bMinV = *(real32*)(void*)(right+sizeof(real32)*4);
 real32 bMaxV = *(real32*)(void*)(right+sizeof(real32)*5);
 
 real32 & oMinL = *(real32*)(void*)(out);
 real32 & oMaxL = *(real32*)(void*)(out+sizeof(real32));
 real32 & oMinU = *(real32*)(void*)(out+sizeof(real32)*2);
 real32 & oMaxU = *(real32*)(void*)(out+sizeof(real32)*3);
 real32 & oMinV = *(real32*)(void*)(out+sizeof(real32)*4);
 real32 & oMaxV = *(real32*)(void*)(out+sizeof(real32)*5);
 
 oMinL = math::Min(aMinL,bMinL);
 oMaxL = math::Max(aMaxL,bMaxL);
 oMinU = math::Min(aMinU,bMinU);
 oMaxU = math::Max(aMaxU,bMaxU);
 oMinV = math::Min(aMinV,bMinV);
 oMaxV = math::Max(aMaxV,bMaxV);  
}

void BoundLuvDSC::Join(nat32 n,const byte ** in,byte * out) const
{
 real32 & oMinL = *(real32*)(void*)(out);
 real32 & oMaxL = *(real32*)(void*)(out+sizeof(real32));
 real32 & oMinU = *(real32*)(void*)(out+sizeof(real32)*2);
 real32 & oMaxU = *(real32*)(void*)(out+sizeof(real32)*3);
 real32 & oMinV = *(real32*)(void*)(out+sizeof(real32)*4);
 real32 & oMaxV = *(real32*)(void*)(out+sizeof(real32)*5);

 bit first = true;
 for (nat32 i=0;i<n;i++)
 {
  if (in[i])
  {
   if (first) mem::Copy(out,in[i],Bytes());
   else
   {
    real32 tMinL = *(real32*)(void*)(in[i]);
    real32 tMaxL = *(real32*)(void*)(in[i]+sizeof(real32));
    real32 tMinU = *(real32*)(void*)(in[i]+sizeof(real32)*2);
    real32 tMaxU = *(real32*)(void*)(in[i]+sizeof(real32)*3);
    real32 tMinV = *(real32*)(void*)(in[i]+sizeof(real32)*4);
    real32 tMaxV = *(real32*)(void*)(in[i]+sizeof(real32)*5);
    
    oMinL = math::Min(oMinL,tMinL);
    oMaxL = math::Max(oMaxL,tMaxL);
    oMinU = math::Min(oMinU,tMinU);
    oMaxU = math::Max(oMaxU,tMaxU);
    oMinV = math::Min(oMinV,tMinV);
    oMaxV = math::Max(oMaxV,tMaxV);  
   }
  }
 }
}

nat32 BoundLuvDSC::WidthLeft() const
{
 return left.Size(0);
}

nat32 BoundLuvDSC::HeightLeft() const
{
 return left.Size(1);
}

void BoundLuvDSC::Left(nat32 x,nat32 y,byte * out) const
{
 real32 & oMinL = *(real32*)(void*)(out);
 real32 & oMaxL = *(real32*)(void*)(out+sizeof(real32));
 real32 & oMinU = *(real32*)(void*)(out+sizeof(real32)*2);
 real32 & oMaxU = *(real32*)(void*)(out+sizeof(real32)*3);
 real32 & oMinV = *(real32*)(void*)(out+sizeof(real32)*4);
 real32 & oMaxV = *(real32*)(void*)(out+sizeof(real32)*5);
 
 oMinL = left.Get(x,y).l;
 oMaxL = left.Get(x,y).l;
 oMinU = left.Get(x,y).u;
 oMaxU = left.Get(x,y).u;
 oMinV = left.Get(x,y).v;
 oMaxV = left.Get(x,y).v; 
 
 if (x!=0)
 {
  real32 sL = 0.5 * (left.Get(x,y).l + left.Get(x-1,y).l);
  real32 sU = 0.5 * (left.Get(x,y).u + left.Get(x-1,y).u);
  real32 sV = 0.5 * (left.Get(x,y).v + left.Get(x-1,y).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }
 
 if (x+1!=left.Size(0))
 {
  real32 sL = 0.5 * (left.Get(x,y).l + left.Get(x+1,y).l);
  real32 sU = 0.5 * (left.Get(x,y).u + left.Get(x+1,y).u);
  real32 sV = 0.5 * (left.Get(x,y).v + left.Get(x+1,y).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }

 if (y!=0)
 {
  real32 sL = 0.5 * (left.Get(x,y).l + left.Get(x,y-1).l);
  real32 sU = 0.5 * (left.Get(x,y).u + left.Get(x,y-1).u);
  real32 sV = 0.5 * (left.Get(x,y).v + left.Get(x,y-1).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }
 
 if (y+1!=left.Size(1))
 {
  real32 sL = 0.5 * (left.Get(x,y).l + left.Get(x,y+1).l);
  real32 sU = 0.5 * (left.Get(x,y).u + left.Get(x,y+1).u);
  real32 sV = 0.5 * (left.Get(x,y).v + left.Get(x,y+1).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }

 oMinL *= mult;
 oMaxL *= mult;
 oMinU *= mult;
 oMaxU *= mult;
 oMinV *= mult;
 oMaxV *= mult;
}

nat32 BoundLuvDSC::WidthRight() const
{
 return right.Size(0);
}

nat32 BoundLuvDSC::HeightRight() const
{
 return right.Size(1);
}

void BoundLuvDSC::Right(nat32 x,nat32 y,byte * out) const
{
 real32 & oMinL = *(real32*)(void*)(out);
 real32 & oMaxL = *(real32*)(void*)(out+sizeof(real32));
 real32 & oMinU = *(real32*)(void*)(out+sizeof(real32)*2);
 real32 & oMaxU = *(real32*)(void*)(out+sizeof(real32)*3);
 real32 & oMinV = *(real32*)(void*)(out+sizeof(real32)*4);
 real32 & oMaxV = *(real32*)(void*)(out+sizeof(real32)*5);
 
 oMinL = right.Get(x,y).l;
 oMaxL = right.Get(x,y).l;
 oMinU = right.Get(x,y).u;
 oMaxU = right.Get(x,y).u;
 oMinV = right.Get(x,y).v;
 oMaxV = right.Get(x,y).v;
 
 if (x!=0)
 {
  real32 sL = 0.5 * (right.Get(x,y).l + right.Get(x-1,y).l);
  real32 sU = 0.5 * (right.Get(x,y).u + right.Get(x-1,y).u);
  real32 sV = 0.5 * (right.Get(x,y).v + right.Get(x-1,y).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }
 
 if (x+1!=right.Size(0))
 {
  real32 sL = 0.5 * (right.Get(x,y).l + right.Get(x+1,y).l);
  real32 sU = 0.5 * (right.Get(x,y).u + right.Get(x+1,y).u);
  real32 sV = 0.5 * (right.Get(x,y).v + right.Get(x+1,y).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 } 

 if (y!=0)
 {
  real32 sL = 0.5 * (right.Get(x,y).l + right.Get(x,y-1).l);
  real32 sU = 0.5 * (right.Get(x,y).u + right.Get(x,y-1).u);
  real32 sV = 0.5 * (right.Get(x,y).v + right.Get(x,y-1).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }
 
 if (y+1!=right.Size(1))
 {
  real32 sL = 0.5 * (right.Get(x,y).l + right.Get(x,y+1).l);
  real32 sU = 0.5 * (right.Get(x,y).u + right.Get(x,y+1).u);
  real32 sV = 0.5 * (right.Get(x,y).v + right.Get(x,y+1).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 } 

 oMinL *= mult;
 oMaxL *= mult;
 oMinU *= mult;
 oMaxU *= mult;
 oMinV *= mult;
 oMaxV *= mult;
}

real32 BoundLuvDSC::Cost(nat32 leftX,nat32 rightX,nat32 y) const
{
 byte temp[sizeof(real32) * 12];
 Left(leftX,y,temp);
 Right(rightX,y,temp + sizeof(real32)*6);
 return Cost(temp,temp + sizeof(real32)*6);
}

cstrconst BoundLuvDSC::TypeString() const
{
 return "eos::stereo::BoundLuvDSC";
}

//------------------------------------------------------------------------------
SqrBoundLuvDSC::SqrBoundLuvDSC(const svt::Field<bs::ColourLuv> & l,const svt::Field<bs::ColourLuv> & r,real32 m,real32 c)
:left(l),right(r),mult(m),cap(c)
{}

SqrBoundLuvDSC::~SqrBoundLuvDSC()
{}

DSC * SqrBoundLuvDSC::Clone() const
{
 return new SqrBoundLuvDSC(left,right,mult,cap);
}

void SqrBoundLuvDSC::SetMult(real32 m)
{
 mult = m;
}

void SqrBoundLuvDSC::SetCap(real32 c)
{
 cap = c;
}

nat32 SqrBoundLuvDSC::Bytes() const
{
 return sizeof(real32) * 6;
}

real32 SqrBoundLuvDSC::Cost(const byte * left,const byte * right) const
{
 real32 aMinL = *(real32*)(void*)(left);
 real32 aMaxL = *(real32*)(void*)(left+sizeof(real32));
 real32 aMinU = *(real32*)(void*)(left+sizeof(real32)*2);
 real32 aMaxU = *(real32*)(void*)(left+sizeof(real32)*3);
 real32 aMinV = *(real32*)(void*)(left+sizeof(real32)*4);
 real32 aMaxV = *(real32*)(void*)(left+sizeof(real32)*5);

 real32 bMinL = *(real32*)(void*)(right);
 real32 bMaxL = *(real32*)(void*)(right+sizeof(real32));
 real32 bMinU = *(real32*)(void*)(right+sizeof(real32)*2);
 real32 bMaxU = *(real32*)(void*)(right+sizeof(real32)*3);
 real32 bMinV = *(real32*)(void*)(right+sizeof(real32)*4);
 real32 bMaxV = *(real32*)(void*)(right+sizeof(real32)*5);

 real32 dL = math::Max<real32>(0.0,aMinL-bMaxL,bMinL-aMaxL);
 real32 dU = math::Max<real32>(0.0,aMinU-bMaxU,bMinU-aMaxU);
 real32 dV = math::Max<real32>(0.0,aMinV-bMaxV,bMinV-aMaxV);

 return math::Min((math::Sqr(dL) + math::Sqr(dU) + math::Sqr(dV))*mult,cap);
}

void SqrBoundLuvDSC::Join(const byte * left,const byte * right,byte * out) const
{
 real32 aMinL = *(real32*)(void*)(left);
 real32 aMaxL = *(real32*)(void*)(left+sizeof(real32));
 real32 aMinU = *(real32*)(void*)(left+sizeof(real32)*2);
 real32 aMaxU = *(real32*)(void*)(left+sizeof(real32)*3);
 real32 aMinV = *(real32*)(void*)(left+sizeof(real32)*4);
 real32 aMaxV = *(real32*)(void*)(left+sizeof(real32)*5);

 real32 bMinL = *(real32*)(void*)(right);
 real32 bMaxL = *(real32*)(void*)(right+sizeof(real32));
 real32 bMinU = *(real32*)(void*)(right+sizeof(real32)*2);
 real32 bMaxU = *(real32*)(void*)(right+sizeof(real32)*3);
 real32 bMinV = *(real32*)(void*)(right+sizeof(real32)*4);
 real32 bMaxV = *(real32*)(void*)(right+sizeof(real32)*5);
 
 real32 & oMinL = *(real32*)(void*)(out);
 real32 & oMaxL = *(real32*)(void*)(out+sizeof(real32));
 real32 & oMinU = *(real32*)(void*)(out+sizeof(real32)*2);
 real32 & oMaxU = *(real32*)(void*)(out+sizeof(real32)*3);
 real32 & oMinV = *(real32*)(void*)(out+sizeof(real32)*4);
 real32 & oMaxV = *(real32*)(void*)(out+sizeof(real32)*5);
 
 oMinL = math::Min(aMinL,bMinL);
 oMaxL = math::Max(aMaxL,bMaxL);
 oMinU = math::Min(aMinU,bMinU);
 oMaxU = math::Max(aMaxU,bMaxU);
 oMinV = math::Min(aMinV,bMinV);
 oMaxV = math::Max(aMaxV,bMaxV);  
}

void SqrBoundLuvDSC::Join(nat32 n,const byte ** in,byte * out) const
{
 real32 & oMinL = *(real32*)(void*)(out);
 real32 & oMaxL = *(real32*)(void*)(out+sizeof(real32));
 real32 & oMinU = *(real32*)(void*)(out+sizeof(real32)*2);
 real32 & oMaxU = *(real32*)(void*)(out+sizeof(real32)*3);
 real32 & oMinV = *(real32*)(void*)(out+sizeof(real32)*4);
 real32 & oMaxV = *(real32*)(void*)(out+sizeof(real32)*5);

 bit first = true;
 for (nat32 i=0;i<n;i++)
 {
  if (in[i])
  {
   if (first) mem::Copy(out,in[i],Bytes());
   else
   {
    real32 tMinL = *(real32*)(void*)(in[i]);
    real32 tMaxL = *(real32*)(void*)(in[i]+sizeof(real32));
    real32 tMinU = *(real32*)(void*)(in[i]+sizeof(real32)*2);
    real32 tMaxU = *(real32*)(void*)(in[i]+sizeof(real32)*3);
    real32 tMinV = *(real32*)(void*)(in[i]+sizeof(real32)*4);
    real32 tMaxV = *(real32*)(void*)(in[i]+sizeof(real32)*5);
    
    oMinL = math::Min(oMinL,tMinL);
    oMaxL = math::Max(oMaxL,tMaxL);
    oMinU = math::Min(oMinU,tMinU);
    oMaxU = math::Max(oMaxU,tMaxU);
    oMinV = math::Min(oMinV,tMinV);
    oMaxV = math::Max(oMaxV,tMaxV);  
   }
  }
 }
}

nat32 SqrBoundLuvDSC::WidthLeft() const
{
 return left.Size(0);
}

nat32 SqrBoundLuvDSC::HeightLeft() const
{
 return left.Size(1);
}

void SqrBoundLuvDSC::Left(nat32 x,nat32 y,byte * out) const
{
 real32 & oMinL = *(real32*)(void*)(out);
 real32 & oMaxL = *(real32*)(void*)(out+sizeof(real32));
 real32 & oMinU = *(real32*)(void*)(out+sizeof(real32)*2);
 real32 & oMaxU = *(real32*)(void*)(out+sizeof(real32)*3);
 real32 & oMinV = *(real32*)(void*)(out+sizeof(real32)*4);
 real32 & oMaxV = *(real32*)(void*)(out+sizeof(real32)*5);
 
 oMinL = left.Get(x,y).l;
 oMaxL = left.Get(x,y).l;
 oMinU = left.Get(x,y).u;
 oMaxU = left.Get(x,y).u;
 oMinV = left.Get(x,y).v;
 oMaxV = left.Get(x,y).v; 
 
 if (x!=0)
 {
  real32 sL = 0.5 * (left.Get(x,y).l + left.Get(x-1,y).l);
  real32 sU = 0.5 * (left.Get(x,y).u + left.Get(x-1,y).u);
  real32 sV = 0.5 * (left.Get(x,y).v + left.Get(x-1,y).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }
 
 if (x+1!=left.Size(0))
 {
  real32 sL = 0.5 * (left.Get(x,y).l + left.Get(x+1,y).l);
  real32 sU = 0.5 * (left.Get(x,y).u + left.Get(x+1,y).u);
  real32 sV = 0.5 * (left.Get(x,y).v + left.Get(x+1,y).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }

 if (y!=0)
 {
  real32 sL = 0.5 * (left.Get(x,y).l + left.Get(x,y-1).l);
  real32 sU = 0.5 * (left.Get(x,y).u + left.Get(x,y-1).u);
  real32 sV = 0.5 * (left.Get(x,y).v + left.Get(x,y-1).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }
 
 if (y+1!=left.Size(1))
 {
  real32 sL = 0.5 * (left.Get(x,y).l + left.Get(x,y+1).l);
  real32 sU = 0.5 * (left.Get(x,y).u + left.Get(x,y+1).u);
  real32 sV = 0.5 * (left.Get(x,y).v + left.Get(x,y+1).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }

 oMinL *= mult;
 oMaxL *= mult;
 oMinU *= mult;
 oMaxU *= mult;
 oMinV *= mult;
 oMaxV *= mult;
}

nat32 SqrBoundLuvDSC::WidthRight() const
{
 return right.Size(0);
}

nat32 SqrBoundLuvDSC::HeightRight() const
{
 return right.Size(1);
}

void SqrBoundLuvDSC::Right(nat32 x,nat32 y,byte * out) const
{
 real32 & oMinL = *(real32*)(void*)(out);
 real32 & oMaxL = *(real32*)(void*)(out+sizeof(real32));
 real32 & oMinU = *(real32*)(void*)(out+sizeof(real32)*2);
 real32 & oMaxU = *(real32*)(void*)(out+sizeof(real32)*3);
 real32 & oMinV = *(real32*)(void*)(out+sizeof(real32)*4);
 real32 & oMaxV = *(real32*)(void*)(out+sizeof(real32)*5);
 
 oMinL = right.Get(x,y).l;
 oMaxL = right.Get(x,y).l;
 oMinU = right.Get(x,y).u;
 oMaxU = right.Get(x,y).u;
 oMinV = right.Get(x,y).v;
 oMaxV = right.Get(x,y).v;
 
 if (x!=0)
 {
  real32 sL = 0.5 * (right.Get(x,y).l + right.Get(x-1,y).l);
  real32 sU = 0.5 * (right.Get(x,y).u + right.Get(x-1,y).u);
  real32 sV = 0.5 * (right.Get(x,y).v + right.Get(x-1,y).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }
 
 if (x+1!=right.Size(0))
 {
  real32 sL = 0.5 * (right.Get(x,y).l + right.Get(x+1,y).l);
  real32 sU = 0.5 * (right.Get(x,y).u + right.Get(x+1,y).u);
  real32 sV = 0.5 * (right.Get(x,y).v + right.Get(x+1,y).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 } 

 if (y!=0)
 {
  real32 sL = 0.5 * (right.Get(x,y).l + right.Get(x,y-1).l);
  real32 sU = 0.5 * (right.Get(x,y).u + right.Get(x,y-1).u);
  real32 sV = 0.5 * (right.Get(x,y).v + right.Get(x,y-1).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }
 
 if (y+1!=right.Size(1))
 {
  real32 sL = 0.5 * (right.Get(x,y).l + right.Get(x,y+1).l);
  real32 sU = 0.5 * (right.Get(x,y).u + right.Get(x,y+1).u);
  real32 sV = 0.5 * (right.Get(x,y).v + right.Get(x,y+1).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 } 

 oMinL *= mult;
 oMaxL *= mult;
 oMinU *= mult;
 oMaxU *= mult;
 oMinV *= mult;
 oMaxV *= mult;
}

real32 SqrBoundLuvDSC::Cost(nat32 leftX,nat32 rightX,nat32 y) const
{
 byte temp[sizeof(real32) * 12];
 Left(leftX,y,temp);
 Right(rightX,y,temp + sizeof(real32)*6);
 return Cost(temp,temp + sizeof(real32)*6);
}

cstrconst SqrBoundLuvDSC::TypeString() const
{
 return "eos::stereo::SqrBoundLuvDSC";
}

//------------------------------------------------------------------------------
ManBoundLuvDSC::ManBoundLuvDSC(const svt::Field<bs::ColourLuv> & l,const svt::Field<bs::ColourLuv> & r,real32 m,real32 c)
:left(l),right(r),mult(m),cap(c)
{}

ManBoundLuvDSC::~ManBoundLuvDSC()
{}

DSC * ManBoundLuvDSC::Clone() const
{
 return new ManBoundLuvDSC(left,right,mult,cap);
}

void ManBoundLuvDSC::SetMult(real32 m)
{
 mult = m;
}

void ManBoundLuvDSC::SetCap(real32 c)
{
 cap = c;
}

nat32 ManBoundLuvDSC::Bytes() const
{
 return sizeof(real32) * 6;
}

real32 ManBoundLuvDSC::Cost(const byte * left,const byte * right) const
{
 real32 aMinL = *(real32*)(void*)(left);
 real32 aMaxL = *(real32*)(void*)(left+sizeof(real32));
 real32 aMinU = *(real32*)(void*)(left+sizeof(real32)*2);
 real32 aMaxU = *(real32*)(void*)(left+sizeof(real32)*3);
 real32 aMinV = *(real32*)(void*)(left+sizeof(real32)*4);
 real32 aMaxV = *(real32*)(void*)(left+sizeof(real32)*5);

 real32 bMinL = *(real32*)(void*)(right);
 real32 bMaxL = *(real32*)(void*)(right+sizeof(real32));
 real32 bMinU = *(real32*)(void*)(right+sizeof(real32)*2);
 real32 bMaxU = *(real32*)(void*)(right+sizeof(real32)*3);
 real32 bMinV = *(real32*)(void*)(right+sizeof(real32)*4);
 real32 bMaxV = *(real32*)(void*)(right+sizeof(real32)*5);

 real32 dL = math::Max<real32>(0.0,aMinL-bMaxL,bMinL-aMaxL);
 real32 dU = math::Max<real32>(0.0,aMinU-bMaxU,bMinU-aMaxU);
 real32 dV = math::Max<real32>(0.0,aMinV-bMaxV,bMinV-aMaxV);

 return math::Min((dL + dU + dV)*mult,cap);
}

void ManBoundLuvDSC::Join(const byte * left,const byte * right,byte * out) const
{
 real32 aMinL = *(real32*)(void*)(left);
 real32 aMaxL = *(real32*)(void*)(left+sizeof(real32));
 real32 aMinU = *(real32*)(void*)(left+sizeof(real32)*2);
 real32 aMaxU = *(real32*)(void*)(left+sizeof(real32)*3);
 real32 aMinV = *(real32*)(void*)(left+sizeof(real32)*4);
 real32 aMaxV = *(real32*)(void*)(left+sizeof(real32)*5);

 real32 bMinL = *(real32*)(void*)(right);
 real32 bMaxL = *(real32*)(void*)(right+sizeof(real32));
 real32 bMinU = *(real32*)(void*)(right+sizeof(real32)*2);
 real32 bMaxU = *(real32*)(void*)(right+sizeof(real32)*3);
 real32 bMinV = *(real32*)(void*)(right+sizeof(real32)*4);
 real32 bMaxV = *(real32*)(void*)(right+sizeof(real32)*5);
 
 real32 & oMinL = *(real32*)(void*)(out);
 real32 & oMaxL = *(real32*)(void*)(out+sizeof(real32));
 real32 & oMinU = *(real32*)(void*)(out+sizeof(real32)*2);
 real32 & oMaxU = *(real32*)(void*)(out+sizeof(real32)*3);
 real32 & oMinV = *(real32*)(void*)(out+sizeof(real32)*4);
 real32 & oMaxV = *(real32*)(void*)(out+sizeof(real32)*5);
 
 oMinL = math::Min(aMinL,bMinL);
 oMaxL = math::Max(aMaxL,bMaxL);
 oMinU = math::Min(aMinU,bMinU);
 oMaxU = math::Max(aMaxU,bMaxU);
 oMinV = math::Min(aMinV,bMinV);
 oMaxV = math::Max(aMaxV,bMaxV);  
}

void ManBoundLuvDSC::Join(nat32 n,const byte ** in,byte * out) const
{
 real32 & oMinL = *(real32*)(void*)(out);
 real32 & oMaxL = *(real32*)(void*)(out+sizeof(real32));
 real32 & oMinU = *(real32*)(void*)(out+sizeof(real32)*2);
 real32 & oMaxU = *(real32*)(void*)(out+sizeof(real32)*3);
 real32 & oMinV = *(real32*)(void*)(out+sizeof(real32)*4);
 real32 & oMaxV = *(real32*)(void*)(out+sizeof(real32)*5);

 bit first = true;
 for (nat32 i=0;i<n;i++)
 {
  if (in[i])
  {
   if (first) mem::Copy(out,in[i],Bytes());
   else
   {
    real32 tMinL = *(real32*)(void*)(in[i]);
    real32 tMaxL = *(real32*)(void*)(in[i]+sizeof(real32));
    real32 tMinU = *(real32*)(void*)(in[i]+sizeof(real32)*2);
    real32 tMaxU = *(real32*)(void*)(in[i]+sizeof(real32)*3);
    real32 tMinV = *(real32*)(void*)(in[i]+sizeof(real32)*4);
    real32 tMaxV = *(real32*)(void*)(in[i]+sizeof(real32)*5);
    
    oMinL = math::Min(oMinL,tMinL);
    oMaxL = math::Max(oMaxL,tMaxL);
    oMinU = math::Min(oMinU,tMinU);
    oMaxU = math::Max(oMaxU,tMaxU);
    oMinV = math::Min(oMinV,tMinV);
    oMaxV = math::Max(oMaxV,tMaxV);  
   }
  }
 }
}

nat32 ManBoundLuvDSC::WidthLeft() const
{
 return left.Size(0);
}

nat32 ManBoundLuvDSC::HeightLeft() const
{
 return left.Size(1);
}

void ManBoundLuvDSC::Left(nat32 x,nat32 y,byte * out) const
{
 real32 & oMinL = *(real32*)(void*)(out);
 real32 & oMaxL = *(real32*)(void*)(out+sizeof(real32));
 real32 & oMinU = *(real32*)(void*)(out+sizeof(real32)*2);
 real32 & oMaxU = *(real32*)(void*)(out+sizeof(real32)*3);
 real32 & oMinV = *(real32*)(void*)(out+sizeof(real32)*4);
 real32 & oMaxV = *(real32*)(void*)(out+sizeof(real32)*5);
 
 oMinL = left.Get(x,y).l;
 oMaxL = left.Get(x,y).l;
 oMinU = left.Get(x,y).u;
 oMaxU = left.Get(x,y).u;
 oMinV = left.Get(x,y).v;
 oMaxV = left.Get(x,y).v; 
 
 if (x!=0)
 {
  real32 sL = 0.5 * (left.Get(x,y).l + left.Get(x-1,y).l);
  real32 sU = 0.5 * (left.Get(x,y).u + left.Get(x-1,y).u);
  real32 sV = 0.5 * (left.Get(x,y).v + left.Get(x-1,y).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }
 
 if (x+1!=left.Size(0))
 {
  real32 sL = 0.5 * (left.Get(x,y).l + left.Get(x+1,y).l);
  real32 sU = 0.5 * (left.Get(x,y).u + left.Get(x+1,y).u);
  real32 sV = 0.5 * (left.Get(x,y).v + left.Get(x+1,y).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }

 if (y!=0)
 {
  real32 sL = 0.5 * (left.Get(x,y).l + left.Get(x,y-1).l);
  real32 sU = 0.5 * (left.Get(x,y).u + left.Get(x,y-1).u);
  real32 sV = 0.5 * (left.Get(x,y).v + left.Get(x,y-1).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }
 
 if (y+1!=left.Size(1))
 {
  real32 sL = 0.5 * (left.Get(x,y).l + left.Get(x,y+1).l);
  real32 sU = 0.5 * (left.Get(x,y).u + left.Get(x,y+1).u);
  real32 sV = 0.5 * (left.Get(x,y).v + left.Get(x,y+1).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }

 oMinL *= mult;
 oMaxL *= mult;
 oMinU *= mult;
 oMaxU *= mult;
 oMinV *= mult;
 oMaxV *= mult;
}

nat32 ManBoundLuvDSC::WidthRight() const
{
 return right.Size(0);
}

nat32 ManBoundLuvDSC::HeightRight() const
{
 return right.Size(1);
}

void ManBoundLuvDSC::Right(nat32 x,nat32 y,byte * out) const
{
 real32 & oMinL = *(real32*)(void*)(out);
 real32 & oMaxL = *(real32*)(void*)(out+sizeof(real32));
 real32 & oMinU = *(real32*)(void*)(out+sizeof(real32)*2);
 real32 & oMaxU = *(real32*)(void*)(out+sizeof(real32)*3);
 real32 & oMinV = *(real32*)(void*)(out+sizeof(real32)*4);
 real32 & oMaxV = *(real32*)(void*)(out+sizeof(real32)*5);
 
 oMinL = right.Get(x,y).l;
 oMaxL = right.Get(x,y).l;
 oMinU = right.Get(x,y).u;
 oMaxU = right.Get(x,y).u;
 oMinV = right.Get(x,y).v;
 oMaxV = right.Get(x,y).v;
 
 if (x!=0)
 {
  real32 sL = 0.5 * (right.Get(x,y).l + right.Get(x-1,y).l);
  real32 sU = 0.5 * (right.Get(x,y).u + right.Get(x-1,y).u);
  real32 sV = 0.5 * (right.Get(x,y).v + right.Get(x-1,y).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }
 
 if (x+1!=right.Size(0))
 {
  real32 sL = 0.5 * (right.Get(x,y).l + right.Get(x+1,y).l);
  real32 sU = 0.5 * (right.Get(x,y).u + right.Get(x+1,y).u);
  real32 sV = 0.5 * (right.Get(x,y).v + right.Get(x+1,y).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 } 

 if (y!=0)
 {
  real32 sL = 0.5 * (right.Get(x,y).l + right.Get(x,y-1).l);
  real32 sU = 0.5 * (right.Get(x,y).u + right.Get(x,y-1).u);
  real32 sV = 0.5 * (right.Get(x,y).v + right.Get(x,y-1).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }
 
 if (y+1!=right.Size(1))
 {
  real32 sL = 0.5 * (right.Get(x,y).l + right.Get(x,y+1).l);
  real32 sU = 0.5 * (right.Get(x,y).u + right.Get(x,y+1).u);
  real32 sV = 0.5 * (right.Get(x,y).v + right.Get(x,y+1).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 } 

 oMinL *= mult;
 oMaxL *= mult;
 oMinU *= mult;
 oMaxU *= mult;
 oMinV *= mult;
 oMaxV *= mult;
}

real32 ManBoundLuvDSC::Cost(nat32 leftX,nat32 rightX,nat32 y) const
{
 byte temp[sizeof(real32) * 12];
 Left(leftX,y,temp);
 Right(rightX,y,temp + sizeof(real32)*6);
 return Cost(temp,temp + sizeof(real32)*6);
}

cstrconst ManBoundLuvDSC::TypeString() const
{
 return "eos::stereo::ManBoundLuvDSC";
}

//------------------------------------------------------------------------------
RangeLuvDSC::RangeLuvDSC(const svt::Field<bs::ColourLuv> & l,const svt::Field<bs::ColourLuv> & r,real32 m,real32 c)
:left(l),right(r),mult(m),cap(c)
{}

RangeLuvDSC::~RangeLuvDSC()
{}

DSC * RangeLuvDSC::Clone() const
{
 return new RangeLuvDSC(left,right,mult,cap);
}

void RangeLuvDSC::SetMult(real32 m)
{
 mult = m;
}

void RangeLuvDSC::SetCap(real32 c)
{
 cap = c;
}

nat32 RangeLuvDSC::Bytes() const
{
 return sizeof(real32) * 6;
}

real32 RangeLuvDSC::Cost(const byte * left,const byte * right) const
{
 real32 aMinL = *(real32*)(void*)(left);
 real32 aMaxL = *(real32*)(void*)(left+sizeof(real32));
 real32 aMinU = *(real32*)(void*)(left+sizeof(real32)*2);
 real32 aMaxU = *(real32*)(void*)(left+sizeof(real32)*3);
 real32 aMinV = *(real32*)(void*)(left+sizeof(real32)*4);
 real32 aMaxV = *(real32*)(void*)(left+sizeof(real32)*5);

 real32 bMinL = *(real32*)(void*)(right);
 real32 bMaxL = *(real32*)(void*)(right+sizeof(real32));
 real32 bMinU = *(real32*)(void*)(right+sizeof(real32)*2);
 real32 bMaxU = *(real32*)(void*)(right+sizeof(real32)*3);
 real32 bMinV = *(real32*)(void*)(right+sizeof(real32)*4);
 real32 bMaxV = *(real32*)(void*)(right+sizeof(real32)*5);

 real32 dL = math::Max<real32>(bMaxL-aMinL,aMaxL-bMinL);
 real32 dU = math::Max<real32>(bMaxU-aMinU,aMaxU-bMinU);
 real32 dV = math::Max<real32>(bMaxV-aMinV,aMaxV-bMinV);

 return math::Min(math::Sqrt(math::Sqr(dL) + math::Sqr(dU) + math::Sqr(dV))*mult,cap);
}

void RangeLuvDSC::Join(const byte * left,const byte * right,byte * out) const
{
 real32 aMinL = *(real32*)(void*)(left);
 real32 aMaxL = *(real32*)(void*)(left+sizeof(real32));
 real32 aMinU = *(real32*)(void*)(left+sizeof(real32)*2);
 real32 aMaxU = *(real32*)(void*)(left+sizeof(real32)*3);
 real32 aMinV = *(real32*)(void*)(left+sizeof(real32)*4);
 real32 aMaxV = *(real32*)(void*)(left+sizeof(real32)*5);

 real32 bMinL = *(real32*)(void*)(right);
 real32 bMaxL = *(real32*)(void*)(right+sizeof(real32));
 real32 bMinU = *(real32*)(void*)(right+sizeof(real32)*2);
 real32 bMaxU = *(real32*)(void*)(right+sizeof(real32)*3);
 real32 bMinV = *(real32*)(void*)(right+sizeof(real32)*4);
 real32 bMaxV = *(real32*)(void*)(right+sizeof(real32)*5);
 
 real32 & oMinL = *(real32*)(void*)(out);
 real32 & oMaxL = *(real32*)(void*)(out+sizeof(real32));
 real32 & oMinU = *(real32*)(void*)(out+sizeof(real32)*2);
 real32 & oMaxU = *(real32*)(void*)(out+sizeof(real32)*3);
 real32 & oMinV = *(real32*)(void*)(out+sizeof(real32)*4);
 real32 & oMaxV = *(real32*)(void*)(out+sizeof(real32)*5);
 
 oMinL = math::Min(aMinL,bMinL);
 oMaxL = math::Max(aMaxL,bMaxL);
 oMinU = math::Min(aMinU,bMinU);
 oMaxU = math::Max(aMaxU,bMaxU);
 oMinV = math::Min(aMinV,bMinV);
 oMaxV = math::Max(aMaxV,bMaxV);  
}

void RangeLuvDSC::Join(nat32 n,const byte ** in,byte * out) const
{
 real32 & oMinL = *(real32*)(void*)(out);
 real32 & oMaxL = *(real32*)(void*)(out+sizeof(real32));
 real32 & oMinU = *(real32*)(void*)(out+sizeof(real32)*2);
 real32 & oMaxU = *(real32*)(void*)(out+sizeof(real32)*3);
 real32 & oMinV = *(real32*)(void*)(out+sizeof(real32)*4);
 real32 & oMaxV = *(real32*)(void*)(out+sizeof(real32)*5);

 bit first = true;
 for (nat32 i=0;i<n;i++)
 {
  if (in[i])
  {
   if (first) mem::Copy(out,in[i],Bytes());
   else
   {
    real32 tMinL = *(real32*)(void*)(in[i]);
    real32 tMaxL = *(real32*)(void*)(in[i]+sizeof(real32));
    real32 tMinU = *(real32*)(void*)(in[i]+sizeof(real32)*2);
    real32 tMaxU = *(real32*)(void*)(in[i]+sizeof(real32)*3);
    real32 tMinV = *(real32*)(void*)(in[i]+sizeof(real32)*4);
    real32 tMaxV = *(real32*)(void*)(in[i]+sizeof(real32)*5);
    
    oMinL = math::Min(oMinL,tMinL);
    oMaxL = math::Max(oMaxL,tMaxL);
    oMinU = math::Min(oMinU,tMinU);
    oMaxU = math::Max(oMaxU,tMaxU);
    oMinV = math::Min(oMinV,tMinV);
    oMaxV = math::Max(oMaxV,tMaxV);  
   }
  }
 }
}

nat32 RangeLuvDSC::WidthLeft() const
{
 return left.Size(0);
}

nat32 RangeLuvDSC::HeightLeft() const
{
 return left.Size(1);
}

void RangeLuvDSC::Left(nat32 x,nat32 y,byte * out) const
{
 real32 & oMinL = *(real32*)(void*)(out);
 real32 & oMaxL = *(real32*)(void*)(out+sizeof(real32));
 real32 & oMinU = *(real32*)(void*)(out+sizeof(real32)*2);
 real32 & oMaxU = *(real32*)(void*)(out+sizeof(real32)*3);
 real32 & oMinV = *(real32*)(void*)(out+sizeof(real32)*4);
 real32 & oMaxV = *(real32*)(void*)(out+sizeof(real32)*5);
 
 oMinL = left.Get(x,y).l;
 oMaxL = left.Get(x,y).l;
 oMinU = left.Get(x,y).u;
 oMaxU = left.Get(x,y).u;
 oMinV = left.Get(x,y).v;
 oMaxV = left.Get(x,y).v; 
 
 if (x!=0)
 {
  real32 sL = 0.5 * (left.Get(x,y).l + left.Get(x-1,y).l);
  real32 sU = 0.5 * (left.Get(x,y).u + left.Get(x-1,y).u);
  real32 sV = 0.5 * (left.Get(x,y).v + left.Get(x-1,y).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }
 
 if (x+1!=left.Size(0))
 {
  real32 sL = 0.5 * (left.Get(x,y).l + left.Get(x+1,y).l);
  real32 sU = 0.5 * (left.Get(x,y).u + left.Get(x+1,y).u);
  real32 sV = 0.5 * (left.Get(x,y).v + left.Get(x+1,y).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }

 if (y!=0)
 {
  real32 sL = 0.5 * (left.Get(x,y).l + left.Get(x,y-1).l);
  real32 sU = 0.5 * (left.Get(x,y).u + left.Get(x,y-1).u);
  real32 sV = 0.5 * (left.Get(x,y).v + left.Get(x,y-1).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }
 
 if (y+1!=left.Size(1))
 {
  real32 sL = 0.5 * (left.Get(x,y).l + left.Get(x,y+1).l);
  real32 sU = 0.5 * (left.Get(x,y).u + left.Get(x,y+1).u);
  real32 sV = 0.5 * (left.Get(x,y).v + left.Get(x,y+1).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }

 oMinL *= mult;
 oMaxL *= mult;
 oMinU *= mult;
 oMaxU *= mult;
 oMinV *= mult;
 oMaxV *= mult;
}

nat32 RangeLuvDSC::WidthRight() const
{
 return right.Size(0);
}

nat32 RangeLuvDSC::HeightRight() const
{
 return right.Size(1);
}

void RangeLuvDSC::Right(nat32 x,nat32 y,byte * out) const
{
 real32 & oMinL = *(real32*)(void*)(out);
 real32 & oMaxL = *(real32*)(void*)(out+sizeof(real32));
 real32 & oMinU = *(real32*)(void*)(out+sizeof(real32)*2);
 real32 & oMaxU = *(real32*)(void*)(out+sizeof(real32)*3);
 real32 & oMinV = *(real32*)(void*)(out+sizeof(real32)*4);
 real32 & oMaxV = *(real32*)(void*)(out+sizeof(real32)*5);
 
 oMinL = right.Get(x,y).l;
 oMaxL = right.Get(x,y).l;
 oMinU = right.Get(x,y).u;
 oMaxU = right.Get(x,y).u;
 oMinV = right.Get(x,y).v;
 oMaxV = right.Get(x,y).v;
 
 if (x!=0)
 {
  real32 sL = 0.5 * (right.Get(x,y).l + right.Get(x-1,y).l);
  real32 sU = 0.5 * (right.Get(x,y).u + right.Get(x-1,y).u);
  real32 sV = 0.5 * (right.Get(x,y).v + right.Get(x-1,y).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }
 
 if (x+1!=right.Size(0))
 {
  real32 sL = 0.5 * (right.Get(x,y).l + right.Get(x+1,y).l);
  real32 sU = 0.5 * (right.Get(x,y).u + right.Get(x+1,y).u);
  real32 sV = 0.5 * (right.Get(x,y).v + right.Get(x+1,y).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 } 

 if (y!=0)
 {
  real32 sL = 0.5 * (right.Get(x,y).l + right.Get(x,y-1).l);
  real32 sU = 0.5 * (right.Get(x,y).u + right.Get(x,y-1).u);
  real32 sV = 0.5 * (right.Get(x,y).v + right.Get(x,y-1).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 }
 
 if (y+1!=right.Size(1))
 {
  real32 sL = 0.5 * (right.Get(x,y).l + right.Get(x,y+1).l);
  real32 sU = 0.5 * (right.Get(x,y).u + right.Get(x,y+1).u);
  real32 sV = 0.5 * (right.Get(x,y).v + right.Get(x,y+1).v);
  
  oMinL = math::Min(oMinL,sL);
  oMaxL = math::Max(oMaxL,sL);
  oMinU = math::Min(oMinU,sU);
  oMaxU = math::Max(oMaxU,sU);
  oMinV = math::Min(oMinV,sV);
  oMaxV = math::Max(oMaxV,sV);  
 } 

 oMinL *= mult;
 oMaxL *= mult;
 oMinU *= mult;
 oMaxU *= mult;
 oMinV *= mult;
 oMaxV *= mult;
}

real32 RangeLuvDSC::Cost(nat32 leftX,nat32 rightX,nat32 y) const
{
 byte temp[sizeof(real32) * 12];
 Left(leftX,y,temp);
 Right(rightX,y,temp + sizeof(real32)*6);
 return Cost(temp,temp + sizeof(real32)*6);
}

cstrconst RangeLuvDSC::TypeString() const
{
 return "eos::stereo::RangeLuvDSC";
}

//------------------------------------------------------------------------------
RegionDSC::RegionDSC(const DSC * dsc,nat32 r,real32 cornerWeight)
:radius(r),dim(r*2+1)
{
 if (radius!=0) SetFalloff(-math::Ln(cornerWeight)/math::Sqrt(2.0*real32(radius)*real32(radius)));
           else SetFalloff(1.0);
 child = dsc->Clone();
}

RegionDSC::~RegionDSC()
{
 delete child;
}

DSC * RegionDSC::Clone() const
{
 RegionDSC * ret = new RegionDSC(child,radius);
 ret->SetFalloff(falloff);
 return ret;
}

void RegionDSC::SetFalloff(real32 fo)
{
 falloff = fo;
 
 // Recalculate the weight array...
  real32 sum = 0.0;
  weight.Size(dim*dim);
  nat32 i = 0;
  for (int32 v=-int32(radius);v<=int32(radius);v++)
  {
   for (int32 u=-int32(radius);u<=int32(radius);u++)
   {
    weight[i] = math::Exp(-falloff*math::Sqrt(real32(v*v + u*u)));
    sum += weight[i];
    i += 1;
   }
  }
  
  for (nat32 i=0;i<weight.Size();i++) weight[i] /= sum;
}


nat32 RegionDSC::Bytes() const
{
 return weight.Size()*child->Bytes();
}

real32 RegionDSC::Cost(const byte * left,const byte * right) const
{
 real32 ret = 0.0;
 for (nat32 i=0;i<weight.Size();i++)
 {
  ret += weight[i]*child->Cost(left,right);
  left += child->Bytes();
  right += child->Bytes();
 }
 return ret;
}

void RegionDSC::Join(const byte * left,const byte * right,byte * out) const
{
 for (nat32 i=0;i<weight.Size();i++)
 {
  child->Join(left,right,out);
  left += child->Bytes();
  right += child->Bytes();
  out += child->Bytes();
 }
}

void RegionDSC::Join(nat32 n,const byte ** in,byte * out) const
{
 for (nat32 i=0;i<weight.Size();i++)
 {
  child->Join(n,in,out);
  
  out += child->Bytes();
  for (nat32 j=0;j<n;j++)
  {
   if (in[j]!=null<const byte*>()) in[j] += child->Bytes();
  }
 }
 
 for (nat32 j=0;j<n;j++)
 {
  if (in[j]!=null<const byte*>()) in[j] -= weight.Size()*child->Bytes();
 }
}

nat32 RegionDSC::WidthLeft() const
{
 return child->WidthLeft();
}

nat32 RegionDSC::HeightLeft() const
{
 return child->HeightLeft();
}

void RegionDSC::Left(nat32 x,nat32 y,byte * out) const
{
 for (int32 v=-int32(radius);v<=int32(radius);v++)
 {
  for (int32 u=-int32(radius);u<=int32(radius);u++)
  {
   int32 nx = math::Clamp<int32>(int32(x)+u,0,child->WidthLeft()-1);
   int32 ny = math::Clamp<int32>(int32(y)+v,0,child->HeightLeft()-1);
   
   child->Left(nx,ny,out);
   out += child->Bytes();
  }
 }
}

nat32 RegionDSC::WidthRight() const
{
 return child->WidthRight();
}

nat32 RegionDSC::HeightRight() const
{
 return child->HeightRight();
}

void RegionDSC::Right(nat32 x,nat32 y,byte * out) const
{
 for (int32 v=-int32(radius);v<=int32(radius);v++)
 {
  for (int32 u=-int32(radius);u<=int32(radius);u++)
  {
   int32 nx = math::Clamp<int32>(int32(x)+u,0,child->WidthRight()-1);
   int32 ny = math::Clamp<int32>(int32(y)+v,0,child->HeightRight()-1);
   
   child->Right(nx,ny,out);
   out += child->Bytes();
  }
 }
}

real32 RegionDSC::Cost(nat32 leftX,nat32 rightX,nat32 y) const
{
 real32 ret = 0.0;
 
 nat32 i = 0;
 for (int32 v=-int32(radius);v<=int32(radius);v++)
 {
  for (int32 u=-int32(radius);u<=int32(radius);u++)
  {
   int32 nxl = math::Clamp<int32>(int32(leftX)+u,0,child->WidthLeft()-1);
   int32 nxr = math::Clamp<int32>(int32(rightX)+u,0,child->WidthRight()-1);
   int32 ny = math::Clamp<int32>(int32(y)+v,0,child->HeightRight()-1);
   
   ret += weight[i]*child->Cost(nxl,nxr,ny);
   
   i += 1;
  }
 }
 
 return ret;
}

cstrconst RegionDSC::TypeString() const
{
 return "eos::stereo::RegionDSC";
}

//------------------------------------------------------------------------------
LuvRegionDSC::LuvRegionDSC(const svt::Field<bs::ColourLuv> & le,const svt::Field<bs::ColourLuv> & ri,const DSC * dsc,nat32 r,real32 m,real32 cornerWeight)
:radius(r),dim(r*2+1),mult(m),left(le),right(ri)
{
 if (radius!=0) SetFalloff(-math::Ln(cornerWeight)/math::Sqrt(2.0*real32(radius)*real32(radius)));
           else SetFalloff(1.0);
 child = dsc->Clone();
}

LuvRegionDSC::~LuvRegionDSC()
{
 delete child;
}

DSC * LuvRegionDSC::Clone() const
{
 LuvRegionDSC * ret = new LuvRegionDSC(left,right,child,radius,mult);
 ret->SetFalloff(falloff);
 return ret;
}

void LuvRegionDSC::SetFalloff(real32 fo)
{
 falloff = fo;
 
 // Recalculate the weight array...
  real32 sum = 0.0;
  weight.Size(dim*dim);
  nat32 i = 0;
  for (int32 v=-int32(radius);v<=int32(radius);v++)
  {
   for (int32 u=-int32(radius);u<=int32(radius);u++)
   {
    weight[i] = math::Exp(-falloff*math::Sqrt(real32(v*v + u*u)));
    sum += weight[i];
    i += 1;
   }
  }
  
  for (nat32 i=0;i<weight.Size();i++) weight[i] /= sum;
}

nat32 LuvRegionDSC::Bytes() const
{
 return weight.Size()*child->Bytes();
}

real32 LuvRegionDSC::Cost(const byte * left,const byte * right) const
{
 real32 ret = 0.0;
 for (nat32 i=0;i<weight.Size();i++)
 {
  ret += weight[i]*child->Cost(left,right);
  left += child->Bytes();
  right += child->Bytes();
 }
 return ret;
}

void LuvRegionDSC::Join(const byte * left,const byte * right,byte * out) const
{
 for (nat32 i=0;i<weight.Size();i++)
 {
  child->Join(left,right,out);
  left += child->Bytes();
  right += child->Bytes();
  out += child->Bytes();
 }
}

void LuvRegionDSC::Join(nat32 n,const byte ** in,byte * out) const
{
 for (nat32 i=0;i<weight.Size();i++)
 {
  child->Join(n,in,out);
  
  out += child->Bytes();
  for (nat32 j=0;j<n;j++)
  {
   if (in[j]!=null<const byte*>()) in[j] += child->Bytes();
  }
 }
 
 for (nat32 j=0;j<n;j++)
 {
  if (in[j]!=null<const byte*>()) in[j] -= weight.Size()*child->Bytes();
 }
}

nat32 LuvRegionDSC::WidthLeft() const
{
 return child->WidthLeft();
}

nat32 LuvRegionDSC::HeightLeft() const
{
 return child->HeightLeft();
}

void LuvRegionDSC::Left(nat32 x,nat32 y,byte * out) const
{
 for (int32 v=-int32(radius);v<=int32(radius);v++)
 {
  for (int32 u=-int32(radius);u<=int32(radius);u++)
  {
   int32 nx = math::Clamp<int32>(int32(x)+u,0,child->WidthLeft()-1);
   int32 ny = math::Clamp<int32>(int32(y)+v,0,child->HeightLeft()-1);
   
   child->Left(nx,ny,out);
   out += child->Bytes();
  }
 }
}

nat32 LuvRegionDSC::WidthRight() const
{
 return child->WidthRight();
}

nat32 LuvRegionDSC::HeightRight() const
{
 return child->HeightRight();
}

void LuvRegionDSC::Right(nat32 x,nat32 y,byte * out) const
{
 for (int32 v=-int32(radius);v<=int32(radius);v++)
 {
  for (int32 u=-int32(radius);u<=int32(radius);u++)
  {
   int32 nx = math::Clamp<int32>(int32(x)+u,0,child->WidthRight()-1);
   int32 ny = math::Clamp<int32>(int32(y)+v,0,child->HeightRight()-1);
   
   child->Right(nx,ny,out);
   out += child->Bytes();
  }
 }
}

real32 LuvRegionDSC::Cost(nat32 leftX,nat32 rightX,nat32 y) const
{
 real32 ret = 0.0;
 
 nat32 i = 0;
 real32 totWeight = 0.0;
 for (int32 v=-int32(radius);v<=int32(radius);v++)
 {
  for (int32 u=-int32(radius);u<=int32(radius);u++)
  {
   int32 nxl = math::Clamp<int32>(int32(leftX)+u,0,child->WidthLeft()-1);
   int32 nxr = math::Clamp<int32>(int32(rightX)+u,0,child->WidthRight()-1);
   int32 ny = math::Clamp<int32>(int32(y)+v,0,child->HeightRight()-1);
   
   real32 ld = left.Get(leftX,y).Diff(left.Get(nxl,ny));
   real32 rd = right.Get(rightX,y).Diff(right.Get(nxr,ny));
   real32 we = weight[i] * math::Exp(-mult*math::Min(ld,rd));
   
   ret += we*child->Cost(nxl,nxr,ny);
   totWeight += we;
   
   i += 1;
  }
 }
 
 ret /= totWeight;
 
 return ret;
}

cstrconst LuvRegionDSC::TypeString() const
{
 return "eos::stereo::LuvRegionDSC";
}

//------------------------------------------------------------------------------
ManhattanDSC::ManhattanDSC()
:bytes(0)
{}

ManhattanDSC::~ManhattanDSC()
{
 for (nat32 i=0;i<dsc.Size();i++) delete dsc[i];
}

DSC * ManhattanDSC::Clone() const
{
 ManhattanDSC * ret = new ManhattanDSC();
 for (nat32 i=0;i<dsc.Size();i++) ret->Add(dsc[i]);
 return ret;
}

void ManhattanDSC::Add(const DSC * d)
{
 nat32 pos = dsc.Size();
 dsc.Size(pos+1);
 dsc[pos] = d->Clone();
 bytes += dsc[pos]->Bytes();
}

nat32 ManhattanDSC::Bytes() const
{
 return bytes;
}

real32 ManhattanDSC::Cost(const byte * left,const byte * right) const
{
 real32 ret = 0.0;
 nat32 offset = 0;
 for (nat32 i=0;i<dsc.Size();i++)
 {
  ret += dsc[i]->Cost(left+offset,right+offset);
  offset += dsc[i]->Bytes();
 }
 return ret;
}

void ManhattanDSC::Join(const byte * left,const byte * right,byte * out) const
{
 nat32 offset = 0;
 for (nat32 i=0;i<dsc.Size();i++)
 {
  dsc[i]->Join(left+offset,right+offset,out+offset);
  offset += dsc[i]->Bytes();
 }
}

void ManhattanDSC::Join(nat32 n,const byte ** in,byte * out) const
{
 nat32 sum = 0;
 for (nat32 i=0;i<dsc.Size();i++)
 {
  dsc[i]->Join(n,in,out+sum);
  for (nat32 j=0;j<n;j++) if (in[j]) in[j] += dsc[i]->Bytes();
  sum += dsc[i]->Bytes();
 }
 for (nat32 j=0;j<n;j++) if (in[j]) in[j] -= sum;
}

nat32 ManhattanDSC::WidthLeft() const
{
 return dsc[0]->WidthLeft();
}

nat32 ManhattanDSC::HeightLeft() const
{
 return dsc[0]->HeightLeft();
}

void ManhattanDSC::Left(nat32 x,nat32 y,byte * out) const
{
 nat32 offset = 0;
 for (nat32 i=0;i<dsc.Size();i++)
 {
  dsc[i]->Left(x,y,out+offset);
  offset += dsc[i]->Bytes();
 }
 log::Assert(offset==bytes);
}

nat32 ManhattanDSC::WidthRight() const
{
 return dsc[0]->WidthRight();
}

nat32 ManhattanDSC::HeightRight() const
{
 return dsc[0]->HeightRight();
}

void ManhattanDSC::Right(nat32 x,nat32 y,byte * out) const
{
 nat32 offset = 0;
 for (nat32 i=0;i<dsc.Size();i++)
 {
  dsc[i]->Right(x,y,out+offset);
  offset += dsc[i]->Bytes();
 }
 log::Assert(offset==bytes);
}

cstrconst ManhattanDSC::TypeString() const
{
 return "eos::stereo::ManhattanDSC";
}

//------------------------------------------------------------------------------
EuclideanDSC::EuclideanDSC()
:bytes(0)
{}

EuclideanDSC::~EuclideanDSC()
{
 for (nat32 i=0;i<dsc.Size();i++) delete dsc[i];
}

DSC * EuclideanDSC::Clone() const
{
 ManhattanDSC * ret = new ManhattanDSC();
 for (nat32 i=0;i<dsc.Size();i++) ret->Add(dsc[i]);
 return ret;
}

void EuclideanDSC::Add(const DSC * d)
{
 nat32 pos = dsc.Size();
 dsc.Size(pos+1);
 dsc[pos] = d->Clone();
 bytes += dsc[pos]->Bytes();
}

nat32 EuclideanDSC::Bytes() const
{
 return bytes;
}

real32 EuclideanDSC::Cost(const byte * left,const byte * right) const
{
 real32 ret = 0.0;
 nat32 offset = 0;
 for (nat32 i=0;i<dsc.Size();i++)
 {
  ret += math::Sqr(dsc[i]->Cost(left+offset,right+offset));
  offset += dsc[i]->Bytes();
 }
 return math::Sqrt(ret);
}

void EuclideanDSC::Join(const byte * left,const byte * right,byte * out) const
{
 nat32 offset = 0;
 for (nat32 i=0;i<dsc.Size();i++)
 {
  dsc[i]->Join(left+offset,right+offset,out+offset);
  offset += dsc[i]->Bytes();
 }
}

void EuclideanDSC::Join(nat32 n,const byte ** in,byte * out) const
{
 nat32 sum = 0;
 for (nat32 i=0;i<dsc.Size();i++)
 {
  dsc[i]->Join(n,in,out+sum);
  for (nat32 j=0;j<n;j++) if (in[j]) in[j] += dsc[i]->Bytes();
  sum += dsc[i]->Bytes();
 }
 for (nat32 j=0;j<n;j++) if (in[j]) in[j] -= sum;
}

nat32 EuclideanDSC::WidthLeft() const
{
 return dsc[0]->WidthLeft();
}

nat32 EuclideanDSC::HeightLeft() const
{
 return dsc[0]->HeightLeft();
}

void EuclideanDSC::Left(nat32 x,nat32 y,byte * out) const
{
 nat32 offset = 0;
 for (nat32 i=0;i<dsc.Size();i++)
 {
  dsc[i]->Left(x,y,out+offset);
  offset += dsc[i]->Bytes();
 }
 log::Assert(offset==bytes);
}

nat32 EuclideanDSC::WidthRight() const
{
 return dsc[0]->WidthRight();
}

nat32 EuclideanDSC::HeightRight() const
{
 return dsc[0]->HeightRight();
}

void EuclideanDSC::Right(nat32 x,nat32 y,byte * out) const
{
 nat32 offset = 0;
 for (nat32 i=0;i<dsc.Size();i++)
 {
  dsc[i]->Right(x,y,out+offset);
  offset += dsc[i]->Bytes();
 }
 log::Assert(offset==bytes);
}

cstrconst EuclideanDSC::TypeString() const
{
 return "eos::stereo::EuclideanDSC";
}

//------------------------------------------------------------------------------
SwapDSC::SwapDSC(const DSC * d)
:dsc(d->Clone())
{}

SwapDSC::~SwapDSC()
{
 delete dsc;
}

DSC * SwapDSC::Clone() const
{
 return new SwapDSC(dsc);
}

nat32 SwapDSC::Bytes() const
{
 return dsc->Bytes();
}

real32 SwapDSC::Cost(const byte * left,const byte * right) const
{
 return dsc->Cost(left,right);
}

void SwapDSC::Join(const byte * left,const byte * right,byte * out) const
{
 dsc->Join(left,right,out);
}

void SwapDSC::Join(nat32 n,const byte ** in,byte * out) const
{
 dsc->Join(n,in,out);
}

nat32 SwapDSC::WidthLeft() const
{
 return dsc->WidthRight();
}

nat32 SwapDSC::HeightLeft() const
{
 return dsc->HeightRight();
}

void SwapDSC::Left(nat32 x,nat32 y,byte * out) const
{
 dsc->Right(x,y,out);
}

nat32 SwapDSC::WidthRight() const
{
 return dsc->WidthLeft();
}

nat32 SwapDSC::HeightRight() const
{
 return dsc->HeightLeft();
}

void SwapDSC::Right(nat32 x,nat32 y,byte * out) const
{
 dsc->Left(x,y,out);
}

real32 SwapDSC::Cost(nat32 leftX,nat32 rightX,nat32 y) const
{
 return dsc->Cost(rightX,leftX,y);
}

cstrconst SwapDSC::TypeString() const
{
 return "eos::stereo::SwapDSC";
}

//------------------------------------------------------------------------------
HierarchyDSC::HierarchyDSC()
{}

HierarchyDSC::~HierarchyDSC()
{
 // Take out the (multiple pointed to) dsc...
  if (levels.Size()>0) delete levels[0].dsc;
  
 // Wipe out the layer storage...
  for (nat32 i=0;i<levels.Size();i++)
  {
   delete[] levels[i].leftData;
   delete[] levels[i].rightData;
  }
  
 // Deal with the masks...
  for (nat32 l=0;l<leftMask.Size();l++) delete leftMask[l];
  for (nat32 l=0;l<rightMask.Size();l++) delete rightMask[l];
}

void HierarchyDSC::Set(const DSC & d,const svt::Field<bit> & lm,const svt::Field<bit> & rm)
{
 LogBlock("eos::stereo::HierarchyDSC::Set","");

 // Clone the dsc...
  DSC * dsc = d.Clone();


 // Calculate how many levels are required, resize levels accordingly...
  int32 lCount = math::Max(math::TopBit(dsc->WidthLeft()),
                           math::TopBit(dsc->HeightLeft()),
                           math::TopBit(dsc->WidthRight()));
  levels.Size(lCount);
  leftMask.Size(lCount);
  rightMask.Size(lCount);


 // Fill in the dsc and dimension parameters for each level...
  levels[0].dsc = dsc;
  levels[0].widthLeft = dsc->WidthLeft();
  levels[0].widthRight = dsc->WidthRight();
  levels[0].height = dsc->HeightLeft();

  for (nat32 l=1;l<levels.Size();l++)
  {
   levels[l].dsc = dsc;
   levels[l].widthLeft = (levels[l-1].widthLeft/2) + ((levels[l-1].widthLeft&1)?1:0);
   levels[l].widthRight = (levels[l-1].widthRight/2) + ((levels[l-1].widthRight&1)?1:0);
   levels[l].height = (levels[l-1].height/2) + ((levels[l-1].height&1)?1:0);
  }
  
  
 // Setup the left mask if applicable...
  if (lm.Valid())
  {
   bit maskIni = false;
   leftMask[0] = new svt::Var(lm);
   leftMask[0]->Add("mask",maskIni);
   leftMask[0]->Commit(false);
   
   svt::Field<bit> m(leftMask[0],"mask");
   m.CopyFrom(lm);
   
   for (nat32 l=1;l<leftMask.Size();l++)
   {
    leftMask[l] = new svt::Var(lm);
    leftMask[l]->Setup2D(levels[l].widthLeft,levels[l].height);
    leftMask[l]->Add("mask",maskIni);
    leftMask[l]->Commit(false);
    
    svt::Field<bit> m(leftMask[l],"mask");
    svt::Field<bit> pm(leftMask[l-1],"mask");
    for (nat32 y=0;y<m.Size(1);y++)
    {
     for (nat32 x=0;x<m.Size(0);x++)
     {
      nat32 fromX = x*2;
      nat32 fromY = y*2;
     
      bit okX = (fromX+1)<pm.Size(0);
      bit okY = (fromY+1)<pm.Size(1);

      m.Get(x,y) = pm.Get(fromX,fromY);
      if (okX) m.Get(x,y) |= pm.Get(fromX+1,fromY);
      if (okY) m.Get(x,y) |= pm.Get(fromX,fromY+1);
      if (okX&&okY) m.Get(x,y) |= pm.Get(fromX+1,fromY+1);
     }
    }
   }
  }
  else
  {
   for (nat32 l=0;l<leftMask.Size();l++) leftMask[l] = null<svt::Var*>();
  }


 // Setup the right mask if applicable...
  if (rm.Valid())
  {
   bit maskIni = false;
   rightMask[0] = new svt::Var(rm);
   rightMask[0]->Add("mask",maskIni);
   rightMask[0]->Commit(false);
   
   svt::Field<bit> m(rightMask[0],"mask");
   m.CopyFrom(rm);
   
   for (nat32 l=1;l<rightMask.Size();l++)
   {
    rightMask[l] = new svt::Var(rm);
    rightMask[l]->Setup2D(levels[l].widthRight,levels[l].height);
    rightMask[l]->Add("mask",maskIni);
    rightMask[l]->Commit(false);
    
    svt::Field<bit> m(rightMask[l],"mask");
    svt::Field<bit> pm(rightMask[l-1],"mask");
    for (nat32 y=0;y<m.Size(1);y++)
    {
     for (nat32 x=0;x<m.Size(0);x++)
     {
      nat32 fromX = x*2;
      nat32 fromY = y*2;
     
      bit okX = (fromX+1)<pm.Size(0);
      bit okY = (fromY+1)<pm.Size(1);

      m.Get(x,y) = pm.Get(fromX,fromY);
      if (okX) m.Get(x,y) |= pm.Get(fromX+1,fromY);
      if (okY) m.Get(x,y) |= pm.Get(fromX,fromY+1);
      if (okX&&okY) m.Get(x,y) |= pm.Get(fromX+1,fromY+1);
     }
    }
   }
  }
  else
  {
   for (nat32 l=0;l<rightMask.Size();l++) rightMask[l] = null<svt::Var*>();
  }


 // Fill in the first levels data blocks (Can ignore mask)...
  levels[0].leftData = new byte[dsc->Bytes() * levels[0].widthLeft * levels[0].height];
  byte * targ = levels[0].leftData;
  for (nat32 y=0;y<levels[0].height;y++)
  {
   for (nat32 x=0;x<levels[0].widthLeft;x++)
   {
    dsc->Left(x,y,targ);
    targ += dsc->Bytes();
   }
  }

  levels[0].rightData = new byte[dsc->Bytes() * levels[0].widthRight * levels[0].height];
  targ = levels[0].rightData;
  for (nat32 y=0;y<levels[0].height;y++)
  {
   for (nat32 x=0;x<levels[0].widthRight;x++)
   {
    dsc->Right(x,y,targ);
    targ += dsc->Bytes();
   }
  }


 // Fill in all further levels data blocks (Have to handle masking)...
  for (nat32 l=1;l<levels.Size();l++)
  {
   svt::Field<bit> m;

   if (leftMask[l-1]) leftMask[l-1]->ByName("mask",m);  
   levels[l].leftData = new byte[dsc->Bytes() * levels[l].widthLeft * levels[l].height];
   byte * targ = levels[l].leftData;   
   for (nat32 y=0;y<levels[l].height;y++)
   {
    for (nat32 x=0;x<levels[l].widthLeft;x++)
    {
     nat32 fromX = x*2;
     nat32 fromY = y*2;
     
     bit okX = (fromX+1)<levels[l-1].widthLeft;
     bit okY = (fromY+1)<levels[l-1].height;
     
     const byte * from[4];
     from[0] = levels[l-1].leftData + dsc->Bytes() * (levels[l-1].widthLeft*fromY + fromX);
     if (okX) from[1] = levels[l-1].leftData + dsc->Bytes() * (levels[l-1].widthLeft*fromY + (fromX+1));
         else from[1] = null<byte*>();
     if (okY) from[2] = levels[l-1].leftData + dsc->Bytes() * (levels[l-1].widthLeft*(fromY+1) + fromX);
         else from[2] = null<byte*>();
     if (okX&&okY) from[3] = levels[l-1].leftData + dsc->Bytes() * (levels[l-1].widthLeft*(fromY+1) + (fromX+1));
              else from[3] = null<byte*>();
     
     if (m.Valid())
     {
      if (m.Get(fromX,fromY)==false) from[0] = null<byte*>();
      if ((from[1])&&(m.Get(fromX+1,fromY)==false)) from[1] = null<byte*>();
      if ((from[2])&&(m.Get(fromX,fromY+1)==false)) from[2] = null<byte*>();
      if ((from[3])&&(m.Get(fromX+1,fromY+1)==false)) from[3] = null<byte*>();
     }
     
     dsc->Join(4,from,targ);
     targ += dsc->Bytes();
    }
   }
   

   if (rightMask[l-1]) rightMask[l-1]->ByName("mask",m);
   levels[l].rightData = new byte[dsc->Bytes() * levels[l].widthRight * levels[l].height];
   targ = levels[l].rightData;
   for (nat32 y=0;y<levels[l].height;y++)
   {
    for (nat32 x=0;x<levels[l].widthRight;x++)
    {
     nat32 fromX = x*2;
     nat32 fromY = y*2;

     bit okX = (fromX+1)<levels[l-1].widthRight;
     bit okY = (fromY+1)<levels[l-1].height;

     const byte * from[4];
     from[0] = levels[l-1].rightData + dsc->Bytes() * (levels[l-1].widthRight*fromY + fromX);
     if (okX) from[1] = levels[l-1].rightData + dsc->Bytes() * (levels[l-1].widthRight*fromY + (fromX+1));
         else from[1] = null<byte*>();
     if (okY) from[2] = levels[l-1].rightData + dsc->Bytes() * (levels[l-1].widthRight*(fromY+1) + fromX);
         else from[2] = null<byte*>();
     if (okX&&okY) from[3] = levels[l-1].rightData + dsc->Bytes() * (levels[l-1].widthRight*(fromY+1) + (fromX+1));
              else from[3] = null<byte*>();

     if (m.Valid())
     {
      if (m.Get(fromX,fromY)==false) from[0] = null<byte*>();
      if ((from[1])&&(m.Get(fromX+1,fromY)==false)) from[1] = null<byte*>();
      if ((from[2])&&(m.Get(fromX,fromY+1)==false)) from[2] = null<byte*>();
      if ((from[3])&&(m.Get(fromX+1,fromY+1)==false)) from[3] = null<byte*>();
     }

     dsc->Join(4,from,targ);
     targ += dsc->Bytes();
    }
   }
  }
}

nat32 HierarchyDSC::Levels() const
{
 return levels.Size();
}

const DSC & HierarchyDSC::Level(nat32 l) const
{
 return levels[l];
}

svt::Field<bit> HierarchyDSC::LeftMask(nat32 l) const
{
 svt::Field<bit> ret;
 if (leftMask[l]) leftMask[l]->ByName("mask",ret);
 return ret;
}

svt::Field<bit> HierarchyDSC::RightMask(nat32 l) const
{
 svt::Field<bit> ret;
 if (rightMask[l]) rightMask[l]->ByName("mask",ret);
 return ret;
}

cstrconst HierarchyDSC::TypeString() const
{
 return "eos::stereo::HierarchyDSC";
}

//------------------------------------------------------------------------------
HierarchyDSC::HierLevel::HierLevel()
{}

HierarchyDSC::HierLevel::~HierLevel()
{}

DSC * HierarchyDSC::HierLevel::Clone() const
{
 HierLevel * ret = new HierLevel();

 ret->dsc = dsc;
 ret->widthLeft = widthLeft;
 ret->widthRight = widthRight;
 ret->height = height;
 ret->leftData = leftData;
 ret->rightData = rightData;

 return ret;
}

nat32 HierarchyDSC::HierLevel::Bytes() const
{
 return dsc->Bytes();
}

real32 HierarchyDSC::HierLevel::Cost(const byte * left,const byte * right) const
{
 return dsc->Cost(left,right);
}

void HierarchyDSC::HierLevel::Join(const byte * left,const byte * right,byte * out) const
{
 dsc->Join(left,right,out);
}

void HierarchyDSC::HierLevel::Join(nat32 n,const byte ** in,byte * out) const
{
 dsc->Join(n,in,out);
}

nat32 HierarchyDSC::HierLevel::WidthLeft() const
{
 return widthLeft;
}

nat32 HierarchyDSC::HierLevel::HeightLeft() const
{
 return height;
}

void HierarchyDSC::HierLevel::Left(nat32 x,nat32 y,byte * out) const
{
 mem::Copy(out,leftData + dsc->Bytes()*(widthLeft*y + x),dsc->Bytes());
}

nat32 HierarchyDSC::HierLevel::WidthRight() const
{
 return widthRight;
}

nat32 HierarchyDSC::HierLevel::HeightRight() const
{
 return height;
}

void HierarchyDSC::HierLevel::Right(nat32 x,nat32 y,byte * out) const
{
 mem::Copy(out,rightData + dsc->Bytes()*(widthRight*y + x),dsc->Bytes());
}

real32 HierarchyDSC::HierLevel::Cost(nat32 leftX,nat32 rightX,nat32 y) const
{
 byte * l = leftData + dsc->Bytes()*(widthLeft*y + leftX);
 byte * r = rightData + dsc->Bytes()*(widthRight*y + rightX);
 return dsc->Cost(l,r);
}

cstrconst HierarchyDSC::HierLevel::TypeString() const
{
 return "eos::stereo::HierarchyDSC::HierLevel";
}

//------------------------------------------------------------------------------
DSI::~DSI()
{}

real32 DSI::Cost(nat32 x,nat32 y,nat32 i) const
{
 return -math::Ln(Prob(x,y,i));
}

real32 DSI::Prob(nat32 x,nat32 y,nat32 i) const
{
 return math::Exp(-Cost(x,y,i));
}

real32 DSI::DispWidth(nat32 x,nat32 y,nat32 i) const
{
 return 0.0;
}

void DSI::GetDisp(svt::Field<real32> & out) const
{
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++)
  {
   if (Size(x,y)==0) out.Get(x,y) = 0.0;
   else
   {
    out.Get(x,y) = Disp(x,y,0);
    real32 bestCost = Cost(x,y,0);
    for (nat32 i=1;i<Size(x,y);i++)
    {
     if (Cost(x,y,i)<bestCost)
     {
      out.Get(x,y) = Disp(x,y,i);
      bestCost = Cost(x,y,i);
     }
    }
   }
  }
 }
}

void DSI::GetMask(svt::Field<bit> & out) const
{
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++)
  {
   out.Get(x,y) = Size(x,y)!=0;
  }
 }
}

//------------------------------------------------------------------------------
DummyDSI::DummyDSI(const svt::Field<real32> & d,const svt::Field<bit> * m)
{
 disp = d;
 if (m) mask = *m;
}

DummyDSI::~DummyDSI()
{}

nat32 DummyDSI::Width() const
{
 return disp.Size(0);
}

nat32 DummyDSI::Height() const
{
 return disp.Size(1);
}

nat32 DummyDSI::Size(nat32 x,nat32 y) const
{
 if (mask.Valid())
 {
  if (mask.Get(x,y)) return 1;
                else return 0;
 }
 else return 1;
}

real32 DummyDSI::Disp(nat32 x,nat32 y,nat32 i) const
{
 return disp.Get(x,y);
}

real32 DummyDSI::Cost(nat32 x,nat32 y,nat32 i) const
{
 return 0.0;
}

real32 DummyDSI::Prob(nat32 x,nat32 y,nat32 i) const
{
 return 1.0;
}

real32 DummyDSI::DispWidth(nat32 x,nat32 y,nat32 i) const
{
 return 0.0;
}

cstrconst DummyDSI::TypeString() const
{
 return "eos::stereo::DummyDSI";
}

//------------------------------------------------------------------------------
 };
};
