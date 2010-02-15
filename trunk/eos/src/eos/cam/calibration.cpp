//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "eos/cam/calibration.h"

#include "eos/ds/arrays.h"
#include "eos/cam/homography.h"
#include "eos/math/iter_min.h"

namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------
Zhang98::Zhang98()
:targQ(high)
{}

Zhang98::~Zhang98()
{}

void Zhang98::Reset()
{
 data.Reset();	
}

void Zhang98::Add(nat32 shot,const math::Vect<2,real32> & f,const math::Vect<2,real32> & s)
{
 Node nn;
  nn.shot = shot;
  nn.first[0] = f[0];
  nn.first[1] = f[1];
  nn.second[0] = s[0];
  nn.second[1] = s[1];
 data.AddBack(nn);	
}

void Zhang98::Add(nat32 shot,const math::Vect<2,real64> & f,const math::Vect<2,real64> & s)
{
 Node nn;
  nn.shot = shot;
  nn.first[0] = f[0];
  nn.first[1] = f[1];
  nn.second[0] = s[0];
  nn.second[1] = s[1];
 data.AddBack(nn);	
}

void Zhang98::Add(nat32 shot,const math::Vect<3,real32> & f,const math::Vect<3,real32> & s)
{
 Node nn;
  nn.shot = shot;
  nn.first[0] = f[0]/f[2];
  nn.first[1] = f[1]/f[2];
  nn.second[0] = s[0]/s[2];
  nn.second[1] = s[1]/s[2];
 data.AddBack(nn);	
}

void Zhang98::Add(nat32 shot,const math::Vect<3,real64> & f,const math::Vect<3,real64> & s)
{
 Node nn;
  nn.shot = shot;
  nn.first[0] = f[0]/f[2];
  nn.first[1] = f[1]/f[2];
  nn.second[0] = s[0]/s[2];
  nn.second[1] = s[1]/s[2];
 data.AddBack(nn);	
}

void Zhang98::SetQuality(ResQuality rq)
{
 targQ = rq;	
}

