#ifndef EOS_ALG_FITTING_H
#define EOS_ALG_FITTING_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file fitting.h
/// Provides algorithms for fitting data sets to models, this so far
/// specifically equates to fitting a plane to a set of points.

#include "eos/types.h"
#include "eos/math/matrices.h"
#include "eos/bs/geo3d.h"
#include "eos/ds/arrays.h"

namespace eos
{
 namespace alg
 {
//------------------------------------------------------------------------------
/// A simple class, you call a method with each point, and at any time can then
/// call another method to extract the plane that best fits in a least squares
/// way with the data so far entered.
class EOS_CLASS PlaneFit
{
 public:
  /// &nbsp;
   PlaneFit();

  /// &nbsp;
   ~PlaneFit();

  /// Resets the class as though no data has been fed to it.
   void Reset();

  /// Adds a data point to fit the plane to.
   void Add(const math::Vect<3> & vertex);

  /// Adds a data point to fit the plane to.
   void Add(real32 x,real32 y,real32 z);

  /// Returns true if enough data to fit a plane has been provided.
   bit Valid();

  /// Outputs the plane, only call this if valid is true.
   void Get(bs::PlaneABC & out);


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::alg::PlaneFit";}


 private:
  math::Mat<3,3,real64> mat;
  math::Vect<3,real64> vec;
};

//------------------------------------------------------------------------------
/// An advanced version of PlaneFit. This version uses weighted line segments
/// through which the plane has to fit. Not actually any different, except it 
/// fits the centres of the line segments and weights the costs based on the 
/// length of the line segment. It additionally 'fits' a gaussian model to the 
/// points, and weights points by there probability within the model.
/// Essentially, this is PlaneFit with clever weighting so as to minimise the 
/// affect of outliers and unreliable matches.
class EOS_CLASS LinePlaneFit
{
 public:
  /// &nbsp;
   LinePlaneFit();
   
  /// &nbsp;
   ~LinePlaneFit();


  /// Adds a line segment to be fitted to.
   void Add(const bs::Vertex & a,const bs::Vertex & b,real32 weight);
   
  /// Does the fitting.
   void Run();

  /// Returns the fitted plane.
   const bs::Plane & Plane() const;


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::alg::LinePlaneFit";}


 private:
  struct Entry
  {
   bs::Vert c;
   real32 length; // Length of line segment that was.
   real32 weight;
  };
  
  ds::List<Entry> data;
  
  bs::Plane result;
};

//------------------------------------------------------------------------------
/// A strange little function - given two vectors where one is a noisy multiple
/// of the other this returns the multiplier that multiplies the non-noisy to
/// get to the noisy, so as to minimise error as the sum of absolute differences.
template <typename V>
inline typename V::type VectorScaleDiff(const V & noisy,const V & toMult)
{
 log::Assert(noisy.Size()==toMult.Size());
 
 typename V::type nSum = static_cast<typename V::type>(0);
 typename V::type mSum = static_cast<typename V::type>(0);
 for (nat32 i=0;i<noisy.Size();i++)
 {
  nSum += noisy[i];
  mSum += toMult[i];
 }

 if (math::IsZero(mSum)) return static_cast<typename V::type>(0);
 return nSum/mSum;
}

//------------------------------------------------------------------------------
 };
};
#endif
