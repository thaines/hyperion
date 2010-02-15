//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

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

#include "eos/inf/fg_funcs.h"

#include "eos/math/gaussian_mix.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace inf
 {
//------------------------------------------------------------------------------
FunctionSet::FunctionSet(Function * f,nat32 inst)
:func(f),instances(inst),links(0),mc(null<MessageClass*>()),
data(null<byte*>()),stride(0),in(null<nat32*>()),out(null<nat32*>())
{
 links = func->Links();
 mc = new MessageClass[links]; if (mc==null<MessageClass*>()) return;
 for (nat32 i=0;i<links;i++) func->LinkType(i,mc[i]);
 
 in = new nat32[links]; if (in==null<nat32*>()) return;
 out = new nat32[links]; if (out==null<nat32*>()) return; 
 
 in[0] = 0;
 for (nat32 i=0;i<links-1;i++) in[i+1] = in[i] + mc[i].Size(mc[i]);
 stride = in[links-1] + mc[links-1].Size(mc[links-1]);
 
 for (nat32 i=0;i<links;i++) out[i] = in[i]+stride;
 stride *= 2;
 
 data = mem::Malloc<byte>(stride * instances);
}

FunctionSet::~FunctionSet()
{
 mem::Free(data);
 delete[] out;
 delete[] in;
 delete[] mc;
}

void FunctionSet::Flatline()
{
 byte * targ = data;
 for (nat32 i=0;i<instances;i++)
 {
  for (nat32 j=0;j<links;j++) mc[j].Flatline(mc[j],targ + in[j]);
  for (nat32 j=0;j<links;j++) mc[j].Flatline(mc[j],targ + out[j]);
  targ += stride;
 }
}

void FunctionSet::FlatlineLn()
{
 byte * targ = data;
 for (nat32 i=0;i<instances;i++)
 {
  for (nat32 j=0;j<links;j++) mc[j].FlatlineLn(mc[j],targ + in[j]);
  for (nat32 j=0;j<links;j++) mc[j].FlatlineLn(mc[j],targ + out[j]);
  targ += stride;
 }
}

//------------------------------------------------------------------------------
Distribution::Distribution(nat32 i,nat32 l)
:labels(l),instances(i),data(new Prob[l*i])
{
 for (nat32 i=0;i<labels*instances;i++)
 {
  data[i].ms = false;
  data[i].val = 1.0;
 }
}

Distribution::~Distribution()
{
 delete[] data;
}

void Distribution::SetFreq(nat32 instance,nat32 label,real32 freq)
{
 data[instance*labels+label].ms = false;
 data[instance*labels+label].val = freq;
}

void Distribution::SetFreq(nat32 instance,const real32 * freqs)
{
 for (nat32 i=0;i<labels;i++)
 {
  data[instance*labels + i].ms = false;
  data[instance*labels + i].val = freqs[i];
 }
}

void Distribution::SetFreq(nat32 instance,const math::Vector<real32> & freqs)
{
 for (nat32 i=0;i<labels;i++)
 {
  data[instance*labels + i].ms = false;
  data[instance*labels + i].val = freqs[i];
 }
}

void Distribution::SetCost(nat32 instance,nat32 label,real32 cost)
{
 data[instance*labels+label].ms = true;
 data[instance*labels+label].val = cost;
}

void Distribution::SetCost(nat32 instance,const real32 * costs)
{
 for (nat32 i=0;i<labels;i++)
 {
  data[instance*labels + i].ms = true;
  data[instance*labels + i].val = costs[i];
 }
}

void Distribution::SetCost(nat32 instance,const math::Vector<real32> & costs)
{
 for (nat32 i=0;i<labels;i++)
 {
  data[instance*labels + i].ms = true;
  data[instance*labels + i].val = costs[i];
 }
}

nat32 Distribution::Links() const
{
 return 1;
}

void Distribution::LinkType(nat32,MessageClass & out) const
{
 Frequency::MakeClass(labels,out);
}

void Distribution::ToSP()
{
 for (nat32 i=0;i<instances;i++)
 {
  Prob * targ = data + i*labels;
  
  real32 sum = 0.0;
  for (nat32 l=0;l<labels;l++)
  {
   if (targ[l].ms==true)
   {
    targ[l].val = math::Max<real32>(math::Exp(-targ[l].val),0.0);
    targ[l].ms = false;
   }
   sum += targ[l].val;
  }
  
  sum = 1.0/sum;
  for (nat32 l=0;l<labels;l++) targ[l].val *= sum;
 }
}

void Distribution::SendOneSP(nat32 instance,const MessageSet & ms,nat32)
{
 instance = instance%instances;
 
 Frequency out = ms.GetOut<Frequency>(0);
 for (nat32 i=0;i<labels;i++) out[i] = data[instance*labels + i].val;
 out.Norm();
}

void Distribution::SendAllButOneSP(nat32,const MessageSet &,nat32)
{
 // Theres only one link - there can't be any others to send! Time for whistling practise...
}

void Distribution::SendAllSP(nat32 instance,const MessageSet & ms)
{
 instance = instance%instances;
 
 Frequency out = ms.GetOut<Frequency>(0);
 for (nat32 i=0;i<labels;i++) out[i] = data[instance*labels + i].val;
 out.Norm();
}

void Distribution::ToMS()
{
 for (nat32 i=0;i<instances;i++)
 {
  Prob * targ = data + i*labels;
  
  real32 minVal = math::Infinity<real32>();
  for (nat32 l=0;l<labels;l++)
  {
   if (targ[l].ms==false)
   {
    if (math::IsZero(targ[l].val)) targ[l].val = 100.0;
                              else targ[l].val = math::Min(-math::Ln(targ[l].val),real32(100.0));
    targ[l].ms = true;
   }
   minVal = math::Min(minVal,targ[l].val);
  }
  
  for (nat32 l=0;l<labels;l++) targ[l].val -= minVal;
 }
}

void Distribution::SendOneMS(nat32 instance,const MessageSet & ms,nat32 mtc)
{
 instance = instance%instances;

 Frequency out = ms.GetOut<Frequency>(0);
 for (nat32 i=0;i<labels;i++) out[i] = data[instance*labels + i].val;
}

void Distribution::SendAllButOneMS(nat32 instance,const MessageSet & ms,nat32 mti)
{
 // Theres only one link - there can't be any others to send! Time for whistling practise...
}

void Distribution::SendAllMS(nat32 instance,const MessageSet & ms)
{
 instance = instance%instances;
 
 Frequency out = ms.GetOut<Frequency>(0);
 for (nat32 i=0;i<labels;i++) out[i] = data[instance*labels + i].val;
}

cstrconst Distribution::TypeString() const
{
 return "eos::inf::Distribution";
}
   
//------------------------------------------------------------------------------
JointDistribution::JointDistribution(nat32 i,nat32 l)
:links(l),instances(i),labels(new nat32[l]),stride(new nat32[l+1]),data(null<real32*>())
{}

JointDistribution::~JointDistribution()
{
 delete[] labels;
 delete[] stride;
 delete[] data;
}

void JointDistribution::SetLabels(nat32 * labSize)
{
 for (nat32 i=0;i<links;i++) labels[i] = labSize[i];

 stride[0] = 1;
 for (nat32 i=1;i<=links;i++) stride[i] = stride[i-1]*labels[i-1];
 delete[] data;
 data = new real32[stride[links]*instances];
}

void JointDistribution::SetLabels(const math::Vector<nat32> & labSize)
{
 for (nat32 i=0;i<links;i++) labels[i] = labSize[i];
 
 stride[0] = 1;
 for (nat32 i=1;i<=links;i++) stride[i] = stride[i-1]*labels[i-1];
 delete[] data;
 data = new real32[stride[links]*instances]; 
}

real32 & JointDistribution::Get(nat32 * labInd,nat32 instance)
{
 nat32 offset = 0;
 for (nat32 i=0;i<links;i++) offset += stride[i]*labInd[i];
 offset += stride[links]*instance;
 return data[offset];
}

real32 & JointDistribution::Get(const math::Vector<nat32> & labInd,nat32 instance)
{
 nat32 offset = 0;
 for (nat32 i=0;i<links;i++) offset += stride[i]*labInd[i];
 offset += stride[links]*instance;
 return data[offset];
}

real32 JointDistribution::Evaluate(nat32 * labs,nat32 instance)
{
 return 1.0;
}

void JointDistribution::Build()
{
 nat32 * ind = new nat32[links];
 for (nat32 i=0;i<links;i++) ind[i] = 0;
 while (true)
 {
  for (nat32 i=0;i<instances;i++) Get(ind,i) = Evaluate(ind,i);
  
  ++ind[0];
  if (ind[0]>=labels[0])
  {
   ind[0] = 0;
   nat32 i;
   for (i=1;i<links;i++)
   {
    ++ind[i];
    if (ind[i]<labels[i]) break;
    ind[i] = 0;
   }
   if (i==links) break;
  }
 }
 delete[] ind;
}

nat32 JointDistribution::Links() const
{
 return links;
}

void JointDistribution::LinkType(nat32 ind,MessageClass & out) const
{
 Frequency::MakeClass(labels[ind],out);
}

void JointDistribution::SendOneSP(nat32 instance,const MessageSet & ms,nat32 mtc)
{
 instance  = instance%instances;
 real32 * lData = data + instance*stride[links];

 // Get and iterate over each output frequency member...
  Frequency out = ms.GetOut<Frequency>(mtc);
  for (nat32 i=0;i<out.Labels();i++)
  {
   // Iterate over every possible variable combination, keeping our output
   // message fixed, we do this by recursion as its neat and reduces the
   // amount of calculation required...
    out[i] = Marginalise(0,mtc,i,ms,1.0,lData);
  }
 
 // Normalise, to avoid explosions...
  out.Norm();
}

void JointDistribution::SendAllButOneSP(nat32 instance,const MessageSet & ms,nat32 mti)
{
 for (nat32 i=0;i<links;i++)
 {
  if (i!=mti) SendOneSP(instance,ms,i);
 }
}

void JointDistribution::SendAllSP(nat32 instance,const MessageSet & ms)
{
 for (nat32 i=0;i<links;i++) SendOneSP(instance,ms,i);
}

void JointDistribution::ToMS()
{
 nat32 * ind = new nat32[links];
 for (nat32 i=0;i<links;i++) ind[i] = 0;
 while (true)
 {
  for (nat32 i=0;i<instances;i++)
  {
   real32 & targ = Get(ind,i);
   if (math::Equal(targ,real32(0.0))) targ = 10.0;
                                 else targ = math::Min(-math::Ln(targ),real32(10.0));
  }
  
  ++ind[0];
  if (ind[0]>=labels[0])
  {
   ind[0] = 0;
   nat32 i;
   for (i=1;i<links;i++)
   {
    ++ind[i];
    if (ind[i]<labels[i]) break;
    ind[i] = 0;
   }
   if (i==links) break;
  }
 }
 delete[] ind;
}

void JointDistribution::SendOneMS(nat32 instance,const MessageSet & ms,nat32 mtc)
{
 instance  = instance%instances;
 real32 * lData = data + instance*stride[links];

 // Get and iterate over each output frequency member...
  Frequency out = ms.GetOut<Frequency>(mtc);
  for (nat32 i=0;i<out.Labels();i++)
  {
   // Iterate over every possible variable combination, keeping our output
   // message fixed, we do this by recursion as its neat and reduces the
   // amount of calculation required...
    out[i] = MarginaliseMS(0,mtc,i,ms,0.0,lData);
  }
 
 // Zero mean, to avoid explosions...
  //out.ZeroMean(); best to not do this it seems, base don experiance.
}

void JointDistribution::SendAllButOneMS(nat32 instance,const MessageSet & ms,nat32 mti)
{
 for (nat32 i=0;i<links;i++)
 {
  if (i!=mti) SendOneMS(instance,ms,i);
 }
}

void JointDistribution::SendAllMS(nat32 instance,const MessageSet & ms)
{
 for (nat32 i=0;i<links;i++) SendOneMS(instance,ms,i);
}

cstrconst JointDistribution::TypeString() const
{
 return "eos::inf::JointDistribution";
}

real32 JointDistribution::Marginalise(nat32 link,nat32 skipLink,nat32 skipValue,const MessageSet & ms,real32 mmsd,real32 * targ)
{
 if (link==skipLink) return Marginalise(link+1,skipLink,skipValue,ms,mmsd,targ + stride[link]*skipValue);
 if (link==links) return targ[0] * mmsd;
  
 real32 ret = 0.0; 
  // Sum up recursion for each label at this level...
   Frequency in = ms.GetIn<Frequency>(link);
   for (nat32 i=0;i<labels[link];i++)
   {
    ret += Marginalise(link+1,skipLink,skipValue,ms,mmsd*in[i],targ + stride[link]*i);
   }
  
 return ret;
}

real32 JointDistribution::MarginaliseMS(nat32 link,nat32 skipLink,nat32 skipValue,const MessageSet & ms,real32 mmsd,real32 * targ)
{
 if (link==skipLink) return MarginaliseMS(link+1,skipLink,skipValue,ms,mmsd,targ + stride[link]*skipValue);
 if (link==links) return targ[0] + mmsd;
  
 // Recurse for each label at this level, and return the minimum...
  Frequency in = ms.GetIn<Frequency>(link);
  real32 ret = MarginaliseMS(link+1,skipLink,skipValue,ms,mmsd + in[0],targ);
  for (nat32 i=1;i<labels[link];i++)
  {
   ret = math::Min(MarginaliseMS(link+1,skipLink,skipValue,ms,mmsd + in[i],targ + stride[link]*i),ret);
  }
  
 return ret;
}

//------------------------------------------------------------------------------
DualDistribution::DualDistribution(nat32 inst,nat32 l0,nat32 l1, bit mss)
:instances(inst),labels0(l0),labels1(l1),ms(mss),data(new real32[instances*labels0*labels1])
{}

DualDistribution::DualDistribution()
{
 delete[] data;
}

real32 & DualDistribution::Val(nat32 instance,nat32 lab0,nat32 lab1)
{
 return data[labels0*(labels1*instance + lab1) + lab0];
}

nat32 DualDistribution::Links() const
{
 return 2;
}

void DualDistribution::LinkType(nat32 ind,MessageClass & out) const
{
 if (ind==0) Frequency::MakeClass(labels0,out);
        else Frequency::MakeClass(labels1,out);
}

void DualDistribution::ToSP()
{
 // Convert if need be...
  if (ms)
  {
   for (nat32 i=0;i<instances*labels0*labels1;i++)
   {
    data[i] = math::Exp(-data[i]);
   }
   ms = false;
  }

 // Normalise...
  for (nat32 i=0;i<instances;i++)
  {
   real32 * targ = data + i*labels0*labels1;

   real32 sum = 0.0;
   for (nat32 j=0;j<labels0*labels1;j++) sum += targ[j];

   sum = 1.0/sum;
   for (nat32 j=0;j<labels0*labels1;j++) targ[j] *= sum;
  }
}

void DualDistribution::SendOneSP(nat32 instance,const MessageSet & ms,nat32 mtc)
{
 real32 * targ = data + (instance&instances)*labels0*labels1;
 if (mtc==0)
 {
  Frequency in = ms.GetIn<Frequency>(1);
  Frequency out = ms.GetOut<Frequency>(0);
  
  for (nat32 io=0;io<labels0;io++)
  {
   out[io] = 0.0;
   for (nat32 ii=0;ii<labels1;ii++)
   {
    out[io] += in[ii] * targ[ii*labels0 + io];
   }
  }
  
  out.Norm();
 }
 else
 {
  Frequency in = ms.GetIn<Frequency>(0);
  Frequency out = ms.GetOut<Frequency>(1);
  
  for (nat32 io=0;io<labels1;io++)
  {
   out[io] = 0.0;
   for (nat32 ii=0;ii<labels0;ii++)
   {
    out[io] += in[ii] * targ[io*labels0 + ii];
   }
  }
  
  out.Norm();
 }
}

void DualDistribution::SendAllButOneSP(nat32 instance,const MessageSet & ms,nat32 mti)
{
 SendOneSP(instance,ms,(mti+1)%2);
}

void DualDistribution::SendAllSP(nat32 instance,const MessageSet & ms)
{
 SendOneSP(instance,ms,0);
 SendOneSP(instance,ms,1);
}

void DualDistribution::ToMS()
{
 // Convert if need be...
  if (!ms)
  {
   for (nat32 i=0;i<instances*labels0*labels1;i++)
   {
    if (math::IsZero(data[i])) data[i] = 0.0;
                          else data[i] = -math::Ln(data[i]);
   }
   ms = true;
  }

 // Normalise...
  for (nat32 i=0;i<instances;i++)
  {
   real32 * targ = data + i*labels0*labels1;

   real32 min = targ[0];
   for (nat32 j=1;j<labels0*labels1;j++) min = math::Min(min,targ[j]);

   for (nat32 j=0;j<labels0*labels1;j++) targ[j] -= min;
  }
}

void DualDistribution::SendOneMS(nat32 instance,const MessageSet & ms,nat32 mtc)
{
 real32 * targ = data + (instance&instances)*labels0*labels1;
 if (mtc==0)
 {
  Frequency in = ms.GetIn<Frequency>(1);
  Frequency out = ms.GetOut<Frequency>(0);
  
  for (nat32 io=0;io<labels0;io++)
  {
   out[io] = in[0] + targ[io];
   for (nat32 ii=1;ii<labels1;ii++)
   {
    out[io] = math::Min(out[io],in[ii] + targ[ii*labels0 + io]);
   }
  }
  
  out.Drop(); 
 }
 else
 {
  Frequency in = ms.GetIn<Frequency>(0);
  Frequency out = ms.GetOut<Frequency>(1);
  
  for (nat32 io=0;io<labels1;io++)
  {
   out[io] = in[0] + targ[io*labels0];
   for (nat32 ii=1;ii<labels0;ii++)
   {
    out[io] = math::Min(out[io],in[ii] + targ[io*labels0 + ii]);
   }
  }
  
  out.Drop();
 }
}

void DualDistribution::SendAllButOneMS(nat32 instance,const MessageSet & ms,nat32 mti)
{
 SendOneMS(instance,ms,(mti+1)%2);
}

void DualDistribution::SendAllMS(nat32 instance,const MessageSet & ms)
{
 // Get all the inputs and outputs...  
  Frequency in[2];
  Frequency out[2];
  for (nat32 i=0;i<2;i++)
  {
   in[i]  = ms.GetIn<Frequency>(i);
   out[i] = ms.GetOut<Frequency>(i);
  }
  
  for (nat32 i=0;i<labels0;i++) out[0][i] = math::Infinity<real32>();
  for (nat32 i=0;i<labels1;i++) out[1][i] = math::Infinity<real32>();


 // Iterate the joint distribution once rather than twice, that being the 
 // optimisation applied here...
  real32 * targ = data + (instance&instances)*labels0*labels1;
  for (nat32 l1=0;l1<labels1;l1++)
  {
   for (nat32 l0=0;l0<labels0;l0++)
   {
    out[0][l0] = math::Min(out[0][l0],*targ + in[1][l1]);
    out[1][l1] = math::Min(out[1][l1],*targ + in[0][l0]);
    
    ++targ;
   }
  }


 // Drop 'em...
  for (nat32 i=0;i<2;i++) out[i].Drop();
}

cstrconst DualDistribution::TypeString() const
{
 return "eos::inf::DualDistribution";
}

//------------------------------------------------------------------------------
EqualPotts::EqualPotts(nat32 ii,nat32 l)
:instances(ii),labels(l)
{
 inst = new real32[instances];
 for (nat32 i=0;i<instances;i++)
 {
  inst[i] = 1.0;
 }
}

EqualPotts::~EqualPotts()
{
 delete[] inst;
}

void EqualPotts::Set(nat32 instance,real32 equal)
{
 inst[instance] = equal;
}

nat32 EqualPotts::Links() const
{
 return 2;
}

void EqualPotts::LinkType(nat32 ind,MessageClass & out) const
{
 Frequency::MakeClass(labels,out);
}

void EqualPotts::SendOneSP(nat32 instance,const MessageSet & ms,nat32 mtc)
{
 real32 targ = inst[instance%instances];
 Frequency in = ms.GetIn<Frequency>((mtc+1)%2);
 Frequency out = ms.GetOut<Frequency>(mtc);
 
 real32 sum = 0.0;
 for (nat32 i=0;i<labels;i++) sum += in[i];
 
 targ -= 1.0;
 for (nat32 i=0;i<labels;i++) out[i] = sum + targ*in[i];
 
 out.Norm();
}

void EqualPotts::SendAllButOneSP(nat32 instance,const MessageSet & ms,nat32 mti)
{
 SendOneSP(instance,ms,(mti+1)%2);
}

void EqualPotts::SendAllSP(nat32 instance,const MessageSet & ms)
{
 SendOneSP(instance,ms,0);
 SendOneSP(instance,ms,1);
}

void EqualPotts::ToMS()
{
 for (nat32 i=0;i<instances;i++)
 {
  inst[i] = math::Ln(inst[i]);
 }
}

void EqualPotts::SendOneMS(nat32 instance,const MessageSet & ms,nat32 mtc)
{
 Frequency in = ms.GetIn<Frequency>((mtc+1)%2);
 Frequency out = ms.GetOut<Frequency>(mtc);

 real32 min = in[0];
 for (nat32 i=1;i<labels;i++) min = math::Min(min,in[i]);
 min += inst[instance%instances];
 
 for (nat32 i=0;i<labels;i++)
 {
  out[i] = math::Min(in[i],min);
 }
 
 out.Drop();
}

void EqualPotts::SendAllButOneMS(nat32 instance,const MessageSet & ms,nat32 mti)
{
 SendOneMS(instance,ms,(mti+1)%2);
}

void EqualPotts::SendAllMS(nat32 instance,const MessageSet & ms)
{
 SendOneMS(instance,ms,0);
 SendOneMS(instance,ms,1);
}

cstrconst EqualPotts::TypeString() const
{
 return "eos::inf::EqualPotts";
}

//------------------------------------------------------------------------------
EqualGaussian::EqualGaussian(nat32 ins,nat32 l)
:instances(ins),labels(l)
{
 inst = new Inst[instances];
 for (nat32 i=0;i<instances;i++)
 {	
  inst[i].doCorruption = false;
  inst[i].corruption = 0.0;
  inst[i].range = 0;
  inst[i].gauss = null<real32*>(); 
 }
}

EqualGaussian::~EqualGaussian()
{
 for (nat32 i=0;i<instances;i++) delete[] inst[i].gauss;
 delete[] inst;	
}

void EqualGaussian::Set(nat32 instance,real32 c,real32 sd,real32 r)
{
 Inst & targ = inst[instance%instances];
 targ.corruption = c;
 targ.doCorruption = !math::Equal(targ.corruption,real32(0.0));
 targ.sd = sd;

 // First calculate the range...
  if (targ.doCorruption)
  {
   real32 inSqrt = -2.0*math::Sqr(sd)*math::Ln(targ.corruption);
   if (inSqrt>0.0)
   {
    real32 inter = math::Sqrt(inSqrt);
    targ.range = math::Min(nat32(math::RoundDown(sd*r)),nat32(math::RoundDown(inter)));
   }
   else
   {
    targ.range = 0; // Problem - the gaussian is allways less than the corruption - its all gone to pot!
   }
  }
  else
  {
   targ.range = nat32(math::RoundDown(sd*r));
  }
  
 // Now fill in all the values...
  delete[] targ.gauss;
  targ.gauss = new real32[1+targ.range];
 
  for (nat32 i=0;i<=targ.range;i++)
  {
   targ.gauss[i] = math::Max(math::Gaussian<real64>(sd,i) - targ.corruption,0.0);
  }
}

nat32 EqualGaussian::Links() const
{
 return 2;	
}

void EqualGaussian::LinkType(nat32,MessageClass & out) const
{
 Frequency::MakeClass(labels,out);	
}

void EqualGaussian::SendOneSP(nat32 instance,const MessageSet & ms,nat32 mtc)
{
 Inst & targ = inst[instance%instances];
 Frequency in = ms.GetIn<Frequency>((mtc+1)%2);
 Frequency out = ms.GetOut<Frequency>(mtc);
 
 if (targ.doCorruption)
 {
  real32 amount = 0.0;
  for (nat32 i=0;i<labels;i++) amount += in[i];
  amount *= targ.corruption;
  for (nat32 i=0;i<labels;i++) out[i] = amount;
 }
 else
 {
  for (nat32 i=0;i<labels;i++) out[i] = 0.0;
 }
 
 for (int32 i=0;i<int32(labels);i++)
 {
  int32 start = (i<=int32(targ.range))?0:(i-targ.range);
  int32 end = math::Min(i+targ.range,labels-1);
  for (int32 j=start;j<=end;j++)
  {
   out[i] += in[j]*targ.gauss[math::Abs(i-j)];
  }
 }
 
 out.Norm();
}

void EqualGaussian::SendAllButOneSP(nat32 instance,const MessageSet & ms,nat32 mti)
{
 SendOneSP(instance,ms,(mti+1)%2);
}

void EqualGaussian::SendAllSP(nat32 instance,const MessageSet & ms)
{
 SendOneSP(instance,ms,0);
 SendOneSP(instance,ms,1);
}

void EqualGaussian::ToMS()
{
 // Set the corruption to -ln of the corruption and sd to be 1/(2*sd^2)...
  for (nat32 i=0;i<instances;i++)
  {
   if (math::Equal(inst[i].corruption,real32(0.0))) inst[i].corruption = 10.0;
                                               else inst[i].corruption = -math::Ln(inst[i].corruption);
   inst[i].sd = 1.0/(2.0*math::Sqr(inst[i].sd));
  }
}

void EqualGaussian::SendOneMS(nat32 instance,const MessageSet & ms,nat32 mtc)
{
 Inst & targ = inst[instance%instances];
 Frequency in = ms.GetIn<Frequency>((mtc+1)%2);
 Frequency out = ms.GetOut<Frequency>(mtc);  

 // Find the minimum plus the corruption, this being the maximum 
 // cost of any given label...
  real32 minVal = in[0];
  for (nat32 i=1;i<labels;i++) minVal = math::Min(minVal,in[i]);
  minVal += targ.corruption;


 // Now do two passes, a left to right and a right to left, where 
 // we propagate the lowest parabola at each point...
  // Left to right...
   nat32 offset = 0;
   bit cp = false;
   for (nat32 i=0;i<labels;i++)
   {
    if (cp)
    {
     // Find which is the minimum - the in[i] value, the parabola or
     // minVal, and act accordingly...
      real32 para = math::Sqr(i-offset)*targ.sd + in[offset];
      if ((para<in[i])&&(para<minVal)) out[i] = para;     
      else
      {
       if (minVal<in[i])
       {
        out[i] = minVal;
        cp = false;
       }
       else
       {
        out[i] = in[i];
        offset = i;
       }
      }
    }
    else
    {
     // We are not considering a previous parabola - just check if the minimum
     // of the current point is less than the minVal and act accordingly...
      if (minVal<in[i]) out[i] = minVal;
      else
      {
       out[i] = in[i];
       offset = i;
       cp = true;
      }
    }
   }
  
  // Right to left... 
   offset = labels-1;
   for (int32 i=labels-1;i>=0;i--)
   {
    real32 para = math::Sqr(offset-i)*targ.sd + in[offset];
    if (para<=in[i])
    {
     if (para<out[i]) out[i] = para;
    }
    else
    {
     if (in[i]<out[i]) out[i] = in[i];
     offset = i;
    }
   }
   
  out.Drop();
}

void EqualGaussian::SendAllButOneMS(nat32 instance,const MessageSet & ms,nat32 mti)
{
 SendOneMS(instance,ms,(mti+1)%2);
}

void EqualGaussian::SendAllMS(nat32 instance,const MessageSet & ms)
{
 SendOneMS(instance,ms,0);
 SendOneMS(instance,ms,1);
}

cstrconst EqualGaussian::TypeString() const
{
 return "eos::inf::EqualGaussian";	
}

//------------------------------------------------------------------------------
EqualLaplace::EqualLaplace(nat32 ins,nat32 l)
:instances(ins),labels(l)
{
 inst = new Inst[instances];
 for (nat32 i=0;i<instances;i++)
 {
  inst[i].ms = false;
  inst[i].doCorruption = false;
  inst[i].corruption = 0.0;
  inst[i].range = 0;
  inst[i].laplace = null<real32*>();
 }
}

EqualLaplace::~EqualLaplace()
{
 for (nat32 i=0;i<instances;i++) delete[] inst[i].laplace;
 delete[] inst;
}

void EqualLaplace::Set(nat32 instance,real32 c,real32 sd,real32 r)
{
 Inst & targ = inst[instance%instances];
 targ.ms = false;
 targ.corruption = c;
 targ.doCorruption = !math::Equal(targ.corruption,real32(0.0));
 targ.sd = sd;
 targ.rMult = r;
}

void EqualLaplace::SetMS(nat32 instance,real32 mult,real32 max)
{
 Inst & targ = inst[instance%instances];
 targ.ms = true;
 targ.corruption = max;
 targ.doCorruption = true;
 targ.sd = mult;
 targ.rMult = 2.0;
}

nat32 EqualLaplace::Links() const
{
 return 2;	
}

void EqualLaplace::LinkType(nat32,MessageClass & out) const
{
 Frequency::MakeClass(labels,out);
}

void EqualLaplace::ToSP()
{
 for (nat32 i=0;i<instances;i++)
 {
  if (inst[i].ms==true)
  {
   inst[i].ms = false;
   inst[i].corruption = math::Max<real32>(math::Exp(-inst[i].corruption),0.0);
   inst[i].sd = 1.0/inst[i].sd;
  }
  
  // First calculate the range...
   if (inst[i].doCorruption)
   {
    real32 val = -math::Ln(inst[i].corruption)*inst[i].sd;
    inst[i].range = math::Min(nat32(math::RoundDown(inst[i].sd*inst[i].rMult)),nat32(math::RoundDown(val)));
   }
   else
   {
    inst[i].range = nat32(math::RoundDown(inst[i].sd*inst[i].rMult));
   }
  
  // Now fill in all the values...
   delete[] inst[i].laplace;
   inst[i].laplace = new real32[1+inst[i].range];
 
   for (nat32 j=0;j<=inst[i].range;j++)
   {
    inst[i].laplace[j] = math::Max(math::Laplace<real64>(inst[i].sd,j) - inst[i].corruption,0.0);
   }  
 }
}

void EqualLaplace::SendOneSP(nat32 instance,const MessageSet & ms,nat32 mtc)
{
 Inst & targ = inst[instance%instances];
 Frequency in = ms.GetIn<Frequency>((mtc+1)%2);
 Frequency out = ms.GetOut<Frequency>(mtc);
 
 if (targ.doCorruption)
 {
  real32 amount = 0.0;
  for (nat32 i=0;i<labels;i++) amount += in[i];
  amount *= targ.corruption;
  for (nat32 i=0;i<labels;i++) out[i] = amount;
 }
 else
 {
  for (nat32 i=0;i<labels;i++) out[i] = 0.0;
 }
 
 for (int32 i=0;i<int32(labels);i++)
 {
  int32 start = (i<=int32(targ.range))?0:(i-targ.range);
  int32 end = math::Min(i+targ.range,labels-1);
  for (int32 j=start;j<=end;j++)
  {
   out[i] += in[j]*targ.laplace[math::Abs(i-j)];
  }
 }
 
 out.Norm();
}

void EqualLaplace::SendAllButOneSP(nat32 instance,const MessageSet & ms,nat32 mti)
{
 SendOneSP(instance,ms,(mti+1)%2);
}

void EqualLaplace::SendAllSP(nat32 instance,const MessageSet & ms)
{
 SendOneSP(instance,ms,0);
 SendOneSP(instance,ms,1);
}

void EqualLaplace::ToMS()
{
 // Set the corruption to -ln of the corruption and sd to be 1/sd...
  for (nat32 i=0;i<instances;i++)
  {
   if (inst[i].ms==false)
   {
    inst[i].ms = true;
    if (math::Equal(inst[i].corruption,real32(0.0))) inst[i].corruption = 100.0;
                                                else inst[i].corruption = math::Min<real32>(-math::Ln(inst[i].corruption),100.0);
    inst[i].sd = 1.0/inst[i].sd;
   }   
  }
}

void EqualLaplace::SendOneMS(nat32 instance,const MessageSet & ms,nat32 mtc)
{
 Inst & targ = inst[instance%instances];
 Frequency in = ms.GetIn<Frequency>((mtc+1)%2);
 Frequency out = ms.GetOut<Frequency>(mtc);  

 // Find the minimum plus the corruption, this being the maximum 
 // cost of any given label...
  real32 minVal = in[0];
  for (nat32 i=1;i<labels;i++) minVal = math::Min(minVal,in[i]);
  minVal += targ.corruption;


 // Now do two passes, a left to right and a right to left, where 
 // we propagate the lowest value at each point...
  // Left to right...
   out[0] = in[0];
   for (nat32 i=1;i<labels;i++) out[i] = math::Min(in[i],out[i-1]+targ.sd);

  // Right to left...
   for (int32 i=labels-2;i>=0;i--) out[i] = math::Min(out[i],out[i+1]+targ.sd);
   
  // Truncate...
   for (nat32 i=0;i<labels;i++) out[i] = math::Min(out[i],minVal);
   
 //LogDebug("[lap] msg " << mtc << LogDiv() << in << LogDiv() << out);
   
 out.Drop(); 
}

void EqualLaplace::SendAllButOneMS(nat32 instance,const MessageSet & ms,nat32 mti)
{
 SendOneMS(instance,ms,(mti+1)%2);
}

void EqualLaplace::SendAllMS(nat32 instance,const MessageSet & ms)
{
 SendOneMS(instance,ms,0);
 SendOneMS(instance,ms,1);
}

cstrconst EqualLaplace::TypeString() const
{
 return "eos::inf::EqualLaplace";	
}

//------------------------------------------------------------------------------
EqualVonMises::EqualVonMises(nat32 inst,nat32 lab)
:instances(inst),labels(lab),diffMult((2.0*math::pi)/real32(lab))
{
 para = new Para[instances];
}

EqualVonMises::~EqualVonMises()
{
 delete[] para;
}

void EqualVonMises::Set(nat32 instance,real32 kk,real32 cutMult)
{
 para[instance].k = kk;
 para[instance].minProb = math::Exp(kk)*cutMult;
 if (math::IsZero(cutMult)) para[instance].maxCost = 100.0;
                       else para[instance].maxCost = -kk - math::Ln(cutMult);
}

nat32 EqualVonMises::Links() const
{
 return 2;
}

void EqualVonMises::LinkType(nat32 ind,MessageClass & out) const
{
 Frequency::MakeClass(labels,out);
}

void EqualVonMises::ToSP()
{}

void EqualVonMises::SendOneSP(nat32 instance,const MessageSet & ms,nat32 mtc)
{
 // Get the relevant messages...
  Para & p = para[instance%instances];
  Frequency in = ms.GetIn<Frequency>((mtc+1)%2);
  Frequency out = ms.GetOut<Frequency>(mtc);


 // Fill in the output from the input...
  for (int32 io=0;io<int32(labels);io++)
  {
   out[io] = 0.0;
   for (int32 ii=0;ii<int32(labels);ii++)
   {
    real32 pr = math::Max(p.minProb,math::Exp(p.k*math::Cos(diffMult*real32(io-ii))));
    out[io] += in[ii] * pr;
   }
  }


 // Normalise the output...
  out.Norm();
}

void EqualVonMises::SendAllButOneSP(nat32 instance,const MessageSet & ms,nat32 mti)
{
 SendOneSP(instance,ms,(mti+1)%2);
}

void EqualVonMises::SendAllSP(nat32 instance,const MessageSet & ms)
{
 SendOneSP(instance,ms,0);
 SendOneSP(instance,ms,1);
}

void EqualVonMises::ToMS()
{}

void EqualVonMises::SendOneMS(nat32 instance,const MessageSet & ms,nat32 mtc)
{
 // Get the relevant messages...
  Para & p = para[instance%instances];
  Frequency in = ms.GetIn<Frequency>((mtc+1)%2);
  Frequency out = ms.GetOut<Frequency>(mtc);


 // Calculate the output...
  // Find the minimum input, factor in the maxCost and set the entire output to 
  // the maximum value that it can take...
   real32 maxVal = in[0];
   for (nat32 i=1;i<labels;i++) maxVal = math::Min(maxVal,in[i]);
   maxVal += p.maxCost;
   for (nat32 i=0;i<labels;i++) out[i] = maxVal;

  // Do a forward pass, propagating the distribution, have to loop arround due 
  // to the modulus nature of the distribution...
   // First loop, allways completed...
    bit active = false;
    int32 pos = 0;
    for (int32 i=0;i<int32(labels);i++)
    {
     real32 posCost = in[i] - p.k;
     if (active)
     {
      real32 activeCost = in[pos] - p.k*math::Cos(diffMult*real32(pos-i));
      if (activeCost<posCost)
      {
       if (activeCost<out[i]) out[i] = activeCost;
                         else active = false;
      }
      else
      {
       if (posCost<out[i])
       {
        pos = i;
        out[i] = posCost;
       }
       else active = false;
      }
     }
     else
     {
      if (posCost<out[i])
      {
       active = true;
       pos = i;
       out[i] = posCost;
      }
     }
    }
   
   // Second loop, for the modulus overlap, ushally breaks early...
    if (active)
    {
     for (int32 i=0;i<pos;i++)
     {
      real32 activeCost = in[pos] - p.k*math::Cos(diffMult*real32(pos-i));
      if (activeCost<out[i]) out[i] = activeCost;
                        else break;
     }
    }


  // Backward pass, in compliment to the forward pass, again done in modulus,
  // this covers all bases for the distribution...
   // First loop, allways completed...
    pos = labels-1;
    for (int32 i=labels-1;i>=0;i--)
    {
     real32 posCost = in[i] - p.k;
     real32 activeCost = in[pos] - p.k*math::Cos(diffMult*real32(pos-i));
     if (posCost<=activeCost)
     {
      pos = i;
      out[i] = math::Min(out[i],posCost);
     }
     else
     {
      out[i] = math::Min(out[i],activeCost);
     }
    }
    
   // Second loop, for the modulus overlap, ushally breaks early...
    for (int32 i=labels-1;i>pos;i--)
    {
     real32 activeCost = in[pos] - p.k*math::Cos(diffMult*real32(pos-i));
     if (activeCost<out[i]) out[i] = activeCost;
                       else break;
    }


 // Drop the output...
  out.Drop();
}

void EqualVonMises::SendAllButOneMS(nat32 instance,const MessageSet & ms,nat32 mti)
{
 SendOneMS(instance,ms,(mti+1)%2);
}

void EqualVonMises::SendAllMS(nat32 instance,const MessageSet & ms)
{
 SendOneMS(instance,ms,0);
 SendOneMS(instance,ms,1);
}

cstrconst EqualVonMises::TypeString() const
{
 return "eos::inf::EqualVonMises";
}

//------------------------------------------------------------------------------
DifferencePotts::DifferencePotts(nat32 l,real32 a)
:labels(l),lambda(a)
{}

DifferencePotts::~DifferencePotts()
{}

nat32 DifferencePotts::Links() const
{
 return 3;
}

void DifferencePotts::LinkType(nat32 ind,MessageClass & out) const
{
 if (ind==0) Frequency::MakeClass(labels*2-1,out);
        else Frequency::MakeClass(labels,out);
}

void DifferencePotts::SendOneSP(nat32,const MessageSet & ms,nat32 mtc)
{
 switch(mtc)
 {
  case 0:
  {
   Frequency a = ms.GetOut<Frequency>(0);
   Frequency b = ms.GetIn<Frequency>(1);
   Frequency c = ms.GetIn<Frequency>(2);

   real32 base = 0.0;
   for (int32 bi=0;bi<labels;bi++)
   {
    for (int32 ci=0;ci<labels;ci++) base += b[bi]*c[ci];
   }
   base *= lambda;

   for (int32 ai=0;ai<labels*2-1;ai++)
   {
    a[ai] = 0.0;
    int32 biS = math::Max<int32>(0,ai-labels+1);
    int32 biE = math::Min<int32>(labels-1,ai);
    for (int32 bi=biS;bi<=biE;bi++)
    {
     a[ai] += b[bi] * c[bi-ai+labels-1];
    }
    //a[ai] = a[ai]*(1.0-lambda)/real32(biE-biS+1) + base;
    a[ai] = a[ai]*(1.0-lambda) + base;
   }
   
   a.Norm();
  }
  break;
  case 1:
  {
   Frequency a = ms.GetIn<Frequency>(0);
   Frequency b = ms.GetOut<Frequency>(1);
   Frequency c = ms.GetIn<Frequency>(2);
   
   real32 base = 0.0;
   for (int32 ai=0;ai<labels*2-1;ai++)
   {
    for (int32 ci=0;ci<labels;ci++) base += a[ai]*c[ci];
   }
   base *= lambda;
   
   for (int32 bi=0;bi<labels;bi++)
   {
    b[bi] = 0.0;
    for (int32 ci=0;ci<labels;ci++)
    {
     b[bi] += a[bi+labels-1-ci] * c[ci];
    }
    b[bi] = b[bi]*(1.0-lambda) + base;
   }
   
   b.Norm();
  }
  break;
  case 2:
  {
   Frequency a = ms.GetIn<Frequency>(0);
   Frequency b = ms.GetIn<Frequency>(1);
   Frequency c = ms.GetOut<Frequency>(2);
   
   real32 base = 0.0;
   for (int32 ai=0;ai<labels*2-1;ai++)
   {
    for (int32 bi=0;bi<labels;bi++) base += a[ai]*b[bi];
   }
   base *= lambda;
   
   for (int32 ci=0;ci<labels;ci++)
   {
    c[ci] = 0.0;
    for (int32 bi=0;bi<labels;bi++)
    {
     c[ci] += a[bi+labels-1-ci] * b[bi];
    }
    c[ci] = c[ci]*(1.0-lambda) + base;
   }
   
   c.Norm();
  }  
  break;
 }
}

void DifferencePotts::SendAllButOneSP(nat32 instance,const MessageSet & ms,nat32 mti)
{
 SendOneSP(instance,ms,(mti+1)%3);
 SendOneSP(instance,ms,(mti+2)%3);
}

void DifferencePotts::SendAllSP(nat32 instance,const MessageSet & ms)
{
 SendOneSP(instance,ms,0);
 SendOneSP(instance,ms,1);
 SendOneSP(instance,ms,2);  
}

void DifferencePotts::ToMS()
{
 if (math::Equal(lambda,real32(0.0))) lambda = 10.0;
                                 else lambda = math::Min(-math::Ln(lambda),real32(10.0));
}

void DifferencePotts::SendOneMS(nat32 instance,const MessageSet & ms,nat32 mtc)
{
 switch (mtc)
 {
  case 0:
  {
   Frequency a = ms.GetOut<Frequency>(0);
   Frequency b = ms.GetIn<Frequency>(1);
   Frequency c = ms.GetIn<Frequency>(2);
   
   real32 minB = b[0];
   for (int32 bi=1;bi<labels;bi++) minB = math::Min(minB,b[bi]);
   real32 minC = c[0];
   for (int32 ci=1;ci<labels;ci++) minC = math::Min(minC,c[ci]);
   real32 maxA = math::Min(minB + minC + lambda,real32(10.0));
   
   for (int32 ai=0;ai<labels*2-1;ai++)
   {
    a[ai] = maxA;
    int32 minB = math::Max<int32>(ai-labels+1,0);
    int32 maxB = math::Min<int32>(ai,labels-1);
    for (int32 bi=minB;bi<=maxB;bi++)
    {
     a[ai] = math::Min(a[ai],b[bi] + c[bi-ai+labels-1]);
    }
   }

   a.Drop();
  }
  break;
  case 1:
  {
   Frequency a = ms.GetIn<Frequency>(0);
   Frequency b = ms.GetOut<Frequency>(1);
   Frequency c = ms.GetIn<Frequency>(2);

   real32 minA = a[0];
   for (int32 ai=1;ai<labels*2-1;ai++) minA = math::Min(minA,a[ai]);   
   real32 minC = c[0];
   for (int32 ci=1;ci<labels;ci++) minC = math::Min(minC,c[ci]);   
   real32 maxB = math::Min(minA + minC + lambda,real32(10.0));
   
   for (int32 bi=0;bi<labels;bi++)
   {
    b[bi] = maxB;
    for (int32 ci=0;ci<labels;ci++)
    {
     b[bi] = math::Min(b[bi],a[bi+labels-1-ci] + c[ci]);
    }
   }
   
   b.Drop();
  }
  break;
  case 2:
  {
   Frequency a = ms.GetIn<Frequency>(0);
   Frequency b = ms.GetIn<Frequency>(1);
   Frequency c = ms.GetOut<Frequency>(2);

   real32 minA = a[0];
   for (int32 ai=1;ai<labels*2-1;ai++) minA = math::Min(minA,a[ai]);  
   real32 minB = b[0];
   for (int32 bi=1;bi<labels;bi++) minB = math::Min(minB,b[bi]);
   real32 maxC = math::Min(minA + minB + lambda,real32(10.0));
   
   for (int32 ci=0;ci<labels;ci++)
   {
    c[ci] = maxC;
    for (int32 bi=0;bi<labels;bi++)
    {
     c[ci] = math::Min(c[ci],a[bi+labels-1-ci] + b[bi]);
    }
   }
   
   c.Drop();
  }
  break;
 }
}

void DifferencePotts::SendAllButOneMS(nat32 instance,const MessageSet & ms,nat32 mti)
{
 SendOneMS(instance,ms,(mti+1)%3);
 SendOneMS(instance,ms,(mti+2)%3);
}

void DifferencePotts::SendAllMS(nat32 instance,const MessageSet & ms)
{
 SendOneMS(instance,ms,0);
 SendOneMS(instance,ms,1);
 SendOneMS(instance,ms,2);
}

cstrconst DifferencePotts::TypeString() const
{
 return "eos::inf::DifferencePotts";
}

//------------------------------------------------------------------------------
GeneralPotts::GeneralPotts(nat32 inst,nat32 li,nat32 * la)
:instances(inst),links(li),exc(sizeof(Exc)+sizeof(nat32)*links)
{
 labels = new nat32[links];
 for (nat32 i=0;i<links;i++) labels[i] = la[i];
 
 cost = new real32[instances];
 for (nat32 i=0;i<instances;i++) cost[i] = 1.0;
 
 n = new Node[links];
}

GeneralPotts::~GeneralPotts()
{
 delete n;
 delete cost;
 delete labels;
}

void GeneralPotts::SetCost(nat32 inst,real32 c)
{
 cost[inst] = c;
}

void GeneralPotts::SetException(nat32 instance,real32 cost,nat32 * label)
{
 excBuf << instance << cost;
 for (nat32 i=0;i<links;i++) excBuf << label[i];
}

nat32 GeneralPotts::ExceptionCount() const
{
 nat32 excSize = sizeof(Exc) + sizeof(nat32)*links;
 return excBuf.Size()/excSize;
}

nat32 GeneralPotts::Links() const
{
 return links;
}
  
void GeneralPotts::LinkType(nat32 ind,MessageClass & out) const
{
 Frequency::MakeClass(labels[ind],out);
}

void GeneralPotts::SendOneSP(nat32 instance,const MessageSet & ms,nat32 mtc)
{
 log::Assert(false,"Use of generalised potts with sum-product");
}

void GeneralPotts::SendAllButOneSP(nat32 instance,const MessageSet & ms,nat32 mti)
{
 log::Assert(false,"Use of generalised potts with sum-product");
}   

void GeneralPotts::SendAllSP(nat32 instance,const MessageSet & ms)
{
 log::Assert(false,"Use of generalised potts with sum-product");
}   

void GeneralPotts::ToMS()
{
 // This converts the collected exceptions into a *sorted* array structure,
 // this being the optimal structure for iterating with an eye to cache 
 // coherance...
  // First move the data from excBuf to exc, emptying excBuf to save on 
  // storage space.
  {
   nat32 excSize = sizeof(Exc) + sizeof(nat32)*links;
   exc.Size(excBuf.Size()/excSize);
   data::Buffer::Cursor targ = excBuf.GetCursor();
   for (nat32 i=0;i<exc.Size();i++) targ.Read(&(exc[i]),excSize);
   excBuf.SetSize(0);
  }

  // Sort...
   exc.Sort(&SortFunc,&links);
  
  // Setup the indexing...
   // Setup the indexing array - iterate the instances...
    excInd.Size(instances);
    for (nat32 i=0;i<excInd.Size();i++)
    {
     excInd[i].start = 0;
     excInd[i].size = 0;
    }

   // Iterate the exceptions...
    nat32 nta = 0;
    for (nat32 i=0;i<exc.Size();i++)
    {
     if (exc[i].instance==nta)
     {
      excInd[nta].start = i;
      ++nta;
     }
     excInd[exc[i].instance].size += 1;
    }
}

void GeneralPotts::SendOneMS(nat32 instance,const MessageSet & ms,nat32 mtc)
{
 instance = instance%instances;

 // Find and sum the minimums of each input, also get the output frequency...
  real32 minSum = cost[instance];
  for (nat32 i=0;i<mtc;i++)
  {
   n[i].in = ms.GetIn<Frequency>(i);
   real32 min = n[i].in[0];
   for (nat32 j=1;j<labels[i];j++) min = math::Min(min,n[i].in[j]);
   minSum += min;
  }
  n[mtc].out = ms.GetOut<Frequency>(mtc);  for (nat32 i=mtc+1;i<links;i++)
  {
   n[i].in = ms.GetIn<Frequency>(i);
   real32 min = n[i].in[0];
   for (nat32 j=1;j<labels[i];j++) min = math::Min(min,n[i].in[j]);
   minSum += min;
  }
  
 // Set all outputs to be the maximum value allowed for any value...
  for (nat32 i=0;i<labels[mtc];i++) n[mtc].out[i] = minSum;
  
 // Now loop all the exceptions, factoring them in one by one...
  nat32 end = excInd[instance].start + excInd[instance].size;
  for (nat32 i=excInd[instance].start;i<end;i++)
  {
   real32 val = exc[i].cost;
   for (nat32 j=0;j<mtc;j++) val += n[j].in[exc[i].Label(j)];
   nat32 lab = exc[i].Label(mtc);
   for (nat32 j=mtc+1;j<links;j++) val += n[j].in[exc[i].Label(j)];
  
   n[mtc].out[lab] = math::Min(n[mtc].out[lab],val);
  }
 
 // Drop the output, and relax/return... 
  n[mtc].out.Drop();
}

void GeneralPotts::SendAllButOneMS(nat32 instance,const MessageSet & ms,nat32 mti)
{
 instance = instance%instances;
 
 // Get all the inputs and outputs, calculate the minimum of all the inputs and 
 // a sum of all the inputs including the base cost...
  real32 baseSum = cost[instance];
  for (nat32 i=0;i<links;i++)
  {
   n[i].in = ms.GetIn<Frequency>(i);
   if (i!=mti) n[i].out = ms.GetOut<Frequency>(i);

   n[i].min = n[i].in[0];
   for (nat32 j=1;j<labels[i];j++) n[i].min = math::Min(n[i].min,n[i].in[j]);
   baseSum += n[i].min;  
  }


 // Set all outputs to there relevant maximums...
  for (nat32 i=0;i<mti;i++)
  {
   real32 minSum = baseSum - n[i].min;
   for (nat32 j=0;j<labels[i];j++) n[i].out[j] = minSum;
  }
  for (nat32 i=mti+1;i<links;i++)
  {
   real32 minSum = baseSum - n[i].min;
   for (nat32 j=0;j<labels[i];j++) n[i].out[j] = minSum;
  }


 // Iterate and proccess all exceptions...
  nat32 end = excInd[instance].start + excInd[instance].size;
  for (nat32 i=excInd[instance].start;i<end;i++)
  {
   // Calculate the sum of all...
    real32 val = exc[i].cost;
    for (nat32 j=0;j<links;j++) val += n[j].in[exc[i].Label(j)];

   // Integrate in with each the self-subtraction from the sum of all via minimisation.
   // Now thats a convoluted sentence...
    for (nat32 j=0;j<mti;j++)
    {
     real32 & targ = n[j].out[exc[i].Label(j)];
     targ = math::Min(targ,val - n[j].in[exc[i].Label(j)]);
    }
    for (nat32 j=mti+1;j<links;j++)
    {
     real32 & targ = n[j].out[exc[i].Label(j)];
     targ = math::Min(targ,val - n[j].in[exc[i].Label(j)]);
    }
  }


 // Drop all outputs...
  for (nat32 i=0;i<mti;i++) n[i].out.Drop();
  for (nat32 i=mti+1;i<links;i++) n[i].out.Drop();
}

void GeneralPotts::SendAllMS(nat32 instance,const MessageSet & ms)
{
 instance = instance%instances;
 
 // Get all the inputs and outputs, calculate the minimum of all the inputs and 
 // a sum of all the inputs including the base cost...
  real32 baseSum = cost[instance];
  for (nat32 i=0;i<links;i++)
  {
   n[i].in = ms.GetIn<Frequency>(i);
   n[i].out = ms.GetOut<Frequency>(i);

   n[i].min = n[i].in[0];
   for (nat32 j=1;j<labels[i];j++) n[i].min = math::Min(n[i].min,n[i].in[j]);
   baseSum += n[i].min;  
  }


 // Set all outputs to there relevant maximums...
  for (nat32 i=0;i<links;i++)
  {
   real32 minSum = baseSum - n[i].min;
   for (nat32 j=0;j<labels[i];j++) n[i].out[j] = minSum;
  }


 // Iterate and proccess all exceptions...
  nat32 end = excInd[instance].start + excInd[instance].size;
  for (nat32 i=excInd[instance].start;i<end;i++)
  {
   // Calculate the sum of all...
    real32 val = exc[i].cost;
    for (nat32 j=0;j<links;j++) val += n[j].in[exc[i].Label(j)];

   // Integrate in with each the self-subtraction from the sum of all via minimisation.
   // Now thats a convoluted sentence...
    for (nat32 j=0;j<links;j++)
    {
     real32 & targ = n[j].out[exc[i].Label(j)];
     targ = math::Min(targ,val - n[j].in[exc[i].Label(j)]);
    }
  }


 // Drop all outputs...
  for (nat32 i=0;i<links;i++) n[i].out.Drop();  
}

//------------------------------------------------------------------------------
 };
};
