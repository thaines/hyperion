#ifndef EOS_CAM_CALIBRATION_H
#define EOS_CAM_CALIBRATION_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

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


/// \file calibration.h
/// Provides a class for estimating a cameras intrinsic matrix using multiple
/// images of a 2D pattern of known shape.

#include "eos/types.h"
#include "eos/ds/arrays.h"
#include "eos/ds/lists.h"
#include "eos/time/progress.h"
#include "eos/math/mat_ops.h"
#include "eos/cam/cameras.h"

namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------
/// A (modified) implimentation of 'A Flexible New Techneque for Camera
/// Calibration' by Zhengyou Zhang, 1998. Given a set of point correspondences 
/// for muliple shots of a 2D test pattern (i.e. something that a printer will
/// spit out.) this will calculate both the cameras intrinsic matrix and its
/// radial distortion upto a 4th term. Will work with just 2 photographs,
/// however that involves a simplification of the intrinsic matrix, 3 photographs
/// is the real minimum, and in practise at least 5 should be used. Note that
/// the pattern does not have to be the same between images, just as long as a
/// large number of model/image correspondences are provided per pattern and the
/// paterns are shot from different and non-degenerate angles. (In principal, 
/// though untested, there is nothing to stop you doing multiple patterns in the 
/// same image by having them in different planes, so you could calibrate from
/// just one image.)
/// The modifications are that we use a more detailed camera distortion model, see 
/// the Radial class for details. The extra complexity results in a 3 stage implimentation:
/// - First guess using SVD without distortion of intrinsic matrx.
/// - LM refinement of guess including distortion, but only upto second term with distortion centre and principal point identical.
/// - LM refinement of all parameters. (Without this stage we practically have the original Zhang98.)
class EOS_CLASS Zhang98
{
 public:
  /// Enum of possible result states, from absolute failure to high quality results.
   enum ResQuality {failure, ///< Total fauilure, no useful calibration calculated.
                    low,     ///< Refinment failed, result has been calculated with linear methods only. Should generally be concidered unusable.
                    normal,  ///< Fully minimised with 2 terms of radial distortion, the ushall level cameras are calibrated to, further details can be concidered excessive.
                    high     ///< Calculated to 4 terms of radial distortion with a different centre of radial distortion. This mode is tried but generally fails as the system is too sensitive for a stable minimisation.
                   };


  /// &nbsp;
   Zhang98();
   
  /// &nbsp;
   ~Zhang98();
   
  /// Resets the data, as though none had been Add() -ed to the object.
   void Reset();


  /// Adds a point pair for a specific shot, non-homogenous. The shot numbers should 
  /// start at 0 and increase without gaps, as internally an array is used to group them.
  /// m is the pattern position whilst i is the imaged position that has been recorded.
   void Add(nat32 shot,const math::Vect<2,real32> & m,const math::Vect<2,real32> & i);

  /// Adds a point pair for a specific shot, non-homogenous. The shot numbers should 
  /// start at 0 and increase without gaps, as internally an array is used to group them.
  /// m is the pattern position whilst i is the imaged position that has been recorded.
   void Add(nat32 shot,const math::Vect<2,real64> & m,const math::Vect<2,real64> & i);

  /// Adds a point pair for a specific shot, homogenous. The shot numbers should 
  /// start at 0 and increase without gaps, as internally an array is used to group them.
  /// m is the pattern position whilst i is the imaged position that has been recorded.
   void Add(nat32 shot,const math::Vect<3,real32> & m,const math::Vect<3,real32> & i);

  /// Adds a point pair for a specific shot, homogenous. The shot numbers should 
  /// start at 0 and increase without gaps, as internally an array is used to group them.
  /// m is the pattern position whilst i is the imaged position that has been recorded.
   void Add(nat32 shot,const math::Vect<3,real64> & m,const math::Vect<3,real64> & i);
   
   
  /// Sets the maximum quality level to obtain, defaults to high.
   void SetQuality(ResQuality rq);

  
  /// Once enough data has been provided this calculates the calibration. The various
  /// Get* methods will start producing sensible results after a call to this.
   void Calculate(time::Progress * prog = null<time::Progress*>());   
  

  /// Returns the quality of the result.
   ResQuality GetQuality() const {return resQ;}
   
  /// Returns the residual.
   real64 GetResidual() const {return residual;}

  /// Returns the calculated intrinsic matrix.
   const Intrinsic & GetIntrinsic() const {return intrinsic;}
   
  /// Returns the calculated radial parameters.
   const Radial & GetRadial() const {return radial;}
   
  /// Returns how many shots are contained, incase you have 'forgotten'.
  /// Dosn't work if the algorithm has failed.
   nat32 GetShots() const {return extrinsic.Size();}
   
  /// Returns the extrinsic parameters for a given shot, only call in non-failure
  /// for shots that actually exist.
   const Extrinsic & GetExtrinsic(nat32 shot) const {return extrinsic[shot];}


  /// &nbsp;
   static cstrconst TypeString() {return "eos::cam::Zhang98";}


 private:
  // Data structure used to store data post-calculation...
   struct Node
   {
    nat32 shot;
    math::Vect<2,real64> first;	  
    math::Vect<2,real64> second;
   };
  
   ResQuality targQ; // Maximum quality to get to.
   ds::List<Node> data;
   
  // Data types used by the algorithm during run-time...
   struct Shot
   {
    ds::List<Node> data;
    math::Mat<3,3,real64> hg;
    math::Mat<3,3,real64> hgNorm; // Version of above with noramlised coordinates, for initial estimate.
    real64 residual; // Just for logging purposes.
   };
     
   struct LMdata
   {
    nat32 shots;
    Shot * sd;
   };
   
  // Data for the actual results...
   ResQuality resQ;
   real64 residual;
   Intrinsic intrinsic;
   Radial radial;   
   ds::Array<Extrinsic> extrinsic;


  // Error metrics for the two LM steps...
   static void FirstLM(const math::Vector<real64> & pv,math::Vector<real64> & err,const LMdata & oi);
   static void SecondLM(const math::Vector<real64> & pv,math::Vector<real64> & err,const LMdata & oi);
};

//------------------------------------------------------------------------------
 };
};
#endif
