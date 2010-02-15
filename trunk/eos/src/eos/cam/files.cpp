//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/cam/files.h"

#include "eos/str/tokens.h"
#include "eos/file/xml.h"
#include "eos/cam/triangulation.h"
#include "eos/ds/arrays2d.h"
#include "eos/math/mat_ops.h"

namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------
void CameraCalibration::SetDefault(real64 width,real64 height)
{
 math::Identity(intrinsic);

 radial.aspectRatio = 1.0;
 radial.centre[0] = 0.5*width;
 radial.centre[1] = 0.5*height;
 radial.k[0] = 0.0;
 radial.k[1] = 0.0;
 radial.k[2] = 0.0;
 radial.k[3] = 0.0;

 dim[0] = width;
 dim[1] = height;
}

void CameraCalibration::Crop(real64 minX,real64 dimX,real64 minY,real64 dimY)
{
 // Update intrinsic...
  intrinsic.Offset(minX,minY);

 // Update radial...
  math::Vect<2,real64> dis;
  math::Vect<2,real64> unDis;
   dis[0] = minX;
   dis[1] = minY;

  radial.UnDis(dis,unDis);
  radial.Offset(unDis[0],unDis[1]);

 // Update dimensions...
  dim[0] = dimX;
  dim[1] = dimY;
}

void CameraCalibration::Load(const bs::Element & root)
{
 dim[0] = root.GrabReal(":resolution.width",320.0);
 dim[1] = root.GrabReal(":resolution.height",240.0);

 intrinsic.Load(*root.GetElement("intrinsic"));
 radial.Load(*root.GetElement("radial"));
}

bit CameraCalibration::Load(cstrconst fn)
{
 str::TokenTable tt;
 bs::Element * root = file::LoadXML(tt,fn);
 if (root==null<bs::Element*>()) return false;

 Load(*root);

 delete root;
 return true;
}

bit CameraCalibration::Load(const str::String & fn)
{
 str::TokenTable tt;
 bs::Element * root = file::LoadXML(tt,fn);
 if (root==null<bs::Element*>()) return false;

 Load(*root);

 delete root;
 return true;
}

void CameraCalibration::Save(bs::Element & root)
{
 bs::Element * origin = root.NewChild("origin");
  origin->SetAttribute("value",str::String("bottom-left"));

 bs::Element * res = root.NewChild("resolution");
  res->SetAttribute("width",dim[0]);
  res->SetAttribute("height",dim[1]);

 bs::Element * intr = root.NewChild("intrinsic");
 intrinsic.Save(*intr);

 bs::Element * rad = root.NewChild("radial");
 radial.Save(*rad);
}

bit CameraCalibration::Save(cstrconst fn,bit overwrite)
{
 // Construct the dom...
  str::TokenTable tt;
  bs::Element root(tt,"calibration");
  Save(root);

 // Save the dom to the user selected filename...
  return file::SaveXML(&root,fn,overwrite);
}

bit CameraCalibration::Save(const str::String & fn,bit overwrite)
{
 // Construct the dom...
  str::TokenTable tt;
  bs::Element root(tt,"calibration");
  Save(root);

 // Save the dom to the user selected filename...
  return file::SaveXML(&root,fn,overwrite);
}

//------------------------------------------------------------------------------
void CameraFull::Load(const bs::Element & root)
{
 dim[0] = root.GrabReal(":resolution.width",320.0);
 dim[1] = root.GrabReal(":resolution.height",240.0);

 camera.Load(*root.GetElement("camera"));
 radial.Load(*root.GetElement("radial"));
}

bit CameraFull::Load(cstrconst fn)
{
 str::TokenTable tt;
 bs::Element * root = file::LoadXML(tt,fn);
 if (root==null<bs::Element*>()) return false;

 Load(*root);

 delete root;
 return true;
}

bit CameraFull::Load(const str::String & fn)
{
 str::TokenTable tt;
 bs::Element * root = file::LoadXML(tt,fn);
 if (root==null<bs::Element*>()) return false;

 Load(*root);

 delete root;
 return true;
}

void CameraFull::Save(bs::Element & root)
{
 bs::Element * origin = root.NewChild("origin");
  origin->SetAttribute("value",str::String("bottom-left"));

 bs::Element * res = root.NewChild("resolution");
  res->SetAttribute("width",dim[0]);
  res->SetAttribute("height",dim[1]);

 camera.Save(*root.NewChild("camera"));

 bs::Element * rad = root.NewChild("radial");
 radial.Save(*rad);
}

bit CameraFull::Save(cstrconst fn,bit overwrite)
{
 // Construct the dom...
  str::TokenTable tt;
  bs::Element root(tt,"calibration");
  Save(root);

 // Save the dom to the user selected filename...
  return file::SaveXML(&root,fn,overwrite);
}

