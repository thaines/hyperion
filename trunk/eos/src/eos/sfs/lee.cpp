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

#include "eos/sfs/lee.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
Lee::Lee()
:iters(16),initRad(25.0),startLambda(2000.0),endLambda(0.0),
multiIters(64),tolerance(0.001),speed(0.5),
toLight(0.01,0.01,1.0)
{
 toLight.Normalise();
}

Lee::~Lee()
{}

void Lee::SetImage(const svt::Field<real32> & i)
{
 image = i;
}

void Lee::SetAlbedo(const svt::Field<real32> & a)
{
 albedo = a;
}

void Lee::SetLight(bs::Normal & norm)
{
 toLight = norm;
 if (math::IsZero(toLight[0])&&math::IsZero(toLight[1]))
 {
  toLight[0] = 0.01;
  toLight[1] = 0.01;
 }
 toLight.Normalise();
}

void Lee::SetParas(nat32 i,real32 ir,real32 sl,real32 el,
                   nat32 mi,real32 t,real32 s)
{
 iters = i;
 initRad = ir;
 startLambda = sl;
 endLambda = el;
 multiIters = mi;
 tolerance = t;
 speed = s;
}

void Lee::Run(time::Progress * prog)
{
 LogTime("eos::sfs::Lee::Run");
 prog->Push();
 
 // Create the albedo corrected image...
  ds::Array2D<real32> irr(image.Size(0),image.Size(1));
  for (nat32 y=0;y<irr.Height();y++)
  {
   for (nat32 x=0;x<irr.Width();x++)
   {
    irr.Get(x,y) = math::Min(image.Get(x,y) / albedo.Get(x,y),real32(1.0));
   }
  }  

 
 // Setup the multigrid object...
  alg::Multigrid2D mg;
  mg.SetSize(irr.Width()+1,irr.Height()+1,13);
  mg.SetOffset(0, 0, 0, 0);
  mg.SetOffset(0, 1, 1, 0);
  mg.SetOffset(0, 2, 0, 1);
  mg.SetOffset(0, 3,-1, 0);
  mg.SetOffset(0, 4, 0,-1);
  mg.SetOffset(0, 5, 2, 0);
  mg.SetOffset(0, 6, 1, 1);
  mg.SetOffset(0, 7, 0, 2);
  mg.SetOffset(0, 8,-1, 1);
  mg.SetOffset(0, 9,-2, 0);
  mg.SetOffset(0,10,-1,-1);
  mg.SetOffset(0,11, 0,-2);
  mg.SetOffset(0,12, 1,-1);
  mg.SpreadFirstStencil();
  mg.SetSpeed(speed);
  mg.SetIters(tolerance,multiIters);
  
  

 // Fill the multigrid with a sphere starting shape, to push the solution to a convex solution...
 // (Corners need to be at depth 0.)
 {
  real32 cx = mg.Width(0)*0.5;
  real32 cy = mg.Height(0)*0.5;
  real32 radius = math::Sqrt(math::Sqr(cx) + math::Sqr(cy));
  for (nat32 y=0;y<mg.Height(0);y++)
  {
   for (nat32 x=0;x<mg.Width(0);x++)
   {
    real32 dist = math::Sqrt(math::Sqr(x-cx) + math::Sqr(y-cy))/radius;
    mg.SetX(0,x,y,initRad*math::Cos(dist*math::pi*0.5));
   }
  }
 }
  
  
  
 // Tempory testing code - creates a perfect sphere as the input, and sets the 
 // output to be the answer - test to see if it then moves from the answer to
 // something else...  
 // (Setup irr and bottom level of x's in multigrid.)
 /*{
  real32 cx = real32(irr.Width())*0.5;
  real32 cy = real32(irr.Height())*0.5;
  real32 r = real32(irr.Height())*0.4;
  
  // Iterate and colour in pixels...
   for (nat32 y=0;y<irr.Height();y++)
   {
    for (nat32 x=0;x<irr.Width();x++)
    {
     // Work out sphere coordinates of pixel...
      real32 px = real32(x)+0.5;
      real32 py = real32(y)+0.5;
      px -= cx;
      py -= cy;
      px /= r;
      py /= r;
      
     // Calculate vector if possible...
      if ((math::Sqr(px) + math::Sqr(py))<=1.0)
      {
       bs::Normal orient;
       orient[0] = px;
       orient[1] = py;
       orient[2] = math::Sqrt(1.0-math::Sqr(px)-math::Sqr(py));
      
       irr.Get(x,y) = math::Max<real32>(orient * toLight,0.0);
      }
      else
      {
       irr.Get(x,y) = 0.0;
      }
    }
   }
  
  // Iterate and fill in depth map...
   for (nat32 y=0;y<mg.Height(0);y++)
   {
    for (nat32 x=0;x<mg.Width(0);x++)
    {
     // Work out sphere coordinates of node...
      real32 px = real32(x);
      real32 py = real32(y);
      px -= cx;
      py -= cy;
      px /= r;
      py /= r;
      
     // Calculate vector if possible...
      if ((math::Sqr(px) + math::Sqr(py))<=1.0)
      {
       bs::Normal orient;
       orient[0] = px;
       orient[1] = py;
       orient[2] = math::Sqrt(1.0-math::Sqr(px)-math::Sqr(py));
      
       mg.SetX(0,x,y,r*orient[2] - r);
      }
      else
      {
       mg.SetX(0,x,y,0.0);
      }
    }
   }
 
 }*/


/*nat32 iters = 8;
nat32 div = 3;
prog->Report(0,iters+1);
PrepMultigrid(irr,mg,startLambda,prog);   
for (nat32 i=0;i<iters;i++)
{
 prog->Report(i+1,iters+1);
 for (nat32 l=0;l<mg.Levels();l++)
 {
  LogDebug("residual pre(" << l << ") = " << mg.Residual(l,mg.Width(l)/div,mg.Height(l)/div));
 }
 mg.ReRun(prog);
 for (nat32 l=0;l<mg.Levels();l++)
 {
  LogDebug("residual post(" << l << ") = " << mg.Residual(l,mg.Width(l)/div,mg.Height(l)/div));
 }
}*/

 // Prep storage needed by the prep method, to stop memory thrashing...
  ds::ArrayDel< ds::Array2D<Site> > data(mg.Levels());
  for (nat32 l=0;l<mg.Levels();l++) data[l].Resize(mg.Width(l),mg.Height(l));

 // Loop through solving it all...
  for (nat32 iter=0;iter<iters;iter++)
  {
   prog->Report(iter,iters);
   
   real32 lambda = ((1.0 - (real32(iter)/real32(iters-1))) * (startLambda-endLambda)) + endLambda;
   nat32 runs = 1;
   //if (iter==0) runs = 10;
   //else if (iter==1) runs = 2;
   
   prog->Push();
   prog->Report(0,1+runs);
   PrepMultigrid(data,irr,mg,lambda,prog);   
   
   for (nat32 r=0;r<runs;r++)
   {
    prog->Report(1+r,1+runs);  
    mg.ReRun(prog);
    
    // Set the centre pixel to be a depth of zero, offsetting all depths accordingly...
     //mg.OffsetX(0,-mg.Get(mg.Width(0)/2,mg.Height(0)/2));
   }
   prog->Pop();
  }


 // Extract the result...
  result.Resize(mg.Width(0),mg.Height(0));
  mg.GetX(result);
  
 
 prog->Pop();
}

