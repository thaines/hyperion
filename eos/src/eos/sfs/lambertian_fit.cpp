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

#include "eos/sfs/lambertian_fit.h"

#include "eos/math/iter_min.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
NeedleSeg::NeedleSeg(nat32 segments)
:segs(segments)
{
 for (nat32 i=0;i<segs.Size();i++)
 {
  segs[i].light = 0;
  segs[i].albedo = 0;
  segs[i].samples = null<ds::Array<IrrDir>*>();
 }
}

NeedleSeg::~NeedleSeg()
{}

nat32 NeedleSeg::Segments() const
{
 return segs.Size();
}

void NeedleSeg::Set(nat32 seg,nat32 lightGroup,nat32 albedoGroup,const ds::Array<IrrDir> & samples)
{
 segs[seg].light = lightGroup;
 segs[seg].albedo = albedoGroup;
 segs[seg].samples = &samples;
}

nat32 NeedleSeg::LightGroup(nat32 seg) const
{
 return segs[seg].light;
}

nat32 NeedleSeg::AlbedoGroup(nat32 seg) const
{
 return segs[seg].albedo;
}

const ds::Array<IrrDir> & NeedleSeg::Samples(nat32 seg) const
{
 return *segs[seg].samples;
}

//------------------------------------------------------------------------------
NeedleSegModel::NeedleSegModel(nat32 lightSize,nat32 albedoSize)
:light(lightSize),albedo(albedoSize)
{}

NeedleSegModel::~NeedleSegModel()
{}

void NeedleSegModel::Resize(nat32 lightSize,nat32 albedoSize)
{
 light.Size(lightSize);
 albedo.Size(albedoSize);
}

nat32 NeedleSegModel::Lights() const
{
 return light.Size();
}

bs::Normal & NeedleSegModel::Light(nat32 lightGroup)
{
 return light[lightGroup];
}

const bs::Normal & NeedleSegModel::Light(nat32 lightGroup) const
{
 return light[lightGroup];
}

nat32 NeedleSegModel::Albedos() const
{
 return albedo.Size();
}

real32 & NeedleSegModel::Albedo(nat32 albedoGroup)
{
 return albedo[albedoGroup];
}

const real32 & NeedleSegModel::Albedo(nat32 albedoGroup) const
{
 return albedo[albedoGroup];
}

//------------------------------------------------------------------------------
LambertianRefine::LambertianRefine()
:ns(null<NeedleSeg*>()),nsm(null<NeedleSegModel*>()),
cutoff(math::pi/6.0),halflife(math::pi/36.0)
{}

LambertianRefine::~LambertianRefine()
{}

void LambertianRefine::Set(const NeedleSeg & n)
{
 ns = &n;
}

void LambertianRefine::Set(NeedleSegModel & n)
{
 nsm = &n;
}

void LambertianRefine::Set(real32 c,real32 h)
{
 cutoff = c;
 halflife = h;
}

void LambertianRefine::Run(time::Progress * prog)
{
 LogTime("eos::sfs::LambertianRefine::Run");
 prog->Push();
 prog->Report(0,1);
 
 // Build the parameter vector - first every light orientation as a spherical 
 // coordinate then every albedo...
  math::Vector<real32> pv(nsm->Lights()*2 + nsm->Albedos());
  
  for (nat32 i=0;i<nsm->Lights();i++)
  {
   pv[i*2]   = math::InvCos(nsm->Light(i)[2]/nsm->Light(i).Length());
   pv[i*2+1] = math::InvTan2(nsm->Light(i)[1],nsm->Light(i)[0]);
  }
  
  for (nat32 i=0;i<nsm->Albedos();i++)
  {
   pv[nsm->Lights()*2 + i] = nsm->Albedo(i);
  }


 // Optimise, using LM. The error vector has an entry for every segment - 
 // every pixel would be too much...
  temp.Size(nsm->Lights());
  math::LM(ns->Segments(),pv,*this,Func);


 // Extract the results...
  for (nat32 i=0;i<nsm->Lights();i++)
  {
   nsm->Light(i)[0] = math::Sin(pv[i*2]) * math::Cos(pv[i*2+1]);
   nsm->Light(i)[1] = math::Sin(pv[i*2]) * math::Sin(pv[i*2+1]);
   nsm->Light(i)[2] = math::Cos(pv[i*2]);
  }
  
  for (nat32 i=0;i<nsm->Albedos();i++)
  {
   nsm->Albedo(i) = pv[nsm->Lights()*2 + i];
  }
 
 prog->Pop();
}