bit CameraFull::Save(const str::String & fn,bit overwrite)
{
 // Construct the dom...
  str::TokenTable tt;
  bs::Element root(tt,"calibration");
  Save(root);

 // Save the dom to the user selected filename...
  return file::SaveXML(&root,fn,overwrite);
}

//------------------------------------------------------------------------------
void CameraPair::SetDefault(real64 width,real64 height)
{
 left.SetDefault(width,height);
 right.SetDefault(width,height);

 math::Zero(fun);
 fun[1][2] = -1.0;
 fun[2][1] = 1.0;

 gap = 1.0;

 math::Identity(lp);
 math::Identity(rp);
 rp[0][3] = 1.0;

 math::Identity(unRectLeft);
 math::Identity(unRectRight);

 leftDim[0] = width;
 leftDim[1] = height;
 rightDim[0] = width;
 rightDim[1] = height;
}

void CameraPair::FromFull(const CameraFull & l,const CameraFull & r)
{
 math::Mat<3,3,real64> rot;

 l.camera.Decompose(left.intrinsic,rot);
 left.radial = l.radial;
 left.dim = l.dim;

 r.camera.Decompose(right.intrinsic,rot);
 right.radial = r.radial;
 right.dim = r.dim;


 math::Vect<4,real64> leftCentre;
 l.camera.Centre(leftCentre);

 math::Vect<3,real64> rightEPL;
 math::MultVect(r.camera,leftCentre,rightEPL);

 math::Mat<3,3,real64> epiMat;
 math::SkewSymetric33(rightEPL,epiMat);

 Camera tempCam;
 math::Mult(epiMat,r.camera,tempCam);

 math::Mat<4,3,real64> invCam;
 math::PseudoInverse(l.camera,invCam);

 math::Mult(tempCam,invCam,fun);


 gap = 0.0; // I just don't see the point of working this out, esp. as it could be infinite.


 lp = l.camera;
 rp = r.camera;

 math::Identity(unRectLeft);
 leftDim = l.dim;

 math::Identity(unRectRight);
 rightDim = r.dim;
}

void CameraPair::ScaleLeft(real64 newWidth,real64 newHeight)
{
 LogBlock("CameraPair::ScaleLeft","{newWidth,newHeight}" << LogDiv() << newWidth << LogDiv() << newHeight);

 math::Mat<3,3,real64> pM;
 math::Identity(pM);
 pM[0][0] = leftDim[0]/newWidth;
 pM[1][1] = leftDim[1]/newHeight;

 LogDebug("left" << LogDiv() << pM);

 math::Mat<3,3,real64> temp;
 math::Mult(unRectLeft,pM,temp);
 unRectLeft = temp;

 leftDim[0] = newWidth;
 leftDim[1] = newHeight;
}

void CameraPair::ScaleRight(real64 newWidth,real64 newHeight)
{
 LogBlock("CameraPair::ScaleRight","{newWidth,newHeight}" << LogDiv() << newWidth << LogDiv() << newHeight);

 math::Mat<3,3,real64> pM;
 math::Identity(pM);
 pM[0][0] = rightDim[0]/newWidth;
 pM[1][1] = rightDim[1]/newHeight;

 LogDebug("right" << LogDiv() << pM);

 math::Mat<3,3,real64> temp;
 math::Mult(unRectRight,pM,temp);
 unRectRight = temp;

 rightDim[0] = newWidth;
 rightDim[1] = newHeight;
}

void CameraPair::CropLeft(real64 minX,real64 dimX,real64 minY,real64 dimY)
{
 LogBlock("CameraPair::CropLeft","{minX,dimX,minY,dimY}" << LogDiv() << minX << LogDiv() << dimX << LogDiv() << minY << LogDiv() << dimY);
 math::Mat<3,3,real64> pM;
 math::Identity(pM);
  pM[0][2] = minX;
  pM[1][2] = minY;

 math::Mat<3,3,real64> temp;
 math::Mult(unRectLeft,pM,temp);
 LogDebug("[cam.pair] unRectLeft {before,after}" << LogDiv() << unRectLeft << LogDiv() << temp);
 unRectLeft = temp;

 leftDim[0] = dimX;
 leftDim[1] = dimY;
}

void CameraPair::CropRight(real64 minX,real64 dimX,real64 minY,real64 dimY)
{
 LogBlock("CameraPair::CropLeft","{minX,dimX,minY,dimY}" << LogDiv() << minX << LogDiv() << dimX << LogDiv() << minY << LogDiv() << dimY);
 math::Mat<3,3,real64> pM;
 math::Identity(pM);
  pM[0][2] = minX;
  pM[1][2] = minY;

 math::Mat<3,3,real64> temp;
 math::Mult(unRectRight,pM,temp);
 LogDebug("[cam.pair] unRectRight {before,after}" << LogDiv() << unRectRight << LogDiv() << temp);
 unRectRight = temp;

 rightDim[0] = dimX;
 rightDim[1] = dimY;
}

