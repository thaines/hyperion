//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/cam/cameras.h"

#include "eos/str/tokens.h"
#include "eos/file/xml.h"
#include "eos/data/randoms.h"
#include "eos/alg/fitting.h"

namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------
void Intrinsic::Offset(real64 x,real64 y)
{
 Intrinsic temp[2];
 math::Identity(temp[0]);
 temp[0][0][2] = x;
 temp[0][1][2] = y;
 math::Mult(temp[0],*this,temp[1]);
 *this = temp[1];
}

void Intrinsic::Scale(real64 xs,real64 ys)
{
 Intrinsic temp[2];
 math::Identity(temp[0]);
 temp[0][0][0] = xs;
 temp[0][1][1] = ys;
 math::Mult(temp[0],*this,temp[1]);
 *this = temp[1];
}

void Intrinsic::Load(const bs::Element & root)
{
 FocalX() = root.GrabReal(":focal.x",1.0);
 FocalY() = root.GrabReal(":focal.y",1.0);
 PrincipalX() = root.GrabReal(":principal.x",160.0);
 PrincipalY() = root.GrabReal(":principal.y",120.0);
 Skew() = root.GrabReal(":skew.value",0.0);
}

bit Intrinsic::Load(cstrconst fn)
{
 str::TokenTable tt;
 bs::Element * root = file::LoadXML(tt,fn);
 if (root==null<bs::Element*>()) return false;

 Load(*root);

 return true;
}

bit Intrinsic::Load(const str::String & fn)
{
 str::TokenTable tt;
 bs::Element * root = file::LoadXML(tt,fn);
 if (root==null<bs::Element*>()) return false;

 Load(*root);

 return true;
}

void Intrinsic::Save(bs::Element & root)
{
 bs::Element * focal = root.NewChild("focal");
  focal->SetAttribute("x",FocalX());
  focal->SetAttribute("y",FocalY());
 bs::Element * principal = root.NewChild("principal");
  principal->SetAttribute("x",PrincipalX());
  principal->SetAttribute("y",PrincipalY());
 bs::Element * skew = root.NewChild("skew");
  skew->SetAttribute("value",Skew());
}

bit Intrinsic::Save(cstrconst fn,bit overwrite)
{
 // Construct the dom...
  str::TokenTable tt;
  bs::Element root(tt,"intrinsic");

  Save(root);

 // Save the dom to the user selected filename...
  return file::SaveXML(&root,fn,overwrite);
}

bit Intrinsic::Save(const str::String & fn,bit overwrite)
{
 // Construct the dom...
  str::TokenTable tt;
  bs::Element root(tt,"intrinsic");

  Save(root);

 // Save the dom to the user selected filename...
  return file::SaveXML(&root,fn,overwrite);
}

//------------------------------------------------------------------------------
void Radial::ChangeSize(const bs::Pnt & in,const bs::Pnt & out)
{
 // Adjust centre and aspect ratio...
  real64 inAspectRatio = real64(in[1])/real64(in[0]);
  real64 outAspectRatio = real64(out[1])/real64(out[0]);
  aspectRatio = outAspectRatio*aspectRatio/inAspectRatio;

  centre[0] = centre[0]*(real64(out[0])/real64(in[0]));
  centre[1] = centre[1]*(real64(out[1])/real64(in[1]));

 // Calculate the inverse scale change that distance from centre will undergo...
  real64 ds = real64(in[1])/real64(out[1]);

 // Adjust the k's accordingly...
  k[0] *= ds;
  k[1] *= ds*ds;
  k[2] *= ds*ds*ds;
  k[3] *= ds*ds*ds*ds;
}

void Radial::Offset(real64 x,real64 y)
{
 centre[0] += x;
 centre[1] += y;
}

