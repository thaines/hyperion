//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/cam/resectioning.h"

#include "eos/math/iter_min.h"

namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------
CalculateCamera::CalculateCamera()
:targQ(normal),resQ(failure),residual(-1.0)
{
 radial.aspectRatio = 1.0;
 radial.centre[0] = 0.0;
 radial.centre[1] = 0.0;
 for (nat32 i=0;i<4;i++) radial.k[i] = 0.0;

 math::Identity(camera);
}

CalculateCamera::~CalculateCamera()
{}

void CalculateCamera::AddMatch(const bs::Vert & world,const bs::Pnt & image)
{
 Match m;
  m.world[0] = world[0];
  m.world[1] = world[1];
  m.world[2] = world[2];
  m.image[0] = image[0];
  m.image[1] = image[1];
 data.AddBack(m);
}

void CalculateCamera::AddMatch(const bs::Vertex & world,const bs::Point & image)
{
 Match m;
  m.world[0] = world[0]/world[3];
  m.world[1] = world[1]/world[3];
  m.world[2] = world[2]/world[3];
  m.image[0] = image[0]/image[2];
  m.image[1] = image[1]/image[2];
 data.AddBack(m);
}

void CalculateCamera::Calculate(time::Progress * prog)
{
 LogTime("eos::cam::CalculateCamera::Calculate");
 resQ = failure;
 residual = -1.0;
 if (data.Size()<6) {LogDebug("[cam::CalculateCamera] Insufficient matches."); return;}

 prog->Push();
 nat32 steps;
 switch (targQ)
 {
  case low:    steps = 1; break;
  case normal: steps = 2; break;
  default:     steps = 3; break;
 }



 // Use SVD to get an initial estimate...
 {
  prog->Report(0,steps);
  // Calculate the means of the data...
   math::Vect<3,real64> worldMean(0.0);
   math::Vect<2,real64> imageMean(0.0);
   nat32 matchCount = 0;
   ds::List<Match>::Cursor targ = data.FrontPtr();
   while (!targ.Bad())
   {
    ++matchCount;

    worldMean[0] += (targ->world[0]-worldMean[0])/real64(matchCount);
    worldMean[1] += (targ->world[1]-worldMean[1])/real64(matchCount);
    worldMean[2] += (targ->world[2]-worldMean[2])/real64(matchCount);
    imageMean[0] += (targ->image[0]-imageMean[0])/real64(matchCount);
    imageMean[1] += (targ->image[1]-imageMean[1])/real64(matchCount);

    ++targ;
   }


  // Calculate the standard deviations of the data...
   math::Vect<3,real64> worldSd(0.0);
   math::Vect<2,real64> imageSd(0.0);
   matchCount = 0;
   targ = data.FrontPtr();
   while (!targ.Bad())
   {
    ++matchCount;

    worldSd[0] += (math::Abs(targ->world[0]-worldMean[0])-worldSd[0])/real64(matchCount);
    worldSd[1] += (math::Abs(targ->world[1]-worldMean[1])-worldSd[1])/real64(matchCount);
    worldSd[2] += (math::Abs(targ->world[2]-worldMean[2])-worldSd[2])/real64(matchCount);
    imageSd[0] += (math::Abs(targ->image[0]-imageMean[0])-imageSd[0])/real64(matchCount);
    imageSd[1] += (math::Abs(targ->image[1]-imageMean[1])-imageSd[1])/real64(matchCount);

    ++targ;
   }


  // Create normalisation matrices...
   math::Mat<4,4,real64> normWorld;
   math::Identity(normWorld);
   normWorld[0][0] = 1.0/worldSd[0]; normWorld[0][3] = -worldMean[0]/worldSd[0];
   normWorld[1][1] = 1.0/worldSd[1]; normWorld[1][3] = -worldMean[1]/worldSd[1];
   normWorld[2][2] = 1.0/worldSd[2]; normWorld[2][3] = -worldMean[2]/worldSd[2];

   math::Mat<3,3,real64> normImage;
   math::Identity(normImage);
   normImage[0][0] = 1.0/imageSd[0]; normImage[0][2] = -imageMean[0]/imageSd[0];
   normImage[1][1] = 1.0/imageSd[1]; normImage[1][2] = -imageMean[1]/imageSd[1];


  // Build the matrix to be passed into SVD, normalising as we go...
   math::Matrix<real64> a(2*matchCount,12);
   targ = data.FrontPtr();
   for (nat32 i=0;i<matchCount;i++)
   {
    // Calculate normalised points...
     math::Vect<3,real64> world;
     math::Vect<2,real64> image;
     math::MultVectEH(normWorld,targ->world,world);
     math::MultVectEH(normImage,targ->image,image);


    // Store in matrix...
     a[i*2][0]  = 0.0;
     a[i*2][1]  = 0.0;
     a[i*2][2]  = 0.0;
     a[i*2][3]  = 0.0;
     a[i*2][4]  = world[0];
     a[i*2][5]  = world[1];
     a[i*2][6]  = world[2];
     a[i*2][7]  = 1.0;
     a[i*2][8]  = -image[1]*world[0];
     a[i*2][9]  = -image[1]*world[1];
     a[i*2][10] = -image[1]*world[2];
     a[i*2][11] = -image[1];

     a[i*2+1][0]  = world[0];
     a[i*2+1][1]  = world[1];
     a[i*2+1][2]  = world[2];
     a[i*2+1][3]  = 1.0;
     a[i*2+1][4]  = 0.0;
     a[i*2+1][5]  = 0.0;
     a[i*2+1][6]  = 0.0;
     a[i*2+1][7]  = 0.0;
     a[i*2+1][8]  = -image[0]*world[0];
     a[i*2+1][9]  = -image[0]*world[1];
     a[i*2+1][10] = -image[0]*world[2];
     a[i*2+1][11] = -image[0];

    ++targ;
   }


  // Run SVD and extract the projection matrix, un-normalise...
   // Calculate...
    math::Vect<12,real64> b;
    if (math::RightNullSpace(a,b)==false) return;
    b.Normalise();

   // Extract...
    camera[0][0] = b[0]; camera[0][1] = b[1]; camera[0][2] = b[2];  camera[0][3] = b[3];
    camera[1][0] = b[4]; camera[1][1] = b[5]; camera[1][2] = b[6];  camera[1][3] = b[7];
    camera[2][0] = b[8]; camera[2][1] = b[9]; camera[2][2] = b[10]; camera[2][3] = b[11];

   // Un-normalise...
    math::Mat<3,3,real64> tempImage;
    math::Inverse(normImage,tempImage);

    Camera temp;
    math::Mult(camera,normWorld,temp);
    math::Mult(normImage,temp,camera);

    real64 fn = math::FrobNorm(camera);
    if (!math::IsFinite(fn)) return;
    camera /= fn;


   // Calculate the residual, as it kinda sucks to not know...
   {
    residual = 0.0;
    targ = data.FrontPtr();
    for (nat32 i=0;i<matchCount;i++)
    {
     math::Vect<2,real64> proj;
     math::MultVectEH(camera,targ->world,proj);
     residual += math::Sqr(proj[0]-targ->image[0]) + math::Sqr(proj[1]-targ->image[1]);

     ++targ;
    }
    residual = math::Sqrt(residual);
   }
 }
 resQ = low;
 if (resQ==targQ) return;



 // Use LM to refine without radial estimation...
 {
  prog->Report(1,steps);

  // Setup the parameter vector - just a linearisation of the camera matrix...
   math::Vector<real64> para(12);
   para[0]  = camera[0][0];
   para[1]  = camera[0][1];
   para[2]  = camera[0][2];
   para[3]  = camera[0][3];
   para[4]  = camera[1][0];
   para[5]  = camera[1][1];
   para[6]  = camera[1][2];
   para[7]  = camera[1][3];
   para[8]  = camera[2][0];
   para[9]  = camera[2][1];
   para[10] = camera[2][2];
   para[11] = camera[2][3];


  // Apply the LM...
   residual = math::LM(data.Size(),para,data,&FirstLM,null<void(*)(const math::Vector<real64>&,math::Matrix<real64>&,const ds::List<Match>&)>(),&FirstCon);


  // Decompose the parameter vector back into the camera matrix...
   if (!math::IsFinite(para.Length())) return;
   camera[0][0] = para[0];
   camera[0][1] = para[1];
   camera[0][2] = para[2];
   camera[0][3] = para[3];
   camera[1][0] = para[4];
   camera[1][1] = para[5];
   camera[1][2] = para[6];
   camera[1][3] = para[7];
   camera[2][0] = para[8];
   camera[2][1] = para[9];
   camera[2][2] = para[10];
   camera[2][3] = para[11];
 }
 resQ = normal;
 if (resQ==targQ) return;



 // Use LM to refine with radial estimation...
 {
  prog->Report(2,steps);

  // Setup the parameter vector - a linearisation of the camera matrix followed
  // by the rdaial paramters. To do this we have to extract the starting centre
  // from the camera...
   Intrinsic intr;
   math::Mat<3,3,real64> rot;
   camera.Decompose(intr,rot);

   math::Vector<real64> para(18);
   para[0]  = camera[0][0];
   para[1]  = camera[0][1];
   para[2]  = camera[0][2];
   para[3]  = camera[0][3];
   para[4]  = camera[1][0];
   para[5]  = camera[1][1];
   para[6]  = camera[1][2];
   para[7]  = camera[1][3];
   para[8]  = camera[2][0];
   para[9]  = camera[2][1];
   para[10] = camera[2][2];
   para[11] = camera[2][3];

   para[12] = intr.PrincipalX();
   para[13] = intr.PrincipalY();
   para[14] = 0.0;
   para[15] = 0.0;
   para[16] = 0.0;
   para[17] = 0.0;


  // Apply the LM...
   residual = math::LM(data.Size(),para,data,&SecondLM,null<void(*)(const math::Vector<real64>&,math::Matrix<real64>&,const ds::List<Match>&)>(),&SecondCon);


  // Decompose the parameter vector back into the camera matrix and radial
  // parameters data structure...
   if (!math::IsFinite(para.Length())) return;
   camera[0][0] = para[0];
   camera[0][1] = para[1];
   camera[0][2] = para[2];
   camera[0][3] = para[3];
   camera[1][0] = para[4];
   camera[1][1] = para[5];
   camera[1][2] = para[6];
   camera[1][3] = para[7];
   camera[2][0] = para[8];
   camera[2][1] = para[9];
   camera[2][2] = para[10];
   camera[2][3] = para[11];

   radial.aspectRatio = camera.AspectRatio();
   radial.centre[0] = para[12];
   radial.centre[1] = para[13];
   radial.k[0] = para[14];
   radial.k[1] = para[15];
   radial.k[2] = para[16];
   radial.k[3] = para[17];
 }
 resQ = high;


 prog->Pop();
}

