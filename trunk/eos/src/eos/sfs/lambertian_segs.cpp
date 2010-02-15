//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/sfs/lambertian_segs.h"

#include "eos/math/iter_min.h"
#include "eos/ds/sort_lists.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
LambertianSeg::LambertianSeg()
{
 mode = false;
 refine = true;

 fp_ga.population = 100;
 fp_ga.keptBest = 5;
 fp_ga.immigrants = 5;
 fp_ga.mutationRate = 0.1;
 fp_ga.generations = 250;
 fp_ga.albSd = 1.0;
 fp_ga.lightSd = 0.1;
 
 fp_rs.chance = 0.99;
 fp_rs.limit = 250.0;
 fp_rs.cap = 10000;
 
 fp_shared.cutoff = math::pi/6.0;
 fp_shared.halflife = math::pi/36.0;
 
 irrSd = 2.0;
 irrCap = 4.0;
 freedomCost = 10.0;
}

LambertianSeg::~LambertianSeg()
{}

void LambertianSeg::Set(const svt::Field<real32> & i)
{
 irr = i;
}

void LambertianSeg::Set(const svt::Field<nat32> & s)
{
 seg = s;
}

void LambertianSeg::Set(const svt::Field<bs::Normal> & n)
{
 needle = n;
}

void LambertianSeg::SetGA(nat32 p,nat32 k,nat32 i,real32 m,nat32 g,real32 a,real32 l,real32 c,real32 h)
{
 mode = false;

 fp_ga.population = p;
 fp_ga.keptBest = k;
 fp_ga.immigrants = i;
 fp_ga.mutationRate = m;
 fp_ga.generations = g;
 fp_ga.albSd = a;
 fp_ga.lightSd = l;

 fp_shared.cutoff = c;
 fp_shared.halflife = h;
}

void LambertianSeg::SetRansac(real32 ch,real32 l,nat32 ca,real32 c,real32 h)
{
 mode = true;
 
 fp_rs.chance = ch;
 fp_rs.limit = l;
 fp_rs.cap = ca;
 
 fp_shared.cutoff = c;
 fp_shared.halflife = h;
}

void LambertianSeg::Refine(bit r)
{
 refine = r;
}

void LambertianSeg::SetMC(real32 id,real32 ic,real32 fc)
{
 irrSd = id;
 irrCap = ic;
 freedomCost = fc;
}

