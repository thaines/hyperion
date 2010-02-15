#ifndef EOS_FILTER_RENDER_SEGS_H
#define EOS_FILTER_RENDER_SEGS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file render_segs.h
/// Provides several methods for rendering out the results of segmentation 
/// algorithms.

#include "eos/types.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"
#include "eos/bs/colours.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// This is given a segment field, where each segment has a unique number and
/// the field returns the number for each segment, it then outputs to the out field
/// lines where each segment transitions into another segment. Should only be 
/// given 2D data. The lines are the colour given, non-line areas are not touched,
/// so you can combine this with other techneques.
EOS_FUNC void RenderSegsLines(const svt::Field<nat32> & in,svt::Field<bs::ColourRGB> & out,const bs::ColourRGB & lc = bs::ColourRGB(1.0,1.0,1.0));

/// This is given a segment field, where each segment has a unique number and
/// the field returns the number for each segment, it then outputs to the out 
/// field patches of colour where each segment has a different colour. Should
/// only be given 2D data.
EOS_FUNC void RenderSegsColour(const svt::Field<nat32> & in,svt::Field<bs::ColourRGB> & out);

/// This is given a segment field, where each segment has a unique number and
/// the field returns the number for each segment, and teh original colour of
/// each entry, it then outputs to the out field the average colour of each 
/// segment. This looks quite good. 2D data only please. The second and third
/// fileds can be one and the same.
EOS_FUNC void RenderSegsMean(const svt::Field<nat32> & segs,const svt::Field<bs::ColourRGB> & col,svt::Field<bs::ColourRGB> & out);

//------------------------------------------------------------------------------
 };
};
#endif
