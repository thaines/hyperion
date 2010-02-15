#ifndef EOS_MATH_SVD_H
#define EOS_MATH_SVD_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file svd.h
/// Contains an implimentation of SVD.

#include "eos/types.h"
#include "eos/math/mat_ops.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
/// This solves the Thin SVD problem, outputing the entire decomposition.
/// Given 3 matrices; u,d and v, where u is A on call, decomposes such that
/// A = u * d * v^T. The sizes must all be correct on call.
/// \param u A mxn matrix, on call it is A, on return it will be u. u has orthogonal columns.
/// m must be greater than or equal to n.
/// \param d A n length output vector, to represent a diagonal matrix of the singular values.
/// This vectors values will be decreasing and all positive.
/// \param v A nxn matrix, outputed with orthogonal columns.
/// \param temp A n-1 length vector, used as internal tempory storage. Can be longer than n-1 without complaint.
/// \returns true on success, false on failure. failure is so rare it can almost be ignored.
template <typename MT1,typename MT2,typename MT3,typename MT4>
inline bit SVD(MT1 & u,MT2 & d,MT3 & v,MT4 & temp)
{
 log::Assert((u.Rows()>=u.Cols())&&(u.Cols()==d.Size())&&(v.Rows()==v.Cols())&&
             (v.Cols()==d.Size())&&(u.Cols()-1<=temp.Size()),"math::SVD");
 LogTime("eos::math::SVD");


 // First we need to do the householder bi-diagonalisation on the input, u...
 // (Note to self, should consider putting alternate R-Bidiagonalisation
 //  implimentation for use when rows>>cols. Can be done using v as output of the QR.)
 // The only variables outside of u used are for storage of the output beta
 // values, specifically d is filled with betas for the u matrix and temp is
 // filled with betas for the v matrix.
 {
  //LogTime("eos::math::SVD bi-di");
  for (nat32 c=0;c<u.Cols();c++)
  {
   // Compute householder vector of u[c][c] .. u[u.Rows()-1][c], to zero out
   // the lower triangle...
   // Does not create the vector, rather, outputs the divisor of the given
   // vector to create the vector, under the assumption the first entry in the
   // vector will then be set to 1. This is to avoid extra storage requirements.
    typename MT1::type beta;
    typename MT1::type div;
    {
     typename MT1::type sigma = static_cast<typename MT1::type>(0);
     for (nat32 i=c+1;i<u.Rows();i++) sigma += math::Sqr(u[i][c]);

     if (math::IsZero(sigma))
     {
      beta = static_cast<typename MT1::type>(0);
      div = static_cast<typename MT1::type>(1);
     }
     else
     {
      typename MT1::type mu = math::Sqrt(math::Sqr(u[c][c]) + sigma);
      if (u[c][c]<=static_cast<typename MT1::type>(0))
      {
       div = u[c][c] - mu;
      }
      else
      {
       div = -sigma/(u[c][c] + mu);
      }
      beta = (static_cast<typename MT1::type>(2)*math::Sqr(div)) / (sigma + math::Sqr(div));
     }
    }

    d[c] = beta;


   // Multiply u with householder matrix...
   // Do not calculate for the zeroed region, as they will be modified afterwards
   // to be the householder vector.
    typename MT1::type factor = beta/math::Sqr(div);
    for (nat32 c2=c;c2<u.Cols();c2++)
    {
     // Calculate this columns multiplier...
      typename MT1::type mult = u[c][c2] * div;
      for (nat32 r2=c+1;r2<u.Rows();r2++) mult += u[r2][c2] * u[r2][c];
      mult *= factor;

     // Update this column of the matrix...
      u[c][c2] -= mult * div;
      if (c2!=c)
      {
       for (nat32 r2=c+1;r2<u.Rows();r2++) u[r2][c2] -= mult * u[r2][c];
      }
    }


   // Modify the matrix so the position that would normally have been zeroed
   // becomes the essential part of the householder vector...
    for (nat32 i=c+1;i<u.Rows();i++) u[i][c] /= div;


   // If needed this iteration do the transpose version, to zero out the upper
   // triangle sans super diagonal....
    if (c<u.Cols()-2)
    {
     // Compute householder vector of u[c][c+1] .. u[c][u.Cols()-1]...
     // Does not create the vector, rather, outputs the divisor of the given
     // vector to create the vector, under the assumption the first entry in the
     // vector will then be set to 1. This is to avoid extra storage requirements.
     {
      typename MT1::type sigma = static_cast<typename MT1::type>(0);
      for (nat32 i=c+2;i<u.Cols();i++) sigma += math::Sqr(u[c][i]);

      if (math::IsZero(sigma))
      {
       beta = static_cast<typename MT1::type>(0);
       div = static_cast<typename MT1::type>(1);
      }
      else
      {
       typename MT1::type mu = math::Sqrt(math::Sqr(u[c][c+1]) + sigma);
       if (u[c][c+1]<=static_cast<typename MT1::type>(0))
       {
        div = u[c][c+1] - mu;
       }
       else
       {
        div = -sigma/(u[c][c+1] + mu);
       }
       beta = (static_cast<typename MT1::type>(2)*math::Sqr(div)) / (sigma + math::Sqr(div));
      }
     }

     temp[c] = beta;


     // Multiply u with householder matrix...
     // Don't update the section which would be nulled by the transformation,
     // leaving it such that it can be transformed into the essential part of
     // the householder vector.
      factor = beta/math::Sqr(div);
      for (nat32 r2=c;r2<u.Rows();r2++)
      {
       // Calculate this rows multiplier...
        typename MT1::type mult = u[r2][c+1] * div;
        for (nat32 c2=c+2;c2<u.Cols();c2++) mult += u[r2][c2] * u[c][c2];
        mult *= factor;

       // Update this row of the matrix...
        u[r2][c+1] -= mult * div;
        if (r2!=c)
        {
         for (nat32 c2=c+2;c2<u.Cols();c2++) u[r2][c2] -= mult * u[c][c2];
        }
      }


     // Update the part of the matrix that would of been zeroed out such that it
     // becomes the essential part of the householder vector...
      for (nat32 c2=c+2;c2<u.Cols();c2++) u[c][c2] /= div;
    }
  }
 }


 // Extract the v matrix...
 // (Go through and pre-multiply it with each transposed householder matrix
 // inturn as used to take out the top triangle sans-super-diagonal.)
 {
  //LogTime("eos::math::SVD mk-v");
  math::Identity(v);
  for (int32 i=v.Cols()-3;i>=0;i--)
  {
   for (nat32 c2=i;c2<v.Cols();c2++)
   {
    // Calculate this rows multiplier...
     typename MT1::type mult = v[i+1][c2];
     for (nat32 r2=i+2;r2<v.Rows();r2++) mult += v[r2][c2] * u[i][r2];
     mult *= temp[i];

    // Update this row...
     v[i+1][c2] -= mult;
     for (nat32 r2=i+2;r2<v.Rows();r2++) v[r2][c2] -= mult * u[i][r2];
   }
  }
 }



 // Store the bidiagonalisation in the vectors d and temp, with d the diagonal
 // and temp the super diagonal...
 // (Have to swap for the diagonal as it contains the betas which are still
 // needed to calculate u - the next step.)
  for (nat32 i=0;i<d.Size();i++) math::Swap(d[i],u[i][i]);
  for (nat32 i=0;i<d.Size()-1;i++) temp[i] = u[i][i+1];



 // Extract the u matrix (Inplace)...
 // (Pre-multiply with each householder matrix, only problem being that we are
 // pre-multiplying bits that contain data needed for the calculation at hand.
 // Throwing yourself off a cliff might be better than reading this.)
 {
  //LogTime("eos::math::SVD mk-u");
  for (int32 i=u.Cols()-1;i>=0;i--)
  {
   // Do the columns that contain output data...
    for (nat32 c2=i+1;c2<u.Cols();c2++)
    {
     // Arrange for upper triangular part of this calculation to be zeroed...
      u[i][c2] = static_cast<typename MT1::type>(0);

     // Calculate this columns multiplier...
      typename MT1::type mult = u[i][c2];
      for (nat32 r2=i+1;r2<u.Rows();r2++) mult += u[r2][c2] * u[r2][i];
      mult *= u[i][i];

     // Update this column...
      u[i][c2] -= mult;
      for (nat32 r2=i+1;r2<u.Rows();r2++) u[r2][c2] -= mult * u[r2][i];
    }

   // Update the column that contains the input data, do so by pretending that
   // it is part of an identity and applying the update...
    for (nat32 r2=i+1;r2<u.Rows();r2++) u[r2][i] *= -u[i][i];
    u[i][i] = static_cast<typename MT1::type>(1) - u[i][i];
  }
 }



 // Zero out the super-diagonal iterativly with Golub-Kahan SVD steps...
  static const typename MT1::type epsilon = static_cast<typename MT1::type>(1e-6);

  int32 q = 0; // Distance from bottom right corner of matrix to start of region for step.
  int32 p = 0; // Top right of current sub-matrx.
  nat32 iters = 0; // Counts the iterations to zero out each entry, used to fail if it takes too long, and to avoid infinite run-time.
  static const nat32 max_iters = 64; // Maximum iters per value. I've probably set it too high, but then, I've never seen SVD fail.
  bit all_good = true; // On failure I don't stop - I ignore it and keep going, as it will probably still produice a reasonable result.

  while (true)
  {
   // Find the range of the matrix which we are to apply an iteration step to...
   // (Detect overrun conditions and finished conditions.)
    // Calculate q...
     int32 preQ = q;
     for (int32 i=int32(d.Size())-2-q;i>=0;i--)
     {
      // If the relevant entry is not zero stop moving...
       if (math::Abs(temp[i])>(epsilon * (math::Abs(d[i]) + math::Abs(d[i+1])))) break;

      q += 1;
     }

    // Finish if we have done the entire super-diagonal...
     if ((d.Size()-q)<=1) break;

    // If q doesn't move, incriment iters, if does reset it to 0.
    // If iters==max_iters time to fail - return false...
     if (q==preQ)
     {
      iters += 1;
      if (iters==max_iters)
      {
       LogDebug("SVD Failed {p,q,u,d,v,temp}" << LogDiv()
                << p << LogDiv() << q << LogDiv()
                << u << LogDiv() << d << LogDiv() << v << LogDiv() << temp);
       all_good = false;

       q += 1;
       iters = 0;
       continue;
      }
     }
     else iters = 0;

    // Re-calculate p...
     p = d.Size()-2-q;
     for (;p>0;p--)
     {
      if (math::Abs(temp[p-1])<(epsilon * (math::Abs(d[p-1]) + math::Abs(d[p])))) break;
     }     



   // Check for rows that have a zeroed diagonal, and hence need to be zeroed
   // out for the super-diagonal in the same row, will need to adjust the region
   // size accordingly...
   {
    //LogTime("eos::math::SVD tz");
    bit zeroDiag = false;

    // Calculate threshold for a diagonal entry being 0.0 - frobnorm of B multiplied by epsilon.
    typename MT1::type zeroThreshold = static_cast<typename MT1::type>(0);
    for (nat32 i=p;i<d.Size()-q;i++) zeroThreshold += math::Sqr(d[i]);
    for (nat32 i=p;i<d.Size()-q-1;i++) zeroThreshold += math::Sqr(temp[i]);
    zeroThreshold = epsilon*math::Sqrt(zeroThreshold);

    for (int32 i=d.Size()-1-q;i>=p;i--)
    {
     if (math::Abs(d[i])<zeroThreshold)
     {
      zeroDiag = true;
      // Use givens rotations to shift the superdiagonal out and zero out the row...
      // (Different strategy if it is the last row, have to involve the row above
      //  and go in the reverse direction.)
       if (i==(int32(d.Size())-1-q))
       {
        //LogTime("eos::math::SVD zero-special");
        // Special case, we have to go the other direction - do it by columns
        // and float the problem value up the matrix until it hits the top and
        // explodes...
        // The columns are i, where the problem lies, and j.
        // The problem is stored in temp[i-1] throughout, dispite its migratory
        // behaviour.
         for (int32 j=i-1;true;j--)
         {
          // Calculate the givens transform to annihilate the problem entry...
           typename MT1::type c;
           typename MT1::type s;
           if (math::IsZero(temp[i-1]))
           {
	    c = static_cast<typename MT1::type>(1);
	    s = static_cast<typename MT1::type>(0);
           }
           else
           {
	    if (math::Abs(temp[i-1])>math::Abs(d[j]))
	    {
             typename MT1::type r = -d[j]/temp[i-1];
             s = math::InvSqrt(static_cast<typename MT1::type>(1) + math::Sqr(r));
             c = s*r;
	    }
	    else
	    {
             typename MT1::type r = -temp[i-1]/d[j];
             c = math::InvSqrt(static_cast<typename MT1::type>(1) + math::Sqr(r));
             s = c*r;
	    }
           }

          // Store the transform in the v matrix...
           for (nat32 k=0;k<v.Rows();k++)
           {
            typename MT1::type t1 = v[k][j];
	    typename MT1::type t2 = v[k][i];
	    v[k][j] = c*t1 - s*t2;
	    v[k][i] = s*t1 + c*t2;
           }

          // Apply the transform to zero the problem as it currently stands...
           d[j] = c*d[j] - s*temp[i-1];

          // Unless we have reached the end, apply the transform again to get
          // the new problem. If we have reached the end, break...
           if (j==0)
           {
            temp[i-1] = static_cast<typename MT1::type>(0);
            break;
           }

           temp[i-1]  = s*temp[j-1];
           temp[j-1] *= c;
         }
       }
       else
       {
	//LogTime("eos::math::SVD zero");
	// Normal case, do it by rows from below and take the problem right
	// until it hits the edge of the matrix and dies...
	// (Store the problem in temp[i] throughout, even though its actual
	//  location changes.)
	// The rows are i, where the problem lies, and j.
         for (nat32 j=i+1;true;j++)
         {
          // Calculate the givens rotation to wipe out the zero in the current
          // location and move it to the next...
           typename MT1::type c;
           typename MT1::type s;
           if (math::IsZero(temp[i]))
           {
	    c = static_cast<typename MT1::type>(1);
	    s = static_cast<typename MT1::type>(0);
           }
           else
           {
	    if (math::Abs(temp[i])>math::Abs(d[j]))
	    {
             typename MT1::type r = -d[j]/temp[i];
             s = math::InvSqrt(static_cast<typename MT1::type>(1) + math::Sqr(r));
             c = s*r;
	    }
	    else
	    {
             typename MT1::type r = -temp[i]/d[j];
             c = math::InvSqrt(static_cast<typename MT1::type>(1) + math::Sqr(r));
             s = c*r;
	    }
           }

          // Apply the givens rotation - the problem will move left one place,
          // unless we are at the edge of the matrix where it will vanish...
           // Store the transform in the u matrix...
            for (nat32 k=0;k<u.Rows();k++)
            {
	     typename MT1::type t1 = u[k][j];
	     typename MT1::type t2 = u[k][i];
	     u[k][j] = c*t1 - s*t2;
	     u[k][i] = s*t1 + c*t2;
            }

           // Do the zeroing affect...
            d[j] = c*d[j] - s*temp[i];

           // Calculate the new problem, if needed, else break...
            if (j==(d.Size()-1-q))
            {
             temp[i] = static_cast<typename MT1::type>(0);
             break;
            }

            temp[i]  = s*temp[j];
            temp[j] *= c;
         }
       }
     }
    }

    if (zeroDiag)
    {
     p = d.Size();
     continue;
    }
   }



   // Do a Golub-Kahan step on the selected sub-matrix...
   // (b[p][p]..b[d.Size()-1-q][d.Size()-1-q])
   {
    //LogTime("eos::math::SVT step");
    int32 n = d.Size()-p-q; // Size of sub-matrix.

    // Find the two eigenvalues of the 2x2 matrix at the bottom corner of B^T B...
     // Special case for B being 2x2...
     typename MT1::type fAbove;
     if (n>2) fAbove = temp[n+p-3];
         else fAbove = static_cast<typename MT1::type>(0);

     // Matrix we are calculating for...
      typename MT1::type mult = d[n+p-2] * d[n+p-1];
      typename MT1::type s11 = d[n+p-2]/d[n+p-1] + fAbove*(fAbove/mult);
      typename MT1::type s12 = temp[n+p-2]/d[n+p-1];
      typename MT1::type s22 = d[n+p-1]/d[n+p-2] + temp[n+p-2]*(temp[n+p-2]/mult);

     // Calculation, selection of eigenvalue closest to s22...
      typename MT1::type qu = static_cast<typename MT1::type>(0.5) * (s11+s22);
      typename MT1::type qv = static_cast<typename MT1::type>(0.5) *
                              math::Sqrt(math::Sqr(s11-s22) +
                                         static_cast<typename MT1::type>(4)*math::Sqr(s12));
      typename MT1::type ev;
      if (qu<s22) ev = qu + qv;
             else ev = qu - qv;
      ev *= mult;


    // Loop, chasing the created problem value to oblivion at the bottom corner
    // of the matrix...
     typename MT1::type y = math::Sqr(d[p]) - ev;
     typename MT1::type z = d[p] * temp[p]; // Contains the problem values of the loop, i.e. the single value thats outside the storage.
     for (int32 k=p;true;k++)
     {
      // Givens rotation to be fed to the v matrix...
       // Calculate the rotation...
        typename MT1::type c;
        typename MT1::type s;
        if (math::IsZero(z))
        {
	 c = static_cast<typename MT1::type>(1);
	 s = static_cast<typename MT1::type>(0);
        }
        else
        {
         if (math::Abs(z)>math::Abs(y))
         {
	  typename MT1::type r = -y/z;
	  s = math::InvSqrt(static_cast<typename MT1::type>(1) + math::Sqr(r));
	  c = s * r;
         }
         else
         {
	  typename MT1::type r = -z/y;
	  c = math::InvSqrt(static_cast<typename MT1::type>(1) + math::Sqr(r));
	  s = c * r;
         }
        }

       // Update the v matrix...
        for (nat32 j=0;j<v.Rows();j++)
        {
	 typename MT1::type t1 = v[j][k];
	 typename MT1::type t2 = v[j][k+1];
	 v[j][k]   = c*t1 - s*t2;
	 v[j][k+1] = s*t1 + c*t2;
        }

       // Update the b matrix (Including y and z)...
        // Remove past problem (If it exists)...
         if (p!=k) temp[k-1] = c*temp[k-1] - s*z;

        // Update within storage...
         y = c*d[k] - s*temp[k];
         temp[k] = s*d[k] + c*temp[k];
         d[k] = y;

        // Create new problem...
         z = -s*d[k+1];
         d[k+1] *= c;


      // Givens rotation to be fed to the u matrix...
       // Calculate the rotation...
        if (math::IsZero(z))
        {
	 c = static_cast<typename MT1::type>(1);
	 s = static_cast<typename MT1::type>(0);
        }
        else
        {
         if (math::Abs(z)>math::Abs(y))
         {
	  typename MT1::type r = -y/z;
	  s = math::InvSqrt(static_cast<typename MT1::type>(1) + math::Sqr(r));
	  c = s * r;
         }
         else
         {
	  typename MT1::type r = -z/y;
	  c = math::InvSqrt(static_cast<typename MT1::type>(1) + math::Sqr(r));
	  s = c * r;
         }
        }

       // Update the u matrix...
        for (nat32 j=0;j<u.Rows();j++)
        {
	 typename MT1::type t1 = u[j][k];
	 typename MT1::type t2 = u[j][k+1];
	 u[j][k]   = c*t1 - s*t2;
	 u[j][k+1] = s*t1 + c*t2;
        }

       // Update the b matrix (Including y and z, when not last iteration)...
        // Remove the problem...
         d[k] = c*d[k] - s*z;

        // Update within storage...
         y = c*temp[k] - s*d[k+1];
         d[k+1] = s*temp[k] + c*d[k+1];
         temp[k] = y;

        // Create the new problem, unless we are done, in which case break...
         if (k==(p+n-2)) break;

         z = -s*temp[k+1];
         temp[k+1] *= c;
     }
   }
  }



 // Sort the diagonal of the result, and make it positive...
 // (Insertion sort, bubbling style - unfortunatly necesary to avoid a
 //  requirement for additional storage. Shouldn't be that critical anyway.)
 {
  //LogTime("eos::math::SVD sort");
  for (nat32 i=0;i<d.Size();i++)
  {
   if (d[i]<0.0)
   {
    d[i] = -d[i];
    for (nat32 r=0;r<v.Rows();r++) v[r][i] = -v[r][i];
   }
   for (nat32 j=i;(j>0)&&(d[j]>d[j-1]);j--)
   {
    math::Swap(d[j],d[j-1]);
    u.SwapCols(j,j-1);
    v.SwapCols(j,j-1);
   }
  }
 }


 return all_good;
}

