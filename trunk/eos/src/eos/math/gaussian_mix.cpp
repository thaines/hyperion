//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/math/gaussian_mix.h"

#include "eos/math/mat_ops.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
EOS_FUNC real32 ChiSquareCulmInv(real32 prob,nat32 dims,real32 accuracy)
{
 // Multiply prob by the integral normalisation divisor, so we can then forget it...
  prob  *= math::Pow(2.0,0.5*real32(dims)) * GammaHalfs(dims);
  
 // Loop through, starting from zero and increasing by the step size using 
 // numeric integration, until we have the answer needed...
  real32 sum = 0.0; // Sum so far - stop when it excedes prob.
  real32 x = 0.0; // x value we are upto.
  real32 prev = 0.0; // Previous value - so we can approximate with linear interpolation.
  
  while (sum<prob)
  {
   x += accuracy;
   real32 sample = math::Exp(-0.5*x) * math::Pow<real32>(x,real32(dims)/2.0-1.0);
   sum += 0.5*(sample+prev) * accuracy;
   prev = sample;
  }

 return x;
}

//------------------------------------------------------------------------------
Gauss2D::Gauss2D()
{
 math::Zero(iv);
 ivMean[0] = 0.0;
 ivMean[1] = 0.0;
}

Gauss2D::Gauss2D(const math::Vect<2> & mean,const math::Mat<2> & covar)
{
 iv = covar;
 math::Inverse22(iv);
 math::MultVect(iv,mean,ivMean);
}

Gauss2D::Gauss2D(const Gauss1D & rhs)
{
 iv[0][0] = rhs.iv;
 iv[0][1] = 0.0;
 iv[1][0] = 0.0;
 iv[1][1] = 0.0;

 ivMean[0] = rhs.ivMean;
 ivMean[1] = 0.0;
}

Gauss2D::Gauss2D(const Gauss2D & rhs)
:iv(rhs.iv),ivMean(rhs.ivMean)
{}

Gauss2D::~Gauss2D()
{}

Gauss2D & Gauss2D::operator = (const Gauss2D & rhs)
{
 iv = rhs.iv;
 ivMean = rhs.ivMean;
 return *this;
}

void Gauss2D::Marg1(Gauss1D & out) const
{
 if (math::IsZero(iv[0][0]))
 {
  out.iv = iv[1][1];
  out.ivMean = ivMean[1];
 }
 else
 {
  out.iv = iv[1][1] - iv[0][1]*iv[1][0]/iv[0][0];
  out.ivMean = ivMean[1] - iv[0][1]*ivMean[0]/iv[0][0];
 }
}

void Gauss2D::Marg2(Gauss1D & out) const
{
 if (math::IsZero(iv[1][1]))
 {
  out.iv = iv[0][0];
  out.ivMean = ivMean[0];
 }
 else
 {
  out.iv = iv[0][0] - iv[0][1]*iv[1][0]/iv[1][1];
  out.ivMean = ivMean[0] - iv[0][1]*ivMean[0]/iv[1][1];
 }
}

Gauss2D & Gauss2D::operator *= (const Gauss2D & rhs)
{
 iv += rhs.iv;
 ivMean += rhs.ivMean;
 return *this;
}

//------------------------------------------------------------------------------
GaussMix1D::GaussMix1D(nat32 sz)
:ds::Array<WeightGauss>(sz)
{}

GaussMix1D::~GaussMix1D()
{}

void GaussMix1D::Normalise()
{
 // Sum the probabilities...
  real32 sum = 0.0;
  for (nat32 i=0;i<Size();i++) sum += (*this)[i].prob;

 // Divide through...
  sum = 1.0/sum;
  for (nat32 i=0;i<Size();i++) (*this)[i].prob *= sum;
}

void GaussMix1D::SortByMean()
{
 ds::Array<WeightGauss>::SortNorm();
}

real32 GaussMix1D::F(real32 x) const
{
 real32 ret = 0.0;
 for (nat32 i=0;i<Size();i++)
 {
  ret += (*this)[i].Sample(x);
 }
 return ret;
}

real32 GaussMix1D::Lowest() const
{
 real32 ret = (*this)[0].mean;
 for (nat32 i=1;i<Size();i++) ret = math::Min(ret,(*this)[0].mean);
 return ret;
}

real32 GaussMix1D::Highest() const
{
 real32 ret = (*this)[0].mean;
 for (nat32 i=1;i<Size();i++) ret = math::Max(ret,(*this)[0].mean);
 return ret;
}

Vector<real32> * GaussMix1D::GetSamples(nat32 samples) const
{
real32 min = Lowest();
 real32 mult = (Highest() - min)/real32(samples-1);

 Vector<real32> * ret = new Vector<real32>(samples);
  for (nat32 i=0;i<samples;i++)
  {
   (*ret)[i] = F(min + i*mult);
  }
 return ret;
}

real32 GaussMix1D::MaxMode(real32 tol,nat32 limit) const
{
 LogTime("eos::math::GaussMix1D::MaxMode");

 // Move up from negative infinity to find a lower bound...
  real32 lower = ToFixedPoint((*this)[0].mean,tol,limit);
  real32 bsf  = lower;
  real32 fbsf = F(lower);


 // Move down from positive infinity to find an upper bound...
  real32 upper = ToFixedPoint((*this)[this->Size()-1].mean,tol,limit);
  real32 score = F(upper);
  if (score>fbsf)
  {
   bsf = upper;
   fbsf = score;
  }


 // Divide and conquer the range found...
  BestInRange(lower,upper,bsf,fbsf,tol,limit);
  return bsf;
}

void GaussMix1D::BestInRange(real32 lower,real32 upper,real32 & bsf,real32 & fbsf,real32 tol,nat32 limit) const
{
 if ((upper-lower)<tol) return;

 real32 centre = (lower + upper) * 0.5;
 real32 converge = ToFixedPoint(centre,tol,limit);
 if (converge<centre)
 {
  // Went down...
   if (converge<lower)
   {
    BestInRange(centre,upper,bsf,fbsf,tol,limit);
   }
   else
   {
    real32 score = F(converge);
    if (score>fbsf)
    {
     bsf = converge;
     fbsf = score;
    }

    BestInRange(lower,converge,bsf,fbsf,tol,limit);
    BestInRange(centre,upper,bsf,fbsf,tol,limit);
   }
 }
 else
 {
  // Went up...
   if (converge>upper)
   {
    BestInRange(lower,centre,bsf,fbsf,tol,limit);
   }
   else
   {
    real32 score = F(converge);
    if (score>fbsf)
    {
     bsf = converge;
     fbsf = score;
    }

    BestInRange(converge,upper,bsf,fbsf,tol,limit);
    BestInRange(lower,centre,bsf,fbsf,tol,limit);
   }
 }
}

//------------------------------------------------------------------------------
 };
};
