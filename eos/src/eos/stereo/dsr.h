#ifndef EOS_STEREO_DSR_H
#define EOS_STEREO_DSR_H
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


/// \file dsr.h
/// Provides the disparity-spatial range, essentially a method for providing
/// search ranges to things that then create DSI's, for when they are not
/// hierachical.

#include "eos/types.h"

#include "eos/stereo/dsi.h"
#include "eos/mem/packer.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// An interface that defines access to a per-pixel disparity search range, 
/// to pixel resolution.
/// Used to provide stereo algorithms that are not hierachical with search
/// ranges.
class EOS_CLASS DSR : public Deletable
{
 public:
  /// &nbsp;
   ~DSR();

  /// &nbsp;
   virtual DSR * Clone() const = 0;


  /// Returns the width.
   virtual nat32 Width() const = 0;
   
  /// Returns the height.
   virtual nat32 Height() const = 0;
   
  /// Returns how many matches are contained within.
  /// Counts them, manually, so slow.
   nat32 Matches();
   
  /// Returns the number of ranges assigned to a given pixel.
  /// Ranges can not be adjacent. Can be zero to indicate masked regions.
  /// Ranges must be sorted.
   virtual nat32 Ranges(nat32 x,nat32 y) const = 0;
  
  /// Returns the starting disparity of a range. (Inclusive.)
   virtual int32 Start(nat32 x,nat32 y,nat32 i) const = 0;

  /// Returns the ending disparity of a range. (Inclusive.)
   virtual int32 End(nat32 x,nat32 y,nat32 i) const = 0;


  /// &nbsp;
   virtual cstrconst TypeString() const = 0;
};

//------------------------------------------------------------------------------
/// DSR that returns the same single range for every pixel.
/// Range will usually be set by a user or sparse correspondence algorithm.
/// Obviously not an ideal class to be using.
class EOS_CLASS RangeDSR : public DSR
{
 public:
  /// Constructs it with a given width/height.
   RangeDSR(nat32 width,nat32 height);

  /// &nbsp;
   ~RangeDSR();
   
  /// &nbsp;
   DSR * Clone() const;

  
  /// Sets the search range for all pixels.
   void Set(int32 start,int32 end);


  /// Returns the width.
   nat32 Width() const;

  /// Returns the height.
   nat32 Height() const;


  /// Returns the number of ranges assigned to a given pixel.
  /// Ranges can not be adjacent. Can be zero to indicate masked regions.
   nat32 Ranges(nat32 x,nat32 y) const;

  /// Returns the starting disparity of a range. (Inclusive.)
   int32 Start(nat32 x,nat32 y,nat32 i) const;

  /// Returns the ending disparity of a range. (Inclusive.)
   int32 End(nat32 x,nat32 y,nat32 i) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  nat32 width;
  nat32 height;

  int32 start;
  int32 end;
};

//------------------------------------------------------------------------------
/// Simple DSR implimentor, simply contains its own storage.
/// Can be obtained from a DSI and used to boot strap another
/// algorithm. Generally used so a hierachical but simple algorithm can boot
/// strap an algorithm for which a hierachical implimentation would be
/// difficult or impossible.
/// Not the most efficient of implimentations for editting, fast for access though.
/// Does not limit disparities to in-image ranges - thay can map outside the image.
class EOS_CLASS BasicDSR : public DSR
{
 public:
  /// Constructs it with a given width/height, with no assigned ranges.
   BasicDSR(nat32 width,nat32 height);
  
  /// &nbsp;
   BasicDSR(const DSR & rhs);

  /// Extracts a DSR from the given DSI. Considers each pixel to be +/- 0.5 and 
  /// marks every pixel which has a DSI entry in its range.
   BasicDSR(const DSI & rhs);

  /// &nbsp;
   ~BasicDSR();
   
  /// &nbsp;
   DSR * Clone() const;

  
  /// Adds a range to a pixel.
   void Add(nat32 x,nat32 y,int32 start,int32 end);
   
  /// Grows the ranges of all pixels, this is increadibly helpful for turning a
  /// per pixel disparity assignment into ranges to be searched.
   void Grow(nat32 range);


  /// Returns the width.
   nat32 Width() const;
   
  /// Returns the height.
   nat32 Height() const;

   
  /// Returns the number of ranges assigned to a given pixel.
  /// Ranges can not be adjacent. Can be zero to indicate masked regions.
   nat32 Ranges(nat32 x,nat32 y) const;
  
  /// Returns the starting disparity of a range. (Inclusive.)
   int32 Start(nat32 x,nat32 y,nat32 i) const;

  /// Returns the ending disparity of a range. (Inclusive.)
   int32 End(nat32 x,nat32 y,nat32 i) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  struct Range
  {
   int32 start;
   int32 end;
  };

  ds::ArrayDel2D< ds::Array<Range> > data;
};

