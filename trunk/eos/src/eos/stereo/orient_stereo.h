#ifndef EOS_STEREO_ORIENT_STEREO_H
#define EOS_STEREO_ORIENT_STEREO_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file orient_stereo.h
/// Provides a stereo algorithm that works in orientation space, i.e. applys the
/// DSI directly to surface orientation rather than any other representation.

#include "eos/types.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"
#include "eos/time/progress.h"
#include "eos/bs/colours.h"
#include "eos/bs/geo3d.h"
#include "eos/cam/disparity_converter.h"
#include "eos/inf/field_graphs.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// This attempts to do stereo with a representation of orientation and no 
/// representation of disparity. This uses two parameters to represent 
/// orientation, albedo and direction, these being taken directly from the SFS 
/// cone constraint.
/// This means that the SFS constraint is implicit in the representation.
/// A Potts smoothing term is then applied to albedo, so as to make the 
/// piece-wise constant albedo assumption, a Von-Mises distribution is applied 
/// to direction to select the smoothest surface independent of the Potts term.
/// To apply a DSI to this surface the fact that different orientations equate 
/// to different disparity differences is used. The cost of each orientation is 
/// the minimum cost possible for the sum of two adjacent matches costs, given
/// the disparity difference of the given orientation. This means we have a 
/// joint distribution between matched albedo and direction pairs that encodes
/// the DSI.
class EOS_CLASS OrientStereo
{
 public:
  /// &nbsp;
   OrientStereo();

  /// &nbsp;
   ~OrientStereo();


  /// Sets the disparity range and the resolution of the albedo and orientation 
  /// representations.
   void Set(int32 minDisp,int32 maxDisp,nat32 albRes,nat32 orientRes);

  /// Sets the parameters.
  /// \param alphaK For the surface orientation smoothing term.
  /// \param alphaErr Probability of the surface orientation smoothing term being wrong.
  /// \param beta Cost of a mismatch of the Potts term applied to the albedo.
  /// \param gammaMult Multiplier of difference between matching pixels, for creating the DSI.
  /// \param gammaMax Cap on the cost of a DSI entry.
   void Set(real32 alphaK,real32 alphaErr,real32 beta,real32 gammaMult,real32 gammaMax);


  /// Sets the two input images. They must be the same height.
   void SetIr(const svt::Field<bs::ColourLuv> & left,const svt::Field<bs::ColourLuv> & right);

  /// Sets the two to-light vectors. It automatically normalises them.
   void SetLight(const bs::Normal & left,const bs::Normal & right);

  /// Sets the two output surface orientation fields.
   void SetNeedle(svt::Field<bs::Normal> & left,svt::Field<bs::Normal> & right);

  /// Optional, sets two output fields into which the normalised albedo is written to.
   void SetAlbedo(svt::Field<bs::ColourL> & left,svt::Field<bs::ColourL> & right);
  
   
  /// Sets the convertor for turning disparitys into orientations.
   void SetDispConv(const cam::DispConv & left,const cam::DispConv & right);


  /// Fills in all the given outputs.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  int32 minDisp;
  int32 maxDisp;
  nat32 albRes;
  nat32 orientRes; // Orient==0 is pointing down the x==0 +ve axis.

  real32 alphaK;
  real32 alphaErr;
  real32 beta;
  real32 gammaMult;
  real32 gammaMax;

  svt::Field<bs::ColourLuv> ir[2];
  bs::Normal toLight[2];
  svt::Field<bs::Normal> needle[2];
  svt::Field<bs::ColourL> albedo[2];

  cam::DispConv * dispConv[2];
  
  // Helper methods...
   void RunHalf(time::Progress * prog,nat32 leftInd);
};

//------------------------------------------------------------------------------
 };
};
#endif