void CameraPair::LeftToDefault(math::Mat<4,4,real64> * out)
{
 LogBlock("eos::cam::CameraPair::LeftToDefault","-");
 // Decompose the camera...
  Intrinsic intr;
  math::Mat<3,3,real64> rot;
  math::Vect<4,real64> trans;

  lp.Decompose(intr,rot);
  lp.Centre(trans);


 // Calculate the matrix to the camera...
  math::Mat<4,4,real64> tr1;
  math::Identity(tr1);
  math::SetSub(tr1,rot,0,0);

  math::Mat<4,4,real64> tr2;
  math::Identity(tr2);
  for (nat32 i=0;i<4;i++) tr2[i][3] = -trans[i];

  math::Mat<4,4,real64> tr;
  math::Mult(tr1,tr2,tr);


 // Invert...
 {
  math::Mat<4,4,real64> temp;
  math::Inverse(tr,temp);
 }


 // Apply it, output if needed...
  Camera temp;

  math::Mult(lp,tr,temp);
  lp = temp;

  math::Mult(rp,tr,temp);
  rp = temp;

  LogDebug("[cam.pair] Adjusted pair to default positions {tranformation,left camera,right camera}" << LogDiv()
           << tr << LogDiv() << lp << LogDiv() << rp);

  if (out) *out = tr;
}

bit CameraPair::IsRectified() const
{
 math::Mat<3,3,real64> temp33;

 // Calculate the full left projection matrix...
  math::Mat<3,3,real64> rectLeft = unRectLeft;
  math::Inverse(rectLeft,temp33);
  Camera fullLeft;
  math::Mult(rectLeft,lp,fullLeft);

 // Calculate the full right projection matrix...
  math::Mat<3,3,real64> rectRight = unRectRight;
  math::Inverse(rectRight,temp33);
  Camera fullRight;
  math::Mult(rectRight,rp,fullRight);


 // We need the right epipole, bitch...
  math::Vect<3,real64> epiRight;

  math::Vect<4,real64> leftCentre;
  fullLeft.Centre(leftCentre);
  math::MultVect(fullRight,leftCentre,epiRight);

  Fundamental ssEpiRight;
  math::SkewSymetric33(epiRight,ssEpiRight);


 // Calculate the fundamental matrix...
  math::Mat<4,3,real64> piLeft;
  math::PseudoInverse(fullLeft,piLeft);

  Fundamental tempFun;
  Fundamental fullFun;
  math::Mult(fullRight,piLeft,tempFun);
  math::Mult(ssEpiRight,tempFun,fullFun);


 // Check that is it, within numerical error, the correct matrix...
  fullFun /= fullFun[2][1];

  if (!math::IsZero(fullFun[0][0])) return false;
  if (!math::IsZero(fullFun[0][1])) return false;
  if (!math::IsZero(fullFun[0][2])) return false;
  if (!math::IsZero(fullFun[1][0])) return false;
  if (!math::IsZero(fullFun[1][1])) return false;
  if (!math::Equal(fullFun[1][2],-1.0)) return false;
  if (!math::IsZero(fullFun[2][0])) return false;
  if (!math::IsZero(fullFun[2][2])) return false;

 return true;
}

void CameraPair::GetDispToDepth(real64 & a,real64 & b) const
{
 // Project any arbitary point out of position (0,0) to infinity...
  math::Vect<4,real64> infPos;
  {
   math::Mat<3,3,real64> lpSub;
   math::SubSet(lpSub,lp,0,0);

   math::Mat<3,3,real64> temp;
   math::Inverse(lpSub,temp);
   math::Mult(lpSub,unRectLeft,temp);
   lpSub = temp;

   math::Vect<3,real64> tempV[2];
   tempV[0][0] = 0.0; tempV[0][1] = 0.0; tempV[0][2] = 1.0;
   math::MultVect(lpSub,tempV[0],tempV[1]);

   for (nat32 i=0;i<3;i++) infPos[i] = tempV[1][i];
   infPos[3] = 0.0;
  }


 // Reproject back to find the disparity that is infinity - whilst this
 // mathematically gives us b it is numerically unstable and hence not used...
  real32 ox,oy,od;
  Project(infPos,ox,oy,od);


 // Triangulate out two points either side of the instability...
  real64 disp[2];
  disp[0] = od+500.0;
  disp[1] = od+1000.0;

  math::Vect<4,real64> pos[2];
  Triangulate(0.0,0.0,disp[0],pos[0]);
  Triangulate(0.0,0.0,disp[1],pos[1]);

 // Calculate the depth of the two points...
  real64 depth[2];
  depth[0] = lp.Depth(pos[0]);
  depth[1] = lp.Depth(pos[1]);


 // Now get fiddly with the maths to work out the two relevant values in the equation...
  a = (disp[0]-disp[1]) * (depth[0]*depth[1]) / (depth[1]-depth[0]);
  b = 0.5*( ((a/depth[0]) - disp[0]) + ((a/depth[1]) - disp[1]) );


 #ifdef EOS_DEBUG
 {
  // Test code...
   for (int32 i=-23;i<=23;i+=2)
   {
    math::Vect<4,real64> fPos;
    real32 disp = real32(i) + od;
    Triangulate(0.0,0.0,disp,fPos);
    LogDebug("Depth {i,actual,by equation} " << LogDiv() << i << LogDiv() << lp.Depth(fPos) << LogDiv() << (a/(disp+b)));
   }
 }
 #endif
}

