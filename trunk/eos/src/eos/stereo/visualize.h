#ifndef EOS_STEREO_VISUALIZE_H
#define EOS_STEREO_VISUALIZE_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file visualize.h
/// Provides the ability to render disparities out to images.

#include "eos/types.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"
#include "eos/bs/colours.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// Renders out a stereo disparity map, middlebury style.
/// Renders black to white, multipliying the disparities by mult to get the 
/// [0..1] range required.
/// If mult is 0.0 it calculates a suitable mult to utilise the entire range.
EOS_FUNC void RenderDisp(const svt::Field<real32> & disp,svt::Field<bs::ColourL> & out,real32 mult = 0.0);

/// This is given a 3D field representing similarity or dis-similarity map and
/// a row to render, it then returns a Var containing a b&w image of the row 
/// normalised to fill the greyscale range, showing the values across the 
/// disparity. It also fills in the meta fields min and max with the actual
/// minimum and maximum values found pre-normalisation.
EOS_FUNC svt::Var * RenderDsiRow(const svt::Field<real32> & dsi,nat32 row);


//------------------------------------------------------------------------------
 };
};
#endif
