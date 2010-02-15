#ifndef EOS_REND_BACKGROUNDS_H
#define EOS_REND_BACKGROUNDS_H
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