void CameraPair::Load(const bs::Element & root)
{
 left.Load(*root.GetElement("left"));
 right.Load(*root.GetElement("right"));
 fun.Load(*root.GetElement("fundamental"));

 gap = root.GrabReal(":gap.value",1.0);


 lp.Load(*root.GetElement("left-projection"));
 rp.Load(*root.GetElement("right-projection"));


 str::String def("[1,0,0;0,1,0;0,0,1]");

 {
  str::String & s = const_cast<str::String&>(root.GrabString(":un-rect-left.value",def));
  str::String::Cursor targ = s.GetCursor();
  targ >> unRectLeft;
 }

 {
  str::String & s = const_cast<str::String&>(root.GrabString(":un-rect-right.value",def));
  str::String::Cursor targ = s.GetCursor();
  targ >> unRectRight;
 }

 leftDim[0] = root.GrabReal(":resolution:left.width",320);
 leftDim[1] = root.GrabReal(":resolution:left.height",240);

 rightDim[0] = root.GrabReal(":resolution:right.width",320);
 rightDim[1] = root.GrabReal(":resolution:right.height",240);
}

bit CameraPair::Load(cstrconst fn)
{
 str::TokenTable tt;
 bs::Element * root = file::LoadXML(tt,fn);
 if (root==null<bs::Element*>()) return false;

 Load(*root);

 delete root;
 return true;
}

bit CameraPair::Load(const str::String & fn)
{
 str::TokenTable tt;
 bs::Element * root = file::LoadXML(tt,fn);
 if (root==null<bs::Element*>()) return false;

 Load(*root);

 delete root;
 return true;
}

void CameraPair::Save(bs::Element & root)
{
 bs::Element * origin = root.NewChild("origin");
  origin->SetAttribute("value","bottom-left");

 left.Save(*root.NewChild("left"));
 right.Save(*root.NewChild("right"));
 fun.Save(*root.NewChild("fundamental"));

 bs::Element * gp = root.NewChild("gap");
  gp->SetAttribute("value",gap);

 lp.Save(*root.NewChild("left-projection"));
 rp.Save(*root.NewChild("right-projection"));

 bs::Element * leftUR = root.NewChild("un-rect-left");
 {
  str::String s;
  s << unRectLeft;
  leftUR->SetAttribute("value",s);
 }

 bs::Element * rightUR = root.NewChild("un-rect-right");
 {
  str::String s;
  s << unRectRight;
  rightUR->SetAttribute("value",s);
 }

 bs::Element * dim = root.NewChild("resolution");
 bs::Element * dimLeft = dim->NewChild("left");
  dimLeft->SetAttribute("width",leftDim[0]);
  dimLeft->SetAttribute("height",leftDim[1]);
 bs::Element * dimRight = dim->NewChild("right");
  dimRight->SetAttribute("width",rightDim[0]);
  dimRight->SetAttribute("height",rightDim[1]);
}

bit CameraPair::Save(cstrconst fn,bit overwrite)
{
 // Construct the dom...
  str::TokenTable tt;
  bs::Element root(tt,"pair");
  Save(root);

 // Save the dom to the user selected filename...
  return file::SaveXML(&root,fn,overwrite);
}

bit CameraPair::Save(const str::String & fn,bit overwrite)
{
 // Construct the dom...
  str::TokenTable tt;
  bs::Element root(tt,"pair");
  Save(root);

 // Save the dom to the user selected filename...
  return file::SaveXML(&root,fn,overwrite);
}

