#ifndef EOS_REND_DATABASES_H
#define EOS_REND_DATABASES_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file databases.h
/// Provides implimentations of the RenderableDB type.

#include "eos/rend/renderer.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
/// This is a very simple brute force object database - it simply tests each 
/// ray against every object in the scene. It does test against each objects 
/// bounding sphere first however, so its not completly stupid. Suitable for scenes 
/// with single digit object counts.
class EOS_CLASS BruteDB : public RenderableDB
{
 public:
  /// &nbsp;
   BruteDB();
 
  /// &nbsp; 
   ~BruteDB();


  /// &nbsp;
   void Add(Renderable * rb);


  /// &nbsp;
   void Prepare(time::Progress * prog = null<time::Progress*>());
   
  /// &nbsp;
   void Unprepare(time::Progress * prog = null<time::Progress*>());


  /// &nbsp;
   void Inside(const bs::Vert & point,ds::List<Renderable*> & out) const;
  
  /// &nbsp;
   bit Intercept(const bs::Ray & ray,Renderable *& objOut,Intersection & intOut) const;
  
  /// &nbsp;
   bit Intercept(const bs::FiniteLine & line,Renderable *& objOut,Intersection & intOut) const;  


  /// &nbsp;
   cstrconst TypeString() const;

 
 private:
  struct Node
  {
   Renderable * rend;
   bs::Sphere sphere;
  };
  ds::List<Node> data;
};

//------------------------------------------------------------------------------
 };
};
#endif