//------------------------------------------------------------------------------
/// Pseudo Inverse, this will invert all matrices, even non-square and singular
/// once, uses SVD to accheive this goal.
/// Note that whilst it should allways work failure is possible, so it still
/// returns true on success, false on failure.
/// (False should be extremelly rare however.)
/// This only works for known-sized matrices, i.e. class Mat, not class Matrix.
///
/// Current implimentation is far more inefficient than would be liked, concidering
/// its use in bundle adjustment optimising this is a serious priority.
///
/// \param in A mxn matrix.
/// \param out A nxm matrix.
template <nat32 ROWS,nat32 COLS,typename T>
inline bit PseudoInverse(const math::Mat<ROWS,COLS,T> & in,math::Mat<COLS,ROWS,T> & out)
{
 math::Mat<ROWS,COLS,T> u = in;
 math::Vect<COLS,T> d;
 math::Mat<COLS,COLS,T> v;
 math::Vect<COLS,T> t;

 if (SVD(u,d,v,t)==false) return false;

 for (nat32 i=0;i<COLS;i++)
 {
  if (!math::IsZero(d[i])) d[i] = 1.0/d[i];
                      else d[i] = 0.0;
 }

 math::Mat<COLS,ROWS,T> temp;

 Transpose(u,out);
 DiagMult(d,out,temp);
 Mult(v,temp,out);

 return true;
}

