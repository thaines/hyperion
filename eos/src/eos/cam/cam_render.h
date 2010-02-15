#ifndef EOS_CAM_CAM_RENDER_H
#define EOS_CAM_CAM_RENDER_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file cam_render.h
/// Provides functions for rendering 3D triangles to an image with a .cam file,
/// its most critical capability is it handles radial distortion by a 
/// scanline/raytracing hybrid.

#include "eos/types.h"
#include "eos/cam/files.h"
#include "eos/ds/arrays.h"
#include "eos/svt/field.h"
#include "eos/bs/colours.h"
#include "eos/bs/geo3d.h"
#include "eos/time/progress.h"

namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------
/// Given a CameraFull this generates a ray for each pixel which an objects has
/// to intercept for that pixel to be rendered. This can then be fed into
/// RenderTri to make it go faster. Resizes the given array as neccesary.
void EOS_FUNC CamToRays(const CameraFull & cam,ds::Array2D<bs::Ray> & rays,time::Progress * prog = null<time::Progress*>());

/// This renders a triangle to an image using a CameraFull to set its rendering,
/// handles radial distortion with a hybrid scanline/raycasting approach.
/// Requires a depth map (Remember to initialise with negative infinities.) 
/// and suports the assignment of a different colour to each vertex which is 
/// then interpolated.
/// In addition to the camera you also provide a ray image generated for the 
/// camera with the partner function - this is an optimisation.
void EOS_FUNC RenderTri(const CameraFull & cam,const ds::Array2D<bs::Ray> & rays,
                        svt::Field<bs::ColourRGB> & image,
                        ds::Array2D<real32> & depth,
                        const bs::Vert & a,const bs::ColourRGB & ac,
                        const bs::Vert & b,const bs::ColourRGB & bc,
                        const bs::Vert & c,const bs::ColourRGB & cc);

//------------------------------------------------------------------------------
 };
};
#endif
