//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/cam/rectification.h"

#include "eos/bs/colours.h"
#include "eos/typestring.h"
#include "eos/file/csv.h"
#include "eos/cam/triangulation.h"
#include "eos/svt/sample.h"

namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------
// Helper structure for below function...
struct RectAB
{
 math::Mat<2,2,real64> aLeft;
 math::Mat<2,2,real64> bLeft;
 math::Mat<2,2,real64> aRight;
 math::Mat<2,2,real64> bRight;
};

// Helper function, this does the actual rectification...
// Based on the paper 'Computing Rectifying Homographies for Stereo Vision'
// by Loop and Zhang.
// Scaling and moving to a coordinate system with a suitable origin is left to
// the user.
// Rect left and right transfer unrectified coordinates to rectified coordinates.
// Returns true on success.
bit RawRectify(const Fundamental & fun,
               real64 leftWidth,real64 leftHeight,real64 rightWidth,real64 rightHeight,
               math::Mat<3,3,real64> & rectLeft,math::Mat<3,3,real64> & rectRight)
{
 // First we need the details of the projective part of the transformations...
  math::Vect<3,real64> leftW;
  math::Vect<3,real64> rightW;
  {
   // Calculate the left epipole, plus the skew-symetric version...
    math::Vect<3,real64> leftEpipole;
    {
     Fundamental temp = fun;
     if (math::RightNullSpace(temp,leftEpipole)==false) return false;
    }

    #ifdef EOS_DEBUG
    {
     math::Vect<3,real64> temp;
     math::MultVect(fun,leftEpipole,temp);
     LogDebug("[cam.rectify] left epipole {epipole,zero}" << LogDiv() << leftEpipole << LogDiv() << temp);
    }
    #endif

    math::Mat<3,3,real64> leftEpipoleSS;
    math::SkewSymetric33(leftEpipole,leftEpipoleSS);


   // Calculate the covariance and other matrix for each image...
    // Left...
     math::Mat<3,3,real64> ppLeft;
     {
      math::Zero(ppLeft);
      real64 mult = leftWidth*leftHeight/12.0;
      ppLeft[0][0] = mult * (math::Sqr(leftWidth)-1.0);
      ppLeft[1][1] = mult * (math::Sqr(leftHeight)-1.0);
     }

     math::Mat<3,3,real64> pcpcLeft;
     {
      pcpcLeft[0][0] = 0.25*math::Sqr(leftWidth-1.0);
      pcpcLeft[0][1] = 0.25*(leftWidth-1.0)*(leftHeight-1.0);
      pcpcLeft[0][2] = 0.5*(leftWidth-1.0);

      pcpcLeft[1][0] = pcpcLeft[0][1];
      pcpcLeft[1][1] = 0.25*math::Sqr(leftHeight-1.0);
      pcpcLeft[1][2] = 0.5*(leftHeight-1.0);

      pcpcLeft[2][0] = pcpcLeft[0][2];
      pcpcLeft[2][1] = pcpcLeft[1][2];
      pcpcLeft[2][2] = 1.0;
     }

    // Right...
     math::Mat<3,3,real64> ppRight;
     {
      math::Zero(ppRight);
      real64 mult = rightWidth*rightHeight/12.0;
      ppRight[0][0] = mult * (math::Sqr(rightWidth)-1.0);
      ppRight[1][1] = mult * (math::Sqr(rightHeight)-1.0);
     }

     math::Mat<3,3,real64> pcpcRight;
     {
      pcpcRight[0][0] = 0.25*math::Sqr(rightWidth-1.0);
      pcpcRight[0][1] = 0.25*(rightWidth-1.0)*(rightHeight-1.0);
      pcpcRight[0][2] = 0.5*(rightWidth-1.0);

      pcpcRight[1][0] = pcpcRight[0][1];
      pcpcRight[1][1] = 0.25*math::Sqr(rightHeight-1.0);
      pcpcRight[1][2] = 0.5*(rightHeight-1.0);

      pcpcRight[2][0] = pcpcRight[0][2];
      pcpcRight[2][1] = pcpcRight[1][2];
      pcpcRight[2][2] = 1.0;
     }


   // Calculate the a and b matrices for both cameras...
    RectAB ab;

    {
     math::Mat<3,3,real64> temp[2];

     // left...
      math::TransMult(leftEpipoleSS,ppLeft,temp[0]);
      math::Mult(temp[0],leftEpipoleSS,temp[1]);
      math::SubSet(ab.aLeft,temp[1],0,0);

      math::TransMult(leftEpipoleSS,pcpcLeft,temp[0]);
      math::Mult(temp[0],leftEpipoleSS,temp[1]);
      math::SubSet(ab.bLeft,temp[1],0,0);

     // right...
      math::TransMult(fun,ppRight,temp[0]);
      math::Mult(temp[0],fun,temp[1]);
      math::SubSet(ab.aRight,temp[1],0,0);

      math::TransMult(fun,pcpcRight,temp[0]);
      math::Mult(temp[0],fun,temp[1]);
      math::SubSet(ab.bRight,temp[1],0,0);
    }


   // Find minimas for the two components seperatly to obtain initialisation
   // for the actual minimisation process...
    // Left image component...
     math::Vect<2,real64> zIniLeft;
     {
      math::Mat<2,2,real64> d = ab.aLeft;
      if (math::Cholesky(d)==false) return false;
      math::Transpose(d);
      math::ZeroLowerTri(d);

      math::Mat<2,2,real64> temp[2];
      if (math::Inverse(d,temp[0])==false) return false;

      math::TransMult(d,ab.bLeft,temp[0]);
      math::Mult(temp[0],d,temp[1]);

      math::Vect<2,real64> y;
      math::MaxEigenVector22(temp[1],y);
      math::MultVect(d,y,zIniLeft);
     }

    // Right image component...
     math::Vect<2,real64> zIniRight;
     {
      math::Mat<2,2,real64> d = ab.aRight;
      if (math::Cholesky(d)==false) return false;
      math::Transpose(d);
      math::ZeroLowerTri(d);

      math::Mat<2,2,real64> temp[2];
      if (math::Inverse(d,temp[0])==false) return false;

      math::TransMult(d,ab.bRight,temp[0]);
      math::Mult(temp[0],d,temp[1]);

      math::Vect<2,real64> y;
      math::MaxEigenVector22(temp[1],y);
      math::MultVect(d,y,zIniRight);
     }

    // Combine to get the average...
     math::Vect<2,real64> zIni;
     {
      zIniLeft.Normalise();
      zIniRight.Normalise();

      zIni[0] = zIniLeft[0] + zIniRight[0];
      zIni[1] = zIniLeft[1] + zIniRight[1];
      zIni.Normalise();
     }


   // Iterativly find the z that produces the minimum value, unlike the source
   // paper I move to angular coordinates rather than screw arround with two
   // implimentations, I use LM for minimisation...
    math::Vect<3,real64> z;
    {
     math::Vect<1,real64> p;
     p[0] = math::InvTan2(zIni[1],zIni[0]);

     struct F
     {
      static void Func(const math::Vect<1,real64> & p,math::Vect<1,real64> & err,const RectAB & ab)
      {
       // Extract z...
        math::Vect<2,real64> z;
        z[0] = math::Cos(p[0]);
        z[1] = math::Sin(p[0]);

       // Calculate error in two parts...
        // Left...
         err[0] = math::VectMultVect(z,ab.aLeft,z) / math::VectMultVect(z,ab.bLeft,z);
        // Right...
         err[0] += math::VectMultVect(z,ab.aRight,z) / math::VectMultVect(z,ab.bRight,z);
      }
     };

     if (!math::IsFinite(math::LM(p,ab,&F::Func))) return false;

     z[0] = math::Cos(p[0]);
     z[1] = math::Sin(p[0]);
     z[2] = 0.0;
    }


   // Convert the output into the two w vectors...
    math::MultVect(leftEpipoleSS,z,leftW);
    math::MultVect(fun,z,rightW);

    leftW /= leftW[2];
    rightW /= rightW[2];

    LogDebug("[cam.rectify] projection parts {z,leftW,rightW}" << LogDiv()
             << z << LogDiv() << leftW << LogDiv() << rightW);
  }



 // Convert the projective details into the actual projective matrix ready for
 // latter...
  math::Mat<3,3,real64> projLeft;
  math::Mat<3,3,real64> projRight;
  {
   math::Identity(projLeft);
   projLeft[2][0] = leftW[0];
   projLeft[2][1] = leftW[1];

   math::Identity(projRight);
   projRight[2][0] = rightW[0];
   projRight[2][1] = rightW[1];
  }
  LogDebug("[cam.rectify] projection matrices" << LogDiv() << projLeft << LogDiv() << projRight);



 #ifdef EOS_DEBUG
 {
  math::Mat<3,3,real64> t;
  math::Mat<3,3,real64> invProjLeft = projLeft;
  math::Mat<3,3,real64> invProjRight = projRight;
  math::Inverse(invProjLeft,t);
  math::Inverse(invProjRight,t);

  Fundamental temp[2];
  math::TransMult(invProjRight,fun,temp[0]);
  math::Mult(temp[0],invProjLeft,temp[1]);
  LogDebug("[cam.rectify] fundamental matrix modified by projection alone" << LogDiv() << temp[1]);

  math::Vect<3,real64> tempEpi[2];
  temp[0] = temp[1];
  math::RightNullSpace(temp[0],tempEpi[0]);
  temp[0] = temp[1];
  math::Transpose(temp[0]);
  math::RightNullSpace(temp[0],tempEpi[1]);
  LogDebug("[cam.rectify] epipoles {left,right}" << LogDiv() << tempEpi[0] << LogDiv() << tempEpi[1]);
 }
 #endif



 // Now the similarity transform...
  math::Mat<3,3,real64> simLeft;
  math::Mat<3,3,real64> simRight;
  {
   // Left...
    simLeft[0][0] = fun[2][1] - leftW[1]*fun[2][2];
    simLeft[0][1] = leftW[0]*fun[2][2] - fun[2][0];
    simLeft[0][2] = 0.0;

    simLeft[1][0] = fun[2][0] - leftW[0]*fun[2][2];
    simLeft[1][1] = fun[2][1] - leftW[1]*fun[2][2];
    simLeft[1][2] = fun[2][2];

    simLeft[2][0] = 0.0;
    simLeft[2][1] = 0.0;
    simLeft[2][2] = 1.0;

   // Right...
    simRight[0][0] = rightW[1]*fun[2][2] - fun[1][2];
    simRight[0][1] = fun[0][2] - rightW[0]*fun[2][2];
    simRight[0][2] = 0.0;

    simRight[1][0] = rightW[0]*fun[2][2] - fun[0][2];
    simRight[1][1] = rightW[1]*fun[2][2] - fun[1][2];
    simRight[1][2] = 0.0;

    simRight[2][0] = 0.0;
    simRight[2][1] = 0.0;
    simRight[2][2] = 1.0;
  }
  LogDebug("[cam.rectify] similarity matrices" << LogDiv() << simLeft << LogDiv() << simRight);



 #ifdef EOS_DEBUG
 {
  math::Mat<3,3,real64> bothLeft;
  math::Mult(simLeft,projLeft,bothLeft);

  math::Mat<3,3,real64> bothRight;
  math::Mult(simRight,projRight,bothRight);

  math::Mat<3,3,real64> t;
  math::Inverse(bothLeft,t);
  math::Inverse(bothRight,t);

  Fundamental temp[2];
  math::TransMult(bothRight,fun,temp[0]);
  math::Mult(temp[0],bothLeft,temp[1]);
  LogDebug("[cam.rectify] rectified fundamental matrix" << LogDiv() << temp[1]);

  math::Vect<3,real64> tempEpi[2];
  temp[0] = temp[1];
  math::RightNullSpace(temp[0],tempEpi[0]);
  temp[0] = temp[1];
  math::Transpose(temp[0]);
  math::RightNullSpace(temp[0],tempEpi[1]);
  LogDebug("[cam.rectify] epipoles {left,right}" << LogDiv() << tempEpi[0] << LogDiv() << tempEpi[1]);
 }
 #endif



 // Finally the shear transforms...
  math::Mat<3,3,real64> shearLeft;
  {
   // Calculate the midpoints of the edges...
    math::Vect<3,real64> mp[4];
    mp[0][0] = 0.5*(leftWidth-1.0); mp[0][1] = 0.0;                  mp[0][2] = 1.0;
    mp[1][0] = leftWidth-1.0;       mp[1][1] = 0.5*(leftHeight-1.0); mp[1][2] = 1.0;
    mp[2][0] = 0.5*(leftWidth-1.0); mp[2][1] = leftHeight-1.0;       mp[2][2] = 1.0;
    mp[3][0] = 0.0;                 mp[3][1] = 0.5*(leftHeight-1.0); mp[3][2] = 1.0;

   // Apply the transform so far...
    math::Mat<3,3,real64> both;
    math::Mult(simLeft,projLeft,both);
    math::Vect<3,real64> mpTra[4];
    for (nat32 i=0;i<4;i++)
    {
     math::MultVect(both,mp[i],mpTra[i]);
     mpTra[i] /= mpTra[i][2];
    }

   // Calculate the x and y vectors that put a cross through the image...
    math::Vect<3,real64> x = mpTra[1];
    x -= mpTra[3];

    math::Vect<3,real64> y = mpTra[2];
    y -= mpTra[0];

   // Simple algebra will obtain the optimum shear transform...
    math::Identity(shearLeft);

    real64 aTop = math::Sqr(leftHeight)*math::Sqr(x[1]) + math::Sqr(leftWidth)*math::Sqr(y[1]);
    real64 aBot = leftWidth*leftHeight*(x[1]*y[0] - x[0]*y[1]);
    real64 bTop = math::Sqr(leftHeight)*x[0]*x[1] + math::Sqr(leftWidth)*y[0]*y[1];
    real64 bBot = leftWidth*leftHeight*(x[0]*y[1] - x[1]*y[0]);

    shearLeft[0][0] = aTop/aBot;
    shearLeft[0][1] = bTop/bBot;

    if (shearLeft[0][0]<0.0)
    {
     shearLeft[0][0] *= -1.0;
     shearLeft[0][1] *= -1.0;
    }
  }

  math::Mat<3,3,real64> shearRight;
  {
   // Calculate the midpoints of the edges...
    math::Vect<3,real64> mp[4];
    mp[0][0] = 0.5*(rightWidth-1.0); mp[0][1] = 0.0;                   mp[0][2] = 1.0;
    mp[1][0] = rightWidth-1.0;       mp[1][1] = 0.5*(rightHeight-1.0); mp[1][2] = 1.0;
    mp[2][0] = 0.5*(rightWidth-1.0); mp[2][1] = rightHeight-1.0;       mp[2][2] = 1.0;
    mp[3][0] = 0.0;                  mp[3][1] = 0.5*(rightHeight-1.0); mp[3][2] = 1.0;

   // Apply the transform so far...
    math::Mat<3,3,real64> both;
    math::Mult(simRight,projRight,both);
    math::Vect<3,real64> mpTra[4];
    for (nat32 i=0;i<4;i++)
    {
     math::MultVect(both,mp[i],mpTra[i]);
     mpTra[i] /= mpTra[i][2];
    }

   // Calculate the x and y vectors that put a cross through the image...
    math::Vect<3,real64> x = mpTra[1];
    x -= mpTra[3];

    math::Vect<3,real64> y = mpTra[2];
    y -= mpTra[0];

   // Simple algebra will obtain the optimum shear transform...
    math::Identity(shearRight);

    real64 aTop = math::Sqr(rightHeight)*math::Sqr(x[1]) + math::Sqr(rightWidth)*math::Sqr(y[1]);
    real64 aBot = rightWidth*rightHeight*(x[1]*y[0] - x[0]*y[1]);
    real64 bTop = math::Sqr(rightHeight)*x[0]*x[1] + math::Sqr(rightWidth)*y[0]*y[1];
    real64 bBot = rightWidth*rightHeight*(x[0]*y[1] - x[1]*y[0]);

    shearRight[0][0] = aTop/aBot;
    shearRight[0][1] = bTop/bBot;

    if (shearRight[0][0]<0.0)
    {
     shearRight[0][0] *= -1.0;
     shearRight[0][1] *= -1.0;
    }
  }
  LogDebug("[cam.rectify] shear matrices" << LogDiv() << shearLeft << LogDiv() << shearRight);



 // Combine all the transforms to create the output...
 {
  math::Mat<3,3,real64> temp;

  math::Mult(shearLeft,simLeft,temp);
  math::Mult(temp,projLeft,rectLeft);

  math::Mult(shearRight,simRight,temp);
  math::Mult(temp,projRight,rectRight);
 }
 LogDebug("[cam.rectify] output matrices" << LogDiv() << rectLeft << LogDiv() << rectRight);



 #ifdef EOS_DEBUG
 {
  math::Mat<3,3,real64> t;
  math::Mat<3,3,real64> invLeft = rectLeft;
  math::Mat<3,3,real64> invRight = rectRight;
  math::Inverse(invLeft,t);
  math::Inverse(invRight,t);

  Fundamental temp[2];
  math::TransMult(invRight,fun,temp[0]);
  math::Mult(temp[0],invLeft,temp[1]);
  LogDebug("[cam.rectify] sheared rectified fundamental matrix" << LogDiv() << temp[1]);

  math::Vect<3,real64> tempEpi[2];
  temp[0] = temp[1];
  math::RightNullSpace(temp[0],tempEpi[0]);
  temp[0] = temp[1];
  math::Transpose(temp[0]);
  math::RightNullSpace(temp[0],tempEpi[1]);
  LogDebug("[cam.rectify] epipoles {left,right}" << LogDiv() << tempEpi[0] << LogDiv() << tempEpi[1]);
 }
 #endif


 return true;
}

