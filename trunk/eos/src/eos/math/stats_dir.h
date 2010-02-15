#ifndef EOS_MATH_STATS_DIR_H
#define EOS_MATH_STATS_DIR_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file stats_dir.h
/// Contains stuff to do with directional statistics.

#include "eos/types.h"

#include "eos/data/randoms.h"
#include "eos/math/mat_ops.h"
#include "eos/math/eigen.h"
#include "eos/io/inout.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
/// This class represents a Fisher distribution with a vector, the length of the
/// vector being the concentration.
class EOS_CLASS Fisher : public Vect<3,real32>
{
 protected:
  static real32 Ak(real32 k)
  {
   if (IsZero(k)) return 0.0;
   return Coth(k) - 1.0/k;
  }
  
  static real32 InvAk(real32 x,real32 accuracy=0.001,nat32 limit=1000);


 public:
  /// Initialises it to be the uniform distribution.
   Fisher():Vect<3,real32>(0.0) {} 
  
  /// Initialises it from a vector, will have the direction of the vector and
  /// a concentration parameter equal to the length of the vector.
   Fisher(const Vect<3,real32> & rhs):Vect<3,real32>(rhs) {}
  
  /// Initialise it from a unit vector and a concentration strength.
   Fisher(const Vect<3,real32> & dir,real32 k) {for (nat32 i=0;i<3;i++) (*this)[i] = dir[i]*k;}
  
  /// &nbsp;
   Fisher(const Fisher & rhs):Vect<3,real32>(rhs) {}
  
  /// &nbsp;
   ~Fisher() {}
 

 /// Rotates the distribution.
  void Rotate(const Mat<3,3,real32> & rot)
  {
   Vect<3,real32> temp;
   MultVect(rot,*this,temp);
   *this = temp;
  }
 
  /// &nbsp;
   Fisher & operator *= (const Fisher & rhs) {for (nat32 i=0;i<3;i++) (*this)[i] += rhs[i]; return *this;}

  /// &nbsp;
   Fisher & operator *= (real32 rhs) {for (nat32 i=0;i<3;i++) (*this)[i] *= rhs; return *this;}

   
  /// Convolves this Fisher distribution with another Fisher distribution with 
  /// the given concentration parameter.
  /// There is necesarilly no offset.
   void Convolve(real32 kk)
   {
    LogTime("eos::math::Fisher::Convolve");
    real32 tk = Length();
    if (IsZero(tk)) return;
    if (IsZero(kk))
    {
     for (nat32 i=0;i<3;i++) (*this)[i] = 0.0;
     return;
    }    
    
    real32 nk = InvAk(Ak(tk) * Ak(kk));
    real32 mult = nk/tk;

    for (nat32 i=0;i<3;i++) (*this)[i] *= mult;
   }
   
  /// This is the same as Convolve, except it returns the normalisation constant
  /// after convolution divided by the normalisation constant before convolution.
  /// This number is quite useful in certain scenarios.
   real32 ConvolveRatio(real32 kk)
   {
    LogTime("eos::math::Fisher::ConvolveRatio");
    real32 tk = Length();
    if (IsZero(tk)) return 1.0;
    if (IsZero(kk))
    {
     for (nat32 i=0;i<3;i++) (*this)[i] = 0.0;
     return 0.0;
    }

    real32 nk = InvAk((Coth(tk) - 1.0/tk) * (Coth(kk) - 1.0/kk));
    real32 mult = nk/tk;

    for (nat32 i=0;i<3;i++) (*this)[i] *= mult;

    real32 ret = ((Sinh(tk))/(Sinh(nk)))*(nk/tk); // Obvious way of writting this can cause numerical overflow.
    if (math::IsFinite(ret)) return ret;
                        else return 0.0;
   }


