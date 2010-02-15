#ifndef EOS_REND_SAMPLERS_H
#define EOS_REND_SAMPLERS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file samplers.h
/// Provides samplers. These define the anti-aliasing model and drive the entire
/// render, also provide progress reports.

#include "eos/types.h"
#include "eos/rend/renderer.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
/// This is a standard grid based anti-aliasing arrangment, you specify how many
/// rays to divide each pixel into on each axis and it then fires that many rays
/// squared for each pixel. Reports progress on a pixel by pixel basis.
class EOS_CLASS GridAA : public Sampler
{
 public:
  /// &nbsp;
   GridAA(nat32 dimSamples = 1):dimSamps(dimSamples) {}

  /// &nbsp;
   ~GridAA() {}


  /// Sets the samples per dimension.
   void SetDimSamples(nat32 dimSamples) {dimSamps = dimSamples;}
   
  /// Returns the samples per dimension.
   nat32 GetDimSamples() const {return dimSamps;}


  /// Samples per dimension squared - theres two dimensions in an image!
   nat32 Samples() const {return math::Sqr(dimSamps);}

  /// &nbsp;
   void Render(Job & job,time::Progress * prog);


  /// &nbsp;
   cstrconst TypeString() const {return "eos::rend::GridAA";}


 private:
  nat32 dimSamps;
};

//------------------------------------------------------------------------------
 };
};
#endif
