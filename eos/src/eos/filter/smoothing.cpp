//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines
#include "eos/filter/smoothing.h"

#include "eos/bs/geo2d.h"
#include "eos/ds/arrays.h"
#include "eos/ds/arrays2d.h"
#include "eos/math/mat_ops.h"


namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
// Helper structure for below, used for sorting supporting pixels by support...
struct PixSup
{
 int32 x;
 int32 y;
 real32 support;

 bit operator < (const PixSup & rhs) const
 {
  return support > rhs.support; // Reversed, as we want the largest first.
 }
};

//------------------------------------------------------------------------------
EOS_FUNC void QuantSmooth(const svt::Field<real32> & in,svt::Field<real32> & out,
                          const svt::Field<bs::ColourLuv> & dist,
                          nat32 walkLength,nat32 fitN,real32 distMult,
                          time::Progress * prog,nat32 levels,nat32 iters)
{
 LogTime("eos::Filter::QuantSmooth");
 log::Assert((in.Size(0)==out.Size(0))&&(in.Size(0)==dist.Size(0))&&
             (in.Size(1)==out.Size(1))&&(in.Size(1)==dist.Size(1)));
 prog->Push();

 // Use diffusion to create a supporting set for each pixel, i.e. the pixels 
 // considered similar and therefore used for the smoothing process...
  prog->Report(0,iters+1);
  svt::Var supportVar(in);
  supportVar.Setup3D(in.Size(0),in.Size(1),fitN);
  {
   bs::Pos coordIni;
   supportVar.Add("coord",coordIni);
  }
  supportVar.Commit();
  svt::Field<bs::Pos> support(&supportVar,"coord");
  {
   // Setup some buffers used for the process...
    ds::Array2D<real32> bufA(walkLength*2+1,walkLength*2+1);
    ds::Array2D<real32> bufB(walkLength*2+1,walkLength*2+1);
  
    ds::Array2D<real32> * from = &bufA;
    ds::Array2D<real32> * to = &bufB;
    
   // Generate a weighting map for how the diffusion flows between each pixel...
    ds::Array2D<real32> incX(in.Size(0),in.Size(1));
    ds::Array2D<real32> decX(in.Size(0),in.Size(1));
    ds::Array2D<real32> incY(in.Size(0),in.Size(1));
    ds::Array2D<real32> decY(in.Size(0),in.Size(1));
    for (nat32 y=0;y<in.Size(1);y++)
    {
     for (nat32 x=0;x<in.Size(0);x++)
     {
      incX.Get(x,y) = (x+1<in.Size(0))?math::Exp(-distMult*dist.Get(x,y).Diff(dist.Get(x+1,y))):0.0;
      decX.Get(x,y) =            (x>0)?math::Exp(-distMult*dist.Get(x,y).Diff(dist.Get(x-1,y))):0.0;
      incY.Get(x,y) = (y+1<in.Size(1))?math::Exp(-distMult*dist.Get(x,y).Diff(dist.Get(x,y+1))):0.0;
      decY.Get(x,y) =            (y>0)?math::Exp(-distMult*dist.Get(x,y).Diff(dist.Get(x,y-1))):0.0;
      
      real32 sum = incX.Get(x,y) + decX.Get(x,y) + incY.Get(x,y) + decY.Get(x,y);
      
      incX.Get(x,y) /= sum;
      decX.Get(x,y) /= sum;
      incY.Get(x,y) /= sum;
      decY.Get(x,y) /= sum;
     }
    }
    
   // Construct a list of locations and there scores, ready for use by the below...
    ds::Array<PixSup> pixList(math::Sqr(walkLength*2+1));


   // Iterate and generate the list for every pixel...
    prog->Push();
    for (int32 y=0;y<int32(in.Size(1));y++)
    {
     prog->Report(y,in.Size(1));
     for (int32 x=0;x<int32(in.Size(0));x++)
     {
      // First calculate the grid of diffuse values, with diffusion affected by
      // there similarity with adjacent pixels...
       from->Get(walkLength,walkLength) = 1.0;
       for (nat32 i=0;i<walkLength;i++)
       {
        // Zero the relevant region of the to buffer...
         for (nat32 v=walkLength-i-1;v<=walkLength+i+1;v++)
         {
          for (nat32 u=walkLength-i-1;u<=walkLength+i+1;u++) to->Get(u,v) = 0.0;
         }
         
        // Fill in the to buffer...
         for (nat32 v=walkLength-i;v<=walkLength+i;v++)
         {
          for (nat32 u=walkLength-i;u<=walkLength+i;u++)
          {
           int32 ox = x + int32(u) - int32(walkLength);
           int32 oy = y + int32(v) - int32(walkLength);
           if ((ox<0)||(ox>=int32(in.Size(0)))||(oy<0)||(oy>=int32(in.Size(1)))) continue;
           
           to->Get(u+1,v) += incX.Get(ox,oy) * from->Get(u,v);
           to->Get(u-1,v) += decX.Get(ox,oy) * from->Get(u,v);
           to->Get(u,v+1) += incY.Get(ox,oy) * from->Get(u,v);
           to->Get(u,v-1) += decY.Get(ox,oy) * from->Get(u,v);
          }
         }
        
        // Swap buffers...
         math::Swap(from,to);
       }
      
      // Now find the N highest values in the grid, and stick them into the data
      // structure...
      // (Fill list, sort list, take top 25.)
       nat32 writePos = 0;
       for (int32 v=0;v<int32(walkLength*2+1);v++)
       {
        for (int32 u=0;u<int32(walkLength*2+1);u++)
        {
         if ((v==int32(walkLength))&&(u==int32(walkLength))) continue;
         if (!math::IsZero(to->Get(u,v)))
         {
          pixList[writePos].x = x + u - int32(walkLength);
          pixList[writePos].y = y + v - int32(walkLength);
          pixList[writePos].support = to->Get(u,v);
          writePos += 1;
         }
        }
       }
      
       if (writePos<fitN)
       {
        // Should never happen, but just incase the user (i.e. me) is an idiot...
         for (nat32 i=0;i<fitN;i++)
         {
          support.Get(x,y,i)[0] = pixList[i%writePos].x;
          support.Get(x,y,i)[1] = pixList[i%writePos].y; 
         }
       }
       else
       {
        pixList.SortRangeNorm(0,writePos-1);
        for (nat32 i=0;i<fitN;i++)
        {
         support.Get(x,y,i)[0] = pixList[i].x;
         support.Get(x,y,i)[1] = pixList[i].y; 
        }
       }
     }
    }
    prog->Pop();
  }



 // Calculate the +/- range to be used for clamping...
  real32 clamp = 0.5 / real32(levels-1);

 // Create buffers to store the answer as we calculate it...
  ds::Array2D<real32> bufA(in.Size(0),in.Size(1));
  ds::Array2D<real32> bufB(in.Size(0),in.Size(1));
  
  ds::Array2D<real32> * from = &bufA;
  ds::Array2D<real32> * to = &bufB;
  
 // Support matrices...
  math::Matrix<real32> matA(fitN,3);
  math::Vector<real32> vecB(fitN);
  math::Vect<3,real32> vecX;
  math::Mat<3,3,real32> matTemp;



 // Copy the in to the 'from buffer'...
  for (nat32 y=0;y<in.Size(1);y++)
  {
   for (nat32 x=0;x<in.Size(0);x++) from->Get(x,y) = in.Get(x,y);
  }

 // Do the iterations...
  for (nat32 i=0;i<iters;i++)
  {
   prog->Report(i+1,iters+1);
   prog->Push();
   for (int32 y=0;y<int32(in.Size(1));y++)
   {
    prog->Report(y,in.Size(1));
    for (int32 x=0;x<int32(in.Size(0));x++)
    {
     // Use a pixels supporting set to calculate its correct value, before 
     // clamping it back to its allowed range...
     // Correct value is calculated via plane fitting.
      real32 base = in.Get(x,y);
      for (nat32 i=0;i<fitN;i++)
      {
       matA[i][0] = 1.0;
       matA[i][1] = support.Get(x,y,i)[0] - x;
       matA[i][2] = support.Get(x,y,i)[1] - y;
       vecB[i] = from->Get(support.Get(x,y,i)[0],support.Get(x,y,i)[1]) - base;
      }
      
      if (math::SolveLeastSquares(matA,vecB,vecX,matTemp))
      {
       to->Get(x,y) = math::Clamp(base + math::Clamp(vecX[0],-clamp,clamp),real32(0.0),real32(1.0));
      }
    }
   }
   prog->Pop();
   math::Swap(from,to);
  }

 // Transfer the answer from the 'to buffer' to out...
  for (nat32 y=0;y<out.Size(1);y++)
  {
   for (nat32 x=0;x<out.Size(0);x++) out.Get(x,y) = to->Get(x,y);
  }

 prog->Pop();
}