  /// Returns the multiplicative factor for normalising the distribuution.
   real32 Norm() const
   {
    LogTime("eos::math::Fisher::Norm");
    real32 k = Length();
    if (IsZero(k)) return 1.0/(4.0*math::pi);
    return k/(4.0*math::pi*Sinh(k));
   }
  
  /// Returns the unnormalised probablity of a given normalised vector.
  /// Multiply by the result of the Norm() method to normalise.
  /// Seperated like this because Norm is relativly slow - should be cached for
  /// multiple calls of this method.
   real32 UnnormProb(const Vect<3,real32> & rhs) const
   {
    return Exp(rhs * (*this));
   }


  /// This samples from the distribution, outputting a normalised vector.
   void Sample(Vect<3,real32> & out,data::Random & rand) const;


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::math::Fisher";}
};

//------------------------------------------------------------------------------
/// This class represents a Bingham distribution; uses a symmetric 3x3 matrix.
class EOS_CLASS Bingham : public Mat<3,3,real32>
{
 public:
  /// Initialises it as the uniform distribution.
   Bingham() {Zero(*this);}

  /// Initialises it without rotation, by giving its diagonal components.
  /// (alpha is in x-axis, beta in y.)
   Bingham(real32 alpha,real32 beta)
   {
    Zero(*this);
    (*this)[0][0] = alpha;
    (*this)[1][1] = beta;
   }
  
  /// Fully initialises it; given alpha and beta parameters plus a rotation matrix.
   Bingham(real32 alpha,real32 beta,const Mat<3,3,real32> & rot)
   {
    Zero(*this);
    (*this)[0][0] = alpha;
    (*this)[1][1] = beta;

    Mat<3,3,real32> temp;
    Mult(rot,*this,temp);
    MultTrans(temp,rot,*this);
   }  
  
  /// &nbsp;
   Bingham(const Bingham & rhs):Mat<3,3,real32>(rhs) {}

  /// &nbsp;
   ~Bingham() {}
  
  
  /// Rotates the distribution.
   void Rotate(const Mat<3,3,real32> & rot)
   {
    Mat<3,3,real32> temp;
    Mult(rot,*this,temp);
    MultTrans(temp,rot,*this);
   }
  
  /// &nbsp;
   Bingham & operator *= (const Bingham & rhs) {static_cast<Mat<3,3,real32>&>(*this) += rhs; return *this;}


  /// Returns the unnormalised probablity of a given normalised vector.
   real32 UnnormProb(const Vect<3,real32> & rhs) const
   {
    return Exp(VectMultVect(rhs,*this,rhs));
   }


  /// Seperates the distribution into its rotation matrix and diagonal matrix
  /// parts, such that rot * diag * rot^T is this matrix. Its an eigenvector 
  /// decomposition in other words.
  /// diag will contain a 0.
   void Decompose(Mat<3,3,real32> & rot,Vect<3,real32> & diag) const
   {
    LogTime("eos::math::Bingham::Decompose");
    Mat<3,3,real32> temp = static_cast<const Mat<3,3,real32>&>(*this);
    SymEigen(temp,rot,diag);
    
    real32 min = Min(diag[0],diag[1],diag[2]);
    diag -= min;
   }


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::math::Bingham";}
};

//------------------------------------------------------------------------------
/// This class represents a Fisher-Bingham distribution.
class EOS_CLASS FisherBingham
{
 public:
  /// Initialises to the uniform distribution.
   FisherBingham() {}
  
  /// Initialises to a Fisher distribution alone.
   FisherBingham(const Fisher & rhs):fisher(rhs) {}
  
  /// Initialises to a Bingham distribution alone.
   FisherBingham(const Bingham & rhs):bingham(rhs) {}

  /// Initialises it as the multiplication of a given Fisher and Bingham distribution.
   FisherBingham(const Fisher & fish,const Bingham & bing):fisher(fish),bingham(bing) {}
   
