//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

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

#include "eos/filter/seg_k_mean_grid.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
MeanGridSeg::MeanGridSeg()
:dim(12),minSize(32),colMult(2.0),spatialMult(1.0),maxIters(1000),segments(0)
{}

MeanGridSeg::~MeanGridSeg()
{}

void MeanGridSeg::SetImage(const svt::Field<bs::ColourLuv> & in)
{
 image = in;
}

void MeanGridSeg::SetSize(nat32 d)
{
 dim = d;
}

void MeanGridSeg::SetMinSeg(nat32 size)
{
 minSize = size;
}

void MeanGridSeg::SetDist(real32 colM,real32 spatialM)
{
 colMult = colM;
 spatialMult = spatialM;
}

void MeanGridSeg::SetMaxIters(nat32 iters)
{
 maxIters = iters;
}

void MeanGridSeg::Run(time::Progress * prog)
{
 prog->Push();

 // Prep storage...
  prog->Report(0,104);
  ds::Array2D<nat32> a(image.Size(0),image.Size(1));
  ds::Array2D<nat32> b(image.Size(0),image.Size(1));
  
  ds::Array2D<nat32> * current = &a;
  ds::Array2D<nat32> * old = &b;
  
  nat32 gridWidth  = ((image.Size(0)-1)/dim)+1;
  nat32 gridHeight = ((image.Size(1)-1)/dim)+1;
  ds::Array<Mean> mean(gridWidth*gridHeight);


 // Create the initial state...
  prog->Report(1,104);
  for (nat32 y=0;y<current->Height();y++)
  {
   for (nat32 x=0;x<current->Width();x++)
   {
    nat32 gx = x/dim;
    nat32 gy = y/dim;
    current->Get(x,y) = gy*gridWidth + gx;
   }
  }


 // Iterate till convergance, or we run out of iterations to converge with...
  prog->Report(2,104);
  nat32 maxChanges = 1;
  for (nat32 iter=0;iter<maxIters;iter++)
  {
   CalcMeans(*current,mean);
   nat32 changes = UpdatePixels(*current,*old,mean);
   maxChanges = math::Max(maxChanges,changes);
   math::Swap(current,old);
   if (changes==0) break;
   
   nat32 comp = nat32(100.0*math::Ln(1.0+changes)/math::Ln(1.0+maxChanges));
   prog->Report(102-comp,104);
  }
  
  CalcMeans(*current,mean); // Needed for if changes!=0 in the last run.


 // Remove segments which have been emptied...
  prog->Report(102,104);
  ds::Array<nat32> map(mean.Size());
  segments = 0;
  for (nat32 i=0;i<mean.Size();i++)
  {
   map[i] = segments;
   if (mean[i].samples!=0) segments += 1;
  }
 
 
 // Store the final data...
  prog->Report(103,104);
  out.Resize(image.Size(0),image.Size(1));
  for (nat32 y=0;y<out.Height();y++)
  {
   for (nat32 x=0;x<out.Width();x++)
   {
    out.Get(x,y) = map[current->Get(x,y)];
   }
  }
 
 
 prog->Pop();
}

nat32 MeanGridSeg::Segments() const
{
 return segments;
}

void MeanGridSeg::GetSegments(svt::Field<nat32> & o) const
{
 for (nat32 y=0;y<o.Size(1);y++)
 {
  for (nat32 x=0;x<o.Size(0);x++) o.Get(x,y) = out.Get(x,y);
 }
}

//------------------------------------------------------------------------------
void MeanGridSeg::CalcMeans(const ds::Array2D<nat32> & seg,ds::Array<Mean> & mean)
{
 // Go through and zero out the means...
  for (nat32 i=0;i<mean.Size();i++)
  {
   mean[i].samples = 0;
   mean[i].x = 0.0;
   mean[i].y = 0.0;
   mean[i].l = 0.0;
   mean[i].u = 0.0;
   mean[i].v = 0.0;
  }
  
 // Iterate through all pixels and sum in their affect to the means...
  for (nat32 y=0;y<seg.Height();y++)
  {
   for (nat32 x=0;x<seg.Width();x++)
   {
    nat32 i = seg.Get(x,y);
    const bs::ColourLuv & col = image.Get(x,y);
    
    mean[i].samples += 1;
    real32 weight = 1.0/real32(mean[i].samples);
    
    mean[i].x += weight * (real32(x)-mean[i].x);
    mean[i].y += weight * (real32(y)-mean[i].y);
    mean[i].l += weight * (col.l-mean[i].l);
    mean[i].u += weight * (col.u-mean[i].u);
    mean[i].v += weight * (col.v-mean[i].v);
   }
  }
}

nat32 MeanGridSeg::UpdatePixels(const ds::Array2D<nat32> & oldSeg, ds::Array2D<nat32> & newSeg,
                                const ds::Array<Mean> & mean)
{
 nat32 changes = 0;
 for (nat32 y=0;y<oldSeg.Height();y++)
 {
  for (nat32 x=0;x<oldSeg.Width();x++)
  {
   nat32 cur = oldSeg.Get(x,y);
   nat32 nx = (x!=0)?oldSeg.Get(x-1,y):cur;
   nat32 px = (x+1!=oldSeg.Width())?oldSeg.Get(x+1,y):cur;
   nat32 ny = (y!=0)?oldSeg.Get(x,y-1):cur;
   nat32 py = (y+1!=oldSeg.Height())?oldSeg.Get(x,y+1):cur;
   
   if ((cur!=nx)||(cur!=px)||(cur!=ny)||(cur!=py))
   {
    real32 best = DistSqr(mean[cur],x,y);
    nat32 bestSeg = cur;
    
    if (nx!=cur)
    {
     real32 cost = DistSqr(mean[nx],x,y);
     if (cost<best)
     {
      best = cost;
      bestSeg = nx;
     }
    }
    
    if (px!=cur)
    {
     real32 cost = DistSqr(mean[px],x,y);
     if (cost<best)
     {
      best = cost;
      bestSeg = px;
     }
    }
    
    if (ny!=cur)
    {
     real32 cost = DistSqr(mean[ny],x,y);
     if (cost<best)
     {
      best = cost;
      bestSeg = ny;
     }
    }
    
    if (py!=cur)
    {
     real32 cost = DistSqr(mean[py],x,y);
     if (cost<best)
     {
      best = cost;
      bestSeg = py;
     }
    }
    
    if (bestSeg!=cur) changes += 1;
    newSeg.Get(x,y) = bestSeg;
   }
   else
   {
    newSeg.Get(x,y) = oldSeg.Get(x,y);
   }
  }
 }
 return changes;
}

//------------------------------------------------------------------------------
 };
};
