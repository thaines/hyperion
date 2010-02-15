#ifndef EOS_REND_LIGHTS_H
#define EOS_REND_LIGHTS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file lights.h
/// Provides objects that feed light into a scene. These are obviously only 
/// trully relevent for non-global illumination methods, but it is expected that
/// global illumination methods will support these as well.

#include "eos/types.h"
#include "eos/rend/renderer.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
/// A simple infinitly distant light source, simply uses a source direction
/// and a strength for each colour component.
class EOS_CLASS InfiniteLight : public Light
{
 public:
  /// &nbsp;
   InfiniteLight():col(1.0,1.0,1.0),toLight(0.0,0.0,1.0) {}
   
  /// &nbsp;
   ~InfiniteLight() {}

   
  /// You set the lights position in terms of a normalised vector that points 
  /// to the light source. Defaults to comming from the +ve z axis,
  /// i.e. behind the default camera position.
   bs::Normal & Dir() {return toLight;}

  /// The light source strength, for each of the 3 components.
  /// Defaults to a 1 light strength unit white light, i.e. (1,1,1).
   bs::ColourRGB & Strength() {return col;}


  /// &nbsp;
   void Calc(const bs::Normal & outDir,const Intersection & inter,const Material & mat,
             const class RenderableDB & db,bs::ColourRGB & out) const;


  /// &nbsp;
   cstrconst TypeString() const {return "eos::rend::InfiniteLight";}


 private:
  bs::ColourRGB col;
  bs::Normal toLight;
};

//------------------------------------------------------------------------------
 };
};
#endif
