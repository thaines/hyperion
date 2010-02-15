#ifndef EOS_MATH_EIGEN_H
#define EOS_MATH_EIGEN_H
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


/// \file eigen.h
/// Provides eigenvalue and eigen vector functions, and other related stuff.

#include "eos/types.h"
#include "eos/file/csv.h"
#include "eos/math/mat_ops.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
/// This is given a real square matrix, A, it then finds a matrix U and changes
/// A such that A := U^T A U is a upper Hessenberg matrix.
/// U is a product of householder matrices, this version does not output u.
/// \param a A nxn matrix, replaces with a Hessenberg matrix.
/// \param x A 2xn, or larger, matrix used for tempory storage.
/// \param size n, the size of the matrix. Defaults to the actual matrix size, 
///             but can be changed to do a small calculation in a smaller amount
///             of storage.
template <typename TA,typename TB>
inline void MakeHessenberg(TA & a,TB & x,nat32 size = 0)
{
 if (size==0) size = a.Rows();
 log::Assert((a.Rows()>=size)&&(a.Cols()>=size)&&(x.Rows()>=2)&&(x.Cols()>=size));
 for (nat32 k=0;k<size-2;k++)
 {
  // Create the relevant householder vector and scaler for this step to null the entrys we don't like...
   typename TA::type beta;
   {
    typename TA::type sigma = static_cast<typename TA::type>(0);
    for (nat32 i=k+2;i<size;i++) sigma += math::Sqr(a[i][k]);	   
	      
    for (nat32 i=k+2;i<size;i++) x[0][i] = a[i][k];
    
    if (math::Equal(sigma,static_cast<typename TA::type>(0)))
    {
     x[0][k+1] = static_cast<typename TA::type>(1); // To avoid random nan's.
     beta = static_cast<typename TA::type>(0);
    }
    else
    {    
     typename TA::type mu = math::Sqrt(math::Sqr(a[k+1][k]) + sigma);
     if (a[k+1][k]<=static_cast<typename TA::type>(0))
     {
      x[0][k+1] = a[k+1][k] - mu;
     }
     else
     {
      x[0][k+1] = -sigma/(a[k+1][k] + mu);
     }
     beta = static_cast<typename TA::type>(2) * math::Sqr(x[0][k+1]) / (sigma + math::Sqr(x[0][k+1]));
     typename TA::type vMult = static_cast<typename TA::type>(1)/x[0][k+1];
     for (nat32 i=0;i<size;i++) x[0][i] *= vMult;
    }
   }
  
  // Apply...
   // Step 1...
    for (nat32 c=k;c<size;c++)
    {
     for (nat32 r=k+1;r<size;r++) x[1][r] = a[r][c];
     for (nat32 r=k+1;r<size;r++)
     {
      for (nat32 i=k+1;i<size;i++) a[r][c] -= beta*x[0][r]*x[0][i]*x[1][i];
     }
    }
    
   // Step 2...
    for (nat32 r=0;r<size;r++)
    {
     for (nat32 c=k+1;c<size;c++) x[1][c] = a[r][c];
     for (nat32 c=k+1;c<size;c++)
     {
      for (nat32 i=k+1;i<size;i++) a[r][c] -= beta*x[0][c]*x[0][i]*x[1][i];	     
     }
    }
 }	
}

