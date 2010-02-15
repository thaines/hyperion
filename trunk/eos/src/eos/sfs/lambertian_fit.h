#ifndef EOS_SFS_LAMBERTIAN_FIT_H
#define EOS_SFS_LAMBERTIAN_FIT_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

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


/// \file lambertian_fit.h
/// This fits a lambertian model to pairs of irradiance and orientation
/// information.

#include "eos/types.h"
#include "eos/bs/geo3d.h"
#include "eos/ds/arrays.h"
#include "eos/time/progress.h"
#include "eos/alg/genetic.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
/// This class represents a pixel with irradiance and orientation information.
/// Also assigned a weighting of how reliable it is thought to be.
/// Used for surface model estimation.
class EOS_CLASS IrrDir
{
 public:
  /// Irradiance of sample.
   real32 irr;

  /// Orientation of sample.
   bs::Normal dir;

  /// Weighting of how good it is meant to be, 1 as good as it gets, 0 useless.
   real32 weight;


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::sfs::IrrDir";}
};

//------------------------------------------------------------------------------
/// Support class, this is used by both LambertianRefine and LambertianEA to
/// store the data set they infer there answers from.
/// This consists of multiple pointers to externally stored arrays of IrrDir
/// samples, each with an assigned light and albedo identifier.
class EOS_CLASS NeedleSeg
{
 public:
  /// You construct with the number of segments to be stored.
   NeedleSeg(nat32 segments);

  /// &nbsp;
   ~NeedleSeg();


  /// Number of segments stored.
   nat32 Segments() const;

  /// Sets a given segemnt.
   void Set(nat32 seg,nat32 lightGroup,nat32 albedoGroup,const ds::Array<IrrDir> & samples);

  /// Returns the light id for a given segment.
   nat32 LightGroup(nat32 seg) const;

  /// Returns the albedo id for a given segment.
   nat32 AlbedoGroup(nat32 seg) const;

  /// Returns the IrrDir array for a given segment.
   const ds::Array<IrrDir> & Samples(nat32 seg) const;


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::sfs::NeedleSeg";}


 private:
  struct Segment
  {
   nat32 light;
   nat32 albedo;
   const ds::Array<IrrDir> * samples;
  };

  ds::Array<Segment> segs;
};

//------------------------------------------------------------------------------
/// This represents the output of LambertianRefine and LambertianEA, an
/// assignment of direction for each light group and albedo for each albedo group.
/// Paired with a NeedleSeg the group ids being the array indices for this.
class EOS_CLASS NeedleSegModel
{
 public:
  /// You provide how large the light and albedo arrays need to be.
   NeedleSegModel(nat32 lightSize,nat32 albedoSize);

  /// &nbsp;
   ~NeedleSegModel();
   
  /// Resizes the model.
   void Resize(nat32 lightSize,nat32 albedoSize);


  /// Returns how many light directions exist.
   nat32 Lights() const;

  /// Allows access to a light direction.
   bs::Normal & Light(nat32 lightGroup);

  /// &nbsp;
   const bs::Normal & Light(nat32 lightGroup) const;


  /// Returns how many albedos exist.
   nat32 Albedos() const;

  /// Allows access to an albedo direction.
   real32 & Albedo(nat32 albedoGroup);

  /// &nbsp;
   const real32 & Albedo(nat32 albedoGroup) const;


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::sfs::NeedleSegModel";}


 private:
  ds::Array<bs::Normal> light;
  ds::Array<real32> albedo;
};

//------------------------------------------------------------------------------
/// This is given as input a NeedleSeg and a NeedleSegModel, the NeedleSegModel
/// is then modified to be the output. It takes estimates of the model given in
/// the input NeedleSegModel and refines them with LM to get the final output.
/// Some method of guessing reasonable starting coniditions is required, this is
/// primarilly provided here by LambertianEA.
class EOS_CLASS LambertianRefine
{
 public:
  /// &nbsp;
   LambertianRefine();

