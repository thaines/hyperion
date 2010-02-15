#ifndef EOS_DATA_RANDOMS_H
#define EOS_DATA_RANDOMS_H
//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines

/// \file randoms.h
/// This provides a standard psuedo-random number sequence generator.
/// The algorithm is stolen from Data Structures & Algorithm Analysis in C++ by Weiss

#include "eos/types.h"
#include "eos/math/stats.h"
#include "eos/math/vectors.h"

namespace eos
{
 namespace data
 {
//------------------------------------------------------------------------------
/// A psuedo-random number generator, provides several methods for obtainning 
/// random numbers.
class EOS_CLASS Random
{
 public:
  /// If useSeed is set to false, the default, it automatically makes a best but
  /// certainly not good effort at generating its own starting seed, if set to 
  /// true it uses the passed in seed.
   Random(bit useSeed = false,int32 seed = 0);
   
  /// &nbsp;
   ~Random();


  /// &nbsp;
   void SetSeed(int32 seed);
   
  /// &nbsp;
   int32 GetSeed();


  /// Returns a random byte, [0..255].
   byte Byte();

  /// Returns a random integer in the given range, inclusive.
   int32 Int(int32 min,int32 max);
   
  /// Returns a random real in the given range, inclusive.
   real64 Real(real64 min,real64 max);

   
  /// Returns a random true/false.
   bit Bool();
   
  /// Returns a random real in the [0..1) range.
   real64 Normal();

  /// Returns a random real in the [0..1] range.
   real64 NormalInc();
   
  /// Returns a random real in the (-1..1) range.
   real64 Signed();
   
   
  /// Returns a random sample from a gaussian distribution arround 0 with the given sd.
   real64 Gaussian(real64 sd);
   
  /// Returns a random sample from a Poisson distribution, with parameter lambda.
  /// Note that this is not very efficient - its inner loop will run an average of 
  /// lambda times.
   nat32 Poisson(real64 lambda);


  /// Outputs an uniformly distributed 3D unit vector.
   void UnitVector(math::Vect<3,real32> & out);


  /// Given a 0..1 probability of an event this samples the event,
  /// returning true if it happens and false if it doesn't.
   bit Event(real64 prob);
  
  /// Given two weightings this returns a bit that is true or false depending
  /// on the strength of the two weights. By example if the first weight is 
  /// twice the second it will return true with twice the probability.
   bit Weighted(real64 w1,real64 w2);
   
  /// Given an array of positive weights this returns an index to the array with 
  /// probability matching the contained weights. (Items with weight 0 can't be 
  /// selected, unless all are zero, in which case the last item will be selected.)
   nat32 Select(nat32 size,real32 * weight);

  /// Given an array of positive weights this returns an index to the array with 
  /// probability matching the contained weights. (Items with weight 0 can't be 
  /// selected, unless all are zero, in which case the last item will be selected.)
   nat32 Select(nat32 size,real64 * weight);


  /// This is given a math::Range object, it returns a possible sample from it assuming
  /// a linear range.
   real64 Sample(const math::Range<real32> & r);
  
  /// This is given a math::Range object, it returns a possible sample from it assuming
  /// a linear range.
   real64 Sample(const math::Range<real64> & r);  
  
  /// This is given a math::Sample, it ignores min/max and assumes a gaussian distribution,
  /// returns a suitably selected random value under these assumptions.
   real64 Sample(const math::Sample & s);


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::data::Random";}


 private:
  nat32 Next();

  int32 val;
  
  // It costs some memory, but the gaussian generation method creates
  // two gaussians each time it is called, and it would be a waste to 
  // not use this fact, espcially as its quite a maths intensive function...
   bit gaussValid;
   real64 gaussVal;
};

//------------------------------------------------------------------------------
 };
};
#endif
