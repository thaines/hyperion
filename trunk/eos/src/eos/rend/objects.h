#ifndef EOS_REND_OBJECTS_H
#define EOS_REND_OBJECTS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file objects.h
/// Provides the objects a scene is constructed from.

#include "eos/types.h"
#include "eos/rend/renderer.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
/// A sphere. Does a very good impression of a cabbage on tuesdays. 
/// Radius 1, positioned at (0,0,0), as you would expect.
/// Has no options.
class EOS_CLASS Sphere : public Object
{
 public:
  /// &nbsp;
   Sphere():velocity(0.0,0.0,0.0) {}
   
  /// &nbsp;
   ~Sphere() {}


  /// &nbsp;
   void Bound(bs::Sphere & out) const;
   
  /// &nbsp;
   void Bound(bs::Box & out) const;


  /// &nbsp;
   bit Inside(const bs::Vert & point) const;
   
  /// &nbsp;
   bit Intercept(const bs::Ray & ray,real32 & dist) const;

  /// &nbsp;
   bit Intercept(const bs::Ray & ray,Intersection & out) const;
   
  /// &nbsp;
   bit Intercept(const bs::FiniteLine & line) const;


  /// &nbsp;
   nat32 Materials() const {return 1;}


  /// &nbsp;
   cstrconst TypeString() const {return "eos::rend::Sphere";}
   
   
 /// The rather strange and only parameter. Defaults to none.
 /// Simply passed through the system so motion blur can be approximated
 /// if need be.
  bs::Vert velocity;
};

//------------------------------------------------------------------------------
 };
};
#endif
