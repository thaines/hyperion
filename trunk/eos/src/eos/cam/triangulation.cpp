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

#include "eos/cam/triangulation.h"

#include "eos/svt/var.h"
#include "eos/math/eigen.h"

namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------
EOS_FUNC void RectifiedDepth(const Intrinsic & intr,
                            real32 xt,const svt::Field<real32> & disp,
                            svt::Field<real32> & depth,svt::Field<bit> & mask)
{
 real32 w = intr[0][0]*xt;

 for (nat32 y=0;y<disp.Size(1);y++)
 {
  for (nat32 x=0;x<disp.Size(0);x++)
  {
   if (math::Equal(disp.Get(x,y),real32(0.0)))
   {
    depth.Get(x,y) = 0.0;
    mask.Get(x,y) = false;
   }
   else
   {
    depth.Get(x,y) = w/disp.Get(x,y);
    mask.Get(x,y) = true;
   }
  }
 }
}

//------------------------------------------------------------------------------
EOS_FUNC void RectifiedPos(const Intrinsic & intr,const Radial * rad,
                          real32 xt,const svt::Field<real32> & disp,
                          svt::Field<bs::Vert> & pos,svt::Field<bit> & mask)
{
 real32 w = intr[0][0]*xt;

 math::Mat<3,3,real64> invIntr = intr;
 math::Mat<3,3,real64> temp;
 math::Inverse(invIntr,temp);

 for (nat32 y=0;y<disp.Size(1);y++)
 {
  for (nat32 x=0;x<disp.Size(0);x++)
  {
   if (math::Equal(disp.Get(x,y),real32(0.0)))
   {
    pos.Get(x,y) = bs::Vert(0.0,0.0,0.0);
    mask.Get(x,y) = false;
   }
   else
   {
    // Get the actual point position, sans radial distortion...
     math::Vect<3,real64> p;
     if (rad)
     {
      math::Vect<2,real64> in;
      math::Vect<2,real64> out;

      in[0] = x;
      in[1] = y;
      rad->UnDis(in,out);

      p[0] = out[0];
      p[1] = out[1];
      p[2] = 1.0;
     }
     else
     {
      p[0] = x;
      p[1] = y;
      p[2] = 1.0;
     }

    // Calculate the position...
     math::MultVect(invIntr,p,pos.Get(x,y));
     pos.Get(x,y) *= w/disp.Get(x,y);

    mask.Get(x,y) = true;
   }
  }
 }
}

