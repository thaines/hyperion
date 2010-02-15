//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/stereo/simpleBP.h"

#include "eos/alg/bp2d.h"
#include "eos/math/functions.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
SimpleBP::SimpleBP()
:minDisp(-30),maxDisp(30),
vLimit(1.7),dLimit(15.0/255.0),dMult(255.0*0.07),
output(null<svt::Var*>())
{}

SimpleBP::~SimpleBP()
{
 delete output;
}

void SimpleBP::SetPair(const svt::Field<real32> & l,const svt::Field<real32> & r)
{
 left = l;
 right = r;	
}

void SimpleBP::SetDisp(int32 minD,int32 maxD)
{
 minDisp = minD;
 maxDisp = maxD;	
}

void SimpleBP::SetParas(real32 vL,real32 dL,real32 dM)
{
 vLimit = vL;
 dLimit = dL;
 dMult = dM;	
}

void SimpleBP::Run(time::Progress * prog)
{
 // Create the bp object and suport objects...
  alg::HBP2D<alg::TruncLinearMsgBP2D<SimpleBP,SimpleBP::D,SimpleBP::VM,SimpleBP::VT> > bp;
  
  delete output;
  output = new svt::Var(left);
   int32 dispIni = 0;
   output->Add("disp",dispIni);
  output->Commit();
  
  output->ByName("disp",disp);
  
 // Set it up...   
  svt::Field<nat32> dispNat;
  disp.SubField(0,dispNat);
  bp.SetOutput(dispNat);
  bp.SetLabels(maxDisp-minDisp+1);
  bp.SetPT(*this);

  
 // Run...
  bp.Run(prog);
 
 // Adjust the results...
  for (nat32 y=0;y<disp.Size(1);y++)
  {
   for (nat32 x=0;x<disp.Size(0);x++)
   {
    disp.Get(x,y) += minDisp;
   }
  }
}

void SimpleBP::GetDisparity(svt::Field<int32> & out)
{
 out.CopyFrom(disp);	
}

//------------------------------------------------------------------------------
 };
};
