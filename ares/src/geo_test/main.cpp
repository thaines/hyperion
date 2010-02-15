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
  math::Vect<3> ell;
  ell[0] = rand.Real(0.1,3.0);
  ell[1] = rand.Real(0.1,3.0);
  ell[2] = rand.Real(0.1,3.0);

 // Define a point...
  math::Vect<3> point;
  point[0] = rand.Real(-4.0,4.0);
  point[1] = rand.Real(-4.0,4.0);
  point[2] = rand.Real(-4.0,4.0);
  
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
  con << "\n";


 con.WaitSize(1);
 return 0;
}

//------------------------------------------------------------------------------

