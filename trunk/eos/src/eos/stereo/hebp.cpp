//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/stereo/hebp.h"

#include "eos/file/csv.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
HEBP::HEBP()
:occCostBase(1.0),occCostMult(-0.1),occLimMult(2.0),iters(8),outCount(1),
dsc(null<DSC*>()),dscOcc(null<DSC*>()),ebp(null<EBP*>())
{}

HEBP::~HEBP()
{
 delete dsc;
 delete dscOcc;
 delete ebp;
}

void HEBP::Set(real32 ocb,real32 ocm,real32 olm,nat32 i,nat32 outC)
{
 occCostBase = ocb;
 occCostMult = ocm;
 occLimMult = olm;
 iters = i;
 outCount = outC;
}

void HEBP::Set(DSC * d)
{
 delete dsc;
 dsc = d->Clone();
}

void HEBP::SetOcc(DSC * d)
{
 delete dscOcc;
 dscOcc = d->Clone();
}

void HEBP::Set(const svt::Field<bit> & lMask,const svt::Field<bit> & rMask)
{
 leftMask = lMask;
 rightMask = rMask;
}

void HEBP::Run(time::Progress * prog)
{
 LogBlock("eos::stereo::HEBP::Run","");
 prog->Push();

 // Build the dsc hierachy...
  HierarchyDSC hdsc;
  hdsc.Set(*dsc,leftMask,rightMask);
  
 // If theres a dscOcc build a hierachy for it as well...
  HierarchyDSC hdscOcc;
  if (dscOcc) hdscOcc.Set(*dscOcc,leftMask,rightMask);


 // Create the top level DSR....
  const DSC & topDSC = hdsc.Level(hdsc.Levels()-1);
  svt::Field<bit> topLeftMask = hdsc.LeftMask(hdsc.Levels()-1);
  svt::Field<bit> topRightMask = hdsc.RightMask(hdsc.Levels()-1);

  BasicDSR start(topDSC.WidthLeft(),topDSC.HeightLeft());
  for (nat32 y=0;y<topDSC.HeightLeft();y++)
  {
   for (nat32 x=0;x<topDSC.WidthLeft();x++)
   {
    if ((!topLeftMask.Valid())||(topLeftMask.Get(x,y)))
    {
     bit inRange = false;
     int32 startDisp = 0;
     for (nat32 x2=0;x2<topDSC.WidthRight();x2++)
     {
      if ((!topRightMask.Valid())||(topRightMask.Get(x2,y)))
      {
       if (!inRange)
       {
        inRange = true;
        startDisp = int32(x2) - int32(x);
       }
      }
      else
      {
       if (inRange)
       {
        inRange = false;
        start.Add(x,y,startDisp,int32(x2-1) - int32(x));
       }
      }
     }
     
     if (inRange) start.Add(x,y,startDisp,int32(topDSC.WidthRight()-1) - int32(x));
    }
   }
  }


 // Iterate down all levels of the hierachy, running the EBP algorithm for each
 // to generate a new DSR...
  // Create them, put in relevant parameters...
   ds::Array<EBP*> ll(hdsc.Levels());
   for (nat32 i=0;i<ll.Size();i++)
   {
    ll[i] = new EBP();
    ll[i]->Set(occCostBase,occCostMult,occLimMult,iters,outCount);
   }
   
  // Do the highest entry...
   prog->Report(0,ll.Size());
   ll[ll.Size()-1]->Set(&topDSC);
   ll[ll.Size()-1]->Set(&start);
   
   ll[ll.Size()-1]->Run(prog);

  // Do the rest...
   for (int32 i=ll.Size()-2;i>=0;i--)
   {
    prog->Report(ll.Size()-1-i,ll.Size());
    const DSC & here = hdsc.Level(i);
   
    HierachyDSR above;
    above.Set(here.WidthLeft(),here.HeightLeft());
    above.Set(*ll[i+1],search_range);
    above.Set(hdsc.LeftMask(i),hdsc.RightMask(i));
    
    SpreadDSR spreadAbove;
    spreadAbove.Set(above,spread_range,hdsc.LeftMask(i));
    
    ll[i]->Set(&here);
    if (dscOcc) ll[i]->SetOcc(&hdscOcc.Level(i));
    ll[i]->Set(&spreadAbove);
   
    ll[i]->Run(prog);
   }
 
 
 // Keep the final EBP, terminate the rest...
  ebp = ll[0];
  for (nat32 i=1;i<ll.Size();i++) delete ll[i];


 prog->Pop();
}

nat32 HEBP::Width() const
{
 return ebp->Width();
}

nat32 HEBP::Height() const
{
 return ebp->Height();
}

nat32 HEBP::Size(nat32 x, nat32 y) const
{
 return ebp->Size(x,y);
}

real32 HEBP::Disp(nat32 x, nat32 y, nat32 i) const
{
 return ebp->Disp(x,y,i);
}

real32 HEBP::Cost(nat32 x, nat32 y, nat32 i) const
{
 return ebp->Cost(x,y,i);
}

real32 HEBP::DispWidth(nat32 x, nat32 y, nat32 i) const
{
 return ebp->DispWidth(x,y,i);
}

cstrconst HEBP::TypeString() const
{
 return "eos::stereo::HEBP";
}

//------------------------------------------------------------------------------
 };
};