void LambertianSeg::Run(time::Progress * prog)
{
 LogBlock("eos::sfs::LambertianSeg::Run","-");
 prog->Push();

  // We do two passes.
  // The first merges everything, whilst the second merges light directions
  // alone whilst leaving albedos as from the first pass.
  // Greedy merging and a single cost function that takes into account the model
  // is used.
  
  // First pass - entire segment merging. This primarilly exists to get all
  // those pesky lil' segments scattered arround the image eatten up 
  // into bigger segments, making for stable light assignment and bringing the 
  // segment count down to reasonable levels for the next pass...
  prog->Report(0,2);
  prog->Push();
  {
   // Setup the greedy merger...
    alg::GreedyMerge gm;
    GMI1 * gmi = new GMI1(this);
    gm.Set(gmi);


   // Count the segments...
    nat32 segments = 1;
    for (nat32 y=0;y<seg.Size(1);y++)
    {
     for (nat32 x=0;x<seg.Size(0);x++) segments = math::Max(segments,seg.Get(x,y)+1);
    }


   // Count how many samples are in each segment...
    ds::Array<nat32> segSize(segments);
    for (nat32 i=0;i<segSize.Size();i++) segSize[i] = 0;
    
    for (nat32 y=0;y<seg.Size(1);y++)
    {
     for (nat32 x=0;x<seg.Size(0);x++) segSize[seg.Get(x,y)] += 1;
    }


   // Store the data relevant to each segment in the merger...
    ds::Array<Node1*> segData(segments);
    for (nat32 i=0;i<segData.Size();i++)
    {
     segData[i] = new Node1();
     segData[i]->modelValid = false;
     segData[i]->samples.Size(segSize[i]);
     segSize[i] = 0;
    }
    
    for (nat32 y=0;y<seg.Size(1);y++)
    {
     for (nat32 x=0;x<seg.Size(0);x++)
     {
      nat32 s = seg.Get(x,y);
      IrrDir & id = segData[s]->samples[segSize[s]];
      id.irr = irr.Get(x,y);
      id.dir = needle.Get(x,y);
      id.weight = 1.0;
      segSize[s] += 1;
     }
    }
   
    gm.SetNodeCount(segData.Size());
    for (nat32 i=0;i<segData.Size();i++) gm.SetNodeData(i,segData[i]);


   // Collate the edge list from the segmentation...
    ds::SortList<Edge> el;
    for (nat32 y=0;y<seg.Size(1);y++)
    {
     for (nat32 x=0;x<seg.Size(0);x++)
     {
      if (x!=0)
      {
       if (seg.Get(x-1,y)!=seg.Get(x,y))
       {
        Edge e;
        e.a = math::Min(seg.Get(x-1,y),seg.Get(x,y));
        e.b = math::Max(seg.Get(x-1,y),seg.Get(x,y));
        el.Add(e);
       }
      }
      
      if (y!=0)
      {
       if (seg.Get(x,y-1)!=seg.Get(x,y))
       {
        Edge e;
        e.a = math::Min(seg.Get(x,y-1),seg.Get(x,y));
        e.b = math::Max(seg.Get(x,y-1),seg.Get(x,y));
        el.Add(e);
       }
      }
     }
    }
   
   // Store the edge list in the merger...
   {
    ds::SortList<Edge>::Cursor targ = el.FrontPtr();
    while (!targ.Bad())
    {
     gm.AddEdge(targ->a,targ->b);
     ++targ;
    }
   }


   // Run...
    gm.Run(prog);
    
    
   // Extract the results into the output data structure...
    segToLight.Size(segments);
    segToAlbedo.Size(segments);
    for (nat32 i=0;i<segments;i++)
    {
     segToLight[i] = gm.InToOut(i);
     segToAlbedo[i] = segToLight[i];
    }
    
    light.Size(gm.Nodes());
    albedo.Size(gm.Nodes());
    for (nat32 i=0;i<gm.Nodes();i++)
    {
     const Node1 * targ = static_cast<const Node1*>(gm.Data(i));

     light[i] = targ->model;
     light[i].Normalise();
     albedo[i] = targ->model.Length();
    }
  }
  prog->Pop();


  // Second pass, light source direction merging, as light directions will
  // generally be shared between segments with different albedos...
  prog->Report(1,2);
  prog->Push();
  {
  
  
  
  }
  prog->Pop();


 prog->Pop();




 /*prog->Push();
 prog->Report(0,1);

 // First build a basic model where each segment has its own fitted light and albedo...
  prog->Push();
  // Count the segments...
   nat32 segCount = 1;
   for (nat32 y=0;y<seg.Size(1);y++)
   {
    for (nat32 x=0;x<seg.Size(0);x++) segCount = math::Max(segCount,seg.Get(x,y)+1);
   }


  // Resize the output arrays accordingly...
   segToLight.Size(segCount);
   segToAlbedo.Size(segCount);
   light.Size(segCount);
   albedo.Size(segCount);

 
  // Count the number of pixels in each segment...
   ds::Array<nat32> segSize(segCount);
   for (nat32 i=0;i<segSize.Size();i++) segSize[i] = 0;

   for (nat32 y=0;y<seg.Size(1);y++)
   {
    for (nat32 x=0;x<seg.Size(0);x++) segSize[seg.Get(x,y)] += 1;
   }


  // Create arrays of IrrDir to represent the segment info...
   ds::ArrayDel<ds::Array<IrrDir> > in;
   in.Size(segCount);
   for (nat32 i=0;i<in.Size();i++)
   {
    in[i].Size(segSize[i]);
    segSize[i] = 0;
   }
   
   for (nat32 y=0;y<seg.Size(1);y++)
   {
    for (nat32 x=0;x<seg.Size(0);x++)
    {
     IrrDir & targ = in[seg.Get(x,y)][segSize[seg.Get(x,y)]];
     segSize[seg.Get(x,y)] += 1;
     
     targ.irr = irr.Get(x,y);
     targ.dir = needle.Get(x,y);
     targ.weight = 1.0; // Will probably want to set this in a more advanced fashion latter.
    }
   }


  // Fit to each segment - this is really slow, but not as slow as what comes next...
  {
   NeedleSeg ns(1);
   NeedleSegModel nsm(1,1);
   for (nat32 i=0;i<segCount;i++)
   {
    prog->Report(i,segCount);
    
    // Fill in details...
     ns.Set(0,0,0,in[i]);
    
    // Run...
     LambertianFit lf;
     lf.Set(ns);
     lf.Set(nsm);
     if (mode)
     {
      lf.SetRansac(fp_rs.chance,fp_rs.limit,fp_rs.cap,fp_shared.cutoff,fp_shared.halflife);
     }
     else
     {
      lf.SetGA(fp_ga.population,fp_ga.keptBest,fp_ga.immigrants,fp_ga.mutationRate,fp_ga.generations,
               fp_ga.albSd,fp_ga.lightSd,fp_shared.cutoff,fp_shared.halflife);
     }
     lf.Refine(refine);
     
     lf.Run(prog);
    
    // Extract results...
     segToLight[i] = i;
     segToAlbedo[i] = i;
     light[i] = nsm.Light(0);
     albedo[i] = nsm.Albedo(0);
   }
  }
  
 prog->Pop();*/
}

