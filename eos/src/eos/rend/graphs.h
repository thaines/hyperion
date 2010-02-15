#ifndef EOS_REND_GRAPHS_H
#define EOS_REND_GRAPHS_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


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
