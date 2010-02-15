#ifndef EOS_STEREO_REFINE_NORM_H
#define EOS_STEREO_REFINE_NORM_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file refine_norm.h
/// Uses gaussian integration to combine orientation and disparity information.

#include "eos/types.h"
#include "eos/math/mat_ops.h"
#include "eos/bs/geo3d.h"
#include "eos/stereo/dsi.h"
#include "eos/svt/field.h"
#include "eos/cam/files.h"
#include "eos/time/progress.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// This is given a DSI and a field of normals, plus camera calibration so it 
/// can equate the two.
/// It then does belief propagation with gaussians to effectivly integrate the
/// needle map whilst considering the dsi information provided, to produce a
/// continuous disparity map.
class EOS_CLASS RefineNorm
{
 public:
  /// &nbsp;
   RefineNorm();

  /// &nbsp;
   ~RefineNorm();


  /// Sets the parameters.
  /// \param iters How many iterations to do, defaults to 100.
  /// \param dsiSd Multiplier of the correct standard deviation for each dsi entry.
  ///              Defaults to 1.0.
  /// \param costBase The weight of each disparity going into a 
  ///                 pixels distribution is multiplied by e to the power of 
  ///                 the negative of this multiplied by the cost, where the 
  ///                 costs have been offset to make the lowest zero.
  ///                 Defaults to 1.0.
  /// \param normVar Standard deviation of normals in disparity space after 
  ///                projection. Defaults to 0.1.
  /// \param maxD The maximum difference in disparity to allow from the normal 
  ///             map. Required as steep normals can produce very large values,
  ///             which then dominate. Defaults to 5.0
   void Set(nat32 iters,real32 dsiSd,real32 costBase,real32 normVar,real32 maxD);

  /// Sets a DSC used to assign similarity scores for adjacent pixels to
  /// influence the flow of information between areas of the image.
  /// Uses a soft threshold with a sigmoid function, you set the point where the
  /// threshold drops it to half strength and a multiplier of the width of the
  /// sigmoid. (Standard 1/(1+exp(-t)) variety.)
  /// Optional.
   void Set(const DSC & dsc,real32 threshold = 1.0,real32 width = 0.25);

  /// Sets the DSI. The DSI must have width assigned to each disparity entry.
   void Set(const DSI & dsi);
   
  /// Sets the normal for each pixel.
   void Set(const svt::Field<bs::Normal> & needle);

  /// Sets a segmentation, information is not passed between different segments.
  /// Optional.
   void Set(const svt::Field<nat32> & seg);
   
  /// Sets the camera pair that defines the relationship between orientation
  /// and dsi.
   void Set(const cam::CameraPair & pair);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Extracts the disparity map.
   void Disp(svt::Field<real32> & out) const;


  /// &nbsp;
   static cstrconst TypeString() {return "eos::stereo::BeliefPropSFS";}


 private:
  // In...
   nat32 iters;
   real32 dsiSd;
   real32 costBase;
   real32 normVar;
   real32 maxD;

   const DSC * dsc;
   real32 dsc_threshold;
   real32 dsc_width;

   const DSI * dsi;   

   svt::Field<bs::Normal> needle;
   svt::Field<nat32> seg;
   cam::CameraPair pair;

  // Out...
   ds::Array2D<real32> disp;
   
  // Runtime...
   struct PreCalc
   {
    bit pass[4]; // true if a message is passed, false if not.
    real32 invSd[4]; // For each direction, calculated seperatly hence requirement to cache.
   };
};

//------------------------------------------------------------------------------
/// This interface defines a source of disparity information, expressed as 
/// normal distributions. Will always feed off a DSI, as well as any implimentor
/// specific information, to do this.
class EOS_CLASS GaussianDisp : public Deletable
{
 public:
  // &nbsp;
   ~GaussianDisp();


  /// Sets the DSI.
   void SetDSI(const DSI & dsi);
   
  /// Returns the disparity maps width.
   nat32 Width() const {return dsi->Width();}
  
  /// Returns the disparity maps height.
   nat32 Height() const {return dsi->Height();}


  /// This must output the mean and inverse standard deviation of the disparity
  /// for each pixel.
  /// Returns true if it is defined, false if it is not defined.
   virtual bit GetDisp(nat32 x,nat32 y,real32 & mean,real32 & invSd) const = 0;


 protected:
  /// &nbsp;
   const DSI * dsi;
};

//------------------------------------------------------------------------------
/// This interface defines a source of disparity differential information, in
/// gaussian form. Has built in support for converting orientation and depth to
/// disparity differentials. Will generally eat a needle map, plus a DSI to 
/// compensate for perspective.
class EOS_CLASS GaussianDispDiff : public Deletable
{
 public:
  /// &nbsp;
   GaussianDispDiff();

  /// &nbsp;
   ~GaussianDispDiff();


  /// Sets the camera pair, so it can convert from 3D orientation to disparity 
  /// differential.
   void SetPair(const cam::CameraPair & pair);

  /// This must output a difference and inverse standard deviation for that 
  /// difference for every orientation at every pixel. (Though orientations off 
  /// the edge will be ignored, but still requested.)
  /// Dir is 0 = +ve x, 1 = +ve y, 2 = -ve x, 3 = -ve y.
  /// Returns true if it is defined, false if it is not defined.
  /// Note that not defined should be avoided, as that will prevent all 
  /// propagation of information throught the route in question.
  /// Supplied with the disparity obtained from the GaussianDisp, so it can 
  /// compensate for perspective.
   virtual bit GetDispDiff(nat32 x,nat32 y,nat32 dir,real32 disp,real32 & diff,real32 & invSd) const = 0;


 protected:
  /// &nbsp;
   cam::CameraPair pair;
  
