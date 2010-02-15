#ifndef EOS_STEREO_DSI_H
#define EOS_STEREO_DSI_H
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


/// \file dsi.h
/// Provides a standard dsi interface.

#include "eos/types.h"
#include "eos/bs/colours.h"
#include "eos/ds/arrays.h"
#include "eos/ds/arrays2d.h"
#include "eos/svt/field.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// This defines an uncalculated DSI (Disparity-Spatial Cost),
/// this is simply a class capable of giving the matching cost of any two pixels,
/// it can't help with finding the lowest cost/most consistant set of costs.
/// Used as input to algorithms that then select which pixel pairs to sample.
/// Supports hierachical methods, but for efficiency reasons requires you to
/// supply your own blocks of memory, as a new/delete for every pixel in an
/// image would not be a particularly good use of my time.
class EOS_CLASS DSC : public Deletable
{
 public:
  /// &nbsp;
   ~DSC() {}


  /// As its passed arround making Clones is a desired capability.
   virtual DSC * Clone() const = 0;


  /// Returns how many bytes a value representation takes up, for using with all
  /// the byte pointers that get passed into an implimentor of this interface.
   virtual nat32 Bytes() const = 0;

  /// Given two value representations this returns the cost of matching those
  /// two values.
   virtual real32 Cost(const byte * left,const byte * right) const = 0;

  /// Given two value representations this must output the 'joining' of those values.
  /// This is often just an average of the two values.
  /// It must be safe for out it be ==a or ==b.
  /// (Note that because its for hierachy use a regular pattern is assumed, so
  /// we presume that each side has the same weight, hence its not a weighted
  /// average! This saves 4 bytes per pixel. Thats a good thing.)
   virtual void Join(const byte * left,const byte * right,byte * out) const = 0;

  /// More advanced version of Join - this takes a number of pointers, and any and all
  /// pointers can be null. Should merge them with equal weighting.
  /// If all input pointers are null it has no obligation to the state of out.
  /// Whilst the in bytes can not be touched the method is allowed to screw with
  /// the array of pointers to these bytes, as it feels fit, but must leave it
  /// as it found it, i.e. with the original pointers. (This is conveniant for
  /// offsetting.)
   virtual void Join(nat32 n,const byte ** in,byte * out) const = 0;


  /// Returns the width of a, the user is not allowed to query outside of [0,WidthA()-1].
   virtual nat32 WidthLeft() const = 0;

  /// Returns the height of a, the user is not allowed to query outside of [0,HeightA()-1].
   virtual nat32 HeightLeft() const = 0;

  /// Writes a new value representation into the given data block, for the
  /// coordinate given.
   virtual void Left(nat32 x,nat32 y,byte * out) const = 0;


  /// Returns the width of b, the user is not allowed to query outside of [0,WidthB()-1].
   virtual nat32 WidthRight() const = 0;

  /// Returns the height of b, the user is not allowed to query outside of [0,HeightB()-1].
   virtual nat32 HeightRight() const = 0;

  /// Writes a new value representation into the given data block, for the
  /// coordinate given.
   virtual void Right(nat32 x,nat32 y,byte * out) const = 0;


  /// This returns the cost of matching the two given pixels, higher is bad, 0 is
  /// exceptionally good. Provided for optimisation purposes, as this can be
  /// defined in terms of Left(..)/Right(...), but that involves heap bashing.
   virtual real32 Cost(nat32 leftX,nat32 rightX,nat32 y) const;


  /// &nbsp;
   virtual cstrconst TypeString() const = 0;
};

//------------------------------------------------------------------------------
/// Defines a DSC as simply the difference between two fields of reals.
/// For joining it uses a simple average.
class EOS_CLASS DifferenceDSC : public DSC
{
 public:
  /// &nbsp;
   DifferenceDSC(const svt::Field<real32> & left,const svt::Field<real32> & right,real32 mult = 1.0);

  /// &nbsp;
   ~DifferenceDSC();

  /// &nbsp;
   DSC * Clone() const;


  /// &nbsp;
   nat32 Bytes() const;

  /// &nbsp;
   real32 Cost(const byte * left,const byte * right) const;

  /// &nbsp;
   void Join(const byte * left,const byte * right,byte * out) const;

  /// &nbsp;
   void Join(nat32 n,const byte ** in,byte * out) const;


  /// &nbsp;
   nat32 WidthLeft() const;

  /// &nbsp;
   nat32 HeightLeft() const;

