#ifndef EOS_CAM_HOMOGRAPHY_H
#define EOS_CAM_HOMOGRAPHY_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file homography.h
/// Provides a class for estimating 2D homographys using a set of point pairs.

#include "eos/types.h"
#include "eos/ds/lists.h"
#include "eos/math/vectors.h"
#include "eos/math/matrices.h"
#include "eos/math/mat_ops.h"

namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------
/// This class collects a set of homogenius point pairs, it then, on request, 
/// produces a homography mapping the first point to the second point in each
/// pair using a best fit to minimise manhatton distance between points. 
/// (That is it produces a maximum likelihood answer.) (Note that it dosn't like
/// points at infinity, as they tend to produce infinite costs, so don't give it
/// any.)
/// Uses SVD followed by LM to produce its answer. A slow implimentation, but
/// should produce very good results - the sampson method.
/// Includes normalisation.
class EOS_CLASS Homography2D
{
 public:
  /// &nbsp;
   Homography2D();
   
  /// &nbsp;
   ~Homography2D();
   
  /// Removes all current data so it can be reused.
   void Reset();


  /// Adds a point pair, non-homogenous.
   void Add(const math::Vect<2,real32> & f,const math::Vect<2,real32> & s);

  /// Adds a point pair, non-homogenous.
   void Add(const math::Vect<2,real64> & f,const math::Vect<2,real64> & s);

  /// Adds a point pair, homogenous.
   void Add(const math::Vect<3,real32> & f,const math::Vect<3,real32> & s);

  /// Adds a point pair, homogenous.
   void Add(const math::Vect<3,real64> & f,const math::Vect<3,real64> & s);


  /// Extracts the result, a matrix such that for each pair s = Mf, or at 
  /// least a best fit to this constraint. Note that before calling this Add
  /// must be called at least 4 times, more if there is degenerate data.
  /// Returns the residual of the result.
   real64 Result(math::Mat<3,3,real32> & out);

  /// Extracts the result, a matrix such that for each pair s = Mf, or at 
  /// least a best fit to this constraint. Note that before calling this Add
  /// must be called at least 4 times, more if there is degenerate data.
  /// Returns the residual of the result.
   real64 Result(math::Mat<3,3,real64> & out);


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::cam::Homography2D";}


 private:  
  // Linked list, used to store all the collected pairs.
   ds::List< Pair<math::Vect<2,real64>,math::Vect<2,real64> > > data;
   
  // Error metric function for LM.
   static void LMfunc(const math::Vector<real64> & pv,math::Vector<real64> & err,const Homography2D & self);
};

//------------------------------------------------------------------------------
 };
};
#endif
