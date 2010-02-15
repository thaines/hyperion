#ifndef EOS_SFS_LAMBERTIAN_SEGMENT_H
#define EOS_SFS_LAMBERTIAN_SEGMENT_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file lambertian_segment.h
/// Segments an irradiance and orientation map using lambertian shading models;
/// given a set of models assignes them to pixels with a cost of changing model
/// between pixels. Uses belief propagation.

#include "eos/types.h"
#include "eos/bs/geo3d.h"
#include "eos/bs/colours.h"
#include "eos/inf/model_seg.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
/// This segments an irradiance and orientation map pair by lambertian shading
/// model. For each pixel a cost of assigning each model is calculated, combined
/// with the cost of assigning different models to adjacent pixels this leads 
/// to a problem easilly solved with belief propagation.
class EOS_CLASS LamSeg
{
 public:
  /// &nbsp;
   LamSeg();
   
  /// &nbsp;
   ~LamSeg();
   
   
  /// Sets the irradiance and needle maps.
   void SetMaps(const svt::Field<real32> & irr,const svt::Field<bs::Normal> & needle);
   
  /// After setting maps you can set the standard deviation of the irradiance 
  /// as calculated from a model using the given needle map and irradiance map
  /// on a per pixel basis using this method.
  /// All default to 1.)
   void SetNoise(nat32 x,nat32 y,real32 sd);
   
  /// Sets global parameters.
  /// \param adjProb Sets the probability of having adjacent pixels with 
  ///                different models. Defaults to 0.1.
  /// \param minProb Sets the minimum allowed probability of assigning a model
  ///                to a pixel. Defaults to 0.2.
  /// \param iters Number of iterations to do for each level of the hierachy.
  ///              Defaults to 10.
   void Set(real32 adjProb,real32 minProb,nat32 iters);
   
  /// If called adjProb is overriden and ignored. Instead the eucledian distance
  /// between u and v of adjacent pixels modifies the otherwise constant value 
  /// on a per-neighbour basis. Should improve the segmentations boundary handling.
  /// Uses a sigmoid, with the usual 4 parameters.
   void Set(const svt::Field<bs::ColourLuv> & image,
            real32 minAdjProb,real32 maxAdjProb,
            real32 threshold,real32 halflife);

  /// Sets the number of models avaliable.
   void SetModels(nat32 count);

  /// Sets the model at a given index to the given vector.
   void SetModel(nat32 ind,const bs::Normal & mod);
   
   
  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Returns the number of models avaliable.
   nat32 Models() const;
   
  /// Returns the model at a given index.
   const bs::Normal & Model(nat32 ind) const;
   
  /// Returns the model assigned to each pixel.
   nat32 Pixel(nat32 x,nat32 y) const;
   
  /// Extracts a map of model indices.
   void GetModel(svt::Field<nat32> & out) const;

  /// Extracts a map of model vectors.
   void GetModel(svt::Field<bs::Normal> & out) const;
   
  /// Extracts a map of albedo and light source direction.
   void GetSplit(svt::Field<real32> & irr,svt::Field<bs::Normal> & dir);


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::sfs::LamSeg";}


 private:
  // Input...
   svt::Field<real32> irr;
   svt::Field<bs::Normal> needle;   
   ds::Array2D<real32> sd;
   
   real32 adjProb;
   real32 matchMin;
   nat32 iters;
   
   svt::Field<bs::ColourLuv> image;
   real32 minAdjProb;
   real32 maxAdjProb;
   real32 threshold;
   real32 halflife;
   
   ds::Array<bs::Normal> model;
   
  // Output...
   ds::Array2D<nat32> pixToMod;
};

//------------------------------------------------------------------------------
 };
};
#endif
