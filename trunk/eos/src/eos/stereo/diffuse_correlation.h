#ifndef EOS_STEREO_DIFFUSE_CORRELATION_H
#define EOS_STEREO_DIFFUSE_CORRELATION_H
//------------------------------------------------------------------------------
// Copyright 2009 Tom Haines

/// \file diffuse_correlation.h
/// Provides advanced correlation capabilities, that weight pixels via diffusion
/// and use colour ranges rather than points.

#include "eos/types.h"
#include "eos/time/progress.h"
#include "eos/bs/luv_range.h"
#include "eos/stereo/dsi.h"


namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// Diffusion weight object - stores the weights associated with going in each
/// direction from each pixel in an image. Has a method to calculate from a
/// LuvRangeImage using a LuvRangeDist.
class EOS_CLASS DiffusionWeight
{
 public:
  /// &nbsp;
   DiffusionWeight();
  
  /// &nbsp;
   ~DiffusionWeight();


  /// Fills in the object from a LuvRangeImage and a LuvRangeDist. It takes the
  /// negative exponential of the distances to get relative weightings, and 
  /// makes an effort for stability. It supports a distance multiplier before 
  /// it does this and respects masks by not sending any weight that way, or off
  /// the image.
   void Create(const bs::LuvRangeImage & img, const bs::LuvRangeDist & dist, real32 distMult = 1.0, time::Progress * prog = null<time::Progress*>());


  /// Returns the weight for a given pixel for a given direction. Note that this
  /// weight will be normalised between the 4 directions, which have the typical
  /// 0=+ve x, 1=+ve y,2=-ve x,3=-ve y coding, unless the pixel is invalid in 
  /// which case they will all be zero.
   real32 Get(nat32 x,nat32 y,nat32 dir) const {return data.Get(x,y).dir[dir];}


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::DiffusionWeight";}


 private:
  struct Weight
  {
   real32 dir[4];
  };
  
  ds::Array2D<Weight> data;
};

//------------------------------------------------------------------------------
/// Given a LuvRangeImage and a scanline number this calculates a slice of 
/// diffusion scores, for a given number of steps.
/// Clever enough to cache storage between runs as long as the image
/// width and step count don't change.
/// Once done each pixel in the scanline will have a normalised set of weights 
/// for surrounding pixels within the given walking distance. Note that this is
/// never going to be that fast. It will always give values of zero to masked or
/// out of bound values.
class EOS_CLASS RangeDiffusionSlice
{
 public:
  /// &nbsp;
   RangeDiffusionSlice();

  /// &nbsp;
   ~RangeDiffusionSlice();
   
   
  /// Creates the data for a diffusion slice - you give it an image, the 
  /// y-coordinate of the slice to calculate, how many steps to walk and a 
  /// diffusion weight object and it constructs the slices diffuson masks, one for
  /// each pixel in the slice.
  /// Will not walk off edges of the image, or into masked off areas.
  /// For weighting from the distances takes the negative exponential of
  /// distance. (And offsets first, for stability.)
   void Create(nat32 y, nat32 steps, const bs::LuvRangeImage & img, const DiffusionWeight & dw, time::Progress * prog = null<time::Progress*>());


  /// Returns the width of the slice.
   nat32 Width() const;
   
  /// Returns the y coordinate associated with the slice.
   nat32 Y() const {return y;}
   
  /// Returns the number of steps of the slice.
   nat32 Steps() const;
   
  /// Given a x-coordinate and a (u,v) window coordinate this returns the weight
  /// - out of range values will return 0.0, defined by abs(u) + abs(v) > steps.
   real32 Get(nat32 x,int32 u,int32 v) const;


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::RangeDiffusionSlice";} 


 private:
  // Storage for state...
   nat32 steps;
   nat32 y;
   
   ds::Array2D<real32> data; // Stores the x coordinate in x and the y coordinate is a linearlisation of the diffusion values. Will not have anything for masked entries.
   ds::Array2D<nat32> offset; // Index from (u+steps,v+steps) to the above linearisation. Only valid when abs(u) + abs(v) <= steps.
};

