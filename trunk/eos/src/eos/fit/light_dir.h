#ifndef EOS_FIT_LIGHT_DIR_H
#define EOS_FIT_LIGHT_DIR_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

/// \file light_dir.h
/// Given an image of irradiance values, segments and surface orientation 
/// distributions this infers the light source direction.

#include "eos/types.h"
#include "eos/math/stats_dir.h"
#include "eos/bs/geo3d.h"
#include "eos/ds/priority_queues.h"

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


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::math::Fisher";}


 private:
  // Parameters...
   real32 minAlbedo;
   real32 maxAlbedo;
   real32 maxSegCost;
   
  // Input...
   svt::Field<nat32> seg;
   svt::Field<real32> irr;
   svt::Field<math::Fisher> dir;
  
  // Output...
   bs::Normal bestLightDir;


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
     real32 mult; // Multiplier of term with albedo in, -k*sqrt(1-(u*L)^2).
     real32 irrSqr; // Irradaince squared.
     real32 c; // Added on constant, -k(u*L)I.
    }; // C = (mult*sqrt(a^2 - irrSqr) + c)/a
    
   // Cost range structure, used in calculation of light source cost for a pixel...
    struct CostRange
    {
     real32 minA;
     real32 maxA;
     nat32 depth;
     
     real32 minCost;
     real32 maxCost;
     
     void CalcCost(ds::Array<PixelAux> & tAux,nat32 length); // Fills in the cost range.
     
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
                        ds::Array<PixelAux> & tAux,ds::PriorityQueue<CostRange> & tWork);
   
   // Returns the cost of a given albedo using the data structures used by SegLightCost.
    real32 CalcCost(real32 albedo,ds::Array<LightDir::PixelAux> & tAux,nat32 length);
};

//------------------------------------------------------------------------------
 };
};
#endif
