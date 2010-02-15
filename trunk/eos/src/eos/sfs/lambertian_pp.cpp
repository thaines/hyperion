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

#include "eos/sfs/lambertian_pp.h"

#include "eos/ds/arrays.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
LambertianPP::LambertianPP()
:radius(5),radSD(2.0),prob(0.99),cutoff(2.0),maxSamples(1000)
{}

LambertianPP::~LambertianPP()
{}

void LambertianPP::Set(const svt::Field<real32> & i)
{
 irr = i;
}

void LambertianPP::Set(const svt::Field<bs::Normal> & o)
{
 orient = o;
}

void LambertianPP::SetWin(nat32 r,real32 s)
{
 radius = r;
 radSD = s;
}

void LambertianPP::SetSel(real32 p,real32 c,nat32 m)
{
 prob = p;
 cutoff = c;
 maxSamples = m;
}

void LambertianPP::Run(time::Progress * prog)
{
 LogBlock("eos::sfs::LambertianPP::Run","");
 prog->Push();

 // Create and zero out the output structure, we use it during run-time...
  res.Resize(irr.Size(0),irr.Size(1));
  for (nat32 y=0;y<res.Height();y++)
  {
   for (nat32 x=0;x<res.Width();x++)
   {
    res.Get(x,y).model = bs::Normal(0.0,0.0,0.0);
    res.Get(x,y).weight = 0.0;
   }
  }


 // Create support data structures. We need a mask to use over the window to
 // provide weight and a method of selecting random pixels from the window with
 // a probability proportional to there weighting... (Without double selection.)
  // Create a Gaussian mask over the window, normalise without the centre pixel,
  // as its comming regardless...
   ds::Array2D<real32> mask(radius*2+1,radius*2+1);
   {
    real32 div = 4.0*math::Sqr(radius/radSD);
    for (int32 y=-radius;y<=radius;y++)
    {
     for (int32 x=-radius;x<=radius;x++)
     {
      mask.Get(x+radius,y+radius) = math::Exp(-(math::Sqr(x)+math::Sqr(y))/div);
     }
    }
   
    mask.Get(radius,radius) = 0.0;
   
    real32 sum = 0.0;
    for (int32 y=-radius;y<=radius;y++)
    {
     for (int32 x=-radius;x<=radius;x++) sum += mask.Get(x+radius,y+radius);
    }
    
    for (int32 y=-radius;y<=radius;y++)
    {
     for (int32 x=-radius;x<=radius;x++) mask.Get(x+radius,y+radius) /= sum;
    }
   }
   LogDebug("[lambertian pp] {mask}" << LogDiv() << mask);

  // Create an array of pixel coordinates with culumative weights, summed to 
  // one, so we can randomly select pixel locations...
   ds::Array<Sel> select(math::Sqr(radius*2+1) - 1);
   {
    nat32 i = 0;
    for (int32 y=-radius;y<=radius;y++)
    {
     for (int32 x=-radius;x<=radius;x++)
     {
      if ((x==0)&&(y==0)) continue;
      select[i].weight = mask.Get(x+radius,y+radius);
      select[i].u = x;
      select[i].v = y;
      ++i;
     }
    }
    
    select[0].culum = 0.0;
    for (nat32 i=1;i<select.Size();i++) select[i].culum = select[i-1].culum + select[i-1].weight;
   }
 

 // Create interlacing pattern mask, its size is speced by radius...
  int32 diffRad = 0; 
  // How many passes to do...
   int32 passes = math::Sqr(diffRad+1);
  
  // 2D array, repeated over image, number indicates which pass that pixel
  // should be done in...
  // Calculated in a convoluted brute force way, as I can't figure out an 
  // elegant way to do it.
   ds::Array2D<int32> interMask(diffRad+1,diffRad+1);
   {
    // Fill it with -1's to indicate each value being a dud...
     for (nat32 v=0;v<interMask.Height();v++)
     {
      for (nat32 u=0;u<interMask.Width();u++) interMask.Get(u,v) = -1;
     }
    
    // Loop the number of entrys in the array, introducing a new pass each time...
     for (int32 p=0;p<passes;p++)
     {
      // Find the lowest value in the array...
       int32 lowest = 0;
       for (nat32 v=0;v<interMask.Height();v++)
       {
        for (nat32 u=0;u<interMask.Width();u++) lowest = math::Min(lowest,interMask.Get(u,v));
       }
      
      // First entry that matches the lowest value put the value in, then update
      // the array values to make sure the next value is as far away from this 
      // value as possible...
       bit cont = true;
       for (nat32 v=0;(v<interMask.Height())&&cont;v++)
       {
        for (nat32 u=0;(u<interMask.Width())&&cont;u++)
        {
         if (lowest==interMask.Get(u,v))
         {
          // Update the pixel...
           interMask.Get(u,v) = p;
           
          // Lower costs in the surrounding region...
           for (int32 y=-int32(diffRad);y<=int32(diffRad);y++)
           {
            for (int32 x=-int32(diffRad);x<=int32(diffRad);x++)
            {
             nat32 xc = (interMask.Width() + x)%interMask.Width();
             nat32 yc = (interMask.Height() + x)%interMask.Height();
             if (interMask.Get(xc,yc)<0) interMask.Get(xc,yc) -= math::Abs(y) + math::Abs(x);
            }
           }

          cont = false;
         }
        }
       }
     }
   }
   LogDebug("[lambertian pp] {inter mask}" << LogDiv() << interMask);


 // Iterate every pixel. We do not do this in order, but rather in an 
 // interlaced pattern to save time as we can steal solutions from
 // adjacent pixels that have already been calculated...
  for (int32 p=0;p<passes;p++)
  {
   prog->Report(p,passes);
   prog->Push();
   for (int32 y=0;y<int32(res.Height());y++)
   {
    prog->Report(y,res.Height());
    for (int32 x=0;x<int32(res.Width());x++)
    {
     if (interMask.Get(x%interMask.Width(),y%interMask.Height())==p)
     {
      LogTime("eos::sfs::LambertianPP::Run ransac");
      // Run RANSAC to find the best mode avaliable, unless of course we
      // already stole it from a neighbour...
       for (nat32 i=0;i<maxSamples;i++)
       {
        // First select two pixels in addition to the centre, to calculate a
        // model with all 3. Avoid duplication, handle boundary conditions...
         // First is easy enough...
          nat32 pix1;
          int32 pos1[2];
          while (true)
          {
           Sel dummy;
           dummy.culum = rand.Normal();
           pix1 = select.SearchLargestNorm(dummy);

           pos1[0] = x + select[pix1].u;
           pos1[1] = y + select[pix1].v;
           
           if ((pos1[0]<0)||(pos1[0]>=int32(res.Width()))) continue;
           if ((pos1[1]<0)||(pos1[1]>=int32(res.Height()))) continue;
           break;
          }

         // Second has to be done with exclusion of the first...
          nat32 pix2;
          int32 pos2[2];
          while (true)
          {
           Sel dummy;
           dummy.culum = rand.Normal() * (1.0 - select[pix1].weight);
           if (dummy.culum>=select[pix1].culum) dummy.culum += select[pix1].weight;
           pix2 = select.SearchLargestNorm(dummy);

           pos2[0] = x + select[pix2].u;
           pos2[1] = y + select[pix2].v;
           
           if ((pos2[0]<0)||(pos2[0]>=int32(res.Width()))) continue;
           if ((pos2[1]<0)||(pos2[1]>=int32(res.Height()))) continue;
           break;           
          }


        // Use the centre plus the two locations to calculate a model...
         math::Mat<3,3> a;
         bs::Normal model;
         {
          bs::Normal temp;
          temp = orient.Get(x,y);             a[0][0] = temp[0]; a[0][1] = temp[1]; a[0][2] = temp[2];
          temp = orient.Get(pos1[0],pos1[1]); a[1][0] = temp[0]; a[1][1] = temp[1]; a[1][2] = temp[2];
          temp = orient.Get(pos2[0],pos2[1]); a[2][0] = temp[0]; a[2][1] = temp[1]; a[2][2] = temp[2];

          model[0] = irr.Get(x,y);
          model[1] = irr.Get(pos1[0],pos1[1]);
          model[2] = irr.Get(pos2[0],pos2[1]);

          math::SolveLinear(a,model);
          
          real32 albedo = model.Length();
          if (!math::IsFinite(albedo)) model = bs::Normal(0.0,0.0,0.0);
         }


        // Check if the model is better than previous models, if so replace them...
        {
         real32 tw = Weight(mask,x,y,model);
         if (tw>res.Get(x,y).weight)
         {
          res.Get(x,y).model = model;
          res.Get(x,y).weight = tw;
         }
        }
         
        // Do same check for the other two pixels involved in the calculation...
        /*{
         real32 tw = Weight(mask,pos1[0],pos1[1],model);
         if (tw>res.Get(pos1[0],pos1[1]).weight)
         {
          res.Get(pos1[0],pos1[1]).model = model;
          res.Get(pos1[0],pos1[1]).weight = tw;
         }
        }
        {
         real32 tw = Weight(mask,pos2[0],pos2[1],model);
         if (tw>res.Get(pos2[0],pos2[1]).weight)
         {
          res.Get(pos2[0],pos2[1]).model = model;
          res.Get(pos2[0],pos2[1]).weight = tw;
         }
        }*/


        // Do the RANSAC check, to see if we can stop or need to keep going...
         if (!math::IsZero(res.Get(x,y).weight))
         {
          real32 samples = math::Ln(1.0-prob)/math::Ln(1.0 - math::Sqr(res.Get(x,y).weight));
          if (real32(i)>samples) break;
         }
       }
     }
    }
   }
   prog->Pop();
  }


 prog->Pop();
}

void LambertianPP::Get(nat32 x,nat32 y,bs::Normal &  out)
{
 out = res.Get(x,y).model;
}

real32 LambertianPP::Weight(nat32 x,nat32 y)
{
 return res.Get(x,y).weight;
}

real32 LambertianPP::Weight(const ds::Array2D<real32> & mask,int32 x,int32 y,const bs::Normal & model) const
{
 real32 ret = 0.0;

 for (int32 v=math::Max(y-radius,int32(0));v<=math::Min(y+radius,int32(res.Height())-1);v++)
 {
  for (int32 u=math::Max(x-radius,int32(0));u<=math::Min(x+radius,int32(res.Width())-1);u++)
  {
   real32 predIrr = orient.Get(u,v)*model;
   if (math::Abs(predIrr-irr.Get(u,v))<cutoff) ret += mask.Get(u-x,v-y);
  }
 }
 
 return ret;
}

//------------------------------------------------------------------------------
 };
};