//------------------------------------------------------------------------------
/// This is given a pair of bs::LuvRangeImage's and RangeDiffusionSlice's,
/// it then calculates the correlation between pixels in the two slices.
/// It also makes use of a LuvRangeDist to calculate the difference between pixels.
/// Simply takes the distances weighted by the diffusion weights, added for the
/// two pixels in question. Due to the adding the result is divided by two when
/// done, the output is then a distance metric.
/// A distance cap is provided - distances are capped at this value, to
/// handle outliers. This is also the value used if either pixel is outside the
/// image or masked.
class EOS_CLASS DiffuseCorrelation
{
 public:
  /// &nbsp;
   DiffuseCorrelation();

  /// &nbsp;
   ~DiffuseCorrelation();


  /// Fills in the valid details - note that all passed in objects must survive
  /// the lifetime of this object.
   void Setup(const bs::LuvRangeDist & dist, real32 distCap, const bs::LuvRangeImage & img1, const RangeDiffusionSlice & dif1, const bs::LuvRangeImage & img2, const RangeDiffusionSlice & dif2);
   
  /// Returns the width of image 1.
   nat32 Width1() const;

  /// Returns the width of image 1.
   nat32 Width2() const;
   
  /// Given two x coordinates this returns their matching cost - note that this
  /// does the correlation and is a slow method call.
   real32 Cost(nat32 x1,nat32 x2) const;
   
  /// Returns the distance cap used.
   real32 DistanceCap() const;


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::DiffuseCorrelation";}


 private:
  const bs::LuvRangeDist * dist;
  real32 distCap;
  
  const bs::LuvRangeImage * img1;
  const RangeDiffusionSlice * dif1;
  const bs::LuvRangeImage * img2;
  const RangeDiffusionSlice * dif2;
};

//------------------------------------------------------------------------------
/// This calculates correlation scores for an image pair, using DiffuseCorrelation,
/// the ultimate result is a list of best matches, i.e. distance minimas for
/// each pixel in both images; it is symmetric in its output.
/// A minima has to be so in both images for it to be accepted.
/// Each match will also include its score and the scores of adjacent pixels, so
/// the positions can be refined beyond discrete coordinates.
class EOS_CLASS DiffusionCorrelationImage
{
 public:
  /// &nbsp;
   DiffusionCorrelationImage();

  /// &nbsp;
   ~DiffusionCorrelationImage();


  /// Sets the distance measure and the left and right pyramids.
  /// The pyramids must have the same height and be constructed with the same
  /// settings. Halfing the width must always be on.
  /// Also sets the distance multiplier - this is only used for the diffusion
  /// when converting from distance to weight, and is not used for the 
  /// correlation.
   void Set(const bs::LuvRangeDist & dist, real32 distMult, const bs::LuvRangePyramid & left, const bs::LuvRangePyramid & right);
   
