#ifndef EOS_STEREO_DISP_POST_H
#define EOS_STEREO_DISP_POST_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file disp_post.h
/// Provides post proccessing for disparity maps, to fill in missing values etc.

#include "eos/types.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// Sets all invalid disparities to 0, so they are effectivly at infinity.
EOS_FUNC void FillDispInf(svt::Field<real32> & disp,const svt::Field<bit> & valid);

/// Sets all invalid disparities to there left neighbour, the fill in techneque
/// proposed with the middlebury test.
EOS_FUNC void FillDispLeft(svt::Field<real32> & disp,const svt::Field<bit> & valid);

//------------------------------------------------------------------------------
 };
};
#endif