//------------------------------------------------------------------------------
EOS_FUNC void RectifiedNeedle(const Intrinsic & intr,const Radial * rad,
                              real32 xt,const svt::Field<real32> & disp,
                              svt::Field<bs::Normal> & needle)
{
 // First calculate an array of 3D positions with a validity mask...
  svt::Var var(disp);
   bs::Vert posIni(0.0,0.0,0.0);
   bit maskIni = false;
   var.Add("pos",posIni);
   var.Add("mask",maskIni);
  var.Commit(false);

  svt::Field<bs::Vert> pos(&var,"pos");
  svt::Field<bit> mask(&var,"mask");

  RectifiedPos(intr,rad,xt,disp,pos,mask);


 // Now calculate normals for all central pixels...
  math::Mat<3,3,real64> invIntr = intr;
  math::Mat<3,3,real64> temp;
  math::Inverse(invIntr,temp);

  for (nat32 y=1;y<disp.Size(1)-1;y++)
  {
   for (nat32 x=1;x<disp.Size(0)-1;x++)
   {
    if ((mask.Get(x-1,y)==false)||
        (mask.Get(x+1,y)==false)||
        (mask.Get(x,y-1)==false)||
        (mask.Get(x,y+1)==false))
    {
     // Get the actual point position, sans radial distortion...
      math::Vect<3,real64> p;
      if (rad)
      {
       math::Vect<2,real64> in;
       math::Vect<2,real64> out;

       in[0] = x;
       in[1] = y;
       rad->UnDis(in,out);

       p[0] = out[0];
       p[1] = out[1];
       p[2] = 1.0;
      }
      else
      {
       p[0] = x;
       p[1] = y;
       p[2] = 1.0;
      }

     // Calculate the position...
      math::MultVect(invIntr,p,needle.Get(x,y));

     needle.Get(x,y).Normalise();
     needle.Get(x,y) *= -1.0;
    }
    else
    {
     bs::Vert a = pos.Get(x+1,y); a -= pos.Get(x-1,y);
     bs::Vert b = pos.Get(x,y+1); b -= pos.Get(x,y-1);

     math::CrossProduct(a,b,needle.Get(x,y));
     needle.Get(x,y).Normalise();
    }
   }
  }


 // Fill in the border by growing pixels outwards...
  needle.Get(0,0) = needle.Get(1,1);
  needle.Get(disp.Size(0)-1,0) = needle.Get(disp.Size(0)-2,1);
  needle.Get(0,disp.Size(1)-1) = needle.Get(1,disp.Size(1)-2);
  needle.Get(disp.Size(0)-1,disp.Size(1)-1) = needle.Get(disp.Size(0)-2,disp.Size(1)-2);

  for (nat32 x=1;x<disp.Size(0)-1;x++)
  {
   needle.Get(x,0) = needle.Get(x,1);
   needle.Get(x,disp.Size(1)-1) = needle.Get(x,disp.Size(1)-2);
  }

  for (nat32 y=1;y<disp.Size(1)-1;y++)
  {
   needle.Get(0,y) = needle.Get(1,y);
   needle.Get(disp.Size(0)-1,y) = needle.Get(disp.Size(0)-2,y);
  }
}

//------------------------------------------------------------------------------
EOS_FUNC void OrthoNeedle(const Intrinsic & intr,const Radial * rad,
                          svt::Field<bs::Normal> & needle)
{
 math::Mat<3,3,real64> invIntr = intr;
 math::Mat<3,3,real64> temp;
 math::Inverse(invIntr,temp);

 for (nat32 y=0;y<needle.Size(1);y++)
 {
  for (nat32 x=0;x<needle.Size(0);x++)
  {
   // Calculate the normalised ray direction vector, taking into account radial
   // distortion if provided...
    math::Vect<3,real64> p;
    if (rad)
    {
     math::Vect<2,real64> in;
     math::Vect<2,real64> out;

     in[0] = x;
     in[1] = y;
     rad->UnDis(in,out);

     p[0] = out[0];
     p[1] = out[1];
     p[2] = 1.0;
    }
    else
    {
     p[0] = x;
     p[1] = y;
     p[2] = 1.0;
    }

    bs::Normal dir;
    math::MultVect(invIntr,p,dir);
    dir.Normalise();


   // Calculate the rotation vector to rotate the ray direction vector to (0,0,-1)...
    math::Vect<3> aim; aim[0] = 0.0; aim[1] = 0.0; aim[2] = 1.0;
    math::Mat<3> rot;
    math::Vect<3> axis;
    math::CrossProduct(dir,aim,axis);
    axis *= math::InvCos(dir * aim);
    math::AngAxisToRotMat(axis,rot);


   // Apply rotation...
    bs::Normal temp = needle.Get(x,y);
    math::MultVect(rot,temp,needle.Get(x,y));
  }
 }
}

