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


#include "mat_test/main.h"

//------------------------------------------------------------------------------
int main()
{
 using namespace eos;

 data::Random rand;

 os::Con conObj;
 os::Conversation & con = *conObj.StartConversation();

 // Create a matrix and a vector, fill with random values, solve Ax = b for x,
 // output the lot...
 /*{
  con << "Solving Ax = b:\n\n";
  math::Mat<6> a;
  math::Vect<6> x;

  for (nat32 i=0;i<x.Size();i++) x[i] = rand.Signed();
  for (nat32 r=0;r<a.Rows();r++)
  {
   for (nat32 c=0;c<a.Cols();c++)
   {
    a[r][c] = rand.Signed();
   }
  }

  con << "x = " << x << "\n\n";
  con << "a = " << a << "\n";
  con << "a's determinant = " << Determinant(a) << "\n\n\n";


  math::Mat<6> chol;
  math::Mat<6> at = a;
  Transpose(at);
  Mult(a,at,chol);

  math::Vect<6> b;
  MultVect(a,x,b);
  con << "b = " << b << "\n\n\n";

  at = a;
  SolveLinear(at,b);
  con << "x (solved) = " << b << "\n\n\n\n";



  con << "Testing Cholesky:\n\n";
  con << "c = " << chol << "\n\n";
  if (Cholesky(chol))
  {
   con << "ch = " << chol << "\n\n\n\n";
  } else con << "No Cholesky for c matrix\n\n\n\n";



  con << "Testing Inverse of a:\n\n";

  math::Mat<6> inv = a;
  if (Inverse(inv,at))
  {
   con << "inv = " << inv << "\n\n";

   Mult(inv,a,at);

   con << "inv*a = " << at << "\n\n\n\n";
  } else con << "Inverse failed for matrix a\n\n\n\n";



  con << "Testing SVD of a:\n\n";
  math::Mat<6> u = a;
  math::Vect<6> d;
  math::Mat<6> v;
  if (SVD(u,d,v,b))
  {
   con << "u = " << u << "\n\n";
   con << "d = " << d << "\n\n";
   con << "v = " << v << "\n\n";

   Transpose(v);
   MultDiag(u,d,at);
   math::Mat<6> r;
   Mult(at,v,r);

   con << "u*Diag(d)*v^T = " << r << "\n\n\n\n";
  } else con << "SVD failed for matrix a\n\n\n\n";
 }


 // Do it all again, but this time overspecify, to test a different algorithm...
 {
  con << "Solving Ax = b for the overspecified case:\n\n";
  math::Mat<12,6> a;
  math::Vect<6> x;

  for (nat32 i=0;i<x.Size();i++) x[i] = rand.Signed();
  for (nat32 r=0;r<a.Rows();r++)
  {
   for (nat32 c=0;c<a.Cols();c++)
   {
    a[r][c] = rand.Signed();
   }
  }

  con << "x = " << x << "\n\n";
  con << "a = " << a << "\n\n\n";

  math::Vect<12> b;
  MultVect(a,x,b);
  con << "b = " << b << "\n\n\n";

  math::Mat<6> t;
  if (SolveLeastSquares(a,b,x,t))
  {
   con << "x (solved) = " << x << "\n\n\n";
  }
  else con << "Error solving for X. (Should not happen.)\n\n\n";
 }

 // Test the psuedo inverse...
 {
  con << "Testing Pseudo Inverse:\n\n";
  math::Mat<6> in;

  for (nat32 r=0;r<in.Rows();r++)
  {
   for (nat32 c=0;c<in.Cols();c++)
   {
    in[r][c] = rand.Signed();
   }
  }
  for (nat32 i=0;i<in.Cols();i++) in[4][i] = 0.0;

  math::Mat<6> out = in;
  math::PseudoInverseMatTemp< math::Mat<6> > temp;
  if (PseudoInverse(out,temp))
  {
   con << "in = " << in << "\n\n";
   con << "out = " << out << "\n\n";

   math::Mat<6> idn;
   Mult(in,out,idn);

   con << "idn = " << idn << "\n\n\n\n";
  }
  else con << "Error doing Pseudo Inverse\n\n\n\n";
 }

 // Test the LM implimentation by solving a nasty non-linear problem...
 {
  // We fit to a sphere, select a centre and radius to be 'found'...
  // (Actually, a hemisphere in a plane.)
   math::Vect<3> centre;
    centre[0] = rand.Signed()*16.0;
    centre[1] = rand.Signed()*16.0;
    centre[2] = rand.Signed()*16.0;
   real32 radius = (rand.Normal()+2.0)*5.0;

  // Build a set of points, with noise...
   static const nat32 pntCount = 12;
   math::Mat<pntCount,3> pnt;
   for (nat32 i=0;i<pntCount;i++)
   {
    real32 polA = rand.Real(-math::pi,math::pi);
    real32 polB = rand.Real(-math::pi*0.5,math::pi*0.5);

    pnt[i][0] = math::Sin(polA)*math::Cos(polB);
    pnt[i][1] = math::Sin(polB);
    pnt[i][2] = math::Cos(polA)*math::Cos(polB);

    pnt[i][0] = pnt[i][0]*radius + centre[0] + rand.Gaussian(0.2);
    pnt[i][1] = pnt[i][1]*radius + centre[1] + rand.Gaussian(0.2);
    pnt[i][2] = pnt[i][2]*radius + centre[2] + rand.Gaussian(0.2);
   }

  // Make the parameter vector required, initialise with a boring sphere...
  // First 3 entry are centre, last one is radius.
   math::Vect<4> pa;
    pa[0] = 0.0;
    pa[1] = 0.0;
    pa[2] = 0.0;
    pa[3] = 1.0;

  // The function required...
   struct S
   {
    static void F(const math::Vect<4> & pv,math::Vect<pntCount> & err,const math::Mat<pntCount,3> & pnt)
    {
     for (nat32 i=0;i<pntCount;i++)
     {
      real32 dist = math::Sqrt(math::Sqr(pnt[i][0]-pv[0]) + math::Sqr(pnt[i][1]-pv[1]) + math::Sqr(pnt[i][2]-pv[2]));
      err[i] = dist - pv[3];
     }
    }
   };

  // Call LM...
   real32 residual = LM(pa,pnt,&S::F);

  // Print both the input and output to the console...
   con << "Testing Levenberg Marquardt:\n";
   con << "Actual = " << centre << ":" << radius << "\n";
   con << "Found = " << pa << "\n";
   con << "Residual = " << residual << "\n\n";

   for (nat32 i=0;i<pntCount;i++)
   {
    con << "Point = " << pnt << "\n";
   }
   con << "\n\n";
 }

 // Test the QR and thin QR implimentations, plust the RQ implimentation...
 {
  con << "Testing QR:\n";

  math::Mat<7,4> a;
  for (nat32 r=0;r<a.Rows();r++)
  {
   for (nat32 c=0;c<a.Cols();c++)
   {
    a[r][c] = rand.Signed();
   }
  }

  math::Mat<7,4> r = a;
  math::Mat<7,7> q;
  math::QR(r,q);

  con << "a = " << a << "\n\n";
  con << "q = " << q << "\n\n";
  con << "r = " << r << "\n\n";

  Identity(a);
  math::Mult(q,r,a);

  con << "a' = " << a << "\n\n";
 }

 {
  con << "Testing thin QR:\n";

  math::Mat<7,4> a;
  for (nat32 r=0;r<a.Rows();r++)
  {
   for (nat32 c=0;c<a.Cols();c++)
   {
    a[r][c] = rand.Signed();
   }
  }

  math::Mat<7,4> q = a;
  math::Mat<4,4> r; Identity(r); // To set the lower triangular part to 0.
  math::ThinQR(q,r);

  con << "a = " << a << "\n\n";
  con << "q = " << q << "\n\n";
  con << "r = " << r << "\n\n";

  Identity(a);
  math::Mult(q,r,a);

  con << "a' = " << a << "\n\n";
 }


 {
  con << "Testing RQ:\n";

  math::Mat<5,6> a;
  for (nat32 r=0;r<a.Rows();r++)
  {
   for (nat32 c=0;c<a.Cols();c++)
   {
    a[r][c] = rand.Signed();
   }
  }

  math::Mat<5,6> r = a;
  math::Mat<6,6> q;
  math::RQ(r,q);

  con << "a = " << a << "\n\n";
  con << "q = " << q << "\n\n";
  con << "r = " << r << "\n\n";

  Identity(a);
  math::Mult(r,q,a);

  con << "a' = " << a << "\n\n";
 }


 {
  con << "Testing Hessenberg maker part of eigen calculation:\n";

  math::Mat<6> a;
  for (nat32 r=0;r<a.Rows();r++)
  {
   for (nat32 c=0;c<a.Cols();c++)
   {
    a[r][c] = rand.Signed();
   }
  }

  con << "a = " << a << "\n\n";

  math::Mat<2,6> t;
  math::MakeHessenberg(a,t);

  con << "a' = " << a << "\n\n";


  con << "Applying Real Schur to above:\n";
  if (math::RealSchurHessenberg(a)==false) printf("Failed.\n");

  con << "a'' = " << a << "\n\n";
 }



 {
  printf("Testing Eigenvalue determination:\n");

  math::Mat<6> a;
  for (nat32 r=0;r<a.Rows();r++)
  {
   for (nat32 c=0;c<a.Cols();c++)
   {
    a[r][c] = rand.Signed();
   }
  }

  con << "a = " << a << "\n\n";

  math::Mat<2,6> t;
  math::Vect<6,math::Complex<real32> > ev;
  if (math::Eigenvalues(a,ev,t)==false) con << "Failed\n";

  for (nat32 i=0;i<6;i++) con << "ev[" << i << "] = " << ev[i].x << " + " << ev[i].y << "\n";
  con << "\n\n";
 }



 {
  con << "Testing the polynomial root solver: (Uses eigenvalues.)\n\n";
  math::Vect<6> ri;
  for (nat32 i=0;i<ri.Size();i++) ri[i] = 10.0*rand.Signed();
  for (nat32 i=0;i<6;i++)
  {
   con << "ir" << i << " = " << ri[i] << "\n";
  }
  con << "\n";


  math::Vect<6> p;
  math::RootsToPoly(ri,p);

  math::Vect<6,math::Complex<real32> > r;
  math::Mat<6> tmp;

  if (math::PolyRoot(p,r,tmp)==false) con << "Failure\n";

  con << "f(x) = " << p[0] << " + " << p[1] << "*x + " << p[2] << "*x^2 + " <<
      p[3] << "*x^3 + " << p[4] << "*x^4 + " << p[5] << "*x^5 + x^6\n\n";

  for (nat32 i=0;i<6;i++)
  {
   math::Complex<real32> fr = r[i]; fr ^= 6;
   for (nat32 j=0;j<6;j++)
   {
    math::Complex<real32> temp = r[i];
    temp ^= j;
    temp *= p[j];
    fr += temp;
   }
   con << "r" << i << " = " << r[i].x << " + " << r[i].y << "*i";
   con << "f(r" << i << ") = " << fr.x << " + " << fr.y << "*i\n";
  }
  con << "\n\n";
 }*/



 /*{
  con << "Testing SVD\n";
  static const nat32 rows = 12;
  static const nat32 cols = 4;

  real32 frobMean = 0.0;
  real32 frobMax = -1.0;
  nat32 failedCount = 0;
  nat32 iters = 100000;

  math::Mat<rows,cols> worstA;
  math::Mat<rows,cols> worstU;
  math::Vect<cols> worstD;
  math::Mat<cols,cols> worstV;

  math::Mat<rows,cols> a;
  for (nat32 i=0;i<iters;i++)
  {
   if ((i%2)==0)
   {
    for (nat32 r=0;r<a.Rows();r++)
    {
     for (nat32 c=0;c<a.Cols();c++) a[r][c] = rand.Signed();
    }
   }
   //con << "a = " << a << "\n\n\n";

   math::Mat<rows,cols> u = a;
   math::Vect<cols> d;
   math::Mat<cols,cols> v;
   math::Vect<cols-1> temp;
   if (math::SVD(u,d,v,temp)==false) failedCount += 1;

   //con << "u = " << u << "\n\n";
   //con << "d = " << d << "\n\n";
   //con << "v = " << v << "\n\n";

   math::Mat<rows,cols> temp2;
   math::Mat<rows,cols> temp3;
   math::MultDiag(u,d,temp2);
   math::MultTrans(temp2,v,temp3);
   //con << "a' = " << temp3 << "\n";

   temp3 -= a;
   real32 frob = math::FrobNorm(temp3);
   frobMean += frob;

   if (frobMax<frob)
   {
    frobMax = frob;

    // Store for further analysis...
     worstA = a;
     worstU = u;
     worstD = d;
     worstV = v;
   }
   //con << "Frob norm of diff = " <<  << "\n\n";

   if ((i%2)==0)
   {
    d[rand.Int(0,cols-1)] = 0.0;
    math::MultDiag(u,d,temp2);
    math::MultTrans(temp2,v,a);
   }
  }
  frobMean /= real32(iters);

  con << "Average frob = " << frobMean << "\n";
  con << "Maximum frob = " << frobMax << "\n";
  if (failedCount!=0) con << "Failed " << failedCount << " times\n\n";

  con << "worst a = " << worstA << "\n";
  con << "worst u = " << worstU << "\n";
  con << "worst d = " << worstD << "\n";
  con << "worst v = " << worstV << "\n\n\n";
 }*/



 /*{
  con << "Testing Symmetric Eigendecomposition:\n";
  static const nat32 size = 5;

  math::Mat<size,size> a;
  for (nat32 r=0;r<a.Rows();r++)
  {
   for (nat32 c=r;c<a.Cols();c++)
   {
    real32 val = rand.Signed();
    a[r][c] = val;
    a[c][r] = val;
   }
  }
      
  con << "a = " << a << "\n\n\n";
  
  math::Mat<size,size> q;
  math::Vect<size> d;
  math::Mat<size,size> b = a;
  if (math::SymEigen(b,q,d)==false) con << "Failed\n\n\n";
  else
  {
   con << "q = " << q << "\n\n";
   con << "d = " << d << "\n\n";
   
   math::Mat<size,size> t1,t2;
   math::TransMult(q,a,t1);
   math::Mult(t1,q,t2);
   con << "d2 = " << t2 << "\n\n\n";
  }
 }*/



 {
  con << "Testing polynomial fitting:\n";
  
  static const nat32 length = 6;
  static const nat32 samples = 16;
  static const real32 noise = 0.2;

  // Create random polynomial and print...   
   real32 poly[length];
   for (nat32 i=0;i<length;i++) poly[i] = rand.Signed();
   con << "Starting polynomial = " << poly[0];
   for (nat32 i=1;i<length;i++) con << " + " << poly[i] << "*x^" << i;
   con << "\n";
  
  // Extract a random sample from it, add noise...
   math::Vect<samples> x;
   math::Vect<samples> y;
   for (nat32 i=0;i<samples;i++)
   {
    x[i] = rand.Signed()*5.0;
    y[i] = rand.Gaussian(noise);
    real32 val = 1.0;
    for (nat32 i=0;i<length;i++)
    {
     y[i] += poly[i] * val;
     val *= x[i];
    }
   }
  
  // Fit polynomial and print...
   math::Vect<samples> p;
   bit res;
   {
    math::Vect<samples> tmp[2];
    math::Mat<samples,samples> temp[2];
    res = math::RobustInterPoly(x,y,p,tmp[0],tmp[1],temp[0],temp[1]);
   }
  
   if (res)
   {
    con << "Fitted polynomial = " << p[0];
    for (nat32 i=1;i<length;i++) con << " + " << p[i] << "*x^" << i;
    con << "\n";
   }
   else
   {
    con << "Failed to fit.\n\n";
   }
 }


 con.WaitSize(1);
 return 0;
}

//------------------------------------------------------------------------------

