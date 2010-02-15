#ifndef EOS_FILTER_MATCHING_H
#define EOS_FILTER_MATCHING_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file matching.h
/// Provides methods that take pairs of images and corners lists and then
/// generate a similarity or dis-similarity matrix...

#include "eos/types.h"
#include "eos/svt/field.h"
#include "eos/ds/arrays.h"
#include "eos/filter/corner_harris.h"
#include "eos/bs/colours.h"
#include "eos/time/progress.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// This creates a similarity matrix (As a svt::Field) using normalised cross 
/// correlation. Expects to be given the output matrix allready the correct size.
/// (This is just the basic brute force implimentation - I just don't use this 
/// enough to justify extensive optimisation.)
/// (And incase you havn't guessed, no falloff is used.)
/// \param imgA The first image.
/// \param corA The list of corners in the first image.
/// \param imgB The second image.
/// \param corB The list of corners in the second image.
/// \param out The similarity matrix, the similarity of corner i in the first
///            image to corner j in the second image will on return be set to out.Get(i,j).
/// \param half The half width of the ncc, i.e. the window size is half*2+1.
///             All corners that are within half of the edge will have all relevent 
///             similarities set to 0.
/// \param addative If false, the default, it sets the similarity array,
///                 if true it multiplies it, useful for combining similaritys for 
///                 multiple fields of reals.
/// \param prog Progress bar, as ushall.
EOS_FUNC void MatchNCC(const svt::Field<real32> & imgA,const ds::Array<Corner> & corA,
                       const svt::Field<real32> & imgB,const ds::Array<Corner> & corB,
                       svt::Field<real32> & out,nat32 half,bit addative = false,
                       time::Progress * prog = null<time::Progress*>());

//------------------------------------------------------------------------------
/// This uses the NCC matcher but does it for each channel of a colour image and
/// then multiplies the similarities together to get a composite similarity 
/// matrix.
EOS_FUNC void MatchNCC(const svt::Field<bs::ColourRGB> & imgA,const ds::Array<Corner> & corA,
                       const svt::Field<bs::ColourRGB> & imgB,const ds::Array<Corner> & corB,
                       svt::Field<real32> & out,nat32 half,
                       time::Progress * prog = null<time::Progress*>());

//------------------------------------------------------------------------------
/// Given a similarity matrix this inteligently selects likelly matching corners.
/// Only considers a match if its the highest scoring match for both corners and
/// the second highest scoring match for each corner is less than a factor 
/// multiplied by the highest score.
/// \param simMat The similarity matrix to use.
/// \param out An array of points in the similarity matrix that are considered to be matches.
/// \param ratio The ratio of the best score divided by the second best that must be obtained for a match to be considered safe.
EOS_FUNC void MatchSelect(const svt::Field<real32> & simMat,ds::Array<bs::Pos> & out,real32 ratio = 0.7);

//------------------------------------------------------------------------------
/// A conveniance function, performs the common operation of generating an array
/// of pixels that match between two images. Uses the harris corner detector and
/// NCC, so not exactly advanced, but it works. Mostly.
/// \param imgA The first image.
/// \param imgB The second image.
/// \param out Output array of all matches found, the point in each image. imgA is the first image, imgB the second.
/// \param maxMatches Maximum number of matches to obtain, will ushally be less than whatever you put here however.
/// \param half The half window size used for NCC.
/// \param ratio The match score of the second best match for a corner divided by the best match score must be smaller than this value for it to be considered safe.
/// \param prog Optional progress report.
EOS_FUNC void MatchImages(const svt::Field<bs::ColourRGB> & imgA,const svt::Field<bs::ColourRGB> & imgB,
                          ds::Array<Pair<bs::Pnt,bs::Pnt> > & out,
                          nat32 maxMatches,nat32 half,real32 ratio = 0.7,
                          time::Progress * prog = null<time::Progress*>());

//------------------------------------------------------------------------------
 };
};
#endif
