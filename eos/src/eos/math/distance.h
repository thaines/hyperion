#ifndef EOS_MATH_DISTANCE_H
#define EOS_MATH_DISTANCE_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file distance.h
/// provides an abstraction of distance measures, via functors.

#include "eos/types.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
/// An abstraction of a measure of distance, of arbitary dimension.
class EOS_CLASS Distance : public Deletable
{
 public:
  /// &nbsp;
   ~Distance();
  
  /// Given two points in n dimensional space this must return the distance.
  /// d is the dimensionality, pa and pb arrays of length d that represent the 
  /// coordinates to return the distance between.
   virtual real32 operator () (nat32 d,const real32 * pa,const real32 * pb) const = 0;
   
  /// &nbsp;
   virtual cstrconst TypeString() const = 0;
};

//------------------------------------------------------------------------------
/// Implimentation of euclidean distance.
/// Provides a set-able distance multiplier which default to 1.
class EOS_CLASS EuclideanDistance : public Distance
{
 public:
  /// &nbsp;
   EuclideanDistance():mult(1.0) {}

  /// &nbsp;
   ~EuclideanDistance();


  /// &nbsp;
   real32 operator () (nat32 d,const real32 * pa,const real32 * pb) const;
   
   
  /// &nbsp;
   real32 Mult() const {return mult;}
   
  /// &nbsp;
   real32 Mult(real32 m) {mult = m; return mult;}
    
   
  /// &nbsp;
   cstrconst TypeString() const;  


 private:
  real32 mult;
};

//------------------------------------------------------------------------------
/// Implimentation of manhattan distance.
class EOS_CLASS ManhattanDistance : public Distance
{
 public:
  /// &nbsp;
   ManhattanDistance():mult(1.0) {}

  /// &nbsp;
   ~ManhattanDistance();


  /// &nbsp;
   real32 operator () (nat32 d,const real32 * pa,const real32 * pb) const;
   
   
  /// &nbsp;
   real32 Mult() const {return mult;}
   
  /// &nbsp;
   real32 Mult(real32 m) {mult = m; return mult;}

   
  /// &nbsp;
   cstrconst TypeString() const;
   

 private:
  real32 mult;
};

//------------------------------------------------------------------------------
 };
};
#endif
