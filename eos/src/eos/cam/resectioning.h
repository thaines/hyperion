#ifndef EOS_CAM_RESECTIONING_H
#define EOS_CAM_RESECTIONING_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file resectioning.h
/// Calculates camera projection matrics and radial parameters on being given
/// sufficient 3d to 2d corespondences.

#include "eos/types.h"
#include "eos/cam/cameras.h"
#include "eos/bs/geo2d.h"
#include "eos/bs/geo3d.h"
#include "eos/time/progress.h"

namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------
/// This does resectioning  - given a set of 3d world space and 2d image space
/// tuples (6+) it calculates the cameras matrix. Optionally calculates radial
/// paramters as well. Uses SVD for the first estimate, followed by LM for the
/// second without radial followed by LM with radial parameters. You can choose
/// how far it gets, so doing only very primative SVD is an option.
/// To do radial parameter estimation you will need a lot more matchesthan 6.
/// For LM assumes noise is in the image coordinates, i.e. that the 3d positions
/// are perfect.
class EOS_CLASS CalculateCamera
{
 public:
  /// Enum used to indicate the quality of a result, and set the quality limit,
  /// from failure to the most detailed model.
   enum ResQuality {failure, ///< Total failure, no useful numbers calculated.
                    low,     ///< Linear methods only, poor quality result.
                    normal,  ///< Full LM minimisation without radial parameters.
                    high     ///< Full LM minimisation with radial parameters.
                   };


  /// &nbsp;
   CalculateCamera();

  /// &nbsp;
   ~CalculateCamera();


  /// Adds a match.
   void AddMatch(const bs::Vert & world,const bs::Pnt & image);

  /// Adds a match.
   void AddMatch(const bs::Vertex & world,const bs::Point & image);

  /// Sets the maxmimum level it will calculate to, defaults to normal.
  /// (As high requires lots of data.)
   void SetQuality(ResQuality rq) {targQ = rq;}


  /// Calculates the result to the required quality, af6ter this adding is
  /// pointless but the get methods will return real data.
   void Calculate(time::Progress * prog = null<time::Progress*>());


  /// Returns the quality of the result.
   ResQuality GetQuality() const {return resQ;}

  /// Returns the residual. Will be negative if unknown, i.e. failure.
   real64 GetResidual() const {return residual;}

  /// Returns the calculated radial parameters.
  /// (Will all be zeroed out if not calculated.)
   const Radial & GetRadial() const {return radial;}

  /// Returns the calculated camera matrix.
   const Camera & GetCamera() const {return camera;}



  /// &nbsp;
   static cstrconst TypeString() {return "eos::cam::CalculateCamera";}


 private:
  // In...
   ResQuality targQ;
   struct Match
   {
    math::Vect<3,real64> world;
    math::Vect<2,real64> image;
   };
   ds::List<Match> data;


  // Out...
   ResQuality resQ;
   real64 residual;
   Radial radial;
   Camera camera;


  // Helper functions for LM...
   static void FirstLM(const math::Vector<real64> & pv,math::Vector<real64> & err,const ds::List<Match> & oi);
   static void FirstCon(math::Vector<real64> & pv,const ds::List<Match> & oi);
   static void SecondLM(const math::Vector<real64> & pv,math::Vector<real64> & err,const ds::List<Match> & oi);
   static void SecondCon(math::Vector<real64> & pv,const ds::List<Match> & oi);
};

//------------------------------------------------------------------------------
 };
};
#endif