/// Psuedo inverse for square matrices of a known size at compile-time.
template <nat32 SIZE,typename T>
inline bit PseudoInverse(math::Mat<SIZE,SIZE,T> & mat)
{
 math::Vect<SIZE,T> d;
 math::Mat<SIZE,SIZE,T> v;
 math::Vect<SIZE,T> t;

 if (SVD(mat,d,v,t)==false) return false;

 for (nat32 i=0;i<SIZE;i++)
 {
  if (!math::IsZero(d[i])) d[i] = 1.0/d[i];
                      else d[i] = 0.0;
 }

 math::Mat<SIZE,SIZE,T> temp;

 Transpose(mat);
 DiagMult(d,mat,temp);
 Mult(v,temp,mat);

 return true;
}

/// Structure passed into the PsuedoInverse for variable sized matrices.
/// Must be constructed with the relevant dimensions of the input matrix.
template <typename T>
class EOS_CLASS PseudoInverseTemp
{
 public:
  /// &nbsp;
   PseudoInverseTemp(nat32 rows,nat32 cols)
   :u(rows,cols),d(cols),v(cols,cols),t(cols),temp(cols,rows)
   {}

  /// &nbsp;
   PseudoInverseTemp(const math::Matrix<T> & mat)
   :u(mat.Rows(),mat.Cols()),d(mat.Cols()),v(mat.Cols(),mat.Cols),t(mat.Cols),temp(mat.Cols,mat.Rows)
   {}