void LambertianRefine::Func(const math::Vector<real32> & pv,
                            math::Vector<real32> & err,
                            const LambertianRefine & self)
{
 // Extract the light source directions from the parameter vector...
  for (nat32 i=0;i<self.temp.Size();i++)
  {
   self.temp[i][0] = math::Sin(pv[i*2]) * math::Cos(pv[i*2+1]);
   self.temp[i][1] = math::Sin(pv[i*2]) * math::Sin(pv[i*2+1]);
   self.temp[i][2] = math::Cos(pv[i*2]);
  }


 // Iterate the segments, sum and store the error for each segment...
  for (nat32 i=0;i<err.Size();i++)
  {
   err[i] = 0.0;
   
   const ds::Array<IrrDir> & sa = self.ns->Samples(i);
   bs::Normal & l = self.temp[self.ns->LightGroup(i)];
   real32 a = pv[self.temp.Size()*2 + self.ns->AlbedoGroup(i)];
   
   for (nat32 j=0;j<sa.Size();j++)
   {
    real32 guess = a*math::Max((sa[j].dir * l),real32(0.0));
    real32 e = math::Abs(guess - sa[j].irr);
    err[i] += sa[j].weight * (1.0 - math::SigmoidCutoff(e,self.cutoff,self.halflife));
   }
  }
}

//------------------------------------------------------------------------------
LambertianEA::LambertianEA()
:population(100),keptBest(5),immigrants(5),mutationRate(0.1),
generations(250),albSd(1.0),lightSd(0.1),cutoff(math::pi/6.0),
ns(null<NeedleSeg*>()),nsm(null<NeedleSegModel*>())
{}

LambertianEA::~LambertianEA()
{}

void LambertianEA::Set(const NeedleSeg & n)
{
 ns = &n;
}

void LambertianEA::Set(NeedleSegModel & n)
{
 nsm = &n;
 lightCounter.Size(nsm->Lights());
 albedoCounter.Size(nsm->Albedos());
}

void LambertianEA::Set(nat32 p,nat32 k,nat32 i,real32 m,nat32 g,real32 a,real32 l,real32 c)
{
 population = p;
 keptBest = k;
 immigrants = i;
 mutationRate = m;
 generations = g;
 albSd = a;
 lightSd = l;
 cutoff = c;
}

void LambertianEA::Run(time::Progress * prog)
{
 LogTime("eos::sfs::LambertianEA::Run");

 // Create the genetic algorithm object... 
  alg::Genetic gen(rand);
  gen.Org(this);
  gen.Population(population);
  gen.KeptBest(keptBest);
  gen.Immigrants(immigrants);
  gen.MutationRate(mutationRate);


 // Run it...
  gen.Run(generations,prog);


 // Extract the best result...
  Organism * best = (Organism*)gen.Organism(0);
  
  for (nat32 i=0;i<nsm->Lights();i++)
  {
   nsm->Light(i)[0] = math::Sin(best->pv[i*2]) * math::Cos(best->pv[i*2+1]);
   nsm->Light(i)[1] = math::Sin(best->pv[i*2]) * math::Sin(best->pv[i*2+1]);
   nsm->Light(i)[2] = math::Cos(best->pv[i*2]);
  }
  
  for (nat32 i=0;i<nsm->Albedos();i++)
  {
   nsm->Albedo(i) = best->pv[nsm->Lights()*2 + i];
  }
}

nat32 LambertianEA::Size() const
{
 return sizeof(Organism) - sizeof(real32) + sizeof(real32)*(nsm->Lights()*2 + nsm->Albedos());
}

