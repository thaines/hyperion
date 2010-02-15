//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "bp_test/main.h"

//------------------------------------------------------------------------------
int main()
{
 using namespace eos;

 os::Con conObj;
 os::Conversation & con = *conObj.StartConversation();
 
 str::TokenTable tt;
 svt::Core core(tt);
  
 inf::BinBP2D bp;
 bp.SetSize(32,32);
 bp.SetMomentum(0.01);


 for (nat32 y=0;y<bp.Height();y++)
 {
  for (nat32 x=0;x<bp.Width();x++)
  {
   bp.SetCost(x,y,(((x/2)+(y/2))%2)==0,1.5);
   bp.SetCostX(x,y,false,true,1.0);
   bp.SetCostX(x,y,true,false,1.0);
   bp.SetCostY(x,y,false,true,1.0);
   bp.SetCostY(x,y,true,false,1.0);
  }
 } 
 
 
 bp.Run(&con.BeginProg());
 con.EndProg();


 for (nat32 y=0;y<bp.Height();y++)
 {
  con << "|";
  for (nat32 x=0;x<bp.Width();x++)
  {
   if (bp.State(x,y)) con << "+";
                 else con << " ";
   con << "|";
  }
  con << "\n";
 }
 con << "\n";


 return 0;
}

//------------------------------------------------------------------------------

