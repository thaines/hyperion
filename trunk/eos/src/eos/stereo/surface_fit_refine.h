#ifndef EOS_STEREO_SURFACE_FIT_REFINE_H
#define EOS_STEREO_SURFACE_FIT_REFINE_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file surface_fit_refine.h
/// Provides stereo algorithms based on segmentation followed by surface fitting
/// to a DSI, initially a plane is fitted, but then a more complex model is
/// iterativly refined using more information.

#include "eos/types.h"
#include "eos/svt/field.h"
#include "eos/svt/var.h"
#include "eos/bs/colours.h"
#include "eos/bs/geo3d.h"
#include "eos/cam/files.h"
#include "eos/stereo/dsi_ms.h"
#include "eos/stereo/refine_orient.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// This segments an image via mean shift, except it additionally uses
/// information from a DSI to improve the segmentation, i.e. it attempts to
/// preserve discontinuities in the DSI.
/// Note: Current version has been bastardised to get algoirthm running.
/// Please apply un-bastardisation-lotion.
class EOS_CLASS SegmentDSI
{
 public:
  /// &nbsp;
   SegmentDSI();

  /// &nbsp;
   ~SegmentDSI();


  /// Sets parameters for the segmentation.
  /// \param colourSize Size of the colour window, defaults to 4.5
  /// \param spatialSize Size of the spatial window, defaults to 7.0
  /// \param dispSize Size of the disparity window, defaults to 7.0
  /// \param minMerge Minimum segment size, defaults to 20. Smaller segments are
  ///               un-conditionaly merged with there closest cluster.
  /// \param minKill Minimum segment size, defaults to 0. Smaller segments are
  ///               un-conditionaly killed after the merging step. Must be larger
  ///               than minMerge to matter.
   void Set(real32 colourSize,real32 spatialSize,real32 dispSize,nat32 minMerge,nat32 minKill);

  /// Sets the image to be segmented.
   void Set(const svt::Field<bs::ColourLuv> & image);

  /// Sets the DSI object for the image.
   void Set(const DSI & dsi);


  /// &nbsp;
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Returns the number of segments.
   nat32 Segments() const;

  /// Of the image.
   nat32 Width() const;

  /// Of the image.
   nat32 Height() const;


  /// Outputs the segmentation map.
   void GetSeg(svt::Field<nat32> & out) const;


  /// &nbsp;
   static cstrconst TypeString() {return "eos::stereo::SegmentDSI";}


 private:
  // In...
   real32 colourSize;
   real32 spatialSize;
   real32 dispSize;
   nat32 minMerge;
   nat32 minKill;

   svt::Field<bs::ColourLuv> image;
   const DSI * dsi;

  // Out...
   nat32 segments;
   ds::Array2D<nat32> seg;
};

//------------------------------------------------------------------------------
/// Defines an interface and basic functionality for any class that represents a
/// segmented image with an arbitary surface assigned to each segment.
/// Allows a child class to define methods for sampling position, disparity and
/// orientation for the surfaces. Provides (overridable) methods for then
/// getting out maps of these parameters.
/// Does not provide storage for the segmentation map, that is upto the child.
class EOS_CLASS SurSeg : public Deletable
{
 public:
  /// &nbsp;
   SurSeg() {}

  /// &nbsp;
   ~SurSeg() {}


  /// Returns the number of segments.
   virtual nat32 Segments() const = 0;

  /// Returns the width. All maps must comply.
   virtual nat32 Width() const = 0;

  /// Returns the height. All maps must comply.
   virtual nat32 Height() const = 0;


  /// Returns the segment for a given pixel.
   virtual nat32 Seg(nat32 x,nat32 y) const = 0;

  /// Returns the disparity for a given pixel.
   virtual real32 Disp(nat32 x,nat32 y) const = 0;

  /// Outputs the position in 3D space for a given pixel.
   virtual void Pos(nat32 x,nat32 y,bs::Vertex & out) const = 0;

  /// Outputs the surface orientation for a given pixel, in world space.
   virtual void Norm(nat32 x,nat32 y,bs::Normal & out) const = 0;


  /// Fills in a segmentation map.
   virtual void GetSeg(svt::Field<nat32> & out) const;

