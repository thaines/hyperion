//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines
#include "eos/filter/grad_walk.h"

#include "eos/data/randoms.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
EOS_FUNC void GradWalk(const svt::Field<real32> & l,svt::Field<real32> & dx,svt::Field<real32> & dy,
                       nat32 walks,nat32 length,real32 exp,bit extend,
                       time::Progress * prog)
{
 prog->Push();
 data::Random rand;
 for (int32 y=0;y<int32(l.Size(1));y++)
 {
  prog->Report(y,l.Size(1));
  prog->Push();
  for (int32 x=0;x<int32(l.Size(0));x++)
  {
   prog->Report(x,l.Size(0));
   // For each pixel calculate a value by walking...
    dx.Get(x,y) = 0.0;
    dy.Get(x,y) = 0.0;

    for (nat32 w=0;w<walks;w++)
    {
     int32 tx = x,ty = y;

     for (nat32 le=0;le<length;le++)
     {
      // Make a step to an adjacent pixel - 4 to choose from...
       // Get the luminence values...
        real32 lum[4]; // Indexed by direction, defined in the normal +ve x, +ve y, -ve x, -ve y way.
        if (extend)
        {
         lum[0] = l.Get(math::Clamp<int32>(tx+1,0,int32(l.Size(0)-1)),math::Clamp<int32>(ty,0,int32(l.Size(1)-1)));
         lum[1] = l.Get(math::Clamp<int32>(tx,0,int32(l.Size(0)-1)),math::Clamp<int32>(ty+1,0,int32(l.Size(1)-1)));
         lum[2] = l.Get(math::Clamp<int32>(tx-1,0,int32(l.Size(0)-1)),math::Clamp<int32>(ty,0,int32(l.Size(1)-1)));
         lum[3] = l.Get(math::Clamp<int32>(tx,0,int32(l.Size(0)-1)),math::Clamp<int32>(ty-1,0,int32(l.Size(1)-1)));
        }
        else
        {
         if ((ty>=0)&&(ty<int32(l.Size(1))))
         {
          if (tx+1<int32(l.Size(0))) lum[0] = l.Get(tx+1,ty);
                                else lum[0] = 0.0;
          if (tx-1>=0) lum[2] = l.Get(tx-1,ty);
                  else lum[2] = 0.0;
         }
         else
         {
          lum[0] = 0.0;
          lum[2] = 0.0;
         }

         if ((tx>=0)&&(tx<int32(l.Size(0))))
         {
          if (ty+1<int32(l.Size(1))) lum[1] = l.Get(tx,ty+1);
                                else lum[1] = 0.0;
          if (ty-1>=0) lum[3] = l.Get(tx,ty-1);
                  else lum[3] = 0.0;
         }
         else
         {
          lum[1] = 0.0;
          lum[3] = 0.0;
         }
        }


       // Add the exponent to the luminence values...
        for (nat32 i=0;i<4;i++) lum[i] = math::Pow(lum[i],exp);

       // Select a direction, update target coordinate...
        nat32 dir = rand.Select(4,lum);
        switch (dir)
        {
         case 0:  tx++; break;
         case 1:  ty++; break;
         case 2:  tx--; break;
         default: ty--; break;
        }
     }

     dx.Get(x,y) += real32(tx-x);
     dy.Get(x,y) += real32(ty-y);
    }

    dx.Get(x,y) /= real32(walks);
    dy.Get(x,y) /= real32(walks);
  }
  prog->Pop();
 }
 prog->Pop();
}

