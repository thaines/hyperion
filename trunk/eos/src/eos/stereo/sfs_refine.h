#ifndef EOS_STEREO_SFS_REFINE_H
#define EOS_STEREO_SFS_REFINE_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file sfs_refine.h
/// Does sfs to the surfaces in a surface/segment approach. Also partakes in the
/// related activites for creating entire stereo algoithms out of this.

#include "eos/types.h"
#include "eos/bs/geo3d.h"
#include "eos/stereo/surface_fit_refine.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// This assigns albedo to surfaces given lighting information and surface
/// information. Uses an advanced approach that should be particularly robust to
/// outliers based on density estimation and finding the highest mode within the
/// resulting function.
/// Ignores segment 0, as thats where all the bad pixels go.
/// Note that it does not assume an infinite light source, it assumes that each
/// pixel is at the assigned surface and acts accordingly.
class EOS_CLASS AlbedoEstimate
{
 public:
  /// &nbsp;
   AlbedoEstimate();

  /// &nbsp;
   ~AlbedoEstimate();


  /// Sets the light source position in 3D space, homogenous, suports infinite
  /// points.
   void SetLight(bs::Vertex & light);

  /// Sets the image from which to extract irradiance.
   void SetImage(const svt::Field<bs::ColourLuv> & image);

  /// Can be optionally called to set a mask, areas that are true are ignored
  /// for the calculation. Will ushally represent the output of a specularity
  /// detection process.
   void SetMask(const svt::Field<bit> & mask);

  /// Sets the surface representation.
   void SetSurSeg(const SurSeg & surSeg);

  /// Enabled debug mode - if on it saves out the density estimate for each
  /// segment as a graph, plus logs the bounds of the graph.
   void EnableDebug() {debug = true;}


  /// Does the calculation.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Returns the position of the light used to calculate the albedo.
   const bs::Vertex & Light() const {return light;}

  /// Returns the albedo for a given segment, so as to satisfy the sfs
  /// cone constraint.
   real32 Albedo(nat32 seg) const;


  /// &nbsp;
   static cstrconst TypeString() {return "eos::stereo::AlbedoEstimate";}


 private:
  // Input...
   bs::Vertex light; // 3D position of the light source. Can be at infinity.
   svt::Field<bs::ColourLuv> image;
   svt::Field<bit> mask;
   const SurSeg * surSeg; // The source of surface information.

   bit debug; // True if we want extra info.

  // Output...
   ds::Array<real32> albedo; // For each segment.
};

//------------------------------------------------------------------------------
/// Version 2 of the AlbedoEstimate class. This version calculates per-pixel
/// values using a BP integration approach.
/// Uses the same estimation scheme as AlbedoEstimate with the same constant
/// albedo idea. It however enforces same albedo between pixels rather than over
/// entire segments, so if the segmentation is wrong albedo can very within
/// segments.
/// It additionally passes information between segments by assuming they have
/// the same orientation at the boundary and therefore the albedo ratio is known.
/// Ignores segment 0, as thats where all the bad pixels go.
/// Note that it does not assume an infinite light source, it assumes that each
/// pixel is at the assigned surface and acts accordingly.
class EOS_CLASS AlbedoEstimate2
{
 public:
  /// &nbsp;
   AlbedoEstimate2();

  /// &nbsp;
   ~AlbedoEstimate2();


  /// Sets the light source position in 3D space, homogenous, suports infinite
  /// points.
   void SetLight(bs::Vertex & light);

  /// Sets the image from which to extract irradiance.
  /// Differences in the uv component are used to
   void SetImage(const svt::Field<bs::ColourLuv> & image);

  /// Can be optionally called to set a mask, areas that are true are ignored
  /// for the calculation. Will ushally represent the output of a specularity
  /// detection process.
   void SetMask(const svt::Field<bit> & mask);

  /// Sets the surface representation, also uses the segments defined within.
   void SetSurSeg(const SurSeg & surSeg);

  /// Sets the parameters, specifically:
  /// \param surSd The standard deviation applied to albedo as calculated using
  ///              the given surface and light source. Defaults to 2.0.
  /// \param eqSd The standard deviation applied with the same albedo assumption,
  ///             different orientation assumption. [dosa] Defaults to 0.1.
  /// \param diffSd The standard deviation applied with the different albedo,
  ///               same orientation assumption. [soda] Defaults to 0.25.
  /// \param diffCutoff Whilst different segments allways have a daso assumption
  ///                   applied between them a cutoff can also be set, so within
  ///                   segments pixels with a euclidean difference of greater
  ///                   than this cutoff for the uv channels of Luv also use a
  ///                   daso assumption. Defaults to 5.0.
  /// \param iters The number of iterations per hierachy level for the
  ///              IntegrateBP object to use. Defaults to 100.
   void Set(real32 surSd,real32 eqSd,real32 diffSd,real32 diffCutoff,nat32 iters = 100);


  /// Does the calculation.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Returns the position of the light used to calculate the albedo.
   const bs::Vertex & Light() const {return light;}

  /// Returns the albedo for a given pixel.
   real32 Albedo(nat32 x,nat32 y) const;