void Radial::Scale(real64 xs,real64 ys)
{
 // Adjust centre and aspect ratio...
  aspectRatio *= ys/xs;

  centre[0] = centre[0]*xs;
  centre[1] = centre[1]*ys;

 // Calculate the inverse scale change that distance from centre will undergo...
  real64 ds = 1.0/ys;

 // Adjust the k's accordingly...
  k[0] *= ds;
  k[1] *= ds*ds;
  k[2] *= ds*ds*ds;
  k[3] *= ds*ds*ds*ds;
}

void Radial::Load(const bs::Element & root)
{
 aspectRatio = root.GrabReal(":aspect_ratio.value",0.75);
 centre[0] = root.GrabReal(":centre.x",160.0);
 centre[1] = root.GrabReal(":centre.y",120.0);
 k[0] = root.GrabReal(":factors.k",0.0);
 k[1] = root.GrabReal(":factors.kk",0.0);
 k[2] = root.GrabReal(":factors.kkk",0.0);
 k[3] = root.GrabReal(":factors.kkkk",0.0);
}

bit Radial::Load(cstrconst fn)
{
 str::TokenTable tt;
 bs::Element * root = file::LoadXML(tt,fn);
 if (root==null<bs::Element*>()) return false;

 Load(*root);

 return true;
}

bit Radial::Load(const str::String & fn)
{
 str::TokenTable tt;
 bs::Element * root = file::LoadXML(tt,fn);
 if (root==null<bs::Element*>()) return false;

 Load(*root);

 return true;
}

void Radial::Save(bs::Element & root)
{
 bs::Element * aspect_ratio = root.NewChild("aspect_ratio");
  aspect_ratio->SetAttribute("value",aspectRatio);
 bs::Element * cen = root.NewChild("centre");
  cen->SetAttribute("x",centre[0]);
  cen->SetAttribute("y",centre[1]);
 bs::Element * factors = root.NewChild("factors");
  factors->SetAttribute("k",k[0]);
  factors->SetAttribute("kk",k[1]);
  factors->SetAttribute("kkk",k[2]);
  factors->SetAttribute("kkkk",k[3]);
}

bit Radial::Save(cstrconst fn,bit overwrite)
{
 // Construct the dom...
  str::TokenTable tt;
  bs::Element root(tt,"radial");

  Save(root);

 // Save the dom to the user selected filename...
  return file::SaveXML(&root,fn,overwrite);
}

bit Radial::Save(const str::String & fn,bit overwrite)
{
 // Construct the dom...
  str::TokenTable tt;
  bs::Element root(tt,"radial");

  Save(root);

 // Save the dom to the user selected filename...
  return file::SaveXML(&root,fn,overwrite);
}

//------------------------------------------------------------------------------
EOS_FUNC real32 FocalLength35mmHoriz(real64 width,real64 height,
                                     const Intrinsic & intrinsic,
                                     const Radial * radial)
{
 // Create the two points we are going to calculate for...
  math::Vect<2,real64> a;
  a[0] = 0.0;
  a[1] = real64(height/2);

  math::Vect<2,real64> b;
  b[0] = real64(width-1);
  b[1] = real64(height/2);


 // If radial parameters supplied factor them in...
  if (radial)
  {
   radial->UnDis(a,a);
   radial->UnDis(b,b);
  }


 // Use the intrinsic matrix to calculate the angle between the two points...
  Intrinsic invInt = intrinsic;
  Intrinsic temp;
  math::Inverse(invInt,temp);

  math::Vect<3,real64> ax;
  ax[0] = a[0]; ax[1] = a[1]; ax[2] = 1.0;
  math::Vect<3,real64> ad;
  math::MultVect(invInt,ax,ad);

  math::Vect<3,real64> bx;
  bx[0] = b[0]; bx[1] = b[1]; bx[2] = 1.0;
  math::Vect<3,real64> bd;
  math::MultVect(invInt,bx,bd);

  ad.Normalise();
  bd.Normalise();
  real64 angle = math::InvCos(ad*bd);


 // Use the angle to calculate and return the 35mm equivalent focal length...
  return (0.5*36.0)/math::Tan(0.5*angle);
}

