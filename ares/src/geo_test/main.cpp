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
 /* sfs::FisherAngProb fap;
  fap.Make(0.05,1.0,4.0,120,1800,&con.BeginProg());
  con.EndProg();
  
  con << "  0 degrees = " << fap.Concentration(0.0 * math::pi/180.0) << "\n";
  con << "  1 degrees = " << fap.Concentration(1.0 * math::pi/180.0) << "\n";
  con << "  2 degrees = " << fap.Concentration(2.0 * math::pi/180.0) << "\n";
  con << "  3 degrees = " << fap.Concentration(3.0 * math::pi/180.0) << "\n";
  con << "  4 degrees = " << fap.Concentration(4.0 * math::pi/180.0) << "\n";
  con << "  5 degrees = " << fap.Concentration(5.0 * math::pi/180.0) << "\n";
  con << "  6 degrees = " << fap.Concentration(6.0 * math::pi/180.0) << "\n";
  con << "  7 degrees = " << fap.Concentration(7.0 * math::pi/180.0) << "\n";
  con << "  8 degrees = " << fap.Concentration(8.0 * math::pi/180.0) << "\n";
  con << "  9 degrees = " << fap.Concentration(9.0 * math::pi/180.0) << "\n";        
  con << " 10 degrees = " << fap.Concentration(10.0 * math::pi/180.0) << "\n";
  con << " 20 degrees = " << fap.Concentration(20.0 * math::pi/180.0) << "\n";
  con << " 30 degrees = " << fap.Concentration(30.0 * math::pi/180.0) << "\n";
  con << " 40 degrees = " << fap.Concentration(40.0 * math::pi/180.0) << "\n";
  con << " 50 degrees = " << fap.Concentration(50.0 * math::pi/180.0) << "\n";
  con << " 60 degrees = " << fap.Concentration(60.0 * math::pi/180.0) << "\n";
  con << " 70 degrees = " << fap.Concentration(70.0 * math::pi/180.0) << "\n";
  con << " 80 degrees = " << fap.Concentration(80.0 * math::pi/180.0) << "\n";
  con << " 90 degrees = " << fap.Concentration(90.0 * math::pi/180.0) << "\n";
  con << "120 degree = " << fap.Concentration(120.0 * math::pi/180.0) << "\n";
  con << "150 degree = " << fap.Concentration(150.0 * math::pi/180.0) << "\n";*/
  
  
 
 // Testing of subdivision sphere object - just hammering it and seeing if it breaks...
  // Hammer it...
   fit::SubDivSphere sds;
   fit::SubDivSphere::Tri targ = sds.GetRoot(0);
   for (nat32 i=0;i<6;i++)
   {
    targ = sds.MakeB(targ);
   }
   targ = sds.MakeAdjA(targ);


  // Create ply file of result...
   file::Ply ply;
   ds::Array<nat32> vertInd(sds.VertCount());
   for (nat32 i=0;i<vertInd.Size();i++)
   {
    vertInd[i] = ply.Add(bs::Vert(sds.Vert(i)));
   }
   for (nat32 i=0;i<sds.TriCount();i++)
   {
    fit::SubDivSphere::Tri tri = i;
    if (!sds.HasChildren(tri))
    {
     ply.Add(vertInd[sds.IndA(tri)]);
     ply.Add(vertInd[sds.IndB(tri)]);
     ply.Add(vertInd[sds.IndC(tri)]);
     ply.Face();
    }
   }


  // Save...
   ply.Save("sphere_subdiv.ply",true);
  
 
 con.WaitSize(1);
 return 0;
}

//------------------------------------------------------------------------------