  /// Outputs an albedo map.
   void GetAlbedo(svt::Field<real32> & out) const;


  /// &nbsp;
   static cstrconst TypeString() {return "eos::stereo::AlbedoEstimate2";}


 private:
  static const real32 max_ratio = 2.0; // Max ratio for a soda relationship.

  // Input...
   bs::Vertex light; // 3D position of the light source. Can be at infinity.
   svt::Field<bs::ColourLuv> image;
   svt::Field<bit> mask;
   const SurSeg * surSeg; // The source of surface information.

   real32 surSd;
   real32 eqSd;
   real32 diffSd;
   real32 diffCutoff;
   nat32 iters;

  // Output...
   ds::Array2D<real32> albedo; // For each pixel.
};

//------------------------------------------------------------------------------
/// A third version of the albedo estimate module - a step backwards to
/// something far simpler - taking the mean for each segment. Uses plane fitting
/// to the surface over a window to clean things up.
class EOS_CLASS AlbedoEstimate3
{
 public:
  /// &nbsp;
   AlbedoEstimate3();

  /// &nbsp;
   ~AlbedoEstimate3();


  /// Sets the light source position in 3D space, homogenous, suports infinite
  /// points.
   void SetLight(bs::Vertex & light);

  /// Sets the image from which to extract irradiance.
  /// Differences in the uv component are used to
   void SetImage(const svt::Field<bs::ColourLuv> & image);

  /// Can be optionally called to set a mask, areas that are true are ignored
  /// for the calculation. Will ushally represent the output of a specularity
  /// detection process.
   void SetMask(const svt::Field<bit> & mask);

  /// Sets the surface representation, also uses the segments defined within.
   void SetSurSeg(const SurSeg & surSeg);

  /// Sets the parameters, specifically:
  /// \param winRad Radius of the window, defaults to 5 which is a 11x11 window.
   void Set(nat32 winRad);


  /// Does the calculation.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Returns the position of the light used to calculate the albedo.
   const bs::Vertex & Light() const {return light;}

  /// Returns the albedo for a given pixel.
   real32 Albedo(nat32 x,nat32 y) const;

  /// Outputs an albedo map.
   void GetAlbedo(svt::Field<real32> & out) const;


  /// &nbsp;
   static cstrconst TypeString() {return "eos::stereo::AlbedoEstimate2";}


 private:
  // Input...
   bs::Vertex light; // 3D position of the light source. Can be at infinity.
   svt::Field<bs::ColourLuv> image;
   svt::Field<bit> mask;
   const SurSeg * surSeg; // The source of surface information.

   int32 winRad;

  // Output...
   ds::Array2D<real32> albedo; // For each pixel.
};

//------------------------------------------------------------------------------
/// This does sfs for each surface seperatly, with a Worthington & Hancock based
/// formulation. That is, it has a smoothing step and a restore-to-cone step.
/// It does however vary the light source direction vector between pixels by
/// assuming that the surface position is correct and calculating the light
/// source direction from its position accordingly. This is reasonable as long
/// as the light source is reasonably distant from the scene and the planes are
/// not wildly wrong. Corrected to cone plane normals are used for initialisation.
/// Currently using the DD1 smoothing techneque - could be more advanced.
class EOS_CLASS SurfaceSfS
{
 public:
  /// &nbsp;
   SurfaceSfS();

  /// &nbsp;
   ~SurfaceSfS();


  /// Sets the number of iterations, defaults to 200.
   void Set(nat32 iters);

  /// Sets the irradiance map to use.
   void Set(const svt::Field<bs::ColourLuv> & image);

  /// Sets the surface segmentation used for assigning surface orientation.
   void Set(const SurSeg & ss);

  /// This can be called optionally to set a particular surface as the
  /// 'background surface', this surface will then be ignored for sfs
  /// calculations and whenever any other surface borders this surface normals
  /// will be fixed based on boundary conditions.
   void SetBG(nat32 index);

  /// Sets the light source position.
   void Set(const bs::Vertex & light);

  /// Sets the albedo map.
   void Set(const svt::Field<real32> alb_map);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Returns the surface orientation assigned to a given pixel.
   const bs::Normal & Orient(nat32 x,nat32 y);


  /// &nbsp;
   static cstrconst TypeString() {return "eos::stereo::SurfaceSfS";}


 private:
  // In...
   nat32 iters;
   svt::Field<bs::ColourLuv> image;
   const SurSeg * ss;
   nat32 bgSeg;
   bs::Vertex light;
   svt::Field<real32> ae;

  // Out...
   ds::Array2D<bs::Normal> data;


  // Runtime...
   // Helper methods - the two steps this algorithm is implimented using...
    void CorrectStep(ds::Array2D<bs::Normal> & from,ds::Array2D<bs::Normal> & to,
                     ds::Array2D<bit> & mask,const ds::Array2D<bs::Normal> & toLight);
    void SmoothStep(ds::Array2D<bs::Normal> & from,ds::Array2D<bs::Normal> & to,ds::Array2D<bit> & mask);

};

//------------------------------------------------------------------------------
 };
};
#endif
