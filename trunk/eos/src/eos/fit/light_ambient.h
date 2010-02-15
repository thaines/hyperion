#ifndef EOS_FIT_LIGHT_AMBIENT_H
#define EOS_FIT_LIGHT_AMBIENT_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

/// \file light_ambient.h
/// Provides a tool for estimating the ambient light level of an image given
/// a Fisher distribution for surface orientation for each pixel.

#include "eos/types.h"
#include "eos/time/progress.h"
#include "eos/bs/geo3d.h"
#include "eos/math/stats_dir.h"
#include "eos/ds/arrays.h"
#include "eos/ds/priority_queues.h"
#include "eos/svt/field.h"

namespace eos
{
 namespace fit
 {
//------------------------------------------------------------------------------
/// Estimates the ambient (A) term for an image under the presumed lighting 
/// equation I = a(L*n) + aA. The pixels are provided with Fisher distributions
/// for surface orientation, and segments are provided in which constant
/// albedo (a) is assumed.
/// Outputs the albedo result as well.
class EOS_CLASS LightAmb
{
 public:
  /// &nbsp;
   LightAmb();
   
  /// &nbsp;
   ~LightAmb();


  /// Sets the 3 non-optional inputs - the segmentation, the irradiance and the
  /// surface orientation Fisher distributions.
   void SetData(svt::Field<nat32> seg,svt::Field<real32> irr,svt::Field<math::Fisher> dir);
   
  /// Sets the light source direction - the L of the lighting equation.
   void SetLightDir(const bs::Normal & ld);
   
  /// Sets the albedo range to consider, noting that we presume a light source
  /// of strength 1 with no falloff. Defaults to 0.001 to 1.5
   void SetAlbRange(real32 min,real32 max);

  /// Sets the ambient range to consider, noting that we presume a light source
  /// of strength 1 with no falloff. Defaults to 0.0 to 0.75
   void SetAmbRange(real32 min,real32 max);
   
  /// Sets the maximum cost per segment per pixel, relative to the minimum cost
  /// of that segment - used to cap influence so no one segment biases things 
  /// too strongly. Defaults to 0.1
   void SetSegCapPP(real32 maxCost);
   
  /// Sets the irradiance errors standard deviation, this only matters for
  /// calculating error when albedo is less than irradiance, as presumed too 
  /// small to matter in other situations.
  /// Default is 1/128, i.e. 2 values for a 2^8 level image.
   void SetIrrErr(real32 sd);
   
  /// Sets the thresholding for pruning segments - if the information in a
  /// segment gets a correlation less than this it is ignored. Segments with 
  /// constant colour are ignored anyway for not having a standard deviation.
  /// Defaults to 0.1
   void SetPruneThresh(real32 cor);
   
  /// Sets the number of subdivisions, one for the ambient term, the other for
  /// the albedo term, both default to 7.
   void SetSubdivs(nat32 ambient,nat32 albedo);


  /// &nbsp;
   void Run(time::Progress * prog = null<time::Progress*>());

  
  /// Returns the optimal ambient term.
   real32 BestAmb() const {return bestAmbient;}

  /// Returns the number of segments.
   nat32 SegmentCount() const {return albedo.Size();}
   
  /// Returns the albedo for a given segment number.
   real32 SegmentAlbedo(nat32 s) const {return albedo[s];}

  /// Returns the correlation for a given segment number - this was used to
  /// prune segments with insufficient info.
  /// 0..1 except when its less than 0 to indicate a segment where its undefined.
   real32 SegmentCor(nat32 s) const {return cor[s];}


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::fit::LightAmb";}


 private:
  // Parameters...
   bs::Normal lightDir;
   real32 minAlbedo;
   real32 maxAlbedo;
   real32 minAmbient;
   real32 maxAmbient;   
   real32 maxSegCostPP;
   real32 lowAlbErr;
   real32 segPruneThresh;
   nat32 albedoRecDepth;
   nat32 ambientRecDepth;
   
  // Input...
   svt::Field<nat32> seg;
   svt::Field<real32> irr;
   svt::Field<math::Fisher> dir;

  // Output...
   real32 bestAmbient;
   ds::Array<real32> albedo; // Albedo for each segment number.
   ds::Array<real32> cor; // Correlation for each segment.