//------------------------------------------------------------------------------
EOS_FUNC void SimpleQuantSmooth(svt::Field<bs::ColourRGB> & inOut,real32 cap,
                                time::Progress * prog,nat32 levels,nat32 iters)
{
 prog->Push();
 
 // Copy the input into a tempory buffer, so we know the clamping range after
 // making changes...
  ds::Array2D<bs::ColourRGB> dup(inOut.Size(0),inOut.Size(1));
  for (nat32 y=0;y<dup.Height();y++)
  {
   for (nat32 x=0;x<dup.Width();x++) dup.Get(x,y) = inOut.Get(x,y);
  }
  
 // Calculate the relevant clamping range...
  real32 clampRange = 0.5 / real32(levels-1);
  
 // Do all the iterations...
  for (nat32 i=0;i<iters;i++)
  {
   prog->Report(i,iters); 
   prog->Push();
   for (nat32 y=0;y<inOut.Size(1);y++)
   {
    prog->Report(y,inOut.Size(1));
    for (nat32 x=(i+y)%2;x<inOut.Size(0);x+=2)
    {
     // Get the 4 adjacent values, at the egdes deliberatly set a value that
     // will be ignored below...
      bs::ColourRGB n[4];

      if (x+1<inOut.Size(0)) n[0] = inOut.Get(x+1,y);
                        else n[0] = bs::ColourRGB(-1.0,-1.0,-1.0);
      if (y+1<inOut.Size(1)) n[1] = inOut.Get(x,y+1);
                        else n[1] = bs::ColourRGB(-1.0,-1.0,-1.0);     
      if (x>0) n[2] = inOut.Get(x-1,y);
          else n[2] = bs::ColourRGB(-1.0,-1.0,-1.0);
      if (y>0) n[3] = inOut.Get(x,y-1);
          else n[3] = bs::ColourRGB(-1.0,-1.0,-1.0);

     // Change to absolute difference values...
      for (nat32 d=0;d<4;d++)
      {
       n[d].r = math::Abs(n[d].r-inOut.Get(x,y).r);
       n[d].g = math::Abs(n[d].g-inOut.Get(x,y).g);
       n[d].b = math::Abs(n[d].b-inOut.Get(x,y).b);
      }
      
     // Select the best value - this is done by taking the average of all pixels
     // that are in range of the starting position...
      nat32 count = 0;
      bs::ColourRGB avg(0.0,0.0,0.0);
      for (nat32 d=0;d<4;d++)
      {
       if ((n[d].r+n[d].g+n[d].b)<cap)
       {
        avg += n[d];
        ++count;
       }
      }
      if (count!=0)
      {
       avg.r /= real32(count);
       avg.g /= real32(count);
       avg.b /= real32(count);
       inOut.Get(x,y) = avg;
      }
     
     // Clamp the result...
      inOut.Get(x,y).r = math::Clamp(inOut.Get(x,y).r,
                                     math::Max(dup.Get(x,y).r-clampRange,real32(0.0)),
                                     math::Min(dup.Get(x,y).r+clampRange,real32(1.0)));
      inOut.Get(x,y).g = math::Clamp(inOut.Get(x,y).g,
                                     math::Max(dup.Get(x,y).g-clampRange,real32(0.0)),
                                     math::Min(dup.Get(x,y).g+clampRange,real32(1.0)));     
      inOut.Get(x,y).b = math::Clamp(inOut.Get(x,y).b,
                                     math::Max(dup.Get(x,y).b-clampRange,real32(0.0)),
                                     math::Min(dup.Get(x,y).b+clampRange,real32(1.0)));     
    }
   }
   prog->Pop();
  }
 
 prog->Pop();
}

//------------------------------------------------------------------------------
 };
};