//------------------------------------------------------------------------------
/// Specialist DSR - this is given a DSI thats half the resolution
/// (With incriment if odd.), it then uses it to be a DSR for the next level
/// down, the idea being that this object acts as the glue if chaining together
/// a sequence of DSR eatting, DSI spewing stereo algorithms to form a hierachy.
/// You also provide the range, i.e. each disparity from the above level is made
/// +/- this range.
///
/// Uses caching, so as long as access patterns are sensible it will be very fast.
/// Memory consumption is, of course, minimal.
///
/// WARNING: This requires the assumption that the DSI returns disparities in sorted order.
class EOS_CLASS HierachyDSR : public DSR
{
 public:
  /// &nbsp;
   HierachyDSR();

  /// &nbsp;
   ~HierachyDSR();
  
  /// &nbsp;
   DSR * Clone() const;


  /// Sets the dimensions of the output DSR - required to resolve the odd number ambiguity.
   void Set(nat32 width,nat32 height);
   
  /// Sets the DSI to use and the range to extend its entrys by. (Can be zero.)
   void Set(const DSI & dsi,nat32 range = 0);
   
  /// Sets masks for both sides, these will then prune the regions accordingly.
  /// Optional.
   void Set(const svt::Field<bit> & leftMask,const svt::Field<bit> & rightMask);


  /// &nbsp;
   nat32 Width() const;
   
  /// &nbsp;
   nat32 Height() const;
   
  /// &nbsp;
   nat32 Ranges(nat32 x,nat32 y) const;
  
  /// &nbsp;
   int32 Start(nat32 x,nat32 y,nat32 i) const;

  /// &nbsp;
   int32 End(nat32 x,nat32 y,nat32 i) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  // Actual information...
   nat32 width;
   nat32 height;

   const DSI * dsi;
   nat32 range;

   svt::Field<bit> leftMask;
   svt::Field<bit> rightMask;


  // Cache...
   struct Range
   {
    int32 start;
    int32 end;
   };
  
   mutable nat32 cacheX;
   mutable nat32 cacheY;
   mutable nat32 cacheSize;
   mutable ds::Array<Range> cacheRange; // Never shrunk.
   
   // This fills in the cache for the given x and y, or returns instantly if its a match.
    void CalcCache(nat32 x,nat32 y) const; 
};

//------------------------------------------------------------------------------
/// This DSR implimentation copies a given DSR and 'spreads' it, which is to say
/// that each pixel is set to the union of all ranges in a window surrounding
/// it, sort of a top-hat blur for DSR's. Required in a hierachy, to avoid the
/// grid pattern overidiing the costs and destroying any chance of results.
/// Unlike BasicDSR this uses a very efficient data structure, and makes a 
/// serious effort at being fast.
class EOS_CLASS SpreadDSR : public DSR
{
 public:
  /// Initialised empty.
   SpreadDSR();
   
  /// &nbsp;
   ~SpreadDSR();
   
  /// &nbsp;
   DSR * Clone() const;

   
  /// Sets object up, must be called before use. Mask is optional, and simply
  /// specifies areas which are to remain range-less.
  /// Radius indicates the size of the window to grab entrys from for each pixel.
   void Set(const DSR & dsr,nat32 radius,const svt::Field<bit> & leftMask = svt::Field<bit>());
 
 
  /// &nbsp;
   nat32 Width() const;
   
  /// &nbsp;
   nat32 Height() const;
   
  /// &nbsp;
   nat32 Ranges(nat32 x,nat32 y) const;
  
  /// &nbsp;
   int32 Start(nat32 x,nat32 y,nat32 i) const;

  /// &nbsp;
   int32 End(nat32 x,nat32 y,nat32 i) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  static const nat32 pack_size = 2*1024*1024;
 
  // Standard scanline structure is use as final output...
   struct Range
   {
    int32 start;
    int32 end;
    
    bit Intercepts(const Range & rhs) const
    {
     if (end+1<rhs.start) return false;
     if (start-1>rhs.end) return false;
     return true;
    }
    
    Range & operator += (const Range & rhs)
    {
     start = math::Min(start,rhs.start);
     end = math::Max(end,rhs.end);
     return *this;
    }
   };
   
   struct Scanline
   {
    ds::Array<nat32> index; // Indexed by x, width+1 for dummy.
    ds::Array<Range> data;
   };
   
   ds::ArrayDel<Scanline> data; // Indexed by y.
   
   
  // Suport structures for the work done...
   struct Ran;

   struct RanLink
   {
    Ran * ptr;
    RanLink * next;
    RanLink * last;
   };
   
   struct Ran
   {
    RanLink rl;
    Range range;
   };
   
  // Helper method - given the root RanLink of two sorted range sets this merges
  // one into another. Requires a memory allocator.
   static void Union(RanLink & out,const RanLink & in,mem::Packer & packer);
};

//------------------------------------------------------------------------------
 };
};
#endif