EOS_FUNC real32 FocalLength35mmVert(real64 width,real64 height,
                                    const Intrinsic & intrinsic,
                                    const Radial * radial)
{
 // Create the two points we are going to calculate for...
  math::Vect<2,real64> a;
  a[0] = real64(width/2);
  a[1] = 0.0;

  math::Vect<2,real64> b;
  b[0] = real64(width/2);
  b[1] = real64(height-1);


 // If radial parameters supplied factor them in...
  if (radial)
  {
   radial->UnDis(a,a);
   radial->UnDis(b,b);
  }


 // Use the intrinsic matrix to calculate the angle between the two points...
  Intrinsic invInt = intrinsic;
  Intrinsic temp;
  math::Inverse(invInt,temp);

  math::Vect<3,real64> ax;
  ax[0] = a[0]; ax[1] = a[1]; ax[2] = 1.0;
  math::Vect<3,real64> ad;
  math::MultVect(invInt,ax,ad);

  math::Vect<3,real64> bx;
  bx[0] = b[0]; bx[1] = b[1]; bx[2] = 1.0;
  math::Vect<3,real64> bd;
  math::MultVect(invInt,bx,bd);

  ad.Normalise();
  bd.Normalise();
  real64 angle = math::InvCos(ad*bd);


 // Use the angle to calculate and return the 35mm equivalent focal length...
  return (0.5*24.0)/math::Tan(0.5*angle);
}

EOS_FUNC real32 FocalLength35mmDiag(real64 width,real64 height,
                                    const Intrinsic & intrinsic,
                                    const Radial * radial)
{
 // Create the two points we are going to calculate for...
  math::Vect<2,real64> a;
  a[0] = 0.0;
  a[1] = 0.0;

  math::Vect<2,real64> b;
  b[0] = real64(width-1);
  b[1] = real64(height-1);


 // If radial parameters supplied factor them in...
  if (radial)
  {
   radial->UnDis(a,a);
   radial->UnDis(b,b);
  }


 // Use the intrinsic matrix to calculate the angle between the two points...
  Intrinsic invInt = intrinsic;
  Intrinsic temp;
  math::Inverse(invInt,temp);

  math::Vect<3,real64> ax;
  ax[0] = a[0]; ax[1] = a[1]; ax[2] = 1.0;
  math::Vect<3,real64> ad;
  math::MultVect(invInt,ax,ad);

  math::Vect<3,real64> bx;
  bx[0] = b[0]; bx[1] = b[1]; bx[2] = 1.0;
  math::Vect<3,real64> bd;
  math::MultVect(invInt,bx,bd);

  ad.Normalise();
  bd.Normalise();
  real64 angle = math::InvCos(ad*bd);


 // Use the angle to calculate and return the 35mm equivalent focal length...
  return (0.5*math::Sqrt(math::Sqr(36.0)+math::Sqr(24.0)))/math::Tan(0.5*angle);
}

//------------------------------------------------------------------------------
void Camera::Load(const bs::Element & root)
{
 str::String def("[1,0,0,0;0,1,0,0;0,0,1,0]");
 str::String & s = const_cast<str::String&>(root.GetString("value",def));
 str::String::Cursor targ = s.GetCursor();
 targ >> *this;
}

bit Camera::Load(cstrconst fn)
{
 str::TokenTable tt;
 bs::Element * root = file::LoadXML(tt,fn);
 if (root==null<bs::Element*>()) return false;

 Load(*root);

 return true;
}

bit Camera::Load(const str::String & fn)
{
 str::TokenTable tt;
 bs::Element * root = file::LoadXML(tt,fn);
 if (root==null<bs::Element*>()) return false;

 Load(*root);

 return true;
}

void Camera::Save(bs::Element & root)
{
 str::String s; s << *this;
 root.SetAttribute("value",s);
}

bit Camera::Save(cstrconst fn,bit overwrite)
{
 // Construct the dom...
  str::TokenTable tt;
  bs::Element root(tt,"camera");

  Save(root);

 // Save the dom to the user selected filename...
  return file::SaveXML(&root,fn,overwrite);
}

