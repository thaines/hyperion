#ifndef EOS_SFS_SFS_BP_H
#define EOS_SFS_SFS_BP_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file sfs_bp.h
/// Does shape from shading with belief propagation.

#include "eos/types.h"

#include "eos/time/progress.h"
#include "eos/math/stats_dir.h"
#include "eos/svt/field.h"
#include "eos/bs/geo3d.h"
#include "eos/ds/arrays2d.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
/// This uses a hierachical belief propagation algorithm to solve the
/// Shape-from-shading problem. Uses Fisher-Bingham distributions to handle
/// orientation.
class EOS_CLASS SfS_BP
{
 public:
  /// &nbsp;
   SfS_BP();

  /// &nbsp;
   ~SfS_BP();


  /// Sets the size of the image. After calling this all variables will be left default.
   void SetSize(nat32 width,nat32 height);
   
  /// Sets the horizontal relationship between two pixels, in terms of the concentration
  /// of the Fisher distribution that expresses the probability of them being different.
  /// The pixels are (x,y) and (x+1,y).
  /// Default is 0, which you probably don't want.
   void SetHoriz(nat32 x,nat32 y,real32 k);

  /// Sets the vertical relationship between two pixels, in terms of the concentration
  /// of the Fisher distribution that expresses the probability of them being different.
  /// The pixels are (x,y) and (x,y+1).
  /// Default is 0, which you probably don't want.
   void SetVert(nat32 x,nat32 y,real32 k);
   
  /// Sets the probability of a pixel to be any arbitary FisherBingham distribution.
  /// Default is the uniform distribution.
   void SetDist(nat32 x,nat32 y,const math::FisherBingham & fb);
   
  /// Multiplies the current dstribution of the given pixel with another distribution.
  /// Useful for conveniantly combining information from multiple sources, say the SfS
  /// cone constraint and surface orientation information derived from stereo.
   void MultDist(nat32 x,nat32 y,const math::Fisher & f);
   
  /// Multiplies the current dstribution of the given pixel with another distribution.
  /// Useful for conveniantly combining information from multiple sources, say the SfS
  /// cone constraint and surface orientation information derived from stereo.
   void MultDist(nat32 x,nat32 y,const math::Bingham & b);
   
  /// Multiplies the current dstribution of the given pixel with another distribution.
  /// Useful for conveniantly combining information from multiple sources, say the SfS
  /// cone constraint and surface orientation information derived from stereo.
   void MultDist(nat32 x,nat32 y,const math::FisherBingham & fb);


  /// Given various known parameters of the lambertian shading formula this sets the
  /// distribution of the given pixel accordingly.
  /// The k variable is a indication of confidence, higher making the cone constraint stronger.
  /// The length of light is taken into account.
  /// Handles albedo being set to 0 by using the uniform distribution.
   void SetCone(nat32 x,nat32 y,real32 irr,real32 albedo,const bs::Normal & light,real32 k);
   
  /// Identical to SetCone, except you give 2d fields covering the entire image for each parameter.
  /// It calls SetSize internally with the correct parameters.
   void SetCone(const svt::Field<real32> & irr,const svt::Field<real32> & albedo,const svt::Field<bs::Normal> & light,const svt::Field<real32> & k);
   
  /// Sets the concentration of the Fisher distributions representing the relationships between pixels,
  /// note that the last column for horizontal and last row for vertical will be ignored.
   void SetHorizVert(const svt::Field<real32> & horiz,const svt::Field<real32> & vert);
   
  /// Multiplies the current distributions with the given field of distributions.
   void MultDist(const svt::Field<math::Fisher> & pr);

  /// Multiplies the current distributions with the given field of distributions.
   void MultDist(const svt::Field<math::Bingham> & pr);
  
  /// Multiplies the current distributions with the given field of distributions.
   void MultDist(const svt::Field<math::FisherBingham> & pr);
   

  /// Sets the iterations done for each level of the hierachy.
  /// Note that it does the checkerboard optimisation, so only half the pixels
  /// are updated each iteration.
  /// Defaults to 8.
  /// (Also allows you to cap the number of levels, should never need to touch this.)
   void SetIters(nat32 iters,nat32 levels = 32);
   
  /// Disables calculation of maximum direction - saves time but you will only 
  /// be able to get distributions out.
   void DisableNorms();
   
  /// Runs the algorithm. Expect hair to go grey.
   void Run(time::Progress * prog = null<time::Progress*>());
   
   
  /// &nbsp;
   nat32 Width() const {return dist.Width();}

