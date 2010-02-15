#ifndef EOS_REND_GRAPHS_H
#define EOS_REND_GRAPHS_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file rend/graphs.h
/// Provides the ability to render graphs to images, for visualisation of 
/// anything you feel like.

#include "eos/types.h"

#include "eos/svt/var.h"
#include "eos/math/vectors.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
/// Converts a vector of samples into a graph. The graph will be without axis
/// and a hundred pixels high, and as wide as there are samples. It will be 
/// rendered in simple black & white. A quick and dirty method for the quick
/// visualisation of suitable data.
/// The samples are assumed to all be greater than or equal to 0, it is 
/// normalised so the highest sample gets the top scanline.
/// You are responsible for deleting the returned Var.
EOS_FUNC svt::Var * RenderSimpleGraph(svt::Core & core,math::Vector<real32> * samples);

//------------------------------------------------------------------------------
 };
};
#endif
