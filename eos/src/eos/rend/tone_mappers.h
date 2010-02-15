#ifndef EOS_REND_TONE_MAPPERS_H
#define EOS_REND_TONE_MAPPERS_H
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


/// \file tone_mappers.h
/// Provides tone mappers. These convert a collection of captured rays as 
/// assigned to pixels into an actual image. There main job is in mapping the
/// arbitarily large light levels in the ray image to the limited range
/// avaliable in an image.

#include "eos/types.h"
#include "eos/rend/renderer.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
/// This rather primative tone mapping object linearly scales the brightness 
/// values to put them in range. Has two modes, in the first it makes the 
/// largest irradiance value equate to 1 in the image, in the second it equates 
/// the mean value in the image to the mean value of the irradiance values.
class EOS_CLASS ToneScaler : public ToneMapper
{
 public:
  /// Set meanMode to true to equate means, otherwise it uses the simple 
  /// maximum to one model.
   ToneScaler(bit meanMode = false):meanM(meanMode) {}
   
  /// &nbsp;
  ~ToneScaler() {} 


  /// Call with true to use the mean mode, false to use the maximum to one.
   void SetMode(bit meanMode) {meanM = meanMode;}

  /// Returns true if using the mean mode, false if using the maximum to one mode.
   bit GetMode() const {return meanM;}


  /// &nbsp;
   void Apply(const RayImage & ri,svt::Field<bs::ColourRGB> & out,time::Progress * prog) const;


  /// &nbsp;
   cstrconst TypeString() const {return "eos::rend::ToneScaler";}


 private:
  bit meanM;
};

//------------------------------------------------------------------------------
 };
};
#endif
