#ifndef EOS_STEREO_DSI_MS_2_H
#define EOS_STEREO_DSI_MS_2_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file dsi_ms_2.h
/// The sequal to dsi_ms.h. Impliments a better version of the sparse stereo 
/// algorithm contained within that module, that should produce better results.

#include "eos/types.h"

#include "eos/file/csv.h"
#include "eos/time/progress.h"

#include "eos/ds/arrays.h"
#include "eos/ds/arrays_ns.h"
#include "eos/ds/arrays2d.h"
#include "eos/ds/sparse_bit_array.h"
#include "eos/ds/kd_tree.h"
#include "eos/ds/nth.h"

#include "eos/svt/field.h"
#include "eos/stereo/dsi.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// The sequal to SparseDSI. This version is identical to the version described
/// in the previous versions except for 5 refinements:
/// * It follows the constraint that states that occlusion can only one occur in one image at a time.
/// * Its symetric.
/// * It supports masks.
/// * It works in the case when 'errLim' isn't 0.
/// * Its faster, with an approximatly O(height*width*log(width)) implimentation.
///
/// One subtle but important difference is that this can assign areas as
/// occluded, i.e. it can choose to not assign any disparities to a pixel.
/// It also doesn't produce sorted output like the previous version, so
/// disparities can be arbitarily ordered.
/// (It has a method to correct this, but calling it is optional.)
///
/// It retains all the advantages of the earlier version, such as being
/// hierachical and working with pixel regions rather than points.
/// The interface is almost identical.
class EOS_CLASS SparseDSI2 : public DSI
{
 public:
  /// &nbsp;
   SparseDSI2();

  /// &nbsp;
   ~SparseDSI2();


  /// Sets the parameters.
  /// \param occCost The cost of an occlusion. Should be set considering the 
  ///                difference operator of the given DSC. Defaults to 1.0.
  /// \param vertCost The cost per unit disparity of a difference between two 
  ///                 scanlines. Because of its sparse nature it takes the minimum
  ///                 cost from all the disparitys in the previous scanline.
  ///                 Defaults to 1.0.
  /// \param vertMult The entire vertical cost - the previous match cost + the 
  ///                 difference cost multiplied by vertCost, is multiplied by this.
  ///                 Defaults to 0.2. Without this the vertical cost swamps the 
  ///                 other costs - must be less than 1.
  /// \param pixLim This is the maximum number of matches that can be assigned 
  ///               to a given pixel. Applies to each level of the hierachy 
  ///               during processing and the final output.
  ///               Defaults to 1.
   void Set(real32 occCost,real32 vertCost,real32 vertMult,nat32 pixLim);

  /// Sets the DSC which defines the cost of each match. Its cloned and internaly stored.
   void Set(DSC * dsc);

  /// Sets masks for the left and right images, where false indicates areas 
  /// outside the image that should not be considered. Such areas will be 
  /// assigned no disparities and have no influence on the algorithm.
  /// Optional.
   void Set(const svt::Field<bit> & maskLeft,const svt::Field<bit> & maskRight);


  /// &nbsp;
   void Run(time::Progress * prog = null<time::Progress*>());


  /// The width of the left hand image. Provided to comply with the interface.
   nat32 Width() const {return widthLeft;}

  /// The height of the left hand image. Provided to comply with the interface.
   nat32 Height() const {return heightLeft;}

  /// The width of the left hand image.
   nat32 WidthLeft() const {return widthLeft;}

  /// The height of the left hand image.
   nat32 HeightLeft() const {return heightLeft;}

  /// The width of the the right hand image.
   nat32 WidthRight() const {return widthRight;}

  /// The height of the the right hand image.
   nat32 HeightRight() const {return heightRight;}


  /// Returns how many disparities have been assigned to a particular pixel.
   nat32 Size(nat32 x,nat32 y) const;

  /// Returns the disparity of a given entry at a location.
   real32 Disp(nat32 x,nat32 y,nat32 i) const;

  /// Returns the cost assigned to a disparity at a location.
  /// Lower costs are better, 0 is perfect, this value is 
  /// adjusted to be image width independent.
   real32 Cost(nat32 x,nat32 y,nat32 i) const;

  /// Returns the 'disparity width', i.e. each returned disparity is over the 
  /// range of the value returned +/- this value.
   real32 DispWidth(nat32 x,nat32 y,nat32 i) const {return 0.5;}


  /// Sorts the disparities by disparity, so that the first index for any pixel 
  /// has the lowest disparity, the next the second lowest etc.
   void SortByDisp(time::Progress * prog = null<time::Progress*>());

  /// Sorts the disparities by cost, so that the first index for any pixel 
  /// has the lowest cost, the next the second lowest etc.
   void SortByCost(time::Progress * prog = null<time::Progress*>());


