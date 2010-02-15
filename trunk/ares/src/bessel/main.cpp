//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "bessel/main.h"

//------------------------------------------------------------------------------
int main(int argc,char ** argv)
{
 using namespace eos;

 os::Con conObj;
 os::Conversation & con = *conObj.StartConversation();
 
 if (argc!=3)
 {
  con << "Calculates the modified bessel function of the first kind.\n";
  con << "Usage: bessel order value\n";
  con << "order - The order, times two, an integer.\n";
  con << "value - The value to calculate the bessel function for\n.";
 }
 
 nat32 orderX2 = str::ToInt32(argv[1]);
 real32 x = str::ToReal32(argv[2]);
 
 real32 res = math::ModBesselFirst(orderX2,x);
 con << res << "\n";
 
 if (orderX2==0)
 {
  real32 ires = math::InverseModBesselFirstOrder0(res);
  con << "(Input by inverse " << ires << ")\n";
 }


 return 0;
}

//------------------------------------------------------------------------------

