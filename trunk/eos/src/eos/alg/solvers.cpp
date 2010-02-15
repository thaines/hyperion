//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/alg/solvers.h"

#include "eos/file/csv.h"
#include "eos/math/eigen.h"
#include "eos/math/complex.h"

namespace eos
{
 namespace alg
 {
//------------------------------------------------------------------------------
EOS_FUNC void MaximiseFixedLenDualQuad(real32 a,real32 b,real32 c,real32 d,real32 n,real32 & outX,real32 & outY)
{
 LogTime("eos::alg::MaximiseFixedLenDualQuad");

 // Equate the polynomial of the derivative (re-arranged and bastardised slightly.) to zero, solve for...
  math::Vect<5,real32> poly;
  real32 cmd = c-d;
  poly[0] = n*math::Sqr(a);
  poly[1] = 2.0*n*a*cmd;
  poly[2] = 4.0*n*math::Sqr(cmd) - math::Sqr(a) - math::Sqr(b);
  poly[3] = -2.0*a*cmd;
  poly[4] = -4.0*math::Sqr(cmd);

  math::Vect<4,math::Complex<real32> > root;
  math::Mat<4,4,real32> polyTemp;
  nat32 rCount = math::RobustPolyRoot(poly,root,polyTemp);


 // First try the extrema of x - 0.0 and sqrt(n)...
  real32 sqrtN = math::Sqrt(n);
  
  outX = 0.0;
  outY = math::SignNZ(b) * sqrtN;
  real32 bestScore = a*outX + b*outY + c*math::Sqr(outX) + d*math::Sqr(outY);

  {
   real32 x = math::SignNZ(a) * sqrtN;
   real32 y = 0.0;
   real32 score = a*x + b*y + c*math::Sqr(x) + d*math::Sqr(y);
   
   if (score>bestScore)
   {
    outX = x;
    outY = y;
    bestScore = score;
   }
  }
  
  
 // Now try all solutions to the polynomial of its bastardised derviative set to 0...
 // (Ignore the fact they are complex - just take the real part and try that if
 // it passes the range test - safer to just test a few extra values.)
 for (nat32 i=0;i<rCount;i++)
 {
  real32 x = root[i].x;
  if (math::Abs(x)>sqrtN) continue;
  if (!math::SameSign(a,x)) x = -x;
  real32 y = math::SignNZ(b) * math::Sqrt(n-math::Sqr(x));
  
  real32 score = a*x + b*y + c*math::Sqr(x) + d*math::Sqr(y);
  if (score>bestScore)
  {
   outX = x;
   outY = y;
   bestScore = score;
  }
 }
}

//------------------------------------------------------------------------------
 };
};