  math::Matrix<T> u;
  math::Vector<T> d;
  math::Matrix<T> v;
  math::Vector<T> t;
  math::Matrix<T> temp;
};

/// Psuedo inverse for run-time sized matrices.
template <typename T>
inline bit PseudoInverse(const math::Matrix<T> & in,math::Matrix<T> & out,PseudoInverseTemp<T> & temp)
{
 log::Assert((in.Rows()==out.Cols())&&(in.Cols()==out.Rows())&&
             (temp.u.Rows()==in.Rows())&&(temp.u.Cols()==in.Cols()),"math::PseudoInverse 2");

 temp.u = in;
 if (SVD(temp.u,temp.d,temp.v,temp.t)==false) return false;

 for (nat32 i=0;i<temp.d.Size();i++)
 {
  if (!math::IsZero(temp.d[i])) temp.d[i] = 1.0/temp.d[i];
                           else temp.d[i] = 0.0;
 }

 Transpose(temp.u,out);
 DiagMult(temp.d,out,temp.temp);
 Mult(temp.v,temp.temp,out);

 return true;
}

/// Psuedo inverse for run-time sized square matrices.
template <typename T>
inline bit PseudoInverse(math::Matrix<T> & mat,PseudoInverseTemp<T> & temp)
{
 log::Assert((mat.Rows()==mat.Cols())&&(temp.u.Rows()==mat.Rows())&&(temp.u.Cols()==mat.Cols()),"math::PseudoInverse 1");

 if (SVD(mat,temp.d,temp.v,temp.t)==false) return false;

 for (nat32 i=0;i<temp.d.Size();i++)
 {
  if (!math::IsZero(temp.d[i])) temp.d[i] = 1.0/temp.d[i];
                           else temp.d[i] = 0.0;
 }

 Transpose(mat);
 DiagMult(temp.d,mat,temp.temp);
 Mult(temp.v,temp.temp,mat);

 return true;
}

