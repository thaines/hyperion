#ifndef EOS_REND_BACKGROUNDS_H
#define EOS_REND_BACKGROUNDS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file backgrounds.h
/// Provides background objects, the special object used when a ray misses 
/// everything else.

#include "eos/types.h"
#include "eos/rend/renderer.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
/// It doesn't get simpler than this - sets a solid background colour.
class EOS_CLASS SolidBackground : public Background
{
 public:
  /// &nbsp;
   SolidBackground(const bs::ColourRGB & bg = bs::ColourRGB(0.0,0.0,0.0)):col(bg) {}
   
  /// &nbsp;
   ~SolidBackground() {}


  /// &nbsp;
   void SetColour(const bs::ColourRGB & bg) {col = bg;}

  /// &nbsp;
   const bs::ColourRGB & GetColour() const {return col;}


  /// &nbsp;
   void Calc(const bs::Ray &,bs::ColourRGB & out) const {out = col;}


  /// &nbsp;
   cstrconst TypeString () const {return "eos::rend::SolidBackground";}


 private:
  bs::ColourRGB col;
};

//------------------------------------------------------------------------------
 };
};
#endif
