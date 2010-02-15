//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/alg/genetic.h"

#include "eos/math/functions.h"

namespace eos
{
 namespace alg
 {
//------------------------------------------------------------------------------
GeneticReal::GeneticReal(data::Random & r)
:rand(r),func(null<GeneticRealFunc*>()),
fitnessCrossover(true),mutationRate(0.01),maintainBest(1),immigrants(0)
{}

GeneticReal::~GeneticReal()
{}

GeneticRealFunc * GeneticReal::Function() const
{
 return func;
}

GeneticRealFunc * GeneticReal::Function(GeneticRealFunc * f)
{
 func = f;
 pop.Size(0);
 return func;
}

nat32 GeneticReal::Population() const
{
 return pop.Size();
}

nat32 GeneticReal::Population(nat32 p)
{
 nat32 baseSize = pop.Size();
 pop.Size(p);
  for (nat32 i=baseSize;i<pop.Size();i++)
  {
   pop[i].fitness = -1.0;
   pop[i].paras.SetSize(func->ParaCount());
   for (nat32 j=0;j<func->ParaCount();j++)
   {
    math::Range<real32> range;
    func->ParaRange(j,range);
    pop[i].paras[j] = rand.Sample(range);
   }
  }
 return pop.Size();
}

bit GeneticReal::FitnessCrossover() const
{
 return fitnessCrossover;
}

bit GeneticReal::FitnessCrossover(bit fc)
{
 fitnessCrossover = fc;
 return fitnessCrossover;
}

real32 GeneticReal::MutationRate() const
{
 return mutationRate;
}

real32 GeneticReal::MutationRate(real32 mr)
{
 mutationRate = mr;
 return mutationRate;
}

nat32 GeneticReal::MaintainBest() const
{
 return maintainBest;
}

nat32 GeneticReal::MaintainBest(nat32 mb)
{
 maintainBest = mb;
 return maintainBest;
}

nat32 GeneticReal::Immigrants() const
{
 return immigrants;
}

nat32 GeneticReal::Immigrants(nat32 imm)
{
 immigrants = imm;
 return immigrants;
}

void GeneticReal::Run(nat32 generations,time::Progress * prog)
{
 prog->Push();
 prog->Report(0,generations*2+1);
 CalcFitness(prog);

  for (nat32 i=0;i<generations;i++)
  {
   prog->Report(1+i*2,generations*2+1);
   Breed(prog);  
  
   prog->Report(1+i*2+1,generations*2+1);  
   CalcFitness(prog);  
  }
 
 prog->Pop();
}

const math::Vector<real32> & GeneticReal::GetOrganism(nat32 index) const
{
 return pop[index].paras;
}

real32 GeneticReal::GetFitness(nat32 index) const
{
 return pop[index].fitness;
}

void GeneticReal::CalcFitness(time::Progress * prog)
{
 prog->Push();

 // First calcualte fitnesss for all uncalculated souls...
  bit change = false;
  for (nat32 i=0;i<pop.Size();i++)
  {
   prog->Report(i,pop.Size()+1);
   if (pop[i].fitness<0.0)
   {
    change = true;
    pop[i].fitness = func->Fitness(pop[i].paras,prog);
   }
  }
  prog->Report(pop.Size(),pop.Size()+1);
 
 // If a change has happened sort the entire population then update the fitnessSum's...
  if (change)
  {
   pop.Sort<OrganismSort>();
   
   pop[0].fitnessSum = 0.0;
   for (nat32 i=1;i<pop.Size();i++) pop[i].fitnessSum = pop[i-1].fitnessSum + pop[i-1].fitness;
  }

 prog->Pop();
}

void GeneticReal::Breed(time::Progress * prog)
{
 prog->Push();

 // Create a copy of all the parameters...
  prog->Report(0,pop.Size()-maintainBest+1);
  ds::Array<Organism,mem::MakeNew<Organism>,mem::KillOnlyDel<Organism> > popDup;
  popDup.Size(pop.Size());
  for (nat32 i=0;i<popDup.Size();i++)
  {
   popDup[i].fitness = pop[i].fitness;
   popDup[i].fitnessSum = pop[i].fitnessSum;
   popDup[i].paras = pop[i].paras;
  }

 // Create the immigrants just after the maintained best...
  for (nat32 i=maintainBest;i<maintainBest+immigrants;i++)
  {
   prog->Report(i-maintainBest+1,pop.Size()-maintainBest+1);
   for (nat32 j=0;j<func->ParaCount();j++)
   {
    math::Range<real32> range;
    func->ParaRange(j,range);
    pop[i].paras[j] = rand.Sample(range);    
   }
   pop[i].fitness = -1.0;
  }

 // Replace every organism following the best and the immigrants with a newly
 // bred...
  for (nat32 i=maintainBest+immigrants;i<pop.Size();i++)
  {
   prog->Report(i-maintainBest+1,pop.Size()-maintainBest+1);
   // Find a pair of breeding partners, making sure that no self-breeding
   // occurs, use a binary search to find the relevent index each time...
    // First partner...
     real32 fitnessTotal = popDup[popDup.Size()-1].fitnessSum + popDup[popDup.Size()-1].fitness;    
     Organism dummy;   
     dummy.fitnessSum = rand.Normal()*fitnessTotal;     
     nat32 p1 = nat32(popDup.SearchLargest< ds::SortOp<Organism> >(dummy));
    
    // Second partner, selected from the remaining organisms...
     dummy.fitnessSum = rand.Normal()*(fitnessTotal - popDup[p1].fitness);
     if (dummy.fitnessSum>=popDup[p1].fitnessSum) dummy.fitnessSum += popDup[p1].fitness;
     nat32 p2 = nat32(popDup.SearchLargest< ds::SortOp<Organism> >(dummy));


   // Now breed them to create the new organism...
    if (fitnessCrossover)
    {
     for (nat32 j=0;j<func->ParaCount();j++)
     {
      if (rand.Weighted(popDup[p1].fitness,popDup[p2].fitness)) pop[i].paras[j] = popDup[p1].paras[j];
                                                           else pop[i].paras[j] = popDup[p2].paras[j];
     }
    }
    else
    {
     for (nat32 j=0;j<func->ParaCount();j++)
     {
      if (rand.Bool()) pop[i].paras[j] = popDup[p1].paras[j];
                  else pop[i].paras[j] = popDup[p2].paras[j];
     }    
    }
    
    pop[i].fitness = -1.0;


   // And finally dip the new organism into a vat of glowing green liquid...
    for (nat32 j=0;j<func->ParaCount();j++)
    {
     if (rand.Event(mutationRate))
     {
      math::Range<real32> range;
      func->ParaRange(j,range);
      pop[i].paras[j] = rand.Sample(range);
     }
    }
  }
  
 prog->Pop();
}

//------------------------------------------------------------------------------
GeneticOrg::~GeneticOrg()
{}

void GeneticOrg::Del(void *) const
{}

//------------------------------------------------------------------------------
Genetic::Genetic(data::Random & r)
:rand(r),go(null<GeneticOrg*>()),
population(0),keptBest(1),immigrants(0),mutationRate(0.0),
data(1)
{}

Genetic::~Genetic()
{
 for (nat32 i=0;i<data.Size();i++) go->Del(&(data[i]));
}

const GeneticOrg * Genetic::Org(GeneticOrg * g)
{
 go = g;
 return go;
}

const GeneticOrg * Genetic::Org() const
{
 return go;
}

nat32 Genetic::Population(nat32 newSize)
{
 for (nat32 i=0;i<data.Size();i++) go->Del(&(data[i]));
 data.Size(0);

 population = newSize; 
 return population;
}

nat32 Genetic::Population() const
{
 return population;
}

nat32 Genetic::KeptBest(nat32 n)
{
 keptBest = n;
 return keptBest;
}

nat32 Genetic::KeptBest() const
{
 return keptBest;
}

nat32 Genetic::Immigrants(nat32 imm)
{
 immigrants = imm;
 return immigrants;
}

nat32 Genetic::Immigrants() const
{
 return immigrants;
}

real32 Genetic::MutationRate(real32 mr)
{
 mutationRate = mr;
 return mutationRate;
}

real32 Genetic::MutationRate() const
{
 return mutationRate;
}

void Genetic::Run(nat32 generations,time::Progress * prog)
{
 prog->Push();
 
 // Progress bar fluff...
  nat32 step = 0;
  nat32 steps = generations;


 // If needed create a starting population...
  if (population!=data.Size())
  {
   prog->Report(step++,++steps);
   data.Rebuild(go->Size(),population);
   for (nat32 i=0;i<data.Size();i++) go->New(&(data[i]),rand);
  }


 // Create tempory storage for new children, and storage for the incrimental
 // fitness...
  ds::ArrayRS<byte> nursery(go->Size(),population-keptBest-immigrants);
  ds::Array<real32> fitnessSum(population);


 // Iterate the generations and breed 'em...
  for (nat32 i=0;i<generations;i++)
  {
   prog->Report(step++,steps);
   prog->Push();
   
    // First sort the population by fitness...
     prog->Report(0,4);
     data.Sort(&GreaterThan,go);


    // Now breed the population to create a new set, weight breeding probability
    // by fitness...
     prog->Report(1,4);
     // Fill in the fitness sum array...
      fitnessSum[0] = 0.0;
      for (nat32 j=0;j<data.Size()-1;j++)
      {
       real32 f = go->Fitness(&(data[j]));
       if (!math::IsFinite(f)) f = 0.0;
       fitnessSum[j+1] = fitnessSum[j] + f;
      }
      real32 fitnessTotal = go->Fitness(&(data[data.Size()-1]));
      if (!math::IsFinite(fitnessTotal)) fitnessTotal = 0.0; 
      fitnessTotal += fitnessSum[data.Size()-1];

     // Iterate all the places in the nursery...
      for (nat32 j=0;j<nursery.Size();j++)
      {
       // Randomly select 2 parents, using the fitness sum array to weight
       // accordingly...
        int32 mother = fitnessSum.SearchLargestNorm(rand.Normal()*fitnessTotal);
        int32 father = fitnessSum.SearchLargestNorm(rand.Normal()*fitnessTotal);
        
       // Breed them...
        go->Breed(&(data[mother]),&(data[father]),&(nursery[j]),rand);
      }


    // Randomly paint the children with glowing green goo...
     prog->Report(2,4);
     for (nat32 j=0;j<nursery.Size();j++)
     {
      if (rand.Event(mutationRate)) go->Mutate(&(nursery[j]),rand);
     }


    // Fiddle arround to create a new population with the old members dead,
    // let in immigrants...
     prog->Report(3,4);
     for (nat32 j=0;j<nursery.Size();j++)
     {
      nat32 ind = j+keptBest;
      go->Del(&(data[ind]));
      mem::Copy(&(data[ind]),&(nursery[j]),go->Size());
     }

     for (nat32 j=0;j<immigrants;j++)
     {
      nat32 ind = data.Size() - 1 - j;
      go->Del(&(data[ind]));
      go->New(&(data[ind]),rand);
     }
   
   prog->Pop();
  }

 prog->Pop();
}

const void * Genetic::Organism(nat32 index) const
{
 return &(data[index]);
}

real32 Genetic::Fitness(nat32 index) const
{
 real32 ret = go->Fitness(&(data[index]));
 if (math::IsFinite(ret)) return ret;
                     else return 0.0;
}

bit Genetic::GreaterThan(byte * lhs,byte * rhs,GeneticOrg * go)
{
 real32 lF = go->Fitness(lhs);
 real32 rF = go->Fitness(rhs);
 
 if (!math::IsFinite(lF)) lF = 0.0;
 if (!math::IsFinite(rF)) rF = 0.0;
 
 return lF > rF;
}

//------------------------------------------------------------------------------
 };
};
