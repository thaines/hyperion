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

#include "eos/inf/bin_bp_2d.h"

#include "eos/file/csv.h"

namespace eos
{
 namespace inf
 {
//------------------------------------------------------------------------------
BinBP2D::BinBP2D()
:tol(1e-3),maxIters(10000),momentum(0.0)
{}

BinBP2D::~BinBP2D()
{}

void BinBP2D::SetSize(nat32 width,nat32 height)
{
 data.Resize(width,height);
 for (nat32 y=0;y<data.Height();y++)
 {
  for (nat32 x=0;x<data.Width();x++)
  {
   Node & targ = data.Get(x,y);
   
   for (nat32 i=0;i<2;i++) targ.cost[i] = 0.0;
   for (nat32 i=0;i<2;i++)
   {
    for (nat32 j=0;j<2;j++)
    {
     targ.incX[i][j] = 0.0;
     targ.incY[i][j] = 0.0;
    }
   }
   for (nat32 i=0;i<4;i++)
   {
    for (nat32 j=0;j<2;j++) targ.in[i][j] = 0.0;
   }
   targ.dual = true;
   targ.result = false;
  }
 }
}

void BinBP2D::Disable(nat32 x,nat32 y)
{
 data.Get(x,y).dual = false;
}

void BinBP2D::SetCost(nat32 x,nat32 y,bit state,real32 cost)
{
 data.Get(x,y).cost[state?1:0] = cost;
}

void BinBP2D::SetCostX(nat32 x,nat32 y,bit givenState,bit otherState,real32 cost)
{
 data.Get(x,y).incX[givenState?1:0][otherState?1:0] = cost;
}

void BinBP2D::SetCostY(nat32 x,nat32 y,bit givenState,bit otherState,real32 cost)
{
 data.Get(x,y).incY[givenState?1:0][otherState?1:0] = cost;
}

void BinBP2D::SetMomentum(real32 mom)
{
 momentum = mom;
}

void BinBP2D::SetEnd(real32 t,nat32 mi)
{
 tol = t;
 maxIters = mi;
}

void BinBP2D::Run(time::Progress * prog)
{
 prog->Push();
 
 // Run through message updates until the change is tiny or the maximum number
 // of iterations is reached...
  static const nat32 settleLength = 4;
  nat32 ns[settleLength];
  nat32 probSize = data.Width()*data.Height();
  for (nat32 i=0;i<settleLength;i++) ns[i] = probSize/2;
  
  for (nat32 iter=0;iter<maxIters;iter++)
  {
   {
    nat32 setCount = ns[(iter+3)%4] + ns[(iter+2)%4];
    real32 toSettle = math::Ln(1.0 + 99.0*real32(setCount)/real32(probSize))/math::Ln(100.0);
    prog->Report(math::Max(iter,maxIters-nat32(toSettle*maxIters)),maxIters+1);
   }
   
   nat32 notSettled = 0;
   for (nat32 y=0;y<data.Height();y++)
   {
    for (nat32 x=(iter+y)%2;x<data.Width();x+=2) // Checkerboard update pattern.
    {
     // Get the node, skip if its not simulated, calculate the cost sum...
      Node & targ = data.Get(x,y);
      if (targ.dual==false) continue;
      
      real32 baseCost[2];
      for (nat32 i=0;i<2;i++) baseCost[i] = targ.cost[i];
      for (nat32 i=0;i<4;i++)
      {
       for (nat32 j=0;j<2;j++) baseCost[j] += targ.in[i][j];
      }
     
     // Send messages in all 4 directions, assuming such messages should be sent...
      bit change = false;
      for (nat32 d=0;d<4;d++)
      {
       // Check its safe...
        bit safe = true;
        switch(d)
        {
         case 0: safe = x<(data.Width()-1); break; // +ve x
         case 1: safe = y<(data.Height()-1); break; // +ve y
         case 2: safe = x>0; break; // -ve x
         case 3: safe = y>0; break; // -ve y
        }
        if (!safe) continue;
       
       // Get a pointer to the output data (An array of two values)...
        real32 * out = null<real32*>();
        switch(d)
        {
         case 0: out = data.Get(x+1,y).in[2]; break;
         case 1: out = data.Get(x,y+1).in[3]; break;
         case 2: out = data.Get(x-1,y).in[0]; break;
         case 3: out = data.Get(x,y-1).in[1]; break;
        }
       
       // Get a pointer and flip variable for the relevant probability table...
        real32 (*pt)[2][2] = 0;
        bit flip = false;
        switch(d)
        {
         case 0: pt = &targ.incX; flip = false; break;
         case 1: pt = &targ.incY; flip = false; break;
         case 2: pt = &data.Get(x-1,y).incX; flip = true; break;
         case 3: pt = &data.Get(x,y-1).incY; flip = true; break;
        }
       
       // Calculate the starting costs...
        real32 startCost[2];
        for (nat32 i=0;i<2;i++) startCost[i] = baseCost[i] - targ.in[d][i];
        
       // Fill in the output...
        real32 outTemp[2];
        for (nat32 i=0;i<2;i++)
        {
         outTemp[i] = math::Min((*pt)[flip?i:0][flip?0:i] + startCost[0],
                                (*pt)[flip?i:1][flip?1:i] + startCost[1]);
        }
        
        real32 msgMin = math::Min(outTemp[0],outTemp[1]);
        for (nat32 i=0;i<2;i++)
        {
         outTemp[i] -= msgMin;
         change |= math::Abs(out[i]-outTemp[i])>tol;
         out[i] = momentum*out[i] + (1.0-momentum)*outTemp[i];
        }
      }
      if (change) ++notSettled;
    }
   }
   
   // Decide if to break early due to nothing changing much...
   {
    ns[iter%settleLength] = notSettled;
    bit br = true;
    for (nat32 i=0;i<settleLength;i++) {if (ns[i]!=0) br = false;}
    if (br) break;
   }
  }
 
 
 // Extract the results to set the result variable...
  prog->Report(maxIters,maxIters+1);
  for (nat32 y=0;y<data.Height();y++)
  {
   for (nat32 x=0;x<data.Width();x++)
   {
    Node & targ = data.Get(x,y);
   
    real32 cost[2];
    for (nat32 i=0;i<2;i++) cost[i] = targ.cost[i];
    for (nat32 i=0;i<4;i++)
    {
     for (nat32 j=0;j<2;j++) cost[j] += targ.in[i][j];
    }
    
    targ.result = cost[0]>cost[1];
   }
  }


 prog->Pop();
}

bit BinBP2D::State(nat32 x,nat32 y) const
{
 return data.Get(x,y).result;
}

void BinBP2D::Result(ds::Array2D<bit> & out) const
{
 out.Resize(data.Width(),data.Height());
 for (nat32 y=0;y<out.Height();y++)
 {
  for (nat32 x=0;x<out.Width();x++)
  {
   out.Get(x,y) = data.Get(x,y).result;
  }
 }
}

//------------------------------------------------------------------------------
 };
};
