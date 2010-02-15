//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "fitter/main.h"

#include <stdio.h>

//------------------------------------------------------------------------------
class MyProg : public time::Progress
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
   printf("\n"); fflush(0);
  }
};

//------------------------------------------------------------------------------
int main(int argc,char ** argv)
{
 // A set of parameters to determine how the test works...
  if (argc!=12)
  {
   printf("Usage:\n");
   printf("fitter <n> <ps> <ds> <io> <ao> <ia> <aa> <pn> <dn> <nn> <ini>\n");
   printf("<n>   = Number of tests to do.\n");
   printf("<ps>  = Number of position samples per test.\n");
   printf("<ds>  = Number of orientation samples per test.\n");
   printf("<io>  = Minimum longitude of sampling divided by pi\n");
   printf("<ao>  = Maximum longitude of sampling divided by pi\n");
   printf("<ia>  = Minimum latitude of sampling divided by pi\n");
   printf("<aa>  = Maximum latitude of sampling divided by pi\n");   
   printf("<pn>  = Gaussian noise in x-y position of samples taken.\n");
   printf("<dn>  = Gaussian noise in depth of position samples.\n");
   printf("<nn>  = Gaussian noise in normal of orientation samples.\n");
   printf("<ini> = Initialisation condition:\n");
   printf("          0 = Radius 1 sphere at origin.\n");
   printf("          1 = Radius to contain all with centre as mean.\n");
   printf("          2 = The answer.\n");      
   return 1;
  }
 
  nat32 testIters = str::ToInt32(argv[1]);
 
  real32 minSphereSize = 8.0;
  real32 maxSphereSize = 16.0;
  real32 spherePosRange = 50.0;

  nat32 posSamples = str::ToInt32(argv[2]);
  nat32 dirSamples = str::ToInt32(argv[3]);

  real32 minLon = str::ToReal32(argv[4])*math::pi;
  real32 maxLon = str::ToReal32(argv[5])*math::pi;
  real32 minLat = str::ToReal32(argv[6])*math::pi;
  real32 maxLat = str::ToReal32(argv[7])*math::pi;

  real32 posNoise = str::ToReal32(argv[8]);
  real32 depthNoise = str::ToReal32(argv[9]);
  real32 normNoise = str::ToReal32(argv[10]);

  nat32 iniMode = str::ToInt32(argv[11]);


 // Iterate for each of the tests, for each one generate a test sphere, a
 // sampling and then run each algorithm in turn...
  AllAlgs allAlgs;
  Sphere sphere;
  DataSet ds; 
   ds.dw = 1.0;
   ds.ew = 1.0;
  
  data::Random rand;
  MyProg prog;
  
  for (nat32 i=0;i<testIters;i++)
  {
   prog.Report(i,testIters);

   // Create a random sphere...    
    sphere.MakeRandom(rand,math::Range(-spherePosRange,spherePosRange),math::Range(minSphereSize,maxSphereSize));
    
   // Create a random sampling...
    ds.Build(rand,sphere,posSamples,dirSamples,math::Range(minLat,maxLat),math::Range(minLon,maxLon));
    ds.AddNoise(rand,posNoise,depthNoise,normNoise);
       
   // Fit it all...    
    allAlgs.Add(ds,iniMode);
  }


 // Output statistics for each algorithm...
  io::StreamFile so(stdout);
  io::VirtOut<io::StreamFile> sov(so);
  allAlgs.Print(sov);
  printf("\n\n");
  
 // Output the best algorithms, as measured by mean in each
 // category & mean + sd in each category...
  AlgRep temp;
  BestAlg bestAlg;
  allAlgs.GetBest(temp,bestAlg);
  bestAlg.Print(sov);

 return 0;
}

//------------------------------------------------------------------------------