  /// Fills in a disparity map.
   virtual void GetDisp(svt::Field<real32> & out) const;

  /// Fills in a position map.
   virtual void GetPos(svt::Field<bs::Vertex> & out) const;

  /// Fills in a surface orientation map.
   virtual void GetNorm(svt::Field<bs::Normal> & out) const;


  /// Helper method - this returns the index of the 'background segment', uses
  /// a simple voting scheme where each pixel on the edge of the image casts
  /// one vote.
   nat32 GetBackground() const;


  /// &nbsp;
   virtual cstrconst TypeString() const = 0;
};

//------------------------------------------------------------------------------
/// Basic implimentation of a SurSeg, you provide a disparity map, a
/// segmentation and a camera pair and it prepares it all.
class EOS_CLASS SurSegBasic : public SurSeg
{
 public:
  /// &nbsp;
   SurSegBasic():segments(0),var(null<svt::Var*>()) {}

  /// &nbsp;
   ~SurSegBasic() {delete var;}


  /// Fills in the object, do not use until called.
   void Set(const svt::Field<real32> & disp,const svt::Field<nat32> & seg,const cam::CameraPair & pair);


  /// Returns the number of segments.
   nat32 Segments() const {return segments;}

  /// Returns the width. All maps must comply.
   nat32 Width() const {return var->Size(0);}

  /// Returns the height. All maps must comply.
   nat32 Height() const {return var->Size(1);}


  /// Returns the segment for a given pixel.
   nat32 Seg(nat32 x,nat32 y) const {return seg.Get(x,y);}

  /// Returns the disparity for a given pixel.
   real32 Disp(nat32 x,nat32 y) const {return disp.Get(x,y);}

  /// Outputs the position in 3D space for a given pixel.
   void Pos(nat32 x,nat32 y,bs::Vertex & out) const {out = pos.Get(x,y);}

  /// Outputs the surface orientation for a given pixel, in world space.
   void Norm(nat32 x,nat32 y,bs::Normal & out) const {out = norm.Get(x,y);}


  /// &nbsp;
   cstrconst TypeString() const {return "eos::stereo::SurSegBasic";}


 private:
  nat32 segments;
  svt::Var * var;
  svt::Field<nat32> seg;
  svt::Field<real32> disp;
  svt::Field<bs::Vertex> pos;
  svt::Field<bs::Normal> norm;
};

//------------------------------------------------------------------------------
/// This creates a DSI, segments the image and then fits planes to the DSI for
/// each segment. This exists as an initialisation step for more sophisticated
/// algorithms, i.e. once that use advanced surfaces.
/// The DSI is based on a sparse hierachical dynamic programing approach.
/// The segmentation uses the disparity image generated from an intelligently
/// weighted average from the DSI for each pixel. (The average is designed to
/// minimise the effect of outliers.) It takes this, the colour channels and
/// position and applies mean shift.
/// The plane fitting is parameterless, and based on weighted least squares
/// followed by LM. The LM follows through the range model of the DSI, where
/// disparitys represent rnages rather single points.
/// Plane fiting is done in 3D space rather than dispairity space, so conversion
/// has to happen.
class EOS_CLASS PlaneStereo : public SurSeg
{
 public:
  /// &nbsp;
   PlaneStereo();

  /// &nbsp;
   ~PlaneStereo();


  /// Sets parameters for the DSI generation.
  /// \param occCost Cost of an occlusion. Defaults to 1.0.
  /// \param vertCost Cost of vertical consistancy, defaults to 1.0
  /// \param vertMult Multiplier of vertical cost, to keep it scaled well. Defaults to 0.2.
  /// \param errLim The error per pixel allowed before a disparity is discarded. Defaults to 0.1.
   void Set(real32 occCost,real32 vertCost,real32 vertMult,real32 errLim);

  /// Sets the parameters for the disparity selection used for the segmentation.
  /// Default to 3 for radius (7x7 window) and 1.0 for the mult.
  /// Also sets the maximum number of disparities to send through per pixel to
  /// the plane fitter, defaults to 3.
   void Set(nat32 radius,real32 mult,nat32 cap);