//------------------------------------------------------------------------------
void CalculateCamera::FirstLM(const math::Vector<real64> & pv,math::Vector<real64> & err,const ds::List<Match> & oi)
{
 Camera cam;
 cam[0][0] = pv[0];
 cam[0][1] = pv[1];
 cam[0][2] = pv[2];
 cam[0][3] = pv[3];
 cam[1][0] = pv[4];
 cam[1][1] = pv[5];
 cam[1][2] = pv[6];
 cam[1][3] = pv[7];
 cam[2][0] = pv[8];
 cam[2][1] = pv[9];
 cam[2][2] = pv[10];
 cam[2][3] = pv[11];

 ds::List<Match>::Cursor targ = oi.FrontPtr();
 for (nat32 i=0;i<err.Size();i++)
 {
  math::Vect<2,real64> proj;
  math::MultVectEH(cam,targ->world,proj);

  err[i] = math::Sqrt(math::Sqr(proj[0]-targ->image[0]) + math::Sqr(proj[1]-targ->image[1]));

  ++targ;
 }
}

void CalculateCamera::FirstCon(math::Vector<real64> & pv,const ds::List<Match> & oi)
{
 pv.Normalise();
}

void CalculateCamera::SecondLM(const math::Vector<real64> & pv,math::Vector<real64> & err,const ds::List<Match> & oi)
{
 Camera cam;
 cam[0][0] = pv[0];
 cam[0][1] = pv[1];
 cam[0][2] = pv[2];
 cam[0][3] = pv[3];
 cam[1][0] = pv[4];
 cam[1][1] = pv[5];
 cam[1][2] = pv[6];
 cam[1][3] = pv[7];
 cam[2][0] = pv[8];
 cam[2][1] = pv[9];
 cam[2][2] = pv[10];
 cam[2][3] = pv[11];

 Radial rad;
 rad.aspectRatio = cam.AspectRatio();
 rad.centre[0] = pv[12];
 rad.centre[1] = pv[13];
 rad.k[0] = pv[14];
 rad.k[1] = pv[15];
 rad.k[2] = pv[16];
 rad.k[3] = pv[17];

 ds::List<Match>::Cursor targ = oi.FrontPtr();
 for (nat32 i=0;i<err.Size();i++)
 {
  math::Vect<2,real64> proj;
  math::MultVectEH(cam,targ->world,proj);

  rad.Dis(proj,proj);

  err[i] = math::Sqrt(math::Sqr(proj[0]-targ->image[0]) + math::Sqr(proj[1]-targ->image[1]));

  ++targ;
 }
}

void CalculateCamera::SecondCon(math::Vector<real64> & pv,const ds::List<Match> & oi)
{
 real64 length = 0.0;
 for (nat32 i=0;i<12;i++) length += math::Sqr(pv[i]);
 length = math::InvSqrt(length);
 for (nat32 i=0;i<12;i++) pv[i] *= length;
}

//------------------------------------------------------------------------------
 };
};
