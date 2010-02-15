//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines
#include "eos/data/randoms.h"

#include "eos/time/times.h"
#include "eos/math/functions.h"
 
namespace eos
{
 namespace data
 {
//------------------------------------------------------------------------------
// Some numbers we require...
// (Taken from weiss.)
 static const nat32 A = 48271;
 static const nat32 M = 2147483647;
 static const nat32 Q = M/A;
 static const nat32 R = M%A;

//------------------------------------------------------------------------------
Random::Random(bit useSeed,int32 seed)
:gaussValid(false)
{
 if (useSeed) val = seed;
 else
 {
  // We have to somehow find enough bits to make some half arsed effort at a
  // good seed - this is impossible, as computers just don't provide such things...
  // (At least without propriatry stuff)
  // We do however have a good go at it - we get time from several different sources
  // and xor it together...
   val = time::Time();
   val ^= time::MilliTime();
   union
   {
    real64 rp;
    int32 ip[2];	   
   };
   rp = time::UltraTime();
   val ^= ip[0];
   val ^= ip[0];
 }

 if (val<0) val += M;
 if (val==0) val = 1;
}

Random::~Random()
{}

void Random::SetSeed(int32 seed)
{
 val = seed;
 if (val<0) val += M;
 if (val==0) val = 1;

 gaussValid = false;
}

int32 Random::GetSeed()
{
 return val;	
}

byte Random::Byte()
{
 return byte(Next()%256);
}

int32 Random::Int(int32 min,int32 max)
{
 return int32(Next()%(max-min+1))+min;
}

real64 Random::Real(real64 min,real64 max)
{
 return NormalInc()*(max-min)+min;
}

bit Random::Bool()
{
 if (Next()%2) return true;
          else return false;
}

real64 Random::Normal()
{
 return real64(Next())/real64(M);
}

real64 Random::NormalInc()
{
 return real64(Next())/real64(M-1);
}

real64 Random::Signed()
{
 return (real64(Next()+1)/(real64(M+1)*0.5)) - 1.0;
}

real64 Random::Gaussian(real64 sd)
{
 if (gaussValid==true)
 {
  gaussValid = false;
  return gaussVal * sd;
 }

 // Code copied from http://www.taygeta.com/random/gaussian.html
  real64 x1, x2, w;
 
  do 
  {
   x1 = 2.0 * NormalInc() - 1.0;
   x2 = 2.0 * NormalInc() - 1.0;
   w = math::Sqr(x1) + math::Sqr(x2);
  } while (w >= 1.0);

  w = math::Sqrt((-2.0 * math::Ln(w))/w);
   gaussValid = true;
   gaussVal = x2 * w;
  return x1 * w * sd;
}

nat32 Random::Poisson(real64 lambda)
{
 nat32 ret = 0;
  real64 sum = 0.0;
  while (true)
  {
   sum += -1.0/lambda * math::Ln(1.0-Normal());
   if (sum>1.0) break;
   ++ret;
  }
 return ret;
}

void Random::UnitVector(math::Vect<3,real32> & out)
{
 real32 phi = math::InvCos(Signed());
 real32 theta = Normal() * math::pi * 2.0;
    
 out[0] = math::Sin(phi) * math::Cos(theta);
 out[1] = math::Sin(phi) * math::Sin(theta);
 out[2] = math::Cos(phi);
}

bit Random::Event(real64 prob)
{
 return Normal()<prob;
}

nat32 Random::Select(nat32 size,real32 * weight)
{
 real32 sum = 0.0;
 for (nat32 i=0;i<size;i++) sum += weight[i];
 
 real32 index = sum * real32(Next())/real32(M);
 
 for (nat32 i=0;i<size;i++)
 {
  if (index<weight[i]) return i;
  index -= weight[i];
 }
 return Next()%size; // Fallback, should only happen in the all-zeroes case.
}

nat32 Random::Select(nat32 size,real64 * weight)
{
 real64 sum = 0.0;
 for (nat32 i=0;i<size;i++) sum += weight[i];
 
 real64 index = sum * real64(Next())/real64(M);
 
 for (nat32 i=0;i<size;i++)
 {
  if (index<weight[i]) return i;
  index -= weight[i];
 }
 return Next()%size; // Fallback, should only happen in the all-zeroes case.
}

bit Random::Weighted(real64 w1,real64 w2)
{
 return Normal()<(w1/(w1+w2));
}

real64 Random::Sample(const math::Range<real32> & r)
{
 nat32 v = Next(); 
 nat32 div = M;
 
 if (!r.minInc) {++v; ++div;}
 if (r.maxInc) --div;
 
 return (r.max-r.min)*real64(v)/real64(div) + r.min;
}

real64 Random::Sample(const math::Range<real64> & r)
{
 nat32 v = Next(); 
 nat32 div = M;
 
 if (!r.minInc) {++v; ++div;}
 if (r.maxInc) --div;
 
 return (r.max-r.min)*real64(v)/real64(div) + r.min;
}

real64 Random::Sample(const math::Sample & s)
{
 return s.Mean() + Gaussian(s.Sd());
}

nat32 Random::Next()
{
 int32 temp = A*(val%Q) - R*(val/Q);
 if (temp>=0) val = temp;
         else val = temp+M;
 return val;
}


//------------------------------------------------------------------------------
 };
};
