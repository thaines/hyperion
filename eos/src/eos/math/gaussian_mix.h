#ifndef EOS_MATH_GAUSSIAN_MIX_H
#define EOS_MATH_GAUSSIAN_MIX_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file gaussian_mix.h
/// Contains everything to do with the gaussian (normal) function. This includes
/// an ultrafast gaussian mixtures implimentation.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/math/functions.h"
#include "eos/math/vectors.h"
#include "eos/math/matrices.h"
#include "eos/ds/arrays.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
/// Returns the gaussian for a given standard deviation and x, with a mean of 0.
template <typename T>
inline T Gaussian(T sd,T x)
{
 return Exp(-0.5*Sqr(x)/Sqr(sd))/(sd*Sqrt(T(2.0)*pi));
}

/// Returns the first differential of the gaussian for a given standard deviation 
/// and x, with a mean of 0.
template <typename T>
inline T GaussianDiff(T sd,T x)
{
 return (-x * Exp(-0.5*Sqr(x)/Sqr(sd))) / (sd*sd*sd*Sqrt(T(2.0)*pi));
}


/// Returns the un-normalised gaussian for a given standard deviation and x,
/// with a mean of 0.
template <typename T>
inline T UnNormGaussian(T sd,T x)
{
 return Exp(-0.5*Sqr(x)/Sqr(sd));
}

//------------------------------------------------------------------------------
/// Outputs a multivariate gaussian for a given covariance matrix and x, with 
/// a mean of the zero vector. The covariance matrix is provided as a diagonal
/// matrix.
template <nat32 S,typename T>
inline T Gaussian(const Vect<S,T> & coVar,const Vect<S,T> & x)
{
 T det = T(1.0);
 for (nat32 i=0;i<S;i++) det *= coVar[i];
 
 T top = T(0.0);
 for (nat32 i=0;i<S;i++) top += Sqr(x[i])/coVar[i];
 
 T pim2 = T(1.0);
 for (nat32 i=0;i<S;i++) pim2 *= 2.0*pi; // With luck should be optimised away.
 
 return Exp(-0.5*top)/Sqrt(det*pim2);
}

//------------------------------------------------------------------------------
/// Outputs a multivariate gaussian for a given covariance matrix and x, with 
/// a mean of the zero vector. Instead of just providing the covariance matrix
/// you provide its determinant and its inverse, these being the pairs actually
/// used. Saves on calculation when repeated samples are made.
template <nat32 S,typename T>
inline T Gaussian(T coVarDet,const Mat<S,S,T> & invCoVar,const Vect<S,T> & x)
{
 T pim2 = T(1.0);
 for (nat32 i=0;i<S;i++) pim2 *= 2.0*pi; // With luck should be optimised away.

 return Exp(-0.5*VectMultVect(x,invCoVar,x))/Sqrt(coVarDet*pim2);
}

//------------------------------------------------------------------------------
/// This class represents a 1 dimensional gaussian, representing it as the 
/// inverse of the variance multiplied by the mean and the inverse of the 
/// variance.
/// This makes multiplying two gaussians the simple addition of these two 
/// variables, but everything else complex. 
/// This is ideal for a message passing system where multiplcation is the norm.
class EOS_CLASS Gauss1D
{
 public:
  /// Sets it to be such that multiplying with another Gauss1D will set it to be
  /// that Gauss1D. In other words, it sets the variance to infinity, so don't 
  /// go asking for it, or the mean, or the sd!
   Gauss1D() {iv = 0.0; ivMean = 0.0;}

  /// &nbsp;
   Gauss1D(real32 mean,real32 sd) {iv = 1.0/math::Sqr(sd); ivMean = iv * mean;}

  /// &nbsp;
   Gauss1D(const Gauss1D & rhs) {iv = rhs.iv; ivMean = rhs.ivMean;}

  /// &nbsp;
   ~Gauss1D() {}


  /// &nbsp;
   Gauss1D & operator = (const Gauss1D & rhs) {iv = rhs.iv; ivMean = rhs.ivMean; return *this;}


  /// &nbsp;
   Gauss1D & operator *= (const Gauss1D & rhs) {iv += rhs.iv; ivMean += rhs.ivMean; return *this;}
   
  /// &nbsp;
   Gauss1D & operator /= (const Gauss1D & rhs) {iv -= rhs.iv; ivMean -= rhs.ivMean; return *this;}


  /// &nbsp;
   real32 Mean() const {return ivMean/iv;}