  /// &nbsp;
   nat32 Height() const {return dist.Height();}

  /// Extracts the final distribution for a given pixel.
   void GetDist(nat32 x,nat32 y,math::FisherBingham & out) const;

  /// Extracts the final distribution map.
   void GetDist(svt::Field<math::FisherBingham> & out) const;
   
  /// Returns a direction of maximum probability for the given pixel, extracted
  /// from the distribution.
   void GetDir(nat32 x,nat32 y,bs::Normal & out) const;
   
  /// Extracts a needle map.
   void GetDir(svt::Field<bs::Normal> & out) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  nat32 iters;
  nat32 maxLevels;
  bit doML; // If true fill in distMax, else don't - a time saver if not req.

  ds::Array2D<math::FisherBingham> data; // Input probability distribution for each pixel.
  ds::Array2D<real32> horiz; // Horizontal concentration parameters of equality.
  ds::Array2D<real32> vert; // Vertical concentration parameters of equality.

  ds::Array2D<math::FisherBingham> dist; // Output distributions, as calculated by bp.
  ds::Array2D<bs::Normal> distMax; // Most likelly directions of output distributions - a needle map.
  
  
  // Helper structure, used by Run()...
   struct MsgSet
   {
    math::FisherBingham data; // Data term.
    math::FisherBingham in[4]; // The incomming message for each direction. (+ve x,+ve y,-ve x,-ve y)
    real32 send[4]; // If negative it shouldn't send a message in the given direction, if positive it should, and its the concentration to use.
   };
};

//------------------------------------------------------------------------------
/// Nicer version of SfS_BP, provides less visible functionality but gives an
/// interface more consistant with the other SfS algorithms avaliable.
class EOS_CLASS SfS_BP_Nice
{
 public:
  /// &nbsp;
   SfS_BP_Nice();
   
  /// &nbsp;
   ~SfS_BP_Nice();


  /// Sets the image to use.
   void SetImage(const svt::Field<real32> & image);

  /// Sets an albedo map to use.
   void SetAlbedo(const svt::Field<real32> & albedo);

  /// Sets the light source direction, this comes as a normal that
  /// points towards the light at its infinite position.
   void SetLight(bs::Normal & norm);


  /// Sets the parameters to use for gradient calculation.
  /// \param blur  Strength of gaussian blur used on image before gradient calculation, 0 to disable. Defaults to sqrt(2)
  /// \param length Maximum diffusion path length, essentially sets algorithm run time. Defaults to 8.
  /// \param exp Exponent of probability weights, higher makes the gradient more directed but more sensative to noise. Default of 12.
  /// \param stopChance Chance of a path stopping after each step. Defaults to 0, which disables is and makes it slightly faster.
   void SetParasGrad(real32 blur,nat32 length,real32 exp,real32 stopChance);
   
  /// Sets the core algorithm parameters.
  /// \param simK Similarity between adjacent pixels, as concentration of Fisher distribution of convolution. Defaults to 4.5
  /// \param coneK Concentration parameter for the cone distribution. Default of 16.
  /// \param fadeTo Multiplier of coneK that it fades to as it faces the light. Default of 0.
  /// \param iters Number of iterations to do per level. Defaults to 8.
   void SetParasCore(real32 simK,real32 coneK,real32 fadeTo,nat32 iters);
   
  /// Sets the parameters which provide the extra information that allows it to actually work.
  /// \param gradDisc For application of gradient term, adds a disc term to make it prefer a normal in the disc of the gradiant.
  ///                 Default of 4.
  /// \param gradBias For application of gradient term, adds a Fisher distribution on the cone in the direction of the gradient.
  ///                 Essentially chooses if its concave or convex, negative for the first, positive for the latter.
  ///                 Default of 4.
  /// \param borderK Border strength, for a border constraint as a Fisher distribution perpendicular to the camer view direction.
  ///                Default of 0.
  /// \param project If true ends the algorithm by projecting directly onto the cone, otherwise does nothing as is default.
   void SetParasExtra(real32 gradDisc,real32 gradBias,real32 borderK,bit project);


  /// Blah.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Extracts the calculated needle map.
   void GetNeedle(svt::Field<bs::Normal> & out) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  // Parameters...
   real32 blur;

   nat32 gradLength;
   real32 gradExp;
   real32 gradStopChance;

   real32 simK;
   real32 coneK;
   real32 fadeTo;

   real32 gradDisc; 
   real32 gradBias; 
   real32 borderK;

   bit project;
   nat32 iters;