  /// Sets various parameters - the maximum number of minimas to store for each
  /// pixel and the distance cap to use at the base level, the multiplier 
  /// for it to adjust the distance cap for each higher level, and the multiplier
  /// to get the threshold for prunning output at each hierachy level.
  /// range is how many pixels either side of a maxima to store the correlation
  /// scores for - don't set it too high, range is the diffusion range to use.
   void Set(nat32 minimaLimit = 8, real32 baseDistCap = 1.0, real32 distCapMult = 2.0, real32 distCapThreshold = 0.5, nat32 range = 2, nat32 steps = 5);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());
   
  
  /// Returns the range that te algorithm was run with.
   int32 Range() const;


  /// Extracts how many minima exist for a given left pixel.
  /// Can return 0 if it just doesn't know.
   nat32 CountLeft(nat32 x,nat32 y) const;
   
  /// Extracts the disparity for a given minima i, for the given pixel in the left image.
  /// Note that the minimas will be ordered from best scoring to worst scoring.
   int32 DisparityLeft(nat32 x,nat32 y,nat32 i) const;
   
  /// Extracts the correlation distance score for the given maxima of the given
  /// pixel in the left image at the given offset to the disparity.
  /// The offset must be in [-range,range]
   real32 ScoreLeft(nat32 x,nat32 y,nat32 i,int32 offset) const;
  

  /// Extracts how many minima exist for a given right pixel.
  /// Can return 0 if it just doesn't know.
   nat32 CountRight(nat32 x,nat32 y) const;
   
  /// Extracts the disparity for a given minima i, for the given pixel in the right image.
  /// Note that the minimas will be ordered from best scoring to worst scoring.
   int32 DisparityRight(nat32 x,nat32 y,nat32 i) const;
   
  /// Extracts the correlation distance score for the given minima of the given
  /// pixel in the right image at the given offset to the disparity.
  /// The offset must be in [-range,range]
   real32 ScoreRight(nat32 x,nat32 y,nat32 i,int32 offset) const;


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::DiffusionCorrelationImage";}


 private:
  // Inputs...
   const bs::LuvRangeDist * dist;
   const bs::LuvRangePyramid * left;
   const bs::LuvRangePyramid * right;
  
  // Parameters...
   real32 distMult;
   nat32 minimaLimit;
   real32 baseDistCap;
   real32 distCapMult;
   real32 distCapThreshold;
   nat32 range;
   nat32 steps;

  // Outputs...
   ds::Array<nat32> offsetLeft; // Indexed by y*width + x - gives you the offset into dispLeft to ge thte data for a pixel, and if multiplier by (range*2+1) the offset into scoreLeft. Has an extra value so you can determine size by subtracting from the incrimented index.
   ds::Array<int32> dispLeft;
   ds::Array<real32> scoreLeft;
   
   ds::Array<nat32> offsetRight;
   ds::Array<int32> dispRight;
   ds::Array<real32> scoreRight;
   
  // Runtime...
   struct Match
   {
    int32 y;
    int32 xLeft;
    int32 xRight;
    real32 score;
    
    bit operator < (const Match & rhs) const
    {
     if (y!=rhs.y) return y<rhs.y;
     if (xLeft!=rhs.xLeft) return xLeft<rhs.xLeft;
     return xRight<rhs.xRight;
    }
   };
   
   struct Disp
   {
    int32 d;
    real32 score;
    
    bit operator < (const Disp & rhs) const
    {
     if (!math::Equal(score,rhs.score)) return score < rhs.score;
     return d < rhs.d;
    }
   };
};

//------------------------------------------------------------------------------
/// An actual stereopsis algorithm that uses the diffusion correlation - wraps
/// the various parts in a neat interface and also refines the final location by 
/// fitting a polynoimial. (Note: Whilst I'm sure this could give good results
/// its just too computationally intensive - I havn't tested it as I havn't the
/// patience to let it run on any real input.)
class EOS_CLASS DiffCorrStereo : public DSI
{
 public:
  /// &nbsp;
   DiffCorrStereo();

  /// &nbsp;
   ~DiffCorrStereo();


  /// Sets the image pair to use.
   void SetImages(const svt::Field<bs::ColourLuv> & left,const svt::Field<bs::ColourLuv> & right);

  /// Optionally call to set masks.
   void SetMasks(const svt::Field<bit> & left,const svt::Field<bit> & right);
   
  /// Sets luv range pyramid construction options. All default to true.
   void SetPyramid(bit useHalfX, bit useHalfY, bit useCorners, bit halfHeight);
   
  /// Sets diffusion parameters.
  /// The multiplier for distance when doing diffusion before the negative log
  /// is taken and the number of steps to take - default to 0.1 and 5
   void SetDiff(real32 distMult,nat32 diffSteps);
   
  /// Sets the parameters to do with corelation and its use in the hierachy.
  /// minima limit is the maximum number of minimas to finally output per pixel,
  /// defaults to 8.
  /// baseDistCap is the distance cap at the base level, whilst dispCapMult is 
  /// the multiplier to get from one level to the next reduced resolution level,
  /// and distCapThreshold is the multiplier of the current levels distance cap
  /// to get the threshold to be passed to the next higher resolution level.
  /// They default to 4.0, 2.0 and 0.5 respectivly.
  /// Finally, dispRange is the number of adjacent correlation scores to give
  /// for each match - they are then used for fitting the final gaussian.
  /// also affects the search range around matches passed down from lower 
  /// resolution levels. Defaults to 2.
   void SetCorr(nat32 minimaLimit,real32 baseDistCap,real32 distCapMult,real32 distCapThreshold,nat32 dispRange);
   
