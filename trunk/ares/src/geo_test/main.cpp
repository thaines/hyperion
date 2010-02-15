//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "geo_test/main.h"

//------------------------------------------------------------------------------
int main()
{
 using namespace eos;

 data::Random rand;

 os::Con conObj;
 os::Conversation & con = *conObj.StartConversation();


 // Define an ellipsoid by its axis lengths...
 /* math::Vect<3> ell;
  ell[0] = rand.Real(0.1,3.0);
  ell[1] = rand.Real(0.1,3.0);
  ell[2] = rand.Real(0.1,3.0);

 // Define a point...
  math::Vect<3> point;
  point[0] = rand.Real(-1.0,1.0);
  point[1] = rand.Real(-1.0,1.0);
  point[2] = rand.Real(-1.0,1.0);
  
 // Find the points on the ellipsoid that have normals pointing at the point...
  math::Vect<3> out[6];
  nat32 num = alg::PointEllipsoidDir(ell,point,out,1e-6,1000);
  
 // Print out the results...
  con << "Ellipsoid is " << ell << ".\n";
  con << "Point is " << point << ".\n";
  con << num << " points of perpendicularlity found.\n";
  for (nat32 i=0;i<num;i++)
  {
   con << i << ": " << out[i] << "\n";
  }
  con << "\n";*/
  
  
  
 // Test NaN handling, nope, this isn't geometrical at all, but it sure as 
 // hell affects my geometrical algorithms...
  /*real32 nan = math::Sqrt(-1.0);
  con << "nan = " << nan << "\n";

  con << " isNan = " << math::IsNan(nan) << "\n";
  con << " isInf = " << math::IsInf(nan) << "\n";
  con << " isFinite = " << math::IsFinite(nan) << "\n";
  
  real32 inf = 1.0/0.0;
  con << "inf = " << inf << "\n";

  con << " isNan = " << math::IsNan(inf) << "\n";
  con << " isInf = " << math::IsInf(inf) << "\n";
  con << " isFinite = " << math::IsFinite(inf) << "\n";

  real32 normal = 0.0;
  con << "num = " << normal << "\n";

  con << " isNan = " << math::IsNan(normal) << "\n";
  con << " isInf = " << math::IsInf(normal) << "\n";
  con << " isFinite = " << math::IsFinite(normal) << "\n";
  
  real64 nan64 = math::Sqrt(-1.0);
  con << "nan64 = " << nan64 << "\n";

  con << " isNan = " << math::IsNan(nan64) << "\n";
  con << " isInf = " << math::IsInf(nan64) << "\n";
  con << " isFinite = " << math::IsFinite(nan64) << "\n";
  
  real64 inf64 = 1.0/0.0;
  con << "inf64 = " << inf64 << "\n";

  con << " isNan = " << math::IsNan(inf64) << "\n";
  con << " isInf = " << math::IsInf(inf64) << "\n";
  con << " isFinite = " << math::IsFinite(inf64) << "\n";

  real64 normal64 = 0.0;
  con << "num64 = " << normal64 << "\n";

  con << " isNan = " << math::IsNan(normal64) << "\n";
  con << " isInf = " << math::IsInf(normal64) << "\n";
  con << " isFinite = " << math::IsFinite(normal64) << "\n"; */ 

 // Lets get brute force...
 /*{
  nat32 val = 0;
  do
  {
   if ((val%0xFFFFFF)==0) con << (val/0xFFFFFF) << "\n";
   real32 rv = *(real32*)(void*)&val;
   if (math::IsNan(rv))
   {
    con << "\nFound a nan! - " << rv << " - " << val << "\n";
   }
   if ((rv==rv)==false)
   {
    con << "\nFound a nan!(2) - " << rv << " - " << val << "\n";
   }
   
   val++;
  } while (val!=0);
 }*/



 // Testing of concentration from probability within an angle class...
 // (The geometrical link is really getting stretched here.)
  sfs::FisherAngProb fap;
 
 con.WaitSize(1);
 return 0;
}

//------------------------------------------------------------------------------

