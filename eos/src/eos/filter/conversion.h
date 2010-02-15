#ifndef EOS_FILTER_CONVERSION_H
#define EOS_FILTER_CONVERSION_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file filter/conversion.h
/// Methods to convert images from one colour space to another. Two interfaces
/// are provided for each, one which is a simple field to field mode and another
/// which you give an image and it does the conversion. The pass-an-image form
/// does not delete the field it is converting from, so its more of an expansion.

#include "eos/types.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"
#include "eos/bs/colours.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// This converts the given l field to a rgb field, given two fields of the same
/// dimensionality and size.
EOS_FUNC void LtoRGB(const svt::Field<bs::ColourL> & l,svt::Field<bs::ColourRGB> & rgb);

/// This converts the given rgb field to a l field, given two fields of the same
/// dimensionality and size.
EOS_FUNC void RGBtoL(const svt::Field<bs::ColourRGB> & rgb,svt::Field<bs::ColourL> & l);

/// This converts the given rgb field to a luv field, given two fields of the same
/// dimensionality and size.
EOS_FUNC void RGBtoLuv(const svt::Field<bs::ColourRGB> & rgb,svt::Field<bs::ColourLuv> & luv);

/// This converts the given luv field to a l field, given two fields of the same
/// dimensionality and size.
EOS_FUNC void LuvtoL(const svt::Field<bs::ColourLuv> & luv,svt::Field<bs::ColourL> & rgb);

/// This converts the given luv field to a rgb field, given two fields of the same
/// dimensionality and size.
EOS_FUNC void LuvtoRGB(const svt::Field<bs::ColourLuv> & luv,svt::Field<bs::ColourRGB> & rgb);

//------------------------------------------------------------------------------
/// This is given a var, by default it converts the l field in the var to create
/// a new rgb field. If the rgb field allready exists it is overwritten. You can
/// change the names of the fields if your feeling non-standard.
EOS_FUNC void LtoRGB(svt::Var * var,cstrconst l = "l",cstrconst rgb = "rgb");

/// This is given a var, by default it converts the rgb field in the var to create
/// a new l field. If the l field allready exists it is overwritten. You can
/// change the names of the fields if your feeling non-standard.
EOS_FUNC void RGBtoL(svt::Var * var,cstrconst rgb = "rgb",cstrconst l = "l");

/// This is given a var, by default it converts the rgb field in the var to create
/// a new luv field. If the luv field allready exists it is overwritten. You can
/// change the names of the fields if your feeling non-standard.
EOS_FUNC void RGBtoLuv(svt::Var * var,cstrconst rgb = "rgb",cstrconst luv = "luv");

/// This is given a var, by default it converts the luv field in the var to create
/// a new l field. If the l field allready exists it is overwritten. You can
/// change the names of the fields if your feeling non-standard.
EOS_FUNC void LuvtoL(svt::Var * var,cstrconst luv = "luv",cstrconst l = "l");

/// This is given a var, by default it converts the luv field in the var to create
/// a new rgb field. If the rgb field allready exists it is overwritten. You can
/// change the names of the fields if your feeling non-standard.
EOS_FUNC void LuvtoRGB(svt::Var * var,cstrconst luv = "luv",cstrconst rgb = "rgb");

//------------------------------------------------------------------------------
/// Given a Var containning "rgb" with bs::ColourRGB this makes and returns a
/// Var containning "rgb" with bs::ColRGB, for strange things that require bytes
/// instead of reals.
EOS_FUNC svt::Var * MakeByteRGB(svt::Var * var,cstrconst rgb = "rgb");

//------------------------------------------------------------------------------
/// A strange little method, this quantizises a field of real values in [0,1]
/// by a given number of steps. This is useful for algorithms which are badly
/// designed and require the data to be in such a form. Such algorithms are
/// especially badly designed when such behaviour is not explicitly indicated
/// and it takes me days to spot the problem. Such algorithm designers should
/// avoid living on the same continent as me.
EOS_FUNC void Quant(const svt::Field<real32> & in,svt::Field<real32> & out,nat32 steps = 256);

//------------------------------------------------------------------------------
/// Renders a mask to a rgb image, ushally to be saved to file imediately
/// afterwards. You can even choose the two mask colours, if you so desire.
/// (false==black and white==true by default.)
EOS_FUNC void MaskToRGB(const svt::Field<bit> & mask,svt::Field<bs::ColourRGB> & rgb,
                        const bs::ColourRGB & colF = bs::ColourRGB(0.0,0.0,0.0),
                        const bs::ColourRGB & colT = bs::ColourRGB(1.0,1.0,1.0));

//------------------------------------------------------------------------------
 };
};
#endif