void LambertianSeg::GetModel(svt::Field<bs::Vert> & out)
{
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++)
  {
   out.Get(x,y)  = light[segToLight[seg.Get(x,y)]];
   out.Get(x,y) *= albedo[segToAlbedo[seg.Get(x,y)]];
  }
 }
}

void LambertianSeg::GetLight(svt::Field<bs::Normal> & out)
{
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++)
  {
   out.Get(x,y) = light[segToLight[seg.Get(x,y)]];
  }
 }
}

void LambertianSeg::GetAlbedo(svt::Field<real32> & out)
{
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++)
  {
   out.Get(x,y) = albedo[segToAlbedo[seg.Get(x,y)]];
  }
 }
}

real32 LambertianSeg::ModelCost(const NeedleSeg & ns,const NeedleSegModel & nsm) const
{
 LogTime("eos::sfs::LambertianSeg::ModelCost");
 real32 cost = 0.0;
 
 // First sum in the model fitting probabilities...
 // Iterate each segment, for each segment calculate its model, then iterate 
 // every pixel in the segment, calculate its probability and sum in its cost 
 // accordingly, with the cap...
  for (nat32 i=0;i<ns.Segments();i++)
  {
   // Get the segments model...
    nat32 lightGroup = ns.LightGroup(i);
    nat32 albedoGroup = ns.AlbedoGroup(i);
    
    bs::Normal mod = nsm.Light(lightGroup);
    mod *= nsm.Albedo(albedoGroup);


   // Iterate the samples...
    const ds::Array<IrrDir> & data = ns.Samples(i);
    for (nat32 j=0;j<data.Size();j++)
    {
     real32 irrPred = mod * data[j].dir;
     real32 err = math::Abs(irrPred - data[j].irr);
     err = math::Min(err,irrCap);

     real32 c = math::Sqr(err)/(2.0*math::Sqr(irrSd));
     // c += math::Ln(irrSd * math::Sqrt(2.0*math::pi));
     // Term above not needed as we will only be comparing models for which it is identical.
     
     cost += c*data[j].weight;
    }
  }


 // Then sum in the costs of the degrees of freedom, i.e. the cost of how many 
 // parameters the model has...
  nat32 degrees = nsm.Lights()*2 + nsm.Albedos();
  cost += degrees * freedomCost;


 // And finally sum in the cost derived from the approximated hessian, i.e. the
 // cost associated with how sensative the fitted model is...
  // Calculate the Jacobian for the model, this is rather complicated as we have
  // to convert to spherical coordinates and use a function to work out the 
  // error vector for a parameter vector...
   // Create parameter vector...
    math::Vector<real32> pv(degrees);

    for (nat32 i=0;i<nsm.Lights();i++)
    {
     pv[i*2]   = math::InvCos(nsm.Light(i)[2]/nsm.Light(i).Length());
     pv[i*2+1] = math::InvTan2(nsm.Light(i)[1],nsm.Light(i)[0]);
    }

    for (nat32 i=0;i<nsm.Albedos();i++)
    {
     pv[nsm.Lights()*2 + i] = nsm.Albedo(i);
    }

   // Create suport structure...
    Store store;
    store.cutoff = fp_shared.cutoff;
    store.halflife = fp_shared.halflife;
    store.ns = &ns;
    store.temp.Size(nsm.Lights());

   // Calculate the Jacobian...
    math::Matrix<real32> jacobian(ns.Segments(),degrees);
    math::JacobianLM(ns.Segments(),pv,store,&Func,jacobian);
    
  
  // Get Hessian via the J^T J approximation...
   math::Matrix<real32> hessian(degrees,degrees);
   math::TransMult(jacobian,jacobian,hessian);

  // Calculate the term we need to add to the final cost from the hessian approximation...
   hessian *= 1.0 / (2.0 * math::pi);
   cost -= 0.5 * math::Determinant(hessian);


 return cost;
}

