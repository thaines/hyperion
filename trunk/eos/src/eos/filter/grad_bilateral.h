#ifndef EOS_FILTER_GRAD_BILATERAL_H
#define EOS_FILTER_GRAD_BILATERAL_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

/// \file grad_bilateral.h
/// Contains functions for working out image gradiants using bilateral filters.

#include "eos/types.h"
#include "eos/bs/colours.h"
#include "eos/svt/field.h"
#include "eos/time/progress.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// This calculates the gradient of an image using the differential of a 
/// Gaussian of the spatial component of the image in two directions (With a 
/// normal Gaussian perpendicular) and a Gaussian in the domain of the image to
/// weight it.
EOS_FUNC void GradBilatGauss(const svt::Field<bs::ColourLuv> & in,svt::Field<real32> & dx,svt::Field<real32> & dy,
                             real32 spatialSd,real32 domainSd,real32 winSdMult = 2.0,
                             time::Progress * prog = null<time::Progress*>());

//------------------------------------------------------------------------------
 };
};
#endif