//------------------------------------------------------------------------------
EOS_FUNC bit CorrectMatch(math::Vect<2,real64> & pa,math::Vect<2,real64> & pb,
                          const Fundamental & fun)
{
 //LogBlock("eos::cam::CorrectMatch","{pa,pb,fun}" << LogDiv() << pa << LogDiv() << pb << LogDiv() << fun);
 // This is all taken directly from the eyeball book, page 318 in 2nd edition...
  // Create the inverse transformations...
   math::Mat<3,3,real64> ta;
   math::Mat<3,3,real64> tb;
   math::Mat<3,3,real64> temp;

   math::Identity(ta);
   math::Identity(tb);

   ta[0][2] = -pa[0]; ta[1][2] = -pa[1];
   tb[0][2] = -pb[0]; tb[1][2] = -pb[1];

   math::Inverse(ta,temp);
   math::Inverse(tb,temp);

   //LogDebug("{ta,tb}" << LogDiv() << ta << LogDiv() << tb);

  // Create a 2nd version of the fundamental matrix...
   Fundamental fun2;
   math::TransMult(tb,fun,temp);
   math::Mult(temp,ta,fun2);

   //LogDebug("{fun2}" << LogDiv() << fun2);

  // Compute both epipoles of the 2nd fun matrix, normalise them both...
   math::Vect<3,real64> ea;
   math::Vect<3,real64> eb;

   Fundamental fun3;
   fun3 = fun2;
   math::RightNullSpace(fun3,ea);
   fun3 = fun2;
   math::Transpose(fun3);
   math::RightNullSpace(fun3,eb);

   ea /= math::Sqrt(math::Sqr(ea[0]) + math::Sqr(ea[1]));
   eb /= math::Sqrt(math::Sqr(eb[0]) + math::Sqr(eb[1]));

   //LogDebug("{ea,eb}" << LogDiv() << ea << LogDiv() << eb);

  // Create a pair of rotation matrices to move the epipoles to just where
  // we want them...
   math::Mat<3,3,real64> ra;
   math::Mat<3,3,real64> rb;

   math::Identity(ra);
   math::Identity(rb);

   ra[0][0] =  ea[0]; ra[0][1] = ea[1];
   ra[1][0] = -ea[1]; ra[1][1] = ea[0];
   rb[0][0] =  eb[0]; rb[0][1] = eb[1];
   rb[1][0] = -eb[1]; rb[1][1] = eb[0];

   //LogDebug("{ra,rb}" << LogDiv() << ra << LogDiv() << rb);

  // Create a 3rd fundamental matrix using the rotation matrices...
   math::Mult(rb,fun2,temp);
   math::MultTrans(temp,ra,fun3);

   //LogDebug("{fun3}" << LogDiv() << fun3);

  // Form the polynomial and solve. (This is the important bit)
  // If it fails return without ever editting the inputs - we can't do jack...
   real64 a = fun3[1][1];
   real64 a2 = a*a;
   real64 a3 = a2*a;
   real64 a4 = a2*a2;
   real64 b = fun3[1][2];
   real64 b2 = b*b;
   real64 b3 = b2*b;
   real64 b4 = b2*b2;
   real64 c = fun3[2][1];
   real64 c2 = c*c;
   real64 c3 = c2*c;
   real64 c4 = c2*c2;
   real64 d = fun3[2][2];
   real64 d2 = d*d;
   real64 d3 = d2*d;
   real64 d4 = d2*d2;
   real64 f = ea[2];
   real64 f2 = f*f;
   real64 f4 = f2*f2;
   real64 g = eb[2];
   real64 g2 = g*g;
   real64 g4 = g2*g2;

   //LogDebug("{a,b,c,d,f,g}" << LogDiv() << a << LogDiv() << b << LogDiv() << c << LogDiv() << d << LogDiv() << f << LogDiv() << g);

   math::Vect<7,real64> poly;
    poly[0] = b2*c*d - a*b*d2;
    poly[1] = 2.0*b2*d2*g2 - a2*d2 + b2*c2 + d4*g4 + b4;
    poly[2] = -a2*c*d + 4.0*b2*c*d*g2 + 4.0*a*b*d2*g2 + 2.0*b2*c*d*f2 - 2.0*a*b*d2*f2 + a*b*c2 + 4.0*a*b3 + 4.0*c*d3*g4;
    poly[3] = 8.0*a*b*c*d*g2 + 2.0*a2*d2*g2 + 2.0*b2*c2*g2 - 2.0*a2*d2*f2 + 2.0*b2*c2*f2 + 6.0*a2*b2 + 6.0*c2*d2*g4;
    poly[4] = 4.0*a3*b + 4.0*a2*c*d*g2 + 4.0*a*b*c2*g2 - 2.0*a2*c*d*f2 + 2.0*a*b*c2*f2 + 4.0*c3*d*g4 + b2*c*d*f4 - a*b*d2*f4;
    poly[5] = 2.0*a2*c2*g2 + c4*g4 - a2*d2*f4 + b2*c2*f4 + a4;
    poly[6] = a*b*c2*f4 - a2*c*d*f4;


   math::Vect<6,math::Complex<real64> > root;
   math::Mat<6,6,real64> polyTemp;
   nat32 rCount = math::RobustPolyRoot(poly,root,polyTemp);
   if (rCount==0) return false;

  // Check all roots, find which one produces the minimum value.
  // Also check the infinite case...
   nat32 bestInd = 6; // 0..5 indicates roots, 6 indicates infinity.
   real64 bestEval = 1.0/f2 + (c*c)/(a2+g2*c2);
   for (nat32 i=0;i<rCount;i++)
   {
    real64 eval  = math::Sqr(root[i].x)/(1.0+math::Sqr(f)*math::Sqr(root[i].x));
    //LogDebug(i << " eval a = " << eval);
    eval += math::Sqr(c*root[i].x + d)/(math::Sqr(a*root[i].x + b) + math::Sqr(g)*math::Sqr(c*root[i].x + d));
    //LogDebug(i << " eval b = " << eval);
    if (eval<bestEval)
    {
     bestEval = eval;
     bestInd = i;
    }
   }

   //LogDebug("{bestEval,bestInd}" << LogDiv() << bestEval << LogDiv() << bestInd);

  // Now find the closest point on the lines for each point,
  // special case if we have selected infinity as the best score...
   math::Vect<3,real64> xa;
   math::Vect<3,real64> xb;
   if (bestInd==6)
   {
    // Infinite case - I don't know what to do about this as the book is unclear,
    // so treating it as an error...
     return false;
   }
   else
   {
    // Finite case...
     math::Vect<3,real64> la;
     math::Vect<3,real64> lb;

     la[0] = root[bestInd].x*f;
     la[1] = 1.0;
     la[2] = -root[bestInd].x;

     lb[0] = -g*(c*root[bestInd].x + d);
     lb[1] = a*root[bestInd].x + b;
     lb[2] = c*root[bestInd].x + d;

     xa[0] = -la[0]*la[2];
     xa[1] = -la[1]*la[2];
     xa[2] = math::Sqr(la[0]) + math::Sqr(la[1]);

     xb[0] = -lb[0]*lb[2];
     xb[1] = -lb[1]*lb[2];
     xb[2] = math::Sqr(lb[0]) + math::Sqr(lb[1]);
   }

  // Transform the pair of points and save them over the input variables...
   /*math::Vect<3,real64> paX;
   paX[0] = pa[0]; paX[1] = pa[1]; paX[2] = 1.0;
   math::Vect<3,real64> pbX;
   pbX[0] = pb[0]; pbX[1] = pb[1]; pbX[2] = 1.0;*/

   math::Vect<3,real64> hgA;
   math::MultTrans(ta,ra,temp);
   math::MultVect(temp,xa,hgA);
   hgA /= hgA[2];
   pa[0] = hgA[0]; pa[1] = hgA[1];

   math::Vect<3,real64> hgB;
   math::MultTrans(tb,rb,temp);
   math::MultVect(temp,xb,hgB);
   hgB /= hgB[2];
   pb[0] = hgB[0]; pb[1] = hgB[1];

   //LogDebug("{pa,pb}" << LogDiv() << pa << LogDiv() << pb);


  // Testing...
   //LogDebug("CorrectMatch Input residual" << LogDiv() << math::VectMultVect(pbX,fun,paX));
   //LogDebug("CorrectMatch Output residual" << LogDiv() << math::VectMultVect(hgB,fun,hgA));

 return true;
}


