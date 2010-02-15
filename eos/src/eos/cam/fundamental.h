#ifndef EOS_CAM_FUNDAMENTAL_H
#define EOS_CAM_FUNDAMENTAL_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file fundamental.h
/// Provides robust fundamental matrix calculation tools.

#include "eos/types.h"
#include "eos/cam/cameras.h"
#include "eos/bs/geo2d.h"
#include "eos/time/progress.h"

namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------
/// Data structure used to represent two mathcing points, used throughout the
/// fundamental matrix calcualtion system.
struct EOS_CLASS FunMatch
{
 math::Vect<2,real64> left; ///< Point in left image.
 math::Vect<2,real64> right; ///< Point in right image.

 /// Helper method, given a fundamental matrix calculates the sum distance of
 /// each point from its epipolar line - this is the error term used throughout
 /// the fundamnetal matrix estimation system.
  real64 Dist(const Fundamental & fun) const
  {
   // Calcualte epipolar lines...
    math::Vect<3,real64> xb;
    xb[0] = right[0];
    xb[1] = right[1];
    xb[2] = 1.0;
    math::Vect<3,real64> la;
    math::TransMultVect(fun,xb,la);

    math::Vect<3,real64> xa;
    xa[0] = left[0];
    xa[1] = left[1];
    xa[2] = 1.0;
    math::Vect<3,real64> lb;
    math::MultVect(fun,xa,lb);

  // Normalise the lines...
   la.NormLine();
   lb.NormLine();

   // Calculate and return sum of distances from the epipolar lines...
    return math::Abs(la*xa) + math::Abs(lb*xb);
  }
};

//------------------------------------------------------------------------------
/// The seven point fundamental matrix calculation algorithm.
/// You provide an array of pointers to seven matches, it gives you the matching
/// matrix and returns true, or fails and returns false.
/// This does not normalise its input, whilst that shouldn't matter as this
/// isn't minimising you almost certainly should for numerical stability.
/// Provided external cos you might want to use this directly, but realistically
/// this exists for FunCalc to use.
bit SevenPointFun(FunMatch * match[7],Fundamental & out);

//------------------------------------------------------------------------------
/// This is given a fundamental matrix for both input and output and a set of
/// pointers to matches, it then refines the fundamental matrix to minimise the
/// residual. It ignores all provided matches that are too far away in either
/// the left or right image by using a capped distance measure in the minimisation.
/// You supply what too far is in the unit of the matches, pixels usually.
/// It does not normalise the given matches, it is not necesary as it uses LM
/// exclusivly.
/// It ignores any entry in the array that is set null.
/// Returns the residual it obtains, or -1.0 if it all goes to pot.
/// This is primarilly for use by FunCalc.
/// A negative tolerance switches off the tolerance affect.
real64 ManyPointFun(nat32 matchSize,FunMatch ** match,Fundamental & fun,real64 tolerance);

//------------------------------------------------------------------------------
/// Given a set of matches this calculates the fundamental matrix.
/// Uses the SVD approach, so this does not produce good quality output, see
/// ManyPointFun for that. This exists because ManyPointFun requires an input
/// guess, whilst this does not, so this can be used to initialise ManyPointFun.
/// The input can contain null pointers just like ManyPointFun, returns true on
/// success, false on failure. (Failure will mostly only happen due to
/// insufficient info.)
/// Degrades to using the 7 point algoirthm if only given 7 points.
bit ManyPointMirth(nat32 matchSize,FunMatch ** match,Fundamental & out);

//------------------------------------------------------------------------------
/// This calculates the fundamental matrix between two cameras. You provide it
/// with a large number of correspondences, each being a pair of 2d points with
/// a bit to indicate if they are reliable or not. If it has less than 7 reliable
/// points it uses ransac to get a set of reliable points. Further to that it
/// uses a full blooded minimisation for the final result with all points deemed
/// reliable. It allows querying of which points are deemed reliable and which are
/// not, post-calculation.
/// (Note that whilst a reliable point will definatly be used for initialisation
/// it can latter be ignored if it is found to not be good enough.)
/// Uses the most advanced techneques avaliable: ransac with the 7 point
/// algorithm and refinement using LM assumping Gaussian noise on input matches.
class EOS_CLASS FunCalc
{
 public:
  /// &nbsp;
   FunCalc();

  /// &nbsp;
   ~FunCalc();


  /// Adds a matching point pair, returns its index.
  /// \param left Coordinate in the left view.
  /// \param right Coordinate in te right view.
  /// \param reliable True if it should presume the pair to definatly be correct, false if it might be wrong.
  /// \returns The index of the point, acts like a growing array so it will be one
  ///          greater than the last point entered/equal to the current point count.
   nat32 AddMatch(const bs::Pnt & left,const bs::Pnt & right,bit reliable = false);

  /// Adds a matching point pair, returns its index.
  /// \param left Coordinate in the left view.
  /// \param right Coordinate in te right view.
  /// \param reliable True if it should presume the pair to definatly be correct, false if it might be wrong.
  /// \returns The index of the point, acts like a growing array so it will be one
  ///          greater than the last point entered/equal to the current point count.
   nat32 AddMatch(const math::Vect<2,real64> & left,const math::Vect<2,real64> & right,bit reliable = false);

  /// Returns how many matches are contained within.
   nat32 Matches() const;


  /// This calculates the fundamental matrix.
  /// Requires at least 7 reliable matches, prefably a lot more to do a good job.
  /// Ransac can't really cope with anything greater than a 50% error rate in the
  /// input data, so that has to be of a reasonable standard.
  /// Returns true on success, false on failure, and is even so nice as to provide
  /// a progress bar, though thats not very consistant as a measure of when things
  /// will complete.
  /// reliability sets how reliable a result ransac should produce, whilst cap is the
  /// maximum number of ransac runs to try before giving up and declaring failure.
   bit Run(time::Progress * prog = null<time::Progress*>(),real64 reliability = 0.99,nat32 cap = 10000);


  /// After Run returns true you can extract the fundamental matrix using this.
  /// It will go from the left view to the right view, you can reverse this by
  /// transposing it.
   const Fundamental & Fun() const;

  /// After Run returns true this returns the residual, this being the 2 norm of
  /// the vector distances from epipolar line to point for both views.
  /// Returns a negative if Fun() is returning bullshit.
   real64 Residual() const;

  /// This returns a different residual - the average Dist error per pixel.
  /// Makes more sense to human beings and small gnomes.
   real64 MeanError() const;

  /// Returns the number of matches actually used.
   nat32 UsedCount() const;

  /// This will output an array of bits, indicating which matches were used for
  /// the final answer.
  /// The array indexes will equate with the numbers returned by AddMatch.
  /// Will resize the passed in array if need be.
   void Used(ds::Array<bit> & out) const;


  /// &nbsp;
   static cstrconst TypeString() {return "eos::cam::FunCalc";}


 private:
  static const real64 ransacTol = 2.5;

  struct Match : public FunMatch
  {
   bit reliable; // True if the match is to be assumed correct, false if not.
   bit used; // True if the match was used, false if it wasn't.
   FunMatch norm; // Normalised version of the match, used during the calculation.
  };

  ds::List<Match> data;

  Fundamental fun;
  real64 residual;
  real64 meanError;
  nat32 usedCount;
};

//------------------------------------------------------------------------------
 };
};
#endif
