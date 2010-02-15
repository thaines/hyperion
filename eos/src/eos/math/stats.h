#ifndef EOS_MATH_STATS_H
#define EOS_MATH_STATS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file stats.h
/// Contains statistical tools.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/math/functions.h"
#include "eos/math/vectors.h"
#include "eos/math/matrices.h"
#include "eos/io/inout.h"
#include "eos/ds/arrays.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
/// Represents a range of numbers, a minimum and a maximum. Expresses if its 
/// inclusive or not at each end. When used with text i/o it uses [ to indicate
/// inclusive and ( to indicate exclusive.
template<typename T = real32>
struct Range
{
 /// Does nothing, leaves the data random.
  Range() {}
  
 /// Initialises it inclusivly.
  Range(T minV,T maxV)
  :min(minV),minInc(true),max(maxV),maxInc(true) {}
  
 /// &nbsp;
  Range(bit minI,T minV,T maxV,bit maxI)
  :min(minV),minInc(minI),max(maxV),maxInc(maxI) {}
  

 T min; ///< The minimum of the range.
 bit minInc; ///< If true the range is inclusive, otherwise exclusive, at the minimum end.
  
 T max; ///< The maximum of the range.
 bit maxInc; ///< If true the range is inclusive, otherwise exclusive, at the maximum end.


 /// &nbsp;
  static inline cstrconst TypeString()
  {
   static GlueStr ret(GlueStr() << "eos::math::Range<" << typestring<T>() << ">");
   return ret;
  }
};

//------------------------------------------------------------------------------
/// A simple class, is given a set of samples, it records and makes avaliable the
/// minimum, maximum, mean and standard deviation. This class is designed with
/// quality of results over speed, and goes to some effort to ensure numerical
/// stability - a faster version can be written.
/// It also has a 'bad result' counter, which can be incrimented to indicate just
/// that, i.e. how many times it didn't produce a number(!).
/// When used with text IO it outputs/inputs as sample_count<bad_results>{min,mean(sd),max}
/// Bad results are only shown if there are bad results, i.e. <0> will never be output.
class Sample
{
 public:
  /// &nbsp;
   Sample()
   :samples(0),badCount(0)
   {}

  /// &nbsp; 
  ~Sample()
  {}

  /// Resets the object as though no samples are contained.
   void Reset() {samples = 0; badCount=0;}


  /// Adds a sample, until this has been called all other methods except
  /// Samples() will return arbitary data.
   void Add(real32 ns)
   {
    if (samples==0)
    {
     samples = 1;
     min = ns; max = ns;
     mean = ns; sdi = 0.0;
    }
    else
    {
     min = math::Min(min,ns);
     max = math::Max(max,ns);
          
     ++samples;
     real64 meanNext = mean + (ns - mean)/real64(samples);
     sdi += (ns - mean)*(ns - meanNext);
     mean = meanNext;
    }
   }
   
  /// Incriments the count of bad samples.
   void AddBad() {++badCount;}


  /// Returns how many samples have been Add(..)'ed to the class, if it
  /// returns 0 don't query the stats.
   nat32 Samples() const {return samples;}
   
  /// Returns how many bad samples have been added.
   nat32 BadSamples() const {return badCount;}

  /// Returns the maximum sample provided.
   real32 Min() const {return min;}

  /// Return the minimum sample provided.
   real32 Max() const {return max;}

  // Outputs the range of samples provided.
   void GetRange(Range<real32> & out) {out.min = min; out.max = max; out.minInc = true; out.maxInc = true;}

  /// Returns the mean of th samples provided.
   real32 Mean() const {return mean;}

  /// Returns the standard deviation of the samples provided.
  /// Note this encapsulates a Sqrt, so cache this value instead
  /// repeatedly calling this method.
   real32 Sd() const {return (samples>1)?math::Sqrt(sdi/real64(samples-1)):0.0;}


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::math::Sample";}