bit CameraPair::MakeProjection(ds::Array<Pair<bs::Point,bs::Point> > * matches)
{
 LogBlock("CameraPair::MakeProjection","-");

 // First projection matrix is rather easy...
  math::Identity(lp);


 // Arrange that the intrinsic matrices precisly compute the fundamental matrix...
  Intrinsic leftI  = left.intrinsic;
  Intrinsic rightI = right.intrinsic;

  math::Mat<3,3,real64> u;
  math::Vect<3,real64> d;
  math::Mat<3,3,real64> v;  
  {
   leftI /= math::FrobNorm(leftI);
   rightI /= math::FrobNorm(rightI);
  
   // Create the extrinsic from current parameters...
    {
     math::Mat<3,3,real64> temp;
     math::TransMult(rightI,fun,temp);
     math::Mult(temp,leftI,u);
     u /= math::FrobNorm(u);
    }
  
   // Decompose...
    {
     math::Vect<3,real64> temp;
     if (SVD(u,d,v,temp)==false) return false;
    }
    
   // Calculate the inverse post-multiply correction...
    math::Mat<3,3,real64> postCor;
    {
     real32 dAvg = 0.5*(d[0]+d[1]);
     math::Vect<3,real64> cor;
     cor[0] = dAvg/d[0];
     cor[1] = dAvg/d[1];
     cor[2] = 1.0;
     
     LogDebug("[make_proj] {Correction diag,d}" << LogDiv() << cor << LogDiv() << d);
     d[0] = dAvg;
     d[1] = dAvg; // Not strictly needed, has been used for testing though.
     d[2] = 0.0;

     math::Mat<3,3,real64> temp;
     //math::DiagMult(cor,v,temp);
     //math::TransMult(v,temp,postCor);
     math::MultDiag(v,cor,temp);
     math::MultTrans(temp,v,postCor);
    }
   
   // Apply to left intrinsic matrix - it will no longer be correct, but whatever...
    math::Mat<3,3,real64> temp = leftI;
    math::Mult(temp,postCor,leftI);
    
    LogDebug("[make_proj] {Correction,Left intrinsic}" << LogDiv() << postCor << LogDiv() << leftI);
  }

  LogDebug("[make_proj] Decomposed essential matrix {u,d,v}" << LogDiv() << u << LogDiv() 
           << d << LogDiv() << v);


 // Create the 4 possible second projection matrices...
  Camera pp[4];
  {
   math::Mat<3,3,real64> w;
   math::Zero(w);
   w[0][1] = -1.0;
   w[1][0] = 1.0;
   w[2][2] = 1.0;

   math::Mat<3,3,real64> t1;
   math::Mat<3,3,real64> t2;

   real64 u3l = 0.0;
   for (nat32 i=0;i<3;i++) u3l += math::Sqr(u[i][2]);
   u3l = gap*math::InvSqrt(u3l);


   math::Mult(u,w,t1);
   math::MultTrans(t1,v,t2);

   math::SetSub(pp[0],t2,0,0);
   math::SetSub(pp[1],t2,0,0);

   for (nat32 i=0;i<3;i++) pp[0][i][3] = u[i][2]*u3l;
   for (nat32 i=0;i<3;i++) pp[1][i][3] = -u[i][2]*u3l;


   math::MultTrans(u,w,t1);
   math::MultTrans(t1,v,t2);

   math::SetSub(pp[2],t2,0,0);
   math::SetSub(pp[3],t2,0,0);

   for (nat32 i=0;i<3;i++) pp[2][i][3] = u[i][2]*u3l;
   for (nat32 i=0;i<3;i++) pp[3][i][3] = -u[i][2]*u3l;
  }


 // Apply the intrinsic calibration matrices to revert from normalised space...
 {
  Camera temp;
  temp = lp;
  math::Mult(leftI,temp,lp);
  lp /= math::FrobNorm(lp);

  LogDebug("First projection {camera}" << LogDiv() << lp);
  for (nat32 i=0;i<4;i++)
  {
   temp = pp[i];
   math::Mult(rightI,temp,pp[i]);
   pp[i] /= math::FrobNorm(pp[i]);
   
   //fun.MatchTo(lp,pp[i]); // Make sure the fundamental matrix is a perfect match.

   #ifdef EOS_DEBUG
   LogDebug("Second projection, #" << i << "{camera}" << LogDiv() << pp[i]);
   Fundamental calcFun;
   calcFun.SetCameraPair(lp,pp[i]);
   calcFun /= math::FrobNorm(calcFun);
   LogDebug("Calculated fun {fun, residual}" << LogDiv() << calcFun << LogDiv() << math::FrobNormDiffSign(calcFun,fun));
   #endif
  }
 }


 // Depending on the mode choose between either testing given matches
 // or generating our own matches and testing them...
  nat32 score[4];
  real32 decider[4];
  {
   for (nat32 i=0;i<4;i++)
   {
    score[i] = 0;
    decider[i] = 1e10;
   }

   // If no matches have been provided create out own set of imaginary matches...
    bit delMatches = false;
    if (!matches)
    {
     delMatches = true;
     // We are going to need the epipolar points below...
      math::Vect<3,real64> ea;
      math::Vect<3,real64> eb;
       fun.EpipoleA(ea);
       fun.EpipoleB(eb);

     // Select 9 lines in the second image perpendicular to the epipolar line passing
     // through the centre point, equally distributed...
      // Image A...
       math::Vect<3,real64> lpa[9];
       math::Vect<2,real64> celDir;
       {
        math::Vect<3,real64> cp;
         cp[0] = 0.5*left.dim[0]; cp[1] = 0.5*left.dim[1]; cp[2] = 1.0;

        math::Vect<3,real64> cel;
        math::CrossProduct(ea,cp,cel);
        cel.NormLine();

        celDir[0] = cel[0];
        celDir[1] = cel[1];

        lpa[4][0] = -cel[1];
        lpa[4][1] = cel[0];
        lpa[4][2] = 0.0;
        lpa[4][2] = -(lpa[4]*cp);

        real64 step = (1.0/9.0)*math::Min(left.dim[0],left.dim[1]);
        for (int32 i=0;i<9;i++)
        {
         if (i==4) continue;

         math::Vect<3,real64> mp = cp;
          mp[0] += (i-4)*step*lpa[4][0];
          mp[1] += (i-4)*step*lpa[4][1];

         lpa[i][0] = -cel[1];
         lpa[i][1] = cel[0];
         lpa[i][2] = 0.0;
         lpa[i][2] = -(lpa[i]*mp);
        }
       }

      // Image B...
       math::Vect<3,real64> lpb[9];
       {
        math::Vect<3,real64> cp;
         cp[0] = 0.5*right.dim[0]; cp[1] = 0.5*right.dim[1]; cp[2] = 1.0;

        math::Vect<3,real64> cel;
        math::CrossProduct(eb,cp,cel);
        cel.NormLine();

        lpb[4][0] = -cel[1];
        lpb[4][1] = cel[0];
        lpb[4][2] = 0.0;
        lpb[4][2] = -(lpb[4]*cp);

        real64 step = (1.0/9.0)*math::Min(right.dim[0],right.dim[1]);
        for (int32 i=0;i<9;i++)
        {
         if (i==4) continue;

         math::Vect<3,real64> mp = cp;
          mp[0] += (i-4)*step*lpb[4][0];
          mp[1] += (i-4)*step*lpb[4][1];

         lpb[i][0] = -cel[1];
         lpb[i][1] = cel[0];
         lpb[i][2] = 0.0;
         lpb[i][2] = -(lpb[i]*mp);
        }
       }

     // In image A select 9 points on the central perpendicular line,
     // find 9 epipolar lines in both images...
      math::Vect<3,real64> lea[9];
      math::Vect<3,real64> leb[9];
      {
       // Select points...
        math::Vect<3,real64> pia[9];
        pia[4][0] = 0.5*left.dim[0];
        pia[4][1] = 0.5*left.dim[1];
        pia[4][2] = 1.0;

        real64 step = (1.0/9.0)*math::Min(left.dim[0],left.dim[1]);
        for (int32 i=0;i<9;i++)
        {
         if (i==4) continue;

         pia[i][0] = pia[4][0]+(i-4)*step*celDir[0];
         pia[i][1] = pia[4][1]+(i-4)*step*celDir[1];
         pia[i][2] = 1.0;
        }

       // Generate epipolar lines...
        for (nat32 i=0;i<9;i++)
        {
         math::CrossProduct(pia[i],ea,lea[i]);
         math::MultVect(fun,pia[i],leb[i]);
        }
      }


     // Create 81 matches by intercepting each combination of 2 lines in the two views...
      matches = new ds::Array<Pair<bs::Point,bs::Point> > (81);
      for (nat32 v=0;v<9;v++)
      {
       for (nat32 u=0;u<9;u++)
       {
        math::Vect<3,real64> pa;
        math::Vect<3,real64> pb;
        math::CrossProduct(lea[v],lpa[u],pa);
        math::CrossProduct(leb[v],lpb[u],pb);
        pa /= pa[2];
        pb /= pb[2];

        Pair<bs::Point,bs::Point> & targ = (*matches)[9*v+u];
        targ.first[0] = pa[0];
        targ.first[1] = pa[1];
        targ.second[0] = pb[0];
        targ.second[1] = pb[1];
       }
      }
    }

   // Iterate the matches, count the number in front for each camera and update
   // the decider to be the closest in front point to the camera...
    for (nat32 i=0;i<matches->Size();i++)
    {
     for (nat32 j=0;j<4;j++)
     {
      math::Vect<2,real64> pa;
      math::Vect<2,real64> pb;
       pa[0] = (*matches)[i].first[0];
       pa[1] = (*matches)[i].first[1];
       pb[0] = (*matches)[i].second[0];
       pb[1] = (*matches)[i].second[1];

      math::Vect<4,real64> pos;
      if (cam::Triangulate(pa,pb,lp,pp[j],pos))
      {
       if (lp.Depth(pos)<0.0) score[j] += 1;
       if (pp[j].Depth(pos)<0.0) score[j] += 1;
      }
     }
    }

   // If we created our own match database
    if (delMatches) delete matches;
  }


 // Choose the matrix with the best score...
  nat32 bestScore = score[0];
  real32 bestDecider = decider[0];
  rp = pp[0];
  LogDebug("Score for Projection #" << nat32(0) << "{score,decider}" << LogDiv() << score[0] << LogDiv() << decider[0]);

  for (nat32 i=1;i<4;i++)
  {
   LogDebug("Score for Projection #" << i << "{score,decider}" << LogDiv() << score[i] << LogDiv() << decider[i]);
   if (score[i]>=bestScore)
   {
    if ((score[i]>bestScore)||(decider[i]>bestDecider))
    {
     bestScore = score[i];
     bestDecider = decider[i];
     rp = pp[i];
    }
   }
  }


 // Set 'em both to frob norm 1...
  lp /= math::FrobNorm(lp);
  rp /= math::FrobNorm(rp);


 // If logging is enabled produce some basic stats about the selected cameras
 // - relative rotation and offset...
  #ifdef EOS_DEBUG
   LogDebug("Final first projection matrix {camera}" << LogDiv() << lp);
   LogDebug("Final second projection matrix {camera}" << LogDiv() << rp);
   {
    math::Vect<4,real64> c;
    rp.Centre(c);
    LogDebug("[cam.make_proj] Second camera. {centre}" << LogDiv() << c);
   }
   {
    Intrinsic intr;
    math::Mat<3,3,real64> rot;
    rp.Decompose(intr,rot);
    LogDebug("[cam.make_proj] Second camera. {intrinsic,rotation}" << LogDiv() << intr << LogDiv() << rot);
   }
  #endif

 return true;
}

