#ifndef EOS_REND_RERENDER_H
#define EOS_REND_RERENDER_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file rerender.h
/// Provides tools for re-rendering images based on changing global parameters,
/// such as light source direction for instance.

#include "eos/types.h"
#include "eos/bs/geo3d.h"
#include "eos/svt/field.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
/// Given an albedo and a normal for each point in an image this assumes basic
/// lambertion lighting, from an infinite light source that dosn't cast shadows,
/// and re-renders the image accordingly.
/// \param albedo Albedo for each pixel in the image.
/// \param needle Surface normal for each pixel in the image.
/// \param toLight Normal pointing towards light source at infinity, its length is the strength of he light. (Any longer than 1 and the image will probably suffer from saturation.)
/// \param out The output brightness map.
EOS_FUNC void LambertianNeedleRender(const svt::Field<real32> & albedo,
                                     const svt::Field<bs::Normal> & needle,
                                     const bs::Normal & toLight,
                                     svt::Field<real32> & out);

//------------------------------------------------------------------------------
 };
};
#endif