void LambertianEA::New(void * newOrgV,data::Random & rand) const
{
 Organism * newOrg = (Organism*)newOrgV;

 newOrg->fitness = -1.0;
 
 // Zero out the counters ready for below...
  for (nat32 i=0;i<lightCounter.Size();i++) lightCounter[i] = 0;
  for (nat32 i=0;i<albedoCounter.Size();i++) albedoCounter[i] = 0;


 // Iterate every segment and randomly select 3 samples, use linear methods to 
 // then calculate an albedo and light direction. We then need to decide if we 
 // store it or not - we need to have an even chance of each light/albedo comming
 // from all segments that are applicable...
  for (nat32 i=0;i<ns->Segments();i++)
  {
   // Check if we are going to use either albedo/light from this segment...
    lightCounter[ns->LightGroup(i)] += 1;
    albedoCounter[ns->AlbedoGroup(i)] += 1;

    bit useLight  = rand.Event(1.0/real32(lightCounter[ns->LightGroup(i)]));
    bit useAlbedo = rand.Event(1.0/real32(albedoCounter[ns->AlbedoGroup(i)]));

  
   // If theres any point calculate the albedo and light...
    if (useLight||useAlbedo)
    {     
     // Randomly select 3 samples...
      const ds::Array<IrrDir> & sa = ns->Samples(i);
      nat32 ind[3];
      for (nat32 j=0;j<3;j++) ind[j] = rand.Int(0,sa.Size()-1);


     // Calculate the relevant relationship vector...
      math::Mat<3,3> a;
      math::Vect<3> xb;
      
      for (nat32 j=0;j<3;j++)
      {
       for (nat32 k=0;k<3;k++) a[j][k] = sa[ind[j]].dir[k];
       xb[j] = sa[ind[j]].irr;
      }
      
      math::SolveLinear(a,xb);
      if (math::IsZero(xb.LengthSqr()))
      {
       xb[2] = 1.0;
      }


     // Extract the light direction/albedo as required...
      real32 xbLen = xb.Length();

      if (useLight)
      {
       newOrg->pv[ns->LightGroup(i)*2]   = math::InvCos(xb[2]/xbLen);
       newOrg->pv[ns->LightGroup(i)*2+1] = math::InvTan2(xb[1],xb[0]);
      }
      
      if (useAlbedo)
      {
       newOrg->pv[nsm->Lights()*2 + ns->AlbedoGroup(i)] = xbLen;
      }   
    }
  }
}

real32 LambertianEA::Fitness(const void * orgV) const
{
 const Organism * org = (const Organism*)orgV;

 if (org->fitness<0.0) CalcFitness(org);
 return org->fitness;
}

void LambertianEA::Breed(const void * motherV,const void * fatherV,
                         void * childV,data::Random & rand) const
{
 const Organism * mother = (const Organism*)motherV;
 const Organism * father = (const Organism*)fatherV;
 Organism * child = (Organism*)childV;

 // Calculate the fitness if needed, work out the probability of a fathers gene 
 // being used...
  if (mother->fitness<0.0) CalcFitness(mother);
  if (father->fitness<0.0) CalcFitness(father);
 
  real32 fatherProb = father->fitness/(mother->fitness + father->fitness);


 // Copy in the mothers genes with a memory copy...
  child->fitness = -1.0;
  mem::Copy(child->pv,mother->pv,nsm->Lights()*2 + nsm->Albedos());


 // Copy over the fathers genes, when a random test is passed...
  for (nat32 i=0;i<nsm->Lights();i++)
  {
   if (rand.Event(fatherProb))
   {
    child->pv[i*2]   = father->pv[i*2];
    child->pv[i*2+1] = father->pv[i*2+1];
   }
  }
  
  for (nat32 i=0;i<nsm->Albedos();i++)
  {
   if (rand.Event(fatherProb))
   {
    child->pv[nsm->Lights()*2 + i] = father->pv[nsm->Lights()*2 + i];
   }   
  }
}

void LambertianEA::Mutate(void * orgV,data::Random & rand) const
{
 Organism * org = (Organism*)orgV;

 org->fitness = -1.0;

 // The lights...
  for (nat32 i=0;i<nsm->Lights()*2;i++)
  {
   org->pv[i] += rand.Gaussian(lightSd);
  }


 // The albedos...
  for (nat32 i=0;i<nsm->Albedos();i++)
  {
   org->pv[nsm->Lights()*2 + i] += rand.Gaussian(albSd);
   if (org->pv[nsm->Lights()*2 + i]<0.01) org->pv[nsm->Lights()*2 + i] = 0.01;
  }
}

