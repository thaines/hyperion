#ifndef EOS_FIT_LIGHT_DIR_H
#define EOS_FIT_LIGHT_DIR_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

/// \file light_dir.h
/// Given an image of irradiance values, segments and surface orientation 
/// distributions this infers the light source direction.

#include "eos/types.h"
#include "eos/math/stats_dir.h"
#include "eos/time/progress.h"
#include "eos/bs/geo3d.h"
#include "eos/ds/priority_queues.h"
#include "eos/svt/field.h"


namespace eos
{
 namespace fit
 {
//------------------------------------------------------------------------------
class EOS_CLASS LightDir
{
 public:
  /// &nbsp;
   LightDir();
   
  /// &nbsp;
   ~LightDir();
   
   
  /// Sets the 3 non-optional inputs - the segmentation, the irradiance and the
  /// surface orientation Fisher distributions.
   void SetData(svt::Field<nat32> seg,svt::Field<real32> irr,svt::Field<math::Fisher> dir);
   
  /// Sets the albedo range to consider, noting that we presume a light source
  /// of strength 1 with no falloff. Defaults to 0.001 to 3.0
   void SetAlbRange(real32 min,real32 max);
   
  /// Sets the maximum cost per segment per pixel, relative to the minimum cost
  /// of that segment - used to cap influence so no one segment biases things 
  /// too strongly. Defaults to 0.1
   void SetSegCapPP(real32 maxCost);
   
  /// Sets the irradiance errors standard deviation, this only matters for
  /// calculating error when albedo is less than irradiance, as too small to
  /// consider in other situations.
  /// Default is 1/128, i.e. 2 values for a 2^8 colour image.
   void SetIrrErr(real32 sd);
   
  /// Sets the number of subdivision of the alg::HemiOfNorm class used to 
  /// generate light source directions to sample.
  /// Defaults to 4.
   void SetSampleSubdiv(nat32 subdiv);
  
  /// Sets the recursionn depth when finding the optimal albedo value via a 
  /// recursive search. Defaults to 8.
   void SetRecursion(nat32 depth);
  
  
  /// &nbsp;
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Returns the calculated optimal light source direction.
   const bs::Normal & BestLightDir() const;
   
  /// Returns the number of driections sampled.
   nat32 SampleSize() const;
   
  /// Returns the direction for sample i.
   const bs::Normal & SampleDir(nat32 i) const;

  /// Returns the cost for sample i.
   real32 SampleCost(nat32 i) const;
   
  /// Returns the number of segments.
   nat32 SegmentCount() const {return albedo.Size();}
   
  /// Returns the albedo for a given segment number.
   real32 SegmentAlbedo(nat32 s) const {return albedo[s];}


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::math::Fisher";}


 private:
  // Parameters...
   real32 minAlbedo;
   real32 maxAlbedo;
   real32 maxSegCostPP;
   real32 lowAlbErr;
   nat32 subdiv;
   nat32 recDepth;
   
  // Input...
   svt::Field<nat32> seg;
   svt::Field<real32> irr;
   svt::Field<math::Fisher> dir;
  
  // Output...
   bs::Normal bestLightDir;
   
   struct LightCost
   {
    bs::Normal dir;
    real32 cost;
   };
   ds::Array<LightCost> lc; // Cost for each light source direction - for diagnostics really.
   
   ds::Array<real32> albedo; // Albedo for each segment number.


  // Runtime...
   // Structure for storing a pixel - its irradiance and distribution on surface orientation...
    struct Pixel
    {
     real32 irr;
     math::Fisher dir;
    };
    
   // Structure for storing complimentary info to Pixel, used for the light source direction costing method...
    struct PixelAux
    {
     real32 mult; // Multiplier of term with albedo in, -k*sqrt(1-(u*L)^2). (Always negative.)
     real32 irr;
     real32 irrSqr; // Irradiance squared.
     real32 c; // Added on constant, -k(u*L)I.
     real32 lowAlbCost; // Base cost when albedo is less than irradiance.
    }; // C = (mult*sqrt(a^2 - irrSqr) + c)/a when a^2>irrSqr, lowAlbCost + err otherwise
    
   // Cost range structure, used in calculation of light source cost for a pixel...
    struct CostRange
    {
     real32 minA;
     real32 maxA;
     nat32 depth;
     
     real32 minCost;
     real32 maxCost;
     
     void CalcCost(ds::Array<PixelAux> & tAux,nat32 length,real32 lowAlbErr); // Fills in the cost range.
     
     bit operator < (const CostRange & rhs) const {return minCost < rhs.minCost;}
    };
    
   // Method that returns the cost of assigning a given light direction to a 
   // given segment - the segment is given as a range in an array of Pixel's,
   // an array of PixelAux is provided that is at least as long.
   // (Does not cap the cost.)
   // Also given a priority queue of cost ranges, which it might make larger, and
   // a recursion depth.
   // Uses a divide and conquer recursive approach
    real32 SegLightCost(const bs::Normal & lightDir,nat32 recDepth,
                        const ds::Array<Pixel> & data,nat32 startInd,nat32 length,
                        ds::Array<PixelAux> & tAux,ds::PriorityQueue<CostRange> & tWork,
                        real32 * bestAlbedo = null<real32*>());
   
   // Returns the cost of a given albedo using the data structures used by SegLightCost.
    real32 CalcCost(real32 albedo,ds::Array<LightDir::PixelAux> & tAux,nat32 length);
};

//------------------------------------------------------------------------------
 };
};
#endif