//------------------------------------------------------------------------------
EOS_FUNC void GradWalkPerfect(const svt::Field<real32> & l,svt::Field<real32> & dx,svt::Field<real32> & dy,
                              nat32 length,real32 exp,real32 stopChance,time::Progress * prog)
{
 // Require 2 buffers to store propagation values in...
  ds::Array2D<real32> bufA(length*2+1,length*2+1);
  ds::Array2D<real32> bufB(length*2+1,length*2+1);

  ds::Array2D<real32> * from = &bufA;
  ds::Array2D<real32> * to = &bufB;


 // Now do the per-pixel calculation...
 prog->Push();
 for (int32 y=0;y<int32(l.Size(1));y++)
 {
  prog->Report(y,l.Size(1));
  prog->Push();
  for (int32 x=0;x<int32(l.Size(0));x++)
  {
   prog->Report(x,l.Size(0));
   // Fill in the starting weight, zero the gradiant vector...
    from->Get(length,length) = 1.0;
    dx.Get(x,y) = 0.0;
    dy.Get(x,y) = 0.0;
    real32 remains = 1.0; // How much of 1 is left to be added in, for the probalistically shorter walks.


   // Iterate through each step of the walk, diffusing the probabilities, bouncing between buffers...
    int32 bx = x-length;
    int32 by = y-length;
    for (nat32 step=0;step<length;step++)
    {
     // Zero out the to grid...
      for (int32 v=int32(length-step-1);v<=int32(length+step+1);v++)
      {
       for (int32 u=int32(length-step-1);u<=int32(length+step+1);u++) to->Get(u,v) = 0.0;
      }

     // Do the step...
      for (int32 v=int32(length-step);v<=int32(length+step);v++)
      {
       for (int32 u=int32(length-step);u<=int32(length+step);u++)
       {
        if (!math::IsZero(from->Get(u,v)))
        {
         // Calculate the weights for the neighbours...
          real32 weight[4];
          weight[0] = l.Get(math::Clamp<int32>(bx+u+1,0,int32(l.Size(0)-1)),math::Clamp<int32>(by+v,0,int32(l.Size(1)-1)));
          weight[1] = l.Get(math::Clamp<int32>(bx+u,0,int32(l.Size(0)-1)),math::Clamp<int32>(by+v+1,0,int32(l.Size(1)-1)));
          weight[2] = l.Get(math::Clamp<int32>(bx+u-1,0,int32(l.Size(0)-1)),math::Clamp<int32>(by+v,0,int32(l.Size(1)-1)));
          weight[3] = l.Get(math::Clamp<int32>(bx+u,0,int32(l.Size(0)-1)),math::Clamp<int32>(by+v-1,0,int32(l.Size(1)-1)));

         // Modify by the exponential term...
          for (nat32 i=0;i<4;i++) weight[i] = 0.0001 + math::Pow(weight[i],exp); // 0.001 to avoid div by zero.

         // Normalise them by the weight from the from grid...
          real32 sum = 0.0;
          for (nat32 i=0;i<4;i++) sum += weight[i];
          real32 mult = from->Get(u,v)/sum;
          for (nat32 i=0;i<4;i++) weight[i] *= mult;

         // Add them to the to grid...
          to->Get(u+1,v) += weight[0];
          to->Get(u,v+1) += weight[1];
          to->Get(u-1,v) += weight[2];
          to->Get(u,v-1) += weight[3];
        }
       }
      }

     // Swap for the next pass...
      math::Swap(from,to);

     // Sum into the gradient all the walks that have ended at this point...
      if (!math::IsZero(stopChance))
      {
       real32 weight = stopChance * math::Pow(real32(1.0-stopChance),real32(step));
       remains -= weight;

       for (int32 v=int32(length-step-1);v<=int32(length+step+1);v++)
       {
        for (int32 u=int32(length-step-1);u<=int32(length+step+1);u++)
        {
         dx.Get(x,y) += real32(u-int32(length)) * from->Get(u,v) * weight;
         dy.Get(x,y) += real32(v-int32(length)) * from->Get(u,v) * weight;
        }
       }
      }
    }

   // Sum the gradiant vector from the field of probability values, to make sure the final sum is 1...
    for (int32 v=0;v<int32(from->Height());v++)
    {
     for (int32 u=0;u<int32(from->Width());u++)
     {
      dx.Get(x,y) += real32(u-int32(length)) * from->Get(u,v) * remains;
      dy.Get(x,y) += real32(v-int32(length)) * from->Get(u,v) * remains;
     }
    }
  }
  prog->Pop();
 }
 prog->Pop();
}