//------------------------------------------------------------------------------
/// This decomposes a real matrix, H, such that Q^T H Q = T where the real
/// eigenvalues can be found on the diagonal of T and the complex eigenvalues
/// can be found as the eignvalues of 2x2 matrixes on the diagonal.
/// This version does not output Q.
/// Uses the QR algorithm with the Francis QR step, i.e. the most advanced
/// version given in Golub and Van Loan.
/// \param h As input an upper hessenberg square matrix to be decomposed, as output the T matrix.
/// \param tol The tolerance used to decide when success has been obtained. Set this too low and it can fail.
/// \param max_iters Maximum number of iterations, infinite loops suck.
/// \param size By default, n, the size of the matrix, but can be overloaded to do a smaller calculation in
///             a larger matrix.
/// \returns true on success, false on failure - you set tol and max_iters too low, or the numbers passed in are too extreme.
template <typename TA>
inline bit RealSchurHessenberg(TA & h,typename TA::type tol = static_cast<typename TA::type>(1e-6),nat32 max_iters = 1000,nat32 size = 0)
{
 if (size==0) size = h.Rows();
 log::Assert((h.Rows()>=size)&&(h.Cols()>=size));

 for (nat32 i=0;i<max_iters;i++)
 {
  // Iterate and set all sub-diagonal elements that are close 
  // enough to zero to be called zero to be zero.
  // Simultaneously find p and q, which bounds the unreduced sub-matrix that
  // directly follows the lower corners quasi-triangular part, i.e. the part
  // that is complete.
  // Unlike the book we set p to be the upper-left corner and q to be the
  // lower-right corner, so for the Francis QR step we are handling a matrix
  // going from h[p][p] to h[q][q].
   nat32 p = size;
   nat32 q = 0;
   nat32 nonz = 0; // Number of non-zeros seen in a row. (Please ignore overloaded language and pun-like use.)
   for (nat32 r=size-1;r>0;r--)
   {
    bit isZ = Abs(h[r][r-1]) <= tol*(Abs(h[r][r]) + Abs(h[r-1][r-1]));
    if (isZ)
    {
     h[r][r-1] = static_cast<typename TA::type>(0);
     if ((nonz>1)&&(p==size))
     {
      p = r+1;
      q = r+nonz;
     }
     nonz = 0;
    }
    else ++nonz;
   }
   if (p==size)
   {
    if (nonz>1)
    {
     p = 0;
     q = nonz;
    }
    else
    {
     // Success - the entire matrix is upper-quasi-triangular. Have beer!..
      return true;
    }
   }


  // Perform a Francis QR step...
  // (I can not claim to understand this code - I understand the ideas its based
  // on, and how to turn psuedo code into real code. Filling the gap between
  // would of taken more time than I could justify.)
  // I do understand that this requires a 3x3 matrix, and the above could give a
  // 2x2 matrix - fuck knows whats happenning there?
   // Create a load of variables - something to do with computing a first column...
    typename TA::type s = h[q-1][q-1] + h[q][q];
    typename TA::type t = h[q-1][q-1]*h[q][q] - h[q-1][q]*h[q][q-1];
    
    typename TA::type x = Sqr(h[p][p]) + h[p][p+1]*h[p+1][p] - s*h[p][p] + t;
    typename TA::type y = h[p+1][p] * (h[p][p]+h[p+1][p+1]-s);
    typename TA::type z = h[p+1][p]*h[p+2][p+1];


   // Loop and do something... Its probably loopy.
    for (nat32 k=p;k<q-1;k++)
    {
     // Calculate the householder of [x,y,z]^T...
      typename TA::type beta;
      Vect<3,typename TA::type> v;
      {
       typename TA::type sigma = Sqr(y) + Sqr(z);
       v[0] = static_cast<typename TA::type>(1); v[1] = y; v[2] = z;
       if (math::Equal(sigma,static_cast<typename TA::type>(0))) beta = static_cast<typename TA::type>(0);
       else
       {
	typename TA::type mu = Sqrt(Sqr(x) + sigma);
	if (x<=0) v[0] = x - mu;
	     else v[0] = -sigma/(x+mu);
        beta = static_cast<typename TA::type>(2)*Sqr(v[0])/(sigma+Sqr(v[0]));
        v /= v[0];
       }
      }

     // Apply it...
      nat32 sc = k; if (sc!=0) --sc;      
      Vect<3,typename TA::type> t;
      for (nat32 c=sc;c<=q;c++)
      {
       for (nat32 r=k;r<k+3;r++) t[r-k] = h[r][c];
       for (nat32 r=k;r<k+3;r++)
       {
	for (nat32 i=k;i<k+3;i++) h[r][c] -= beta*v[r-k]*v[i-k]*t[i-k];
       }
      }
      
      nat32 er = Min(k+3,q);
      for (nat32 r=0;r<=er;r++)
      {
       for (nat32 c=k;c<k+3;c++) t[c-k] = h[r][c];
       for (nat32 c=k;c<k+3;c++)
       {
	for (nat32 i=k;i<k+3;i++) h[r][c] -= beta*v[c-k]*v[i-k]*t[i-k];       
       }
      }

     // Update x,y and z for the next loop...
      x = h[k+1][k];
      y = h[k+2][k];
      if (k<=q-3) z = h[k+3][k];
    }


   // Final step (Yes, another householder application.)...
    // Householder of [x,y]^T...
     typename TA::type beta;
     Vect<2,typename TA::type> v;
     {
      typename TA::type sigma = Sqr(y);
      v[0] = static_cast<typename TA::type>(1); v[1] = y;
      if (math::Equal(sigma,static_cast<typename TA::type>(0))) beta = static_cast<typename TA::type>(0);
      else
      {
       typename TA::type mu = Sqrt(Sqr(x) + sigma);
       if (x<=0) v[0] = x - mu;
            else v[0] = -sigma/(x+mu);
        beta = static_cast<typename TA::type>(2)*Sqr(v[0])/(sigma+Sqr(v[0]));
        v /= v[0];
      }
     }

    // Apply the householder...
    {
     Vect<2,typename TA::type> t;
     for (nat32 c=q-2;c<=q;c++)
     {
      typename TA::type a = h[q-1][c];
      typename TA::type b = h[q][c];
      h[q-1][c] -= beta*(v[0]*v[0]*a + v[0]*v[1]*b);
      h[q][c]   -= beta*(v[1]*v[0]*a + v[1]*v[1]*b);
     }
     
     for (nat32 r=p;r<=q;r++)
     {
      typename TA::type a = h[r][q-1];
      typename TA::type b = h[r][q];
      h[r][q-1] -= beta*(v[0]*v[0]*a + v[1]*v[0]*b);
      h[r][q]   -= beta*(v[0]*v[1]*a + v[1]*v[1]*b);
     }
    }
 }
 
 // Upper triangularize the 2x2 blocks that have real eigenvalues...
 
 // Code me ************************************************** (And remember to make sure this code is called, as this is currently skipped on success.)
 
 return false;
}