  /// &nbsp;
   void Left(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   nat32 WidthRight() const;

  /// &nbsp;
   nat32 HeightRight() const;

  /// &nbsp;
   void Right(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   real32 Cost(nat32 leftX,nat32 rightX,nat32 y) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  svt::Field<real32> left;
  svt::Field<real32> right;
  real32 mult;
};

//------------------------------------------------------------------------------
/// Defines a DSC as simply the difference between two fields of reals.
/// It uses areas, initialising each region as the min/max of the 1 pixel square
/// centered on the requested region.
/// Cost is the difference between the closest points, if the regions overlap it will be zero.
/// Joining is than the union of bounding regions.
class EOS_CLASS BoundDifferenceDSC : public DSC
{
 public:
  /// &nbsp;
   BoundDifferenceDSC(const svt::Field<real32> & left,const svt::Field<real32> & right,real32 mult = 1.0);

  /// &nbsp;
   ~BoundDifferenceDSC();

  /// &nbsp;
   DSC * Clone() const;


  /// &nbsp;
   nat32 Bytes() const;

  /// &nbsp;
   real32 Cost(const byte * left,const byte * right) const;

  /// &nbsp;
   void Join(const byte * left,const byte * right,byte * out) const;

  /// &nbsp;
   void Join(nat32 n,const byte ** in,byte * out) const;


  /// &nbsp;
   nat32 WidthLeft() const;

  /// &nbsp;
   nat32 HeightLeft() const;

  /// &nbsp;
   void Left(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   nat32 WidthRight() const;

  /// &nbsp;
   nat32 HeightRight() const;

  /// &nbsp;
   void Right(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   real32 Cost(nat32 leftX,nat32 rightX,nat32 y) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  svt::Field<real32> left;
  svt::Field<real32> right;
  real32 mult;
};

//------------------------------------------------------------------------------
/// Defines a DSC in terms of the Luv colour space, using euclidian distance for costs.
/// Uses averages.
/// Also suports the setting of a cost cap.
class EOS_CLASS LuvDSC : public DSC
{
 public:
  /// &nbsp;
   LuvDSC(const svt::Field<bs::ColourLuv> & left,const svt::Field<bs::ColourLuv> & right,real32 mult = 1.0,real32 cap = math::Infinity<real32>());

  /// &nbsp;
   ~LuvDSC();

  /// &nbsp;
   DSC * Clone() const;


  /// &nbsp;
   nat32 Bytes() const;

  /// &nbsp;
   real32 Cost(const byte * left,const byte * right) const;

  /// &nbsp;
   void Join(const byte * left,const byte * right,byte * out) const;

  /// &nbsp;
   void Join(nat32 n,const byte ** in,byte * out) const;


  /// &nbsp;
   nat32 WidthLeft() const;

  /// &nbsp;
   nat32 HeightLeft() const;

  /// &nbsp;
   void Left(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   nat32 WidthRight() const;

  /// &nbsp;
   nat32 HeightRight() const;

  /// &nbsp;
   void Right(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   real32 Cost(nat32 leftX,nat32 rightX,nat32 y) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  svt::Field<bs::ColourLuv> left;
  svt::Field<bs::ColourLuv> right;
  real32 mult;
  real32 cap;
};

//------------------------------------------------------------------------------
/// Defines a DSC in terms of the Luv colour space, using euclidian distance for costs.
/// Uses averages.
/// Returns the square of the distance, see LuvDSC to get it straight.
/// Also suports the setting of a cost cap.
class EOS_CLASS SqrLuvDSC : public DSC
{
 public:
  /// &nbsp;
   SqrLuvDSC(const svt::Field<bs::ColourLuv> & left,const svt::Field<bs::ColourLuv> & right,real32 mult = 1.0,real32 cap = math::Infinity<real32>());

  /// &nbsp;
   ~SqrLuvDSC();

  /// &nbsp;
   DSC * Clone() const;


  /// &nbsp;
   nat32 Bytes() const;

  /// &nbsp;
   real32 Cost(const byte * left,const byte * right) const;

  /// &nbsp;
   void Join(const byte * left,const byte * right,byte * out) const;

  /// &nbsp;
   void Join(nat32 n,const byte ** in,byte * out) const;


  /// &nbsp;
   nat32 WidthLeft() const;

  /// &nbsp;
   nat32 HeightLeft() const;

  /// &nbsp;
   void Left(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   nat32 WidthRight() const;

  /// &nbsp;
   nat32 HeightRight() const;

  /// &nbsp;
   void Right(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   real32 Cost(nat32 leftX,nat32 rightX,nat32 y) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  svt::Field<bs::ColourLuv> left;
  svt::Field<bs::ColourLuv> right;
  real32 mult;
  real32 cap;
};

//------------------------------------------------------------------------------
/// Defines a DSC in terms of the Luv colour space, using manhatten distance for costs.
/// Uses averages.
/// Also suports the setting of a cost cap.
class EOS_CLASS ManLuvDSC : public DSC
{
 public:
  /// &nbsp;
   ManLuvDSC(const svt::Field<bs::ColourLuv> & left,const svt::Field<bs::ColourLuv> & right,real32 mult = 1.0,real32 cap = math::Infinity<real32>());

  /// &nbsp;
   ~ManLuvDSC();

  /// &nbsp;
   DSC * Clone() const;


  /// &nbsp;
   nat32 Bytes() const;

  /// &nbsp;
   real32 Cost(const byte * left,const byte * right) const;

  /// &nbsp;
   void Join(const byte * left,const byte * right,byte * out) const;

  /// &nbsp;
   void Join(nat32 n,const byte ** in,byte * out) const;


  /// &nbsp;
   nat32 WidthLeft() const;

  /// &nbsp;
   nat32 HeightLeft() const;

  /// &nbsp;
   void Left(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   nat32 WidthRight() const;

  /// &nbsp;
   nat32 HeightRight() const;

  /// &nbsp;
   void Right(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   real32 Cost(nat32 leftX,nat32 rightX,nat32 y) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  svt::Field<bs::ColourLuv> left;
  svt::Field<bs::ColourLuv> right;
  real32 mult;
  real32 cap;
};

//------------------------------------------------------------------------------
/// Defines a DSC in terms of the Luv colour space, using euclidian distance for costs.
/// Uses a 3 dimensional bounding box in Luv space, with bounding box union and
/// horizontal 1 square initialisation. i.e. +/- 0.5 on the X axis but not on the Y.
/// This matches the Scale Invariant Pixel matching function thingamygig.
/// If you want the +/- 0.5 on the Y axis see BoundLuvDSC
/// Cost is the difference between the closest points, if the regions overlap it will be zero.
/// Also suports the setting of a cost cap.
class EOS_CLASS HalfBoundLuvDSC : public DSC
{
 public:
  /// &nbsp;
   HalfBoundLuvDSC(const svt::Field<bs::ColourLuv> & left,const svt::Field<bs::ColourLuv> & right,real32 mult = 1.0,real32 cap = math::Infinity<real32>());

  /// &nbsp;
   ~HalfBoundLuvDSC();

  /// &nbsp;
   DSC * Clone() const;


  /// Sets the multiplier, incase you want to change it.
   void SetMult(real32 mult);

  /// Sets a cap on how high the cost can be, Defaults to infinity, i.e. no limit.
   void SetCap(real32 cap);


  /// &nbsp;
   nat32 Bytes() const;

  /// &nbsp;
   real32 Cost(const byte * left,const byte * right) const;

  /// &nbsp;
   void Join(const byte * left,const byte * right,byte * out) const;

  /// &nbsp;
   void Join(nat32 n,const byte ** in,byte * out) const;


  /// &nbsp;
   nat32 WidthLeft() const;

  /// &nbsp;
   nat32 HeightLeft() const;

  /// &nbsp;
   void Left(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   nat32 WidthRight() const;

  /// &nbsp;
   nat32 HeightRight() const;

  /// &nbsp;
   void Right(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   real32 Cost(nat32 leftX,nat32 rightX,nat32 y) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  svt::Field<bs::ColourLuv> left;
  svt::Field<bs::ColourLuv> right;
  real32 mult;
  real32 cap;
};

//------------------------------------------------------------------------------
/// Defines a DSC in terms of the Luv colour space, using euclidian distance for costs.
/// Uses a 3 dimensional bounding box in Luv space, with bounding box union and
/// 1 square initialisation. (i.e. +/- 0.5 on the X axis and on the Y axis.
/// Cost is the difference between the closest points, if the regions overlap it will be zero.
/// Also suports the setting of a cost cap.
class EOS_CLASS BoundLuvDSC : public DSC
{
 public:
  /// &nbsp;
   BoundLuvDSC(const svt::Field<bs::ColourLuv> & left,const svt::Field<bs::ColourLuv> & right,real32 mult = 1.0,real32 cap = math::Infinity<real32>());

  /// &nbsp;
   ~BoundLuvDSC();

  /// &nbsp;
   DSC * Clone() const;


  /// Sets the multiplier, incase you want to change it.
   void SetMult(real32 mult);

  /// Sets a cap on how high the cost can be, Defaults to infinity, i.e. no limit.
   void SetCap(real32 cap);


  /// &nbsp;
   nat32 Bytes() const;

  /// &nbsp;
   real32 Cost(const byte * left,const byte * right) const;

  /// &nbsp;
   void Join(const byte * left,const byte * right,byte * out) const;

  /// &nbsp;
   void Join(nat32 n,const byte ** in,byte * out) const;


  /// &nbsp;
   nat32 WidthLeft() const;

  /// &nbsp;
   nat32 HeightLeft() const;

  /// &nbsp;
   void Left(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   nat32 WidthRight() const;

  /// &nbsp;
   nat32 HeightRight() const;

  /// &nbsp;
   void Right(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   real32 Cost(nat32 leftX,nat32 rightX,nat32 y) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  svt::Field<bs::ColourLuv> left;
  svt::Field<bs::ColourLuv> right;
  real32 mult;
  real32 cap;
};

//------------------------------------------------------------------------------
/// Defines a DSC in terms of the Luv colour space, using euclidian distance for costs.
/// Uses a 3 dimensional bounding box in Luv space, with bounding box union and
/// 1 square initialisation. (i.e. +/- 0.5 on the X axis and on the Y axis.
/// Cost is the difference between the closest points, if the regions overlap it will be zero.
/// Also suports the setting of a cost cap.
/// This version returns the square of the eucledian distance between the
/// regions, rather than the straight eucledian as BoundLuvDSC does.
class EOS_CLASS SqrBoundLuvDSC : public DSC
{
 public:
  /// &nbsp;
   SqrBoundLuvDSC(const svt::Field<bs::ColourLuv> & left,const svt::Field<bs::ColourLuv> & right,real32 mult = 1.0,real32 cap = math::Infinity<real32>());

  /// &nbsp;
   ~SqrBoundLuvDSC();

  /// &nbsp;
   DSC * Clone() const;


  /// Sets the multiplier, incase you want to change it.
   void SetMult(real32 mult);

  /// Sets a cap on how high the cost can be, Defaults to infinity, i.e. no limit.
   void SetCap(real32 cap);


  /// &nbsp;
   nat32 Bytes() const;

  /// &nbsp;
   real32 Cost(const byte * left,const byte * right) const;

  /// &nbsp;
   void Join(const byte * left,const byte * right,byte * out) const;

  /// &nbsp;
   void Join(nat32 n,const byte ** in,byte * out) const;


  /// &nbsp;
   nat32 WidthLeft() const;

  /// &nbsp;
   nat32 HeightLeft() const;

  /// &nbsp;
   void Left(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   nat32 WidthRight() const;

  /// &nbsp;
   nat32 HeightRight() const;

  /// &nbsp;
   void Right(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   real32 Cost(nat32 leftX,nat32 rightX,nat32 y) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  svt::Field<bs::ColourLuv> left;
  svt::Field<bs::ColourLuv> right;
  real32 mult;
  real32 cap;
};

//------------------------------------------------------------------------------
/// Defines a DSC in terms of the Luv colour space, using manhatten distance for costs.
/// Uses a 3 dimensional bounding box in Luv space, with bounding box union and
/// 1 square initialisation. (i.e. +/- 0.5 on the X axis and on the Y axis.
/// Cost is the difference between the closest points, if the regions overlap it will be zero.
/// Also suports the setting of a cost cap.
class EOS_CLASS ManBoundLuvDSC : public DSC
{
 public:
  /// &nbsp;
   ManBoundLuvDSC(const svt::Field<bs::ColourLuv> & left,const svt::Field<bs::ColourLuv> & right,
                  real32 mult = 1.0,real32 cap = math::Infinity<real32>());

  /// &nbsp;
   ~ManBoundLuvDSC();

  /// &nbsp;
   DSC * Clone() const;


  /// Sets the multiplier, incase you want to change it.
   void SetMult(real32 mult);

  /// Sets a cap on how high the cost can be, Defaults to infinity, i.e. no limit.
   void SetCap(real32 cap);


  /// &nbsp;
   nat32 Bytes() const;

  /// &nbsp;
   real32 Cost(const byte * left,const byte * right) const;

  /// &nbsp;
   void Join(const byte * left,const byte * right,byte * out) const;

  /// &nbsp;
   void Join(nat32 n,const byte ** in,byte * out) const;


  /// &nbsp;
   nat32 WidthLeft() const;

  /// &nbsp;
   nat32 HeightLeft() const;

  /// &nbsp;
   void Left(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   nat32 WidthRight() const;

  /// &nbsp;
   nat32 HeightRight() const;

  /// &nbsp;
   void Right(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   real32 Cost(nat32 leftX,nat32 rightX,nat32 y) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  svt::Field<bs::ColourLuv> left;
  svt::Field<bs::ColourLuv> right;
  real32 mult;
  real32 cap;
};

//------------------------------------------------------------------------------
/// Defines a DSC in terms of the Luv colour space, using euclidian distance for costs.
/// Uses a 3 dimensional bounding box in Luv space, with bounding box union and
/// 1 square initialisation. i.e. +/- 0.5 on the X axis and on the Y axis.
/// Cost is the difference between the furthest points, will only return zero if the regions precisly match.
/// Also suports the setting of a cost cap.
class EOS_CLASS RangeLuvDSC : public DSC
{
 public:
  /// &nbsp;
   RangeLuvDSC(const svt::Field<bs::ColourLuv> & left,const svt::Field<bs::ColourLuv> & right,real32 mult = 1.0,real32 cap = math::Infinity<real32>());

  /// &nbsp;
   ~RangeLuvDSC();

  /// &nbsp;
   DSC * Clone() const;


  /// Sets the multiplier, incase you want to change it.
   void SetMult(real32 mult);

  /// Sets a cap on how high the cost can be, Defaults to infinity, i.e. no limit.
   void SetCap(real32 cap);


  /// &nbsp;
   nat32 Bytes() const;

  /// &nbsp;
   real32 Cost(const byte * left,const byte * right) const;

  /// &nbsp;
   void Join(const byte * left,const byte * right,byte * out) const;

  /// &nbsp;
   void Join(nat32 n,const byte ** in,byte * out) const;


  /// &nbsp;
   nat32 WidthLeft() const;

  /// &nbsp;
   nat32 HeightLeft() const;

  /// &nbsp;
   void Left(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   nat32 WidthRight() const;

  /// &nbsp;
   nat32 HeightRight() const;

  /// &nbsp;
   void Right(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   real32 Cost(nat32 leftX,nat32 rightX,nat32 y) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  svt::Field<bs::ColourLuv> left;
  svt::Field<bs::ColourLuv> right;
  real32 mult;
  real32 cap;
};

//------------------------------------------------------------------------------
/// Allows you to combine an arbitary number of DSC's, simple summing there
/// costs to get a final output.
class EOS_CLASS ManhattanDSC : public DSC
{
 public:
  /// &nbsp;
   ManhattanDSC();

  /// &nbsp;
   ~ManhattanDSC();

  /// &nbsp;
   DSC * Clone() const;


  /// Adds a child DSC to the sum, it must be the same sizes as all previous
  /// additions, it will be cloned.
   void Add(const DSC * dsc);


  /// &nbsp;
   nat32 Bytes() const;

  /// &nbsp;
   real32 Cost(const byte * left,const byte * right) const;

  /// &nbsp;
   void Join(const byte * left,const byte * right,byte * out) const;

  /// &nbsp;
   void Join(nat32 n,const byte ** in,byte * out) const;


  /// &nbsp;
   nat32 WidthLeft() const;

  /// &nbsp;
   nat32 HeightLeft() const;

  /// &nbsp;
   void Left(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   nat32 WidthRight() const;

  /// &nbsp;
   nat32 HeightRight() const;

  /// &nbsp;
   void Right(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  nat32 bytes;
  ds::Array<DSC*> dsc;
};

//------------------------------------------------------------------------------
/// Allows you to combine an arbitary number of DSC's, sums the squares of the
/// inputs and returns the square root of the total, i.e. as the name of the
/// class indicates.
class EOS_CLASS EuclideanDSC : public DSC
{
 public:
  /// &nbsp;
   EuclideanDSC();

  /// &nbsp;
   ~EuclideanDSC();

  /// &nbsp;
   DSC * Clone() const;


  /// Adds a child DSC to the sum, it must be the same sizes as all previous
  /// additions, it will be cloned.
   void Add(const DSC * dsc);


  /// &nbsp;
   nat32 Bytes() const;

  /// &nbsp;
   real32 Cost(const byte * left,const byte * right) const;

  /// &nbsp;
   void Join(const byte * left,const byte * right,byte * out) const;

  /// &nbsp;
   void Join(nat32 n,const byte ** in,byte * out) const;


  /// &nbsp;
   nat32 WidthLeft() const;

  /// &nbsp;
   nat32 HeightLeft() const;

  /// &nbsp;
   void Left(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   nat32 WidthRight() const;

  /// &nbsp;
   nat32 HeightRight() const;

  /// &nbsp;
   void Right(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  nat32 bytes;
  ds::Array<DSC*> dsc;
};

//------------------------------------------------------------------------------
/// Simple container for a DSC - swaps the left and right images arround.
class EOS_CLASS SwapDSC : public DSC
{
 public:
  /// Clones the given dsc.
   SwapDSC(const DSC * dsc);

  /// &nbsp;
   ~SwapDSC();

  /// &nbsp;
   DSC * Clone() const;


  /// &nbsp;
   nat32 Bytes() const;

  /// &nbsp;
   real32 Cost(const byte * left,const byte * right) const;

  /// &nbsp;
   void Join(const byte * left,const byte * right,byte * out) const;

  /// &nbsp;
   void Join(nat32 n,const byte ** in,byte * out) const;


  /// &nbsp;
   nat32 WidthLeft() const;

  /// &nbsp;
   nat32 HeightLeft() const;

  /// &nbsp;
   void Left(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   nat32 WidthRight() const;

  /// &nbsp;
   nat32 HeightRight() const;

  /// &nbsp;
   void Right(nat32 x,nat32 y,byte * out) const;


  /// &nbsp;
   real32 Cost(nat32 leftX,nat32 rightX,nat32 y) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  DSC * dsc;
};

//------------------------------------------------------------------------------
/// This is given a DSC, it then generates a hierachy using the DSC and
/// expresses a DSC for each level of the hierachy. This is a module in many
/// hierachical stereo algorithms.
class EOS_CLASS HierarchyDSC
{
 public:
  ///&nbsp;
   HierarchyDSC();

  /// &nbsp;
   ~HierarchyDSC();


  /// Sets the DSC and (re)builds the hierarchy.
  /// It is internally copied.
  /// Also sets masks to be used, locations with false in the mask will not be used.
   void Set(const DSC & dsc,
            const svt::Field<bit> & leftMask = svt::Field<bit>(),
            const svt::Field<bit> & rightMask = svt::Field<bit>());


  /// Returns the number of levels calculated. Follows usuall 2D hierarchy rules.
   nat32 Levels() const;

  /// Returns a reference to a DSC for the level indicated. May be cloned at
  /// will, though it is dependent on this objects data structure, so this must
  /// not die first.
   const DSC & Level(nat32 l) const;
   
  /// Allows you to get the mask for a particular level of the hierarchy.
  /// Left version.
   svt::Field<bit> LeftMask(nat32 l) const;
   
  /// Allows you to get the mask for a particular level of the hierarchy.
  /// Right version.
   svt::Field<bit> RightMask(nat32 l) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  // Structure for each level of hierarchy - impliments DSC interface.
  // Does not delete the data it points to or the DSC it points to - they are
  // owned by the containning class.
   struct EOS_CLASS HierLevel : public DSC
   {
    // Data...
     const DSC * dsc;
     nat32 widthLeft;
     nat32 widthRight;
     nat32 height;
     byte * leftData;
     byte * rightData;

    // Interface...
      HierLevel();
     ~HierLevel();
     DSC * Clone() const;

     nat32 Bytes() const;
     real32 Cost(const byte * left,const byte * right) const;
     void Join(const byte * left,const byte * right,byte * out) const;
     void Join(nat32 n,const byte ** in,byte * out) const;

     nat32 WidthLeft() const;
     nat32 HeightLeft() const;
     void Left(nat32 x,nat32 y,byte * out) const;

     nat32 WidthRight() const;
     nat32 HeightRight() const;
     void Right(nat32 x,nat32 y,byte * out) const;

     real32 Cost(nat32 leftX,nat32 rightX,nat32 y) const;

     cstrconst TypeString() const;
   };

  // Storage of Level's...
   ds::ArrayDel<HierLevel> levels;
   
  // Storage of masks...
   ds::Array<svt::Var*> leftMask;
   ds::Array<svt::Var*> rightMask;
};

//------------------------------------------------------------------------------
/// Defines a standardised interface for any limited DSI representation.
/// (By limited that means having only a limited number of candidates for each
/// pixel.)
/// Can also be used for disparity map representation, it will simply have only
/// 1 entry per pixel.
///
/// This can also double up as an initialisation for a stereo algorithm - the
/// disparities are used to indicate likelly candidates, with the refinement
/// algorithm then taking disparities plus and minus a given value as the search
/// space.
class EOS_CLASS DSI : public Deletable
{
 public:
  /// &nbsp;
   ~DSI();


  /// Returns the width of the image.
   virtual nat32 Width() const = 0;

  /// Returns the height of the image.
   virtual nat32 Height() const = 0;


  /// Returns how many disparitys are assigned to a given pixel.
   virtual nat32 Size(nat32 x,nat32 y) const = 0;

  /// Allows you to access the multiple disparities assigned to an indexed pixel.
   virtual real32 Disp(nat32 x,nat32 y,nat32 i) const = 0;

  /// Allows you to obtain the cost of a given disparity. By default implimented
  /// as the -ln(Prob(...)), whilst prob is implimented visa-versa - i.e. you *must*
  /// impliment at least one of them.
   virtual real32 Cost(nat32 x,nat32 y,nat32 i) const;

  /// Allows you to obtain the probability of a given disparity, not necesarily
  /// normalied. By default implimented as e^(-Cost(...)), whilst Cost is
  /// implimented visa-versa - i.e. you *must* impliment at least one of them.
   virtual real32 Prob(nat32 x,nat32 y,nat32 i) const;

  /// Returns the +/- value for a disparity, i.e. take any returned disparity to
  /// in fact cover the range of that disparity +/- this value.
  /// Mostly returns 0, but some disparity matching tools work continuously, and
  /// hence work with regions.
   virtual real32 DispWidth(nat32 x,nat32 y,nat32 i) const;


  /// The obtains a disparity map by selecting the lowest cost disparity for
  /// each pixel. Sets to 0 pixels with no disparity assignment.
   void GetDisp(svt::Field<real32> & out) const;

  /// Helper method, extracts a mask, true where the DSI has at least one valid
  /// value, false where its 0.
   void GetMask(svt::Field<bit> & out) const;


  /// &nbsp;
   virtual cstrconst TypeString() const = 0;
};

//------------------------------------------------------------------------------
/// Simply dummy DIS that passes through a dsi field of disparities, including
/// support for a mask.
class EOS_CLASS DummyDSI : public DSI
{
 public:
  /// &nbsp;
   DummyDSI(const svt::Field<real32> & disp,const svt::Field<bit> * mask = null<svt::Field<bit>*>());

  /// &nbsp;
   ~DummyDSI();


  /// Returns the width of the image.
   nat32 Width() const;

  /// Returns the height of the image.
   nat32 Height() const;


  /// Returns how many disparitys are assigned to a given pixel.
   nat32 Size(nat32 x,nat32 y) const;

  /// Allows you to access the multiple disparities assigned to an indexed pixel.
   real32 Disp(nat32 x,nat32 y,nat32 i) const;

  /// Allows you to obtain the cost of a given disparity. By default implimented
  /// as the -ln(Prob(...)), whilst prob is implimented visa-versa - i.e. you *must*
  /// impliment at least one of them.
   real32 Cost(nat32 x,nat32 y,nat32 i) const;

  /// Allows you to obtain the probability of a given disparity, not necesarily
  /// normalied. By default implimented as e^(-Cost(...)), whilst Cost is
  /// implimented visa-versa - i.e. you *must* impliment at least one of them.
   real32 Prob(nat32 x,nat32 y,nat32 i) const;

  /// Returns the +/- value for a disparity, i.e. take any returned disparity to
  /// in fact cover the range of that disparity +/- this value.
  /// Mostly returns 0, but some disparity matching tools work continuously, and
  /// hence work with regions.
   real32 DispWidth(nat32 x,nat32 y,nat32 i) const;


  /// &nbsp;
   cstrconst TypeString() const;

 private:
  svt::Field<real32> disp;
  svt::Field<bit> mask;
};

//------------------------------------------------------------------------------
 };
};
#endif