  /// &nbsp;
   ~LambertianRefine();


  /// Sets the input NeedleSeg.
   void Set(const NeedleSeg & ns);

  /// Sets the input and output NeedleSegModel.
   void Set(NeedleSegModel & nsm);

  /// Sets the parameters, these are simply a cutoff to make the process robust
  /// to outliers.
  /// This done softly with a sigmoid on angular error (In radians) between
  /// surface orientation given and the cone calculated by the lambertian equation.
  /// Defaults to a cutoff of pi/6 and a halflife of pi/36.
   void Set(real32 cutoff,real32 halflife);


  /// Runs the algorithm, once done the given NeedleSegModel will have been
  /// changed.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::sfs::LambertianRefine";}


 private:
  // Input/output variables...
   const NeedleSeg * ns;
   NeedleSegModel * nsm;
   real32 cutoff;
   real32 halflife;

  // Tempory variable used to optimise below helper...
   mutable ds::Array<bs::Normal> temp;

  // Helper function, this is where all the clever shit goes...
   static void Func(const math::Vector<real32> & pv,
                    math::Vector<real32> & err,
                    const LambertianRefine & self);
};

//------------------------------------------------------------------------------
/// This uses a genetic algorithm approach to estimate shading models for the
/// system descibed by a NeedleSeg, outputting the result into a NeedleSegModel.
class EOS_CLASS LambertianEA : private alg::GeneticOrg
{
 public:
  /// &nbsp;
   LambertianEA();

  /// &nbsp;
   ~LambertianEA();


  /// Sets the input NeedleSeg.
   void Set(const NeedleSeg & ns);

  /// Sets the output NeedleSegModel.
   void Set(NeedleSegModel & nsm);

  /// Sets the parameters of the model.
  /// \param population Population of the society that goes through the genetic algorithm. Defaults to 100.
  /// \param keptBest Number of best scorers that pass from each generation to the next. Defaults to 5.
  /// \param immigrants Number of completly new members created each generation. Defaults to 5.
  /// \param mutationRate Chance of any given member being mutated, horribly. Defaults to 0.1.
  /// \param generations Number of generations to simulate. Defaults to 250.
  /// \param albSd When mutating albedo it is offset by a gaussian with this sd. Defaults to 1.0.
  /// \param lightSd When mutating light directions each component has a gaussian with this sd added. Done with spherical coordinates. Defaults to 0.1.
  /// \param cutoff Maxmimum error, in radians, per sample. For calculating fitness. Defaults to pi/6.
   void Set(nat32 population,nat32 keptBest,nat32 immigrants,real32 mutationRate,
            nat32 generations,real32 albSd,real32 lightSd,
            real32 cutoff);


  /// Runs the algorithm, once done the given NeedleSegModel will have been
  /// filled in.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::sfs::LambertianEA";}


 private:
  // Parameters...
   nat32 population;
   nat32 keptBest;
   nat32 immigrants;
   real32 mutationRate;
   nat32 generations;
   real32 albSd;
   real32 lightSd;
   real32 cutoff;

  // Input/Output...
   const NeedleSeg * ns;
   NeedleSegModel * nsm;

  // Random number generator...
   data::Random rand;

  // Structure representing the head of an organism...
   struct Organism
   {
    mutable real32 fitness; // Cached, negative used to indicate uncalculated.

    // Followed by size(lightGroup)*2 reals as spherical coordinates
    // representing light directions then size(albedoGroup) reals representing
    // albedos.
    real32 pv[1]; // First entry, makes indexing 'em easy.
   };

  // Methods from GeneticOrg...
   nat32 Size() const;
   void New(void * newOrg,data::Random & rand) const;
   real32 Fitness(const void * org) const;
   void Breed(const void * mother,const void * father,void * child,data::Random & rand) const;
   void Mutate(void * org,data::Random & rand) const;