/// Given a real matrix this computers its real Schur decomposition.
/// \param a The input nxn matrix, replaced with its Schur decomposition.
/// \param x Tempory 2xn matrix, can be larger if conveniant.
/// \param tol Tolerance it uses to decide when its accurate enough.
/// \param max_iters Maximum number of iterations, so it has to complete.
/// \param size n, the size of the matrix. By default its actual size,
///             but can be used to do the calculation on a larger matrix than is used.
/// \returns true on success, false on failure. Will fail due to not having enough iterations to converge or trying to converge to a tolerance thats too small for the number of iterations avaliable.
template <typename TA,typename TB>
inline bit RealSchur(TA & a,TB & x,typename TA::type tol = static_cast<typename TA::type>(1e-6),nat32 max_iters = 1000,nat32 size = 0)
{
 MakeHessenberg(a,x,size);
 return RealSchurHessenberg(a,tol,max_iters,size);
}

//------------------------------------------------------------------------------
// Internal method, also used by the polynomial solver...
template <typename TA,typename TV>
inline void ExtractEigenvalues(TA & a,TV & out,nat32 size)
{
 for (nat32 i=0;i<size;i++)
 {
  if ((i==size-1)||(math::Equal(a[i+1][i],static_cast<typename TA::type>(0))))
  {
   // Real eigenvalue...
    out[i].x = a[i][i];
    out[i].y = static_cast<typename TV::type::type>(0);	   
  }
  else
  {
   // Complex eigenvalue (I allow the for once that are actual real, even though that shouldn't happen)...
    typename TV::type::type cd = static_cast<typename TV::type::type>(4)*a[i][i+1]*a[i+1][i] + Sqr(a[i][i] - a[i+1][i+1]);
    if (cd>=static_cast<typename TV::type::type>(0))
    {
     // Actually real...
      cd = static_cast<typename TV::type::type>(0.5)*Sqrt(cd);
      out[i].x = static_cast<typename TV::type::type>(0.5)*(a[i][i] + a[i+1][i+1]);
      out[i+1].x = out[i].x;
      out[i].x += cd;
      out[i+1].x -= cd;
      out[i].y = static_cast<typename TV::type::type>(0);
      out[i+1].y = static_cast<typename TV::type::type>(0);
    }
    else
    {
     // Truly complex...
      cd = static_cast<typename TV::type::type>(0.5)*Sqrt(-cd);
      out[i].x = static_cast<typename TV::type::type>(0.5)*(a[i][i] + a[i+1][i+1]);
      out[i].y = cd;
      out[i+1].x = out[i].x;
      out[i+1].y = -cd;
    }
   ++i;	   
  }
 }
}