bit CameraPair::Triangulate(real32 x,real32 y,real32 disp,math::Vect<4,real64> & out) const
{
 LogTime("eos::cam::CameraPair::Triangulate");

 // Un-rectify...
  math::Vect<3,real64> lRect;
  lRect[0] = x;
  lRect[1] = y;
  lRect[2] = 1.0;

  math::Vect<3,real64> rRect;
  rRect[0] = x + disp;
  rRect[1] = y;
  rRect[2] = 1.0;

  math::Vect<3,real64> lProj;
  math::MultVect(unRectLeft,lRect,lProj);

  math::Vect<3,real64> rProj;
  math::MultVect(unRectRight,rRect,rProj);


 // Triangulate...
  math::Mat<4,4,real64> a;

  for (nat32 i=0;i<4;i++) a[0][i] = lProj[0]*lp[2][i] - lProj[2]*lp[0][i];
  for (nat32 i=0;i<4;i++) a[1][i] = lProj[1]*lp[2][i] - lProj[2]*lp[1][i];

  for (nat32 i=0;i<4;i++) a[2][i] = rProj[0]*rp[2][i] - rProj[2]*rp[0][i];
  for (nat32 i=0;i<4;i++) a[3][i] = rProj[1]*rp[2][i] - rProj[2]*rp[1][i];

  return math::RightNullSpace(a,out);
}

