#ifndef EOS_MYA_DISPARITY_H
#define EOS_MYA_DISPARITY_H
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


/// \file disparity.h
/// Provides an implimentation of a disparity based data type.

#include "eos/mya/ied.h"

namespace eos
{
 namespace mya
 {
//------------------------------------------------------------------------------
/// An Ied for representing Disparity data from a stereo algorithm.
class EOS_CLASS Disparity : public Ied
{
 public:
  /// &nbsp;
  /// \param mult The multiplier to get depth, theoretically focal length multiplied by distance between cameras.
  /// \param disp The disparity map.
   Disparity(real32 mult,const svt::Field<real32> & disp);
  
  /// &nbsp;
  /// \param mult The multiplier to get depth, theoretically focal length multiplied by distance between cameras.
  /// \param disp The disparity map.
  /// \param valid A validity map, true where disparity is valid, false where it is not.
   Disparity(real32 mult,const svt::Field<real32> & disp,const svt::Field<bit> & valid);
   
  /// &nbsp; 
   ~Disparity();


  /// &nbsp;
   str::Token Type(str::TokenTable & tt) const;

  /// &nbsp;
   real32 Distance(const math::Vector<real32> & a,const math::Vector<real32> & b) const;

  /// &nbsp;
   real32 FitCost(const math::Vector<real32> & feat,const Surface & surface,real32 x,real32 y) const;


  /// &nbsp;
   cstrconst TypeString() const;
};

//------------------------------------------------------------------------------
 };
};
#endif
