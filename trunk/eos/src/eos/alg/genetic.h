#ifndef EOS_ALG_GENETIC_H
#define EOS_ALG_GENETIC_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file genetic.h
/// Provides genetic algorithms for optimising stuff.

#include "eos/types.h"
#include "eos/math/stats.h"
#include "eos/math/vectors.h"
#include "eos/time/progress.h"
#include "eos/ds/arrays.h"
#include "eos/data/randoms.h"

namespace eos
{
 namespace alg
 {
//------------------------------------------------------------------------------
/// Pure abstract class which returns the fitness of a given real parameter set.
/// To use the genetic algorithm optimiser you inherit from this class providing
/// all the relevent methods for it to work.
class EOS_CLASS GeneticRealFunc : public Deletable
{
 public:
  /// &nbsp;
   ~GeneticRealFunc() {}


  /// This returns how many parameters this function takes.
   virtual nat32 ParaCount() const = 0;
   
  /// This outputs the range of acceptable values for a given parameter index.
   virtual void ParaRange(nat32 index,math::Range<real32> & out) const = 0;
   
  /// Given a parameter set this should return the fitness of the particular
  /// parameter set. As it can take some time a progress object is also provided.
   virtual real32 Fitness(const math::Vector<real32> & paras,time::Progress * prog) const = 0;
};

//------------------------------------------------------------------------------
/// Given a GeneticRealFunc this will find a parameter set that maximises fitness,
/// a basic genetic algorithm approach is used, lots of options can be set with 
/// this class. Repeated application is also suported, so a best can be found,
/// saved and then an improvment found and so on until the user decides the 
/// result is good enough. Uses fitness proportional selection.
class EOS_CLASS GeneticReal
{
 public:
  /// &nbsp;
   GeneticReal(data::Random & rand);
   
  /// &nbsp;
   ~GeneticReal();


  /// Returns the function being optimised.
   GeneticRealFunc * Function() const;
  
  /// This sets the function to be optimised. Must be called before anything else is.
  /// The user is responsible for deleting it, after this has finished using it.
  /// Returns its input. Resets the whole system - only thing that can not be changed
  /// mid-run.
   GeneticRealFunc * Function(GeneticRealFunc * func);


  /// Returns the size of the population.
   nat32 Population() const;

  /// Sets the population, defaults to 0 and so must be called,
  /// returns the new population size.
  /// If called mid-run it will fill in new members with random data
  /// or delete the least fit of the current members.
   nat32 Population(nat32 pop);


  /// Returns true if fitness proportional crossover is used, in such a scheme
  /// an organism with twice the fitness of another will have its genes twice
  /// as likelly to be used if they breed, if false then the chance is allways
  /// 50:50. Defaults to true.
   bit FitnessCrossover() const;
   
  /// Sets if fitness proportional crossover is used, see parameter-less version for details.
   bit FitnessCrossover(bit fc);


  /// Returns the probability of any given parameter suffering a mutation and being set to
  /// a random value every time breeding occurs. Defaults to 0.01.
   real32 MutationRate() const;
   
  /// Sets the mutation rate.
   real32 MutationRate(real32 mr);


  /// Returns how many of the fitests organisms from one generation are copied to the next,
  /// so as to avoid reverse evolution. Defaults to 1.
   nat32 MaintainBest() const;
 
  /// Sets if the number of top dogs that get to hang arround each generation.
   nat32 MaintainBest(nat32 mb);


  /// Returns how many organisms in each new generation are randomly generated from scratch
  /// rather than created by breeding. Obviously meaningless for the first generation as
  /// they are all totally random. Defaults to 0.
   nat32 Immigrants() const;

  /// Sets the number of random organisms that spontaneously appear each generation.
   nat32 Immigrants(nat32 imm);



  /// Runs the system for a given number of generations, with progress reporting.
   void Run(nat32 generations,time::Progress * prog);



  /// After Run has been called this returns the organism at the given index.
  /// The organisms are sorted by fitness, so the best will allways be at index 0
  /// and the worst at index Population()-1.
   const math::Vector<real32> & GetOrganism(nat32 index) const;

  /// This compliments Organism(...), returns the fitness of the organism at the given index.
  // Higher fitness is good, negative fitness is not allowed.
   real32 GetFitness(nat32 index) const;


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::alg::GeneticReal";}


 private:
  data::Random & rand;
 
  GeneticRealFunc * func;
  bit fitnessCrossover;
  real32 mutationRate;
  nat32 maintainBest;
  nat32 immigrants;
  
  struct Organism
  {
   bit operator < (const Organism & rhs) const {return fitnessSum < rhs.fitnessSum;}
  
   real32 fitness; // Set negative to indicate unknown.
   real32 fitnessSum; // Sum of fitness so far, not including fitness of current Organism.
   math::Vector<real32> paras;
  };
  
  class OrganismSort : public ds::Sort
  {
   public:
    static bit LessThan(const Organism & lhs, const Organism & rhs)
    {
     return lhs.fitness>rhs.fitness; // Reversed cos we want the largest first.
    }
  };
  