  /// &nbsp;
   cstrconst TypeString() const {return "eos::stereo::SparseDSI2";}


 private:
  static const int32 range = 3; // At each level +/- this inclusive is the search range arround each offset from above.


  // Suport structures...
   // This structure represents a match between a pixel in the left image and a
   // pixel in the right image, used during run-time...
    struct Match
    {
     int32 left; // x coordinate of pixel in left image.
     int32 right; // y coordinate of pixel in left image.

     real32 cost; // Cost of using this match, calculated in first pass.
     real32 incCost; // Cheapest cost from start of images to get to this match.
     real32 decCost; // Cheapest cost from end of images to get to this match.

     // Disparity of match...
      int32 Disp() const {return right-left;}

     // Total cost of this pixel...
      real32 TotalCost() const {return cost + incCost + decCost;}
    };

   // This is used to store the final disparity/cost pairs for each pixel.
   // The storage structure gives the left image coordinate...
    struct DispCost
    {
     int32 disp;
     real32 cost;
     
     struct SortDisp : public ds::Sort
     {
      static bit LessThan(const DispCost & lhs,const DispCost & rhs) {return lhs.disp < rhs.disp;}
     };

     struct SortCost : public ds::Sort
     {
      static bit LessThan(const DispCost & lhs,const DispCost & rhs) {return lhs.cost < rhs.cost;}
     };
    };

   // This stores al the output for a final row in the left image, a scanline...
   // Index is indexed by x to give the offset to get to the relevant entrys,
   // The next entry tells you how many entrys are assigned to the current one.
    struct Scanline
    {
     ds::Array<DispCost> data;
     ds::Array<nat32> index; // Allways width+1 in length, final entry being size and a dummy.
     
     Scanline & operator = (const Scanline & rhs)
     {
      data.Size(rhs.data.Size());
      for (nat32 i=0;i<data.Size();i++) data[i] = rhs.data[i];

      index.Size(rhs.index.Size());
      for (nat32 i=0;i<index.Size();i++) index[i] = rhs.index[i];

      return *this;
     }
    };
    
   // Nice simple structure used during the dynamic programming...
    struct BestCost
    {
     real32 cost[2]; // cost upto pos, *exclusive*.
     int32 pos[2];
    };
    
   // Structure used for vertical consistancy constraint...
    struct BestMatch
    {
     int32 other; // Index of the other dimension that makes this the lwoest scorer.
     real32 cost; // The cost of the match, used during the calculation.
    };


  // In...
   // Parameters...
    real32 occCost; // Cost of an occlusion per occluded pixel.
    nat32 pixLim; // Maximum number of matches to assign to any given pixel.
    real32 vertCost; // Cost per pixel offset of a difference in vertical. The base cost is added to this.
    real32 vertMult; // Multiplier of vertical cost, must be less than 1 otherwise it overwhelms all other costs.

   // Data structures...
    DSC * dsc;
    svt::Field<bit> maskLeft;
    svt::Field<bit> maskRight;


  // General...
   int32 widthLeft;
   int32 heightLeft;
   int32 widthRight;
   int32 heightRight;


  // Out...
   ds::ArrayDel<Scanline> data;


  // Helper methods, these cover the real work done by run...
   // First pass - calculates costs for each match in a match array...
    void CostPass(ds::ArrayNS<Match> & matchSet,byte * leftCol,byte * rightCol,
                  ds::InplaceKdTree<byte*,2> * prevRow);
   
   // Second pass - dynamic programming forwards through the structure to 
   // calculate the inc costs...
    void IncPass(int32 level,ds::ArrayNS<Match> & matchSet,
                 ds::Array<BestCost> & leftCost,ds::Array<BestCost> & rightCost);
   
   // Third pass - dynamic programming backwards through the structure to 
   // calculate the dec costs...
    void DecPass(int32 level,ds::ArrayNS<Match> & matchSet,
                 ds::Array<BestCost> & leftCost,ds::Array<BestCost> & rightCost);
   
   // Prune pass, removes all matches with insufficient scores...
    void Prune(int32 level,ds::ArrayNS<Match> & matchSet,
               ds::Array<real32> & bestLeft,ds::Array<real32> & bestRight,ds::MultiNth & nth);
   
   // Expand pass, expands from one level of the hierachy to the next, pruning 
   // duplicates and putting it into the correct order. Handles masking...
    void Expand(int32 newLevel,ds::ArrayNS<Match> & matchSet,
                const ds::Array<bit> & leftMask,const ds::Array<bit> & rightMask,
                ds::ArrayDel<ds::SparseBitArray> & temp);
   
   // Extract - converts a final match array into a scanline...
    void Extract(const ds::ArrayNS<Match> & matchSet,Scanline & out);
};

//------------------------------------------------------------------------------
 };
};
#endif
