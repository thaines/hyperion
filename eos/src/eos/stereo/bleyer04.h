#ifndef EOS_STEREO_BLEYER04_H
#define EOS_STEREO_BLEYER04_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file bleyer04.h
/// An implimentation of a stereo algorithm, specifically choosen as #3 on the
/// new middlebury stereo test as of 29/1/06, (Was #2 not that long before.) this
/// being the highest algorithm not marked as anonymous.

#include "eos/types.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"
#include "eos/bs/colours.h"
#include "eos/time/progress.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// An implimentation of 'A layered stereo matching algorithm using image
/// segmentation and global visibility constraints' by Michael Bleyer & Margrit
/// Gelautz, 2004. Should be identical to the paper in question, note there is a
/// difference in the scale of some parameters due to using [0,1] ranged RGB
/// colour components instead of [0,255]. The class is a standard algorithm class,
/// where you construct it, call methods to input the data it requires and then
/// call the run method before finally calling other methods to extract the
/// final results. Results currently incorrect, this message will self destruct
/// when correctness is (finally) obtained.
class EOS_CLASS Bleyer04
{
 public:
  /// &nbsp;
   Bleyer04();

  /// &nbsp;
   ~Bleyer04();


  /// Sets the image pair to operate on, a left and right image. Only method that
  /// must be called before run.
   void SetImages(const svt::Field<bs::ColourRGB> & left,const svt::Field<bs::ColourRGB> & right);

  /// If called stereo override is enabled - you provide a disparity map and
  /// mask to use for bootstrap rather than the embedded algorithm.
  /// These must obviously have the dimensions of the left image.
  /// (The mask will be and-ed with the Setmask method if called.)
   void BootOverride(const svt::Field<real32> & disp,const svt::Field<bit> & mask);

  /// Optionally call to set masks for the two images - masked areas will not be assigned
  /// disparity values or be used in the calculation in general.
   void SetMasks(const svt::Field<bit> & left,const svt::Field<bit> & right);

  /// Sets the maximumn and minimum disparity ranges to concider, used in the boot
  /// strap stereo algorithm. Defaults to [-30,30]. (In principal it can and will
  /// produce values outside the range given, however don't expect the right
  /// result if the range given isn't sufficinet.)
  /// Irrelevant of stereo override is used.
   void SetRange(int32 minDisp,int32 maxDisp);


  /// Sets the radius of the mean shift when applying to segments to form layers.
  /// 0.6 is the default.
   void SetPlaneRadius(real32 rad);

  /// Sets the cost associated with a given warped image compared with the image
  /// it is warping to. This is the cost per occlusion and cost per discontinuity.
  /// Note that because of the colour scaling the original parameters given in the
  /// paper must be divided by 255 to make sense here. Default to 0.078 and 0.01
  /// respectivly.
   void SetWarpCost(real32 occ,real32 disc);


  /// Sets the minimum pixel size of the cluster to be concidered valid, used in
  /// the boot strap stereo method. Defaults to 20.
  /// Irrelevant if bootstrap override is used.
   void SetMinCluster(nat32 minC);

  /// Sets the number of final iterations without improvment for the layer r
  /// e-classification to give up. Defaults to 12.
   void SetBailOut(nat32 bo);


  /// Sets the 3 basic parameters of a mean shift segmentation, the spatial radius,
  /// the range radius and the minimum segment size. Defaults to 7, 4.5 and 20
  /// respectivly.
   void SetSegment(real32 spatial,real32 range,nat32 minSeg);

  /// Extra parameters for the mean shift segmentation, to do with the synergistic
  /// bit. The window radius, the mix between the values provided and the border
  /// cutoff for a merge to happen. Default to 2, 0.3 and 0.9 respectivly.
   void SetSegmentExtra(nat32 rad,real32 mix,real32 edge);


  /// Runs the algorithm, takes a progress object so it can be monitored, which
  /// is a good thing as it takes some time.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Extracts the disparity map.
   void GetDisparity(svt::Field<real32> & disp);


  /// Returns how many segments were created.
   nat32 GetSegCount();

  /// Extracts the segmentation.
   void GetSegs(svt::Field<nat32> & out);

  /// Returns how many layers were created.
   nat32 GetLayerCount();

  /// Extracts the layer segmentation.
   void GetLayers(svt::Field<nat32> & out);


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::Bleyer04";}


 private:
  // Internal constants...
   static const nat32 maxIter = 16;

  // Inputs...
   svt::Field<bs::ColourRGB> left;
   svt::Field<bs::ColourRGB> right;

   svt::Field<bit> leftMask;
   svt::Field<bit> rightMask;

   svt::Field<real32> dispOverride;
   svt::Field<bit> maskOverride;

   int32 minDisp;
   int32 maxDisp;

   real32 planeRadius;
   real32 occCost;
   real32 discCost;

   nat32 minCluster;
   nat32 bailOut;

   real32 spatialRadius;
   real32 rangeRadius;
   nat32 minSeg;
   nat32 windowSize;
   real32 edgeMix;
   real32 edgeLimit;

  // Outputs...
   svt::Var * out;

   nat32 segCount;
   nat32 layerCount;
};

//------------------------------------------------------------------------------
 };
};
#endif
