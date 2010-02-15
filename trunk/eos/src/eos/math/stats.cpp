//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/math/stats.h"

#include "eos/math/gaussian_mix.h"
#include "eos/ds/stacks.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
UniDensityEstimate::UniDensityEstimate()
:size(0),sd(0.0)
{}

UniDensityEstimate::~UniDensityEstimate()
{}

void UniDensityEstimate::Add(real32 x)
{
 if (size==data.Size()) data.Size(data.Size()+incSize);
 
 data[size] = x;
 size += 1;
}

nat32 UniDensityEstimate::Samples() const
{
 return size;
}

void UniDensityEstimate::Run()
{
 LogTime("eos::math::UniDensityEstimate::Run");
 // Shrink the array down to the amount of data we have...
  data.Size(size);
  
 // Sort the array...
  data.Sort<ds::SortOp<real32> >();
  
 // Calculate the mean and standard deviation of the data set...
  Sample sample;
  for (nat32 i=0;i<data.Size();i++) sample.Add(data[i]);
 
 // Calculate the window size...
  sd = 1.06 * sample.Sd() * math::Pow<real32>(data.Size(),-1.0/5.0);
}

real32 UniDensityEstimate::Width() const
{
 return sd;
}

real32 UniDensityEstimate::F(real32 x) const
{
 real32 ret = 0.0;
  for (nat32 i=0;i<data.Size();i++) ret += Gaussian(sd,x-data[i]);
  
  //real32 divisor = sd * math::Sqrt(2.0*pi) * real32(data.Size());
  //if (!math::IsZero(divisor)) ret /= divisor;
  
 return ret;
}

real32 UniDensityEstimate::Lowest() const
{
 return data[0];
}

real32 UniDensityEstimate::Highest() const
{
 return data[data.Size()-1];
}

Vector<real32> * UniDensityEstimate::GetSamples(nat32 samples) const
{
 LogTime("eos::math::UniDensityEstimate::GetSamples");
 real32 min = Lowest();
 real32 mult = (Highest() - min)/real32(samples-1);

 Vector<real32> * ret = new Vector<real32>(samples);
  for (nat32 i=0;i<samples;i++)
  {
   (*ret)[i] = F(min + i*mult);
  }
 return ret;
}