//------------------------------------------------------------------------------
// This is given an input and output field of real32's plus a matrix
// transforming from the output to the input and a sampling count, an optional
// input mask can also be provided, an output mask is a must.
// It fills in the output as it can from the input using the transform, sets
// the mask to false where no data is avaliable...
void RectifySampleReal(const svt::Field<real32> & in,svt::Field<real32> & out,
                       const svt::Field<bit> * inMask,svt::Field<bit> * outMask,
                       const math::Mat<3,3,real64> & o2i,const Radial & rad,nat32 samples,time::Progress * prog)
{
 prog->Push();
 for (nat32 y=0;y<out.Size(1);y++)
 {
  prog->Report(y,out.Size(1));
  for (nat32 x=0;x<out.Size(0);x++)
  {
   out.Get(x,y) = 0.0;
   nat32 ss = 0;
   for (nat32 v=0;v<samples;v++)
   {
    for (nat32 u=0;u<samples;u++)
    {
     // Calculate position to sample...
      // Output position...
       math::Vect<3,real64> sa;
        sa[0] = x + real64(u+1)/real64(samples+1) - 0.5;
        sa[1] = y + real64(v+1)/real64(samples+1) - 0.5;
        sa[2] = 1.0;

      // Undistorted input position...
       math::Vect<3,real64> sb;
       math::MultVect(o2i,sa,sb);
       sb /= sb[2];

       math::Vect<2,real64> sc;
       sc[0] = sb[0];
       sc[1] = sb[1];

      // Distorted input position...
       math::Vect<2,real64> sd;
       rad.Dis(sc,sd);


     // Sample it, handling if the location is bad...
      real32 sample;
      if (SampleRealLin2D(sample,sd[0],sd[1],in,inMask))
      {
       ++ss;
       out.Get(x,y) += sample;
      }
    }
   }

   if (ss!=0)
   {
    if (outMask) outMask->Get(x,y) = true;
    out.Get(x,y) /= ss;
   }
   else
   {
    if (outMask) outMask->Get(x,y) = false;
   }
  }
 }
 prog->Pop();
}