  // Internal use only methods, used by the streaming system.
    nat32 & SamplesRef() {return samples;}
    nat32 & BadSamplesRef() {return badCount;}    
   real32 & MinRef() {return min;}
   real32 & MaxRef() {return max;}
   real64 & MeanRef() {return mean;}
   real64 & SdiRef() {return sdi;}
   const real64 & SdiRef() const {return sdi;}


 private:
  nat32 samples;
  nat32 badCount;

  real32 min;
  real32 max;

  real64 mean; // Mean so far.
  real64 sdi; // Variance * (samples-1).
};

//------------------------------------------------------------------------------
/// This uses kernel density estimation on a 1-dimensional input to estimate
/// an unknown pdf given a set of samples from that pdf. It uses the simplistic
/// 'rule of thumb' approach, with normal distributions, of Deheuvels 1977. 
/// (Taken from 'Bandwidth Selection in Kernel Density Estimation: A Review' 
/// by Turlach.)
///
/// This approach will produce reasonable results for uni-model data, but can 
/// not be expected to do as well for multi-modal data, where more advanced 
/// algorithms would be prefered.
/// Provides a method for obtaining the highest mode, as well as the obvious 
/// sampling of density.
/// This is most useful as an improvment on the mean when a Gaussian pdf can not
/// be reasonably assumed, it is certainly extremelly robust to outliers.
///
/// Instances run in two modes - you first add all the data, then you call run 
/// and can extract the data. No further addition is allowed at that point.
class EOS_CLASS UniDensityEstimate
{
 public:
  /// &nbsp;
   UniDensityEstimate();
   
  /// &nbsp;
   ~UniDensityEstimate();


  /// Adds a data point, that is sampled from our unknown pdf somehow.
  /// Must not be called after Run has been called.
   void Add(real32 x);

  /// Returns how many samples are contained - if it returns 0 don't do anything
  /// other than Add(...) to it.
   nat32 Samples() const;

   
  /// Convert from addition mode to results mode - this is not any slower than 
  /// calling F.
   void Run();

  
  /// Returns the window width it has decided on.
   real32 Width() const;

  /// Returns the probability density at a given point.
  /// O(n) where n is the number of times Add(...) was called.
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
   real32 MaxMode(real32 tol = 0.001,nat32 limit = 1000) const;


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::math::UniDensityEstimate";}


 private:
  static const nat32 incSize = 128; // How much to incriment the array each time it overflows.

  // Samples are collected in an array, on running they array is shrunk to
  // actual size and sorted, the sorting allows for optimisation of the MaxMode
  // method.
   nat32 size; // Size independent of array size - can be smaller for efficiency.
   ds::Array<real32> data;
   
   real32 sd; // Assigned sd for each gaussian that surrounds each sample - Run basically sets this.


  // Helper data structure, used to indicate a range that needs to be searched.
  // Indicates if it is bounded by modes or not...
   struct ToSearch
   {
    struct Edge
    {
     bit mode; // True if bounded by a mode, false if unbounded.
     real32 limit; // The edge of the range.
     int32 index; // Index of the first entry in the array within the range.
    };
    
    Edge low;
    Edge high;
   };
};

//------------------------------------------------------------------------------
/// A simple templated structure that is quite useful for representing
/// multi-variate statistics.
/// Nothing more than a shell.
template <nat32 N,typename T = real32>
class EOS_CLASS MeanCovar
{
 public:
  /// &nbsp;
   Vect<N,T> mean;
   
  /// &nbsp;
   Mat<N,N,T> covar;
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

  template <typename T,typename T2>
  inline T & StreamRead(T & lhs,math::Range<T2> & rhs,Binary)
  {
   lhs >> rhs.min >> rhs.minInc >> rhs.max >> rhs.maxInc;
   return lhs;
  }
  
