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
#include "eos/ds/arrays.h"
#include "eos/ds/arrays_resize.h"
#include "eos/svt/field.h"


namespace eos
{
 namespace fit
 {
//------------------------------------------------------------------------------
/// Light source direction estimation based on a bayesian/sampling model - 
/// uses stereopsis as its input.
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
  /// of strength 1 with no falloff. Defaults to 0.001 to 1.5
   void SetAlbRange(real32 min,real32 max);
  
  /// Sets the presumed ambient term, defaults to 0.
   void SetAmbient(real32 ambient);
   
  /// Sets the maximum cost per segment per pixel, relative to the minimum cost
  /// of that segment - used to cap influence so no one segment biases things 
  /// too strongly. Defaults to 0.1
   void SetSegCapPP(real32 maxCost);
   
  /// Sets the irradiance errors standard deviation, this only matters for
  /// calculating error when albedo is less than irradiance, as too small to
  /// consider in other situations.
  /// Default is 1/128, i.e. 2 values for a 2^8 colour image.
   void SetIrrErr(real32 sd);
   
  /// Sets the thresholding for pruning segments - if the information in a
  /// segment gets a correlation less than this it is ignored. Segments with 
  /// constant colour are ignored anyway for not having a standard deviation.
  /// Defaults to 0.1
   void SetPruneThresh(real32 cor);
   
  /// Sets the number of subdivision of the fit::SubDivSphere class used to 
  /// generate light source directions to sample in the first place and then
  /// the number of further subdivisions it does around the best value to refine
  /// the direction.
  /// Defaults to 1 for initial, 3 for further.
  /// (Further equates to around 18 samples per level, so 3 is 54 extra samples,
  /// whilst 1 initial is 25 samples to start with. (0 initial is 8, 2 is 625.)
   void SetSampleSubdiv(nat32 subdiv,nat32 furtherSubdiv);
  
  /// Sets the recursion depth when finding the optimal albedo value via
  /// branch and bound. Defaults to 7.
   void SetRecursion(nat32 depth);
  
  
  /// &nbsp;
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Returns the calculated optimal light source direction.
   const bs::Normal & BestLightDir() const;
   
  /// Returns the number of directions sampled.
   nat32 SampleSize() const;
   
  /// Returns the direction for sample i.
   const bs::Normal & SampleDir(nat32 i) const;

  /// Returns the cost for sample i.
   real32 SampleCost(nat32 i) const;
   
  /// Returns the number of segments.
   nat32 SegmentCount() const {return albedo.Size();}
   
  /// Returns the albedo for a given segment number.
   real32 SegmentAlbedo(nat32 s) const {return albedo[s];}

  /// Returns the correlation for a given segment number - this was used to
  /// prune segments with insufficient info.
  /// 0..1 except when its less than 0 to indicate a segment where its undefined.
   real32 SegmentCor(nat32 s) const {return cor[s];}


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::fit::Fisher";}


 private:
  // Parameters...
   real32 minAlbedo;
   real32 maxAlbedo;
   real32 ambient;
   real32 maxSegCostPP;
   real32 lowAlbErr;
   real32 segPruneThresh;
   nat32 subdiv;
   nat32 furtherSubdiv;
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
    nat32 index;
   };
   ds::ArrayResize<LightCost> lc; // Cost for each light source direction - for diagnostics really.
   
   ds::Array<real32> albedo; // Albedo for each segment number.
   ds::Array<real32> cor; // Correlation for each segment.


  // Runtime...
   // Structure for storing a pixel - its irradiance and distribution on surface orientation...
   // (These are stored by segment, hence the duplication from the fields.)
    struct Pixel
    {
     real32 irr;
     math::Fisher dir;
    };
    
   // Structure for storing complimentary info to Pixel, used for the light source direction costing method...
   // (Info specific to the light source direction being sampled.)
    struct PixelAux
    {
     real32 s;
     real32 t;
     real32 irr;
     real32 minR; // r value for the minimum, where cost will be minC
     real32 minC; // minimum cost, we subtract this from the function so 0 is the minimum cost.
    }; // C = s*r^2 + t*sqrt(1-r^2), where r = I/a + A (I = irr, a = albedo, A = ambient.)
    
   // Cost range structure, used in calculation of light source cost for a pixel...
    struct CostRange
    {
     real32 minAlbedo;
     real32 maxAlbedo;
     nat32 depth;
     
     real32 lowMinCost;
     real32 highMinCost;
     
     void CalcCost(ds::Array<PixelAux> & tAux,nat32 length,
                   real32 ambient,real32 lowAlbErr); // Fills in the cost range.
     
     bit operator < (const CostRange & rhs) const {return lowMinCost < rhs.lowMinCost;}
    };
    
   // This is used to calculate the correlation for each segment...
    struct SegValue
    {
     real32 div;
 
     real32 expI; // Expectation of irradiance and axes multiplied by div.
     real32 expX;
     real32 expY;
     real32 expZ;
 
     real32 expSqrI; // As above but squared (Before calcaulting expectation).
     real32 expSqrX;
     real32 expSqrY;
     real32 expSqrZ;
 
     real32 expIrrX; // Expectation multiplied by div of irradiance multiplied by each of the axes.
     real32 expIrrY;
     real32 expIrrZ;
    };


   // Method that returns the cost of assigning a given light direction to a 
   // given segment - the segment is given as a range in an array of Pixel's,
   // an array of PixelAux is provided that is at least as long.
   // (Does not cap the cost.)
   // Also given a priority queue of cost ranges, which it might make larger, and
   // a recursion depth.
   // Uses branch and bound.
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