void CameraPair::Project(const math::Vect<4,real64> & pos,real32 & outX,real32 & outY,real32 & outDisp,
                         const math::Mat<3,3,real64> * rectLeft,
                         const math::Mat<3,3,real64> * rectRight) const
{
 LogTime("eos::cam::CameraPair::Project 1");

 math::Mat<3,3,real64> temp[2];

 // Left...
  math::Vect<3,real64> lProj;
  math::MultVect(lp,pos,lProj);

  math::Vect<3,real64> lRect;
  if (rectLeft==0)
  {
   temp[0] = unRectLeft;
   math::Inverse(temp[0],temp[1]);
   rectLeft = &temp[0];
  }
  math::MultVect(*rectLeft,lProj,lRect);

  lRect /= lRect[2];


 // Right...
  math::Vect<3,real64> rProj;
  math::MultVect(rp,pos,rProj);

  math::Vect<3,real64> rRect;
  if (rectRight==0)
  {
   temp[0] = unRectRight;
   math::Inverse(temp[0],temp[1]);
   rectRight = &temp[0];
  }
  math::MultVect(*rectRight,rProj,rRect);

  rRect /= rRect[2];


 // Output...
  outX = lRect[0];
  outY = lRect[1];
  outDisp = rRect[0] - lRect[0];

  //LogDebug("Project {left,right}" << LogDiv() << lRect << LogDiv() << rRect); // *************************************
}