real32 UniDensityEstimate::MaxMode(real32 tol,nat32 limit) const
{
 LogTime("eos::math::UniDensityEstimate::MaxMode");

 // Create a stack of ranges to be searched, put the entire range on to start 
 // with...
  ds::Stack<ToSearch> work;
  {
   ToSearch start;
    start.low.mode = false;
    start.low.limit = -math::Infinity<real32>();
    start.low.index = 0;
    start.high.mode = false;
    start.high.limit = math::Infinity<real32>();
    start.high.index = data.Size()-1;   
   work.Push(start);
  }


 // Now keep popping ranges off the stack and proccessing them, keeping track of
 // the best mode found...
  real32 bestPos = 0.0;
  real32 bestScore = -math::Infinity<real32>();
 
  while (work.Size()!=0)
  {
   // Grab the next range to search...
    ToSearch range = work.Peek();
    work.Pop();

   // If not bounded by a mode from below move upwards...
    if (range.low.mode==false)
    {
     real32 pos = data[range.low.index];
     for (nat32 i=0;i<limit;i++)
     {
      real32 newPos = 0.0;
      real32 div = 0.0;
      for (nat32 j=0;j<data.Size();j++)
      {
       real32 g = Gaussian(sd,pos-data[j]);
       newPos += g * data[j];
       div += g;
      }
      newPos /= div;
 
      real32 diff = math::Abs(pos-newPos);
      pos = newPos;

      if (diff<tol) break;
     }
     
     real32 score = F(pos);
     if (score>bestScore)
     {
      bestScore = score;
      bestPos = pos;
     }
     
     range.low.mode = true;
     range.low.limit = pos;
     while ((data[range.low.index]<range.low.limit)&&(range.low.index<range.high.index)) range.low.index += 1;
    }
   
   // If not bounded by a mode from above move downwards...
    if (range.high.mode==false)
    {
     real32 pos = data[range.high.index];
     for (nat32 i=0;i<limit;i++)
     {
      real32 newPos = 0.0;
      real32 div = 0.0;
      for (nat32 j=0;j<data.Size();j++)
      {
       real32 g = Gaussian(sd,pos-data[j]);
       newPos += g * data[j]; // /data.Size()
       div += g; // /data.Size()
      }
      newPos /= div;
 
      real32 diff = math::Abs(pos-newPos);
      pos = newPos;

      if (diff<tol) break;
     }
     
     real32 score = F(pos);
     if (score>bestScore)
     {
      bestScore = score;
      bestPos = pos;
     }
     
     range.high.mode = true;
     range.high.limit = pos;
     while ((data[range.high.index]>range.high.limit)&&(range.high.index>range.low.index)) range.high.index -= 1;
    }

   // If the range is so tight that the range can be considered done then finish
   // with this range...
    if ((range.low.index>=range.high.index)||((range.high.limit-range.low.limit)<(tol*10.0))) continue;
   
   // Select a centre sample and iterate, create two ranges from it and push 
   // them onto the stack...
   {
    nat32 startInd = (range.high.index+range.low.index)/2;
    real32 pos = data[startInd];
    for (nat32 i=0;i<limit;i++)
    {
     real32 newPos = 0.0;
     real32 div = 0.0;
     for (nat32 j=0;j<data.Size();j++)
     {
      real32 g = Gaussian(sd,pos-data[j]);
      newPos += g * data[j]; // /data.Size()
      div += g; // /data.Size()
     }
     newPos /= div;
 
     real32 diff = math::Abs(pos-newPos);
     pos = newPos;

     if (diff<tol) break;
     if ((pos<range.low.limit)||(pos>range.high.limit)) break;
    }
    
    if ((pos>=range.low.limit)&&(pos<=range.high.limit))
    {
     real32 score = F(pos);
     if (score>bestScore)
     {
      bestScore = score;
      bestPos = pos;
     }
     
     if (pos<data[startInd])
     {
      ToSearch lnr;
       lnr.low = range.low;
       lnr.high.mode = true;
       lnr.high.limit = pos;
       lnr.high.index = startInd-1;
       while ((data[lnr.high.index]>pos)&&(lnr.high.index>lnr.low.index)) lnr.high.index -= 1;
      work.Push(lnr);

      ToSearch hnr;
       hnr.low.mode = true;
       hnr.low.limit = data[startInd];
       hnr.low.index = startInd+1;
       hnr.high = range.high;
      work.Push(hnr);
     }
     else
     {
      ToSearch lnr;
       lnr.low = range.low;
       lnr.high.mode = true;
       lnr.high.limit = data[startInd];
       lnr.high.index = startInd-1;
      work.Push(lnr);
     
      ToSearch hnr;
       hnr.high = range.high;      
       hnr.low.mode = true;
       hnr.low.limit = pos;
       hnr.low.index = startInd+1;
       while ((data[hnr.low.index]<pos)&&(hnr.low.index<hnr.high.index)) hnr.low.index += 1;
      work.Push(hnr);
     }
    }
    else
    {
     if (pos<range.low.limit)
     {
      ToSearch nr;
       nr.low.mode = true;
       nr.low.limit = data[startInd];
       nr.low.index = startInd+1;
       nr.high = range.high;
      work.Push(nr);
     }
     else
     {
      ToSearch nr;
       nr.low = range.low;
       nr.high.mode = true;
       nr.high.limit = data[startInd];
       nr.high.index = startInd-1;
      work.Push(nr);   
     }
    }
   }  
  }


 return bestPos;
}

//------------------------------------------------------------------------------
 };
};
