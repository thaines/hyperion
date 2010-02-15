//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/math/bessel.h"

#include "eos/math/functions.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
EOS_FUNC real32 ModBesselFirst(nat32 orderX2,real32 x,real32 accuracy,nat32 limit)
{
 LogTime("eos::math::ModBesselFirst");

 // Various simple things...
  real32 halfX = 0.5*x;
  real32 order = 0.5*real32(orderX2); 
  real32 mainMult = math::Sqr(halfX);

 // Calculate the initial summand, for r==0, putting it straight into ret...  
  real32 ret = 1.0;
  {
   real32 temp = 0.0;
   if ((orderX2%2)==1)
   {
    ret = math::Sqrt(2) * math::Sqrt(x) / math::Sqrt(math::pi);
    temp = 0.5;
   }

   for (nat32 p=1;p<=orderX2/2;p++)
   {
    ret *= halfX;
    ret /= real32(p) + temp;
   }
  }


 // Iterate through following summands, calculating each with the aid of the previous...
  real32 summand = ret;
  for (nat32 r=1;r<limit;r++)
  {   
   summand *= mainMult;
   summand /= order + real32(r);
   summand /= real32(r);
   real32 ret2 = ret+summand;
   if (!math::IsFinite(ret2)) break; // If this first the function hasn't worked, but at least you get a finite answer.
   ret = ret2;

   if (summand<accuracy) break;
  }

 return ret;
}

//------------------------------------------------------------------------------
EOS_FUNC real32 InverseModBesselFirstOrder0(real32 x,real32 accuracy,nat32 limit)
{
 LogTime("eos::math::InverseModBesselFirstOrder0");
 if ((x-1.0)<accuracy) return 0.0;

 // Initialise, we go to some effort, as the following iterations are extremelly
 // expensive so a good initialisation makes a hell of a lot of difference...
  real32 ret = math::Ln(5.0*x - 4.0); // The function x = 0.8 * 0.2*exp(ret) fits well for x<10.
  if (x>10.0)
  {
   /// Above x=10 we make use of x = exp(ret)/sqrt(2*pi*ret), which is accurate
   /// for large values. We can't solve this directly, so do newton iterations
   /// as there far cheaper for this function to get closer to the final answer...
    for (nat32 i=0;i<limit;i++)
    {
     real32 shared = math::Exp(ret)/math::Sqrt(2.0*math::pi);
     real32 ivsqret = math::InvSqrt(ret);
     real32 f = shared * ivsqret - x;
     real32 df = shared * (ivsqret - 0.5*math::Pow(ret,real32(-1.5)));
     
     real32 offset = -f/df;
     ret += offset;
     
     if (math::Abs(offset)<(accuracy*10.0)) break;
    }
  }

  
 // Do newton iterations, until convergance (Capped by limit)...
  for (nat32 i=0;i<limit;i++)
  {
   // Initialise offset with r=0 entrys...
    real32 offsetDividend = x - 1.0;
    real32 offsetDivisor = 0.0;
    real32 rho = 1.0/ret;
    
   // Iterate to calculate the correct offset, stopping when its accurate enough...
    for (nat32 r=1;r<limit;r++)
    {
     rho *= 0.25 * math::Sqr(ret) / math::Sqr(real32(r));
     offsetDividend -= rho * ret;
     offsetDivisor += rho * 2.0 * real32(r);
     
     if (offsetDivisor>1e10)
     {
      rho /= offsetDivisor;
      offsetDividend /= offsetDivisor;
      offsetDivisor = 1.0;
     }
     
     if (rho*ret<accuracy*offsetDivisor) break;
    }

   // Apply the offset, break if close enough...
    real32 offset = offsetDividend/offsetDivisor;
    ret += offset;
    if (math::Abs(offset)<accuracy) break;
  }


 return ret;
}

//------------------------------------------------------------------------------
 };
};
