//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/mya/layers.h"

namespace eos
{
 namespace mya
 {
//------------------------------------------------------------------------------
Layers::Layers()
:fa(Prune),random(null<data::Random*>())
{}

Layers::~Layers()
{}

void Layers::SetSegs(nat32 sc,const svt::Field<nat32> & s)
{
 data.Size(sc);
 segs = s;
 
 // Add the null score to the cache, just makes latter code neater as it acts as
 // a nice dummy...
  CacheNode * nn = new CacheNode(sc);
  nn->SetScore(0.0);
  cache.Add(nn);
}

void Layers::SetFitMethod(Layers::FitAlg f,data::Random * r)
{
 fa = f;
 random = r;
}

nat32 Layers::AddIed(Ied * ied)
{
 nat32 ret = da.Size();
  da.Size(ret+1);
 
  da[ret].ied = ied;
  da[ret].cutoff = 1.0; 
 return ret;
}

void Layers::SetOutlierDist(nat32 di,real32 cutoff)
{
 da[di].cutoff = cutoff;
}

nat32 Layers::AddSurfaceType(SurfaceType * st)
{
 nat32 ret = sa.Size();
  sa.Size(ret+1);
 
  sa[ret].type = st;
  sa[ret].weight = 1.0; 
 return ret; 
}

void Layers::SetSurfaceWeight(nat32 si,real32 weight)
{
 sa[si].weight = weight;
}

void Layers::Commit(time::Progress * prog)
{
 prog->Push();  
  
  // Build a cache of segment-members, to make the fitting code a reasonable speed,
  // different cache for each data type...
   prog->Report(0,2);
   nat32 * segValidSize = new nat32[data.Size()];
   for (nat32 i=0;i<da.Size();i++)
   {
    // First count the # valid items for each segment, building the structure...     
     for (nat32 j=0;j<data.Size();j++) segValidSize[j] = 0;
     for (nat32 y=0;y<segs.Size(1);y++)
     {
      for (nat32 x=0;x<segs.Size(0);x++)
      {
       if (da[i].ied->Valid(x,y)) segValidSize[segs.Get(x,y)] += 1;
      }
     }
   
    // Now build the structure...
     da[i].surfData.Size(data.Size());
     for (nat32 j=0;j<data.Size();j++)
     {
      da[i].surfData[j].Size(segValidSize[j]);
      for (nat32 k=0;k<da[i].surfData[j].Size();k++) da[i].surfData[j][k].vec.SetSize(da[i].ied->Length());
     }
     
    // Now fill it with data...
     for (nat32 j=0;j<data.Size();j++) segValidSize[j] = 0;
     for (nat32 y=0;y<segs.Size(1);y++)
     {
      for (nat32 x=0;x<segs.Size(0);x++)
      {
       NodeSample & targ = da[i].surfData[segs.Get(x,y)][segValidSize[segs.Get(x,y)]];
       if (da[i].ied->Feature(x,y,targ.vec))
       {
        targ.x = (2.0*x)/segs.Size(0) - 1.0;
        targ.y = (2.0*y)/segs.Size(0) - 1.0;
        segValidSize[segs.Get(x,y)] += 1;
       }
      }
     }  
   }
   delete[] segValidSize;
   
   
  // Build the data structure, fitting surfaces to each segment...
   layers = data.Size();
   prog->Report(1,2);
   prog->Push();
   for (nat32 i=0;i<data.Size();i++)
   {
    prog->Report(i,data.Size());
    data[i].parent = null<Node*>();
    data[i].seg = i;
    data[i].editted = false;
    data[i].segSize = 0;    
    
    data[i].surface = SegFit(i,data[i].cost);
    if (data[i].surface==null<Surface*>()) data[i].cost = 1e10;
    data[i].pixels = 0;
   }
   prog->Pop();
   
  // Iterate all segments, to sum up the segSize/pixels counts...
   for (nat32 y=0;y<segs.Size(1);y++)
   {
    for (nat32 x=0;x<segs.Size(0);x++)
    {
     nat32 seg = segs.Get(x,y);
     data[seg].segSize += 1;
     data[seg].pixels += 1;     
    }
   }

 prog->Pop();
}

nat32 Layers::SegmentCount() const
{
 return data.Size();
}

nat32 Layers::LayerCount() const
{
 return layers;
}

nat32 Layers::SegToLayer(nat32 seg) const
{
 return data[seg].Head()->seg;
}

void Layers::GetLayers(ds::Array<nat32> & out) const
{
 out.Size(layers);
 nat32 pos = 0;
 for (nat32 i=0;i<data.Size();i++)
 {
  if (data[i].parent==null<Node*>())
  {
   out[pos] = i;
   ++pos;
  }
 }
}

void Layers::GetLayerFlags(nat32 layer,ds::Array<bit> & foi,bit setFalse) const
{
 Node * head = data[layer].Head();
 for (nat32 i=0;i<data.Size();i++)
 {
  if (data[i].Head()==head) foi[i] = true;
         else if (setFalse) foi[i] = false;
 }
}

Surface * Layers::LayerToSurf(nat32 layer) const
{
 return data[layer].Head()->surface;
}

nat32 Layers::SegmentSize(nat32 seg) const
{
 return data[seg].segSize;
}

nat32 Layers::LayerSize(nat32 seg) const
{
 return data[seg].Head()->pixels;
}

real32 Layers::FitCost(nat32 seg) const
{
 return data[seg].Head()->cost;
}
    
void Layers::SetSeglayer(nat32 seg,nat32 layer)
{
  Node * head = data[layer].Head();
  if (head!=data[seg].Head())
  {
   // Remove it from its current layer...
    Seperate(seg);

   // Add it to the new...
    data[seg].surface = null<Surface*>();
    
    data[seg].parent = head;
    head->pixels += data[seg].segSize;
    head->editted = true;
    --layers;
  }
}

void Layers::MergeLayers(nat32 lay1,nat32 lay2)
{
 Node * h1 = data[lay1].Head();
 Node * h2 = data[lay2].Head();
 if (h1!=h2)
 {
  h2->parent = h1;
  
  if (h1->cost>h2->cost)
  {
   h1->surface = h2->surface;   
   h1->cost = h2->cost;
  }
  h2->surface = null<Surface*>();
  h2->cost = 1e10;
  
  h1->editted = true;
  h1->pixels += h2->pixels;
  --layers;
 }
}

void Layers::Seperate(nat32 seg)
{
 if (data[seg].parent==null<Node*>())
 {
  // It is its own head, if it has children we are going to have to make
  // one of them the new parent...
   nat32 i;
   for (i=0;i<data.Size();i++)
   {
    if ((data[i].Head()==&data[seg])&&(i!=seg)) break;
   }
   if (i==data.Size()) return; // Allready seperate, lets bug out.
   
   data[i].surface = data[seg].surface;
   data[seg].surface = null<Surface*>();
   data[i].parent = null<Node*>();
   data[seg].parent = &data[i];
   data[i].pixels = data[seg].pixels;
   
  // Path shorten the rest...
   for (;i<data.Size();i++) data[i].Head();
 }
 else
 {
  // It has a parent - all we have to do is path shorten every node...
   for (nat32 i=0;i<data.Size();i++) data[i].Head();
 }
 
 data[seg].Head()->editted = true;
 data[seg].Head()->pixels -= data[seg].segSize;
 data[seg].pixels = data[seg].segSize;
 data[seg].parent = null<Node*>();
 data[seg].editted = true;
 ++layers;
}

void Layers::Refit(time::Progress * prog)
{
 prog->Push();
  ds::Array<bit> foi(data.Size());
  for (nat32 i=0;i<data.Size();i++)
  {
   if ((data[i].editted)&&(data[i].parent==null<Node*>()))
   {
    GetLayerFlags(i,foi); 
    data[i].surface = LayerFit(foi,data[i].cost);
    if (data[i].surface==null<Surface*>()) data[i].cost = 1e10;
   }
  }
 prog->Pop();
}

Surface * Layers::SegFit(nat32 seg,real32 & cost) const
{
 // Check if ths answer is allready cached, if so use the cached version...
  CacheNode node(SegmentCount());
   node.AddMember(seg);
  CacheNode ** targ = cache.Get(&node);
  if (targ!=null<CacheNode**>())
  {
   log::Assert(*targ);
   cost = (*targ)->GetScore();
   return (*targ)->GetSurface();
  }

 // Setup the segSet array...
  segSet.Size(1);
  segSet[0] = seg;

 // Calculate and store in the cache the best fit surface...
  Surface * ret = SegFitInt(cost);
  
  CacheNode * nn = new CacheNode(SegmentCount());
   nn->AddMember(seg);  
   nn->SetScore(cost);
   nn->SetSurface(ret);
  cache.Add(nn);

 // Return the best fitted surface...   
  return ret;
}

real32 Layers::SegFitCost(nat32 seg) const
{
 real32 ret = 1e10;
  SegFit(seg,ret);
 return ret;
}

Surface * Layers::LayerFit(const ds::Array<bit> & foi,real32 & cost) const
{
 // Check if the answer is allready cached, if so use the cached version...
  CacheNode node(SegmentCount());
   node.AddMembers(foi);
  CacheNode ** targ = cache.Get(&node);
  if (targ!=null<CacheNode**>())
  {
   cost = (*targ)->GetScore();
   return (*targ)->GetSurface();
  }
  
 // Setup the segSet array...
  nat32 segCount = 0;
  for (nat32 i=0;i<foi.Size();i++) if (foi[i]) ++segCount;
  segSet.Size(segCount);
  
  segCount = 0;
  for (nat32 i=0;i<foi.Size();i++) if (foi[i]) segSet[segCount++] = i;

 // Calculate and store in the cache the best fit surface...
  Surface * ret = SegFitInt(cost);
  
  CacheNode * nn = new CacheNode(SegmentCount());  
   nn->AddMembers(foi);
   nn->SetScore(cost);
   nn->SetSurface(ret);
  cache.Add(nn);
  
 // Return the best fitted surface...   
  return ret;
}

real32 Layers::LayerFitCost(const ds::Array<bit> & foi) const
{
 real32 ret = 1e10;
  LayerFit(foi,ret);
 return ret;
}

real32 Layers::GetDepth(real32 limit,real32 x,real32 y)
{
 // Get segment numbers for each corner, taking care of the borders...
  int32 xBase = nat32(math::RoundDown(x));
  int32 yBase = nat32(math::RoundDown(y));  
  nat32 cs[2][2];
   cs[0][0] = segs.Get(math::Clamp<int32>(xBase,0,segs.Size(0)-1),math::Clamp<int32>(yBase,0,segs.Size(1)-1));
   cs[0][1] = segs.Get(math::Clamp<int32>(xBase,0,segs.Size(0)-1),math::Clamp<int32>(yBase+1,0,segs.Size(1)-1));
   cs[1][0] = segs.Get(math::Clamp<int32>(xBase+1,0,segs.Size(0)-1),math::Clamp<int32>(yBase,0,segs.Size(1)-1));
   cs[1][1] = segs.Get(math::Clamp<int32>(xBase+1,0,segs.Size(0)-1),math::Clamp<int32>(yBase+1,0,segs.Size(1)-1));
     
 // Get values for the 4 surrounding pixels, handling boundary conditions and failures.
 // We optimise to only sample each surface once, as repeated samplings are a waste...
 // (We don't check for duplication across the diagonal however, as that ushally dosn't happen.)
  math::Vect<2> ss[2][2];
   Surface * temp = LayerToSurf(cs[0][0]);
   if (temp) temp->Get((2.0*x)/real32(segs.Size(0))-1.0,(2.0*y)/real32(segs.Size(0))-1.0,ss[0][0]);
        else {ss[0][0][0] = 0.0; ss[0][0][1] = 0.0;}
 
   if (cs[0][0]!=cs[0][1])
   {
    temp = LayerToSurf(cs[0][1]);
    if (temp) temp->Get((2.0*x)/real32(segs.Size(0))-1.0,(2.0*y)/real32(segs.Size(0))-1.0,ss[0][1]);
         else {ss[0][1][0] = 0.0; ss[0][1][1] = 0.0;}
   }
   else ss[0][1] = ss[0][0];
   
   if (cs[0][0]!=cs[1][0])
   {
    temp = LayerToSurf(cs[1][0]);
    if (temp) temp->Get((2.0*x)/real32(segs.Size(0))-1.0,(2.0*y)/real32(segs.Size(0))-1.0,ss[1][0]);
         else {ss[1][0][0] = 0.0; ss[1][0][1] = 0.0;}
   }
   else ss[1][0] = ss[0][0];

   if (cs[0][1]==cs[1][1]) ss[1][1] = ss[0][1];
   else if (cs[1][0]==cs[1][1]) ss[1][1] = ss[1][0];
   else 
   {
    temp = LayerToSurf(cs[1][1]);
    if (temp) temp->Get((2.0*x)/real32(segs.Size(0))-1.0,(2.0*y)/real32(segs.Size(0))-1.0,ss[1][1]);
         else {ss[1][1][0] = 0.0; ss[1][1][1] = 0.0;}
   }

 // Average the returned values, excluding bad values and handling all values bad...
  nat32 sum = 0;
  real32 ret = 0.0;
   if (!math::Equal(ss[0][0][1],real32(0.0))) {++sum; ret +=ss[0][0][0]/ss[0][0][1];}
   if (!math::Equal(ss[0][1][1],real32(0.0))) {++sum; ret +=ss[0][1][0]/ss[0][1][1];}
   if (!math::Equal(ss[1][0][1],real32(0.0))) {++sum; ret +=ss[1][0][0]/ss[1][0][1];}
   if (!math::Equal(ss[1][1][1],real32(0.0))) {++sum; ret +=ss[1][1][0]/ss[1][1][1];}

  if (sum==0) return limit;
         else return math::Min(ret/real32(sum),limit);
}

real32 Layers::GetDisp(real32 mult,real32 x,real32 y)
{
 // Get segment numbers for each corner, taking care of the borders...
  int32 xBase = nat32(math::RoundDown(x));
  int32 yBase = nat32(math::RoundDown(y));  
  nat32 cs[2][2];
   cs[0][0] = segs.Get(math::Clamp<int32>(xBase,0,segs.Size(0)-1),math::Clamp<int32>(yBase,0,segs.Size(1)-1));
   cs[0][1] = segs.Get(math::Clamp<int32>(xBase,0,segs.Size(0)-1),math::Clamp<int32>(yBase+1,0,segs.Size(1)-1));
   cs[1][0] = segs.Get(math::Clamp<int32>(xBase+1,0,segs.Size(0)-1),math::Clamp<int32>(yBase,0,segs.Size(1)-1));
   cs[1][1] = segs.Get(math::Clamp<int32>(xBase+1,0,segs.Size(0)-1),math::Clamp<int32>(yBase+1,0,segs.Size(1)-1));
     
 // Get values for the 4 surrounding pixels, handling boundary conditions and failures.
 // We optimise to only sample each surface once, as repeated samplings are a waste...
 // (We don't check for duplication across the diagonal however, as that ushally dosn't happen.)
  math::Vect<2> ss[2][2];
   Surface * temp = LayerToSurf(cs[0][0]);
   if (temp) temp->Get((2.0*x)/real32(segs.Size(0))-1.0,(2.0*y)/real32(segs.Size(0))-1.0,ss[0][0]);
        else {ss[0][0][0] = 0.0; ss[0][0][1] = 0.0;}
 
   if (cs[0][0]!=cs[0][1])
   {
    temp = LayerToSurf(cs[0][1]);
    if (temp) temp->Get((2.0*x)/real32(segs.Size(0))-1.0,(2.0*y)/real32(segs.Size(0))-1.0,ss[0][1]);
         else {ss[0][1][0] = 0.0; ss[0][1][1] = 0.0;}
   }
   else ss[0][1] = ss[0][0];
   
   if (cs[0][0]!=cs[1][0])
   {
    temp = LayerToSurf(cs[1][0]);
    if (temp) temp->Get((2.0*x)/real32(segs.Size(0))-1.0,(2.0*y)/real32(segs.Size(0))-1.0,ss[1][0]);
         else {ss[1][0][0] = 0.0; ss[1][0][1] = 0.0;}
   }
   else ss[1][0] = ss[0][0];

   if (cs[0][1]==cs[1][1]) ss[1][1] = ss[0][1];
   else if (cs[1][0]==cs[1][1]) ss[1][1] = ss[1][0];
   else 
   {
    temp = LayerToSurf(cs[1][1]);
    if (temp) temp->Get((2.0*x)/real32(segs.Size(0))-1.0,(2.0*y)/real32(segs.Size(0))-1.0,ss[1][1]);
         else {ss[1][1][0] = 0.0; ss[1][1][1] = 0.0;}
   }

 // Average the returned values, excluding bad values and handling all values bad...
  nat32 sum = 0;
  real32 ret = 0.0;
   if (!math::Equal(ss[0][0][0],real32(0.0))) {++sum; ret += mult*ss[0][0][1]/ss[0][0][0];}
   if (!math::Equal(ss[0][1][0],real32(0.0))) {++sum; ret += mult*ss[0][1][1]/ss[0][1][0];}
   if (!math::Equal(ss[1][0][0],real32(0.0))) {++sum; ret += mult*ss[1][0][1]/ss[1][0][0];}
   if (!math::Equal(ss[1][1][0],real32(0.0))) {++sum; ret += mult*ss[1][1][1]/ss[1][1][0];}

  if (sum==0) return 0.0;
         else return ret/real32(sum);
}

void Layers::GetDepthMap(real32 limit,svt::Field<real32> & depth) const
{
 for (nat32 y=0;y<segs.Size(1);y++)
 {
  for (nat32 x=0;x<segs.Size(0);x++)
  {
   nat32 seg = segs.Get(x,y);
   Surface * surface = data[seg].Head()->surface;
   
   if (surface)
   {
    math::Vect<2> z;   
    surface->Get((2.0*x)/real32(segs.Size(0))-1.0,(2.0*y)/real32(segs.Size(0))-1.0,z);
    if (math::Equal(z[1],real32(0.0))) depth.Get(x,y) = limit;
                                 else depth.Get(x,y) = math::Min(z[0]/z[1],limit);
   }
   else depth.Get(x,y) = limit;
  }
 }
}

void Layers::GetDispMap(real32 mult,svt::Field<real32> & disp) const
{
 for (nat32 y=0;y<segs.Size(1);y++)
 {
  for (nat32 x=0;x<segs.Size(0);x++)
  {
   nat32 seg = segs.Get(x,y);
   Surface * surface = data[seg].Head()->surface;
   
   if (surface)
   {
    math::Vect<2> z;
    surface->Get((2.0*x)/real32(segs.Size(0))-1.0,(2.0*y)/real32(segs.Size(0))-1.0,z);
    if (math::Equal(z[0],real32(0.0))) disp.Get(x,y) = 0.0;
                                  else disp.Get(x,y) = mult*z[1]/z[0];
   }
   else disp.Get(x,y) = 0.0;
  }
 }
}

void Layers::GetLayerMap(svt::Field<nat32> & layers) const
{
 // First create a segment to layer map...
  ds::Array<nat32> map(SegmentCount());
  ds::Array<nat32> lm(LayerCount());
  GetLayers(lm);
  for (nat32 i=0;i<lm.Size();i++) map[lm[i]] = i;
  for (nat32 i=0;i<map.Size();i++) map[i] = map[SegToLayer(i)];
 
 // Then write the output...
  for (nat32 y=0;y<segs.Size(1);y++)
  {
   for (nat32 x=0;x<segs.Size(0);x++)
   {
    layers.Get(x,y) = map[segs.Get(x,y)];
   }
  }
}

void Layers::GetSurfaceMap(svt::Field<nat32> & surfaces) const
{
 // Create a surface number for each layer...
  ds::Array<nat32> lts(LayerCount());
  GetLayers(lts);
  for (nat32 i=0;i<lts.Size();i++)
  {
   Surface * targ = LayerToSurf(lts[i]);
   if (targ==null<Surface*>()) lts[i] = 0;
   else
   {
    for (nat32 j=0;j<sa.Size();j++)
    {
     if (sa[j].type->IsMember(targ))
     {
      lts[i] = j+1;
      break; 
     }
    }
   }
  }
 
 
 // Create a segment to layer surface number map...
  ds::Array<nat32> map(SegmentCount());
  ds::Array<nat32> lm(LayerCount());
  GetLayers(lm);
  for (nat32 i=0;i<lm.Size();i++) map[lm[i]] = lts[i];
  for (nat32 i=0;i<map.Size();i++) map[i] = map[SegToLayer(i)];  
 
 // Then write the output...
  for (nat32 y=0;y<segs.Size(1);y++)
  {
   for (nat32 x=0;x<segs.Size(0);x++)
   {
    surfaces.Get(x,y) = map[segs.Get(x,y)];
   }
  }
}

void Layers::GetValidity(svt::Field<bit> & valdity) const
{
 // Create a segment to validity map...
  ds::Array<bit> map(SegmentCount());
  ds::Array<nat32> lm(LayerCount());
   GetLayers(lm);
  for (nat32 i=0;i<lm.Size();i++) map[lm[i]] = LayerToSurf(i)!=null<Surface*>();
  for (nat32 i=0;i<map.Size();i++) map[i] = map[SegToLayer(i)];  
  
 // Then write the output...
  for (nat32 y=0;y<segs.Size(1);y++)
  {
   for (nat32 x=0;x<segs.Size(0);x++)
   {
    valdity.Get(x,y) = map[segs.Get(x,y)];
   }
  }
}

void Layers::IntCoord(const math::Vect<2> & in,math::Vect<2> & out)
{
 out[0] = 2.0*in[0]/real32(segs.Size(0)) - 1.0;
 out[1] = 2.0*in[1]/real32(segs.Size(0)) - 1.0;
}
 
Surface * Layers::SegFitInt(real32 & bestScore) const
{
 // Iterate each surface type and generate the best surface,
 // return the surface with the least number of weighted outliers...
  Surface * best = null<Surface*>();
  ds::Array<NodeIed*> lda;
  for (nat32 i=0;i<sa.Size();i++)
  {
   // Check if we have enough data to even concider fitting this surface type,
   // if not give up early, or reduce the fitting algorithm from ransac if 
   // there is enough data to fit but not enough for ransac.
    FitAlg lfa = fa;
    nat32 totalInfo = 0;
    nat32 totalDTS = 0;
    for (nat32 j=0;j<da.Size();j++)
    {
     str::Token dataType = da[j].ied->Type(tt);
     nat32 outType;
     if (sa[i].type->Supports(tt,dataType,outType))
     {
      ++totalDTS;
      da[j].sampleCount = 0;
      for (nat32 k=0;k<segSet.Size();k++) da[j].sampleCount += da[j].surfData[segSet[k]].Size();
      
      totalInfo += sa[i].type->TypeDegrees(outType) * da[j].sampleCount;
     }
    }    
    nat32 reqInfo = sa[i].type->Degrees();
    if (totalInfo<reqInfo) continue;
    if (totalInfo<reqInfo*ranScale) lfa = Prune; // Not enough data to do anything advanced, revert to a prune.


   // Generate a fitting data set customised to the surface type...
    lda.Size(totalDTS);
    nat32 pos = 0;
    nat32 totalSamples = 0;
    for (nat32 j=0;j<da.Size();j++)
    {
     str::Token dataType = da[j].ied->Type(tt);
     nat32 outType;
     if (sa[i].type->Supports(tt,dataType,outType))
     {
      lda[pos] = &da[j];
      lda[pos]->ind = outType;
      lda[pos]->sampleSum = totalSamples;
      totalSamples += lda[pos]->sampleCount;
      ++pos;
     }
    }
   
   
   // Do the fitting, factor in weighting...
    real32 score;    
    Surface * surface = SegFitInt(lfa,sa[i],lda,score);
    if (surface)
    {
     score *= sa[i].weight;
     
     // If its the best fitting so far record it, otherwise delete it...
      if ((best==null<Surface*>())||(bestScore>score))
      {
       delete best;
       best = surface;
       bestScore = score;
      }
      else delete surface;
    }
  }
  
 return best;
}

Surface * Layers::SegFitInt(FitAlg lfa,NodeSurface & st,const ds::Array<NodeIed*> & ds,real32 & out) const
{
 // First create an initial fitting, if its the prune method we just fit all the
 // data, otherwise its ransac time...
  Surface * ret = null<Surface*>();
  Fitter * fitter = st.type->NewFitter();
  if (lfa==Ransac)
  {
   // Do ransac, iterate giving a minimal number of samples, finally selecting 
   // the solution with the most inliers...
   // (We use the standard finishing condition.)
    nat32 dataAvaliable = ds[ds.Size()-1]->sampleSum + ds[ds.Size()-1]->sampleCount;
   
    ret = null<Surface*>(); // ret is the best surface found.
    nat32 mostInliers = st.type->Degrees();
    nat32 runs = 0;
    for (nat32 k=0;k<maxRansacIter;k++)
    {
     // Do a minimal fit with a random set of vectors...
     // (If the fit fails we ignore it and move on.)
      fitter->Reset();
      for (nat32 l=0;l<st.type->Degrees();l++) // Limit the iterations, otherwise we can hit an infinite loop in some (semi-obscure) circumstances.
      {
       // Select a random data item...
        nat32 val = random->Int(0,dataAvaliable-1);
        
       // Calculate which data block its in...
        nat32 db = 0;
        for (;db<ds.Size();db++)
        {
         if (val<ds[db]->sampleCount) break;
         val -= ds[db]->sampleCount;
        }
        
       // Calculate which segment in the data block its in...       
        nat32 sb = 0;
        for (;sb<segSet.Size();sb++)
        {
         if (val<ds[db]->surfData[segSet[sb]].Size()) break;
         val -= ds[db]->surfData[segSet[sb]].Size();
        }
       
       // Add the entry in, if the fitter reports it is now satisfied we then break...
        NodeSample & targ = ds[db]->surfData[segSet[sb]][val];
                      
        if (fitter->Add(ds[db]->ind,targ.x,targ.y,targ.vec)) break;
      }
      Surface * calc = st.type->NewSurface();
      if (fitter->Extract(*calc)==false)
      {
       delete calc;
       continue;
      }      
     
     // Count the inliers and outliers...
      nat32 inlierC = 0;
      nat32 outlierC = 0;
      for (nat32 i=0;i<ds.Size();i++)
      {
       for (nat32 j=0;j<segSet.Size();j++)
       {
        for (nat32 l=0;l<ds[i]->surfData[segSet[j]].Size();l++)
        {
         NodeSample & targ = ds[i]->surfData[segSet[j]][l];
         real32 dist = ds[i]->ied->FitCost(targ.vec,*calc,targ.x,targ.y);
         if (dist<ds[i]->cutoff) ++inlierC;
                           else ++outlierC;
        }
       }
      }
      
     // If an improvment record it...
      if (inlierC>mostInliers)
      {
       mostInliers = inlierC;
       delete ret;
       ret = calc;
       out = outlierC;
      }
      else delete calc;
     
     ++runs;
     nat32 curLimit = nat32(math::Ln(1.0-ransacProb)/math::Ln(1.0-math::Pow<real32>(real32(mostInliers)/real32(dataAvaliable),st.type->Degrees())));
     if (curLimit<runs) break;
    }
  }
  
 // If we havn't fit a surface fit with all data, this matches up witha  prune and as a good
 // fallback if ransac fails...
  if (ret==null<Surface*>())
  {
   // Simply fit all the data avaliable to the surface, using whatever method
   // the surface internally uses...
    for (nat32 i=0;i<ds.Size();i++)
    {
     for (nat32 j=0;j<segSet.Size();j++)
     {
      for (nat32 l=0;l<ds[i]->surfData[segSet[j]].Size();l++)
      {
       NodeSample & targ = ds[i]->surfData[segSet[j]][l];
       
       fitter->Add(ds[i]->ind,targ.x,targ.y,targ.vec);
      }
     }
    }

    ret = st.type->NewSurface();
    if (fitter->Extract(*ret)==false)
    {
     delete ret;
     ret = null<Surface*>();
    }   
  }
  
 // Refine the best fit we have so far, by using inliers for the fit...  
  if (ret)
  {
   // Use the standard outlier prunning approach to refine the initial fitting,
   // regardless of fitting mode (This matches up with ransac nicelly)...    
    for (nat32 k=0;k<maxPruneIter;k++)
    {
     // Fit all the current inliers, keeping count of the outliers so it can
     // be returned...
      nat32 inlierCount = 0;
      fitter->Reset();
      
      for (nat32 i=0;i<ds.Size();i++)
      {
       for (nat32 j=0;j<segSet.Size();j++)
       {
        for (nat32 l=0;l<ds[i]->surfData[segSet[j]].Size();l++)
        {
         NodeSample & targ = ds[i]->surfData[segSet[j]][l];
        
         real32 dist = ds[i]->ied->FitCost(targ.vec,*ret,targ.x,targ.y);
         if (dist<=ds[i]->cutoff)
         {
          fitter->Add(ds[i]->ind,targ.x,targ.y,targ.vec);
          ++inlierCount;
         }
        }
       }
      }

     // Extract the new surface, if theres no change beyond numerical error
     // between the previous version and the version before that (!) break
     // out, otherwise use the new Surface and keep trundling around.
      Surface * fit = st.type->NewSurface();
      if (fitter->Extract(*fit)==false)
      {
       delete fit;
       break;
      }
      if (*fit==*ret)
      {
       delete fit;
       break;
      }
      else
      {
       delete ret;
       ret = fit;
      }
    }
  }


 // Calculate a score for the surface, we use a sum of Blake Zisserman functions of variation
 // from the fitted surface...
  if (ret)
  {
   out = 0.0;   
   for (nat32 i=0;i<ds.Size();i++)
   {
    real32 cop = math::Exp(-math::Sqr(ds[i]->cutoff));
    for (nat32 j=0;j<segSet.Size();j++)
    {
     for (nat32 l=0;l<ds[i]->surfData[segSet[j]].Size();l++)
     {
      NodeSample & targ = ds[i]->surfData[segSet[j]][l];
       
      real32 dist = ds[i]->ied->FitCost(targ.vec,*ret,targ.x,targ.y);
      out += -math::Ln(math::Exp(-math::Sqr(dist))+cop);
     }
    }
   }
  }


 // Clean up...
  delete fitter;
  
 return ret;
}

//------------------------------------------------------------------------------
 };
};
