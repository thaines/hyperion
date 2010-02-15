#ifndef EOS_MYA_NEEDLES_H
#define EOS_MYA_NEEDLES_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file needles.h
/// Provides an implimentation of a needle map based data type.

#include "eos/mya/ied.h"
#include "eos/bs/geo3d.h"

namespace eos
{
 namespace mya
 {
//------------------------------------------------------------------------------
/// An Ied for representing a Needle map from a sfs algorithm with a smoothness
/// constraint.
class EOS_CLASS Needles : public Ied
{
 public:
  /// &nbsp;
  /// \param dir A normal for each pixel.
   Needles(const svt::Field<bs::Normal> & dir);

  /// &nbsp;
  /// \param dir A normal for each pixel.
  /// \param valid A validity map, true where disparity is valid, false where it is not.
   Needles(const svt::Field<bs::Normal> & dir,const svt::Field<bit> & valid);

  /// &nbsp; 
   ~Needles();


  /// &nbsp;
   str::Token Type(str::TokenTable & tt) const;

  /// &nbsp;
   real32 Distance(const math::Vector<real32> & a,const math::Vector<real32> & b) const;

  /// &nbsp;
   real32 FitCost(const math::Vector<real32> & feat,const Surface & surface,real32 x,real32 y) const;


  /// &nbsp;
   cstrconst TypeString() const;
};

//------------------------------------------------------------------------------
 };
};
#endif