  /// &nbsp;
   real32 Variance() const {return 1.0/iv;}

  /// &nbsp;
   real32 Sd() const {return math::Sqrt(1.0/iv);}
   
  /// Returns true if its defined, false if not. False means the standard 
  /// deviation is infinity, and all values are equally likelly.
   bit Defined() const {return !math::IsZero(iv);}


  /// Direct accessor of inverse variance.
  /// Should not be used unless you know what your doing.
   real32 & InvCoVar() {return iv;}
   
  /// Direct accessor of inverse variance multiplied by the mean.
  /// Should not be used unless you know what your doing.
   real32 & InvCoVarMean() {return ivMean;}


  /// &nbsp;
   static cstrconst TypeString() {return "eos::math::Gauss1D";}


 private:
  friend class Gauss2D;

  real32 iv; // Inverse variance.
  real32 ivMean; // Inverse variance multiplied by the mean.
};

//------------------------------------------------------------------------------
/// This class represents a 2 dimensional gaussian, representing it as the 
/// inverse of the covariance matrix multiplied by the mean vector and the 
/// inverse of the covariance matrix.
/// Like Gauss1D this is good for multiplication and little else, making it 
/// good for message passing.
class EOS_CLASS Gauss2D
{
 public:
  /// Sets it up so that if multiplied by another Gauss2D the other Gauss2D will
  /// replace this one, i.e. the variance is set to infinity for both variables.
   Gauss2D();

  /// &nbsp;
   Gauss2D(const math::Vect<2> & mean,const math::Mat<2> & covar);

  /// On being given a Gauss1D this expands it, making it variable one whilst
  /// variable 2 is set with a variance of infinity.
   Gauss2D(const Gauss1D & rhs);

  /// &nbsp;
   Gauss2D(const Gauss2D & rhs);

  /// &nbsp;
   ~Gauss2D();


  /// &nbsp;
   Gauss2D & operator = (const Gauss2D & rhs);

  /// This marginalises over variable 1, outputing variable 2 alone.
   void Marg1(Gauss1D & out) const;

  /// This marginalises over variable 2, outputing variable 1 alone.
   void Marg2(Gauss1D & out) const;


  /// &nbsp;
   Gauss2D & operator *= (const Gauss2D & rhs);
   
  
  /// Direct accessor of inverse covariance.
  /// Should not be used unless you know what your doing.
   math::Mat<2> & InvCoVar() {return iv;}
   
  /// Direct accessor of inverse covariance multiplied by the mean.
  /// Should not be used unless you know what your doing.
   math::Vect<2> & InvCoVarMean() {return ivMean;}


  /// &nbsp;
   static cstrconst TypeString() {return "eos::math::Gauss2D";}


 private:
  math::Mat<2> iv;
  math::Vect<2> ivMean;
};

//------------------------------------------------------------------------------
/// This represents a weighted guassian - simply a mean, standard deviation and
/// multiplier all stored in a single class for conveniance.
class EOS_CLASS WeightGauss
{
 public:
  /// Compares the means.
   bit operator < (const WeightGauss & rhs) const {return mean<rhs.mean;}
 
  /// This returns the probability of a given x, normalised and multiplied by prob.
   real32 Sample(real32 x) const
   {return (prob/(sd*Sqrt(2.0*pi)))*Gaussian(sd,mean-x);}


  /// Mean.
   real32 mean;

  /// Standard deviation.
   real32 sd;

  /// Probability, ushally set to 1, but if this class is used in a set then
  /// the sum of all members is ushally 1.
   real32 prob;
};

//------------------------------------------------------------------------------
/// This enhanced array is filled by the user with guassians, so as to form a 1D
/// gaussian mixture. It then provides some useful functionality, most noticably
/// for finding the mixtures most probable value.
///
/// WARNING: Not 100% sure the MaxMode method is reliable - tests have found 
/// instances of it diverging from the maximum maxima.
class EOS_CLASS GaussMix1D : public ds::Array<WeightGauss>
{
 public:
  /// &nbsp;
   GaussMix1D(nat32 sz = 0);

  /// &nbsp;
   ~GaussMix1D();
   
  
  /// Normalises the probability values given, for if you filled the array with
  /// weights
   void Normalise();
   
  /// Sorts the array by mean, so the lowest mean is first.
   void SortByMean();


  /// Returns the probability density at a given point.
  /// O(Size()).
  /// Does not normalise - the values are relative.
   real32 F(real32 x) const;
   
  
  /// Returns the lowest value contained within.
   real32 Lowest() const;