  /// Sets parameters for the segmentation.
  /// \param colourSize Size of the colour window, defaults to 4.5
  /// \param spatialSize Size of the spatial window, defaults to 7.0
  /// \param dispSize Size of the disparity window, defaults to 7.0
  /// \param minMerge Minimum segment size, defaults to 20. Smaller segments are
  ///               un-conditionaly merged with there closest cluster.
  /// \param minKill Minimum segment size, defaults to 0. Smaller segments are
  ///               un-conditionaly killed after the merging step. Must be larger
  ///               than minMerge to matter.
   void Set(real32 colourSize,real32 spatialSize,real32 dispSize,nat32 minMerge,nat32 minKill);

  /// Sets the left and right images. Must be called before run.
   void Set(const svt::Field<bs::ColourLuv> & left,const svt::Field<bs::ColourLuv> & right);

  /// Sets the camera pair for the two images. Must be size-matched with the mages.
   void Set(const cam::CameraPair & pair);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Returns the number of segments.
   nat32 Segments() const;

  /// For the left image.
   nat32 Width() const;

  /// For the left image.
   nat32 Height() const;


  /// Returns the plane for a given segment. Will be normalised.
   const bs::Plane & Surface(nat32 i) const;


  /// Returns the segment for a given pixel.
   nat32 Seg(nat32 x,nat32 y) const;

  /// &nbsp;
   real32 Disp(nat32 x,nat32 y) const;

  /// &nbsp;
   void Pos(nat32 x,nat32 y,bs::Vertex & out) const;

  /// &nbsp;
   void Norm(nat32 x,nat32 y,bs::Normal & out) const;


  /// Outputs a segmentation map.
   void GetSeg(svt::Field<nat32> & out) const;

  /// Outputs a disparity map.
   void GetDisp(svt::Field<real32> & out) const;

  /// Outputs a 3D position map.
   void GetPos(svt::Field<bs::Vertex> & out) const;

  /// Outputs a surface orientation map.
   void GetNorm(svt::Field<bs::Normal> & out) const;


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::PlaneStereo";}


 protected:
  // In...
   real32 occCost;
   real32 vertCost;
   real32 vertMult;
   real32 errLim;

   nat32 radius;
   real32 mult;
   nat32 cap;

   real32 colourSize;
   real32 spatialSize;
   real32 dispSize;
   nat32 minMerge;
   nat32 minKill;

   svt::Field<bs::ColourLuv> left;
   svt::Field<bs::ColourLuv> right;

   cam::CameraPair pair;


  // Out...
   SparsePos sparsePos; // So we can latter recalculate stuff given further info.

   svt::Var * out; // Contains the segmentation, disparity map and position map.
   svt::Field<nat32> seg;
   svt::Field<real32> disp;
   svt::Field<bs::Vertex> pos;

   ds::Array<bs::Plane> plane; // One for each segment, normalised.
};

//------------------------------------------------------------------------------
/// This creates a DSI, segments the image and then fits local planes within
/// each segment to a window. Once done the details are made avaliable, however,
/// the class has a further use - you can then run again providing a needle map,
/// it will then re-fit, constraining the planes to the needle map.
///
/// The DSI is based on a sparse hierachical dynamic programing approach.
/// The segmentation uses the disparity image generated from an intelligently
/// weighted average from the DSI for each pixel. (The average is designed to
/// minimise the effect of outliers.) It takes this, the colour channels and
/// position and applies mean shift.
///
/// The plane fitting is parameterless, and based on weighted least squares.
/// Plane fiting is done in 3D space rather than dispairity space, so conversion
/// has to happen.
class EOS_CLASS LocalPlaneStereo : public SurSeg
{
 public:
  /// &nbsp;
   LocalPlaneStereo();

  /// &nbsp;
   ~LocalPlaneStereo();


  /// Sets parameters for the DSI generation.
  /// \param occCost - Cost of an occlusion. Defaults to 1.0.
  /// \param vertCost - Cost of vertical consistancy, defaults to 0.2
  /// \param vertMult Multiplier of vertical cost, to keep it scaled well. Defaults to 0.2.
  /// \param errLim - The error per pixel allowed before a disparity is discarded. Defaults to 0.1.
   void Set(real32 occCost,real32 vertCost,real32 vertMult,real32 errLim);

