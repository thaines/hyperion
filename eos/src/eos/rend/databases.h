#ifndef EOS_REND_DATABASES_H
#define EOS_REND_DATABASES_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


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
