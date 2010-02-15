//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "ba_test/main.h"

#include <stdio.h>

using namespace eos;

//------------------------------------------------------------------------------
class MyProg : public eos::time::Progress
{
 public:
  MyProg() {}

  void OnChange()
  {
   nat32 x,y;
   Part(Depth()-1,x,y);
   if (x%((y/100)+1)!=0) return;

   for (nat32 i=0;i<Depth();i++)
   {
    nat32 x,y;
    Part(i,x,y);
    printf("%i: [%i of %i]\n",(int)i,(int)x,(int)y);
   }
   printf("\n");
  }
};

//------------------------------------------------------------------------------
int main()
{
 MyProg prog;
 data::Random rand; 
 
 // Simulate bundle adjustment with a couple cameras and a lot of points, generate
 // point and camera positions randomly...
  static const nat32 camCount = 2;
  static const nat32 pointCount = 40; 
 
  // Create a set of 3D points in a cloud...
   math::Vect<4> pnt3[pointCount];
   for (nat32 i=0;i<pointCount;i++)
   {
    pnt3[i][0] = rand.Real(-10.0,10.0);
    pnt3[i][1] = rand.Real(-10.0,10.0);
    pnt3[i][2] = rand.Real(-10.0,10.0);
    pnt3[i][3] = 1.0;
   }


  // Create a set of cameras looking at the cloud...
  // We represent a camera by a 3 vector for its position followed by a 3 vector
  // for its rotation as a direction looking. The camera is allways oriented to be 
  // the right way up. We select from a cloud of positions seperate from the point
  // cloud and then point it directly at a random point in the set...
  // (We are also assuming the intrinsic matrix to be the identity.)
   math::Vect<6> cam[camCount];
   for (nat32 i=0;i<camCount;i++)
   {
    cam[i][0] = rand.Real(-25.0,-15.0);
    cam[i][1] = rand.Real(-20.0,20.0);
    cam[i][2] = rand.Real(-20.0,20.0);
    nat32 pn = rand.Int(0,pointCount-1);
    cam[i][3] = pnt3[pn][0] - cam[i][0];
    cam[i][4] = pnt3[pn][1] - cam[i][1];
    cam[i][5] = pnt3[pn][2] - cam[i][2];
    real32 mult = 1.0/math::Sqrt(math::Sqr(cam[i][3]) + math::Sqr(cam[i][4]) + math::Sqr(cam[i][5]));
    cam[i][3] *= mult; cam[i][4] *= mult; cam[i][5] *= mult;
   }


  // Calculate from the cameras the projection points of the points...
   math::Vect<2> pnt2[camCount][pointCount];
   for (nat32 i=0;i<camCount;i++)
   {
    math::Mat<3,4> cm;

    cm[0][0] = -cam[i][5]; cm[0][1] = 0.0; cm[0][2] = cam[i][3];
    cm[1][0] = -cam[i][3]*cam[i][4]; cm[1][1] = cam[i][3]*cam[i][3] + cam[i][5]*cam[i][5]; cm[1][2] = -cam[i][5]*cam[i][4];
    cm[2][0] = cam[i][3]; cm[2][1] = cam[i][4]; cm[2][2] = cam[i][5];
    cm[0][3] = cam[i][0]; cm[1][3] = cam[i][1]; cm[2][3] = cam[i][2];
    
    for (nat32 j=0;j<pointCount;j++)
    {
     math::Vect<3> res;
     MultVect(cm,pnt3[j],res);
     pnt2[i][j][0] = res[0]/res[2];
     pnt2[i][j][1] = res[1]/res[2];     
    }
   }
  
  // Move it all into 64 bit Vector objects and add gaussian noise to it all.
  // Especially to the cameras, so they have a long way to travel to
  // get to the correct answer...
   math::Vector<real64> vCam[camCount];
   math::Vector<real64> vPnt3[pointCount];
   math::Vector<real64> vPnt2[camCount][pointCount];
   
   for (nat32 i=0;i<camCount;i++)
   {
    vCam[i] = cam[i];
    for (nat32 j=0;j<3;j++) vCam[i][j] += rand.Gaussian(5.0);
    for (nat32 j=3;j<6;j++) vCam[i][j] += rand.Gaussian(1.5);
   }
   
   for (nat32 i=0;i<pointCount;i++)
   {
    vPnt3[i] = pnt3[i];
    for (nat32 j=0;j<3;j++) vPnt3[i][j] += rand.Gaussian(5.0);
   }
   
   for (nat32 i=0;i<camCount;i++)
   {
    for (nat32 j=0;j<pointCount;j++)
    {
     vPnt2[i][j] = pnt2[i][j];
     //for (nat32 k=0;k<2;k++) vPnt2[i][j][k] += rand.Gaussian(0.2);
    }
   }
      
  // Solve the system with bundle adjustment...
   struct FC
   {
    static void F(const math::Vector<real64> & a, const math::Vector<real64> & b, const math::Vector<real64> & m, math::Vector<real64> & err)
    {
     math::Mat<3,4> cm;
      cm[0][0] = -a[5]; cm[0][1] = 0.0; cm[0][2] = a[3];
      cm[1][0] = -a[3]*a[4]; cm[1][1] = a[3]*a[3] + a[5]*a[5]; cm[1][2] = -a[5]*a[4];
      cm[2][0] = a[3]; cm[2][1] = a[4]; cm[2][2] = a[5];
      cm[0][3] = a[0]; cm[1][3] = a[1]; cm[2][3] = a[2];
      
     math::Vect<3> res;
      MultVect(cm,b,res);
      err[0] = res[0]/res[2] - m[0];
      err[1] = res[1]/res[2] - m[1]; 
    }
    
    static void CC(math::Vector<real64> & a)
    {
     real64 mult = math::Sqrt(math::Sqr(a[3])+math::Sqr(a[4])+math::Sqr(a[5]));
     if (math::Equal(mult,0.0))
     {
      a[3] = 1.0; a[4] = 0.0; a[5] = 0.0;      
     }
     else
     {
      mult = 1.0/mult;
      a[3] *= mult; a[4] *= mult; a[5] *= mult;
     }
    }
    
    static void PC(math::Vector<real64> & a)
    {
     real64 mult = 1.0/a[3];
     a[0] *= mult; a[1] *= mult; a[2] *= mult; a[3] = 1.0;
    }    
   };
   
   math::SparseLM solver;
    solver.SetSizes(6,4,2);
    for (nat32 i=0;i<camCount;i++) solver.AddParaA(vCam[i]);
    for (nat32 i=0;i<pointCount;i++) solver.AddParaB(vPnt3[i]);
    for (nat32 i=0;i<camCount;i++)
    {
     for (nat32 j=0;j<pointCount;j++)
     {
      solver.AddError(i,j,vPnt2[i][j],&FC::F);
     }
    }
    
    for (nat32 i=0;i<camCount;i++) solver.AddConsA(i,&FC::CC);
    for (nat32 i=0;i<pointCount;i++) solver.AddConsB(i,&FC::PC);
    
    
   real64 residual = solver.Run(&prog);


  // Print out the actual and calculated camera positions, plus the residual...
   for (nat32 i=0;i<camCount;i++)
   {
    solver.GetParaA(i,vCam[i]);
    printf("Actual = (%f,%f,%f):{%f,%f,%f}\n",cam[i][0],cam[i][1],cam[i][2],cam[i][3],cam[i][4],cam[i][5]);
    printf("Found  = (%f,%f,%f):{%f,%f,%f}\n",vCam[i][0],vCam[i][1],vCam[i][2],vCam[i][3],vCam[i][4],vCam[i][5]);
   }
   printf("Residual = %f\n",residual);
  
 return 0;
}

//------------------------------------------------------------------------------