  /// Returns the highest value contained within.
   real32 Highest() const;
  
  /// Generates a vector of the given length, of probability densities.
  /// The first entry is at the lowest value, the last at the highest value.
  /// All other entrys are linearly interpolated between.
  /// And yes, it is slow.
  /// You have to delete the returnee.
   Vector<real32> * GetSamples(nat32 samples) const;


  /// Finds the position of the highest mode in the expressed pdf.
  /// Runs a fixed point iterative algorithm from every gaussian in the mixture,
  /// and whilst it optimises out overlap as its in the 1D domain in general the
  /// entire space of the function could be iterated. This is slow.
  /// Its at best O(n^2), at worst more like O(n^3), in reality anywhere between
  /// those two and possibly worse. It is however very tight and good at 
  /// avoiding un-necesary work, and so should  converge quickly for most data 
  /// sets. A pathological case will cause old age however.
  /// When an iteration moves less than tol its assumed to have converged, only 
  /// limit moves can be made before it gives up.
  ///
  /// The array *must* be SortByMean()-ed before calling this method,
  /// for efficiency reasons.
   real32 MaxMode(real32 tol = 0.001,nat32 limit = 1000) const;
   
   
  /// &nbsp;
   static inline cstrconst TypeString()
   {return "eos::math::GaussMix1D";}
   
   
 private:
  // Helper for MaxMode - given a starting point this uses a fixed point 
  // algorithm to find the nearest mode to the fixed point. Can be given +/i 
  // infinity...
   inline real32 ToFixedPoint(real32 pos,real32 tol,nat32 limit) const
   {
    for (nat32 i=0;i<limit;i++)
    {
     real32 newPos = 0.0;
     real32 div = 0.0;
     for (nat32 j=0;j<Size();j++)
     {
      real32 cf = (*this)[j].prob * Gaussian((*this)[j].sd,pos-(*this)[j].mean);
      cf /= math::Sqr((*this)[j].sd);
      
      newPos += cf * (*this)[j].mean;
      div += cf;
     }
     newPos /= div;
     
     //LogDebug("fpt" << LogDiv() << pos << " (" << F(pos) << ")" << LogDiv() << newPos << " (" << F(newPos) << ")");
 
     real32 diff = math::Abs(pos-newPos);
     pos = newPos;

     if (diff<tol) break;
    }
    return pos;
   }


  // Helper for MaxMode - given a range it returns the best mode in the range
  // found. Recursive, does limited tail recursion...
   void BestInRange(real32 lower,real32 upper,real32 & bsf,real32 & fbsf,real32 tol,nat32 limit) const;
};

//------------------------------------------------------------------------------
/// This represents a Gaussian Mixture. This is a density function
/// (Integrates to 1 over [-inf,+inf], allways positive.) that approximatly
/// represents a function of some kind using a set of weighted gaussian functions.
/// Provides operations to multiply, add and construct from function pointers,
/// vectors etc. Suport for integration and differentiation is also provided.
/// Of course, it allows for sampling the represented function.
///
/// This implimentation is designed to be insanly efficient, and internally uses
/// a balanced kd tree to sort the kernels, which it then uses for the various
/// operations. Additional information is stored beyond the gaussian parameters
/// for each function to assist with this, though it does still keep a 
/// reasonably small memory footprint. In fact, multiple implimentations of
/// certain algorithms are provided so the user can choose the level of speed 
/// versus accuracy to go for. The number of gaussians to use for representation
/// is runtime selectable.
///
/// Templated on the dimensionality, as it can work in any dimension. Note that
/// this shouldn't really surpass 3, as it just can't be trusted at that point.
///
/// NOT IMPLIMENTED.
template <nat32 D>
class EOS_CLASS GaussMix
{
 public:
  /// Provided with the number of member gaussians to use for the
  /// representation, constructs it containning random data.
   GaussMix(nat32 mg);
   
  /// &nbsp;
   ~GaussMix();


  /// &nbsp;
   static inline cstrconst TypeString() 
   {
    static GlueStr ret(GlueStr() << "eos::math::GaussMix<" << D << ">");
    return ret;
   } 


 private:
 
};

//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
// I/O functions...
namespace eos
{
 namespace io
 {


  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::math::Gauss1D & rhs,Text)
  {
   lhs << rhs.Mean() << "(" << rhs.Sd() << ")";
   return lhs;
  }


 };      
};
//------------------------------------------------------------------------------
#endif
