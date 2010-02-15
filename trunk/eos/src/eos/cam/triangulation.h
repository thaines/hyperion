#ifndef EOS_CAM_TRIANGULATION_H
#define EOS_CAM_TRIANGULATION_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file triangulation.h
/// Provides triangulation, i.e. given the relevent details about a camera and 
/// two points this module will output 3D locations/depths etc. Provides a variety
/// of variants on this theme, for special cases.

#include "eos/types.h"
#include "eos/cam/cameras.h"
#include "eos/svt/field.h"
#include "eos/bs/geo3d.h"

namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------
/// This calculates depth from rectified images given the relevent details and 
/// enough assumptions. (See P.249 in eyeball book, 2nd edition.)
/// For this to be correct the following assumptions must hold:
/// - The two camera views must have the same intrinsic calibration.
/// - The motion between them must be a translation in the X axis. If they have been rectified to such a postion then depth will be from the ensuring virtual cameras, only rectification that makes such virtual cameras will work. (i.e. the polar method will not work with this techneque.)
/// - The intrinsic calibration must be known.
/// - The distance along the x axis must be known to obtain scale.
/// - The depth output is along the primary cameras principal axis. Expansion to a complete position is = depth * K^-1 * [x,y,1]^T. (See RectifiedPos(...).)
///
/// \param intr The intrinsic matrix shared by both cameras.
/// \param xt The translation between the two cameras in the x axis. As everything is to a scale you can set this to 1 if unknown to obtain everything to an arbitary scale.
/// \param disp The disparity map.
/// \param depth The output depth map.
/// \param mask An output mask, set to true where depth is known. This is because a disparity of zero represents a point at infinity, with all the associated problems.
EOS_FUNC void RectifiedDepth(const Intrinsic & intr,
                            real32 xt,const svt::Field<real32> & disp,
                            svt::Field<real32> & depth,svt::Field<bit> & mask);

//------------------------------------------------------------------------------
/// This extends RectifiedDepth, identical except it outputs 3D positions rather
/// than depth. This includes optional adjustment for Radial distortion.
/// Presumes the camera to be at (0,0,0).
EOS_FUNC void RectifiedPos(const Intrinsic & intr,const Radial * rad,
                           real32 xt,const svt::Field<real32> & disp,
                           svt::Field<bs::Vert> & pos,svt::Field<bit> & mask);

//------------------------------------------------------------------------------
/// This extends RectifiedPos (Which itself extends RectifiedDepth.) to output 
/// surface orientation. Uses central difference, and unlike other Rectified*
/// functions provide a reasonable value for all points. Points at infinity are
/// given a surface orientation pointing directly at the camera; points at the 
/// edge get there values copied from neighbours.
EOS_FUNC void RectifiedNeedle(const Intrinsic & intr,const Radial * rad,
                              real32 xt,const svt::Field<real32> & disp,
                              svt::Field<bs::Normal> & needle);

//------------------------------------------------------------------------------
/// This takes the output from RectifiedNeedle and converts it to orthographic 
/// directions from projective directions. This consists of applying a rotation
/// to each orientation such that the intrinsic matrix derived ray direction
/// becomes (0,0,1), this produces the negative of the standard needle map 
/// format used in most SfS algorithms.
EOS_FUNC void OrthoNeedle(const Intrinsic & intr,const Radial * rad,
                          svt::Field<bs::Normal> & needle);

//------------------------------------------------------------------------------
/// Given two image points and a fundamental matrix this updates the two points
/// to match the fundamental matrix constraint.
/// The points are changed by the least amount possible.
/// Uses the method given on P.318 of the 2nd edition of the eyeball book.
/// Returns true on success, false on failure. On failure pa and pb are not editted.
EOS_FUNC bit CorrectMatch(math::Vect<2,real64> & pa,math::Vect<2,real64> & pb,
                          const Fundamental & fun);

//------------------------------------------------------------------------------
/// This is given two image points and two projection matrices, it then 
/// triangulates the two points to obtain a 3D point. Its uses the SVD based 
/// linear method, so it is not projective invariant on its own. 
/// To obtain projective invariance you should first correct the image points to
/// perfectly lie on an epipolar line using the CorrectMatch function.
/// The output is homogenous, as points at infinity are a regular occurance.
/// Returns true on success, very unlikelly to fail.
EOS_FUNC bit Triangulate(const math::Vect<2,real64> & pa,const math::Vect<2,real64> & pb,
                         const Camera & ca,const Camera & cb,
                         math::Vect<4,real64> & out);

/// This is given two image points and two projection matrices, it then 
/// triangulates the two points to obtain a 3D point. Its uses the SVD based 
/// linear method, so it is not projective invariant on its own. 
/// To obtain projective invariance you should first correct the image points to
/// perfectly lie on an epipolar line using the CorrectMatch function.
/// The output is homogenous, as points at infinity are a regular occurance.
/// Returns true on success, very unlikelly to fail.
EOS_FUNC bit Triangulate(const math::Vect<3,real64> & pa,const math::Vect<3,real64> & pb,
                         const Camera & ca,const Camera & cb,
                         math::Vect<4,real64> & out);

//------------------------------------------------------------------------------
/// Identical to triangulate, except it outputs depth as in distance along the 
/// cameras principal axis rather than an actual coordinate.
/// Will output infinity when the point is at infinity.
/// Returns true on success, very unlikelly to fail.
EOS_FUNC bit Depth(const math::Vect<2,real64> & pa,const math::Vect<2,real64> & pb,
                   const Camera & ca,const Camera & cb,
                   real64 & out);

//------------------------------------------------------------------------------
/// Given a depth, pixel and camera this calculates an actual 3D coordinate.
/// Returns true on success, unlikelly to fail.
EOS_FUNC bit DepthToPos(const math::Vect<2,real64> & p,const Camera & c,
                        real64 depth,math::Vect<4,real64> & out);

//------------------------------------------------------------------------------
 };
};
#endif