  /// Helper method for implimentors - given a location, disparity, orientation 
  /// and direction this outputs the relevant disparity differential.
  /// The normal does not need to be normalised.
  /// Assumes, as is necesary, that the camera is rectified.
   real32 NormToDiff(nat32 x,nat32 y,nat32 dir,real32 disp,const bs::Normal & norm) const;


 private:
  // Support information derived from the camera pair in the Set method.
  // Used by NormToDiff.
   math::Vect<4,real64> leftCentre;
   math::Mat<4,3,real64> inverseLP;
   math::Mat<3,3,real64> rectLeft;
   math::Mat<3,3,real64> rectRight;
    
  // Cache for NormToDiff, to optimise the call for Triangulate for each pixel/disparity...
   mutable nat32 cacheX;
   mutable nat32 cacheY;
   mutable real32 cacheD;
   mutable math::Vect<4,real64> cachePos;
};

//------------------------------------------------------------------------------
/// This class takes a source of gaussian disparity information and a source of
/// Gaussian disparity differential information and produces a final disparty
/// map, using the inf::IntegrateBP algorithm.
class EOS_CLASS GaussRefine
{
 public:
  /// &nbsp;
   GaussRefine();

  /// &nbsp;
   ~GaussRefine();


  /// Sets the source of disparity information.
   void Set(const GaussianDisp & gd);
  
  /// Sets the source of disparity differential information.
   void Set(const GaussianDispDiff & gdd);
   
  /// Sets the algorithms parameter, simply the number of iterations to do.
  /// Defaults to 100.
   void Set(nat32 iters);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Extracts the disparity mask, true where good disparities exist, false 
  /// where they are bad.
   void MaskMap(svt::Field<bit> & out) const;
  
  /// Extracts an indiviudal mask entry.
   bit Mask(nat32 x,nat32 y) const;

  /// Extracts the disparity map.
   void DispMap(svt::Field<real32> & out) const;
  
  /// Extracts an individual disparity.
   real32 Disp(nat32 x,nat32 y) const;


  /// &nbsp;
   static cstrconst TypeString() {return "eos::stereo::GaussRefine";}


 private:
  // Input...
   const GaussianDisp * gd;
   const GaussianDispDiff * gdd;
   nat32 iters;

  // Output...
   ds::Array2D<bit> mask;
   ds::Array2D<real32> disp;
};

//------------------------------------------------------------------------------
/// An implimentor of GaussianDisp. This version simply fits a Gaussian to the 
/// set of disparity values outputed by the DSI for each pixel.
/// It has two modes of operation, the obvious fitting, and a fitting where the
/// mean is forced to be the best match in the DSI for a given pixel, with the
/// standard deviation calculated relative to this.
class EOS_CLASS GaussianDispSimple : public GaussianDisp
{
 public:
  /// &nbsp;
   GaussianDispSimple();
  
  /// &nbsp;
   ~GaussianDispSimple();


  /// Sets the mode, false to do a basic Gaussian fit, true to force the mean as
  /// the centre of the highest scoring match. Defaults to false.
   void SetMode(bit mode);
   
   
  /// &nbsp;
   bit GetDisp(nat32 x,nat32 y,real32 & mean,real32 & invSd) const;
  

  /// &nbsp;
   static cstrconst TypeString() {return "eos::stereo::GaussianDispSimple";}
   
   
 private:
  bit mode;
};

//------------------------------------------------------------------------------
/// An implimentor of GaussianDisp. This version additionally uses information 
/// from the images, to measure how 'slidable' disparity entrys are, as well as
/// the DSI information. This should produce a better measure of in which areas
/// stereo should be used and in which areas SfS can take over.
///
/// NOT YET IMPLIMENTED
class EOS_CLASS GaussianDispSlide : public GaussianDisp
{
 public:
 
 
 private:

};

//------------------------------------------------------------------------------
/// An implimentor of GaussianDispDiff.
/// You provide a needle map, it then outputs accordingly.
/// The needles can be optionally un-normalised, you provide the inverse 
/// standard deviation for a length of one and it is scaled accordingly.
/// The inverse standard deviation can be additionally scaled by the sigmoid of
/// manhatten distance between pixels, again optionally.
class EOS_CLASS GaussianDispDiffSimple : public GaussianDispDiff
{
 public:
  /// &nbsp;
   GaussianDispDiffSimple();

  /// &nbsp;
   ~GaussianDispDiffSimple();


  /// Sets the needle map. Remember that length matters, and a length of 0 is 
  /// taken to mean 'no information'.
   void SetNeedle(const svt::Field<bs::Normal> & needle);

  /// Sets the inverse standard deviation that equates to a needle of length one,
  /// is multiplied by actual needle length, so a needle of length 2 gets twice 
  /// this standard deviation, for instance.
  /// Defaults to 1.
   void SetInvSd(real32 invSd);

  /// You can optionally call this to set the left image, with parameters for a 
  /// sigmoid applied to manhatten distance between colours.
  /// Adjacent pixels then have there inverse standard deviations multiplied by
  /// the sigmoid.
   void SetImageMod(const svt::Field<bs::ColourLuv> & image,real32 threshold,real32 halflife);


  /// &nbsp;
   bit GetDispDiff(nat32 x,nat32 y,nat32 dir,real32 disp,real32 & diff,real32 & invSd) const;


  /// &nbsp;
   static cstrconst TypeString() {return "eos::stereo::GaussianDispDiffSimple";}


 private:
  svt::Field<bs::Normal> needle;
  real32 baseInvSd;

  svt::Field<bs::ColourLuv> image;
  real32 threshold;
  real32 halflife;
};

//------------------------------------------------------------------------------
 };
};
#endif