bit Camera::Save(const str::String & fn,bit overwrite)
{
 // Construct the dom...
  str::TokenTable tt;
  bs::Element root(tt,"camera");

  Save(root);

 // Save the dom to the user selected filename...
  return file::SaveXML(&root,fn,overwrite);
}

//------------------------------------------------------------------------------
void Fundamental::EpipoleA(math::Vect<3,real64> & ea) const
{
 Fundamental temp = *this;
 math::RightNullSpace(temp,ea);
 ea.Normalise();
}

void Fundamental::EpipoleB(math::Vect<3,real64> & eb) const
{
 Fundamental temp = *this;
 math::Transpose(temp);
 math::RightNullSpace(temp,eb);
 eb.Normalise();
}

void Fundamental::Decompose(math::Vect<3,real64> & eb,math::Mat<3,3,real64> & m) const
{
 LogBlock("void Fundamental::Decompose(eb,m) const","{*this}" << LogDiv() << *this);

 // eb is by definition the epipole - thats easy. We also temporarilly need epipole a...
  EpipoleB(eb);


 // Simply generate random vectors for use in the [eb]_X F + eb v^T formular,
 // test each one for being numerically stable and return the moment one passes the test...
  math::Mat<3,3,real64> ebss;
  math::SkewSymetric33(eb,ebss);

  data::Random rand;
  real64 range = 1.0;
  while (true)
  {
   // Create random vector...
    math::Vect<3,real64> vt;
    for (nat32 i=0;i<3;i++) vt[i] = rand.Real(-range,range);
    if (range<10000.0) range *= 1.5;

   // Create m using random vector...
    math::Mult(ebss,*this,m);
    for (nat32 r=0;r<3;r++)
    {
     for (nat32 c=0;c<3;c++)
     {
      m[r][c] += eb[r]*vt[c];
     }
    }
    m *= 1.0/math::FrobNorm(m);

   // Test it - SVD it then check the diagonal is all non-zero and large enough...
    math::Mat<3,3,real64> u;
    math::Vect<3,real64> d;
    math::Mat<3,3,real64> v;
    math::Vect<3,real64> t;

    u = m;
    math::SVD(u,d,v,t);

    if (!math::IsZero(d[0]))
    {
     if ((d[2]/d[0])>0.01) break;
    }
  }
}

void Fundamental::GetCameraPair(Camera & ca,Camera & cb) const
{
 math::Identity(ca);

 math::Vect<3,real64> eb;
 EpipoleB(eb);

 math::Mat<3,3,real64> t;
 math::SkewSymetric33(eb,t);
 t *= -1.0;
 math::SetSub(cb,t,0,0);

 cb[0][3] = eb[0];
 cb[1][3] = eb[1];
 cb[2][3] = eb[2];
}

void Fundamental::SetCameraPair(const Camera & left,const Camera & right)
{
 math::Mat<4,3,real64> leftInv;
 left.GetInverse(leftInv);

 math::Mat<3,3,real64> hg;
 math::Mult(right,leftInv,hg);

 math::Vect<4,real64> leftCentre;
 left.Centre(leftCentre);

 math::Vect<3,real64> epiRight;
 math::MultVect(right,leftCentre,epiRight);

 math::Mat<3,3,real64> epiRightSS;
 math::SkewSymetric33(epiRight,epiRightSS);

 math::Mult(epiRightSS,hg,*this);
}