//------------------------------------------------------------------------------
GradWalkSelect::GradWalkSelect()
:length(8),exp(6.0),bufA(8*2+1,8*2+1),bufB(8*2+1,8*2+1)
{}

GradWalkSelect::~GradWalkSelect()
{}

void GradWalkSelect::SetInput(const svt::Field<real32> & ll)
{
 l = ll;
}

void GradWalkSelect::SetParas(nat32 l,real32 e)
{
 length = l;
 exp = e;
}

void GradWalkSelect::Query(nat32 x,nat32 y,real32 & dx,real32 & dy)
{
 // Prep the buffers...
  ds::Array2D<real32> * from = &bufA;
  ds::Array2D<real32> * to = &bufB;
  
  from->Get(length,length) = 1.0;


 // Do the walks...
  for (nat32 step=0;step<length;step++)
  {
   // Zero out the to grid...
    for (int32 v=int32(length-step-1);v<=int32(length+step+1);v++)
    {
     for (int32 u=int32(length-step-1);u<=int32(length+step+1);u++) to->Get(u,v) = 0.0;
    }
    
   // Do the step...
    int32 bx = int32(x) - int32(length);
    int32 by = int32(y) - int32(length);
    for (int32 v=int32(length-step);v<=int32(length+step);v++)
    {
     for (int32 u=int32(length-step);u<=int32(length+step);u++)
     {
      if (!math::IsZero(from->Get(u,v)))
      {
       // Calculate the weights for the neighbours...
        real32 weight[4];
        weight[0] = l.Get(math::Clamp<int32>(bx+u+1,0,int32(l.Size(0)-1)),
                          math::Clamp<int32>(by+v,0,int32(l.Size(1)-1)));
        weight[1] = l.Get(math::Clamp<int32>(bx+u,0,int32(l.Size(0)-1)),
                          math::Clamp<int32>(by+v+1,0,int32(l.Size(1)-1)));
        weight[2] = l.Get(math::Clamp<int32>(bx+u-1,0,int32(l.Size(0)-1)),
                          math::Clamp<int32>(by+v,0,int32(l.Size(1)-1)));
        weight[3] = l.Get(math::Clamp<int32>(bx+u,0,int32(l.Size(0)-1)),
                          math::Clamp<int32>(by+v-1,0,int32(l.Size(1)-1)));

       // Modify by the exponential term...
        for (nat32 i=0;i<4;i++) weight[i] = 0.0001 + math::Pow(weight[i],exp); // 0.001 to avoid div by zero.

       // Normalise them by the weight from the from grid...
        real32 sum = 0.0;
        for (nat32 i=0;i<4;i++) sum += weight[i];
        real32 mult = from->Get(u,v)/sum;
        for (nat32 i=0;i<4;i++) weight[i] *= mult;

       // Add them to the to grid...
        to->Get(u+1,v) += weight[0];
        to->Get(u,v+1) += weight[1];
        to->Get(u-1,v) += weight[2];
        to->Get(u,v-1) += weight[3];
      }
     }
    }

   // Swap for the next pass...
    math::Swap(from,to);
  }


 // Sum out the final mean direction...
  dx = 0.0;
  dy = 0.0;
  for (int32 v=0;v<int32(from->Height());v++)
  {
   for (int32 u=0;u<int32(from->Width());u++)
   {
    dx += real32(u-int32(length)) * from->Get(u,v);
    dy += real32(v-int32(length)) * from->Get(u,v);
   }
  }
}

cstrconst GradWalkSelect::TypeString() const
{
 return "eos::filter::GradWalkSelect";
}

//------------------------------------------------------------------------------
 };
};
