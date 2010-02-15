//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include <stdio.h>
#include <iostream>
#include <iomanip>

#include "fg_test/main.h"

using namespace eos;

//------------------------------------------------------------------------------
// Constants used by tests 5 through to 7...
static const nat32 labels = 4;
static const nat32 side = 4;
static const nat32 corner[4] = {0,3,0,3};
static const real32 nCost[4] = {8,10,4,10};
static const real32 smoothMult = 1.0;
static const real32 smoothMax = 10.0;

// Functions used by the BP implimentation to provide costs etc.
real32 D(void * const & data,nat32 x,nat32 y,nat32 label)
{
 if (x==0)
 {
  if ((y==0)&&(label!=corner[0])) return nCost[0];
  if ((y==side-1)&&(label!=corner[2])) return nCost[2];
 }

 if (x==side-1)
 {
  if ((y==0)&&(label!=corner[1])) return nCost[1];
  if ((y==side-1)&&(label!=corner[3])) return nCost[3];
 }

 return 0.0;
}

real32 VM(void * const & data)
{
 return smoothMult;
}

real32 VT(void * const & data)
{
 return smoothMax;
}

//------------------------------------------------------------------------------
int main(int argc,char ** argv)
{
 os::Con console;
 os::Conversation & con = *console.StartConversation();



 {
  LogBlock("test 1","-");
  
  inf::FactorGraph fg(false,false);
  
   // Function A...
    inf::Distribution * funcA = new inf::Distribution(1,2);   
     real32 distA[2] = {0.3,0.7};
     funcA->SetFreq(0,distA);   
    nat32 A = fg.MakeFuncs(funcA,1);

   // Function B...
    inf::Distribution * funcB = new inf::Distribution(1,3);
     real32 distB[3] = {0.25,0.5,0.25};
     funcB->SetFreq(0,distB);   
    nat32 B = fg.MakeFuncs(funcB,1);

   // Function C...
    inf::JointDistribution * funcC = new inf::JointDistribution(1,3);
     nat32 labSize[3] = {2,3,2};
     funcC->SetLabels(labSize);
    
     nat32 labInd[3];
     labInd[0] = 0; labInd[1] = 0; labInd[2] = 0; funcC->Get(labInd,0) = 0.2;
     labInd[0] = 0; labInd[1] = 0; labInd[2] = 1; funcC->Get(labInd,0) = 0.8;
     labInd[0] = 0; labInd[1] = 1; labInd[2] = 0; funcC->Get(labInd,0) = 0.4;
     labInd[0] = 0; labInd[1] = 1; labInd[2] = 1; funcC->Get(labInd,0) = 0.6;
     labInd[0] = 0; labInd[1] = 2; labInd[2] = 0; funcC->Get(labInd,0) = 0.9;
     labInd[0] = 0; labInd[1] = 2; labInd[2] = 1; funcC->Get(labInd,0) = 0.1;
     labInd[0] = 1; labInd[1] = 0; labInd[2] = 0; funcC->Get(labInd,0) = 0.1;
     labInd[0] = 1; labInd[1] = 0; labInd[2] = 1; funcC->Get(labInd,0) = 0.9;
     labInd[0] = 1; labInd[1] = 1; labInd[2] = 0; funcC->Get(labInd,0) = 0.6;
     labInd[0] = 1; labInd[1] = 1; labInd[2] = 1; funcC->Get(labInd,0) = 0.4;
     labInd[0] = 1; labInd[1] = 2; labInd[2] = 0; funcC->Get(labInd,0) = 0.5;
     labInd[0] = 1; labInd[1] = 2; labInd[2] = 1; funcC->Get(labInd,0) = 0.5;

    nat32 C = fg.MakeFuncs(funcC,1);   

   // Fix C...
    inf::Distribution * fixC = new inf::Distribution(1,2);
     real32 fixCdist[3] = {0.0,1.0};
     fixC->SetFreq(0,fixCdist);   
    nat32 Cf = fg.MakeFuncs(fixC,1);


   // Random variables, a,b and c...
    inf::Variable * var = new inf::Variable();
   
    fg.MakeLink(var,A,0,0);
    fg.MakeLink(var,C,0,0);    
    nat32 vA = fg.MakeVar(var);
    
    fg.MakeLink(var,B,0,0);
    fg.MakeLink(var,C,0,1);    
    nat32 vB = fg.MakeVar(var);

    fg.MakeLink(var,C,0,2);
    fg.MakeLink(var,Cf,0,0);
    nat32 vC = fg.MakeVar(var);
       
    delete var;


  // Run...
   if (fg.Run(&con.BeginProg())==false)
   {
    con << "Not a tree\n";
    con.EndProg();
    return 1;
   }
   con.EndProg();
  
  // Print results...
   con << "Solved:\n"; 

   inf::Frequency rA = fg.Prob<inf::Frequency>(vA);
   inf::Frequency rB = fg.Prob<inf::Frequency>(vB);
   inf::Frequency rC = fg.Prob<inf::Frequency>(vC);  
   
   con << "A: " << rA << "\n";
   con << "B: " << rB << "\n";
   con << "C: " << rC << "\n";
 }



 {
  LogBlock("test 2","-");

  inf::FactorGraph fg(false,true);
   fg.SetIters(10);
  static const nat32 labels = 7;
  
  
  inf::Distribution * dist = new inf::Distribution(2,labels);
  
  con << " 1:";
   for (nat32 i=0;i<labels;i++)
   {
    real32 p = math::Abs(math::Sin(0.5*math::pi*real32(i)/real32(labels)));
    dist->SetFreq(0,i,p);
    con << " " << p;
   }
  con << "\n";

  con << " 2:";
   for (nat32 i=0;i<labels;i++)
   {
    real32 p = math::Abs(math::Sin(0.5*math::pi*real32(i)/real32(labels) + 0.25*math::pi));
    dist->SetFreq(1,i,p);
    con << " " << p;
   }
  con << "\n";


  inf::EqualGaussian * equal = new inf::EqualGaussian(1,labels);
  equal->Set(0,0.1,1.5);
  
  nat32 dFunc = fg.MakeFuncs(dist,2);
  nat32 eFunc = fg.MakeFuncs(equal,1);
  
  
  inf::Variable var;
  
  fg.MakeLink(&var,dFunc,0,0);
  fg.MakeLink(&var,eFunc,0,0);
  nat32 v1 = fg.MakeVar(&var);
  
  fg.MakeLink(&var,dFunc,1,0);
  fg.MakeLink(&var,eFunc,0,1);
  nat32 v2 = fg.MakeVar(&var);
  
  
  fg.Run(&con.BeginProg());
  con.EndProg();
  
  inf::Frequency p1 = fg.Prob<inf::Frequency>(v1);
  inf::Frequency p2 = fg.Prob<inf::Frequency>(v2);
  
  con << "p1:";
  for (nat32 i=0;i<labels;i++) con << " " << p1[i];   
  con << "\n";
  
  con << "p2:";
  for (nat32 i=0;i<labels;i++) con << " " << p2[i];
  con << "\n\n\n";  
 }
 
 
 
 {
  LogBlock("test 3","-");
  
  inf::FactorGraph fg(false,true);
   fg.SetIters(50);
  
  // The distribution functions... 
   inf::Distribution * f1Obj = new inf::Distribution(1,8);
   inf::Distribution * f2Obj = new inf::Distribution(1,15);
   inf::Distribution * f3Obj = new inf::Distribution(1,29);
  
   /*real32 f1Freq[8] = {0.4,0.6,0.8,1.0,0.8,0.6,0.4,0.2};
   real32 f2Freq[15] = {0.2,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0,0.9,0.8,0.7,0.6};
   real32 f3Freq[29] = {0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0,0.9,0.8,
                        0.7,0.6,0.5,0.4,0.3,0.2,0.1,0.2,0.3,0.4,
                        0.5,0.4,0.3,0.2,0.1,0.2,0.3,0.4,0.5};*/
  
   real32 f1Freq[8] = {0.3,0.3,0.3,1.0,0.3,0.3,0.3,0.3};
   real32 f2Freq[15] = {0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3,1.0,0.3,0.3,0.3,0.3};
   real32 f3Freq[29] = {0.3,0.3,0.3,0.3,0.3,0.3,0.3,1.0,0.3,0.3,
                        0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3,
                        0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3,0.3};

   f1Obj->SetFreq(0,f1Freq);
   f2Obj->SetFreq(0,f2Freq);
   f3Obj->SetFreq(0,f3Freq);
  
   nat32 f1 = fg.MakeFuncs(f1Obj,1);
   nat32 f2 = fg.MakeFuncs(f2Obj,1);
   nat32 f3 = fg.MakeFuncs(f3Obj,1);
  
  // The difference functions...
   nat32 d1 = fg.MakeFuncs(new inf::DifferencePotts(8),1);
   nat32 d2 = fg.MakeFuncs(new inf::DifferencePotts(8),1);
   nat32 d3 = fg.MakeFuncs(new inf::DifferencePotts(15),1);
   
  // The variables...
   inf::Variable var;
  
   eos::log::Assert(fg.MakeLink(&var,f1,0,0));
   eos::log::Assert(fg.MakeLink(&var,d1,0,2));
   nat32 v1 = fg.MakeVar(&var);

   eos::log::Assert(fg.MakeLink(&var,d1,0,1));
   eos::log::Assert(fg.MakeLink(&var,d2,0,2));
   nat32 v2 = fg.MakeVar(&var);
 
   eos::log::Assert(fg.MakeLink(&var,d2,0,1));
   nat32 v3 = fg.MakeVar(&var);
   
   eos::log::Assert(fg.MakeLink(&var,f2,0,0));
   eos::log::Assert(fg.MakeLink(&var,d1,0,0));
   eos::log::Assert(fg.MakeLink(&var,d3,0,2));  
   nat32 v4 = fg.MakeVar(&var);  

   eos::log::Assert(fg.MakeLink(&var,d3,0,1));
   eos::log::Assert(fg.MakeLink(&var,d2,0,0));  
   nat32 v5 = fg.MakeVar(&var);  
 
   eos::log::Assert(fg.MakeLink(&var,d3,0,0));
   eos::log::Assert(fg.MakeLink(&var,f3,0,0));  
   nat32 v6 = fg.MakeVar(&var);

  // Runtime...
   fg.Run(&con.BeginProg());
   con.EndProg();
   //fg.FromNegLn();
 
  // Results... 
   inf::Frequency v1f = fg.Prob<inf::Frequency>(v1);
   inf::Frequency v2f = fg.Prob<inf::Frequency>(v2);
   inf::Frequency v3f = fg.Prob<inf::Frequency>(v3);
   inf::Frequency v4f = fg.Prob<inf::Frequency>(v4);
   inf::Frequency v5f = fg.Prob<inf::Frequency>(v5);
   inf::Frequency v6f = fg.Prob<inf::Frequency>(v6);
   
   con << "v1: " << v1f << "\n";
   con << "v2: " << v2f << "\n";
   con << "v3: " << v3f << "\n\n";
   
   con << "v4: " << v4f << "\n";
   con << "v5: " << v5f << "\n";
   con << "v6: " << v6f << "\n\n\n";
   
   std::cout << "Most probable:\n";
   if (fg.DoMS())
   {
    con << "v1 = " << v1f.Lowest() << "\n";
    con << "v2 = " << v2f.Lowest() << "\n";
    con << "v3 = " << v3f.Lowest() << "\n";
    con << "v4 = " << v4f.Lowest() << "\n";
    con << "v5 = " << v5f.Lowest() << "\n";
    con << "v6 = " << v6f.Lowest() << "\n";
   }
   else
   {
    con << "v1 = " << v1f.Highest() << "\n";
    con << "v2 = " << v2f.Highest() << "\n";
    con << "v3 = " << v3f.Highest() << "\n";
    con << "v4 = " << v4f.Highest() << "\n";
    con << "v5 = " << v5f.Highest() << "\n";
    con << "v6 = " << v6f.Highest() << "\n";   
   }
   con << "\n\n";
 }
 
 
 
 {
  LogBlock("test 4","-");
  
  inf::FactorGraph fg(false,true);
   fg.SetIters(50);
   
   const nat32 labels = 12;
   const nat32 steps = 8;

  // Create a 1D array of difference functions...
   nat32 ds = fg.MakeFuncs(new inf::DifferencePotts(labels,0.1),steps-1);
   
  // Create a distribution to set the first variable and the difference...
   inf::Distribution * distA = new inf::Distribution(1,labels);
   real32 freqA[labels] = {0.1,0.1,0.1,1.0,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1};
   distA->SetFreq(0,freqA);
   nat32 da = fg.MakeFuncs(distA,1);

   inf::Distribution * distB = new inf::Distribution(1,labels*2-1);
   real32 freqB[labels*2-1] = {0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,
                               1.0,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1};
   distB->SetFreq(0,freqB);
   nat32 db = fg.MakeFuncs(distB,1);


  // Create a line of variables, linking them up such that they should
  // form a sequence with constant difference...
  // Additionally add in one variable connected to all the differences, 
  // so they can be fixed to one value.
   inf::Variable var;
   nat32 varInd[steps];
   for (nat32 i=0;i<steps;i++)
   {
    if (i==0) eos::log::Assert(fg.MakeLink(&var,da,0,0));
    if (i!=0) eos::log::Assert(fg.MakeLink(&var,ds,i-1,1));
    if (i!=steps-1) eos::log::Assert(fg.MakeLink(&var,ds,i,2));
    
    varInd[i] = fg.MakeVar(&var);
   }
   
   for (nat32 i=0;i<steps-1;i++) eos::log::Assert(fg.MakeLink(&var,ds,i,0));
   eos::log::Assert(fg.MakeLink(&var,db,0,0));
   /*nat32 varDif =*/ fg.MakeVar(&var);


  // Run...
   fg.Run(&con.BeginProg());
   con.EndProg();
   
  // Extract the sequence discovered...
   for (nat32 i=0;i<steps;i++)
   {
    inf::Frequency v = fg.Prob<inf::Frequency>(varInd[i]);

    con << i << ": " << v << "\n";
    con << i << ": best = " << v.Highest() << "\n";
   }
   con << "\n\n";
 }


  
 {
  LogBlock("test 5","-");

  inf::FactorGraph fg(true,true);
  fg.SetIters(1000);


  inf::Distribution * dist[4];
  for (nat32 i=0;i<4;i++)
  {
   dist[i] = new inf::Distribution(1,labels);
   for (nat32 j=0;j<labels;j++)
   {
    if (j==corner[i]) dist[i]->SetCost(0,j,0);
                 else dist[i]->SetCost(0,j,nCost[i]);
   }
  }
  
  inf::EqualLaplace * eq = new inf::EqualLaplace(1,labels);
  eq->SetMS(0,smoothMult,smoothMax);


  nat32 distH[4];
  for (nat32 i=0;i<4;i++) distH[i] = fg.MakeFuncs(dist[i],1);
  nat32 eqH = fg.MakeFuncs(eq,side*(side-1)*2);


  inf::Variable var;
  ds::Array2D<nat32> vh(side,side);
  for (nat32 y=0;y<side;y++)
  {
   for (nat32 x=0;x<side;x++)
   {
    if ((x==0)&&(y==0)) fg.MakeLink(&var,distH[0],0,0);
    if ((x==(side-1))&&(y==0)) fg.MakeLink(&var,distH[1],0,0);
    if ((x==0)&&(y==(side-1))) fg.MakeLink(&var,distH[2],0,0);
    if ((x==(side-1))&&(y==(side-1))) fg.MakeLink(&var,distH[3],0,0);

    if (x!=(side-1)) fg.MakeLink(&var,eqH,y*(side-1) + x,0);
    if (x!=0) fg.MakeLink(&var,eqH,y*(side-1) + x-1,1);

    if (y!=(side-1)) fg.MakeLink(&var,eqH,side*(side-1) + x*(side-1) + y,0);
    if (y!=0) fg.MakeLink(&var,eqH,side*(side-1) + x*(side-1) + y-1,1);
   
    vh.Get(x,y) = fg.MakeVar(&var);
   }
  }
  
  
  fg.Run(&con.BeginProg());
  con.EndProg();
  //fg.FromNegLn();
  
  for (nat32 y=0;y<side;y++)
  {
   for (nat32 x=0;x<side;x++)
   {
    inf::Frequency v = fg.Prob<inf::Frequency>(vh.Get(x,y));
    
    con << "(" << x << "," << y << "): " << v << " = " << v.Lowest() << "\n";
   }
  }
  con << "\n\n";
 }



 {
  LogBlock("test 6","-");


  inf::FieldGraph fg = inf::FieldGraph(true);


  inf::GridFreq2D * freq = new inf::GridFreq2D(labels,side,side);
  for (nat32 y=0;y<side;y++)
  {
   for (nat32 x=0;x<side;x++)
   {
    for (nat32 l=0;l<labels;l++) freq->SetCost(x,y,l,0.0);
   }
  }
  
  for (nat32 i=0;i<labels;i++)
  {
   if (corner[0]!=i) freq->SetCost(0,0,i,nCost[0]);
   if (corner[1]!=i) freq->SetCost(side-1,0,i,nCost[1]);
   if (corner[2]!=i) freq->SetCost(0,side-1,i,nCost[2]);
   if (corner[3]!=i) freq->SetCost(side-1,side-1,i,nCost[3]);
  }
  
  
  inf::GridSmoothLaplace2D * sx = new inf::GridSmoothLaplace2D(false);
  inf::GridSmoothLaplace2D * sy = new inf::GridSmoothLaplace2D(true);
  sx->SetMS(smoothMult,smoothMax);
  sy->SetMS(smoothMult,smoothMax);
  
  
  nat32 freqH = fg.NewFP(freq);
  nat32 sxH = fg.NewFP(sx);
  nat32 syH = fg.NewFP(sy);
  
  nat32 vH = fg.NewVP();
  
  fg.Lay(freqH,0,vH);
  fg.Lay(sxH,0,vH);
  fg.Lay(syH,0,vH);
  
  bit res = fg.Run(&con.BeginProg());
  con.EndProg();
  
  if (res)
  {
   for (nat32 y=0;y<side;y++)
   {
    for (nat32 x=0;x<side;x++)
    {
     inf::Frequency v = fg.Get(vH,y*side+x);
     con << "(" << x << "," << y << "): " << v << " = " << v.Highest() << "\n";
    }
   }
   con << "\n\n";
  }
  else
  {
   LogDebug("Error with test 6");
  }
 }
 
 
 
 {
  LogBlock("test 7","-");
  
  str::TokenTable tt;
  svt::Core core(tt);

  nat32 natIni = 0;
  real32 realIni = 0.0;

  svt::Var outVar(core);
  outVar.Setup2D(side,side);
  outVar.Add("out",natIni);
  outVar.Commit();
  
  svt::Var debVar(core);
  debVar.Setup3D(side,side,labels);
  debVar.Add("deb",realIni);
  debVar.Commit();
  
  svt::Field<nat32> out(&outVar,"out");
  svt::Field<real32> deb(&debVar,"deb");


  alg::HBP2D<alg::TruncLinearMsgBP2D<void*,D,VM,VT> > bp;
  bp.SetLabels(labels);
  bp.SetIterations (32,1000);
  bp.SetOutput(out);
  bp.SetDebugOutput(deb);
  
  bp.Run(&con.BeginProg());
  con.EndProg();
  
  for (nat32 y=0;y<side;y++)
  {
   for (nat32 x=0;x<side;x++)
   {
    con << "(" << x << "," << y << "): <";
    for (nat32 i=0;i<labels;i++)
    {
     con << deb.Get(x,y,i);
     if (i+1!=labels) con << ",";
    }
    con << "> = " << out.Get(x,y) << "\n";
   }
  }
 }


 return 0;
}

//------------------------------------------------------------------------------
