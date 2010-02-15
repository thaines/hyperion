#ifndef EOS_ALG_MEAN_SHIFT_H
#define EOS_ALG_MEAN_SHIFT_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \namespace eos::alg
/// Provides various algorithm implimentations, complicated algorithms only. 
/// Anything which dosn't specifically belong in another namespace ends up here.

/// \file mean_shift.h
/// Provides the mean shift algorithm. Supports lattice optimisation.

#include "eos/types.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"
#include "eos/ds/arrays.h"
#include "eos/ds/lists.h"
#include "eos/time/progress.h"

namespace eos
{
 namespace alg
 {
//------------------------------------------------------------------------------
/// A mean shift algorithm implimentation, you create the class for each run,
/// tied in directly with the svt. To do a run you first register all the 
/// information it needs, then you call Run(), then you extract the resulting
/// information you want, before finally deleting the class.
/// You start and often end by specifying a set of features, there are two types, 
/// either Field objects as sources of features on a per sample basis or 
/// specifying that the indexing information for samples is also a feature.
/// The latter is subject to a lattice optimisation, and is very much 
/// prefered for speed. All features have a scale, by which they are 
/// multiplied before they are used in the algorithm so you can adjust there
/// relevent importance. You can also provide a window size, this is the same
/// as multiplying all the scales by 1/size as internaly the window is fixed 
/// to range 1. The algorithm can use either no falloff or linear falloff for
/// its kernel, which reaches 0 at the edge of the window, but with scalars for
/// each entry, so its a diagonal matrix in other words. As a note on optimisation, 
/// the algorithm will work faster with the limited dimensions first in the input
/// structure and the larger scale features given first.
class EOS_CLASS MeanShift
{
 public:
  /// &nbsp;
   MeanShift();
  
  /// &nbsp;
   ~MeanShift();


  /// Changes the distance measure to a function of your own choice.
  /// By default the distance is eucledian, however, any other good measure 
  /// will do, though the ability to use dimensions as features will fail at
  /// that point if you allow dimensions larger that there distance to be accepted.
  /// It should return true if its in range, false otherwise. The feature
  /// vectors will be arrays of fvSize, of real values, they will have been
  /// scaled at this point. The features in the vector will be in the same order
  /// as you call the AddFeature function, presuming no dimension features are added,
  /// as they allways appear first in dimension index order. pt provides a pass through
  /// value, for scaling and the like. Due to the scaling the cutoff is 1.0, so to 
  /// impliment your own eucledian you work out the distance between the two passed
  /// variables, if less than 1 return true, else return false.
   void SetDistance(bit (*Distance)(nat32 fvSize,real32 * fv1,real32 * fv2,real32 pt),real32 pt);

  /// This registers a svt::Field which must go to a real32 as an element in the
  /// feature vector. You also supply a weighting, which will multiply any 
  /// extracted value before it is used. All fields must have the same dimensions and
  /// sizes. At least one of these must be provided, as they are used to get the size
  /// of the data structure, and running mean shift without these would be silly anyway.
  /// (Unless svt::Var supported sparse data structures, but it doesn't.)
   void AddFeature(const svt::Field<real32> & field,real32 scale = 1.0);

  /// This registers a given dimension as an element in the feature vector, so 
  /// position in the data structure is used for clustering, enables the lattice 
  /// optimisation. You can also supply a weighting. dim must be in the range of 
  /// the allready passed in Var.
   void AddFeature(nat32 dim,real32 scale = 1.0);

  /// This sets a weighting for each sample when calculating the mean, if not called
  /// it is presumed to be constant, should only be called once.
   void SetWeight(const svt::Field<real32> & field);
   
  /// The passover optimisation only works if using all dimensions as feature vector members,
  /// and will in all likelyhood reduce the quality of the results, it does however 
  /// produce a major speed up, and has little effect on the results of image segmentation.
  /// Defaults to false, as there arn't many situations when you want this on. Essentially
  /// it works by checking which item the feature vector is currently dimensionally over, 
  /// if its one that has not converged it assumes its going to converge to the same 
  /// point and sets its convergence point to where this point ultimatly ends up.
  /// It only conciders points where there euclidean distance for the scaled non-positional parts
  /// of the feature vector is less than half. This just so happens to be the required
  /// behaviour for a mean shift image smoothing:-)
   void Passover(bit enabled);

  /// A conveniance, allows you to set the window size independently of the scales set.
   void SetWindowSize(real32 size);
   
  /// Allow you to set the cutoff points for convergence, the cutoff shift distance squared, so
  /// that cutoff stops when the shift is smaller than the given value, and an absolute
  /// maximum number of iterations, to make sure it runs fast enough.
  /// The cutoff defaults to 0.01 and the maximum iteration count to 100, these are 
  /// appropriate values for smoothing an image but for most other scenarios larger values
  /// would be recomended.
   void SetCutoff(real32 change,nat32 maxIter);
 

  /// This calculates the results, it switches the class over from setup mode to
  /// result extraction mode.
   void Run(time::Progress * prog = null<time::Progress*>());



  /// This extracts the results, you give it the Field for which you want to 
  /// extract the mean shift converged values and then an output field to write
  /// these values to. They can be the same field. Returns true on success, false
  /// on error.
   bit Get(svt::Field<real32> & index,svt::Field<real32> & out);

  /// This extracts the results, you give it the dimension for which you want
  /// to extract the mean shift values and an output field to write these values
  /// to. The output field has to be floating point, due to the nature of the
  /// algorithm. Returns true on success, false on error.
   bit Get(nat32 dim,svt::Field<real32> & out);


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::alg::MeanShift";}


 private:
  // Result quality variables...
   real32 cutoff; // When the shift eucledian distance is less than the square-root of this stop shifting.
   real32 max_iter; // Maximum number of iterations to do before giving up, assuming above is not reached.
   bit passOpt; // When true the pass-over optimisation is done.

  // Methods used during the calculation...
   // Calculates the starting vector for a particular entry.
    void CalcVector(nat32 * pos,real32 * out);
   // Calculates shift, given mi,ma and pos as tempory data for it to use, arrays of dim size, 
   // data is the array to get values from, stride is the offset for each dimension, vector as 
   // the vector+weight of the vector to calculate for and mean as the output mean value for the window.
    void CalcShift(nat32 * mi,nat32 * ma,nat32 * pos,real32 * data,nat32 * stride,real32 * vector,real32 * mean);

  // Inputs...
   real32 window; // Size of window.
   svt::Field<real32> weight; // Only used if .Valid()

   struct Dim
   {
    bit use; // True if we use the dimension, as indicated by the index in the dim array, as a feature.
    real32 scale;
   };   
   ds::Array<Dim> dim;

   struct Feat
   {
    svt::Field<real32> field;
    real32 scale;
   };
   ds::Array<Feat> sf; // Dynamically resized with each new entry, slow yes, but not compared to Run().   


  // Intermediate...
   nat32 samples; // Also the size of the below forest.
   nat32 diUsed; // Number of entry taken from dimensions in feature vector. Stored at the front of the vector.
   nat32 fvSize; // Feature vector size.


  // Outputs...
   real32 * out;

  // Distance vector replacement stuff...
   // The pointer to the distance function...
    bit (*Distance)(nat32 fvSize,real32 * fv1,real32 * fv2,real32 pt);
   // The pass through variable, passed into the distance function, ushally a scaler.
    real32 passThrough;
   // The default feature vector distance proccessor...
    static bit DistDefault(nat32 fvSize,real32 * fv1,real32 * fv2,real32 pt);
};

//------------------------------------------------------------------------------
 };
};
#endif