void LambertianSeg::Func(const math::Vector<real32> & pv,math::Vector<real32> & err,const Store & store)
{
 // Extract the light source directions from the parameter vector...
  for (nat32 i=0;i<store.temp.Size();i++)
  {
   store.temp[i][0] = math::Sin(pv[i*2]) * math::Cos(pv[i*2+1]);
   store.temp[i][1] = math::Sin(pv[i*2]) * math::Sin(pv[i*2+1]);
   store.temp[i][2] = math::Cos(pv[i*2]);
  }


 // Iterate the segments, sum and store the error for each segment...
  for (nat32 i=0;i<err.Size();i++)
  {
   err[i] = 0.0;
   
   const ds::Array<IrrDir> & sa = store.ns->Samples(i);
   bs::Normal & l = store.temp[store.ns->LightGroup(i)];
   real32 a = pv[store.temp.Size()*2 + store.ns->AlbedoGroup(i)];
   
   for (nat32 j=0;j<sa.Size();j++)
   {
    real32 guess = a*math::Max((sa[j].dir * l),real32(0.0));
    real32 e = math::Abs(guess - sa[j].irr);
    err[i] += sa[j].weight * (1.0 - math::SigmoidCutoff(e,store.cutoff,store.halflife));
   }
  }
}

//------------------------------------------------------------------------------
LambertianSeg::GMI1::GMI1(LambertianSeg * s)
:self(s),ns1(1),ns2(2),nsm(1,1)
{
 if (self->mode)
 {
  // Ransac...
   lf.SetRansac(self->fp_rs.chance,self->fp_rs.limit,self->fp_rs.cap,
                self->fp_shared.cutoff,self->fp_shared.halflife);
 }
 else
 {
  // Ga...
   lf.SetGA(self->fp_ga.population,self->fp_ga.keptBest,self->fp_ga.immigrants,self->fp_ga.mutationRate,
            self->fp_ga.generations,self->fp_ga.albSd,self->fp_ga.lightSd,
            self->fp_shared.cutoff,self->fp_shared.halflife);
 }

 lf.Refine(self->refine);              
}

LambertianSeg::GMI1::~GMI1()
{}

real32 LambertianSeg::GMI1::Cost(const Deletable * a) const
{
 LogTime("eos::sfs::LambertianSeg::GMI1::Cost(1)");
 const Node1 * na = static_cast<const Node1*>(a);
 
 ns1.Set(0,0,0,na->samples);

 if (!na->modelValid)
 {
  lf.Set(ns1);
  lf.Set(nsm);
  lf.Run();
  
  na->model = nsm.Light(0);
  na->model *= nsm.Albedo(0);
  na->modelValid = true;
 }
 
 return self->ModelCost(ns1,nsm);
}

real32 LambertianSeg::GMI1::Cost(const Deletable * a,const Deletable * b) const
{
 LogTime("eos::sfs::LambertianSeg::GMI1::Cost(2)");
 const Node1 * na = static_cast<const Node1*>(a);
 const Node1 * nb = static_cast<const Node1*>(b);

 ns2.Set(0,0,0,na->samples);
 ns2.Set(1,0,0,nb->samples);
 
 lf.Set(ns2);
 lf.Set(nsm);
 lf.Run();

 return self->ModelCost(ns2,nsm);
}

Deletable * LambertianSeg::GMI1::Merge(const Deletable * a,const Deletable * b) const
{
 LogTime("eos::sfs::LambertianSeg::GMI1::Merge");
 const Node1 * na = static_cast<const Node1*>(a);
 const Node1 * nb = static_cast<const Node1*>(b);
 
 // Make new node...
  Node1 * ret = new Node1();
  
 // Fill in samples...
  ret->samples.Size(na->samples.Size()+nb->samples.Size());
  for (nat32 i=0;i<na->samples.Size();i++) ret->samples[i] = na->samples[i];
  for (nat32 i=0;i<nb->samples.Size();i++) ret->samples[na->samples.Size() + i] = nb->samples[i];

 // Calculate and fill in model...
  ns1.Set(0,0,0,ret->samples);

  lf.Set(ns1);
  lf.Set(nsm);
  lf.Run();
  
  ret->model = nsm.Light(0);
  ret->model *= nsm.Albedo(0);
  ret->modelValid = true;

 // Return...
  return ret;
}

//------------------------------------------------------------------------------
 };
};
