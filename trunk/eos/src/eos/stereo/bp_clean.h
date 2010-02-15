#ifndef EOS_STEREO_BP_CLEAN_H
#define EOS_STEREO_BP_CLEAN_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file bp_clean.h
/// Cleans disparity maps using belief propagation. Additionally useful as it
/// outputs continuous output even if given discreet input, i.e. it interpolates.

#include "eos/types.h"
#include "eos/svt/field.h"
#include "eos/bs/colours.h"
#include "eos/stereo/dsi.h"
#include "eos/ds/arrays2d.h"
#include "eos/time/progress.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// This uses a belief propagation approach to smooth a disparity map, whilst
/// maintaining detail through various bodges. Not a well thought out algorithm,
/// and it could most certainly be improved, but for making the disparity map
/// output by a stereo process look good it works fine.
/// (Takes a DSI as input but produces a disparity map as output, so it also
/// handles that problem. Note however that it can not handle large numbers of
/// disparity assignments to a single pixel, as it models them internally as a
/// single Gaussian.)
class EOS_CLASS CleanDSI
{
 public:
  /// &nbsp;
   CleanDSI();

  /// &nbsp;
   ~CleanDSI();


  /// Sets the left image, used for weighting the smoothing.
   void Set(const svt::Field<bs::ColourLuv> & image);

  /// Sets the input DSI.
   void Set(const DSI & dsi);
   
  /// Optionally sets a mask on the disparity map - only areas where its true
  /// have calculations done.
   void SetMask(const svt::Field<bit> & mask);
   
  /// Optionally sets an input field of standard deviations, which then override
  /// its internal calculation system.
   void SetSD(const svt::Field<real32> & sd);

  /// Sets the parameters.
  /// \param strength How strong the smoothing is, an inverse standard deviation
  ///                 on the confidence of two adjacent disparities being identical.
  ///                 Defaults to 1.
  /// \param cutoff Cutoff in eucledian colour distance for smoothing to stop.
  ///               Defaults to 16.
  /// \param width Cutoff is done with a sigmoid, this is its 'half-life'.
  ///              Defaults to 2.
  /// \param iters Number of iterations of bp to do. Defaults to 100.
   void Set(real32 strength,real32 cutoff,real32 width,nat32 iters = 100);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Wibble.
   nat32 Width() const;

  /// Wobble.
   nat32 Height() const;

  /// For sampling individual values.
  /// Will return infinity for bad values, for which no estimate exists.
   real32 Get(nat32 x,nat32 y) const;

  /// Extracts a disparity map. Remember that infinite values can appear.
   void GetMap(svt::Field<real32> & out);


  /// &nbsp;
   static cstrconst TypeString() {return "eos::stereo::CleanDSI";}


 private:
  // Input...
   svt::Field<bs::ColourLuv> image;
   const DSI * dsi;
   
   svt::Field<bit> mask;
   svt::Field<real32> sdOverride;
   

   real32 strength;
   real32 cutoff;
   real32 width;
   nat32 iters;

  // Output...
   ds::Array2D<real32> out;
};

//------------------------------------------------------------------------------
 };
};
#endif