void Zhang98::Calculate(time::Progress * prog)
{
 LogBlock("void Zhang98::Calculate()","-");
 prog->Push();
 nat32 prog_steps = 5;
 switch (targQ)
 {
  case low: prog_steps = 2; break;
  case normal: prog_steps = 4; break;
  case high: case failure: prog_steps = 5; break;	 
 }
 prog->Report(0,prog_steps);
 
 
 
 //
 // Administration pass... (Rather inefficient, but who cares? This is going to be dwarfed by what comes next.)
 //
  // Count the shots...
   nat32 shots = 0;
   ds::List<Node>::Cursor targ = data.FrontPtr();
   while (!targ.Bad())
   {
    shots = math::Max(targ->shot+1,shots);
    ++targ;
   }

  // Construct lists for each shot...
   Shot * sd = new Shot[shots];
   LogDebug("[cam.calibration] There are " << shots << " images");
 
  // Fill the lists with the data for each shot, also track the minimum and maximum
  // of both axises combined for both the first and second coordinates...
   targ = data.FrontPtr();      
   while (!targ.Bad())
   {
    sd[targ->shot].data.AddBack(*targ);
    ++targ;
   }


  // Find the mean of all the points in all the shots simultaneously, so we
  // can zero mean the data, for both model and image coordinates...
   math::Vect<2,real64> firstMean(0.0);
   math::Vect<2,real64> secondMean(0.0);
   nat32 pointCount = 0;
   targ = data.FrontPtr();
   while (!targ.Bad())
   {
    ++pointCount;
    firstMean[0] += (targ->first[0]-firstMean[0])/real32(pointCount);
    firstMean[1] += (targ->first[1]-firstMean[1])/real32(pointCount);
    secondMean[0] += (targ->second[0]-secondMean[0])/real32(pointCount);
    secondMean[1] += (targ->second[1]-secondMean[1])/real32(pointCount);
    ++targ;
   }

  
  // Now find the geometric standard deviation from the mean points of all points...
   math::Vect<2,real64> firstSd(0.0);
   math::Vect<2,real64> secondSd(0.0);
   targ = data.FrontPtr();
   pointCount = 0;
   while (!targ.Bad())
   {
    ++pointCount;
    firstSd[0] += (math::Abs(targ->first[0]-firstMean[0])-firstSd[0])/real32(pointCount);
    firstSd[1] += (math::Abs(targ->first[1]-firstMean[1])-firstSd[1])/real32(pointCount);
    secondSd[0] += (math::Abs(targ->second[0]-secondMean[0])-secondSd[0])/real32(pointCount);
    secondSd[1] += (math::Abs(targ->second[1]-secondMean[1])-secondSd[1])/real32(pointCount);    
    ++targ;
   }


  // Use the just collected data to make two matrices, one to normalise the
  // model data and one to normalise all the image data.
   math::Mat<3,3,real64> firstNorm; Identity(firstNorm);
    firstNorm[0][0] = 1.0/firstSd[0];
    firstNorm[1][1] = 1.0/firstSd[1];
    firstNorm[0][2] = -firstMean[0]/firstSd[0];
    firstNorm[1][2] = -firstMean[1]/firstSd[1];

   math::Mat<3,3,real64> secondNorm; Identity(secondNorm);
    secondNorm[0][0] = 1.0/secondSd[0];
    secondNorm[1][1] = 1.0/secondSd[1];
    secondNorm[0][2] = -secondMean[0]/secondSd[0];
    secondNorm[1][2] = -secondMean[1]/secondSd[1];
   
   LogDebug("[cam.calibration] Normalisation matrices {model,visualisation}" << LogDiv() << firstNorm << LogDiv() << secondNorm);


    
 //
 // First estimate, linear SVD method...
 //
 prog->Report(1,prog_steps);
 prog->Push();
 {
  // Calculate a homography for each shot, we make an un-adjusted one and a normalised one in each case...
   for (nat32 i=0;i<shots;i++)
   {
    prog->Report(i,shots+2);
    // Calculate the normalised version, as this is robust...
    {
     Homography2D h2d;
      targ = sd[i].data.FrontPtr();
      while (!targ.Bad())
      {
       math::Vect<2,real64> f;
       math::Vect<2,real64> s;
        MultVectEH(firstNorm,targ->first,f);
        MultVectEH(secondNorm,targ->second,s);
       h2d.Add(f,s);
       ++targ;	     
      }
     sd[i].residual = h2d.Result(sd[i].hgNorm);          
    }

    // Calculate the un-normalised from the normalised...
    {
     math::Mat<3,3,real64> temp[2];
     
     temp[0] = secondNorm;
     math::Inverse(temp[0],temp[1]);
     math::Mult(temp[0],sd[i].hgNorm,temp[1]);
     math::Mult(temp[1],firstNorm,sd[i].hg);
    }
        
    LogDebug("[cam.calibration] Initial homography {i,residual,h,h-norm}" << LogDiv()
             << i << LogDiv() << sd[i].residual << LogDiv() << sd[i].hg << LogDiv() << sd[i].hgNorm);
   }


  // Build a big fat matrix from all the homographys...
   prog->Report(shots,shots+2);
   math::Matrix<real64> mat(shots*2,6);
   for (nat32 i=0;i<shots;i++)
   {
    // First create the two vectors from the homography...
     math::Mat<3,3,real64> & h = sd[i].hgNorm;
     math::Vect<3,real64> h1;
     h1[0] = h[0][0]; h1[1] = h[1][0]; h1[2] = h[2][0];
     math::Vect<3,real64> h2;
     h2[0] = h[0][1]; h2[1] = h[1][1]; h2[2] = h[2][1];
     
    // Create the 3 matrices of multipliers...
     math::Mat<3,3,real64> v12;
     math::Mat<3,3,real64> v11;
     math::Mat<3,3,real64> v22;
     math::VectVectTrans(h1,h2,v12);
     math::VectVectTrans(h1,h1,v11);
     math::VectVectTrans(h2,h2,v22);
     
    // Fill in the relevant matrix entrys, linearising the absolute conic...
     mat[i*2][0] = v12[0][0];
     mat[i*2][1] = v12[0][1] + v12[1][0];
     mat[i*2][2] = v12[1][1];
     mat[i*2][3] = v12[0][2] + v12[2][0];
     mat[i*2][4] = v12[1][2] + v12[2][1];
     mat[i*2][5] = v12[2][2];
    
     mat[i*2+1][0] = v11[0][0] - v22[0][0];
     mat[i*2+1][1] = v11[0][1] + v11[1][0] - (v22[0][1] + v22[1][0]);
     mat[i*2+1][2] = v11[1][1] - v22[1][1];
     mat[i*2+1][3] = v11[0][2] + v11[2][0] - (v22[0][2] + v22[2][0]);
     mat[i*2+1][4] = v11[1][2] + v11[2][1] - (v22[1][2] + v22[2][1]);
     mat[i*2+1][5] = v11[2][2] - v22[2][2];
     
       
    /*mat[i*2][0] = h[0][0]*h[1][0];
    mat[i*2][1] = h[0][0]*h[1][1] + h[0][1]*h[1][0];
    mat[i*2][2] = h[0][1]*h[1][1];
    mat[i*2][3] = h[0][2]*h[1][0] + h[0][0]*h[1][2];
    mat[i*2][4] = h[0][2]*h[1][1] + h[0][1]*h[1][2];
    mat[i*2][5] = h[0][2]*h[1][2];
    
    mat[i*2+1][0] = h[0][0]*h[0][0] - h[1][0]*h[1][0];
    mat[i*2+1][1] = h[0][0]*h[0][1] + h[0][1]*h[0][0] - h[1][0]*h[1][1] - h[1][1]*h[1][0];
    mat[i*2+1][2] = h[0][1]*h[0][1] - h[1][1]*h[1][1];
    mat[i*2+1][3] = h[0][2]*h[0][0] + h[0][0]*h[0][2] - h[1][2]*h[1][0] - h[1][0]*h[1][2];
    mat[i*2+1][4] = h[0][2]*h[0][1] + h[0][1]*h[0][2] - h[1][2]*h[1][1] - h[1][1]*h[1][2];
    mat[i*2+1][5] = h[0][2]*h[0][2] - h[1][2]*h[1][2];*/
   }

   
  // Extract its right null vector...
   math::Vect<6,real64> b;
   RightNullSpace(mat,b);
   b.Normalise(); // Just for stability.
   prog->Report(shots+1,shots+2);


  // Build our first estimate of the solution...
   /*Identity(intrinsic);
   intrinsic.PrincipalY() = (b[1]*b[3] - b[0]*b[4])/(b[0]*b[2] - math::Sqr(b[1]));
   real64 lambda = b[5] - (math::Sqr(b[3]) + intrinsic.PrincipalY()*(b[1]*b[3] - b[0]*b[4]))/b[0];
   intrinsic.FocalX() = math::Sqrt(lambda/b[0]);
   intrinsic.FocalY() = math::Sqrt((lambda*b[0])/(b[0]*b[2] - math::Sqr(b[1])));
   intrinsic.Skew() = (-b[1]*math::Sqr(intrinsic.FocalX())*intrinsic.FocalY())/lambda;
   intrinsic.PrincipalX() = (intrinsic.Skew()*intrinsic.PrincipalY())/intrinsic.FocalY() - (b[3]*math::Sqr(intrinsic.FocalX()))/lambda;*/
  {
   b.Normalise();
   intrinsic[0][0] = b[0]; intrinsic[0][1] = b[1]; intrinsic[0][2] = b[3];
   intrinsic[1][0] = b[1]; intrinsic[1][1] = b[2]; intrinsic[1][2] = b[4];
   intrinsic[2][0] = b[3]; intrinsic[2][1] = b[4]; intrinsic[2][2] = b[5];
   if (intrinsic[0][0]<0.0) intrinsic *= -1.0;
   intrinsic /= math::FrobNorm(intrinsic);
   
   Intrinsic temp;
   math::Inverse(intrinsic,temp);

   if (math::Cholesky(intrinsic)==false)
   {
    residual = -1.0;
    resQ = failure;
    LogDebug("Cholesky failed when calcualting intrinsic matrix from image of absolute conic");
    delete[] sd;
    return;
   }
   
   math::Transpose(intrinsic);
   
   intrinsic[1][0] = 0.0;
   intrinsic[2][0] = 0.0; intrinsic[2][1] = 0.0;
   intrinsic /= intrinsic[2][2];
   if (intrinsic[0][0]<0.0)
   {
    for (nat32 j=0;j<3;j++) intrinsic[j][0] *= -1.0;	   
   }
   if (intrinsic[1][1]<0.0)
   {
    for (nat32 j=0;j<3;j++) intrinsic[j][1] *= -1.0;	   
   }
  }
   


  // Remove the affect of the earlier normalisation of the data...
   Inverse(secondNorm,firstNorm); // Recyling the firstNorm as a tempory, and yes, this can be done better, but I don't care.
   Intrinsic temInt = intrinsic;
   Mult(secondNorm,temInt,intrinsic);
   

  // Construct a radial distortion model to use as our starting condition, assume there is none...
   radial.aspectRatio = intrinsic.AspectRatio();
   radial.centre[0] = intrinsic.PrincipalX();
   radial.centre[1] = intrinsic.PrincipalY();
   radial.k[0] = 0.0;
   radial.k[1] = 0.0;
   radial.k[2] = 0.0;
   radial.k[3] = 0.0;

   LogDebug("[cam.calibration] First estimate {intrinsic,radial}" <<
            LogDiv() << intrinsic << LogDiv() << radial);

   
  // Check the solution isn't nuts - i.e. that there are no nan's or inf's,
  // if there are we have to report total failure...
   bit bad = false;
   for (nat32 r=0;r<3;r++)
   {
    for (nat32 c=0;c<3;c++) bad |= !math::IsFinite(intrinsic[r][c]);
   }
   if (bad)
   {
    residual = -1.0;
    resQ = failure;
    delete[] sd;
    return;
   }
 }
 prog->Pop();
 if (targQ==low) {residual = -1.0; resQ = low; delete[] sd; return;}

 
 
 //
 // We have a result, at this point reconstruct the extrinsic matrices of the shots...
 //
 prog->Report(2,prog_steps);
 {
  // Inverse intrinsic matrix, to remove its effect from the homographies...
   math::Mat<3,3,real64> invInt = intrinsic;
   math::Mat<3,3,real64> temp;
   Inverse(invInt,temp);


  extrinsic.Size(shots);
  for (nat32 i=0;i<shots;i++)
  {
   // Remove the intrinsic matrix and multiplier effects...
    Mult(invInt,sd[i].hg,temp);
    
    real64 lambda = 1.0/math::Sqrt(math::Sqr(temp[0][0]) + math::Sqr(temp[1][0]) + math::Sqr(temp[2][0]));
    temp *= lambda;


    // Extract the translation component...
     extrinsic[i][0][3] = temp[0][2];
     extrinsic[i][1][3] = temp[1][2];
     extrinsic[i][2][3] = temp[2][2];
    
    // Complete, refine and extract the rotation component...
     temp[0][2] = temp[1][0]*temp[2][1] - temp[2][0]*temp[1][1];
     temp[1][2] = temp[2][0]*temp[0][1] - temp[0][0]*temp[2][1];
     temp[2][2] = temp[0][0]*temp[1][1] - temp[1][0]*temp[0][1];
      
     BestRot(temp);
     
     extrinsic[i][0][0] = temp[0][0]; extrinsic[i][0][1] = temp[0][1]; extrinsic[i][0][2] = temp[0][2];
     extrinsic[i][1][0] = temp[1][0]; extrinsic[i][1][1] = temp[1][1]; extrinsic[i][1][2] = temp[1][2];
     extrinsic[i][2][0] = temp[2][0]; extrinsic[i][2][1] = temp[2][1]; extrinsic[i][2][2] = temp[2][2];
     
     LogDebug("[cam.calibration] First estimate extrinsic matrix for shot " << i << ". {extrinsic}" << LogDiv() << extrinsic[i]);
  }
 }



 //
 // Second estimate, LM on limited parameters...
 //
 prog->Report(3,prog_steps);
 {
  // We use the following vector to describe the problem:
  // [Intrinsic Matrix,2 * radial parameters,# shots of sv]^T (7)
  // where sv is a vector for each shot:
  // [rotation matrix as 3 vector, translation as 3 vector]^T (6 * # of shots)
  // We then minimise this with a x and y difference as the residual.
 
  // Create the initial vector...
   math::Vector<real64> vec(7+shots*6);
   // Global data...
    vec[0] = intrinsic.PrincipalX();
    vec[1] = intrinsic.PrincipalY();
    vec[2] = intrinsic.FocalX();
    vec[3] = intrinsic.FocalY();
    vec[4] = intrinsic.Skew();
    vec[5] = 0.0;
    vec[6] = 0.0;


   // Data for each shot...
    for (nat32 i=0;i<shots;i++)
    {
     // Rotation...
      math::Mat<3,3,real64> temp;
       temp[0][0] = extrinsic[i][0][0]; temp[0][1] = extrinsic[i][0][1]; temp[0][2] = extrinsic[i][0][2];
       temp[1][0] = extrinsic[i][1][0]; temp[1][1] = extrinsic[i][1][1]; temp[1][2] = extrinsic[i][1][2];
       temp[2][0] = extrinsic[i][2][0]; temp[2][1] = extrinsic[i][2][1]; temp[2][2] = extrinsic[i][2][2];
      
      math::Vect<3,real64> aa;
       RotMatToAngAxis(temp,aa);
     
      vec[7 + i*6 + 0] = aa[0];
      vec[7 + i*6 + 1] = aa[1];
      vec[7 + i*6 + 2] = aa[2];
      
     // Translation...
      vec[7 + i*6 + 3] = extrinsic[i][0][3];
      vec[7 + i*6 + 4] = extrinsic[i][1][3];
      vec[7 + i*6 + 5] = extrinsic[i][2][3];      
    }


  // Run LM...
   LMdata lmd;
    lmd.shots = shots;
    lmd.sd = sd;
    
   nat32 evs = 0;
   for (nat32 i=0;i<shots;i++) evs += sd[i].data.Size();
   evs *= 2;
 
   residual = LM(evs,vec,lmd,&FirstLM);
   LogDebug("[cam.calibration] Output of second steps LM. {vec}" << LogDiv() << vec);


  // If we have a bad residual then we have failed with a normal result and must
  // make do with the low quality SVD calculated one...
   if (!math::IsFinite(residual))
   {
    residual = -1.0;
    resQ = low;
    delete[] sd;
    return;  
   }


  // Extract the results...
   intrinsic.PrincipalX() = vec[0];
   intrinsic.PrincipalY() = vec[1];
   intrinsic.FocalX() = vec[2];
   intrinsic.FocalY() = vec[3];
   intrinsic.Skew() = vec[4];

   radial.aspectRatio = intrinsic.AspectRatio();
   radial.centre[0] = intrinsic.PrincipalX();
   radial.centre[1] = intrinsic.PrincipalY();
   radial.k[0] = vec[5];
   radial.k[1] = vec[6];
   
   for (nat32 i=0;i<shots;i++)
   {
    // Rotation...
     math::Vect<3,real64> aa;
      aa[0] = vec[7 + i*6 + 0];
      aa[1] = vec[7 + i*6 + 1];
      aa[2] = vec[7 + i*6 + 2];     
     math::Mat<3,3,real64> temp;
      AngAxisToRotMat(aa,temp);
     
     extrinsic[i][0][0] = temp[0][0]; extrinsic[i][0][1] = temp[0][1]; extrinsic[i][0][2] = temp[0][2];
     extrinsic[i][1][0] = temp[1][0]; extrinsic[i][1][1] = temp[1][1]; extrinsic[i][1][2] = temp[1][2];
     extrinsic[i][2][0] = temp[2][0]; extrinsic[i][2][1] = temp[2][1]; extrinsic[i][2][2] = temp[2][2];
      
    // Translation...
     extrinsic[i][0][3] = vec[7 + i*6 + 3];
     extrinsic[i][1][3] = vec[7 + i*6 + 4];
     extrinsic[i][2][3] = vec[7 + i*6 + 5];
   }
 }
 if (targQ==normal) {resQ = normal; delete[] sd; return;}



 //
 // Third estimate, LM on all paramters...
 //
 prog->Report(4,prog_steps);
 {
  // We use the following vector to describe the problem:
  // [Intrinsic Matrix,radial centre,4 * radial parameters,# shots of sv]^T (11)
  // where sv is a vector for each shot:
  // [rotation matrix as 3 vector, translation as 3 vector]^T (6 * # of shots)
  // We then minimise this with a x and y difference as the residual.
  // (So for a typical 
 
  // Create the initial vector...
   math::Vector<real64> vec(11+shots*6);
   // Global data...
    vec[0]  = intrinsic.PrincipalX();
    vec[1]  = intrinsic.PrincipalY();
    vec[2]  = intrinsic.FocalX();
    vec[3]  = intrinsic.FocalY();
    vec[4]  = intrinsic.Skew();    
    vec[5]  = radial.centre[0];
    vec[6]  = radial.centre[1];
    vec[7]  = radial.k[0];
    vec[8]  = radial.k[1];
    vec[9]  = radial.k[2];
    vec[10] = radial.k[3];
           
   // Inverse intrinsic matrix, as we need it for the below loop...
    math::Mat<3,3,real64> invInt = intrinsic;
    math::Mat<3,3,real64> temp;
    Inverse(invInt,temp);
  
   // Data for each shot...
    for (nat32 i=0;i<shots;i++)
    {
     // Rotation...
      math::Mat<3,3,real64> temp;
       temp[0][0] = extrinsic[i][0][0]; temp[0][1] = extrinsic[i][0][1]; temp[0][2] = extrinsic[i][0][2];
       temp[1][0] = extrinsic[i][1][0]; temp[1][1] = extrinsic[i][1][1]; temp[1][2] = extrinsic[i][1][2];
       temp[2][0] = extrinsic[i][2][0]; temp[2][1] = extrinsic[i][2][1]; temp[2][2] = extrinsic[i][2][2];
      
      math::Vect<3,real64> aa;
       RotMatToAngAxis(temp,aa);
     
      vec[11 + i*6 + 0] = aa[0];
      vec[11 + i*6 + 1] = aa[1];
      vec[11 + i*6 + 2] = aa[2];
      
     // Translation...
      vec[11 + i*6 + 3] = extrinsic[i][0][3];
      vec[11 + i*6 + 4] = extrinsic[i][1][3];
      vec[11 + i*6 + 5] = extrinsic[i][2][3];      
    }


  // Run LM...
   LMdata lmd;
    lmd.shots = shots;
    lmd.sd = sd;
    
   nat32 evs = 0;
   for (nat32 i=0;i<shots;i++) evs += sd[i].data.Size();
   evs *= 2;
 
   real64 nRes = LM(evs,vec,lmd,&SecondLM);


  // if this has gone wrong we break on the ushall level of refinement.
   if (!math::IsFinite(nRes))
   {
    resQ = normal;
    delete[] sd;
    return;
   }
   else
   {
    residual = nRes;
    resQ = high;   
   }


  // Extract the results...
   intrinsic.PrincipalX() = vec[0];
   intrinsic.PrincipalY() = vec[1];
   intrinsic.FocalX() = vec[2];
   intrinsic.FocalY() = vec[3];
   intrinsic.Skew() = vec[4];

   radial.aspectRatio = intrinsic.AspectRatio();
   radial.centre[0] = vec[5];
   radial.centre[1] = vec[6];
   radial.k[0] = vec[7];
   radial.k[1] = vec[8];
   radial.k[2] = vec[9];
   radial.k[3] = vec[10];
   
   for (nat32 i=0;i<shots;i++)
   {
    // Rotation...
     math::Vect<3,real64> aa;
      aa[0] = vec[11 + i*6 + 0];
      aa[1] = vec[11 + i*6 + 1];
      aa[2] = vec[11 + i*6 + 2];     
     math::Mat<3,3,real64> temp;
      AngAxisToRotMat(aa,temp);
     
     extrinsic[i][0][0] = temp[0][0]; extrinsic[i][0][1] = temp[0][1]; extrinsic[i][0][2] = temp[0][2];
     extrinsic[i][1][0] = temp[1][0]; extrinsic[i][1][1] = temp[1][1]; extrinsic[i][1][2] = temp[1][2];
     extrinsic[i][2][0] = temp[2][0]; extrinsic[i][2][1] = temp[2][1]; extrinsic[i][2][2] = temp[2][2];
      
    // Translation...
     extrinsic[i][0][3] = vec[11 + i*6 + 3];
     extrinsic[i][1][3] = vec[11 + i*6 + 4];
     extrinsic[i][2][3] = vec[11 + i*6 + 5];
   }   
 }
  

 // Clean up... 
  delete[] sd;
  
 prog->Pop();
}