void LambertianEA::CalcFitness(const Organism * org) const
{
 org->fitness = 0.0;

 for (nat32 i=0;i<ns->Segments();i++)
 {
  // Calculate the light direction for the segment...
   bs::Normal l;
   l[0] = math::Sin(org->pv[ns->LightGroup(i)*2]) * math::Cos(org->pv[ns->LightGroup(i)*2+1]);
   l[1] = math::Sin(org->pv[ns->LightGroup(i)*2]) * math::Sin(org->pv[ns->LightGroup(i)*2+1]);
   l[2] = math::Cos(org->pv[ns->LightGroup(i)*2]);


  // Obtain the albedo for the segment...
   real32 a = org->pv[nsm->Lights()*2 + ns->AlbedoGroup(i)];


  // Iterate all members of the segment and sum up there fitnesses...
   const ds::Array<IrrDir> & sa = ns->Samples(i);
   for (nat32 j=0;j<sa.Size();j++)
   {
    real32 guess = a * math::Max((sa[j].dir * l),real32(0.0));
    real32 e = math::Abs(guess - sa[j].irr);
    org->fitness += math::Max(sa[j].weight * (cutoff - e),real32(0.0));
   }
 }
}

//------------------------------------------------------------------------------
LambertianRANSAC::LambertianRANSAC()
:cutoff(math::pi/6.0),chance(0.99),limit(250.0),cap(10000),
ns(null<NeedleSeg*>()),nsm(null<NeedleSegModel*>())
{}

LambertianRANSAC::~LambertianRANSAC()
{}

void LambertianRANSAC::Set(const NeedleSeg & n)
{
 ns = &n;
}

void LambertianRANSAC::Set(NeedleSegModel & n)
{
 nsm = &n;
}

void LambertianRANSAC::Set(real32 cu,real32 ch,real32 l,nat32 ca)
{
 cutoff = cu;
 chance = ch;
 limit = l;
 cap = ca;
}