//------------------------------------------------------------------------------
EOS_FUNC bit Triangulate(const math::Vect<2,real64> & pa,const math::Vect<2,real64> & pb,
                          const Camera & ca,const Camera & cb,
                          math::Vect<4,real64> & out)
{
 // Construct matrix...
  math::Mat<4,4,real64> a;

  for (nat32 i=0;i<4;i++) a[0][i] = pa[0]*ca[2][i] - ca[0][i];
  for (nat32 i=0;i<4;i++) a[1][i] = pa[1]*ca[2][i] - ca[1][i];

  for (nat32 i=0;i<4;i++) a[2][i] = pb[0]*cb[2][i] - cb[0][i];
  for (nat32 i=0;i<4;i++) a[3][i] = pb[1]*cb[2][i] - cb[1][i];


 // Normalise matrix...
  a /= math::FrobNorm(a);


 // Calculate null space, the answer...
  if (math::RightNullSpace(a,out)==false) return false;

 return true;
}

EOS_FUNC bit Triangulate(const math::Vect<3,real64> & pa,const math::Vect<3,real64> & pb,
                          const Camera & ca,const Camera & cb,
                          math::Vect<4,real64> & out)
{
 // Construct matrix...
  math::Mat<4,4,real64> a;

  for (nat32 i=0;i<4;i++) a[0][i] = pa[0]*ca[2][i] - pa[2]*ca[0][i];
  for (nat32 i=0;i<4;i++) a[1][i] = pa[1]*ca[2][i] - pa[2]*ca[1][i];

  for (nat32 i=0;i<4;i++) a[2][i] = pb[0]*cb[2][i] - pb[2]*cb[0][i];
  for (nat32 i=0;i<4;i++) a[3][i] = pb[1]*cb[2][i] - pb[2]*cb[1][i];


 // Normalise matrix rows...
  for (nat32 r=0;r<4;r++)
  {
   real32 len = 0.0;
   for (nat32 c=0;c<4;c++) len += math::Sqr(a[r][c]);
   len = math::InvSqrt(len);

   for (nat32 c=0;c<4;c++) a[r][c] *= len;
  }


 // Calculate null space, the answer...
  if (math::RightNullSpace(a,out)==false) return false;


 return true;
}