  // Helper - calculates the fitness of an organism, to be called when it is negative.
   void CalcFitness(const Organism * org) const;

  // Support structures to optimise the 'New' method...
   ds::Array<nat32> lightCounter;
   ds::Array<nat32> albedoCounter;
};

//------------------------------------------------------------------------------
/// This uses a RANSAC approach to estimate shading models for the
/// system descibed by a NeedleSeg, outputting the result into a NeedleSegModel.
class EOS_CLASS LambertianRANSAC
{
 public:
  /// &nbsp;
   LambertianRANSAC();

  /// &nbsp;
   ~LambertianRANSAC();


  /// Sets the input NeedleSeg.
   void Set(const NeedleSeg & ns);

  /// Sets the output NeedleSegModel.
   void Set(NeedleSegModel & nsm);

  /// Sets the parameters of the model.
  /// \param cutoff Maxmimum error, in radians, for a sample to count. Defaults to pi/6.
  /// \param chance Chance of success. Defaults to 0.99.
  /// \param limit Maximum albedo accepted - albedos larger than this are assumed to be silly and ignored. Defaults to 250.
  /// \param cap Maximum number of iterations, to avoid insane processing times. Defaults to 10000.
   void Set(real32 cutoff,real32 chance,real32 limit,nat32 cap);


  /// Runs the algorithm, once done the given NeedleSegModel will have been
  /// filled in.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::sfs::LambertianRANSAC";}


 private:
  // Parameters...
   real32 cutoff;
   real32 chance;
   real32 limit;
   nat32 cap;

  // Input/Output...
   const NeedleSeg * ns;
   NeedleSegModel * nsm;

  // Random number generator...
   data::Random rand;
};

//------------------------------------------------------------------------------
/// This simply provides an interface to LambertianEA/LambertianRansac and 
/// LambertianRefine, calling them in turn and providing a single interface for
/// parameters etc.
class EOS_CLASS LambertianFit
{
 public:
  /// &nbsp;
   LambertianFit();

  /// &nbsp;
   ~LambertianFit();


  /// Sets the input NeedleSeg.
   void Set(const NeedleSeg & ns);

  /// Sets the output NeedleSegModel.
   void Set(NeedleSegModel & nsm);

  /// Sets the parameters of the model for doing it with a genetic algorithm.
  /// These are the merged list from the two modules used, see LambertianEA and
  /// LambertianRefine for details. Note that cutoff is used differently by the
  /// two, though should be set to the same value hence only being provided once
  /// here.
   void SetGA(nat32 population,nat32 keptBest,nat32 immigrants,real32 mutationRate,
              nat32 generations,real32 albSd,real32 lightSd,
              real32 cutoff,real32 halflife);

  /// Sets the parameters of the model for doing it with ransac.
  /// These are the merged list from the two modules used, see LambertianRansac and
  /// LambertianRefine for details. Note that cutoff is used differently by the
  /// two, though should be set to the same value hence only being provided once
  /// here.
   void SetRansac(real32 chance,real32 limit,nat32 cap,
                  real32 cutoff,real32 halflife);
                  
  /// Sets whether to refine or not, defaults to true.
   void Refine(bit r);

  /// Runs the algorithm, once done the given NeedleSegModel will have been
  /// filled in.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::sfs::LambertianFit";}


 private:
  // Parameters...
   // Mode...
    bit mode; // false = ga, true = ransac.
    bit refine; // Whether to refine or not.
   // GA parameters...
    nat32 population;
    nat32 keptBest;
    nat32 immigrants;
    real32 mutationRate;
    nat32 generations;
    real32 albSd;
    real32 lightSd;
  
   // ransac parameters...
    real32 chance;
    real32 limit;
    nat32 cap;
    
   // Shared...
    real32 cutoff;
    real32 halflife;

  // Input/Output...
   const NeedleSeg * ns;
   NeedleSegModel * nsm;
};

//------------------------------------------------------------------------------
 };
};
#endif
