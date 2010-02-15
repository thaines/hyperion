#ifndef EOS_FILTER_NORMALISE_H
#define EOS_FILTER_NORMALISE_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file normalise.h
/// Normalises an image.

#include "eos/types.h"
#include "eos/svt/field.h"
#include "eos/svt/var.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// Given a field of reals this normalises them to the 0..1 range, useful for 
/// visualisation.
EOS_FUNC void Normalise(svt::Field<real32> & f);

//------------------------------------------------------------------------------
 };
};
#endif
