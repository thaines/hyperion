#ifndef EOS_BS_LUV_RANGE_H
#define EOS_BS_LUV_RANGE_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file luv_range.h
/// Provides some suport classes for algoirthms that work in luv colourspace 
/// with colour ranges.

#include "eos/types.h"
#include "eos/bs/colours.h"
#include "eos/ds/arrays.h"
#include "eos/ds/arrays2d.h"
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
   
  // Outputs the average colour of the range - can be useful for visualisation...
   void Average(bs::ColourLuv & out) const
   {
    out.l = 0.5 * (max.l + min.l);
    out.u = 0.5 * (max.u + min.u);
    out.v = 0.5 * (max.v + min.v);
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
  /// include their interpolation with neighbours.
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
   
  /// Yet another difference operator. This returns the eucledian distance
  /// between the two most distant points taken from the areas of each range.
  /// Given point estimates this will be the eucledian distance between them,
  /// but as the area of the ranges increases the minimum cost increases also.
   real32 operator ^ (const LuvRange & rhs) const
   {
    real32 dl = math::Max(max.l-rhs.min.l,rhs.max.l-min.l);
    real32 du = math::Max(max.u-rhs.min.u,rhs.max.u-min.u);
    real32 dv = math::Max(max.v-rhs.min.v,rhs.max.v-min.v);

    return math::Sqrt(math::Sqr(dl) + math::Sqr(du) + math::Sqr(dv));
   }


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::bs::LuvRange";}


  /// The minimum.
   bs::ColourLuv min;

  /// The maximum.
   bs::ColourLuv max; 
};

//------------------------------------------------------------------------------
/// Provides an image expressed as a set of pixel ranges. Supports multiple
/// possible initialisations - from images, from other LuvRangeImages, with
/// parameters to decide initialisation. Includes a built in mask.
class EOS_CLASS LuvRangeImage
{
 public:
  /// Initialises with a 0x0 image - have to call a Create method to get
  /// anything interesting.
   LuvRangeImage();
  
  /// &nbsp;
   ~LuvRangeImage();
   
   
  /// Copies a standard field of luv colours - you get a choice in regards to
  /// how it initialises ranges. The actually colour of each pixel is always
  /// used, however you can optionally use the linear interpolation half way
  /// points and/or the corner points.
  /// Can hand in a invalid mask if you choose.
   void Create(const svt::Field<bs::ColourLuv> & img, const svt::Field<bit> & mask, bit useHalfX = true, bit useHalfY = true, bit useCorners = true);
   
  /// Copies another LuvRangeImage, with support for halfing the resolution in either direction.
   void Create(const LuvRangeImage & img, bit halfWidth = false, bit halfHeight = false);
  
   
  /// &nbsp;
   nat32 Width() const;

  /// &nbsp;
   nat32 Height() const;
   
  /// &nbsp;
   bit Valid(nat32 x,nat32 y) const;
   
  /// Extended version of valid - will handle out of bound values and return false for them.
   bit ValidExt(int32 x,int32 y) const;

  /// &nbsp;
   const LuvRange & Get(nat32 x,nat32 y) const;


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::bs::LuvRangeImage";} 
  
 
 private:
  ds::Array2D<LuvRange> data;
  ds::Array2D<bit> mask;
};

//------------------------------------------------------------------------------
/// Given a luv image creates a hierachy of LuvRangeImage-s. You can choose the
/// halfing mode - i.e. it can half just one of x/y or both. You can also choose
/// the initialisation techneque.
class EOS_CLASS LuvRangePyramid
{
 public:
  /// Initialises empty.
   LuvRangePyramid();
  
  /// &nbsp;
   ~LuvRangePyramid();
   
  
  /// Fills in the pyramid from the given image with the given settings.
   void Create(const svt::Field<bs::ColourLuv> & img,const svt::Field<bit> & mask, bit useHalfX = true, bit useHalfY = true, bit useCorners = true, bit halfWidth = true, bit halfHeight = true);
   
  
  /// Returns how many levels exist.
   nat32 Levels() const;
   
  /// Returns true if it halfed width during construction.
   bit HalfWidth() const;

  /// Returns true if it halfed height during construction.
   bit HalfHeight() const;


  /// Returns the LuvRangeImage for the given level. Level 0 is the largest, 
  /// with each following level halfed inat  least one dimension. Image with the
  /// highest level will have dimension of 1 in all dimensions which were halfed.
   const LuvRangeImage & Level(nat32 l) const;


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::bs::LuvRangePyramid";} 


 private:
  bit halfWidth;
  bit halfHeight;
  ds::ArrayDel<LuvRangeImage> data;
};

//------------------------------------------------------------------------------
/// This provides the distance between two luv ranges - provided like
/// this as there are simply so many possible distance functions that may be
/// definied between LuvRanges that an extensable system is needed.
/// Whilst defined as distance no assumption that it is actually a distance
/// metric need be made, though symmetry and some degree of sanity is a must.
class EOS_CLASS LuvRangeDist : public Deletable
{
 public:
  /// &nbsp;
   ~LuvRangeDist();
  
  /// Given two LuvRanges this returns a distance between them.
   virtual real32 operator () (const LuvRange & lhs,const LuvRange & rhs) const = 0;
   
  /// &nbsp;
   virtual cstrconst TypeString() const = 0;
};

//------------------------------------------------------------------------------
/// Probably the most basic reasonable instance of LuvRangeDist.
class EOS_CLASS BasicLRD : public LuvRangeDist
{
 public:
  /// &nbsp;
   ~BasicLRD();
   
  /// &nbsp;
   real32 operator () (const LuvRange & lhs,const LuvRange & rhs) const;
   
  /// &nbsp;
   cstrconst TypeString() const;
};

//------------------------------------------------------------------------------
/// Given an image this generates a colour range hierachy. For the bottom layer
/// of pixels it assigns colour ranges assuming each pixel covers a range that 
/// includes the pixel weighted 50:50 with each of its 4 neighbours. For each 
/// layer of of the hierachy it combines the 4 parent ranges as you would expect.
/// Note that LuvRangePyramid is a more sophisticated version of this - this has
/// been depreciated, but is kept to save recoding its users.
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