  /// Sets the parameters for the disparity selection used for the segmentation.
  /// Default to 3 for radius (7x7 window) and 1.0 for the mult.
  /// Also sets the maximum number of disparities to send through per pixel to
  /// the plane fitter, defaults to 3.
   void Set(nat32 radius,real32 mult,nat32 cap);

  /// Sets parameters for the segmentation.
  /// \param colourSize Size of the colour window, defaults to 4.5
  /// \param spatialSize Size of the spatial window, defaults to 7.0
  /// \param dispSize Size of the disparity window, defaults to 7.0
  /// \param minMerge Minimum segment size, defaults to 20. Smaller segments are
  ///               un-conditionaly merged with there closest cluster.
  /// \param minKill Minimum segment size, defaults to 0. Smaller segments are
  ///               un-conditionaly killed after the merging step. Must be larger
  ///               than minMerge to matter.
   void Set(real32 colourSize,real32 spatialSize,real32 dispSize,nat32 minMerge,nat32 minKill);

  /// Sets the radius used for localised plane fitting.
   void Set(nat32 radius);

  /// Sets the left and right images. Must be called before run.
   void Set(const svt::Field<bs::ColourLuv> & left,const svt::Field<bs::ColourLuv> & right);

  /// Sets the camera pair for the two images. Must be size-matched with the mages.
   void Set(const cam::CameraPair & pair);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());

  /// Does a re-run of the final steps on being given a needle map such that the
  /// fitted planes have identical normals - this is done by fitting planes with
  /// restricted orientations.
   void ReRun(const svt::Field<bs::Normal> & needle,
              time::Progress * prog = null<time::Progress*>());


  /// Returns the number of segments.
   nat32 Segments() const;

  /// For the left image.
   nat32 Width() const;

  /// For the left image.
   nat32 Height() const;


  /// Returns the segment for a given pixel.
   nat32 Seg(nat32 x,nat32 y) const;

  /// &nbsp;
   real32 Disp(nat32 x,nat32 y) const;

  /// &nbsp;
   void Pos(nat32 x,nat32 y,bs::Vertex & out) const;

  /// &nbsp;
    void Norm(nat32 x,nat32 y,bs::Normal & out) const;


  /// Outputs a segmentation map.
   void GetSeg(svt::Field<nat32> & out) const;

  /// Outputs a disparity map.
   void GetDisp(svt::Field<real32> & out) const;

  /// Outputs a 3D position map.
   void GetPos(svt::Field<bs::Vertex> & out) const;

  /// Outputs a surface orientation map.
   void GetNorm(svt::Field<bs::Normal> & out) const;


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::LocalPlaneStereo";}


 protected:
  // In...
   real32 occCost;
   real32 vertCost;
   real32 vertMult;
   real32 errLim;

   nat32 radius;
   real32 mult;
   nat32 cap;

   real32 colourSize;
   real32 spatialSize;
   real32 dispSize;
   nat32 minMerge;
   nat32 minKill;

   nat32 planeRadius;

   svt::Field<bs::ColourLuv> left;
   svt::Field<bs::ColourLuv> right;

   cam::CameraPair pair;


  // Out...
   SparsePos sparsePos; // So we can latter recalculate stuff given further info.

   svt::Var * out; // Contains the segmentation, disparity map, position map and needle maps.
   nat32 segCount;
   svt::Field<nat32> seg;
   svt::Field<real32> disp;
   svt::Field<bs::Vertex> pos;
   svt::Field<bs::Normal> norm;
};

//------------------------------------------------------------------------------
/// This creates a DSI, segments the image and then fits a point surface.
/// Once done the details are made avaliable, however,
/// the class has a further use - you can then run again providing a needle map,
/// it will then re-fit, constraining the surface with the needle map.
///
/// The DSI is based on a sparse hierachical dynamic programing approach.
/// The segmentation uses the disparity image generated from an intelligently
/// weighted average from the DSI for each pixel. (The average is designed to
/// minimise the effect of outliers.) It takes this, the colour channels and
/// position and applies mean shift.
class EOS_CLASS CloudStereo : public SurSeg
{
 public:
  /// &nbsp;
   CloudStereo();

  /// &nbsp;
   ~CloudStereo();