/// This calculates the eigenvalues of the given real matrix, outputing them 
/// into a vector.
/// \param a An input nxn matrix of real values, is corrupted by the proccess.
/// \param out The output, must be a size n vector of complex numbers.
/// \param x Tempory storage for the algorithm, a 2xn matrix.
/// \param tol Tolerance used to calculate the result.
/// \param max_iters maximum number of iterations to perform.
/// \param size Size of the matrix - n. Can be set smaller if need be.
/// \returns true on success, false can only happen if insufficient numerical accuracy exists within the tolerance given or not enough iterations are avaliable.
template <typename TA,typename TV,typename TB>
inline bit Eigenvalues(TA & a,TV & out,TB & x,
                       typename TA::type tol = static_cast<typename TA::type>(1e-6),nat32 max_iters = 1000,
                       nat32 size = 0)
{
 if (size==0) size = a.Rows();

 // Do a real schur...
  if (RealSchur(a,x,tol,max_iters,size)==false) return false;
 
 // Iterate the diagonal and extract all eigenvalues...
  ExtractEigenvalues(a,out,size);
 
 return true;	
}

//------------------------------------------------------------------------------// Helper for below...
template <typename T>
inline typename T::type RootsToPolyPermSum(const T & in,nat32 permSize,nat32 pos)
{
 typename T::type ret;
 
 log::Assert(in.Size()-pos>=permSize);
 if ((in.Size()-pos)==permSize)
 {
  // No choice - only space for a single permutation...
   ret = -in[pos];
   for (nat32 i=pos+1;i<in.Size();i++) ret *= -in[i];
 }
 else
 {
  // We have a choice in permutation possibilities, create and sum them all...
   // Permutations that include pos...
    if (permSize==1) ret = -in[pos];
    else
    {
     ret = RootsToPolyPermSum(in,permSize-1,pos+1);
     ret *= -in[pos];
    }
   
   // Permutations that do not include pos...
    ret += RootsToPolyPermSum(in,permSize,pos+1);
 }

 return ret;
}

/// This is simply given the roots of a polynomial, it then outputs the
/// vector of polynomial constants, where the top polynomial term has a 
/// constant of 1. This is precisly the inverse of PolyRoot.
/// The two input vectors must be of identical size, complex numbers may be 
/// used. Rather brute force in its approach, but only implimented for testing 
/// purposes.
template <typename T>
inline void RootsToPoly(const T & in,T & out)
{
 log::Assert(in.Size()==out.Size());
 
 // Iterate through the outputs and sum each one, each one being the sum 
 // of all permutations of a praticular size...
 for (nat32 i=0;i<in.Size();i++)
 {
  out[i] = RootsToPolyPermSum(in,in.Size()-i,0);
 }
}

//------------------------------------------------------------------------------
/// Solves a real polynomials roots. Uses the eigenvalue techneque and should be
/// reliable. Uses a normalised (monic) polynomial representation.
/// \param in The input vector, must be real. Represents a normalised polynomial,
///           i.e. the top factor is 1. The polynomial has the same number of 
///           degrees as this vector's has elements, as the top value of 1 is 
///           not provided.
/// \param out The output vector, must be complex and the same size as the input
///            vector. In the event of multiple roots expect repeated values, 
///            except it will probably fail.
/// \param temp A tempory square matrix, big enough to contain the input vector.
///             Used for tempory storage.
/// \param tol Tolerance used for the calculation.
/// \param max_iters Makes sure it doesn't end up in an infinite loop cos it can't obtain the tolerance.
/// \param size If left as 0 it uses the size of the various things given, otherwise can be used
///             as an override to solve a size smaller than the storage.
/// \returns True on success, false on failure. Failure happens if the tolerance can not be obtained.
template <typename VA,typename VB,typename MA>
inline bit PolyRoot(const VA & in,VB & out,MA & temp,
                    typename MA::type tol = static_cast<typename MA::type>(1e-6),nat32 max_iters = 1000,nat32 size = 0)
{
 if (size==0) size = in.Size();
 log::Assert((in.Size()>=size)&&(out.Size()>=size)&&(temp.Rows()>=size)&&(temp.Cols()>=size));
 
 // Create the companion matrix for the given vector...
  Zero(temp);
  for (nat32 i=1;i<size;i++) temp[i][i-1] = static_cast<typename MA::type>(1);
  for (nat32 i=0;i<size;i++) temp[i][size-1] = -in[i];

 // Apply the Schur decomposition...
  if (RealSchurHessenberg(temp,tol,max_iters,size)==false) return false;

 // Extract the eigenvalues...
  ExtractEigenvalues(temp,out,size);

 return true;
}

