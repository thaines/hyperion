#ifndef EOS_CAM_RECTIFICATION_H
#define EOS_CAM_RECTIFICATION_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file rectification.h
/// Provides two algorithms for rectification, the plane techneque and the
/// polar techneque.

#include "eos/types.h"
#include "eos/svt/field.h"
#include "eos/cam/cameras.h"
#include "eos/time/progress.h"

namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------
/// This is an implimentation of the planar techneque of image rectification.
/// Its not particularly intelligent, and should only be applied to allmost classical
/// pairs to refine them to perfectly classical pairs.
/// \param inA The fields that make up the first image, it will interpolate
///            these fields to create the transformed left image. It only
///            handles fields of bs::ColourRGB, bs::ColourL and real32 type.
///            It takes any field named mask of type bit to be just that.
/// \param inB Same as inA, but for the other image of course.
/// \param radA Radial distortion for the A input.
/// \param radB Radial distortion for the B input.
/// \param fun The fundamental matrix, calculated to go from A to B.
/// \param outA The output version of inA, will be resized and contain the
///             fields of inA that have been understood and distorted by the
///             rectification proccess. Additionally it will contain a mask,
///             indicating which areas are valid. (Invalid areas will be set
///             to basic values so you can use it direct without consideration
///             of the mask if visualisation is all that matters.)
///             A field called 'original' is also created, of type bs::Pnt,
///             which contains the original coordinates of each rectified pixel.
///             Any data within outA on passin will be annihilated, as it is
///             resized and reset.
/// \param outB Same as outA, but for the B input.
/// \param outTA Set to a homography transforming the rectified A image to the original
///              coordinates.
/// \param outTB Set to a homography transforming the rectified B image to the original
///              coordinates.
/// \param samples How many samples to take in each dimension, similer to the
///                anti-aliasing setting found in raytracers.
/// \param doOriginal Can be set false to disable the output of the original field.
/// \param doMask Can be set false to disable the output of a mask.
/// \param prog Optional progress reporter.
/// \returns true on success, false on failure. failure can happen if the images
///          are too big and insufficient memory is avaliable.
EOS_FUNC bit PlaneRectify(svt::Var * inA,svt::Var * inB,
                          const Radial & radA,const Radial & radB,const Fundamental & fun,
                          svt::Var * outA,svt::Var * outB,
                          math::Mat<3,3,real64> * outTA = null<math::Mat<3,3,real64>*>(),
                          math::Mat<3,3,real64> * outTB = null<math::Mat<3,3,real64>*>(),
                          nat32 samples = 1,bit doOriginal = true,bit doMask = true,
                          time::Progress * prog = null<time::Progress*>());

//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
 };
};
#endif