void Fundamental::MatchTo(Intrinsic & left,Intrinsic & right) const
{
 LogDebug("[intrinsic fun match] Inputs {fun,left,right}" << LogDiv() << *this << LogDiv() << left << LogDiv() << right);


 // Calculate the essential matrix, assuming all values match up and are good...
  math::Mat<3,3,real64> essential;
  {
   math::Mat<3,3,real64> temp;
   math::TransMult(right,*this,temp);
   math::Mult(temp,left,essential);
   essential /= math::FrobNorm(essential);
   LogDebug("[intrinsic fun match] Essential matrix" << LogDiv() << essential);
  }


 // Decompose it with SVD...
  math::Mat<3,3,real64> u = essential;
  math::Vect<3,real64> d;
  math::Mat<3,3,real64> v;
  {
   math::Vect<3,real64> temp;
   if (math::SVD(u,d,v,temp)==false) return;
   LogDebug("[intrinsic fun match] Decomposed essential matrix {u,d,v}" << LogDiv() << u << LogDiv() << d << LogDiv() << v);
  }


 // Calculate the diagonal matrix to correct the diagonal - both pre- and post- multiplied...
 math::Mat<3,3,real64> correction;
 {
  math::Identity(correction);

  real32 target = 0.5*(d[0] + d[1]);
  correction[0][0] = math::Sqrt(target/d[0]);
  correction[1][1] = math::Sqrt(target/d[1]);
  LogDebug("[intrinsic fun match] Correction" << LogDiv() << correction);
 }


 // Push the two correction matrices out of the essential matrix...
  // Left...
   math::Mat<3,3,real64> corL;
   {
    math::Mat<3,3,real64> temp;
    math::Mat<3,3,real64> uInv = u;
    math::Inverse(uInv,temp);

    math::Mult(u,correction,temp);
    math::Mult(temp,uInv,corL);
   }

  // Right...
   math::Mat<3,3,real64> corR;
   {
    math::Mat<3,3,real64> temp;
    math::Mat<3,3,real64> vTraInv = v;
    math::Transpose(vTraInv);
    math::Inverse(vTraInv,temp);

    math::Mult(vTraInv,correction,temp);
    math::MultTrans(temp,v,corR);
   }

   LogDebug("[intrinsic fun match] Moved corrections {left,right}" << LogDiv() << corL << LogDiv() << corR);


 // Multiply the intrinsic matrices with the correction matrices...
  // Left (Right intrinsic)...
  {
   Intrinsic temp;
   math::Transpose(right);
   math::Inverse(right,temp);
   math::Mult(right,corL,temp);
   math::Assign(right,temp);
   math::Inverse(right,temp);
   math::Transpose(right);
  }

  // Right (Left intrinsic)...
  {
   Intrinsic temp;
   math::Inverse(left,temp);
   math::Mult(corR,left,temp);
   math::Assign(left,temp);
   math::Inverse(left,temp);
  }

  LogDebug("[intrinsic fun match] Output {left,right}" << LogDiv() << left << LogDiv() << right);
}