// The colour version of above...
void RectifySampleColour(const svt::Field<bs::ColourRGB> & in,svt::Field<bs::ColourRGB> & out,
                         const svt::Field<bit> * inMask,svt::Field<bit> * outMask,
                         const math::Mat<3,3,real64> & o2i,const Radial & rad,nat32 samples,time::Progress * prog)
{
 prog->Push();
 for (nat32 y=0;y<out.Size(1);y++)
 {
  prog->Report(y,out.Size(1));
  for (nat32 x=0;x<out.Size(0);x++)
  {
   out.Get(x,y) = bs::ColourRGB(0.0,0.0,0.0);
   nat32 ss = 0;
   for (nat32 v=0;v<samples;v++)
   {
    for (nat32 u=0;u<samples;u++)
    {
     // Calculate position to sample...
      // Output position...
       math::Vect<3,real64> sa;
        sa[0] = x + real64(u+1)/real64(samples+1) - 0.5;
        sa[1] = y + real64(v+1)/real64(samples+1) - 0.5;
        sa[2] = 1.0;

      // Undistorted input position...
       math::Vect<3,real64> sb;
       math::MultVect(o2i,sa,sb);
       sb /= sb[2];

       math::Vect<2,real64> sc;
       sc[0] = sb[0];
       sc[1] = sb[1];

      // Distorted input position...
       math::Vect<2,real64> sd;
       rad.Dis(sc,sd);


     // Check if its a valid location...
      bs::ColourRGB sample;
      if (SampleColourRGBLin2D(sample,sd[0],sd[1],in,inMask))
      {
       ++ss;
       out.Get(x,y) += sample;
      }
    }
   }

   if (ss!=0)
   {
    if (outMask) outMask->Get(x,y) = true;
    out.Get(x,y) /= ss;
   }
   else
   {
    if (outMask) outMask->Get(x,y) = false;
    out.Get(x,y) = bs::ColourRGB(1.0,0.0,1.0);
   }
  }
 }
 prog->Pop();
}

