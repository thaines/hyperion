#ifndef EOS_SFS_ALBEDO_EST_H
#define EOS_SFS_ALBEDO_EST_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

/// \file albedo_est.h
/// Provides some simple albedo estimation stuff.

#include "eos/types.h"
#include "eos/bs/geo3d.h"
#include "eos/svt/field.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
/// Simple function for estimation the albedo - finds the mode of the
/// inputs. Mode is done to avoid outliers. Only uses pixels in the given range,
/// so as to avoid specularity effects and the extreme volatility of small values.
EOS_FUNC real32 AlbedoEstimate(const svt::Field<real32> & l,
                               const svt::Field<bs::Normal> & needle,
                               const bs::Normal & toLight,
                               real32 minL = 0.1,real32 maxL = 0.9);

//------------------------------------------------------------------------------
 };
};
#endif