  // Size() is the Population size, maintained sorted with the highest fitness first.
   ds::Array<Organism,mem::MakeNew<Organism>,mem::KillOnlyDel<Organism> > pop;


  // Various internal use functions that do actual work...
   void CalcFitness(time::Progress * prog); // Calculates the fitness & fitnessSum for any organisms with negative fitness, and sorts them.
   void Breed(time::Progress * prog); // Creates a new unsorted and unranked organism set from the current batch.
};

//------------------------------------------------------------------------------
/// This interface is provided to the Genetic class to specialise its behaviour
/// to a particular problem. It defines the organism being optimised, including
/// the creation of random organisms, matting, mutation and calculation of
/// fitness.
class EOS_CLASS GeneticOrg : public Deletable
{
 public:
  /// &nbsp;
   ~GeneticOrg();
   
   
  /// Returns the size of an organism in bytes, so that allocation can be 
  /// handled by the Genetic class.
   virtual nat32 Size() const = 0;
   
  /// This creates a new random organism, putting it into newOrg.
   virtual void New(void * newOrg,data::Random & rand) const = 0;
   
  /// This deletes an organism. Called on all memory blocks prior to them being
  /// deleted or re-used for a New/child.
  /// The default implimentation provided does nothing.
   virtual void Del(void * deadOrg) const;
  
   
  /// This should return the fitness of the organism. For speed the organism 
  /// should cache fitness so it is only calculated once.
  /// Fitness should increase as the organism gets better, negative numbers are
  /// not suported. A fitness of 0 is a definite death sentence.
   virtual real32 Fitness(const void * org) const = 0;
   
  /// This breeds two organisms and outputs a new organism into child.
  /// The implimentor can choose if fitness-based weighting is used.
   virtual void Breed(const void * mother,
                      const void * father,
                      void * child,
                      data::Random & rand) const = 0;
   
  /// This mutates an organism.
   virtual void Mutate(void * org,data::Random & rand) const = 0;


  /// &nbsp;
   virtual cstrconst TypeString() const = 0;
};

//------------------------------------------------------------------------------
/// This is an arbitary genetic algorithm implimentation, where you give it an 
/// interface instance that defines near enough everything.
/// This makes it a reasonably light-weight framework, where more functionality
/// is in the GeneticOrg the user has to impliment than the framework itself, 
/// but it is none the less conveniant for doing a whole host of tedius stuff.
class EOS_CLASS Genetic
{
 public:
  /// &nbsp;
   Genetic(data::Random & rand);

  /// &nbsp;
   ~Genetic();


  /// Sets the GeneticOrg, this defines all the details about the organism and
  /// the function being optimised etc.
  /// You must not delete the given object till after this is done with it.
  /// Returns the org just given.
   const GeneticOrg * Org(GeneticOrg * go);

  /// Returns the GeneticOrg object being used.
   const GeneticOrg * Org() const;

  /// Sets the population size. 
  /// All previous members of the population will die.
  /// Returns the new population size.
  /// Defaults to 0, so it must be called before use.  
   nat32 Population(nat32 newSize);

  /// Returns the size of the population.
   nat32 Population() const;

  /// Sets the number of top dogs that get to hang arround each generation.
  /// Returns the new count.
  /// Defaults to 1.
   nat32 KeptBest(nat32 n);

  /// Returns the number of top dogs that are kept each generation.
   nat32 KeptBest() const;

  /// Sets the number of random organisms that spontaneously appear each generation.
  /// Returns this new number.
  /// Defaults to 0.
   nat32 Immigrants(nat32 imm);

  /// Returns the number of spontaneous organisms.
   nat32 Immigrants() const;

  /// Sets the mutation rate - the percentage chance of a mutation happenning.
  /// Immigrants and best kept can not suffer a mutation.
  /// Returns the new mutation rate.
  /// Defaults to 0.0.
   real32 MutationRate(real32 mr);

  /// Returns the mutation rate.
   real32 MutationRate() const;


  /// Runs the system for a given number of generations, with progress reporting.
  /// Can be called repetedly, so you can decide when fitness is low
  /// enough/unchanging enough to stop.
   void Run(nat32 generations,time::Progress * prog = null<time::Progress*>());


  /// After Run has been called this returns the organism at the given indoex. 
  /// The organisms are sorted by fitness, so the best will allways be at index
  /// 0 and the worst at index Population()-1.
   const void * Organism(nat32 index) const;

  /// Returns the fitness of the organism at a given index.
   real32 Fitness(nat32 index) const;


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::alg::Genetic";}


 private:
  data::Random & rand;

  // Inputs...
   GeneticOrg * go;
   nat32 population;
   nat32 keptBest;
   nat32 immigrants;
   real32 mutationRate;

  // State...
   ds::ArrayRS<byte> data;
   
  // Helper function for sorting by fitness...
   static bit GreaterThan(byte * lhs,byte * rhs,GeneticOrg * go);
};

//------------------------------------------------------------------------------
 };
};
#endif
