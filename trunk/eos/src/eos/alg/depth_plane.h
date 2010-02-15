#ifndef EOS_ALG_DEPTH_PLANE_H
#define EOS_ALG_DEPTH_PLANE_H
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


/// \file depth_plane.h
/// Provide a class for fitting planes with a pre-set orientation through a set 
/// of points.

#include "eos/types.h"
#include "eos/bs/geo3d.h"
#include "eos/ds/lists.h"
#include "eos/time/progress.h"

namespace eos
{
 namespace alg
 {
//------------------------------------------------------------------------------
/// You provide this with an orientation specified with a 3-vector and a bunch 
/// of homogenous coordinates. It then finds the best fit plane through
/// the points under the constraint that it must be perpendicular to the 
/// orientation given, in other words it finds the correct depth for the plane.
/// It does this using 1D kernel density estimation, making it extremely robust.
class EOS_CLASS DepthPlane
{
 public:
  /// &nbsp;
   DepthPlane();
 
  /// &nbsp;
   ~DepthPlane();


  /// Sets the planes final orientation. Must be normalsied.
   void Set(const bs::Normal & norm);

  /// Adds a coordinate to the set to be fitted against.
   void Add(const bs::Vertex & pos);


  /// Calculates an answer.
   void Run();


  /// Obtain the answer.
   const bs::Plane & Plane() const;


  /// &nbsp;
   static cstrconst TypeString(); 


 private:
  bs::Normal norm;
  ds::List<bs::Vertex> list;
  
  bs::Plane result;
};

//------------------------------------------------------------------------------
 };
};
#endif