  /// Initialises to a Bingham-Mardia distribution.
  /// (A cone shape in other words.)
   FisherBingham(const Vect<3,real32> & dir,real32 angle,real32 k)
   :fisher(dir,2.0 * k * Cos(angle))
   {
    VectVectTrans(dir,dir,bingham);
    static_cast<Mat<3,3,real32>&>(bingham) *= -k;
   }
   
  /// Initialises it to a symmetric bi-model distribution, no fisher component,
  /// just a Bingham component with two of its eigenvectors 0, the other ev;
  /// rotated so its maximums are along the given vector and its negative.
   FisherBingham(const Vect<3,real32> & dir,real32 ev)
   {
    Vect<3,real32> bing(0.0);
    bing[0] = ev;
    
    math::Mat<3,3,real32> rotT;
    Vect<3,real32> o1,o2;
    FinishCordSys(dir,o1,o2);
    rotT[0][0] = dir[0]; rotT[0][1] = o1[0]; rotT[0][2] = o2[0];
    rotT[1][0] = dir[1]; rotT[1][1] = o1[1]; rotT[1][2] = o2[1];
    rotT[2][0] = dir[2]; rotT[2][1] = o1[2]; rotT[2][2] = o2[2];
    
    math::Mat<3,3,real32> temp;
    MultDiag(rotT,bing,temp);
    MultTrans(temp,rotT,bingham);
   }

  /// Initialises it to a disc distribution, no fisher component,
  /// just a Bingham component with two of its eigenvectors ev, the other 0,
  /// rotated so its minimas are along the given vector and its negative.
   FisherBingham(real32 ev,const Vect<3,real32> & dir)
   {
    Vect<3,real32> bing(0.0);
    bing[1] = ev;
    bing[2] = ev;
    
    math::Mat<3,3,real32> rotT;
    Vect<3,real32> o1,o2;
    FinishCordSys(dir,o1,o2);
    rotT[0][0] = dir[0]; rotT[0][1] = o1[0]; rotT[0][2] = o2[0];
    rotT[1][0] = dir[1]; rotT[1][1] = o1[1]; rotT[1][2] = o2[1];
    rotT[2][0] = dir[2]; rotT[2][1] = o1[2]; rotT[2][2] = o2[2];
    
    math::Mat<3,3,real32> temp;
    MultDiag(rotT,bing,temp);
    MultTrans(temp,rotT,bingham);
   }

  /// &nbsp;
   FisherBingham(const FisherBingham & rhs):fisher(rhs.fisher),bingham(rhs.bingham) {}
  
  /// &nbsp;
   ~FisherBingham() {}
   
  
  /// Returns true if the distribution contains nan's or infinities...
   bit Bad() const
   {
    if (!math::IsFinite(fisher[0])) return true;
    if (!math::IsFinite(fisher[1])) return true;
    if (!math::IsFinite(fisher[2])) return true;
    if (!math::IsFinite(bingham[0][0])) return true;
    if (!math::IsFinite(bingham[0][1])) return true;
    if (!math::IsFinite(bingham[0][2])) return true;
    if (!math::IsFinite(bingham[1][0])) return true;
    if (!math::IsFinite(bingham[1][1])) return true;
    if (!math::IsFinite(bingham[1][2])) return true;
    if (!math::IsFinite(bingham[2][0])) return true;
    if (!math::IsFinite(bingham[2][1])) return true;
    if (!math::IsFinite(bingham[2][2])) return true;
    return false;
   }

  /// Rotates the distribution.
   void Rotate(const Mat<3,3,real32> & rot)
   {
    fisher.Rotate(rot);
    bingham.Rotate(rot);
   }

  /// &nbsp;
   FisherBingham & operator *= (const Fisher & rhs)
   {
    fisher *= rhs;
    return *this;
   }

  /// &nbsp;
   FisherBingham & operator *= (const Bingham & rhs)
   {
    bingham *= rhs;
    return *this;
   }
   