void CameraPair::Project(const math::Vect<4,real64> & pos,
                         math::Vect<2,real64> & outLeft,math::Vect<2,real64> & outRight,
                         const math::Mat<3,3,real64> * rectLeft,const math::Mat<3,3,real64> * rectRight) const
{
 LogTime("eos::cam::CameraPair::Project 2");

 math::Mat<3,3,real64> temp[2];

 // Left...
  math::Vect<3,real64> lProj;
  math::MultVect(lp,pos,lProj);

  math::Vect<3,real64> lRect;
  if (rectLeft==0)
  {
   temp[0] = unRectLeft;
   math::Inverse(temp[0],temp[1]);
   rectLeft = &temp[0];
  }
  math::MultVect(*rectLeft,lProj,lRect);

  left.radial.Dis(lRect,lRect);
 
  lRect /= lRect[2];
  outLeft[0] = lRect[0];
  outLeft[1] = lRect[1];


 // Right...
  math::Vect<3,real64> rProj;
  math::MultVect(rp,pos,rProj);

  math::Vect<3,real64> rRect;
  if (rectRight==0)
  {
   temp[0] = unRectRight;
   math::Inverse(temp[0],temp[1]);
   rectRight = &temp[0];
  }
  math::MultVect(*rectRight,rProj,rRect);

  right.radial.Dis(rRect,rRect);

  rRect /= rRect[2];
  outRight[0] = rRect[0];
  outRight[1] = rRect[1];  
}

void CameraPair::Convert(const svt::Field<real32> & disp,svt::Field<bs::Vertex> & pos) const
{
 math::Vect<4,real64> p;
 for (nat32 y=0;y<pos.Size(1);y++)
 {
  for (nat32 x=0;x<pos.Size(0);x++)
  {
   Triangulate(x,y,disp.Get(x,y),p);
   pos.Get(x,y) = p;
  }
 }
}

void CameraPair::Convert(const svt::Field<real32> & disp,svt::Field<bs::Normal> & needle) const
{
 ds::Array2D< math::Vect<4,real64> > pos(needle.Size(0),needle.Size(1));

 // Get 3D positions...
  for (nat32 y=0;y<pos.Height();y++)
  {
   for (nat32 x=0;x<pos.Width();x++) Triangulate(x,y,disp.Get(x,y),pos.Get(x,y));
  }

 // Calculate normals...
  for (nat32 y=0;y<needle.Size(1);y++)
  {
   for (nat32 x=0;x<needle.Size(0);x++)
   {
    nat32 lowX = x; if (lowX!=0) lowX -= 1;
    nat32 highX = x+1; if (highX==needle.Size(0)) highX -= 1;
    nat32 lowY = y; if (lowY!=0) lowY -= 1;
    nat32 highY = y+1; if (highY==needle.Size(1)) highY -= 1;

    bs::Normal dx;
    bs::Normal dy;
    bs::Normal temp;

    dx.FromHomo(pos.Get(highX,y));
    temp.FromHomo(pos.Get(lowX,y));
    dx -= temp;

    dy.FromHomo(pos.Get(x,highY));
    temp.FromHomo(pos.Get(x,lowY));
    dy -= temp;

    math::CrossProduct(dx,dy,needle.Get(x,y));
    needle.Get(x,y).Normalise();
   }
  }
}

void CameraPair::Convert(const svt::Field<bs::Vertex> & pos,svt::Field<real32> & disp) const
{
 math::Mat<3,3,real64> temp;

 math::Mat<3,3,real64> rectLeft;
 rectLeft = unRectLeft;
 math::Inverse(rectLeft,temp);

 math::Mat<3,3,real64> rectRight;
 rectRight = unRectRight;
 math::Inverse(rectRight,temp);

 for (nat32 y=0;y<disp.Size(1);y++)
 {
  for (nat32 x=0;x<disp.Size(0);x++)
  {
   math::Vect<4,real64> p;
   for (nat32 i=0;i<4;i++) p[i] = pos.Get(x,y)[i];

   real32 oX,oY;
   Project(p,oX,oY,disp.Get(x,y),&rectLeft,&rectRight);
  }
 }
}

void CameraPair::Normalise()
{
 lp /= math::FrobNorm(lp);
 rp /= math::FrobNorm(rp);

 unRectLeft /= math::FrobNorm(unRectLeft);
 unRectRight /= math::FrobNorm(unRectRight);
}

//------------------------------------------------------------------------------
 };
};