  /// Refinement parameters - if doLR it does a left right check,
  /// distCapDifference indicates the distance cap multiplier used to determine
  /// how much below the other minima the best minima has to be to be considered
  /// authorative.
  /// Default to true then 0.25.
   void SetRefine(bit doLR,real32 distCapDifference);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());

  
  /// &nbsp;
   nat32 Width() const;
   
  /// &nbsp;
   nat32 Height() const;
   
  /// &nbsp;
   nat32 Size(nat32 x, nat32 y) const;
   
  /// &nbsp;
   real32 Disp(nat32 x, nat32 y, nat32 i) const;
   
  /// &nbsp;
   real32 Cost(nat32 x, nat32 y, nat32 i) const;
   
  /// &nbsp;
   real32 Prob(nat32 x, nat32 y, nat32 i) const;
  
  /// &nbsp;
   real32 DispWidth(nat32 x, nat32 y, nat32 i) const;


  /// &nbsp;
   void GetDisp(svt::Field<real32> & disp) const;

  /// &nbsp;
   void GetMask(svt::Field<bit> & mask) const;
   

  /// &nbsp;
   cstrconst TypeString() const;


 private:
  // In...
   svt::Field<bs::ColourLuv> left;
   svt::Field<bs::ColourLuv> right;
   svt::Field<bit> leftMask;
   svt::Field<bit> rightMask;

  // Parameters...
   bit useHalfX;
   bit useHalfY;
   bit useCorners;
   bit halfHeight;
   
   real32 distMult;
   nat32 minimaLimit;
   real32 baseDistCap;
   real32 distCapMult;
   real32 distCapThreshold;
   nat32 dispRange;
   nat32 diffSteps;
   
   bit doLR;
   real32 distCapDifference;
   
  
  // Out...
   ds::Array2D<real32> disp; // Infinity means its masked.
};

//------------------------------------------------------------------------------
/// A simple stereopsis refinement algorithm - given a disparity map takes each
/// disparity value and refines its position by running diffusion correlation in
/// a region around the results - if there is no maxima it prunes the value, if
/// there is it does subpixel refinement by fitting a polynomial based on area.
class EOS_CLASS DiffCorrRefine 
{
 public:
  /// &nbsp;
   DiffCorrRefine();

  /// &nbsp;
   ~DiffCorrRefine();


  /// Sets the image pair to use.
   void SetImages(const svt::Field<bs::ColourLuv> & left,const svt::Field<bs::ColourLuv> & right);

  /// Optionally call to set masks.
   void SetMasks(const svt::Field<bit> & left,const svt::Field<bit> & right);

  /// Sets the disparity map to refine.
   void SetDisparity(const svt::Field<real32> & disp);

  /// Optional sets a mask for the disparity map to refine.
   void SetDisparityMask(const svt::Field<bit> & dispMask);


  /// Sets luv range from image construction flags. All default to true.
   void SetFlags(bit useHalfX, bit useHalfY, bit useCorners);
   
  /// Sets diffusion parameters.
  /// The multiplier for distance when doing diffusion before the negative log
  /// is taken, and the number of steps to take - default to 0.01 and 7
   void SetDiff(real32 distMult,nat32 diffSteps);
   
  /// Sets distance related stuff - the distance cap nad the distance difference
  /// required for a minima to be good enough compared to neighbours.
  /// Default to 64.0 and 4.0
   void SetDist(real32 cap,real32 prune);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// &nbsp;
   void GetDisp(svt::Field<real32> & disp) const;

  /// &nbsp;
   void GetMask(svt::Field<bit> & mask) const;


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::DiffCorrRefine";}


 private:
  // Input...
   svt::Field<bs::ColourLuv> left;
   svt::Field<bs::ColourLuv> right;
   svt::Field<bit> leftMask;
   svt::Field<bit> rightMask;
   svt::Field<real32> disp;
   svt::Field<bit> dispMask;
  
  // Parameters...
   bit useHalfX;
   bit useHalfY;
   bit useCorners;
   
   real32 distMult;
   nat32 diffSteps;
   
   real32 cap;
   real32 prune;

  // Output...
   ds::Array2D<real32> out; // I use infinity to indicate masked values.
};

//------------------------------------------------------------------------------
 };
};
#endif