//------------------------------------------------------------------------------
/// This solves a very common scenario, it calculates the right null space of
/// a matrix of variable size but with a known number of columns at compile time.
/// This happens to be useful for solving a very large number of problems.
///
/// Current implimentation is very slow, could be and will be optimised to hell
/// and back.
///
/// \param a A matrix with a known number of columns and the number of rows
/// greater than or equal the number of columns. Is destroyed by the operation.
/// \param ns A fixed-sized Vect, with size equal to the # of columns for a.
/// \returns true on success, false on failure. Failure will be extremely rare.
template <typename MT,nat32 COLS>
inline bit RightNullSpace(MT & a,Vect<COLS,typename MT::type> & ns)
{
 log::Assert(a.Cols()==COLS,"math::RightNullSpace");
 Mat<COLS,COLS,typename MT::type> v;
 Vect<COLS-1,typename MT::type> t;
 if (SVD(a,ns,v,t)==false) return false;

 for (nat32 i=0;i<COLS;i++) ns[i] = v[i][COLS-1];

 return true;
}

//------------------------------------------------------------------------------
/// An obscure one this, given a square matrix it replaces it with the best fit
/// rotation matrix so as to minimise the frobnius norm between the input and
/// output. An SVD based calcaulation.
template <nat32 SIZE,typename T>
inline void BestRot(Mat<SIZE,SIZE,T> & rot)
{
 Mat<SIZE,SIZE,T> u = rot;
 Vect<SIZE,T> d;
 Mat<SIZE,SIZE,T> v;
 Vect<SIZE,T> temp;
 SVD(u,d,v,temp);

 MultTrans(u,v,rot);
}

//------------------------------------------------------------------------------
 };
};
#endif
