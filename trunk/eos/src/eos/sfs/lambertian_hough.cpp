//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/sfs/lambertian_hough.h"

#include "eos/alg/shapes.h"
#include "eos/math/gaussian_mix.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
LamHough::LamHough()
:subDivs(7),minAlb(0.0),maxAlb(200.0),albDivs(200),
albSd(2.0),cutoff(2.0),
bestCap(3),maxErr(2.0)
{}

LamHough::~LamHough()
{}

void LamHough::Set(nat32 sd,real32 minA,real32 maxA,nat32 aD)
{
 subDivs = sd;
 minAlb = minA;
 maxAlb = maxA;
 albDivs = aD;
}

void LamHough::Set(real32 albS,real32 c)
{
 albSd = albS;
 cutoff = c;
}

void LamHough::Set(nat32 bc,real32 me)
{
 bestCap = bc;
 maxErr = me;
}

void LamHough::Add(real32 irr,const bs::Normal & dir,real32 weight,nat32 segment)
{
 Sample s;
 s.irr = irr;
 s.dir = dir;
 s.weight = weight;
 s.segment = segment;
 
 in.AddBack(s);
}

void LamHough::Add(const svt::Field<real32> & irr,const svt::Field<bs::Normal> & norm,
                   const svt::Field<real32> & weight,const svt::Field<nat32> & segment,
                   const svt::Field<bit> & mask)
{
 for (nat32 y=0;y<irr.Size(1);y++)
 {
  for (nat32 x=0;x<irr.Size(0);x++)
  {
   if ((!mask.Valid())||(mask.Get(x,y)))
   {
    Sample s;
    s.irr = irr.Get(x,y);
    s.dir = norm.Get(x,y);
    s.weight = weight.Valid()?weight.Get(x,y):1.0;
    s.segment = segment.Valid()?segment.Get(x,y):0;

    in.AddBack(s);
   }
  }
 }
}