void Zhang98::FirstLM(const math::Vector<real64> & pv,math::Vector<real64> & err,const LMdata & oi)
{
 // Construct the intrinsic matrix object...
  Intrinsic intrinsic;
    intrinsic.PrincipalX() = pv[0];
    intrinsic.PrincipalY() = pv[1];
    intrinsic.FocalX() = pv[2];
    intrinsic.FocalY() = pv[3];
    intrinsic.Skew() = pv[4];
 
 // Construct the radial distortion object...
  Radial radial;
   radial.aspectRatio = intrinsic.AspectRatio();
   radial.centre[0] = intrinsic.PrincipalX();
   radial.centre[1] = intrinsic.PrincipalY();
   radial.k[0] = pv[5];
   radial.k[1] = pv[6];
   radial.k[2] = 0.0;
   radial.k[3] = 0.0;
    
 // For each shot calculate the relevent homography, then transform each point
 // and subtract the visualised point to get a mother of an error vector.
 nat32 ep = 0;
 for (nat32 i=0;i<oi.shots;i++)
 {
  // Calculate the homography...
   math::Mat<3,3,real64> hgo;
   math::Vect<3,real64> aa;
    aa[0] = pv[7 + i*6 + 0];
    aa[1] = pv[7 + i*6 + 1];
    aa[2] = pv[7 + i*6 + 2];
   AngAxisToRotMat(aa,hgo);
   
   hgo[0][2] = pv[7 + i*6 + 3];
   hgo[1][2] = pv[7 + i*6 + 4];
   hgo[2][2] = pv[7 + i*6 + 5];
   
   math::Mat<3,3,real64> hg;
   Mult(intrinsic,hgo,hg);

      
  // Iterate all points, for each one output the difference between our
  // calculated and actual value in x and y seperatly...
   ds::List<Node>::Cursor targ = oi.sd[i].data.FrontPtr();
   while (!targ.Bad())
   {
    // Apply homography...
     math::Vect<2,real64> t1;
     MultVectEH(hg,targ->first,t1);
     
    // Apply radial...
     math::Vect<2,real64> t2;
     radial.Dis(t1,t2);
    
    // Output difference...   
     err[ep]   = t2[0] - targ->second[0];
     err[ep+1] = t2[1] - targ->second[1];
    
    ep += 2;
    ++targ;	   
   }
 }
}