//------------------------------------------------------------------------------
EOS_FUNC bit PlaneRectify(svt::Var * inA,svt::Var * inB,
                          const Radial & radA,const Radial & radB,const Fundamental & fun,
                          svt::Var * outA,svt::Var * outB,
                          math::Mat<3,3,real64> * outTA,math::Mat<3,3,real64> * outTB,
                          nat32 samples,bit doOriginal,bit doMask,time::Progress * prog)
{
 LogBlock("eos::cam::PlaneRectify(...)","{fun}" << LogDiv() << fun);
 prog->Push();
 prog->Report(0,3);


 // Use an external function to do the 'heavy' maths...
  math::Mat<3,3,real64> traA;
  math::Mat<3,3,real64> traB;
  if (RawRectify(fun,inA->Size(0),inA->Size(1),inB->Size(0),inB->Size(1),traA,traB)==false)
  {
   prog->Pop();
   return false;
  }


 // Vraiables to be filled in by the below code...
  bit revY = false;
  bit revLeftX = false;
  bit revRightX = false;

 // Find the range of the first image...
  real64 minAX,maxAX,minAY,maxAY;
  {
   math::Vect<2,real64> val[8];
    val[0][0] = 0.0;              val[0][1] = 0.0;
    val[1][0] = radA.centre[0];   val[1][1] = 0.0;
    val[2][0] = inA->Size(0)-1.0; val[2][1] = 0.0;
    val[3][0] = inA->Size(0)-1.0; val[3][1] = radA.centre[1];
    val[4][0] = inA->Size(0)-1.0; val[4][1] = inA->Size(1)-1.0;
    val[5][0] = radA.centre[0];   val[5][1] = inA->Size(1)-1.0;
    val[6][0] = 0.0;              val[6][1] = inA->Size(1)-1.0;
    val[7][0] = 0.0;              val[7][1] = radA.centre[1];

   math::Vect<3,real64> lowCorner;
   math::Vect<3,real64> highCorner;
   for (nat32 i=0;i<8;i++)
   {
    math::Vect<2,real64> unDis;
    radA.UnDis(val[i],unDis);

    math::Vect<3,real64> unDisH;
    unDisH[0] = unDis[0];
    unDisH[1] = unDis[1];
    unDisH[2] = 1.0;

    math::Vect<3,real64> rectP;
    math::MultVect(traA,unDisH,rectP);
    rectP /= rectP[2];

    if (i==0) lowCorner = rectP;
    if (i==4) highCorner = rectP;

    LogDebug("[cam.rectify] Left extremity transfer {unrect,rectified}" << LogDiv()
             << val[i] << LogDiv() << rectP);

    if (i==0)
    {
     minAX = rectP[0]; maxAX = rectP[0];
     minAY = rectP[1]; maxAY = rectP[1];
    }
    else
    {
     minAX = math::Min(minAX,rectP[0]);
     maxAX = math::Max(maxAX,rectP[0]);
     minAY = math::Min(minAY,rectP[1]);
     maxAY = math::Max(maxAY,rectP[1]);
    }
   }

   revLeftX = highCorner[0] < lowCorner[0];
   revY = highCorner[1] < lowCorner[1];
  }


 // Find the range of the second image...
  real64 minBX,maxBX,minBY,maxBY;
  {
   math::Vect<2,real64> val[8];
    val[0][0] = 0.0;              val[0][1] = 0.0;
    val[1][0] = radB.centre[0];   val[1][1] = 0.0;
    val[2][0] = inB->Size(0)-1.0; val[2][1] = 0.0;
    val[3][0] = inB->Size(0)-1.0; val[3][1] = radB.centre[1];
    val[4][0] = inB->Size(0)-1.0; val[4][1] = inB->Size(1)-1.0;
    val[5][0] = radB.centre[0];   val[5][1] = inB->Size(1)-1.0;
    val[6][0] = 0.0;              val[6][1] = inB->Size(1)-1.0;
    val[7][0] = 0.0;              val[7][1] = radB.centre[1];

   math::Vect<3,real64> lowCorner;
   math::Vect<3,real64> highCorner;
   for (nat32 i=0;i<8;i++)
   {
    math::Vect<2,real64> unDis;
    radB.UnDis(val[i],unDis);

    math::Vect<3,real64> unDisH;
    unDisH[0] = unDis[0];
    unDisH[1] = unDis[1];
    unDisH[2] = 1.0;

    math::Vect<3,real64> rectP;
    math::MultVect(traB,unDisH,rectP);
    rectP /= rectP[2];

    if (i==0) lowCorner = rectP;
    if (i==4) highCorner = rectP;

    LogDebug("[cam.rectify] Right extremity transfer {unrect,rectified}" << LogDiv()
             << val[i] << LogDiv() << rectP);

    if (i==0)
    {
     minBX = rectP[0]; maxBX = rectP[0];
     minBY = rectP[1]; maxBY = rectP[1];
    }
    else
    {
     minBX = math::Min(minBX,rectP[0]);
     maxBX = math::Max(maxBX,rectP[0]);
     minBY = math::Min(minBY,rectP[1]);
     maxBY = math::Max(maxBY,rectP[1]);
    }
   }

   revRightX = highCorner[0] < lowCorner[0];
  }

  LogDebug("[cam.rectify] Transform A's range {minX,maxX,minY,maxY}" <<
           LogDiv() << minAX << LogDiv() << maxAX << LogDiv() << minAY << LogDiv() << maxAY);
  LogDebug("[cam.rectify] Transform B's range {minX,maxX,minY,maxY}" <<
           LogDiv() << minBX << LogDiv() << maxBX << LogDiv() << minBY << LogDiv() << maxBY);


 // Now update the two transforms such that they are of a scale suitable for our
 // input images, i.e. so we don't lose (much) detail due to changes of scale...
 // We take the point in the middle of both images and calculate the epipoler line
 // it inhabits, we then take a short span along this epipolar line and transform
 // it into rectified space, where we take its length. We also do this
 // perpendicular to the epipolar line. A scale factor is then selected for the
 // x and y axis independently so as to make the smaller of the two transformed
 // distances match.
 {
  const real64 spanLen = 20.0;

  // Calculate the epipoles of both images...
   math::Vect<3,real64> epiA;
   math::Vect<3,real64> epiB;
   {
    Fundamental funTemp = fun;
    math::RightNullSpace(funTemp,epiA);
    funTemp = fun;
    math::Transpose(funTemp);
    math::RightNullSpace(funTemp,epiB);
   }

  // Calculate and normalise the epipolar line through the centre point of each image...
   math::Vect<3,real64> centreA;
   math::Vect<3,real64> centreB;
    centreA[0] = 0.5*inA->Size(0); centreA[1] = 0.5*inA->Size(1); centreA[2] = 1.0;
    centreB[0] = 0.5*inB->Size(0); centreB[1] = 0.5*inB->Size(1); centreB[2] = 1.0;
   math::Vect<3,real64> lineA;
   math::Vect<3,real64> lineB;
    math::CrossProduct(centreA,epiA,lineA);
    math::CrossProduct(centreB,epiB,lineB);
   lineA.NormLine();
   lineB.NormLine();

  // Calculate the lengths of the various transformed spans...
   real64 xSpanA,xSpanB,ySpanA,ySpanB;
   {
    math::Vect<3,real64> p;
    math::Vect<3,real64> p1r;
    math::Vect<3,real64> p2r;

    // xSpanA...
     p = centreA;
     p[0] +=  0.5*spanLen*lineA[1];
     p[1] += -0.5*spanLen*lineA[0];
     math::MultVect(traA,p,p1r);
     p1r /= p1r[2];

     p = centreA;
     p[0] += -0.5*spanLen*lineA[1];
     p[1] +=  0.5*spanLen*lineA[0];
     math::MultVect(traA,p,p2r);
     p2r /= p2r[2];

     p1r -= p2r;
     xSpanA = p1r.Length();

    // ySpanA...
     p = centreA;
     p[0] += 0.5*spanLen*lineA[0];
     p[1] += 0.5*spanLen*lineA[1];
     math::MultVect(traA,p,p1r);
     p1r /= p1r[2];

     p = centreA;
     p[0] += -0.5*spanLen*lineA[0];
     p[1] += -0.5*spanLen*lineA[1];
     math::MultVect(traA,p,p2r);
     p2r /= p2r[2];

     p1r -= p2r;
     ySpanA = p1r.Length();

    // xSpanB...
     p = centreB;
     p[0] +=  0.5*spanLen*lineB[1];
     p[1] += -0.5*spanLen*lineB[0];
     math::MultVect(traB,p,p1r);
     p1r /= p1r[2];

     p = centreB;
     p[0] += -0.5*spanLen*lineB[1];
     p[1] +=  0.5*spanLen*lineB[0];
     math::MultVect(traB,p,p2r);
     p2r /= p2r[2];

     p1r -= p2r;
     xSpanB = p1r.Length();

    // ySpanB...
     p = centreB;
     p[0] += 0.5*spanLen*lineB[0];
     p[1] += 0.5*spanLen*lineB[1];
     math::MultVect(traB,p,p1r);
     p1r /= p1r[2];

     p = centreB;
     p[0] += -0.5*spanLen*lineB[0];
     p[1] += -0.5*spanLen*lineB[1];
     math::MultVect(traB,p,p2r);
     p2r /= p2r[2];

     p1r -= p2r;
     ySpanB = p1r.Length();
   }


  // Select the scale factor, update the transforms and ranges accordingly...
   real64 scaleXA = spanLen/xSpanA;
   real64 scaleXB = spanLen/xSpanB;
   real64 scaleY = spanLen/math::Max(ySpanA,ySpanB);

   if (revLeftX) scaleXA *= -1.0;
   if (revRightX) scaleXB *= -1.0;
   if (revY) scaleY *= -1.0;

   LogDebug("[cam.rectify] Scale factors {xa,xb,y}" << LogDiv() << scaleXA << LogDiv() << scaleXB << LogDiv() << scaleY);

   for (nat32 i=0;i<3;i++)
   {
    traA[0][i] *= scaleXA;
    traA[1][i] *= scaleY;
    traB[0][i] *= scaleXB;
    traB[1][i] *= scaleY;
   }

   minAX *= scaleXA; maxAX *= scaleXA;
   minAY *= scaleY; maxAY *= scaleY;
   minBX *= scaleXB; maxBX *= scaleXB;
   minBY *= scaleY; maxBY *= scaleY;

   if (revLeftX) math::Swap(minAX,maxAX);
   if (revRightX) math::Swap(minBX,maxBX);
   if (revY)
   {
    math::Swap(minAY,maxAY);
    math::Swap(minBY,maxBY);
   }
 }


 // Testing code...
 #ifdef EOS_DEBUG
 {
  math::Mat<3,3,real64> temp[2];
  // Inverses...
   math::Mat<3,3,real64> invTA = traA;
   math::Inverse(invTA,temp[0]);
   math::Mat<3,3,real64> invTB = traB;
   math::Inverse(invTB,temp[0]);

  // Calculate the fundamental matrix as it is under the transformations
  // and log it, so it can be verified as being correct...
   math::TransMult(invTB,fun,temp[0]);
   math::Mult(temp[0],invTA,temp[1]);

   temp[1] /= temp[1][1][2];

   LogDebug("[cam.rectify] Post-scale fundamental matrix" << LogDiv() << temp[1]);
 }
 #endif


 // Offset the transformations so the output starts at 0,
 // just a conveniance, and calculate the actual dimensions of the output...
  nat32 sizeXA,sizeXB,sizeY;
  {
   math::Mat<3,3,real64> offset;
    math::Identity(offset);
    offset[1][2] = -math::Max(minAY,minBY);

   math::Mat<3,3,real64> temp2;
   offset[0][2] = -minAX; math::Mult(offset,traA,temp2); traA = temp2;
   offset[0][2] = -minBX; math::Mult(offset,traB,temp2); traB = temp2;

   sizeXA = nat32(math::RoundUp(maxAX - minAX));
   sizeXB = nat32(math::RoundUp(maxBX - minBX));
   sizeY = nat32(math::RoundUp(math::Min(maxAY,maxBY) - math::Max(minAY,minBY)));
  }
  traA /= math::FrobNorm(traA);
  traB /= math::FrobNorm(traB);

  LogDebug("[cam.rectify] Final transforms {a,b}" << LogDiv() << traA << LogDiv() << traB);
  LogDebug("[cam.rectify] Image sizes {A's width,B's width,Shared height}" <<
           LogDiv() << sizeXA << LogDiv() << sizeXB << LogDiv() << sizeY);


 // Invert the transforms - we take a sampling approach and
 // so want to go from the new images to the old images, not
 // the way we are currently going...
 {
  math::Mat<3,3,real64> temp;
  math::Inverse(traA,temp);
  math::Inverse(traB,temp);

  if (outTA) *outTA = traA;
  if (outTB) *outTB = traB;
 }

 LogDebug("[cam.rectify] Inverse transforms {a,b}" << LogDiv() << traA << LogDiv() << traB);


 // Testing code...
 #ifdef EOS_DEBUG
 {
  // Calculate the fundamental matrix as it is under the transformations
  // and log it, so it can be verified as being correct...
   math::Mat<3,3,real64> temp[2];
   math::TransMult(traB,fun,temp[0]);
   math::Mult(temp[0],traA,temp[1]);

   temp[1] /= temp[1][1][2];

   LogDebug("[cam.rectify] Transformed fundamental matrix" << LogDiv() << temp[1]);
 }
 #endif


 // Tok's needed below...
  str::Token maskTok = outA->GetCore().GetTT()("mask");
  str::Token realTok = outA->GetCore().GetTT()(typestring<real32>());
  str::Token greyTok = outA->GetCore().GetTT()(typestring<bs::ColourL>());
  str::Token colTok = outA->GetCore().GetTT()(typestring<bs::ColourRGB>());


 // Generate the output Var's...
 prog->Report(1,3);
 {
  prog->Push();
  prog->Report(0,2);

  bit maskIni = false;
  bs::Pnt originalIni = bs::Pnt(0.0,0.0);

  outA->Setup2D(sizeXA,sizeY);
   for (nat32 i=0;i<inA->Fields();i++)
   {
    if ((inA->FieldType(i)==realTok)||(inA->FieldType(i)==greyTok)||(inA->FieldType(i)==colTok))
    {
     outA->Add(inA->FieldName(i),inA->FieldType(i),inA->FieldSize(i),inA->FieldDef(i));
    }
   }
   if (doMask) outA->Add(maskTok,maskIni);
   if (doOriginal) outA->Add("original",originalIni);
  outA->Commit();

  prog->Report(1,2);
  outB->Setup2D(sizeXB,sizeY);
   for (nat32 i=0;i<inB->Fields();i++)
   {
    if ((inB->FieldType(i)==realTok)||(inB->FieldType(i)==greyTok)||(inB->FieldType(i)==colTok))
    {
     outB->Add(inB->FieldName(i),inB->FieldType(i),inB->FieldSize(i),inB->FieldDef(i));
    }
   }
   if (doMask) outB->Add(maskTok,maskIni);
   if (doOriginal) outB->Add("original",originalIni);
  outB->Commit();

  prog->Pop();
 }


 // And finally apply the two transforms. This is also dam complicated,
 // so we impliment it elsewhere and just have to loop the fields here...
 // (This is the time consuming bit)
 {
  prog->Report(2,3);
  prog->Push();



   // All fields in image A...
    nat32 steps = outA->Fields() + outB->Fields();
    svt::Field<bit> inMaskA(inA,"mask");
    svt::Field<bit> outMaskA(outA,"mask");
    for (nat32 i=0;i<outA->Fields();i++)
    {
     LogDebug("[cam.rectify] Considering A field {field}" << LogDiv() << inA->GetCore().GetTT().Str(outA->FieldName(i)));
     prog->Report(i,steps);
     if ((outA->FieldType(i)==realTok)||(outA->FieldType(i)==greyTok))
     {
      LogDebug("[cam.rectify] Rectifying field of real32's in A");
      svt::Field<real32> in(inA,outA->FieldName(i));
      svt::Field<real32> out(outA,outA->FieldName(i));
      RectifySampleReal(in,out,inMaskA.Valid()?(&inMaskA):null<svt::Field<bit>*>(),outMaskA.Valid()?(&outMaskA):null<svt::Field<bit>*>(),traA,radA,samples,prog);
     }
     else if (outA->FieldType(i)==colTok)
     {
      LogDebug("[cam.rectify] Rectifying field of ColourRGB's in A");
      svt::Field<bs::ColourRGB> in(inA,outA->FieldName(i));
      svt::Field<bs::ColourRGB> out(outA,outA->FieldName(i));
      RectifySampleColour(in,out,inMaskA.Valid()?(&inMaskA):null<svt::Field<bit>*>(),outMaskA.Valid()?(&outMaskA):null<svt::Field<bit>*>(),traA,radA,samples,prog);
     }
     else if (outA->FieldType(i)==maskTok)
     {
      // Do nothing:-)
     }
     else
     {
      // Must be the "original" field...
       svt::Field<bs::Pnt> oriA(outA,"original");
       prog->Push();
       for (nat32 y=0;y<oriA.Size(1);y++)
       {
        prog->Report(y,oriA.Size(1));
        for (nat32 x=0;x<oriA.Size(0);x++)
        {
         math::Vect<3,real64> a;
          a[0] = x; a[1] = y; a[2] = 1.0;
         math::Vect<3,real64> b;
         math::MultVect(traA,a,b);
         b /= b[2];
         oriA.Get(x,y)[0] = b[0];
         oriA.Get(x,y)[1] = b[1];
        }
       }
       prog->Pop();
     }
    }

   // All fields in image B...
    svt::Field<bit> inMaskB(inB,"mask");
    svt::Field<bit> outMaskB(outB,"mask");
    for (nat32 i=0;i<outB->Fields();i++)
    {
     LogDebug("[cam.rectify] Considering B field {field}" << LogDiv() << inB->GetCore().GetTT().Str(outB->FieldName(i)));
     prog->Report(outA->Fields() + i,steps);
     if ((outB->FieldType(i)==realTok)||(outB->FieldType(i)==greyTok))
     {
      LogDebug("[cam.rectify] Rectifying field of real32's in B");
      svt::Field<real32> in(inB,outB->FieldName(i));
      svt::Field<real32> out(outB,outB->FieldName(i));
      RectifySampleReal(in,out,inMaskB.Valid()?(&inMaskB):null<svt::Field<bit>*>(),outMaskB.Valid()?(&outMaskB):null<svt::Field<bit>*>(),traB,radB,samples,prog);
     }
     else if (outB->FieldType(i)==colTok)
     {
      LogDebug("[cam.rectify] Rectifying field of ColourRGB's in B");
      svt::Field<bs::ColourRGB> in(inB,outB->FieldName(i));
      svt::Field<bs::ColourRGB> out(outB,outB->FieldName(i));
      RectifySampleColour(in,out,inMaskB.Valid()?(&inMaskB):null<svt::Field<bit>*>(),outMaskB.Valid()?(&outMaskB):null<svt::Field<bit>*>(),traB,radB,samples,prog);
     }
     else if (outB->FieldType(i)==maskTok)
     {
      // Do nothing:-)
     }
     else
     {
      // Must be the "original" field...
       svt::Field<bs::Pnt> oriB(outB,"original");
       prog->Push();
       for (nat32 y=0;y<oriB.Size(1);y++)
       {
        prog->Report(y,oriB.Size(1));
        for (nat32 x=0;x<oriB.Size(0);x++)
        {
         math::Vect<3,real64> a;
          a[0] = x; a[1] = y; a[2] = 1.0;
         math::Vect<3,real64> b;
         math::MultVect(traB,a,b);
         b /= b[2];
         oriB.Get(x,y)[0] = b[0];
         oriB.Get(x,y)[1] = b[1];
        }
       }
       prog->Pop();
     }
    }


  prog->Pop();
 }

 prog->Pop();
 return true;
}

//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
 };
};
