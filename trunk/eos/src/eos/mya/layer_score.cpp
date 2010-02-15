//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/mya/layer_score.h"

namespace eos
{
 namespace mya
 {
//------------------------------------------------------------------------------
LayerScore::LayerScore(Layers & l)
:layers(l)
{}

LayerScore::~LayerScore()
{}

void LayerScore::OnSetSeglayer(nat32,nat32)
{}

void LayerScore::OnMergeLayers(nat32,nat32)
{}

void LayerScore::OnSeperate(nat32)
{}

void LayerScore::OnRebuild()
{}

//------------------------------------------------------------------------------
OutlierScore::OutlierScore(Layers & layers)
:LayerScore(layers),foi(layers.SegmentCount())
{}

OutlierScore::~OutlierScore()
{}

real32 OutlierScore::IfSetSegLayer(nat32 seg,nat32 layer) const
{
 real32 oldScore = layers.FitCost(seg) + layers.FitCost(layer);

 layers.GetLayerFlags(seg,foi);
 foi[seg] = false;
 real32 newScore = layers.LayerFitCost(foi);

 layers.GetLayerFlags(layer,foi);
 foi[seg] = true;
 newScore += layers.LayerFitCost(foi);

 return newScore-oldScore;
}

real32 OutlierScore::IfMergeLayers(nat32 lay1,nat32 lay2) const
{
 real32 oldScore = layers.FitCost(lay1) + layers.FitCost(lay2);
 
 layers.GetLayerFlags(lay1,foi,true);
 layers.GetLayerFlags(lay2,foi,false); 
 real32 newScore = layers.LayerFitCost(foi);
 
 return newScore-oldScore;
}

real32 OutlierScore::IfSeperate(nat32 seg) const
{
 real32 oldScore = layers.FitCost(seg);
 
 layers.GetLayerFlags(seg,foi);
 foi[seg] = false;
 real32 newScore = layers.LayerFitCost(foi) + layers.SegFitCost(seg);
  
 return newScore-oldScore;
}

cstrconst OutlierScore::TypeString() const
{
 return "eos::mya::OutlierScore";
}

//------------------------------------------------------------------------------
WarpScore::WarpScore(Layers & layers,nat32 segC,const svt::Field<nat32> & s,const svt::Field<bs::ColourRGB> & l,const svt::Field<bs::ColourRGB> & r)
:LayerScore(layers),warp(r,segC),
segCount(segC),segs(s),
left(l),right(r),mult(1.0),
foiA(segC),foiB(segC),foiC(segC)
{
 // Build the segment information cache...
  sed = new Node*[segCount];
  for (nat32 i=0;i<segCount;i++) sed[i] = null<Node*>();

  for (nat32 y=0;y<left.Size(1);y++)
  {
   for (int32 x=left.Size(0)-1;x>=0;x--)
   {
    Node * nn = new Node;

    nat32 s = segs.Get(x,y);
    nn->next = sed[s];
    sed[s] = nn;

    nn->x = x;
    nn->y = y;
    nn->colour = left.Get(x,y);
    nn->paired = (nn->next!=null<Node*>())&&(nn->next->x==nat32(x+1))&&(nn->next->y==y);
    if (nn->paired)
    {
     nn->incStart = true;
     nn->next->incStart = false;
     if (nn->next->paired) nn->incEnd = false;
                      else nn->incEnd = true;
    }
   }
  }


 // Render out the initial warping...
  warp.Weights(20.0/255.0);
  for (nat32 i=0;i<segCount;i++) AddSeg(i,layers.LayerToSurf(i)); 
}

WarpScore::~WarpScore()
{
 // Terminate the cache...
  for (nat32 i=0;i<segCount;i++)
  {
   Node * targ = sed[i];
   while (targ)
   {
    Node * victim = targ;
    targ = targ->next;
    delete victim;
   }
  }
  delete[] sed;
}

void WarpScore::SetOcclusionCost(real32 cost)
{
 warp.Weights(cost);
}

void WarpScore::SetDisparityMult(real32 m)
{
 mult = m;
}

real32 WarpScore::IfSetSegLayer(nat32 seg,nat32 layer) const
{
 // Apply the change...
  // Remove the segments that are members of the involved layers...
   layers.GetLayerFlags(seg,foiA);
   layers.GetLayerFlags(layer,foiB);
   for (nat32 i=0;i<segCount;i++) if (foiA[i]||foiB[i]) warp.Remove(i);
  
  // Re-fit the layers in question to the new surfaces...
   foiA[seg] = false;
   foiB[seg] = true;

   real32 discarded;
   Surface * nsSeg   = layers.LayerFit(foiA,discarded);
   Surface * nsLayer = layers.LayerFit(foiB,discarded);
  
  // Add back the segments with the new surfaces...
   for (nat32 i=0;i<segCount;i++)
   {
    if (foiA[i]) AddSeg(i,nsSeg);
    else if (foiB[i]) AddSeg(i,nsLayer);
   }

 // Record the score after change...
  real32 nScore = warp.Score();

 // Remove the change...
  // Remove the involved segments...
   for (nat32 i=0;i<segCount;i++) if (foiA[i]||foiB[i]) warp.Remove(i);
 
  // Adjust variables...
   foiA[seg] = true;
   foiB[seg] = false;

   nsSeg   = layers.LayerToSurf(seg);
   nsLayer = layers.LayerToSurf(layer);

  // Write back involved segments...
   for (nat32 i=0;i<segCount;i++)
   {
    if (foiA[i]) AddSeg(i,nsSeg);
    else if (foiB[i]) AddSeg(i,nsLayer);
   }   
 
 // Return the delta...
  return nScore - warp.Score();
}

real32 WarpScore::IfMergeLayers(nat32 lay1,nat32 lay2) const
{
 // Apply the change...
  // Remove the segments that are members of the involved layers...
   layers.GetLayerFlags(lay1,foiA);
   layers.GetLayerFlags(lay2,foiB);
   for (nat32 i=0;i<segCount;i++)
   {
    if (foiA[i]||foiB[i]) {warp.Remove(i); foiC[i] = true;}
                     else foiC[i] = false;
   }
   
  // Calculate the new surface and apply it...
   real32 discarded;
   Surface * mSurf = layers.LayerFit(foiC,discarded);
   for (nat32 i=0;i<segCount;i++) if (foiC[i]) AddSeg(i,mSurf);
 
 // Record the score after change...
  real32 nScore = warp.Score(); 
    
 // Remove the change...
  Surface * osLay1 = layers.LayerToSurf(lay1);
  Surface * osLay2 = layers.LayerToSurf(lay2);
  
  for (nat32 i=0;i<segCount;i++)
  {
   if (foiA[i]) AddSeg(i,osLay1);
   else if (foiB[i]) AddSeg(i,osLay2);
  }  
 
 // Return the delta...
  return nScore - warp.Score();
}

real32 WarpScore::IfSeperate(nat32 seg) const
{
 // Apply the change...
  // Remove the layer in question...
   layers.GetLayerFlags(seg,foiA);
   for (nat32 i=0;i<segCount;i++) if (foiA[i]) warp.Remove(i);
 
  // Fit the relevent layers to new surfaces...
   foiA[seg] = false;
   
   real32 discarded;
   Surface * nsSeg   = layers.SegFit(seg,discarded);
   Surface * nsLayer = layers.LayerFit(foiA,discarded);  
   
  // Add back the segments with the new surfaces...
   for (nat32 i=0;i<segCount;i++) if (foiA[i]) AddSeg(i,nsLayer);
   AddSeg(seg,nsSeg);
 
 // Record the score after change...
  real32 nScore = warp.Score(); 
    
 // Remove the change...
  foiA[seg] = true;
  nsLayer = layers.LayerToSurf(seg);
  for (nat32 i=0;i<segCount;i++) if (foiA[i]) AddSeg(i,nsLayer);
 
 // Return the delta...
  return nScore - warp.Score();
}

void WarpScore::OnSetSeglayer(nat32 seg,nat32 layer)
{
 // Remove current associated segments...
  layers.GetLayerFlags(seg,foiA);
  for (nat32 i=0;i<segCount;i++) if (foiA[i]) warp.Remove(i);
   
 // Apply the new arrangment...
  Surface * nsLayer = layers.LayerToSurf(seg);
  for (nat32 i=0;i<segCount;i++) if (foiA[i]) AddSeg(i,nsLayer);
}

void WarpScore::OnMergeLayers(nat32 lay1,nat32 lay2)
{
 // Remove current associated segments...
  layers.GetLayerFlags(lay1,foiA);
  for (nat32 i=0;i<segCount;i++) if (foiA[i]) warp.Remove(i);
   
 // Apply the new arrangment...
  Surface * nsLayer = layers.LayerToSurf(lay1);
  for (nat32 i=0;i<segCount;i++) if (foiA[i]) AddSeg(i,nsLayer);
  
  
 // Tempory, for testing... save out warp image, with current score in filename...
  
}

void WarpScore::OnSeperate(nat32 seg)
{
 // The below code is wrong due to the limitations of the framework, it does
 // make a change however which should result in a reasonable improvment...
  warp.Remove(seg);
  AddSeg(seg,layers.LayerToSurf(seg));
}

void WarpScore::OnRebuild()
{}

cstrconst WarpScore::TypeString() const
{
 return "eos::mya::WarpScore";
}

// Little helper function for below two methods...
real32 SurfaceDisp(Surface * surface,real32 mult,real32 x,real32 y)
{
 math::Vect<2> z;
 surface->Get(x,y,z);
 return mult*z[1]/z[0];
}

void WarpScore::AddSeg(nat32 seg,Surface * surface) const
{
 if (surface==null<Surface*>()) return;
 
 Node * targ = sed[seg];
 nat32 width = warp.Width();
 while (targ)
 {
  if (targ->paired)
  {
   // Find the start and end points...
    real32 start = targ->x + SurfaceDisp(surface,mult,targ->x,targ->y);
    real32 end = targ->x + 1.0 + SurfaceDisp(surface,mult,targ->x+1,targ->y);
    
    real32 mult = 1.0/(end-start);
    real32 base = start;
    
    if (targ->incStart) start = targ->x-0.5 + SurfaceDisp(surface,mult,targ->x-0.5,targ->y);
    if (targ->incEnd) end = targ->x + 1.5 + SurfaceDisp(surface,mult,targ->x+1.5,targ->y);
    if (start>end) {real32 temp = start; start = end; end = temp;}
        
   // For each integer in the range interpolate a colour and write...
    for (nat32 i=math::Max<int32>(int32(math::RoundUp(start)),0);(real32(i)<end)&&(i<width);i++)
    {
     real32 t = (real32(i)-base)*mult;
     bs::ColourRGB colour;
      colour.r = (1.0-t)*targ->colour.r + t*targ->next->colour.r;
      colour.g = (1.0-t)*targ->colour.g + t*targ->next->colour.g;
      colour.b = (1.0-t)*targ->colour.b + t*targ->next->colour.b;
     
     warp.Add(i,targ->y,SurfaceDisp(surface,mult,targ->x+t,targ->y),colour,seg);
    }
  }
  targ = targ->next;	 
 }
}

//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
 };
};
