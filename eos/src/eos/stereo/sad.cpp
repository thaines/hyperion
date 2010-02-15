//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/stereo/sad.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
Sad::Sad()
:radius(1),minDisp(-30),maxDisp(30),maxDiff(1e2)
{}

Sad::~Sad()
{}

void Sad::AddField(const svt::Field<real32> & left,const svt::Field<real32> & right)
{
 nat32 ind = in.Size();
 in.Size(ind+1);
 in[ind].first = left;
 in[ind].second = right;
}

void Sad::SetRadius(nat32 r)
{
 radius = r;
}

void Sad::SetRange(int32 minD,int32 maxD)
{
 minDisp = minD;
 maxDisp = maxD;
}

nat32 Sad::Depth()
{
 return 1+maxDisp-minDisp;
}

void Sad::SetMaxDiff(real32 maxD)
{
 maxDiff = maxD;
}

void Sad::SetOutput(svt::Field<real32> & o)
{
 out = o;
}

void Sad::Run(time::Progress * prog)
{
 prog->Push();

 // Some variables...
  nat32 width = in[0].first.Size(0);
  nat32 height = in[0].first.Size(1);

 // In the first pass we calculate across each x line for all disparities 
 // completly ignoring the y. In other words we calculate for window length 
 // slices instead of entire windows. This is done using a standard 'take the
 // last value, subtract the bit leaving the window, add the bit entering 
 // techneque'.
  for (int32 d=minDisp;d<=maxDisp;d++)
  {
   for (nat32 y=0;y<height;y++)
   {
    // For each line of the stereo pair...
     real64 lastValue = 0.0;
     // Calculate the first value of the line, so we can work from it...
      for (int32 u=-radius;u<=int32(radius);u++)
      {
       lastValue += AD(0 + u,y,d);
      }
      out.Get(0,y,d-minDisp) = lastValue;
     

    // Iterate through the rest of the line, calculating each value from the last...
     for (nat32 x=1;x<width;x++)
     {
      // Subtract the part moving out of the window...
       lastValue -= AD(x - radius - 1,y,d);
      // Add in the part moving into the window...
       lastValue += AD(x + radius,y,d);
      // Write out...
       out.Get(x,y,d-minDisp) = lastValue;
     }
   }
  }


 // We then iterate down each Y column, using a sliding window of subtractions
 // and additions to calculate the final sad value...
 // (A little more complicated due to working in-place and working with entire
 // slices, we have to use a sliding window to store previous values.)
  real32 maxSliceDiff = 0.0;//maxDiff * (radius*2 + 1);
  ds::Array<real32> window(radius+1);
  nat32 winPos; // Above window array is a circular buffer, this is the current position.

  for (int32 d=minDisp;d<=maxDisp;d++)
  {
   for (nat32 x=0;x<width;x++)
   {
    // Initialise the window...
     winPos = radius;
     for (nat32 i=0;i<radius;i++) window[i] = maxSliceDiff;
     window[radius] = out.Get(x,0,d-minDisp);

    // Calculate the value for the first entry in the line...
     real64 lastValue = maxSliceDiff*radius;
     for (nat32 i=0;i<=radius;i++) lastValue += out.Get(x,i,d-minDisp);
     out.Get(x,0,d-minDisp) = lastValue;

    // Then iterate the entire column, writting out the final value for every last entry...
     for (nat32 y=1;y<height;y++)
     {
      // Subtract the exitting value and move the window on one...
       ++winPos;
       lastValue -= window[winPos%(radius+1)];
       window[winPos%(radius+1)] = out.Get(x,y,d-minDisp);
      // Add in the entering value...
       if ((y+radius)<height) lastValue += out.Get(x,y+radius,d-minDisp);
                         else lastValue += maxSliceDiff;
      // Write out...
       out.Get(x,y,d-minDisp) = lastValue;
     }
   }
  }

 prog->Pop();
}

//------------------------------------------------------------------------------
 };
};
