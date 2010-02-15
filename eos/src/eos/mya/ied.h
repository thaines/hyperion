#ifndef EOS_MYA_IED_H
#define EOS_MYA_IED_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file ied.h
/// Provides an abstraction of 'Image Extracted Data', (No, not improvised 
/// explosive device, though that would be far more fun.) this essentially
/// provides a data type to work with the SurfaceType etc classes, with the 
/// various features required for an input to a parametric surface fitting
/// stereo algorithm.

#include "eos/types.h"
#include "eos/math/vectors.h"
#include "eos/str/tokens.h"
#include "eos/ds/arrays.h"
#include "eos/svt/field.h"
#include "eos/mya/surfaces.h"

namespace eos
{
 namespace mya
 {
//------------------------------------------------------------------------------
/// Represents Image Extracted Data for a particular image. The interface allows
/// for a set of image-constant features and a set of image-variable features to
/// be set, it then forms for each location in the image on request a representation
/// vector consisting of both groups of data. Designed to be inherited from, it
/// provides much of the interface protected so the interface is adapted to a
/// particular scenario by the child. In addition a child class has to provide
/// methods for returning a distance metric between the data and a given surface
/// representing depth, and between two feature vectors.
class EOS_CLASS Ied : public Deletable
{
 public:
  /// The object is constructed with the numbers of constant features and
  /// variable features, these are ushally provided by the child 
  /// implimentation rather than the child user.
   Ied(nat32 constFeats,nat32 variableFeats);

  /// &nbsp;
   ~Ied();


  /// This returns the length of the representation vector, so you can declare one
  /// that is at least the right size.
   nat32 Length() const;
   
  /// Returns the width of the image.
   nat32 Width() const;
   
  /// Returns the height of the image.
   nat32 Height() const;
   
  /// Returns if the representation at a particular point is valid or not, identical 
  /// to the Feature method except it dosn't output the feature itself.
   bit Valid(nat32 x,nat32 y) const;
   
  /// This outputs the feature representation at a given location, the vector passed in
  /// must be at least as large as Length(), but can be bigger, any extra 
  /// elements will be ignored. returns true if out is set, false otherwise.
  /// false indicates the data is invalid at this particular point.
   bit Feature(nat32 x,nat32 y,math::Vector<real32> & out) const;
   
   
  /// This returns the type token for the data type, which will correlate with the
  /// type passed into the SurfaceType object. This should imply a particular 
  /// layout and meaning of representation vector, so the Fitter class knows what to do
  /// with it.
   virtual str::Token Type(str::TokenTable & tt) const = 0;
   
  /// Returns a distance between any two representations, any distance metric will do.
   virtual real32 Distance(const math::Vector<real32> & a,const math::Vector<real32> & b) const = 0;

  /// Returns a distance metric between a representation and a surface representing depth, where 0 indicates
  /// the surface in question matches the feature and the larger the number means the greater the 
  /// surface deviates from the representation. This all happens at a particular (x,y) coordinate. This
  /// is used for both minimisation and for outlier/inlier decision making.
   virtual real32 FitCost(const math::Vector<real32> & feat,const Surface & surface,real32 x,real32 y) const = 0;


  /// &nbsp;
   virtual cstrconst TypeString() const = 0;


 protected:
  /// Sets the validity map, which indicates where we have valid 
  /// features and where we don't. If not called it assumes validity
  /// everywhere.
   void SetValidity(const svt::Field<bit> & valid);

  /// Sets the image-constant feature at the given [0..constFeats) index.
   void SetConst(nat32 index,real32 value);

  /// Sets the image-constant feature at the given [0..variableFeats) index.
   void SetVariable(nat32 index,const svt::Field<real32> & value);


 private:
  svt::Field<bit> valid;
  
  ds::Array< real32 > constFeats;
  ds::Array< svt::Field<real32> > variableFeats; 
};

//------------------------------------------------------------------------------
 };
};
#endif