  /// Sets parameters for the DSI generation.
  /// \param occCost - Cost of an occlusion. Defaults to 1.0.
  /// \param vertCost - Cost of vertical consistancy, defaults to 0.2
  /// \param vertMult Multiplier of vertical cost, to keep it scaled well. Defaults to 0.2.
  /// \param errLim - The error per pixel allowed before a disparity is discarded. Defaults to 0.1.
   void Set(real32 occCost,real32 vertCost,real32 vertMult,real32 errLim);

  /// Sets the parameters for the disparity selection used for the segmentation.
  /// Default to 3 for radius (7x7 window) and 1.0 for the mult.
  /// Also sets the maximum number of disparities to send through per pixel to
  /// the plane fitter, defaults to 3.
   void Set(nat32 radius,real32 mult,nat32 cap);

  /// Sets parameters for the segmentation.
  /// \param colourSize Size of the colour window, defaults to 4.5
  /// \param spatialSize Size of the spatial window, defaults to 7.0
  /// \param dispSize Size of the disparity window, defaults to 7.0
  /// \param minMerge Minimum segment size, defaults to 20. Smaller segments are
  ///               un-conditionaly merged with there closest cluster.
  /// \param minKill Minimum segment size, defaults to 0. Smaller segments are
  ///               un-conditionaly killed after the merging step. Must be larger
  ///               than minMerge to matter.
   void Set(real32 colourSize,real32 spatialSize,real32 dispSize,nat32 minMerge,nat32 minKill);

  /// Sets the parameters used for surface fitting.
   void Set(nat32 radius,real32 damping,real32 stepCost,nat32 iters);

  /// Sets the left and right images. Must be called before run.
   void Set(const svt::Field<bs::ColourLuv> & left,const svt::Field<bs::ColourLuv> & right);

  /// Sets the camera pair for the two images. Must be size-matched with the mages.
   void Set(const cam::CameraPair & pair);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());

  /// Does a re-run of the final steps on being given a needle map such that the
  /// fitted planes have identical normals - this is done by fitting planes with
  /// restricted orientations.
   void ReRun(const svt::Field<bs::Normal> & needle,
              time::Progress * prog = null<time::Progress*>());


  /// Returns the number of segments.
   nat32 Segments() const;

  /// For the left image.
   nat32 Width() const;

  /// For the left image.
   nat32 Height() const;


  /// Returns the segment for a given pixel.
   nat32 Seg(nat32 x,nat32 y) const;

  /// &nbsp;
   real32 Disp(nat32 x,nat32 y) const;

  /// &nbsp;
   void Pos(nat32 x,nat32 y,bs::Vertex & out) const;

  /// &nbsp;
    void Norm(nat32 x,nat32 y,bs::Normal & out) const;


  /// Outputs a segmentation map.
   void GetSeg(svt::Field<nat32> & out) const;

  /// Outputs a disparity map.
   void GetDisp(svt::Field<real32> & out) const;

  /// Outputs a 3D position map.
   void GetPos(svt::Field<bs::Vertex> & out) const;

  /// Outputs a surface orientation map.
   void GetNorm(svt::Field<bs::Normal> & out) const;


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::CloudStereo";}


 protected:
  // In...
   real32 occCost;
   real32 vertCost;
   real32 vertMult;
   real32 errLim;

   nat32 radius;
   real32 mult;
   nat32 cap;

   real32 colourSize;
   real32 spatialSize;
   real32 dispSize;
   nat32 minMerge;
   nat32 minKill;

   nat32 surRadius;
   real32 surDamping;
   real32 surStepCost;
   nat32 surIters;


   svt::Field<bs::ColourLuv> left;
   svt::Field<bs::ColourLuv> right;

   cam::CameraPair pair;


  // Out...
   // So we can latter re-calculate stuff given further info.
    SparseDSI sdsi;
    SparsePos sparsePos;
    RefineOrient refOrient;

   svt::Var * out; // Contains the segmentation, disparity map, position map and needle maps.
   nat32 segCount;
   svt::Field<nat32> seg;
   svt::Field<real32> disp;
   svt::Field<bs::Vertex> pos;
   svt::Field<bs::Normal> norm;
};

//------------------------------------------------------------------------------
 };
};
#endif