  // Runtime...
   // Simple function for calcualting the number of computational expensive 
   // steps involved in processing a branch and bound albedo range of depth n...
    inline nat32 DivSteps(nat32 n)
    {
     return (3<<n) - 2; // Don't ask. f(0) = 1, f(n) = 2 + 2*f(n-1) = 2^n + 2^(n+1) - 2
    }
   
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


   // This struct contains the parameters for the function being optimised
   // - i.e. the constants for each pixel...
    struct PixelAux
    {
     real32 s;
     real32 t;
     real32 irr;
     real32 minR; // r value for the minimum, where cost will be minC
     real32 minC; // minimum cost, we subtract this from the function so 0 is the minimum cost.
    }; // C = s*r + t*sqrt(1-r^2), where r = I/a + A (I = irr, a = albedo, A = ambient.)
   
   
   // Structure used to find the optimum ambient value...
    struct AmbRange
    {
     real32 minAmbient;
     real32 maxAmbient;
     nat32 depth;

     real32 lowMinCost;
     real32 highMinCost;

     bit operator < (const AmbRange & rhs) const {return lowMinCost < rhs.lowMinCost;}
    };
   
   // Structure used to find the optimum albedo...
    struct AlbRange
    {
     real32 minAlbedo;
     real32 maxAlbedo;
     nat32 depth;
     
     real32 lowMinCost;
     real32 highMinCost;
     
     bit operator < (const AlbRange & rhs) const {return lowMinCost < rhs.lowMinCost;}
    };


   // Given various details this fills in an AmbRange's lowMinCost and 
   // highMinCost values - simply loops the relevant segments and sums it all 
   // up...
    void AmbRangeCost(AmbRange & amb,const ds::Array<PixelAux> & pixel,
                      const ds::Array<nat32> & pixelOffset,ds::PriorityQueue<AlbRange> & work);
   
   // Given an ambient value this returns its cost. It also outputs albedo values
   // into a segment sized array of reals...
    real32 AmbCost(real32 amb,ds::Array<real32> & albedo,const ds::Array<PixelAux> & pixel,
                   const ds::Array<nat32> & pixelOffset,ds::PriorityQueue<AlbRange> & work);


   // Given a segments details and an ambient range this uses branch and bound
   // to find the range of possible optimal costs...
   // (outHigh is tight, outLow can be lower than it could be tightened to, but
   //  as used for simply choosing the order of analysis at the higher level 
   //  this doesn't matter so much, well, some computation is wasted but a tighter
   //  bound would probably cost more overall.)
    void SegCostRange(real32 lowAmb,real32 highAmb,
                      real32 & outLow,real32 & outHigh,
                      const ds::Array<PixelAux> & pixel,nat32 start,nat32 size,
                      ds::PriorityQueue<AlbRange> & work);
  
   // Given a segments details and an ambient value this uses branch and bound
   // to find the optimal cost, and associated albedo value...
   // Returns the optimal cost, optinally sticks the albedo into a given pointer.
    real32 SegCost(real32 amb,const ds::Array<PixelAux> & pixel,nat32 start,nat32 size,
                   ds::PriorityQueue<AlbRange> & work,real32 * albedo = null<real32*>());


   // Method that is given a single point in the sampling space - returns the 
   // cost for that point (For a specific segment.)...
    real32 Cost(real32 amb,real32 alb,const ds::Array<PixelAux> & pixel,nat32 start,nat32 size);

   // Method that is given a fixed ambient and albedo range which outputs a cost
   // range for the minimum cost of a segment...
   // (Needs a whole host of data.)
    void CostRangeAlb(real32 amb,real32 lowAlb,real32 highAlb,
                      real32 & outLow,real32 & outHigh,
                      const ds::Array<PixelAux> & pixel,nat32 start,nat32 size);

   // Method that is given an ambient range and a fixed albedo which outputs a cost
   // range for the minimum cost of a segment...
   // (Needs a whole host of data.)
    void CostRangeAmb(real32 lowAmb,real32 highAmb,real32 alb,
                      real32 & outLow,real32 & outHigh,
                      const ds::Array<PixelAux> & pixel,nat32 start,nat32 size);
   
   // Method that is given an ambient and albedo range and outputs a cost
   // range for the minimum cost of a segment...
   // (Needs a whole host of data.)
    void CostDualRange(real32 lowAmb,real32 highAmb,
                       real32 lowAlb,real32 highAlb,
                       real32 & outLow,real32 & outHigh,
                       const ds::Array<PixelAux> & pixel,nat32 start,nat32 size);
};

//------------------------------------------------------------------------------
 };
};
#endif