void LambertianRANSAC::Run(time::Progress * prog)
{
 LogTime("eos::sfs::LambertianRANSAC::Run");
 prog->Push();
 
 // We are essentially optimising multiple things simultaneously here, this
 // isn't really compatable with RANSAC. So we randomly generate an entire model,
 // then for each randomly generated model we do ransac on each seperable 
 // component of the model, until all components have passed the ransac ending
 // condition. This does mean that some components will be checked an unnecesary 
 // number of times, but blah. Generating the model involves randomly choosing
 // parameters from each segment and fitting a model to just that segment.
 // The model is then split into lighting and material information. For each 
 // light one of the multiple lights generated this way is then selected with
 // even chance. Same again for the albedo.
 
 // Build data structure to store the best model and its scores...
  // Albedos...
   ds::Array<real32> bestAlbedo(nsm->Albedos());
   ds::Array<real32> bestAlbedoScore(nsm->Albedos());
  
  // Lights...
   ds::Array<bs::Normal> bestLight(nsm->Lights());
       ds::Array<real32> bestLightScore(nsm->Lights());
       
  // Fill in...
   for (nat32 i=0;i<nsm->Albedos();i++)
   {
    bestAlbedo[i] = 0.0;
    bestAlbedoScore[i] = 0.0;
   }
   
   for (nat32 i=0;i<nsm->Lights();i++)
   {
    bestLight[i] = bs::Normal(0.0,0.0,0.0);
    bestLightScore[i] = 0.0;
   }



 // Top scores for each albedo/light...
  ds::Array<real32> topAlbedoScore(nsm->Albedos());
  ds::Array<real32> topLightScore(nsm->Lights());

  for (nat32 i=0;i<topAlbedoScore.Size();i++) topAlbedoScore[i] = 0.0;
  for (nat32 i=0;i<topLightScore.Size();i++) topLightScore[i] = 0.0;

  for (nat32 i=0;i<ns->Segments();i++)
  {
   const ds::Array<IrrDir> & data = ns->Samples(i);
   real32 size = 0.0;
   for (nat32 j=0;j<data.Size();j++) size += data[j].weight;
   
   topAlbedoScore[ns->AlbedoGroup(i)] += size;
   topLightScore[ns->LightGroup(i)] += size;
  }



 // Same again, for currently generated model. Needs extra suport structures...
  // Albeos...
   ds::Array<real32> currAlbedo(nsm->Albedos());
    ds::Array<real32> currAlbedoScore(nsm->Albedos());
    ds::Array<nat32> currAlbedoSegs(nsm->Albedos()); // Number of segs used, for random selection of seg.

  // Lights...
   ds::Array<bs::Normal> currLight(nsm->Lights());
        ds::Array<real32> currLightScore(nsm->Lights());
        ds::Array<nat32> currLightSegs(nsm->Albedos()); // Number of segs used, for random selection.



 // Iterate until all components say stop (There is a cap, just incase)...
  for (nat32 i=0;i<cap;i++)
  {
   // Generate a completly random model...
    // Zero out the seg counters...
     for (nat32 j=0;j<currAlbedoSegs.Size();j++) currAlbedoSegs[j] = 0;
     for (nat32 j=0;j<currLightSegs.Size();j++) currLightSegs[j] = 0;
    
    // Iterate all segments, for each segment calculate a random model, 
    // then split it into light and albedo, before storing it, incrimentally,
    // such that a random selection from avaliable segments is made for
    // each light/albedo...
     for (nat32 j=0;j<ns->Segments();j++)
     {
      // Find out if we are storing either light/albedo from this...
       nat32 albedoGroup = ns->AlbedoGroup(j);
       nat32 lightGroup = ns->LightGroup(j);
       
       currAlbedoSegs[albedoGroup] += 1;
       currLightSegs[lightGroup] += 1;
       
       bit storeAlbedo = rand.Event(1.0/real64(currAlbedoSegs[albedoGroup]));
       bit storeLight = rand.Event(1.0/real64(currLightSegs[lightGroup]));
       if ((storeAlbedo==false)&&(storeLight==false)) continue;


      // Randomly select 3 components within the segment...
      // (Without repetition - not hard to do as only 3 samples needed.)
      // (Insane answers get zeroed.)
       const ds::Array<IrrDir> & data = ns->Samples(j);
      
       // Select 3 samples...
        nat32 samp[3];
        for (nat32 k=0;k<3;k++)
        {
         samp[k] = rand.Int(0,data.Size()-1-k);
         for (nat32 l=0;l<k;l++)
         {
          if (samp[k]>=samp[l]) samp[k] += 1;
         }
        }


       // Fit a model to the samples found - a simple linear method...
        math::Mat<3,3> a;
        bs::Normal mod;
        for (nat32 k=0;k<3;k++)
        {
         for (nat32 l=0;l<3;l++) a[k][l] = data[samp[k]].dir[l];
         mod[k] = data[samp[k]].irr;
        
          real32 dist = math::Sqrt(math::Sqr(a[k][0]) + 
                                   math::Sqr(a[k][1]) + 
                                   math::Sqr(a[k][2]) + 
                                   math::Sqr(mod[k]));
          real32 mult = data[samp[k]].weight/dist;
          for (nat32 l=0;l<3;l++) a[k][l] *= mult;
          mod[k] *= mult;
         }
                
         math::SolveLinear(a,mod);

       
        // Check that all is numerically stable...
         real32 albedo = mod.Length();
         if (!math::IsFinite(albedo)) {mod = bs::Normal(0.0,0.0,0.0); albedo = 0.0;}
         if (albedo>limit) {mod = bs::Normal(0.0,0.0,0.0); albedo = 0.0;}



      // Store the relevant parts of the model...
       if (storeAlbedo) currAlbedo[albedoGroup] = albedo;
       
       if (storeLight)
       {
        currLight[lightGroup] = mod;
        if (!math::IsZero(albedo)) currLight[lightGroup] /= albedo;
       }
     }



   // Zero each score...
    for (nat32 j=0;j<currAlbedoScore.Size();j++) currAlbedoScore[j] = 0.0;
    for (nat32 j=0;j<currLightScore.Size();j++) currLightScore[j] = 0.0;



   // Score each segment,...
    for (nat32 j=0;j<ns->Segments();j++)
    {
     // Get the segments assigned model...
      nat32 albedoGroup = ns->AlbedoGroup(j);
      nat32 lightGroup = ns->LightGroup(j);
       
      bs::Normal light = currLight[lightGroup];
      real32 albedo    = currAlbedo[albedoGroup];


     // Measure the error of every contained sample, count those within tolerance...
      real32 score = 0;
      const ds::Array<IrrDir> & sa = ns->Samples(j);
      for (nat32 k=0;k<sa.Size();k++)
      {
       real32 guess = albedo * (sa[k].dir * light);
       real32 e = math::Abs(guess - sa[k].irr);

       if (e<cutoff) score += sa[k].weight;
      }


     // Add to relevant score totals...
      currAlbedoScore[albedoGroup] += score;
      currLightScore[lightGroup] += score;
    }



   // Grab any better parts of the model and transfer into best...
    // Albedo...
     for (nat32 j=0;j<currAlbedo.Size();j++)
     {
      if (currAlbedoScore[j]>bestAlbedoScore[j])
      {
       bestAlbedo[j] = currAlbedo[j];
       bestAlbedoScore[j] = currAlbedoScore[j];
      }
     }

    // Lights...
     for (nat32 j=0;j<currLight.Size();j++)
     {
      if (currLightScore[j]>bestLightScore[j])
      {
       bestLight[j] = currLight[j];
       bestLightScore[j] = currLightScore[j];
      }
     }



   // Break if all models have enough random samples, update progress bar...
    real32 percentDone = 1.0;
    bit done = true;
  
    for (nat32 j=0;j<bestAlbedo.Size();j++)
    {
     if (!math::IsZero(bestAlbedoScore[j]))
     {
      real32 ip = bestAlbedoScore[j]/topAlbedoScore[j];
      real32 samplesNeeded = math::Ln(1.0-chance)/math::Ln(1.0-math::Pow(ip,real32(3.0)));
      if (samplesNeeded>real32(i))
      {
       done = false;
       percentDone *= real32(i)/samplesNeeded;
      }
     }
     else
     {
      done = false;
      percentDone = 0.0;
     }     
    }
    
    for (nat32 j=0;j<bestLight.Size();j++)
    {
     if (!math::IsZero(bestLightScore[j]))
     {
      real32 ip = bestLightScore[j]/topLightScore[j];
      real32 samplesNeeded = math::Ln(1.0-chance)/math::Ln(1.0-math::Pow(ip,real32(3.0)));
      if (samplesNeeded>real32(i))
      {
       done = false;
       percentDone *= real32(i)/samplesNeeded;
      }
     }
     else
     {
      done = false;
      percentDone = 0.0;
     }
    }
    
    if (done) break;
    percentDone = math::Max(percentDone,real32(i)/real32(cap));
    prog->Report(nat32(percentDone*1000.0),1000);
  }



 // Write the best model into the output...
  for (nat32 i=0;i<bestAlbedo.Size();i++) nsm->Albedo(i) = bestAlbedo[i];
  for (nat32 i=0;i<bestLight.Size();i++) nsm->Light(i) = bestLight[i];


 prog->Pop();
}