//------------------------------------------------------------------------------
EOS_FUNC bit Depth(const math::Vect<2,real64> & pa,const math::Vect<2,real64> & pb,
                   const Camera & ca,const Camera & cb,
                   real64 & out)
{
 math::Vect<4,real64> p;
 if (Triangulate(pa,pb,ca,cb,p)==false) return false;

 math::Vect<3,real64> pw;
 math::MultVect(ca,p,pw);

 math::Mat<3,3,real64> m;
 math::SubSet(m,ca,0,0);

 out  = math::Sign(math::Determinant(m))*pw[2];
 out /= p[3]*math::Length3(m[2][0],m[2][1],m[2][2]);

 return true;
}

//------------------------------------------------------------------------------
EOS_FUNC bit DepthToPos(const math::Vect<2,real64> & p,const Camera & c,
                        real64 depth,math::Vect<4,real64> & out)
{
 // Calculate the cameras centre...
  math::Vect<4,real64> centre;
  c.Centre(centre);

 // Use the psuedo-inverse to obtain a point on the ray...
  math::Mat<4,3,real64> invC;
  if (math::PseudoInverse(c,invC)==false) return false;
  math::Vect<3,real64> ph; ph[0] = p[0]; ph[1] = p[1]; ph[2] = 1.0;
  math::MultVect(invC,ph,out);

 // Subtract the centre, adjust for scaling and depth to get an offset from the
 // centre for this point, add in centre and then return...
  out -= centre;

  depth *= math::Length3(c[0][2],c[1][2],c[2][2]);
  depth /= out[0]*c[0][2] + out[1]*c[1][2] + out[2]*c[2][2];

  out *= depth;
  out += centre;

 return true;
}

//------------------------------------------------------------------------------
 };
};
