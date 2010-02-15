#ifndef EOS_FILTER_SPECULAR_H
#define EOS_FILTER_SPECULAR_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file specular.h
/// Provides specularity discovery algorithms.

#include "eos/types.h"

#include "eos/svt/field.h"
#include "eos/bs/colours.h"
#include "eos/time/progress.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// This is a simple algorithm that creates a mask of specularities in an image.
/// You provide an image and a segmentation of the image, it assumes constant 
/// albedo in each segment. You also provide a light source colour, though this
/// defaults to all white which is the norm for most scenes.
/// Works by creating a colour space where one of the channels entirelly contains
/// all specular affects, assuming dielectric materials. The nature of this colour
/// space is that the specular channel can be assumed constant in an area of 
/// constant albedo, except for the addative affect of specularity. By finding 
/// the minimum and thresholding specular pixels may be found.
/// The output mask has true where it finds specularities.
EOS_FUNC void SimpleSpecMask(const svt::Field<bs::ColourRGB> & image,
                             const svt::Field<nat32> & segs,
                             svt::Field<bit> & mask,const bs::ColourRGB & col = bs::ColourRGB(1.0,1.0,1.0));

//------------------------------------------------------------------------------
/// This removes specularities from an image. If you then need a specularity map
/// you can then calculate it by subtraction.
/// Based on 
/// 'Specularity Removal in Images and Videos: A PDE Approach'
/// by Mallick et al.
/// What its actually implimenting I frankly don't know, but it makes sense.
/// At least, it makes sense to me.
/// You have to provide the primary light colour, but guessing white is
/// reasonable for most images, so it defaults to that.
/// Some areas will simply be unrecoverable, a mask can be extracted to provide 
/// such areas - you can then use that for in-painting if you so desire.
class EOS_CLASS SpecRemoval
{
 public:
  /// &nbsp;
   SpecRemoval();
   
  /// &nbsp;
   ~SpecRemoval();


 /// Sets the input image and the output image. They can be the same image.
  void Set(const svt::Field<bs::ColourRGB> & in,svt::Field<bs::ColourRGB> & out);
  
 /// Sets the light source colour, defaults to white.
  void Set(const bs::ColourRGB & light);
  
 /// (Optionally) sets a mask, which will be set to true wherever the
 /// specularity is too strong to be removed. Such pixels should probably be 
 /// in-painted.
  void Set(svt::Field<bit> & mask);


 /// Calculates the result.
  void Run(time::Progress * prog = null<time::Progress*>());


 /// &nbsp;
  static inline cstrconst TypeString() {return "eos::filter::SpecRemoval";}


 private:
  svt::Field<bs::ColourRGB> in;
  svt::Field<bs::ColourRGB> out;
  bs::ColourRGB light;
  svt::Field<bit> mask;
};

//------------------------------------------------------------------------------
 };
};
#endif