void Zhang98::SecondLM(const math::Vector<real64> & pv,math::Vector<real64> & err,const LMdata & oi)
{
 // Construct the intrinsic matrix object...
  Intrinsic intrinsic;
    intrinsic.PrincipalX() = pv[0];
    intrinsic.PrincipalY() = pv[1];
    intrinsic.FocalX() = pv[2];
    intrinsic.FocalY() = pv[3];
    intrinsic.Skew() = pv[4];
 
 // Construct the radial distortion object...
  Radial radial;
   radial.aspectRatio = intrinsic.AspectRatio();
   radial.centre[0] = pv[5];
   radial.centre[1] = pv[6];
   radial.k[0] = pv[7];
   radial.k[1] = pv[8];
   radial.k[2] = pv[9];
   radial.k[3] = pv[10];
    
 // For each shot calculate the relevent homography, then transform each point
 // and subtract the visualised point to get a mother of an error vector.
 nat32 ep = 0;
 for (nat32 i=0;i<oi.shots;i++)
 {
  // Calculate the homography...
   math::Mat<3,3,real64> hgo;
   math::Vect<3,real64> aa;
    aa[0] = pv[11 + i*6 + 0];
    aa[1] = pv[11 + i*6 + 1];
    aa[2] = pv[11 + i*6 + 2];
   AngAxisToRotMat(aa,hgo);
   
   hgo[0][2] = pv[11 + i*6 + 3];
   hgo[1][2] = pv[11 + i*6 + 4];
   hgo[2][2] = pv[11 + i*6 + 5];
      
   math::Mat<3,3,real64> hg;
   Mult(intrinsic,hgo,hg);   

     
  // Iterate all points, for each one output the difference between our
  // calculated and actual value in x and y seperatly...
   ds::List<Node>::Cursor targ = oi.sd[i].data.FrontPtr();
   while (!targ.Bad())
   {
    // Apply homography...
     math::Vect<2,real64> t1;
     MultVectEH(hg,targ->first,t1);
     
    // Apply radial...
     math::Vect<2,real64> t2;
     radial.Dis(t1,t2);
    
    // Output difference...   
     err[ep]   = t2[0] - targ->second[0];
     err[ep+1] = t2[1] - targ->second[1];
    
    ep += 2;
    ++targ;	   
   }
 }
}

//------------------------------------------------------------------------------
 };
};