  // Inputs...
   svt::Field<real32> image;
   svt::Field<real32> albedo;
   bs::Normal toLight;
  
  // Outputs...
   ds::Array2D<bs::Normal> result;
};

//------------------------------------------------------------------------------
/// Impliments a more sophisticated method of extracting a needle map from a 
/// SfS_BP objects distributions. Extracts both maximas where applicable from 
/// each Fisher-Bingham distribution and uses belief propagation to select 
/// between them, with a relative cost of the two maximas and a matching cost 
/// based on the angle between adjacent directions. After this process all 
/// locations with no maxima are repeatedly set to the average of there
/// neighbours until convergance.
class EOS_CLASS NeedleFromFB
{
 public:
  /// &nbsp;
   NeedleFromFB();

  /// &nbsp;
   ~NeedleFromFB();
   
   
  /// Fills the distribution map ready for processing.
   void Fill(const svt::Field<math::FisherBingham> & source);
  
  /// Fills the distribution map from a SfS_BP.
   void Fill(const SfS_BP & source);
   
  /// Sets the parameters - cost multiplier for the angle between directions in
  /// radians and momentum term for BP solver used.
   void SetParas(real32 angMult = 1.0,real32 momentum = 0.01);
   
   
  /// Runs and calculates the direction for each pixel in the field.
   void Run(time::Progress * prog = null<time::Progress*>());
   
  /// Extracts a needle.
   void GetDir(nat32 x,nat32 y,bs::Normal & out) const;
   
  /// Extracts a needle map.
   void GetDir(svt::Field<bs::Normal> & out) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  real32 momentum; // For belief propagation solver, to accelerate convergance.
  real32 angMult; // Multiplies the angle between directions in radians to get the cost.
  
  ds::Array2D<math::FisherBingham> in;
  ds::Array2D<bs::Normal> out;
  
  // Helper structure...
   typedef bs::Normal DirPair[2];
};

//------------------------------------------------------------------------------
/// An improved version of SfS_BP_Nice, this version uses a more advanced 
/// selection strategy when choosing between maxima on the distribution.
/// This allows it to get away with less assumptions and do a better job.
/// As a bonus it has less parameters.
class EOS_CLASS SfS_BP_Nice2
{
 public:
  /// &nbsp;
   SfS_BP_Nice2();
   
  /// &nbsp;
   ~SfS_BP_Nice2();


  /// Sets the image to use.
   void SetImage(const svt::Field<real32> & image);

  /// Sets an albedo map to use.
   void SetAlbedo(const svt::Field<real32> & albedo);

  /// Sets the light source direction, this comes as a normal that
  /// points towards the light at its infinite position.
   void SetLight(bs::Normal & norm);

   
  /// Sets the core algorithm parameters.
  /// \param simK Similarity between adjacent pixels, as concentration of Fisher distribution of convolution. Defaults to 4.5
  /// \param coneK Concentration parameter for the cone distribution. Default of 16.
  /// \param fadeTo Multiplier of coneK that it fades to as it faces the light. Default of 1.
  /// \param iters Number of iterations to do per level. Defaults to 8.
   void SetParasCore(real32 simK,real32 coneK,real32 fadeTo,nat32 iters);
   
  /// Sets parameters for the boundary constraint.
   void SetParasBound(real32 k = 2.0,nat32 length = 8,real32 exp = 6.0);
   
  /// Sets parameters for the gradient constraint, as implimented with a disc distribution.
   void SetParasGrad(real32 k = 0.0,nat32 length = 8,real32 exp = 6.0);

  /// Sets the extraction method parameters - angle multiplier and momentum.
  /// Defaults of 1.0 and 0.01 respectivly.
   void SetParasExtract(real32 angMult,real32 momentum);


  /// Blah.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Extracts the calculated needle map.
   void GetNeedle(svt::Field<bs::Normal> & out) const;


  /// &nbsp;
   cstrconst TypeString() const;
 
 
 private:
  // Parameters...
   real32 simK;
   real32 coneK;
   real32 fadeTo;
   nat32 iters;
   
   real32 boundK;
   nat32  boundLength;
   real32 boundExp;
   
   real32 gradK;
   nat32  gradLength;
   real32 gradExp;

   real32 angMult;
   real32 momentum;


  // Inputs...
   svt::Field<real32> image;
   svt::Field<real32> albedo;
   bs::Normal toLight;
  
  // Outputs...
   ds::Array2D<bs::Normal> result;
};

//------------------------------------------------------------------------------
 };
};
#endif