/// This is identical to PolyRoot, except you give it all the coefficients of
/// the polynomial, i.e. its not monic, and it makes it monic itself.
/// It handles scenarios when top entrys are set to 0, when the bottom entrys
/// are 0 and reverts to the quadratic formular etc if appropriate.
/// This means that the in vector must be full length, longer than the out 
/// vector by 1 ushally. size is the number of degrees of the polynomial plus one.
/// Returns how many roots were found, 0 on failure, or if there are actually no roots/infinite roots.
template <typename VA,typename VB,typename MA>
inline nat32 RobustPolyRoot(VA & in,VB & out,MA & temp,
                            typename MA::type tol = static_cast<typename MA::type>(1e-6),
                            nat32 max_iters = 1000,nat32 size = 0)
{
 if (size==0) size = in.Size();
 log::Assert((in.Size()>=size)&&(out.Size()>=size-1)&&(temp.Rows()>=size-1)&&(temp.Cols()>=size-1));
 
 // Remove bottom 0's, as we can then solve a simpler polynomial and just add 0 to the root list...
  nat32 lowShift = 0;
  while ((lowShift<size)&&(IsZero(in[lowShift]))) ++lowShift;
  if (lowShift!=0)
  {
   size -= lowShift;
   for (nat32 i=0;i<size;i++) in[i] = in[i+lowShift];
  }

 // Remove top zeros, check we actualy have something to solve...
  while ((size>0)&&(IsZero(in[size-1]))) --size;
  switch (size)
  {
   case 2:
   out[0] = static_cast<typename VB::type>(0);
   return 1;
   
   case 1:
   if (lowShift==0) return 0;
   else
   {
    out[0] = static_cast<typename VB::type>(0);
    return 1;
   }
    
   case 0: return 0;
  }


 // Make monic...
  in /= in[size-1];

 // Solve, revert to basic method if possible...
  --size;
  if (size==2)
  {
   // Quadratic formula...
    typename VB::type::type srb = Sqr(in[1]) - static_cast<typename VB::type::type>(4)*in[0];
    if (srb<static_cast<typename VB::type::type>(0))
    {
     // Complex roots...
      srb = static_cast<typename VB::type::type>(0.5)*Sqrt(-srb);
      out[0].x = static_cast<typename VB::type::type>(-0.5) * in[1];
      out[1].x = out[0].x;
      out[0].y = srb;
      out[1].y = -srb;
    }
    else
    {
     // Real roots...
      srb = static_cast<typename VB::type::type>(0.5)*Sqrt(srb);
      out[0].x = static_cast<typename VB::type::type>(-0.5) * in[1];
      out[1].x = out[0].x - srb;
      out[0].x += srb;
      out[0].y = static_cast<typename VB::type::type>(0);
      out[1].y = static_cast<typename VB::type::type>(0);      
    }
  }
  else
  {
   // Eigenvalue method...
    if (PolyRoot(in,out,temp,tol,max_iters,size)==false) return 0;
  }

 // If we removed bottom zeros add 0 to the list of roots...
  if (lowShift!=0)
  {
   out[size] = static_cast<typename VB::type>(0);
   ++size;
  }
  
 return size;
}

