//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

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

#include "eos/alg/mean_shift.h"

namespace eos
{
 namespace alg
 {
//------------------------------------------------------------------------------
MeanShift::MeanShift()
:cutoff(0.01),max_iter(100),passOpt(false),window(1.0),out(null<real32*>()),Distance(&DistDefault) 
{}

MeanShift::~MeanShift() {mem::Free(out);}

void MeanShift::SetDistance(bit (*D)(nat32 fvSize,real32 * fv1,real32 * fv2,real32 pt),real32 pt)
{
 Distance  = D;
 passThrough = pt;
}

void MeanShift::AddFeature(const svt::Field<real32> & field,real32 scale)
{
 nat32 ind = sf.Size();
 sf.Size(ind+1);
 sf[ind].field = field;
 sf[ind].scale = scale;

 if (dim.Size()!=field.Dims())
 {
  nat32 low = dim.Size();
  dim.Size(field.Dims());
  for (nat32 i=low;i<dim.Size();i++) dim[i].use = false;
 }
}

void MeanShift::AddFeature(nat32 d,real32 scale)
{
 if (dim.Size()<=d)
 {
  nat32 low = dim.Size();
  dim.Size(d+1);
  for (nat32 i=low;i<d;i++) dim[i].use = false;
 }

 dim[d].use = true;
 dim[d].scale = scale;
}

void MeanShift::SetWeight(const svt::Field<real32> & field)
{
 weight = field;
}

void MeanShift::Passover(bit enabled)
{
 passOpt = enabled;
}

void MeanShift::SetWindowSize(real32 size)
{
 window = size;
}

void MeanShift::SetCutoff(real32 change,nat32 maxIter)
{
 cutoff = change;
 max_iter = maxIter;
}

void MeanShift::Run(time::Progress * prog)
{
 prog->Push();

 // Calculate some useful numbers, such as feature vector size...
  diUsed = 0;
  for (nat32 i=0;i<dim.Size();i++)
  {
   if (dim[i].use) ++diUsed;
  }
  fvSize = diUsed + sf.Size();
 

 // Go through and multiply in the window parameter, so we can forget about it...
  window = 1.0/window;
  for (nat32 i=0;i<dim.Size();i++) if (dim[i].use) dim[i].scale *= window;
  for (nat32 i=0;i<sf.Size();i++) sf[i].scale *= window;


 // First initialise an array of vectors for every sample we have...
 // The vector structure is allways <weight, dimension features... , field features... >
  samples = sf[0].field.Count();
  real32 * vect = mem::Malloc<real32>(samples*(fvSize+1));


 // Now we calculate the initial vectors...
  prog->Report(0,2);
  nat32 * pos = mem::Malloc<nat32>(sf[0].field.Dims());
  for (nat32 i=0;i<sf[0].field.Dims();i++) pos[i] = 0;
   
  real32 * targ = vect;
  for (nat32 i=0;i<samples;i++)
  {
   CalcVector(pos,targ);   
   targ += fvSize + 1;
  
   ++pos[0];
   for (nat32 j=0;j<sf[0].field.Dims()-1;j++)
   {
    if (pos[j]>=sf[0].field.Size(j))
    {
     pos[j] = 0;
     ++pos[j+1];
    }
    else break;
   }
  }  


 // And make a copy, so we can change one and leave the others constant...
 // (This copy also happens to be the output.)
  out = mem::Malloc<real32>(samples*(fvSize+1));
  mem::Copy<real32>(out,vect,samples*(fvSize+1));


 // Before the next step we need a load of malloced arrays, in addition to the pos array allready conceived...
  nat32 * mi = mem::Malloc<nat32>(dim.Size());
  nat32 * ma = mem::Malloc<nat32>(dim.Size());
  nat32 * ipos = mem::Malloc<nat32>(dim.Size());
  nat32 * stride = mem::Malloc<nat32>(dim.Size());
  real32 * mean = mem::Malloc<real32>(fvSize);

  stride[0] = 1;
  for (nat32 i=1;i<dim.Size();i++) stride[i] = stride[i-1]*sf[0].field.Size(i-1);

 // If the passover optimisation has been enabled generate its data structure...
 // Simply an array of indexes for the parent of each sample, set to point to self
 // when it has no parent.
  nat32 * pods = null<nat32*>();
  if (passOpt)
  {
   pods = new nat32[samples];
   for (nat32 i=0;i<samples;i++) pods[i] = i;
  }
    

 // For each vector we apply the mean shift algorithm till convergance...
  targ = out;
  for (nat32 i=0;i<samples;i++,targ += fvSize+1)
  {
   prog->Report(i,samples);
   // Run this particular node till convergence...
    // Converge it, different code depending on if the passover optimisation
    // is on or not...     
     if ((!passOpt)||(pods[i]==i))
     {
      nat32 lastOver = i; // Used for a quick break out, saves a bit of pissing about.
      for (nat32 k=0;k<max_iter;k++)
      {
       if (passOpt)
       {
        // Check if we are over a sample that has not converged, if so we become
        // its parent, so it will be forced to converge to the same place this
        // sample converges...
         // Find the offset of the sample...
          nat32 over = 0;
          for (nat32 j=0;j<dim.Size();j++) over += nat32(math::Round(targ[j+1]/dim[j].scale))*stride[j];          
         
         // If its not us check if we can save some sampling...
          if ((over!=lastOver)&&(over!=i))
          { 
	   lastOver = over;         
           // We only concider matching to this pixel if the remainder of the
           // feature vector, the non-dimensional features, eucledian distance
           // in the scaled space is less than half.
            real32 * ot = out + (fvSize+1)*over + 1 + dim.Size();
            real32 dist = 0.0;
            for (nat32 j=0;j<sf.Size();j++) dist += math::Sqr(ot[j]-targ[j+1+dim.Size()]);
            if (dist<0.5)
            {
             // Two scenarios - either the node has not converged, and we arrange
             // for it to converge to the same point we do, or it has converged 
             // so we go where it is going...
              if (pods[over]==over) pods[over] = i;
              else
              {
               // Instead of doing any more mean shifting just head straight to the
               // convergence point of the node found...
                nat32 toUse = pods[over];
                while (toUse!=pods[toUse]) toUse = pods[toUse];
                
                real32 * from = out + (fvSize+1)*toUse; 
                for (nat32 j=0;j<fvSize;j++) targ[j+1] = from[j+1];
              }
            }
          }
       }
       CalcShift(mi,ma,ipos,vect,stride,targ,mean);      
       for (nat32 j=0;j<fvSize;j++) targ[j+1] += mean[j];
    
       real32 shift = 0.0;
       for (nat32 j=0;j<fvSize;j++)
       {
        shift += math::Sqr(mean[j]);
        if (shift>=cutoff) break;
       }
       if (shift<cutoff) break;
      }     
     }
     else
     {
      real32 * from = out + (fvSize+1)*pods[i]; 
      for (nat32 j=0;j<fvSize;j++) targ[j+1] = from[j+1];
     }
  }

 // If the passover optimisation was used we need to clean up...
  if (passOpt) delete[] pods;  

 // Go through the output and remove the affect of the scaler...   
  real32 * scales = mem::Malloc<real32>(fvSize);
  {
   nat32 targ = 0;
   for (nat32 i=0;i<dim.Size();i++)
   {
    if (dim[i].use)
    {
     scales[targ] = 1.0/dim[i].scale;
     ++targ;
    }
   }

   for (nat32 i=0;i<sf.Size();i++)
   {
    scales[targ] = 1.0/sf[i].scale;
    ++targ;
   }
  }

  targ = out;
  for (nat32 i=0;i<samples;i++)
  {
   for (nat32 j=0;j<fvSize;j++) targ[j+1] *= scales[j];
   targ += fvSize + 1;
  }

 // Clean up...
  mem::Free(scales);

  mem::Free(mean);
  mem::Free(stride);
  mem::Free(ipos);
  mem::Free(ma);
  mem::Free(mi);

  mem::Free(pos);
  mem::Free(vect);

 prog->Pop();
}

bit MeanShift::Get(svt::Field<real32> & index,svt::Field<real32> & o)
{
 // Calculate which index we will be returning...
  nat32 i = 0;
  for (;i<sf.Size();i++)
  {
   if (sf[i].field==index) break;
  }
  if (i==sf.Size()) return false;
  nat32 ind = i + 1 + diUsed;

 // Extract that index into the field...
  o.CopyFrom(&out[ind],sizeof(real32)*(1+fvSize));

 return true;
}

bit MeanShift::Get(nat32 d,svt::Field<real32> & o)
{
 // Calculate which index we will be returning... 
  nat32 ind = 1;
  for (nat32 i=0;i<d;i++)
  {
   if (dim[i].use) ind++;
  }
  if (dim[d].use==false) return false;

 // Extract that index into the field...
  o.CopyFrom(&out[ind],sizeof(real32)*(1+fvSize)); 
 return true;
}

void MeanShift::CalcVector(nat32 * pos,real32 * out)
{
 if (weight.Valid()) *out = weight.Get(pos);
                else *out = 1.0;
 ++out;

 for (nat32 i=0;i<dim.Size();i++)
 {
  if (dim[i].use)
  {
   *out = dim[i].scale * real32(pos[i]);
   ++out;
  }
 }

 for (nat32 i=0;i<sf.Size();i++)
 {
  *out = sf[i].field.Get(pos) * sf[i].scale;
  ++out;
 }
}

void MeanShift::CalcShift(nat32 * mi,nat32 * ma,nat32 * pos,real32 * data,nat32 * stride,real32 * vector,real32 * mean)
{
 // Set the shift to 0...
  for (nat32 i=0;i<fvSize;i++) mean[i] = 0.0;
  real32 weight = 0.0;


 // Generate the minimum/maximum values for each coordinate, to save repeated 
 // recalculation in the inner loop...
  nat32 vecPos = 1;
  for (nat32 i=0;i<dim.Size();i++)
  {
   if (dim[i].use)
   {
    mi[i] = math::Max<int32>(0,int32((vector[vecPos]-1.0)/dim[i].scale));
    ma[i] = math::Min(sf[0].field.Size(i)-1,nat32(((vector[vecPos]+1.0)/dim[i].scale)+1));
    ++vecPos;
   }
   else
   {
    mi[i] = 0;
    ma[i] = sf[0].field.Size(i)-1;
   }
  }


 // Iterate every relevent node and sum in its contribution...
  for (nat32 i=0;i<dim.Size();i++) pos[i] = mi[i];
  
  bit ok = true;
  while (ok)
  {
   // Sum in this particular node...
    // Get the relevent pointer to the vector including the weight...
     real32 * targ = data;
     for (nat32 i=0;i<dim.Size();i++) targ += pos[i]*stride[i]*(fvSize+1);

    // Calculate the distance of this vector from our vector, if its too far skip it...
     if (Distance(fvSize,targ+1,vector+1,passThrough))
     {
      // The distance multiplied by the points weight is now the weighting for the point,
      // sum this in...
       real32 we = targ[0];
       weight += we;
       for (nat32 i=0;i<fvSize;i++) mean[i] += we*(targ[i+1] - vector[i+1]);
     }

   // Incriment to the next position...
    ++pos[0];

   // Do the pos incriment count along...
    for (nat32 i=0;i<dim.Size();i++)
    {
     if (pos[i]<=ma[i]) break;
     if (i==dim.Size()-1) {ok = false; break;}
     pos[i] = mi[i];
     ++pos[i+1];
    }
  }


 // Finally, divide through by the weight sum...
  if (weight>0.0)
  {
   weight = 1.0/weight;
   for (nat32 i=0;i<fvSize;i++) mean[i] = mean[i]*weight;
  }
}

bit MeanShift::DistDefault(nat32 fvSize,real32 * fv1,real32 * fv2,real32)
{
 real32 ret = 0.0;
 for (nat32 i=0;i<fvSize;i++)
 {
  ret += math::Sqr(fv1[i] - fv2[i]);
  if (ret>1.0) return false;
 }
 return true;
}

//------------------------------------------------------------------------------
 };
};
