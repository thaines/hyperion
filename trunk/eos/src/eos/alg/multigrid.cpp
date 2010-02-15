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

#include "eos/alg/multigrid.h"

#include "eos/file/csv.h"

namespace eos
{
 namespace alg
 {
//------------------------------------------------------------------------------
Multigrid2D::Multigrid2D()
:speed(0.5),tolerance(0.001),maxIters(1024)
{}

Multigrid2D::~Multigrid2D()
{}

void Multigrid2D::SetSize(nat32 width,nat32 height,nat32 stencilSize)
{
 nat32 levels = math::Min(math::TopBit(width),math::TopBit(height));


 stencil.Size(levels);
 for (nat32 l=0;l<stencil.Size();l++)
 {
  stencil[l].Size(stencilSize);
  for (nat32 i=0;i<stencil[l].Size();i++)
  {
   stencil[l][i].x = 0;
   stencil[l][i].y = 0;
  }
 }


 nat32 nodeSize = sizeof(Node) + sizeof(real32)*(stencilSize-1);
 data.Size(0); // Needed to make sure all data entrys are ready for construction finishing.
 data.Size(levels);
 
 data[0].FinishCons(nodeSize,width,height);
 for (nat32 y=0;y<data[0].Height();y++)
 {
  for (nat32 x=0;x<data[0].Width();x++)
  {
   data[0].Get(x,y).b = 0.0;
   data[0].Get(x,y).x = 0.0;
   for (nat32 s=0;s<stencil[0].Size();s++) data[0].Get(x,y).a[s] = 0.0;
  }
 }
 
 for (nat32 l=1;l<data.Size();l++)
 {
  nat32 newWidth = (data[l-1].Width()/2) + (((data[l-1].Width()%2)==1)?1:0);
  nat32 newHeight = (data[l-1].Height()/2) + (((data[l-1].Height()%2)==1)?1:0);
  data[l].FinishCons(nodeSize,newWidth,newHeight);
  for (nat32 y=0;y<data[l].Height();y++)
  {
   for (nat32 x=0;x<data[l].Width();x++)
   {
    data[l].Get(x,y).b = 0.0;
    data[l].Get(x,y).x = 0.0;
    for (nat32 s=0;s<stencil[l].Size();s++) data[l].Get(x,y).a[s] = 0.0;
   }
  }  
 }
}

nat32 Multigrid2D::Levels() const
{
 return data.Size();
}

nat32 Multigrid2D::Width(nat32 level) const
{
 return data[level].Width();
}

nat32 Multigrid2D::Height(nat32 level) const
{
 return data[level].Height();
}

void Multigrid2D::SetOffset(nat32 level,nat32 se,int32 offX,int32 offY)
{
 if (se!=0)
 {
  stencil[level][se].x = offX;
  stencil[level][se].y = offY;
 }
}

void Multigrid2D::SpreadFirstStencil()
{
 for (nat32 l=1;l<stencil.Size();l++)
 {
  for (nat32 se=0;se<stencil[l].Size();se++) stencil[l][se] = stencil[0][se];
 }
}

void Multigrid2D::SetB(nat32 level,nat32 x,nat32 y,real32 val)
{
 data[level].Get(x,y).b = val;
}

void Multigrid2D::SetA(nat32 level,nat32 x,nat32 y,nat32 se,real32 val)
{
 int32 posX = int32(x) + stencil[level][se].x;
 if ((posX<0)||(posX>=int32(data[level].Width()))) return;
 
 int32 posY = int32(y) + stencil[level][se].y; 
 if ((posY<0)||(posY>=int32(data[level].Height()))) return;
 
 data[level].Get(x,y).a[se] = val;
}

void Multigrid2D::ZeroA(nat32 level,nat32 x,nat32 y)
{
 for (nat32 se=0;se<stencil[level].Size();se++) data[level].Get(x,y).a[se] = 0.0;
}
  
void Multigrid2D::AddA(nat32 level,nat32 x,nat32 y,nat32 se,real32 val)
{
 int32 posX = int32(x) + stencil[level][se].x;
 if ((posX<0)||(posX>=int32(data[level].Width()))) return;
 
 int32 posY = int32(y) + stencil[level][se].y; 
 if ((posY<0)||(posY>=int32(data[level].Height()))) return;
 
 data[level].Get(x,y).a[se] += val;
}

real32 Multigrid2D::GetA(nat32 level,nat32 x,nat32 y,nat32 se)
{
 int32 posX = int32(x) + stencil[level][se].x;
 if ((posX<0)||(posX>=int32(data[level].Width()))) return 0.0;
 
 int32 posY = int32(y) + stencil[level][se].y; 
 if ((posY<0)||(posY>=int32(data[level].Height()))) return 0.0;
 
 return data[level].Get(x,y).a[se];
}
   
void Multigrid2D::Fix(nat32 level,nat32 x,nat32 y,real32 val)
{
 data[level].Get(x,y).b = val;
 data[level].Get(x,y).a[0] = 1.0;
 for (nat32 se=1;se<stencil[level].Size();se++) data[level].Get(x,y).a[se] = 0.0;
}

void Multigrid2D::SetX(nat32 level,nat32 x,nat32 y,real32 val)
{
 data[level].Get(x,y).x = val;
}

void Multigrid2D::OffsetX(nat32 level,real32 offset)
{
 for (nat32 y=0;y<data[level].Height();y++)
 {
  for (nat32 x=0;x<data[level].Width();x++) data[level].Get(x,y).x += offset;
 }
}

void Multigrid2D::SetSpeed(real32 s)
{
 speed = s;
}

void Multigrid2D::SetIters(real32 t,nat32 mi)
{
 tolerance = t;
 maxIters = mi;
}

void Multigrid2D::Run(time::Progress * prog)
{
 LogTime("eos::alg::Multigrid2D::Run");
 prog->Push();
 nat32 step = 0;
 nat32 steps = math::Sqr(data.Size())*2 - 1;
 
 // Do the first solution at the top of the hierachy...
  prog->Report(step++,steps);
  //LogDebug("(I) Pre residual at level " << (data.Size()-1) << " is " << TotalResidual(data.Size()-1));
  Solve(data.Size()-1,prog);
  //LogDebug("(I) Post residual at level " << (data.Size()-1) << " is " << TotalResidual(data.Size()-1));


 // Do the sequence of V's, one down to and back again from each level but the
 // very top...
  for (int32 lowest=int32(data.Size())-2;lowest>=0;lowest--)
  {
   // Down...
    for (int32 lev=int32(data.Size())-2;lev>=lowest;lev--)
    {
     prog->Report(step++,steps);
     TransferDown(lev,prog);
     
     prog->Report(step++,steps);
     //LogDebug("(" << lowest << "D) Pre residual at level " << lev << " is " << TotalResidual(lev));
     Solve(lev,prog);
     //LogDebug("(" << lowest << "D) Post residual at level " << lev << " is " << TotalResidual(lev));
    }
   
   // Up...  
    for (int32 lev=lowest+1;lev<int32(data.Size());lev++)
    {
     prog->Report(step++,steps);
     TransferUp(lev-1,prog);
     
     prog->Report(step++,steps);
     //LogDebug("(" << lowest << "U) Pre residual at level " << lev << " is " << TotalResidual(lev));
     Solve(lev,prog);
     //LogDebug("(" << lowest << "U) Post residual at level " << lev << " is " << TotalResidual(lev));
    }
  }
 
 
 // Take it back down to the bottom, so we may extract the result...
  for (int32 lev=int32(data.Size())-2;lev>=0;lev--)
  {
   prog->Report(step++,steps);
   TransferDown(lev,prog);
   
   prog->Report(step++,steps);
   //LogDebug("(F) Pre residual at level " << lev << " is " << TotalResidual(lev));
   Solve(lev,prog);
   //LogDebug("(F) Post residual at level " << lev << " is " << TotalResidual(lev));
  }
 
 prog->Pop();
}

void Multigrid2D::ReRun(time::Progress * prog)
{
 LogTime("eos::alg::Multigrid2D::ReRun");
 prog->Push();
 nat32 step = 0;
 nat32 steps = (data.Size()-1)*4 + 1;
 
 // First attempt...
  //LogDebug("Start ReRun");
  prog->Report(step++,steps);
  //LogDebug("(U) Pre residual at level 0 is " << TotalResidual(0));
  Solve(0,prog);  
  //LogDebug("(U) Post residual at level 0 is " << TotalResidual(0));


 // Go up to the top of the hierachy...
  for (int32 l=1;l<int32(data.Size());l++)
  {
   prog->Report(step++,steps);
   TransferUp(l-1,prog);
   
   prog->Report(step++,steps);
   //LogDebug("(U) Pre residual at level " << l << " is " << TotalResidual(l));
   Solve(l,prog);
   //LogDebug("(U) Post residual at level " << l << " is " << TotalResidual(l));
  }

 
 // And all the way back down again...
  for (int32 l=int32(data.Size())-2;l>=0;l--)
  {
   prog->Report(step++,steps);
   TransferDown(l,prog);
   
   prog->Report(step++,steps);
   //LogDebug("(D) Pre residual at level " << l << " is " << TotalResidual(l));
   Solve(l,prog);
   //LogDebug("(D) Post residual at level " << l << " is " << TotalResidual(l));
  }
  
  //LogDebug("End ReRun");
 
 prog->Pop();
}

void Multigrid2D::ZeroMean()
{
 // Calculate the mean, incrimentally for stability...
  real32 mean = 0.0;
  nat32 count = 0;
  for (nat32 y=0;y<data[0].Height();y++)
  {
   for (nat32 x=0;x<data[0].Width();x++)
   {
    count += 1;
    mean += (data[0].Get(x,y).x-mean)/real32(count);
   }
  }
  
 // Apply the offset...
  for (nat32 y=0;y<data[0].Height();y++)
  {
   for (nat32 x=0;x<data[0].Width();x++) data[0].Get(x,y).x -= mean;
  }
}

void Multigrid2D::GetX(ds::Array2D<real32> & out) const
{
 for (nat32 y=0;y<data[0].Height();y++)
 {
  for (nat32 x=0;x<data[0].Width();x++) out.Get(x,y) = data[0].Get(x,y).x;
 }
}

void Multigrid2D::GetX(svt::Field<real32> & out) const
{
 for (nat32 y=0;y<data[0].Height();y++)
 {
  for (nat32 x=0;x<data[0].Width();x++) out.Get(x,y) = data[0].Get(x,y).x;
 }
}

real32 Multigrid2D::Residual(nat32 level,nat32 x,nat32 y)
{
 real32 ret = data[level].Get(x,y).b;
   
 for (nat32 se=0;se<stencil[level].Size();se++)
 {
  if (!math::IsZero(data[level].Get(x,y).a[se]))
  {
   real32 rx = data[level].Get(x+stencil[level][se].x,y+stencil[level][se].y).x;
   ret -= data[level].Get(x,y).a[se] * rx;
  }
 }
 
 return ret;
}

cstrconst Multigrid2D::TypeString() const
{
return "eos::alg::Multigrid2D";
}

//------------------------------------------------------------------------------
void Multigrid2D::TransferUp(nat32 from,time::Progress * prog)
{
 LogTime("eos::alg::Multigrid2D::TransferUp");
 prog->Push();
 // First pass to calculate the residual...
  prog->Report(0,2);
  prog->Push();
  for (nat32 y=0;y<data[from].Height();y++)
  {
   prog->Report(y,data[from].Height());
   for (nat32 x=0;x<data[from].Width();x++)
   {
    data[from].Get(x,y).r = data[from].Get(x,y).b;
    
    for (nat32 se=0;se<stencil[from].Size();se++)
    {
     if (!math::IsZero(data[from].Get(x,y).a[se]))
     {
      real32 rx = data[from].Get(x+stencil[from][se].x,y+stencil[from][se].y).x;
      data[from].Get(x,y).r -= data[from].Get(x,y).a[se] * rx;
     }
    }
   }
  }
  prog->Pop();

 // Second pass to transfer the residual up into the b vector and zero the x's...
  prog->Report(1,2);
  prog->Push();
  for (nat32 y=0;y<data[from+1].Height();y++)
  {
   prog->Report(y,data[from+1].Height());
   for (nat32 x=0;x<data[from+1].Width();x++)
   {
    // Zero the x, which is now an error vector...
     data[from+1].Get(x,y).x = 0.0;
  
    // Calculate the b's as the residual from the previous layer, down sampled...
     nat32 twX = x*2;
     nat32 twY = y*2;
    
     bit incX = (twX+1)<data[from].Width();
     bit decX = twX>0;
     bit incY = (twY+1)<data[from].Height();
     bit decY = twY>0;
     
     if (incX&&decX&&incY&&decY)
     {
      data[from+1].Get(x,y).b = 0.25 * data[from].Get(twX,twY).r;
     
      data[from+1].Get(x,y).b += 0.125 * data[from].Get(twX+1,twY).r;
      data[from+1].Get(x,y).b += 0.125 * data[from].Get(twX-1,twY).r;
      data[from+1].Get(x,y).b += 0.125 * data[from].Get(twX,twY+1).r;
      data[from+1].Get(x,y).b += 0.125 * data[from].Get(twX,twY-1).r;
     
      data[from+1].Get(x,y).b += 0.0625 * data[from].Get(twX+1,twY+1).r;
      data[from+1].Get(x,y).b += 0.0625 * data[from].Get(twX+1,twY-1).r;
      data[from+1].Get(x,y).b += 0.0625 * data[from].Get(twX-1,twY+1).r;
      data[from+1].Get(x,y).b += 0.0625 * data[from].Get(twX-1,twY-1).r;
     }
     else
     {
      data[from+1].Get(x,y).b = data[from].Get(twX,twY).r;
     }
   }
  }
  prog->Pop();
  
 prog->Pop();
}

void Multigrid2D::TransferDown(nat32 to,time::Progress * prog)
{
 LogTime("eos::alg::Multigrid2D::TransferDown");
 prog->Push();
 
 // Below uses simple linear interpolation.
  for (nat32 y=0;y<data[to].Height();y++)
  {
   prog->Report(y,data[to].Height());
   for (nat32 x=0;x<data[to].Width();x++)
   {
    nat32 halfX = x/2;
    nat32 halfY = y/2;
    bit oddX = (x%2)==1;
    bit oddY = (y%2)==1;
   
    real32 offset = 0.0;
     if (oddX)
     {
      if (oddY)
      {
       offset = data[to+1].Get(halfX,halfY).x + data[to+1].ClampGet(halfX+1,halfY).x +
                data[to+1].ClampGet(halfX,halfY+1).x + data[to+1].ClampGet(halfX+1,halfY+1).x;
       offset *= 0.25;
      }
      else
      {
       offset = data[to+1].Get(halfX,halfY).x + data[to+1].ClampGet(halfX+1,halfY).x;
       offset *= 0.5;
      }
     }
     else
     {
      if (oddY)
      {
       offset = data[to+1].Get(halfX,halfY).x + data[to+1].ClampGet(halfX,halfY+1).x;
       offset *= 0.5;
      }
      else
      {
       offset = data[to+1].Get(halfX,halfY).x;
      }
     }
    data[to].Get(x,y).x += offset;
   }
  }
  
 prog->Pop();
}

void Multigrid2D::Solve(nat32 level,time::Progress * prog)
{
 LogTime("eos::alg::Multigrid2D::Solve");
 prog->Push();
 real32 lastChange = 0.0; 
 for (nat32 iter=0;iter<maxIters;iter++)
 {
  prog->Report(iter,maxIters);
  real32 change = 0.0;
  // Symmetric implimentation - direction depends on iter number...
   if ((iter%2)==0)
   {
    for (int32 y=0;y<int32(data[level].Height());y++)
    {
     for (int32 x=0;x<int32(data[level].Width());x++)
     {
      real32 val = 0.0;
      for (nat32 se=1;se<stencil[level].Size();se++)
      {
       if (!math::IsZero(data[level].Get(x,y).a[se]))
       {
        val += data[level].Get(x,y).a[se] * data[level].Get(x+stencil[level][se].x,y+stencil[level][se].y).x;
       }
      }
      
      real32 newX = (data[level].Get(x,y).b - val)/data[level].Get(x,y).a[0];
      if (math::IsFinite(newX))
      {
       change += math::Abs(data[level].Get(x,y).x - newX);
       data[level].Get(x,y).x = newX;
      }
     }
    }
   }
   else
   {
    for (int32 y=int32(data[level].Height())-1;y>=0;y--)
    {
     for (int32 x=int32(data[level].Width())-1;x>=0;x--)
     {
      real32 val = 0.0;
      for (nat32 se=1;se<stencil[level].Size();se++)
      {
       if (!math::IsZero(data[level].Get(x,y).a[se]))
       {
        val += data[level].Get(x,y).a[se] * data[level].Get(x+stencil[level][se].x,y+stencil[level][se].y).x;
       }
      }
      
      real32 newX = (data[level].Get(x,y).b - val)/data[level].Get(x,y).a[0];
      if (math::IsFinite(newX))
      {
       change += math::Abs(data[level].Get(x,y).x - newX);
       data[level].Get(x,y).x = (1.0-speed)*data[level].Get(x,y).x + speed*newX;
      }
     }
    }
   }

  if (change<tolerance) break;
  if (math::Abs(change-lastChange)<(math::Sqr(tolerance)*0.1)) break;
  if (lastChange<change) break;
  lastChange = change;
 }
 prog->Pop();
}

real32 Multigrid2D::TotalResidual(nat32 level)
{
 real32 residual = 0.0;
 for (nat32 y=0;y<data[level].Height();y++)
 {
  for (nat32 x=0;x<data[level].Width();x++)
  {
   real32 diff = data[level].Get(x,y).b;
   
   for (nat32 se=0;se<stencil[level].Size();se++)
   {
    if (!math::IsZero(data[level].Get(x,y).a[se]))
    {
     real32 rx = data[level].Get(x+stencil[level][se].x,y+stencil[level][se].y).x;
     diff -= data[level].Get(x,y).a[se] * rx;
    }
   }
   residual += math::Sqr(diff);
  }
 }
 return math::Pow<real32>(2.0,level)*math::Sqrt(residual);
}

//------------------------------------------------------------------------------
 };
};