void Fundamental::MatchTo(Camera & left,Camera & right) const
{
 LogBlock("eos::cam::Fundamental::MatchTo(Camera)","");
 LogDebug("[camera match to fun] Input {left,right}" << LogDiv() << left << LogDiv() << right);
 
 // Calculate the homography that takes the left camera matrix to the identity...
  math::Mat<4,4,real64> leftExt;
  math::Identity(leftExt);
  math::SetSub(leftExt,left,0,0);
  
  math::Mat<4,4,real64> homo = leftExt;
  {
   math::Mat<4,4,real64> temp;
   math::Inverse(homo,temp);
  }


 // Apply the homography to the cameras...
 {
  Camera temp;
  math::Mult(left,homo,temp); left = temp;
  math::Mult(right,homo,temp); right = temp;
  LogDebug("[camera match to fun] Transformed {left,right,homo}" << LogDiv() << left << LogDiv() << right << LogDiv() << homo);
 }


 // Calculate the right epipole...
  math::Vect<3,real64> epiRight;
  { 
   Fundamental funTra = *this;
   math::Transpose(funTra);
   math::RightNullSpace(funTra,epiRight);
   LogDebug("[camera match to fun] Right epipole {epi}" << LogDiv() << epiRight);
  }
  
  math::Mat<3,3,real64> epiRightSS;
  math::SkewSymetric33(epiRight,epiRightSS);


 // Find the offset multiplier...
  real64 lambda;
  {
   math::Vect<3,real64> temp;
   for (nat32 i=0;i<3;i++) temp[i] = right[i][3];
   lambda = alg::VectorScaleDiff(temp,epiRight);
   LogDebug("[camera match to fun] {lambda}" << LogDiv() << lambda);
  }


 // Find the rotation vector...
  math::Vect<3,real64> v;
  {
   math::Mat<3,3,real64> rot;
   math::Mult(epiRightSS,*this,rot);
   
   for (nat32 c=0;c<3;c++)
   {
    math::Vect<3,real64> temp;
    for (nat32 i=0;i<3;i++) temp[i] = right[i][c] - rot[i][c];
    v[c] = alg::VectorScaleDiff(temp,epiRight);
   }
  }
  LogDebug("[camera match to fun] {v}" << LogDiv() << v);


 // Reconstruct the right matrix with the found parameters such that its a perfect match to this...
 {
  math::Mat<3,3,real64> rot;
  math::Mult(epiRightSS,*this,rot);
  for (nat32 r=0;r<3;r++)
  {
   for (nat32 c=0;c<3;c++) rot[r][c] += epiRight[r] * v[c];
  }
  math::SetSub(right,rot,0,0);
   
  for (nat32 i=0;i<3;i++) right[i][3] = lambda * epiRight[i];
 }
 LogDebug("[camera match to fun] Fixed {right}" << LogDiv() << right);


 // Remove the homography derived from the left matrix...
 {
  Camera temp;
  math::Mult(left,leftExt,temp); left = temp;
  math::Mult(right,leftExt,temp); right = temp;
 }


 LogDebug("[camera match to fun] Output {left,right}" << LogDiv() << left << LogDiv() << right);
}

void Fundamental::ChangeSize(const bs::Pnt & inA,const bs::Pnt & inB,
                             const bs::Pnt & outA,const bs::Pnt & outB)
{
 // Create matrices converting from out coordinates to in coordinates...
  math::Mat<3,3,real64> a; math::Identity(a);
  math::Mat<3,3,real64> b; math::Identity(b);

  a[0][0] = inA[0]/outA[0];
  a[1][1] = inA[1]/outA[1];

  b[0][0] = inB[0]/outB[0];
  b[1][1] = inB[1]/outB[1];

 // Apply to fundamental matrix...
  math::Mat<3,3,real64> temp;
  math::TransMult(b,*this,temp);
  math::Mult(temp,a,*this);

 // Reset the frobenius norm to 1 again...
  *this *= 1.0/math::FrobNorm(*this);
}

void Fundamental::Load(const bs::Element & root)
{
 str::String def("[0,0,0;0,0,-1;0,1,0]");
 str::String & s = const_cast<str::String&>(root.GetString("value",def));
 str::String::Cursor targ = s.GetCursor();
 targ >> *this;
}

bit Fundamental::Load(cstrconst fn)
{
 str::TokenTable tt;
 bs::Element * root = file::LoadXML(tt,fn);
 if (root==null<bs::Element*>()) return false;

 Load(*root);

 return true;
}

bit Fundamental::Load(const str::String & fn)
{
 str::TokenTable tt;
 bs::Element * root = file::LoadXML(tt,fn);
 if (root==null<bs::Element*>()) return false;

 Load(*root);

 return true;
}

void Fundamental::Save(bs::Element & root)
{
 str::String s; s << *this;
 root.SetAttribute("value",s);
}

bit Fundamental::Save(cstrconst fn,bit overwrite)
{
 // Construct the dom...
  str::TokenTable tt;
  bs::Element root(tt,"fundamental");

  Save(root);

 // Save the dom to the user selected filename...
  return file::SaveXML(&root,fn,overwrite);
}

bit Fundamental::Save(const str::String & fn,bit overwrite)
{
 // Construct the dom...
  str::TokenTable tt;
  bs::Element root(tt,"fundamental");

  Save(root);

 // Save the dom to the user selected filename...
  return file::SaveXML(&root,fn,overwrite);
}

//------------------------------------------------------------------------------
 };
};