void LamHough::Run(time::Progress * prog)
{
 LogBlock("eos::sfs::LamHough::Run","");
 prog->Push();

 // First create the relevant icosphere...
  prog->Report(0,6);
  alg::Icosphere ico(subDivs);


 // Build and zero out the accumilator...
  ds::Array2D<real32> acc(albDivs,ico.Verts());
  for (nat32 y=0;y<acc.Height();y++)
  {
   for (nat32 x=0;x<acc.Width();x++) acc.Get(x,y) = 0.0;
  }


 // Calculate some stuff to save time later...
  ds::Array<real32> albedo(albDivs);
  for (nat32 i=0;i<albedo.Size();i++)
  {
   albedo[i] = minAlb + (maxAlb-minAlb)*(real32(i)/real32(albedo.Size()-1));
  }
  
  ds::Array<bs::Normal> dir(ico.Verts());
  for (nat32 i=0;i<dir.Size();i++)
  {
   dir[i][0] = ico.Vert(i)[0];
   dir[i][1] = ico.Vert(i)[1];
   dir[i][2] = ico.Vert(i)[2];
  }

  int32 sdSize = int32(math::RoundUp(cutoff * (maxAlb-minAlb)/(real32(albedo.Size()) * albSd)));
  real32 albMult = (albedo.Size()-1.0)/(maxAlb-minAlb);


 // Create an array out of the samples, for speed of access...
  ds::Array<Sample> data(in.Size());
  {
   ds::List<Sample>::Cursor targ = in.FrontPtr();
   for (nat32 i=0;i<data.Size();i++)
   {
    data[i] = *targ;
    ++targ;
   }
  }
 
 
 // Iterate all samples, sum them into the accumilator...
  prog->Report(1,6);
  prog->Push();
  for (nat32 i=0;i<data.Size();i++)
  {
   LogTime("eos::sfs::LamHough::Run sum");
   prog->Report(i,data.Size());
   for (nat32 y=0;y<acc.Height();y++)
   {
    // Calculate the albedo for this direction...
     real32 a = data[i].irr/(dir[y] * data[i].dir);
    
    // If the albedo is within the range collected continue...
     if (math::IsFinite(a)&&(a>=minAlb)&&(a<=maxAlb))
     {
      // Iterate the range given by the standard deviation, calculating the 
      // amount to add and summing it in...
       int32 centre = int32(math::Round(albMult*(a-minAlb)));
       int32 startX = math::Max(centre-sdSize,int32(0));
       int32 endX   = math::Min(centre+sdSize,int32(acc.Width()-1));
       for (int32 x=startX;x<=endX;x++)
       {
        acc.Get(x,y) += data[i].weight * math::UnNormGaussian(albSd,albedo[x]-a);
       }
     }
   }
  }
  prog->Pop();


 // Find all maximas, requires building a suport data structure for speed...
  prog->Report(2,6);
  // Build a list of rows to check for each row, by row - an adjacency list...
   ds::Array<nat32> adjCount(acc.Height());
   ds::Array2D<nat32> adj(6,acc.Height()); // Can't be more than 6 adjacencies for each vertex.
   for (nat32 i=0;i<adjCount.Size();i++) adjCount[i] = 0;
   
   for (nat32 i=0;i<ico.Edges();i++)
   {
    adj.Get(adjCount[ico.Edge(i)[0]],ico.Edge(i)[0]) = ico.Edge(i)[1]; ++adjCount[ico.Edge(i)[0]];
    adj.Get(adjCount[ico.Edge(i)[1]],ico.Edge(i)[1]) = ico.Edge(i)[0]; ++adjCount[ico.Edge(i)[1]];
   }

  // Iterate through every accumilator entry and check if its a maxima.
  // If so record it into a linked list...
  // (Omit extremities of albedo.)
   ds::List<Model> models;
   prog->Push();
   for (nat32 y=0;y<acc.Height();y++)
   {
    prog->Report(y,acc.Height());
    for (nat32 x=1;x<acc.Width()-1;x++)
    {
     if ((acc.Get(x,y)>acc.Get(x-1,y))&&(acc.Get(x,y)>acc.Get(x+1,y)))
     {
      bit bad = false;
      for (nat32 i=0;i<adjCount[y];i++)
      {
       if (acc.Get(x,y)<=acc.Get(x,adj.Get(i,y)))
       {
        bad = true;
        break;
       }
      }
      if (bad) continue;
      
      Model mod;
      mod.mod = dir[y];
      mod.mod *= albedo[x];
      mod.weight = acc.Get(x,y);
      mod.num = 0;
      models.AddBack(mod);
     }
    }
   }
   prog->Pop();



  // Count the number of segments...
   nat32 segCount = 1;
   for (nat32 i=0;i<data.Size();i++)
   { 
    segCount = math::Max(segCount,data[i].segment+1);
   }



  // Iterate all samples (again) and record on a per segment basis the total error
  // of each model, so we can choose the best overall...
   prog->Report(3,6);
   
   ds::ArrayDel< ds::Array<ModError> > me(segCount); // [segment][model].
   for (nat32 i=0;i<me.Size();i++)
   {
    me[i].Size(models.Size());
    ds::List<Model>::Cursor targ = models.FrontPtr();
    for (nat32 j=0;j<me[i].Size();j++)
    {
     me[i][j].mod = targ.Ptr();
     me[i][j].modVec = targ->mod;
     me[i][j].error = 0.0;
     ++targ;
    }
   }

   prog->Push();
   for (nat32 s=0;s<data.Size();s++)
   {
    LogTime("eos::sfs::LamHough::Run weight");
    prog->Report(s,data.Size());
    
    ds::Array<ModError> & mods = me[data[s].segment];
    real32 maxErrWeighted = data[s].weight * maxErr;
    real32 irrWeighted = data[s].weight * data[s].irr;
    bs::Normal dirWeighted = data[s].dir; dirWeighted *= data[s].weight;

    for (nat32 i=0;i<models.Size();i++)
    {
     ModError & tme = mods[i];
     real32 modIrr = dirWeighted*tme.modVec;
     tme.error += math::Min(math::Abs(irrWeighted - modIrr),maxErrWeighted);
    }
   }
   prog->Pop();



  // Go through the segments, sort the models by best, and score the top models
  // so they go through to the outside user...
   prog->Report(4,6);
   prog->Push();
   for (nat32 i=0;i<segCount;i++)
   {
    prog->Report(i,segCount);
    
    me[i].SortNorm();
    
    for (nat32 j=0;j<math::Min(bestCap,me[i].Size());j++)
    {
     me[i][j].mod->num += bestCap-j;
    }
   }
   prog->Pop();



 // Go through maximas and delete those with insufficient support,
 // transfer to output data structure...
  prog->Report(5,6);
  nat32 modelCount = 0;
  {
   ds::List<Model>::Cursor targ = models.FrontPtr();
   while (!targ.Bad())
   {
    if (targ->num!=0) modelCount += 1;
    ++targ;
   }
  }
  
  out.Size(modelCount);
  
  modelCount = 0;
  {
   ds::List<Model>::Cursor targ = models.FrontPtr();
   while (!targ.Bad())
   {
    if (targ->num!=0)
    {
     out[modelCount] = *targ;
     modelCount += 1;
    }
    ++targ;
   }
  }


 prog->Pop();
}

nat32 LamHough::Maximas() const
{
 return out.Size();
}

const bs::Normal & LamHough::Maxima(nat32 ind) const
{
 return out[ind].mod;
}

real32 LamHough::Weight(nat32 ind) const
{
 return out[ind].weight;
}

nat32 LamHough::MaximaValue(nat32 ind) const
{
 return out[ind].num;
}

//------------------------------------------------------------------------------
 };
};