//------------------------------------------------------------------------------
/// This calculates the eigenvalues and eigenvectors of a symmetric square 
/// matrix. No complex numbers involved in this case.
/// The relation q^T a q = d will hold.
/// \param a The input symmetric matrix, will be trashed.
/// \param q Output rotation matrix, with the eigenvectors as the columns. nxn
/// \param d Output diagonal matrix, as a vector - the eigenvalues. Length n.
/// \returns true on success, false on failure.
template <typename M,typename V>
inline bit SymEigen(M & a,M & q,V & d)
{
 LogTime("eos::math::SymEigen");
 log::Assert((a.Rows()==a.Cols())&&(a.Rows()==q.Rows())&&(q.Rows()==q.Cols())&&(a.Rows()==d.Size()));

 // First perform a householder tri-diagonalisation, makes the following QR
 // iterations better...
  for (nat32 k=0;k<a.Rows()-2;k++)
  {
   // Calculate householder vector, store it in the subdiagonal...
   // (Replacing first value of v (Which is always 1) with beta.)
   // (For off diagonal symmetric entrys only fill in super-diagonal.)
    typename M::type sigma = static_cast<typename M::type>(0);
    typename M::type x0 = a[k+1][k];
    for (nat32 r=k+2;r<a.Rows();r++) sigma += math::Sqr(a[r][k]);
    
    if (math::IsZero(sigma)) a[k+1][k] = 0.0;
    else
    {
     typename M::type mu = math::Sqrt(math::Sqr(a[k+1][k]) + sigma);
     if (a[k+1][k]<=static_cast<typename M::type>(0)) a[k+1][k] = a[k+1][k] - mu;
                                                 else a[k+1][k] = -sigma/(a[k+1][k] + mu);
     
     for (nat32 r=k+2;r<a.Rows();r++) a[r][k] /= a[k+1][k];
     a[k+1][k] = static_cast<typename M::type>(2) * math::Sqr(a[k+1][k])/(sigma + math::Sqr(a[k+1][k]));
    }
    
   // Set the symmetric entry, needs info from above...
    a[k][k+1] = math::Sqrt(sigma + math::Sqr(x0));
   
   // Update the matrix with the householder transform (Make use of symmetry)...
    // Calculate p/beta, store in d...
     for (nat32 c=k+1;c<a.Cols();c++)
     {
      d[c] = a[c][k+1]; // First entry of v is 1.
      for (nat32 r=k+2;r<a.Rows();r++) d[c] += a[r][k] * a[c][r];
     }
     
    // Calculate w, replace p with it in d...
     typename M::type mult = d[k+1];
     for (nat32 r=k+2;r<a.Rows();r++) mult += a[r][k] * d[r];
     mult *= math::Sqr(a[k+1][k]) / static_cast<typename M::type>(2);
     
     d[k+1] = a[k+1][k] * d[k+1] - mult;
     for (nat32 c=k+2;c<a.Cols();c++) d[c] = a[k+1][k] * d[c] - mult * a[c][k];

    // Apply the update - make use of symmetry by only calculating the lower 
    // triangular set...
     // First column where first entry of v being 1 matters...
      a[k+1][k+1] -= static_cast<typename M::type>(2) * d[k+1];
      for (nat32 r=k+2;r<a.Rows();r++) a[r][k+1] -= a[r][k] * d[k+1] + d[r];

     // Remainning columns...
      for (nat32 c=k+2;c<a.Cols();c++)
      {
       for (nat32 r=c;r<a.Rows();r++)
       {
        a[r][c] -= a[r][k] * d[c] + a[c][k] * d[r];
       }
      }
      
     // Do the mirroring...
      for (nat32 r=k+1;r<a.Rows();r++)
      {
       for (nat32 c=r+1;c<a.Cols();c++) a[r][c] = a[c][r];
      }
  }


 // Use the stored sub-diagonal house-holder vectors to initialise q...
  math::Identity(q);
  for (int32 k=int32(a.Cols())-3;k>=0;k--)
  {
   // Arrange for v to start with 1 - avoids special cases...
    typename M::type beta = a[k+1][k];
    a[k+1][k] = static_cast<typename M::type>(1);
    
   // Update q, column by column...
    for (nat32 c=nat32(k)+1;c<q.Cols();c++)
    {
     // Copy column to tempory storage...
      for (nat32 r=nat32(k)+1;r<q.Rows();r++) d[r] = q[r][c];

     // Update each row in column...
      for (nat32 r=nat32(k)+1;r<q.Rows();r++)
      {
       typename M::type mult = beta * a[r][k];
       for (nat32 i=nat32(k+1);i<q.Cols();i++) q[r][c] -= mult * a[i][k] * d[i];
      }
    }
  }


 // Now perform QR iterations till we have a diagonalised - at which point it 
 // will be the eigenvalues... (Update q as we go.)
  // These parameters decide how many iterations are required...
   static const typename M::type epsilon = static_cast<typename M::type>(1e-6);
   static const nat32 max_iters = 64; // Maximum iters per value pair.
   nat32 iters = 0; // Number of iterations done on current value pair.

   bit all_good = true; // Return value -set ot false if iters ever reaches max_iters.
 
  // Range of sub-matrix being processed - start is inclusive, end exclusive.
   int32 start = a.Rows(); // Set to force recalculate.
   int32 end = a.Rows();

  // (Remember that below code ignores the sub-diagonal, as its a mirror of the super diagonal.)
  while (true)
  {
   // Move end up as far as possible, finish if done...
    int32 pend = end;
    while (true)
    {
     int32 em1 = end-1;
     int32 em2 = end-2;
     typename M::type tol = epsilon*(math::Abs(a[em2][em2]) + math::Abs(a[em1][em1]));
     if (math::Abs(a[em2][em1])<tol)
     {
      end -= 1;
      if (end<2) break;
     }
     else break;
    }
        
    if (pend==end)
    {
     iters += 1;
     if (iters==max_iters)
     {
      all_good = false;
      if (end==2) break;
      iters = 0;
      end -= 1;
      continue;
     }
    }
    else
    {
     if (end<2) break;
     iters = 0;
    }


   // If end has caught up with start recalculate it...
    if ((start+2)>end)
    {
     start = end-2;
     while (start>0)
     {
      int32 sm1 = start-1;
      typename M::type tol = epsilon*(math::Abs(a[sm1][sm1]) + math::Abs(a[start][start]));
      if (math::Abs(a[sm1][start])>=tol) start -= 1;
                                    else break;
     }
    }


   // Do the QR step, with lots of juicy optimisation...
    // Calculate eigenvalue of trailing 2x2 matrix...
     int32 em1 = end-1;
     int32 em2 = end-2;
     typename M::type temp = (a[em2][em2] - a[em1][em1]) / static_cast<typename M::type>(2);
     typename M::type div = temp + math::Sign(temp) * math::Sqrt(math::Sqr(temp) + math::Sqr(a[em2][em1]));
     typename M::type tev = a[em1][em1] - math::Sqr(a[em2][em1])/div;

    // Calculate and apply relevant sequence of givens transforms to
    // flow the numbers down the super/sub-diagonals...
     typename M::type x = a[start][start] - tev;
     typename M::type z = a[start][start+1];
     for (int32 k=start;;k++)
     {
      // Calculate givens transform...
       typename M::type gc = static_cast<typename M::type>(1);
       typename M::type gs = static_cast<typename M::type>(0);
       if (!math::IsZero(z))
       {
        if (math::Abs(z)>math::Abs(x))
        {
         typename M::type r = -x/z;
         gs = static_cast<typename M::type>(1)/math::Sqrt(static_cast<typename M::type>(1)+math::Sqr(r));
         gc = gs * r;
        }
        else
        {
         typename M::type r = -z/x;
         gc = static_cast<typename M::type>(1)/math::Sqrt(static_cast<typename M::type>(1)+math::Sqr(r));
         gs = gc * r;
        }
       }
       
       typename M::type gcc = math::Sqr(gc);
       typename M::type gss = math::Sqr(gs);


      // Update matrix q (Post multiply)...
       for (nat32 r=0;r<q.Rows();r++)
       {
        typename M::type ck  = q[r][k];
        typename M::type ck1 = q[r][k+1];
        q[r][k]   = gc*ck - gs*ck1;
        q[r][k+1] = gs*ck + gc*ck1;
       }


      // Update matrix a...
       // Conditional on not being at start of range...
        if (k!=start) a[k-1][k] = gc*x - gs*z;
        
       // Non-conditional...
       {
        typename M::type e = a[k][k];
        typename M::type f = a[k+1][k+1];
        typename M::type i = a[k][k+1];
        
        a[k][k] = gcc*e + gss*f - static_cast<typename M::type>(2)*gc*gs*i;
        a[k+1][k+1] = gss*e + gcc*f + static_cast<typename M::type>(2)*gc*gs*i;
        a[k][k+1] = gc*gs*(e-f) + (gcc - gss)*i;
        x = a[k][k+1];
       }

       // Conditional on not being at end of range...
        if (k!=end-2)
        {
         z = -gs*a[k+1][k+2]; // a[k][k+2]
         a[k+1][k+2] *= gc;
        }
        else break;
     }
  }


  // Fill in the diagonal...
   for (nat32 i=0;i<d.Size();i++) d[i] = a[i][i];
 
 return all_good;
}

