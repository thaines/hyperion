//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/inf/gauss_integration.h"

#include "eos/file/csv.h"

namespace eos
{
 namespace inf
 {
//------------------------------------------------------------------------------
IntegrateBP::IntegrateBP(nat32 width,nat32 height)
:iters(100),zeroM(0)
{
 Reset(width,height);
}

IntegrateBP::~IntegrateBP()
{}

void IntegrateBP::Reset(nat32 width,nat32 height)
{
 LogTime("eos::inf::IntegrateBP::Reset");
 in.Size(height);
 for (nat32 i=0;i<height;i++) in[i].Size(width);
 out.Resize(width,height);

 for (nat32 y=0;y<height;y++)
 {
  for (nat32 x=0;x<width;x++)
  {
   in[y][x].lock = false;
   in[y][x].mean = 0.0;
   in[y][x].invSd = 0.0;
   for (nat32 i=0;i<4;i++)
   {
    in[y][x].rel[i].m = 1.0;
    in[y][x].rel[i].z = 0.0;
    in[y][x].rel[i].invSd = 0.0;
   }

   out.Get(x,y) = math::Gauss1D();
  }
 }
}

nat32 IntegrateBP::Width() const
{
 return out.Width();
}

nat32 IntegrateBP::Height() const
{
 return out.Height();
}

void IntegrateBP::SetVal(nat32 x,nat32 y,real32 mean,real32 invSd)
{
 in[y][x].lock = false;
 in[y][x].mean = mean;
 in[y][x].invSd = invSd;
}

void IntegrateBP::SetLocked(nat32 x,nat32 y,real32 val)
{
 in[y][x].lock = true;
 in[y][x].mean = val;
}

void IntegrateBP::SetRel(nat32 x,nat32 y,nat32 dir,real32 m,real32 z,real32 invSd)
{
 in[y][x].rel[dir].m = m;
 in[y][x].rel[dir].z = z;
 in[y][x].rel[dir].invSd = invSd;
}

void IntegrateBP::SetIters(nat32 i)
{
 iters = i;
}

void IntegrateBP::SetZeroMeaning(nat32 n)
{
 zeroM = n;
}

void IntegrateBP::Run(time::Progress * prog)
{
 LogDebug("eos::inf::IntegrateBP::Run");
 prog->Push();

 // Create and null the message passing data structure...
 // It also contains the input messages, so fill them in.
  prog->Report(0,iters+2);
  ds::ArrayDel< ds::Array<Node> > data(out.Height());
  for (nat32 i=0;i<data.Size();i++) data[i].Size(out.Width());

  for (nat32 y=0;y<out.Height();y++)
  {
   for (nat32 x=0;x<out.Width();x++)
   {
    // Expected value...
     if (in[y][x].lock==false)
     {
      data[y][x].exp.InvCoVar() = math::Sqr(in[y][x].invSd);
      data[y][x].exp.InvCoVarMean() = math::Sqr(in[y][x].invSd) * in[y][x].mean;
     }

    // Messages...
     for (nat32 i=0;i<4;i++) data[y][x].msg[i] = math::Gauss1D();
   }
  }


 // Update the input so that all off-grid relationships are set to invSd==0,
 // best that such memory damaging messages are not sent...
  for (nat32 y=0;y<out.Height();y++)
  {
   in[y][0].rel[2].invSd = 0.0;
   in[y][out.Width()-1].rel[0].invSd = 0.0;
  }

  for (nat32 x=0;x<out.Width();x++)
  {
   in[0][x].rel[3].invSd = 0.0;
   in[out.Height()-1][x].rel[1].invSd = 0.0;
  }


 // Go through and create an adjusted set of relationships between pixels to
 // remove bias from the results...
  prog->Report(1,iters+3);
  for (nat32 y=0;y<out.Height();y++)
  {
   for (nat32 x=0;x<out.Width();x++)
   {
    // Horizontal...
     if ((x+1)!=out.Width())
     {
      if (!(math::IsZero(in[y][x].rel[0].invSd)&&math::IsZero(in[y][x+1].rel[2].invSd)))
      {
       real32 invA = math::Sqr(in[y][x].rel[0].invSd);
       real32 invB = math::Sqr(in[y][x+1].rel[2].invSd);
       real32 div = 1.0/(invA+invB);

       in[y][x].rel[0].ubM = (invA*in[y][x].rel[0].m + invB/in[y][x+1].rel[2].m) * div;
       in[y][x].rel[0].ubZ = (invA*in[y][x].rel[0].z - (invB*in[y][x+1].rel[2].z/in[y][x+1].rel[2].m)) * div;

       in[y][x+1].rel[2].ubM = (invB*in[y][x+1].rel[2].m + invA/in[y][x].rel[0].m) * div;
       in[y][x+1].rel[2].ubZ = (invB*in[y][x+1].rel[2].z - (invA*in[y][x].rel[0].z/in[y][x].rel[0].m)) * div;
      }
     }

    // Vertical...
     if ((y+1)!=out.Height())
     {
      if (!(math::IsZero(in[y][x].rel[1].invSd)&&math::IsZero(in[y+1][x].rel[3].invSd)))
      {
       real32 invA = math::Sqr(in[y][x].rel[1].invSd);
       real32 invB = math::Sqr(in[y+1][x].rel[3].invSd);
       real32 div = 1.0/(invA+invB);

       in[y][x].rel[1].ubM = (invA*in[y][x].rel[1].m + invB/in[y+1][x].rel[3].m) * div;
       in[y][x].rel[1].ubZ = (invA*in[y][x].rel[1].z - (invB*in[y+1][x].rel[3].z/in[y+1][x].rel[3].m)) * div;

       in[y+1][x].rel[3].ubM = (invB*in[y+1][x].rel[3].m + invA/in[y][x].rel[1].m) * div;
       in[y+1][x].rel[3].ubZ = (invB*in[y+1][x].rel[3].z - (invA*in[y][x].rel[1].z/in[y][x].rel[1].m)) * div;
      }
     }
   }
  }


 // Pass messages (With a checkboard pattern.)...
  for (nat32 i=0;i<iters;i++)
  {
   prog->Report(2+i,iters+3);
   for (nat32 y=0;y<out.Height();y++)
   {
    for (nat32 x=(i+y)%2;x<out.Width();x+=2)
    {
     if (in[y][x].lock)
     {
      // Message calculation is far easier for locked nodes, simply use the
      // relationship to calculate the absolute value and then apply the invSd to
      // get the message...
       for (nat32 j=0;j<4;j++)
       {
        if (!math::IsZero(in[y][x].rel[j].invSd))
        {
         // Calculate message...
          real32 invVar = math::Sqr(in[y][x].rel[j].invSd);
          real32 val = in[y][x].mean*in[y][x].rel[j].ubM + in[y][x].rel[j].ubZ;

          math::Gauss1D msg;
          msg.InvCoVar() = invVar;
          msg.InvCoVarMean() = invVar * val;

         // Send message...
          switch (j)
          {
           case 0: data[y][x+1].msg[2] = msg; break;
           case 1: data[y+1][x].msg[3] = msg; break;
           case 2: data[y][x-1].msg[0] = msg; break;
           case 3: data[y-1][x].msg[1] = msg; break;
          }
        }
       }
     }
     else
     {
      // Calculate the accumulator of messages, from which we can subtract to get
      // the outputs...
       math::Gauss1D acc = data[y][x].exp;
       for (nat32 j=0;j<4;j++) acc *= data[y][x].msg[j];

      // Pass messages...
       for (nat32 j=0;j<4;j++)
       {
        if (!math::IsZero(in[y][x].rel[j].invSd))
        {
         // Remove the message from the direction we are sending from the
         // expectation...
          math::Gauss1D accSub = acc;
          accSub /= data[y][x].msg[j];

         // Combine the acc with the compatability distribution and marginalise
         // to generate our output message...
          math::Gauss2D msgExt(accSub);

          real32 m = in[y][x].rel[j].ubM;
          real32 z = in[y][x].rel[j].ubZ;
          real32 mult = 0.5 * math::Sqr(in[y][x].rel[j].invSd);

          math::Gauss2D co;
          co.InvCoVar()[0][0] = mult * math::Sqr(m);
          co.InvCoVar()[0][1] = -mult * m;
          co.InvCoVar()[1][0] = -mult * m;
          co.InvCoVar()[1][1] = mult;
          co.InvCoVarMean()[1] = -mult * z * m;
          co.InvCoVarMean()[0] =  mult * z;

          msgExt *= co;
          math::Gauss1D msg;
          msgExt.Marg1(msg);

         // Send message...
          switch (j)
          {
           case 0: data[y][x+1].msg[2] = msg; break;
           case 1: data[y+1][x].msg[3] = msg; break;
           case 2: data[y][x-1].msg[0] = msg; break;
           case 3: data[y-1][x].msg[1] = msg; break;
          }
        }
       }
     }
    }
   }
   
   // Zero mean if needed...
    if ((zeroM!=0)&&(((i+1)%zeroM)==0)) WeightedZeroMean(data);
  }
  
 // Once done zero mean again if needed...
  if (zeroM!=0) WeightedZeroMean(data);


 // Extract the results...
  prog->Report(2+iters,iters+3);
  for (nat32 y=0;y<out.Height();y++)
  {
   for (nat32 x=0;x<out.Width();x++)
   {
    if (!in[y][x].lock)
    {
     out.Get(x,y) = data[y][x].exp;
     for (nat32 i=0;i<4;i++) out.Get(x,y) *= data[y][x].msg[i];
    }
   }
  }


 prog->Pop();
}

bit IntegrateBP::Defined(nat32 x,nat32 y) const
{
 if (in[y][x].lock) return true;
               else return out.Get(x,y).Defined();
}

real32 IntegrateBP::Expectation(nat32 x,nat32 y) const
{
 if (in[y][x].lock) return in[y][x].mean;
               else return out.Get(x,y).Mean();
}

real32 IntegrateBP::StandardDeviation(nat32 x,nat32 y) const
{
 if (in[y][x].lock) return 0.0;
               else return out.Get(x,y).Sd();
}

void IntegrateBP::GetDefined(svt::Field<bit> & o) const
{
 for (nat32 y=0;y<out.Height();y++)
 {
  for (nat32 x=0;x<out.Width();x++)
  {
   o.Get(x,y) = Defined(x,y);
  }
 }
}

void IntegrateBP::GetExpectation(svt::Field<real32> & o) const
{
 for (nat32 y=0;y<out.Height();y++)
 {
  for (nat32 x=0;x<out.Width();x++)
  {
   o.Get(x,y) = Expectation(x,y);
  }
 }
}

void IntegrateBP::GetDeviation(svt::Field<real32> & o) const
{
 for (nat32 y=0;y<out.Height();y++)
 {
  for (nat32 x=0;x<out.Width();x++)
  {
   o.Get(x,y) = StandardDeviation(x,y);
  }
 }
}

void IntegrateBP::WeightedZeroMean(ds::ArrayDel< ds::Array<Node> > & data)
{
 // First calculate the mean, incrimentally - we (conveniantly) weight each by the inverse of variance...
  real32 mean = 0.0;
  real32 weight = 0.0;
  
  for (nat32 y=0;y<data.Size();y++)
  {
   for (nat32 x=0;x<data[0].Size();x++)
   {
    if (!in[y][x].lock)
    {
     math::Gauss1D est = data[y][x].exp;
     for (nat32 i=0;i<4;i++) est *= data[y][x].msg[i];
     
     if (!math::IsZero(est.InvCoVar()))
     {
      weight += est.InvCoVar();
      mean   += (est.InvCoVarMean() - mean*est.InvCoVar()) / weight;
     }
    }
   }
  }


 // Now offset everything by subtracting the calculated mean...
  for (nat32 y=0;y<data.Size();y++)
  {
   for (nat32 x=0;x<data[0].Size();x++)
   {
    data[y][x].exp.InvCoVarMean() -= mean * data[y][x].exp.InvCoVar();
   
    for (nat32 i=0;i<4;i++)
    {
     data[y][x].msg[i].InvCoVarMean() -= mean * data[y][x].msg[i].InvCoVar();
    }
   }
  }
}

//------------------------------------------------------------------------------
 };
};