  /// &nbsp;
   FisherBingham & operator *= (const FisherBingham & rhs)
   {
    fisher *= rhs.fisher;
    bingham *= rhs.bingham;
    return *this;
   }
   
  /// This has no mathematical meaning, but is none the less useful as a sort of
  /// 'smoothing' operator.
  /// Simply divides through the fisher vector and bingham matrix - makes it 
  /// less concentrated.
   FisherBingham & operator /= (real32 rhs)
   {
    fisher /= rhs;
    bingham /= rhs;
    return *this;
   }

  /// Convolves this distribution with a Fisher distribution with the given 
  /// concentration parameter. There is necesarilly no offset.
  /// Has to do a lot of approximation to work, and because the convolution of
  /// a Fisher Bingham distribution can look nothing like one it can be a long
  /// way from the truth. It does well in the required situations however.
   void Convolve(real32 k);
   
  
  /// Returns the unnormalised probablity of a given normalised vector.
  /// The normalisation constant can not be calculated analytically.
   real32 UnnormProb(const Vect<3,real32> & rhs) const
   {
    return Exp(rhs * fisher + VectMultVect(rhs,bingham,rhs));
   }
   
  /// Returns the normalising divider of the distribution. Well, an
  /// approximation, that is insanly hard and rather slow to calculate.
  /// Divide the results of UnnormProb to get normalised answers.
   real32 NormDiv() const;
  
  /// Returns the cost for a given normalised vector.
  /// Has an arbitary offset, i.e. unnormalised. Can be negative.
   real32 Cost(const Vect<3,real32> & rhs) const
   {
    return rhs * fisher + VectMultVect(rhs,bingham,rhs);
   }


  /// Outputs the direction with the greatest probability density function value.
  /// Has to get iterative, so a touch slow. (Initialisation is extremelly close 
  /// to actual answer however, so usually fast to converge.)
  /// Not 100% perfect, can screw up if there is an infinite number of maximas
  /// caused by a cone distribution and return there average, i.e. the direction
  /// of the cone.
   void Maximum(Vect<3> & out,real32 err = 1e-3,nat32 limit = 1000) const;
  
  /// This outputs all critical points of the distribution - uses the techneque 
  /// of the Maximum method but solves for all roots, hence providing all
  /// maxima, minima and standing points. Merges duplicates and returns how many
  /// critical points exist - uses 0 to indicate infinity, i.e. the uniform
  /// distribution. The points will be sorted, maxima to minima so the maxima
  /// will be the first entrys and the minima the last entrys.
   nat32 Critical(Vect<3> out[6],real32 err = 1e-3,nat32 limit = 1000) const;
  
  /// Less sophisticated maximum finding method, that can find multiple maxima in
  /// the case of bimodal distributions making it potentially more useful.
  /// Uses Levenberg Marquardt on spherical coordinates, initialised from 
  /// multiple positions to find multiple maxima. It then merges duplicate locations
  /// and select the maximas or maxima.
  /// Returns the number of maxima found - can be 0 if the distribution is
  /// boring.
   nat32 Maximums(Vect<3> out[2]) const;
   
   
  /// This approximates the distribution by a sum of Fisher distributions. 
  /// Note the use of the word approximate - its not the best match ever.
  /// The output Fisher distributions are weighted by there inverse normalising
  /// multiplier, not provided, such that it is a sum of unnormalised Fisher 
  /// distributions.
   void ToFisher(nat32 num,Fisher * out) const;


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::math::FisherBingham";}


  /// Provides access to the Fisher component.
   Fisher fisher;
  
  /// Provides access to the Bingham component.
   Bingham bingham;
};

//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
// IO stream integration for the above classes...
namespace eos
{
 namespace io
 {
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::math::FisherBingham & rhs,Text)
  {
   lhs << "{" << rhs.fisher << "," << rhs.bingham << "}";
   return lhs;
  }
  
 };
};
//------------------------------------------------------------------------------
#endif
