#ifndef EOS_FILTER_SHAPE_INDEX_H
#define EOS_FILTER_SHAPE_INDEX_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file shape_index.h
/// Provides an implimentation of the shape index method of Koenderink and Doorn.

#include "eos/types.h"
#include "eos/bs/geo3d.h"
#include "eos/bs/colours.h"
#include "eos/svt/field.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// This function is given a normal map and mask, from this it computes the shape
/// index of 'Surface shape and curvature scales' by Koenderink & Doorn 1992. It
/// outputs a field of real values with [-1,1] as the range of shape values. In
/// addition it outputs 2 to indicate areas where no shape index can be
/// calculated due to being a planar surface and -2 to indicate areas which can
/// not be calculated for another reason, i.e. being at the border.
/// The meanings of the values are as follows...
/// - [-1,-7/8) -> Spherical Cap (Elliptic)
/// - [-7/8,-5/8) -> Trough (Elliptic)
/// - [-5/8,-3/8) -> Rut (Parabolic)
/// - [-3/8,-1/8) -> Saddle Rut (Hyperbolic)
/// - [-1/8,1/8) -> Saddle (Hyperbolic)
/// - [1/8,3/8) -> Saddle Ridge (Hyperbolic)
/// - [3/8,5/8) -> Ridge (Parabolic)
/// - [5/8,7/8) -> Dome (Elliptic)
/// - [7/8,1] -> Spherical Cap (Elliptic)
EOS_FUNC void ShapeIndex(const svt::Field<bs::Normal> & needle,const svt::Field<bit> & mask,svt::Field<real32> & index);

//------------------------------------------------------------------------------
/// Given a shape index map, as calculated by the ShapeIndex function this
/// creates a colour image to express the information. It uses the colours given
/// in the original paper, i.e. (Range of shape index -> Type.)
/// Green indicates a shape index of -1.
/// Sky Blue indicates a shape index of -0.5.
/// White indicates a shape index of 0.
/// Yellow indicates a shape index of 0.5.
/// Red indicates a shape index of 1.0.
///
/// Blue is used to indicate planes, i.e. areas where the shape index is undefined.
/// Black is used to indicate areas where it could not be calculated, i.e. borders.
EOS_FUNC void ColourShapeIndex(const svt::Field<real32> & index,svt::Field<bs::ColourRGB> & colour);

//------------------------------------------------------------------------------
 };
};
#endif
