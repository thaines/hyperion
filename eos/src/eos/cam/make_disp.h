#ifndef EOS_CAM_MAKE_DISP_H
#define EOS_CAM_MAKE_DISP_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

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


/// \file make_disp.h
/// Given a camera pair this class eats triangles, when you decide its full it
/// outputs a disparity map (with mask) created by rendering the triangles.

#include "eos/types.h"

#include "eos/bs/geo3d.h"
#include "eos/ds/arrays2d.h"
#include "eos/svt/field.h"
#include "eos/cam/files.h"


namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------
/// This class is given a camera pair, it then becomes a triangle eatter, you 
/// keep feeding it triangles which it uses to render a disparity map. When done
/// you can extract the disparity map with a mask - this is used to generate
/// ground truth for a scene.
/// This is basically a scanline renderer, except it renders disparity rather
/// than anything remotly normal.
class EOS_CLASS MakeDisp
{
 public:
  /// If using this you must call Reset with a pair before use.
   MakeDisp();

  /// You provide the rectified pair that you want the disparity map for.
  /// The output will be the dimensions of the left image as given in the pair.
   MakeDisp(const CameraPair & pair);
   
  /// &nbsp;
   ~MakeDisp();


  /// Call this to reset all data.  
   void Reset(const CameraPair & pair);


  /// Adds a triangle to the rendering - provide the 3 corner coordinates.
   void Add(const bs::Vert & a,const bs::Vert & b,const bs::Vert & c);


  /// Width of output, for conveniance.
   nat32 Width() const;

  /// Height of output, for conveniance.
   nat32 Height() const;

  /// Returns a disparity for a given point.
   real32 Disp(nat32 x,nat32 y) const;

  /// Returns a true if the disparity for a given point is valid.
   bit Mask(nat32 x,nat32 y) const;

  /// Extract the disparity map. Must be the correct dimensions.
   void GetDisp(svt::Field<real32> & out) const;

  /// Extract the disparity mask, it will be true where the disparity map is
  /// valid. Must be the correct dimensions.
   void GetMask(svt::Field<bit> & out) const;


  /// &nbsp;
   static cstrconst TypeString() {return "eos::cam::MakeDisp";}


 private:
  CameraPair pair;

  math::Mat<3,3,real64> rectLeft;  // So we don't need to re-calculate them every triangle.
  math::Mat<3,3,real64> rectRight; // "  
  math::Vect<4,real64> centreLeft; // "
  math::Mat<4,3,real64> invLP;     // "
  bs::Vert dirLeft;                // "
  

  ds::Array2D<real32> disp;
  ds::Array2D<real32> depth; // Depth map, used to save on SVD's.
  ds::Array2D<bit> mask; // True where disparity is valid.
  
  // Helper method - simply a wrapper around the Project method of CameraPair,
  // for conveniance.
   void Project(bs::Vert & loc,bs::Pnt & out,real32 * disp = null<real32*>());
};

//------------------------------------------------------------------------------
 };
};
#endif