//------------------------------------------------------------------------------
LambertianFit::LambertianFit()
:mode(false),refine(true),
population(100),keptBest(5),immigrants(5),mutationRate(0.1),generations(250),albSd(1.0),lightSd(0.1),
chance(0.99),limit(250.0),cap(10000),
cutoff(math::pi/6.0),halflife(math::pi/36.0),
ns(null<NeedleSeg*>()),nsm(null<NeedleSegModel*>())
{}

LambertianFit::~LambertianFit()
{}

void LambertianFit::Set(const NeedleSeg & n)
{
 ns = &n;
}

void LambertianFit::Set(NeedleSegModel & n)
{
 nsm = &n;
}

void LambertianFit::SetGA(nat32 p,nat32 k,nat32 i,real32 m,nat32 g,real32 a,real32 l,real32 c,real32 h)
{
 mode = false;

 population = p;
 keptBest = k;
 immigrants = i;
 mutationRate = m;
 generations = g;
 albSd = a;
 lightSd = l;
 cutoff = c;
 halflife = h;
}

void LambertianFit::SetRansac(real32 ch,real32 l,nat32 ca,real32 cu,real32 h)
{
 mode = true;

 chance = ch;
 limit = l;
 cap = ca;
 cutoff = cu;
 halflife = h;
}

void LambertianFit::Refine(bit r)
{
 refine = r;
}

void LambertianFit::Run(time::Progress * prog)
{
 LogTime("eos::sfs::LambertianFit::Run");
 prog->Push();

  prog->Report(0,2);
  if (mode)
  {
   // LambertianRansac...
    LambertianRANSAC lra;
    lra.Set(*ns);
    lra.Set(*nsm);
    lra.Set(cutoff,chance,limit,cap);
    lra.Run(prog);
  }
  else
  {
   // LambertianEA...
    LambertianEA lea;
    lea.Set(*ns);
    lea.Set(*nsm);
    lea.Set(population,keptBest,immigrants,mutationRate,generations,albSd,lightSd,cutoff);
    lea.Run(prog);
  }


  // LambertianRefine...
   prog->Report(1,2);
   if (refine)
   {
    LambertianRefine lr;
    lr.Set(*ns);
    lr.Set(*nsm);
    lr.Set(cutoff,halflife);
    lr.Run(prog);
   }
 
 prog->Pop();
}

//------------------------------------------------------------------------------
 };
};
