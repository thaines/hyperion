#ifndef EOS_ALG_LOCAL_PLANE_H
#define EOS_ALG_LOCAL_PLANE_H
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


/// \file local_plane.h
/// Provides a kernel density estimation approach that determines the optimal 
/// depth along a half-line and orientation of a plane given various 
/// 3D points/line segments through which it should pass.

#include "eos/types.h"
#include "eos/time/progress.h"
#include "eos/bs/geo3d.h"
#include "eos/ds/lists.h"
#include "eos/math/stats.h"

namespace eos
{
 namespace alg
 {
//------------------------------------------------------------------------------
/// This class represents a region of a sphere. Provides initialisation such 
/// that you may construct 24 regions so as to cover the entire sphere.
/// You may then subdivide regions into 4, recursivly.
/// Various strange querys may then be run.
/// This is specifically for the divide and conquer approach to finding the
/// mode of the kernel density function used by LocalPlane.
class EOS_CLASS SphereRegion
{
 public:
  /// You construct with a number, each number equates to a particular region of
  /// a spheres surface area.
  /// The sphere is divided into 24 regions, 0-11 cover the z>=0 hemisphere.
   SphereRegion(nat32 ini);

  /// &nbsp;
   ~SphereRegion();


  /// This subdivides the region into 4 sub-regions, outputing them into the 
  /// given regions.
   void Subdivide(SphereRegion & a,SphereRegion & b,SphereRegion & c,SphereRegion & d) const;

  /// Define a set of angles to include every angle that fits the following 
  /// definition: (For all directions in the region.)
  /// It is the smallest angle between a direction in the region represented
  /// and all angles that are perpendicular to the direction given to this
  /// method.
  /// This finds and outputs the minimum and maximum angles from this set.
   void PerpNormRange(const bs::Normal & n,math::Range<real32> & out) const;
   
  /// This does the same as PerpNormRange, except it takes an arc
  /// connecting two given normals rather than a single normal.
  /// Same definition as to the meaning of the output, except its now the min 
  /// out of all possible mins for all normals in the arc, and the max of the 
  /// same set.
   void PerpNormRange(const bs::Normal & start,const bs::Normal & end,math::Range<real32> & out) const;


  /// &nbsp;
   static cstrconst TypeString() {return "eos::alg::SphereRegion";}


 private:
  bs::Normal norm[4]; // 4 normals that make up the corners of the angular space polygon, anti-clockwise.
};

//------------------------------------------------------------------------------
//To Do:
//Create the range extracting methods of LocalPlane.
//Create the search procedures.

//------------------------------------------------------------------------------
/// Provides a kernel density estimation approach that determines the optimal 
/// depth along a half-line and orientation of a plane given various 
/// 3D points/line segments through which it should pass. The points/lines are 
/// weighted. This can be used for cleaning up a DSI, as every point in the DSI 
/// can be put into the calculation and it will then find the mode of the 
/// depth/orientation space. The advantage of getting orientation is not to be 
/// sniffed at either. You can also provide an orientation and obtain the optimal
/// depth, or provide a depth and get the optimal orientation.
/// There is a very close similarity between this and a generalised hough 
/// transformation, in fact you could claim this is a specialisation of a 
/// further generalisation, in the same vein as the difference between a 
/// histogram and kernel density estimation.
/// Fully supports infinity, so the plane can be infinitly far away, as much use
/// as that might be to you. The points used can be on the half line, though 
/// such points provide no orientation information.
/// Because the mirror of any normal will get the same score as the plane is not
/// orientated all output orientations are taken to have z>0.
/// For input orientations it doesn't matter, but they must be normalsied.
class EOS_CLASS LocalPlane
{
 public:
  /// &nbsp;
   LocalPlane();

  /// &nbsp;
   ~LocalPlane();


  /// This sets the k parameter of the von-mises distribution used as the kernel.
  /// Defaults to 6.
   void Set(real32 k);

  /// This sets the half-line along which the final point should be selected.
  /// It also restricts such that the plane can't lie outside the given half.
  /// Must be called before use.
   void Set(const bs::Vertex & start,const bs::Normal & direction);


  /// This adds a point to the selection of things to fit to.
   void Add(const bs::Vertex & point,real32 weight);

  /// This adds a line segment to the selection of things to fit to.
   void Add(const bs::Vertex & start,const bs::Vertex & end,real32 weight);
   
  
  /// Returns the weight for a given position/orientation position, higher is 
  /// better. Not normalised in any way, shape or form, take as relative to each
  /// other. Ths pos does not have to be on the line - that only applys for the 
  /// Mode* methods.
   real64 F(const bs::Vertex & pos,const bs::Normal & orient) const;

  
  /// Returns the range of possible values for a region of position/orientation 
  /// space, in other words the lowest possible and highest possible weights.
  /// Ushally these will be over/under estimates - this only exists as suport
  /// for the Mode* methods to narrow down the search region for the point with
  /// the highest weight.
   void RegionRange(const bs::Vertex & start,const bs::Vertex & end,
                    const SphereRegion & orient,math::Range<real32> & out) const;

  /// Same as RegionRange, except it works with a single orientation rather 
  /// than a range.
   void RegionPosRange(const bs::Vertex & start,const bs::Vertex & end,
                       const bs::Normal & orient,math::Range<real32> & out) const;
                    
  /// Same as RegionRange, except it works with a single depth rather than a
  /// range.
   void RegionOrientRange(const bs::Vertex & pos,const SphereRegion & orient,math::Range<real32> & out) const;


  /// This calculates the optimal position on the line and orientation of the 
  /// plane going through it, considering all the data avaliable.
  /// Because lines create regions there could be a region of top scoring
  /// values, in such cases it outputs the centre of the region.
  /// This is slow, incase you hadn't guessed - uses a tree search approach.
   void Mode(bs::Vertex & pos,bs::Normal & orient,time::Progress * prog = null<time::Progress*>()) const;

  /// Identical to Mode, except this returns the optimal position given that
  /// the orientation is given.
   void ModePos(bs::Vertex & pos,const bs::Normal & orient,time::Progress * prog = null<time::Progress*>()) const;

  /// Identical to Mode, except this returns the optimal orientation given that
  /// the position is given.
   void ModeOrient(const bs::Vertex & pos,bs::Normal & orient,time::Progress * prog = null<time::Progress*>()) const;


  /// &nbsp;
   static cstrconst TypeString() {return "eos::alg::LocalPlane";}


 private:
  // Storage structures...
   struct PointSample
   {
    bs::Vertex point;
    real32 weight;
   };
   
   struct LineSample
   {
    bs::Vertex start;
    bs::Vertex end;
    real32 weight;
   };

  // Storage...
   real32 k;
   bs::Vertex start;
   bs::Normal dir;
   ds::List<PointSample> psl;
   ds::List<LineSample> lsl;
   
  // Suport stuff for mode finding...

  // Misc...
   static const nat32 searchDepth = 8; // The maximum depth of the subdivision scheme used to find modes.
   static const nat32 startPosSubdiv = 12; // Starting number of subdivisions in space, orientation allways starts as 12.
};

//------------------------------------------------------------------------------
 };
};
#endif
