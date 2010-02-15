#ifndef EOS_REND_MATERIALS_H
#define EOS_REND_MATERIALS_H
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


/// \file materials.h
/// Guess what this file does. Go on, I dare you. If you get it wrong I'll sell
/// your organs on the black market.

#include "eos/types.h"
#include "eos/rend/renderer.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
/// Yea basic material, a classical lambertian surface. Supports a single 
/// colour texture channel, albedo. If no albedo texture is set albedo is 
/// presumed a constant value of 1 for all channels.
class EOS_CLASS Lambertian : public Material
{
 public:
  /// &nbsp;
   Lambertian():albedo(null<Texture*>()) {}
   
  /// &nbsp;
   ~Lambertian() {}


  /// &nbsp;
   void BDRF(const bs::Normal & inDir,const bs::Normal & outDir,
             const Intersection & inter,bs::ColourRGB & out) const;


  /// &nbsp;
   nat32 Parameters() const;
    
  /// &nbsp;
   TextureType ParaType(nat32 index) const;
   
  /// &nbsp;
   cstrconst ParaName(nat32 index) const;
   
  /// &nbsp;
   void ParaSet(nat32 index,class Texture * rhs);

  /// &nbsp;
   cstrconst TypeString() const {return "eos::rend::Lambertian";}


 private:
  Texture * albedo;
};

//------------------------------------------------------------------------------
 };
};
#endif
