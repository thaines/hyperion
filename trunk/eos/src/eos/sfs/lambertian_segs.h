#ifndef EOS_SFS_LAMBERTIAN_SEGS_H
#define EOS_SFS_LAMBERTIAN_SEGS_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file lambertian_segs.h
/// This works with the lambertian_fit module to assign lambertian models to
/// segments in an image, its job is to merge segment models.

#include "eos/types.h"
#include "eos/svt/field.h"
#include "eos/bs/geo3d.h"
#include "eos/sfs/lambertian_fit.h"
#include "eos/alg/greedy_merge.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
/// As input you provide a segmentation, irradiance and surface orientation.
/// It then assigns a lambertian model to each segment, and merges segments with
/// similar models to get super segments. Merges can be of just lighting or 
/// albedo, rather than always being both at once.
class EOS_CLASS LambertianSeg
{
 public:
  /// &nbsp;
   LambertianSeg();

  /// &nbsp;
   ~LambertianSeg();


  /// Sets the irradiance map.
   void Set(const svt::Field<real32> & irr);

  /// Sets the segmentation.
   void Set(const svt::Field<nat32> & seg);
   
  /// Sets the needle map.
   void Set(const svt::Field<bs::Normal> & needle);
   
  /// Sets the parameters for the model fitter, see LambertianFit for details and defaults.
  /// This sets it into Genetic Algorithm mode.
   void SetGA(nat32 population,nat32 keptBest,nat32 immigrants,real32 mutationRate,
              nat32 generations,real32 albSd,real32 lightSd,
              real32 cutoff,real32 halflife);

  /// Sets the parameters for the model fitter, see LambertianFit for details and defaults.
  /// This sets it into Ransac mode.
   void SetRansac(real32 chance,real32 limit,nat32 cap,
                  real32 cutoff,real32 halflife);
                                    
  /// Sets true to refine, false to not. (With regard to model fitting.) Defaults to true.
   void Refine(bit r);
   
  /// Sets the parameters for model comparison, used for the greedy merging step
  /// to decide when to stop.
  /// \param irrSd Standard deviation of irradiance, for calculating how well a model fits. Defaults to 2.0
  /// \param irrCap Cap on maximum irradiance difference per pixel, to handle outliers. Defaults to 4.0
  /// \param freedomCost Cost of each degree of freedom, negative log probability. Defaults to 10.0
   void SetMC(real32 irrSd,real32 irrCap,real32 freedomCost);
  

  /// Calculates an answer, not fast.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// This extracts a model map, a set of vectors such that irradiance = out*needle.
   void GetModel(svt::Field<bs::Vert> & out);
   
  /// This extracts light orientation alone.
   void GetLight(svt::Field<bs::Normal> & out);
   
  /// This extracts albedo alone.
   void GetAlbedo(svt::Field<real32> & out);


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::sfs::LambertianSeg";}


 private:
  // Input fields...
   svt::Field<real32> irr;
   svt::Field<nat32> seg;
   svt::Field<bs::Normal> needle;


  // Fitter parameters...
   bit mode; // false = ga, true = ransac.
   bit refine;
   struct
   {
    nat32 population;
    nat32 keptBest;
    nat32 immigrants;
    real32 mutationRate;
    nat32 generations;
    real32 albSd;
    real32 lightSd;
   } fp_ga;
   
   struct
   {
    real32 chance;
    real32 limit;
    nat32 cap;
   } fp_rs;
   
   struct
   {
    real32 cutoff;
    real32 halflife;
   } fp_shared;


  // Model choosing parameters...
   real32 irrSd;
   real32 irrCap;
   real32 freedomCost;


  // Output...
   ds::Array<nat32> segToLight;
   ds::Array<nat32> segToAlbedo;
   ds::Array<bs::Normal> light;
   ds::Array<real32> albedo;
   
   
  // This method works out the negative log evidence for a model. Given two 
  // models of the same region(s) the model with the lower negative log evidence
  // should be choosen. Uses the Model Comparison principals of Ch. 28 from
  // 'Information Theory, Inference and learning Algorithms' by Mackay.
  // A larger gap of course indicates one model being significantly better than 
  // the other.
  // The laplace approximation is used, the Hessian is approximated as in 
  // Gauss-Newton iterations by J^T J.
  // Needless to say, this violates the laws of thermodynamics - it can make a 
  // headache pop into existance with it meer presence;-)
   real32 ModelCost(const NeedleSeg & ns,const NeedleSegModel & nsm) const;
   
   
  // Helper structure for the below and above helper methods...
   struct Store
   {
    real32 cutoff;
    real32 halflife;
   
    const NeedleSeg * ns;
    mutable ds::Array<bs::Normal> temp; // Tempory storage for extraction of light vectors.
   };
 

  // Helper method for the previous helper method...
   static void Func(const math::Vector<real32> & pv,
                    math::Vector<real32> & err,
                    const Store & store);


  // For obtainning edge lists...
   struct Edge
   {
    nat32 a; // a<b.
    nat32 b;
    
    bit operator < (const Edge & rhs) const
    {
     if (a!=rhs.a) return a<rhs.a;
     return b<rhs.b;
    }
   };
   
  // Helper stuff for pass 1...
   // The data structure associated with each node...
    class Node1 : public Deletable
    {
     public:
      ~Node1() {}
     
      mutable bit modelValid;
      mutable bs::Normal model; // Only valid if modelValid.
      ds::Array<IrrDir> samples;
    };
   
   // The interface given to teh greedy merger...
    class GMI1 : public alg::GreedyMergeInterface
    {
     public:
       GMI1(LambertianSeg * self);
      ~GMI1();

      real32 Cost(const Deletable * a) const;
      real32 Cost(const Deletable * a,const Deletable * b) const;
      Deletable * Merge(const Deletable * a,const Deletable * b) const;

     private:
      LambertianSeg * self;
      
      mutable LambertianFit lf;
      
      mutable NeedleSeg ns1;
      mutable NeedleSeg ns2;
      mutable NeedleSegModel nsm;
    };
    friend class GMI1;
};

//------------------------------------------------------------------------------
 };
};
#endif