//------------------------------------------------------------------------------
/// Same as SymEigen, except it sorts the eigenvalues into decreasing order.
/// (Uses a simple insertion sort, but then one usually doesn't do this to large
/// matrices, and even if so, the amount of time insertion sort takes is neglible
/// compared to finding the eigen-values/vectors in the first places.)
template <typename M,typename V>
inline bit SymEigenSort(M & a,M & q,V & d)
{
 if (SymEigen(a,q,d)==false) return false;
 
 for (nat32 rr=1;rr<d.Size();rr++)
 {
  for (nat32 r=rr;r>0;r--)
  { 
   if (d[r]<d[r-1]) break;
   math::Swap(d[r],d[r-1]);
   q.SwapRows(r,r-1);
  }
 }

 return true;
}

//------------------------------------------------------------------------------
/// This fits a polynomial function to a set of distinct points.
/// This version allows you to specify the highest exponent as the length
/// of input p minus one, you can even make it one to get a least 
/// squares linear regresion.
/// Note that this is robust to insufficient information - if not enough 
/// information is provided it will still produce a solution that fits the 
/// info avaliable.
/// It doesn't normalise input however - that is left to the user if needed.
/// x and y can be very, very big without problem - its efficiently coded.
/// WARNING - doesn't (yet) work - needs debugging.
/// \param x The vector of x values.
/// \param y The vector of y values. i.e. x=y for corrisponding indices.
/// \param p Output vector of polynomial terms. i.e. y[i] = p[0] + p[1]*x[i] + p[2]*x[i]^2 etc.
/// \param tmpA Tempory storage. Same as p.
/// \param tmpB Tempory storage. Same as p.
/// \param tempA Tempory storage. Square matrix thats the size of p in each dimension. (Or larger.)
/// \param tempB Tempory storage, same as tempA.
/// \returns true on success, false on failure.
template <typename VA,typename VB,typename MT>
bit RobustInterPoly(const VA & x,const VA & y,VB & p,VB & tmpA,VB & tmpB,MT & tempA,MT & tempB)
{
 log::Assert(x.Size()==y.Size());
 log::Assert(p.Size()<=tmpA.Size());
 log::Assert(p.Size()<=tmpB.Size());
 log::Assert((p.Size()<=tempA.Rows())&&(p.Size()<=tempA.Cols()));
 log::Assert((p.Size()<=tempB.Rows())&&(p.Size()<=tempB.Cols()));

 // Calculate the linear equation to solve - this is done by iterating the
 // incomming variables and handling them 1 at a time, as there usually a lot 
 // bigger than the matrix and vector and hence random access of the matrix and
 // vector are better than random access on them for caches. This gets a tad fiddly...
  // Zero out...
   for (nat32 i=0;i<p.Size();i++)
   {
    p[i] = 0.0;
    tempA[0][i] = 0.0;
    tempA[i][p.Size()-1] = 0.0;
   }

  // Iterate data points...
   for (nat32 i=0;i<x.Size();i++)
   {
    typename VA::type pow = static_cast<typename VA::type>(1);
    for (nat32 j=0;j<p.Size();j++)
    {
     p[j] += pow * y[i];
     tempA[0][j] += pow;
     pow *= x[i];
    }
    
    for (nat32 j=1;j<p.Size()-1;j++)
    {
     tempA[j][p.Size()-1] += pow;
     pow *= x[i];
    }
    tempA[p.Size()-1][p.Size()-1] += pow;
   }
  
  // Finish off...
   for (nat32 r=1;r<p.Size();r++)
   {
    for (nat32 c=0;c<p.Size()-1;c++) tempA[r][c] = tempA[r-1][c+1];
   }


 // Robustly solve the equation...
  if (SymEigen(tempA,tempB,tmpA)==false) return false;
  
  TransMultVect(tempB,p,tmpB);
  for (nat32 i=0;i<p.Size();i++)
  {
   tmpB[i] /= tmpA[i];
   if (!IsFinite(tmpB[i])) tmpB[i] = static_cast<typename VB::type>(0); // So it does a psuedo inverse.
  }
  MultVect(tempB,tmpB,p);
 
 return true;
}

//------------------------------------------------------------------------------
 };
};
#endif