void Lee::GetNeedle(svt::Field<bs::Normal> & out) const
{
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++)
  {
   out.Get(x,y)[0] = result.Get(x,y) - result.Get(x+1,y);
   out.Get(x,y)[1] = result.Get(x,y) - result.Get(x,y+1);
   out.Get(x,y)[2] = 1.0;
   out.Get(x,y).Normalise();
  }
 }
}

void Lee::GetDepth(svt::Field<real32> & out) const
{
 for (nat32 y=0;y<out.Size(1);y++)
 {
  for (nat32 x=0;x<out.Size(0);x++)
  {
   out.Get(x,y) = 0.25*(result.Get(x,y) + result.Get(x+1,y) + result.Get(x,y+1) + result.Get(x+1,y+1));
  }
 }
}

cstrconst Lee::TypeString() const
{
 return "eos::sfs::Lee";
}

//------------------------------------------------------------------------------
void Lee::PrepMultigrid(ds::ArrayDel< ds::Array2D<Site> > & data,ds::Array2D<real32> & irr,
                        alg::Multigrid2D & mg,real32 lambda,time::Progress * prog)
{
 LogTime("eos::sfs::Lee::PrepMultigrid");
 prog->Push();

 // Extract from the multigrid depth maps for each level (Zero mean it to stop drift)...
  prog->Report(0,5);
  prog->Push();
  prog->Report(0,data.Size());
  // Level 0...
   prog->Push();
   mg.ZeroMean();
   for (nat32 y=0;y<data[0].Height();y++)
   {
    prog->Report(y,data[0].Height());
    for (nat32 x=0;x<data[0].Width();x++) data[0].Get(x,y).z = mg.Get(x,y);
   }
   prog->Pop();

  
  // Calculate further levels till hierachy full...
  // (Have to use the same techneque used by the multigrid method.)
   for (nat32 l=1;l<data.Size();l++)
   {
    prog->Report(l,data.Size());
    prog->Push();
    for (nat32 y=0;y<data[l].Height();y++)
    {
     prog->Report(y,data[l].Height());
     for (nat32 x=0;x<data[l].Width();x++)
     {
      nat32 twX = x*2;
      nat32 twY = y*2;
    
      bit incX = (twX+1)<data[l-1].Width();
      bit decX = twX>0;
      bit incY = (twY+1)<data[l-1].Height();
      bit decY = twY>0;
      
      real32 div = 0.25;
      data[l].Get(x,y).z = 0.25 * data[l-1].Get(twX,twY).z;
      
      if (incX) {data[l].Get(x,y).z += 0.125 * data[l-1].Get(twX+1,twY).z; div += 0.125;}
      if (decX) {data[l].Get(x,y).z += 0.125 * data[l-1].Get(twX-1,twY).z; div += 0.125;}
      if (incY) {data[l].Get(x,y).z += 0.125 * data[l-1].Get(twX,twY+1).z; div += 0.125;}
      if (decY) {data[l].Get(x,y).z += 0.125 * data[l-1].Get(twX,twY-1).z; div += 0.125;}
     
      if (incX&&incY) {data[l].Get(x,y).z += 0.0625 * data[l-1].Get(twX+1,twY+1).z; div += 0.0625;}
      if (incX&&decY) {data[l].Get(x,y).z += 0.0625 * data[l-1].Get(twX+1,twY-1).z; div += 0.0625;}
      if (decX&&incY) {data[l].Get(x,y).z += 0.0625 * data[l-1].Get(twX-1,twY+1).z; div += 0.0625;}
      if (decX&&decY) {data[l].Get(x,y).z += 0.0625 * data[l-1].Get(twX-1,twY-1).z; div += 0.0625;}
     
      data[l].Get(x,y).z /= div;
     }
    }
    prog->Pop();
   }
   prog->Pop();


 // Iterate and calculate the differentials, for each level... 
  prog->Report(1,5);
  prog->Push();
  for (nat32 l=0;l<data.Size();l++)
  {
   prog->Report(l,data.Size());
   prog->Push();
   real32 levelDiv = math::Pow<real32>(2,l);
   for (nat32 y=0;y<data[l].Height();y++)
   {
    prog->Report(y,data[l].Height());
    for (nat32 x=0;x<data[l].Width();x++)
    {
     // Get safety information for the 4 directions...
      bit xInc = (x+1)<data[l].Width();
      bit xDec = x>0;
      bit yInc = (y+1)<data[l].Height();
      bit yDec = y>0;
      
     // Go through and calculate the value for each of the 6 triangles that use
     // the node, where the triangles exist that is...  
     // Aditionally, zero out differentials when the pixel is black, to indicate
     // any solution is allowed.
      // 0 - (x,y),(x-1,y),(x-1,y-1)...
       if (xDec&&yDec&&(!math::IsZero(irr.Get(x-1,y-1))))
       {
        real32 p = (data[l].Get(x,y).z - data[l].Get(x-1,y).z) / levelDiv;
        real32 q = (data[l].Get(x-1,y).z - data[l].Get(x-1,y-1).z) / levelDiv;
        data[l].Get(x,y).dt[0] = RefDp(p,q);
       }
       else data[l].Get(x,y).dt[0] = 0.0;
        
      // 1 - (x,y),(x-1,y-1),(x,y-1)...
       if (xDec&&yDec&&(!math::IsZero(irr.Get(x-1,y-1))))
       {
        real32 p = (data[l].Get(x,y-1).z - data[l].Get(x-1,y-1).z) / levelDiv;
        real32 q = (data[l].Get(x,y).z - data[l].Get(x,y-1).z) / levelDiv;
        data[l].Get(x,y).dt[1] = RefDq(p,q);
       }
       else data[l].Get(x,y).dt[1] = 0.0;
        
      // 2 - (x,y),(x,y-1),(x+1,y)...
       if (xInc&&yDec&&(!math::IsZero(irr.Get(x,y-1))))
       {
        real32 p = (data[l].Get(x+1,y).z - data[l].Get(x,y).z) / levelDiv;
        real32 q = (data[l].Get(x,y).z - data[l].Get(x,y-1).z) / levelDiv;
        data[l].Get(x,y).dt[2] = RefDq(p,q) - RefDp(p,q);
       }
       else data[l].Get(x,y).dt[2] = 0.0;

      // 3 - (x,y),(x+1,y),(x+1,y+1)...
       if (xInc&&yInc&&(!math::IsZero(irr.Get(x,y))))
       {
        real32 p = (data[l].Get(x+1,y).z - data[l].Get(x,y).z) / levelDiv;
        real32 q = (data[l].Get(x+1,y+1).z - data[l].Get(x+1,y).z) / levelDiv;
        data[l].Get(x,y).dt[3] = -RefDp(p,q);
       }
       else data[l].Get(x,y).dt[3] = 0.0;
        
      // 4 - (x,y),(x+1,y+1),(x,y+1)...
       if (xInc&&yInc&&(!math::IsZero(irr.Get(x,y))))
       {
        real32 p = (data[l].Get(x+1,y+1).z - data[l].Get(x,y+1).z) / levelDiv;
        real32 q = (data[l].Get(x,y+1).z - data[l].Get(x,y).z) / levelDiv;
        data[l].Get(x,y).dt[4] = -RefDq(p,q);
       }
       else data[l].Get(x,y).dt[4] = 0.0;

      // 5 - (x,y),(x,y+1),(x-1,y)...
       if (xDec&&yInc&&(!math::IsZero(irr.Get(x-1,y))))
       {
        real32 p = (data[l].Get(x,y).z - data[l].Get(x-1,y).z) / levelDiv;
        real32 q = (data[l].Get(x,y+1).z - data[l].Get(x,y).z) / levelDiv;
        data[l].Get(x,y).dt[5] = RefDp(p,q) - RefDq(p,q);
       }
       else data[l].Get(x,y).dt[5] = 0.0;
    }
   }
   prog->Pop();
  }
  prog->Pop();


 // Iterate the multigrid levels and fill in 'a' for each level from the 
 // differentials alone...
  prog->Report(2,5);
  prog->Push();
  for (nat32 l=0;l<data.Size();l++)
  {
   prog->Report(l,data.Size());
   prog->Push();
   for (nat32 y=0;y<data[l].Height();y++)
   {
    prog->Report(y,data[l].Height());
    for (nat32 x=0;x<data[l].Width();x++)
    {
     // Get safety information...
       bit xInc = (x+1)<data[l].Width();
       bit xDec = x>0;
       bit yInc = (y+1)<data[l].Height();
       bit yDec = y>0;
           
     // Zero out the a so we can fill it in additavly...
      mg.ZeroA(l,x,y);

     // Entry zero, with offset (0,0), is nice and simple...
     {
      real32 temp = 0.0;
      for (nat32 t=0;t<6;t++) temp += math::Sqr(data[l].Get(x,y).dt[t]);
      if (math::IsZero(temp)) mg.SetA(l,x,y,0,1.0);
                         else mg.SetA(l,x,y,0,2.0*temp);
     }

     // Do the other offsets by iterating the 6 vertices that share triangles
     // with the target vertex...
      // Vert 0...
       if (xDec&&yDec)
       {
        real32 temp  = data[l].Get(x,y).dt[0] * data[l].Get(x-1,y-1).dt[4];
               temp += data[l].Get(x,y).dt[1] * data[l].Get(x-1,y-1).dt[3];
               temp *= 2.0;
        mg.SetA(l,x,y,10,temp);
       }
       else mg.SetA(l,x,y,10,0.0);
       
      // Vert 1...
       if (yDec)
       {
        real32 temp  = data[l].Get(x,y).dt[1] * data[l].Get(x,y-1).dt[5];
               temp += data[l].Get(x,y).dt[2] * data[l].Get(x,y-1).dt[4];
               temp *= 2.0;
        mg.SetA(l,x,y,4,temp);
       }
       else mg.SetA(l,x,y,4,0.0);

      // Vert 2...
       if (xInc)
       {
        real32 temp  = data[l].Get(x,y).dt[2] * data[l].Get(x+1,y).dt[0];
               temp += data[l].Get(x,y).dt[3] * data[l].Get(x+1,y).dt[5];
               temp *= 2.0;
        mg.SetA(l,x,y,1,temp);       
       }
       else mg.SetA(l,x,y,1,0.0);
             
      // Vert 3...
       if (xInc&&yInc)
       {
        real32 temp  = data[l].Get(x,y).dt[3] * data[l].Get(x+1,y+1).dt[1];
               temp += data[l].Get(x,y).dt[4] * data[l].Get(x+1,y+1).dt[0];
               temp *= 2.0;
        mg.SetA(l,x,y,6,temp);   
       }
       else mg.SetA(l,x,y,6,0.0);
             
      // Vert 4...
       if (yInc)
       {
        real32 temp  = data[l].Get(x,y).dt[4] * data[l].Get(x,y+1).dt[2];
               temp += data[l].Get(x,y).dt[5] * data[l].Get(x,y+1).dt[1];
               temp *= 2.0;
        mg.SetA(l,x,y,2,temp);   
       }
       else mg.SetA(l,x,y,2,0.0);
             
      // Vert 5...
       if (xDec)
       {
        real32 temp  = data[l].Get(x,y).dt[5] * data[l].Get(x-1,y).dt[3];
               temp += data[l].Get(x,y).dt[0] * data[l].Get(x-1,y).dt[2];
               temp *= 2.0;
        mg.SetA(l,x,y,3,temp);   
       }
       else mg.SetA(l,x,y,3,0.0);
       
       
      // If the pixel is suitable log this info, so we can see it evolve...
       /*if ((x==(data[l].Width()/3))&&(y==(data[l].Height()/3)))
       {
        LogDebug("{level,x,y}" << LogDiv() << l << LogDiv() << x << LogDiv() << y);
        for (nat32 i=0;i<6;i++)
        {
         LogDebug("{tri,diff}" << LogDiv() << i << LogDiv() << data[l].Get(x,y).dt[i]);
        }
        LogDebug("{se,val}" << LogDiv() << 0 << LogDiv() << mg.GetA(l,x,y,0));
        LogDebug("{se,val}" << LogDiv() << 1 << LogDiv() << mg.GetA(l,x,y,1));
        LogDebug("{se,val}" << LogDiv() << 2 << LogDiv() << mg.GetA(l,x,y,2));
        LogDebug("{se,val}" << LogDiv() << 3 << LogDiv() << mg.GetA(l,x,y,3));
        LogDebug("{se,val}" << LogDiv() << 4 << LogDiv() << mg.GetA(l,x,y,4));
        LogDebug("{se,val}" << LogDiv() << 6 << LogDiv() << mg.GetA(l,x,y,6));
        LogDebug("{se,val}" << LogDiv() << 10 << LogDiv() << mg.GetA(l,x,y,10));
       }*/
    }
   }
   prog->Pop();
  }
  prog->Pop();


 // Second pass of 'a' to add the smoothing information...
  prog->Report(3,5);
  prog->Push();
  for (nat32 l=0;l<data.Size();l++)
  {
   prog->Report(l,data.Size());
   prog->Push();
   real32 mult = lambda / math::Sqr(math::Pow(real32(2.0),real32(l)));
   //real32 mult = lambda / math::Sqr(l+1.0); // Wrong, taken from code.
   for (nat32 y=0;y<data[l].Height();y++)
   {
    prog->Report(y,data[l].Height());
    for (nat32 x=0;x<data[l].Width();x++)
    {
     // Create a full smoothing stencil...
      int32 s[13];
                            s[11] =  1;
                s[10] =  2; s[4]  = -8; s[12] =  2;
      s[9] = 1; s[3]  = -8; s[0]  = 20; s[1]  = -8; s[5] = 1;
                s[8]  =  2; s[2]  = -8; s[6]  =  2;
                            s[7]  =  1;
     
     // Update the stencil with boundary constraints...
      // Outer...
       if (x<2) {s[9] = 0;  s[3] += 2;}
       if (y<2) {s[11] = 0; s[4] += 2;}
       if (x+2>=data[l].Width())  {s[5] = 0; s[1] += 2;}
       if (y+2>=data[l].Height()) {s[7] = 0; s[2] += 2;}
       
      // Inner...
       if (x<1) {s[1] += 2; s[2] += 2; s[4] += 2;}
       if (y<1) {s[1] += 2; s[2] += 2; s[3] += 2;}
       if (x+1>=data[l].Width()) {s[2] += 2; s[3] += 2; s[4] += 2;}
       if (y+1>=data[l].Height()) {s[1] += 2; s[3] += 2; s[4] += 2;}
       
       if (x<1) {s[10] = 0; s[3] = 0; s[8] = 0;}
       if (y<1) {s[10] = 0; s[4] = 0; s[12] = 0;}
       if (x+1>=data[l].Width()) {s[12] = 0; s[1] = 0; s[6] = 0;}
       if (y+1>=data[l].Height()) {s[8] = 0; s[2] = 0; s[6] = 0;}

      // Correct the centre...
       for (nat32 se=0;se<13;se++) s[0] -= s[se];
     
     // Add the entire stencil to the current stencil, noting that 
     // out-of-bound entrys will be ignored...
     // (Multiply in the lambda and scale adjustments.)
      for (nat32 se=0;se<13;se++) mg.AddA(l,x,y,se,mult * s[se]);
    }
   }
   prog->Pop();
  }
  prog->Pop();


 // Fix the centre pixel to a depth of 0, to stop drift...
  //mg.Fix(0,data[0].Width()/2,data[0].Height()/2,0.0);
  //mg.Fix(0,0,0,0.0);



 // Fill in b for the first level of the multigrid only...
  // This converts face index to offset from node coordinate to get irr coordinate...
   static const int32 faceToIrrX[6] = {-1,-1, 0, 0, 0,-1};
   static const int32 faceToIrrY[6] = {-1,-1,-1, 0, 0, 0};
   
  // These indicate the offset and face number within the location indicated by
  // the offset that match a face number at the current location.
  // (2 vertices provided, the third you already have.)
   /*static const int32 faceToOffsetXA[6] = {-1,-1, 0, 1, 1, 0};
   static const int32 faceToOffsetYA[6] = { 0,-1,-1, 0, 1, 1};
   static const nat32 faceToFaceA[6] = {2,3,4,5,0,1};
   
   static const int32 faceToOffsetXB[6] = {-1, 0, 1, 1, 0,-1};
   static const int32 faceToOffsetYB[6] = {-1,-1, 0, 1, 1, 0};
   static const nat32 faceToFaceB[6] = {4,5,0,1,2,3};*/
   
  // Actual work...
   prog->Report(4,5);
   prog->Push();
   for (nat32 y=0;y<data[0].Height();y++)
   {
    prog->Report(y,data[0].Height());
    for (nat32 x=0;x<data[0].Width();x++)
    {
     real32 b = 0.0;
     
     /*if ((x==(data[0].Width()/3))&&(y==(data[0].Height()/3))) // ******************************
     {
      LogDebug("z(-1,-1),z(0,-1),z(1,-1)" << LogDiv() << data[0].Get(x-1,y-1).z 
                                          << LogDiv() << data[0].Get(x,y-1).z
                                          << LogDiv() << data[0].Get(x+1,y-1).z);
      LogDebug("z(-1,0),z(0,0),z(1,0)" << LogDiv() << data[0].Get(x-1,y).z 
                                       << LogDiv() << data[0].Get(x,y).z
                                       << LogDiv() << data[0].Get(x+1,y).z);
      LogDebug("z(-1,1),z(0,1),z(1,1)" << LogDiv() << data[0].Get(x-1,y+1).z 
                                       << LogDiv() << data[0].Get(x,y+1).z
                                       << LogDiv() << data[0].Get(x+1,y+1).z);
     }*/
                                              
     // Go over all 6 faces that interact with this node and sum in there relevant affect...
      for (nat32 f=0;f<6;f++)
      {
       if (!math::IsZero(data[0].Get(x,y).dt[f])) // Essentially boundary checking.
       {
        // Get the target irradiance...
         real32 e = irr.Get(x+faceToIrrX[f],y+faceToIrrY[f]);
           
        // Subtract the current irradiance to get the required change...
         real32 p = 0.0,q = 0.0;
         switch (f)
         {
          case 0:
           p = data[0].Get(x,y).z - data[0].Get(x-1,y).z;
           q = data[0].Get(x-1,y).z - data[0].Get(x-1,y-1).z;
          break;
          case 1:
           p = data[0].Get(x,y-1).z - data[0].Get(x-1,y-1).z;
           q = data[0].Get(x,y).z - data[0].Get(x,y-1).z;
          break;
          case 2:
           p = data[0].Get(x+1,y).z - data[0].Get(x,y).z;
           q = data[0].Get(x,y).z - data[0].Get(x,y-1).z;
          break;
          case 3:
           p = data[0].Get(x+1,y).z - data[0].Get(x,y).z;
           q = data[0].Get(x+1,y+1).z - data[0].Get(x+1,y).z;
          break;
          case 4:
           p = data[0].Get(x+1,y+1).z - data[0].Get(x,y+1).z;
           q = data[0].Get(x,y+1).z - data[0].Get(x,y).z;
          break;
          case 5:
           p = data[0].Get(x,y).z - data[0].Get(x-1,y).z;
           q = data[0].Get(x,y+1).z - data[0].Get(x,y).z;
          break;                                                       
         }
        e -= Ref(p,q);
        
        /*if ((x==(data[0].Width()/3))&&(y==(data[0].Height()/3))) // ******************************
        {
         LogDebug("irr err tri " << f << LogDiv() << e);
        }*/
          
        // Add the affect of the current depth values, so the depth values
        // remain absolute rather than relative to the previous iteration...
         //e += data[0].Get(x,y).z *
         //     data[0].Get(x,y).dt[f];
         //e += data[0].Get(x+faceToOffsetXA[f],y+faceToOffsetYA[f]).z *
         //     data[0].Get(x+faceToOffsetXA[f],y+faceToOffsetYA[f]).dt[faceToFaceA[f]];
         //e += data[0].Get(x+faceToOffsetXB[f],y+faceToOffsetYB[f]).z *
         //     data[0].Get(x+faceToOffsetXB[f],y+faceToOffsetYB[f]).dt[faceToFaceB[f]];
         e += RefDp(p,q) * p;
         e += RefDq(p,q) * q;
         
         
          /*if ((x==(data[0].Width()/3))&&(y==(data[0].Height()/3))) // ******************************
          {
           LogDebug("B tri " << f << LogDiv() << (e * data[0].Get(x,y).dt[f]) << LogDiv() << e);
          }*/
         
        // Add to sum...
         b += e * data[0].Get(x,y).dt[f];
       }
      }
      
     // ********************************************************************************
     /*if ((x==(data[0].Width()/3))&&(y==(data[0].Height()/3)))
     {
      LogDebug("B" << LogDiv() << b);
     }*/
      
     mg.SetB(0,x,y,2.0 * b);
    }
   }
   prog->Pop();


 prog->Pop();
}

//------------------------------------------------------------------------------
 };
};
