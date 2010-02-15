#ifndef EOS_REND_RENDERERS_H
#define EOS_REND_RENDERERS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file renderers.h
/// Provides actual renderers, just one component in an entire renderer.
/// (Note plurality.)
/// These provide the ray casting schedule whilst other modules provide the 
/// things the rays hit. The exception is that each light provides its own 
/// casting schedule for each illumination task.

#include "eos/types.h"

#include "eos/rend/renderer.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
/// The most basic raytracing schedule possible really, it casts a single ray
/// and then reports an irradiance that is simply the sum of that produced by
/// all the light sources, which themselves will apply the BDRF. Suports nothing
/// special, no transparency, no reflections - if there not defined as dull 
/// solid objects on input they will certainly look like dull, solid objects
/// when rendered.
class EOS_CLASS OneHit : public Renderer
{
 public:
  /// &nbsp;
   OneHit() {}
   
  /// &nbsp;
   ~OneHit() {}
  
  /// &nbsp;
   void Cast(TaggedRay & ray,const RenderableDB & db,const ds::List<Renderable*,
             mem::KillDel<Light> > & inside,const ds::List<Light*> & ll,const Background & bg) const;
  
  /// &nbsp;
   cstrconst TypeString() const {return "eos::rend::OneHit";}
};

//------------------------------------------------------------------------------
 };
};
#endif