  template <typename T,typename T2>
  inline T & StreamRead(T & lhs,math::Range<T2> & rhs,Text)
  {
   SkipWS(lhs);
   
   if ((lhs.Peek()!='(')&&(lhs.Peek()!='[')) {lhs.SetError(true); return lhs;}
   rhs.minInc = lhs.Peek()=='[';
   lhs.Skip(1);
   
   lhs >> rhs.min;
   if (lhs.Peek()!=',') {lhs.SetError(true); return lhs;}
   lhs.Skip(1);
   lhs >> rhs.max;
   
   if ((lhs.Peek()!=')')&&(lhs.Peek()!=']')) {lhs.SetError(true); return lhs;}
   rhs.maxInc = lhs.Peek()==']';
   lhs.Skip(1);
   
   return lhs;
  }
  
  template <typename T,typename T2>
  inline T & StreamWrite(T & lhs,const math::Range<T2> & rhs,Binary)
  {
   lhs << rhs.min << rhs.minInc << rhs.max << rhs.maxInc;
   return lhs;
  }
  
  template <typename T,typename T2>
  inline T & StreamWrite(T & lhs,const math::Range<T2> & rhs,Text)
  {
   if (rhs.minInc) lhs << "[";
              else lhs << "(";
   
   lhs << rhs.min << "," << rhs.max;  
   
   if (rhs.maxInc) lhs << "]";
              else lhs << ")";
   
   return lhs;
  }


  template <typename T>
  inline T & StreamRead(T & lhs,math::Sample & rhs,Binary)
  {
   lhs >> rhs.SamplesRef() >> rhs.BadSamplesRef() >> rhs.MinRef() >> rhs.MaxRef() >> rhs.MeanRef() >> rhs.SdiRef();
   return lhs;
  }
  
  template <typename T>
  inline T & StreamRead(T & lhs,math::Sample & rhs,Text)
  {
   SkipWS(lhs);
   lhs >> rhs.SamplesRef();
   
   if (lhs.Peek()=='<')
   {
    lhs.Skip(1);
    lhs >> rhs.BadSamplesRef();
    if (lhs.Peek()!='>') {lhs.SetError(true); return lhs;}
    lhs.Skip(1);
   }
   
   if (lhs.Peek()!='{') {lhs.SetError(true); return lhs;}
   lhs.Skip(1);
   
   lhs >> rhs.MinRef();
   if (lhs.Peek()!=',') {lhs.SetError(true); return lhs;}
   lhs.Skip(1);
   lhs >> rhs.MeanRef();
   
   if (lhs.Peek()!='(') {lhs.SetError(true); return lhs;}
   lhs.Skip(1);
   real64 temp;
   lhs >> temp;
   if (rhs.SamplesRef()>1) rhs.SdiRef() = math::Sqr(temp) * real64(rhs.SamplesRef() - 1);
                      else rhs.SdiRef() = 0.0;
             
   if (lhs.Peek()!=')') {lhs.SetError(true); return lhs;}
   lhs.Skip(1);   
   if (lhs.Peek()!=',') {lhs.SetError(true); return lhs;}
   lhs.Skip(1);
   lhs >> rhs.MaxRef();
   
   if (lhs.Peek()!='}') {lhs.SetError(true); return lhs;}
   lhs.Skip(1);
   
   return lhs;
  }
  
  template <typename T>
  inline T & StreamWrite(T & lhs,const math::Sample & rhs,Binary)
  {
   lhs << rhs.Samples() << rhs.BadSamples() << rhs.Min() << rhs.Max() << rhs.Mean() << rhs.SdiRef();
   return lhs;
  }
  
  template <typename T>
  inline T & StreamWrite(T & lhs,const math::Sample & rhs,Text)
  {
   lhs << rhs.Samples();
   if (rhs.BadSamples()!=0)
   {
    lhs << "<" << rhs.BadSamples() << ">";
   }
   lhs << "{" << rhs.Min() << "," << rhs.Mean() << "(" << rhs.Sd() << ")," << rhs.Max() << "}";
   return lhs;
  }

 };
};

//------------------------------------------------------------------------------
#endif
