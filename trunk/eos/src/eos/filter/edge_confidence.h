#ifndef EOS_FILTER_EDGE_CONFIDENCE_H
#define EOS_FILTER_EDGE_CONFIDENCE_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file edge_confidence.h
/// This provides a method to calculate the rho and eta from the paper
/// 'Edge Detection with Embedded Confidence' by Peter Meer and Bogdan Georgescu.

#include "eos/types.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"
#include "eos/filter/kernel.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// This method is given a single field from which it determines two values, eta
/// and rho. rho is an indication of confidence in there being an edge, eta is 
/// an edge magnitude. Both are ranged [0..1]. kernel is the dx differentiation 
/// kernel to use, you can generate one using the EdgeConfidenceKernel method.
/// The provided epsilon is used to determine if two gradiant values are 
/// identical or not.
EOS_FUNC void EdgeConfidence(const svt::Field<real32> & in,const KernelVect & kernel,svt::Field<real32> & eta,svt::Field<real32> & rho);

/// This creates the standard kernel sugested in the paper to be handed into
/// the EdgeConfidence method, a differentiation kernel that calculates dx.
/// (And its transpose does dy.)
EOS_FUNC void EdgeConfidenceKernel(KernelVect & out,nat32 radius = 2);

//------------------------------------------------------------------------------
 };
};
#endif
