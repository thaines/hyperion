#ifndef EOS_BS_LUV_RANGE_H
#define EOS_BS_LUV_RANGE_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file luv_range.h
/// Provides some suport classes for algoirthms that work in luv colourspace 
/// with colour ranges.

#include "eos/types.h"
#include "eos/bs/colours.h"
#include "eos/svt/field.h"
#include "eos/time/progress.h"

namespace eos
{
 namespace bs
 {
//------------------------------------------------------------------------------
/// A colour range, of Luv colours.
class EOS_CLASS LuvRange
{
 public:
  /// Random data ahoy!
   LuvRange(){}

  /// Sets the minimum and maximumm to be a single point.
   LuvRange(const bs::ColourLuv & rhs) {*this = rhs;}

  /// &nbsp;
   LuvRange(const LuvRange & rhs) {*this = rhs;}

  /// &nbsp;
   ~LuvRange() {}


  /// &nbsp;
   LuvRange & operator = (const bs::ColourLuv & rhs) {min = rhs; max = rhs; return *this;}

  /// &nbsp;
   LuvRange & operator = (const LuvRange & rhs) {min = rhs.min; max = rhs.max; return *this;}


  /// Addition increases the range to include the new colour.
   LuvRange & operator += (const bs::ColourLuv & rhs)
   {
    min.l = math::Min(min.l,rhs.l);
    min.u = math::Min(min.u,rhs.u);
    min.v = math::Min(min.v,rhs.v);
    
    max.l = math::Max(max.l,rhs.l);
    max.u = math::Max(max.u,rhs.u);
    max.v = math::Max(max.v,rhs.v);
    
    return *this;
   }

  /// Addition increases the range to include the new LuvRange.
   LuvRange & operator += (const LuvRange & rhs)
   {
    min.l = math::Min(min.l,rhs.min.l);
    min.u = math::Min(min.u,rhs.min.u);
    min.v = math::Min(min.v,rhs.min.v);
    
    max.l = math::Max(max.l,rhs.max.l);
    max.u = math::Max(max.u,rhs.max.u);
    max.v = math::Max(max.v,rhs.max.v);
    
    return *this;
   }


  /// A difference operator, returns a similarity measure between two ranges,
  /// which will be 0 if they are identical.
  /// Matching smaller but equally distant ranges will produce smaller costs, 
  /// as would be expected.
  /// The cost is the sum of three costs, one for each channel.
  /// Each cost is the length of the minimal containing range subtracted by the 
  /// length of the intersection range.
  /// I wouldn't want to state any properties of this, but it certainly *seems*
  /// reasonable. It is symetric though.
   real32 operator - (const LuvRange & rhs) const
   {
    real32 ret = 0.0;
    
    ret += math::Max(max.l,rhs.max.l) - math::Min(min.l,rhs.min.l);
    ret += math::Max(max.u,rhs.max.u) - math::Min(min.u,rhs.min.u);
    ret += math::Max(max.v,rhs.max.v) - math::Min(min.v,rhs.min.v);

    ret -= math::Max(real32(0.0),math::Min(max.l,rhs.max.l) - math::Max(min.l,rhs.min.l));
    ret -= math::Max(real32(0.0),math::Min(max.u,rhs.max.u) - math::Max(min.u,rhs.min.u));
    ret -= math::Max(real32(0.0),math::Min(max.v,rhs.max.v) - math::Max(min.v,rhs.min.v));

    return ret;
   }
   
  /// A difference operator, returns a similarity measure between two ranges,
  /// which will be 0 if they inhabit the same volume at any point.
  /// This is simply the sum of differences between the closest parts, which
  /// if they overlap will be 0.
  /// This is the method of Birchfield and Tomasi 1998 for single pixels if you
  /// include there interpolation with neighbours.
  /// For larger numbers of pixels it becomes an extension, which is in
  /// some sense reasonable though for large areas if liable to create large
  /// areas which produce a lot of 0's.
   real32 operator / (const LuvRange & rhs) const
   {
    real32 ret = 0.0;
    
    ret += math::Max(real32(0.0),min.l-rhs.max.l,rhs.min.l-max.l);
    ret += math::Max(real32(0.0),min.u-rhs.max.u,rhs.min.u-max.u);
    ret += math::Max(real32(0.0),min.v-rhs.max.v,rhs.min.v-max.v);
    
    return ret;
   }


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::bs::LuvRange";}


  /// The minimum.
   bs::ColourLuv min;

  /// The maximum.
   bs::ColourLuv max; 
};

//------------------------------------------------------------------------------
/// Given an image this generates a colour range hierachy. For the bottom layer
/// of pixels it assigns colour ranges assuming each pixel covers a range that 
/// includes the pixel weighted 50:50 with each of its 4 neighbours. For each 
/// layer of of the hierachy it combines the 4 parent ranges as you would expect.
class EOS_CLASS LuvRangeHierachy
{
 public:
  /// This creates the hierachy, after calling this you can kill the given field
  /// as this contains its own data structure.
  /// A progress bar can be provided if you so choose.
   LuvRangeHierachy(const svt::Field<bs::ColourLuv> & img,time::Progress * prog = null<time::Progress*>());
   
  /// &nbsp;
   ~LuvRangeHierachy();


  /// Returns the number of levels, level 0 is the full image, Levels()-1 will 
  /// allways be a single pixel.
   nat32 Levels() const;
   
  /// Returns the width of the given level.
   nat32 Width(nat32 level) const;

  /// Returns the height of the given level.
   nat32 Height(nat32 level) const;


  /// Returns a reference to a requested range.
   const LuvRange & Get(nat32 level,nat32 x,nat32 y) const;


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::bs::LuvRangeHierachy";}


 private:
  nat32 levels;
  nat32 width;
  nat32 height;
  
  LuvRange ** data;
};

//------------------------------------------------------------------------------
 };
};
#endif
