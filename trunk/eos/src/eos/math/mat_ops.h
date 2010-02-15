#ifndef EOS_MATH_MAT_OPS_H
#define EOS_MATH_MAT_OPS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file mat_ops.h
/// Provides all the matrix and vector functions, the real functionality for
/// these classes. Designed so you can mix and match Matrix classes. A rather
/// eclectic mix as it only contains methods that have been required at some
/// point or another. Most of this has been implimented by reading Matrix
/// Computations by Golub & Loan, 3rd edition.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/math/functions.h"
#include "eos/math/vectors.h"
#include "eos/math/matrices.h"
#include "eos/mem/functions.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
/// Sets a sub matrix, given matrix B sets all entrys in A to the values in B,
/// starting from the given coordinates. The entire range of B is iterated, the
/// coordinates are offsets into A.
template <typename MA,typename MB>
inline void SetSub(MA & a,const MB & b,nat32 sr,nat32 sc)
{
 log::Assert(((sr+b.Rows())<=a.Rows())&&((sc+b.Cols())<=a.Cols()),"math::SetSub");

 for (nat32 r=0;r<b.Rows();r++)
 {
  for (nat32 c=0;c<b.Cols();c++)
  {
   a[r+sr][c+sc] = b[r][c];
  }
 }
}

/// Sets a sub matrix, given matrix B sets all entrys in A to the values in B,
/// starting from the given coordinates. The entire range of A is iterated, the
/// coordinates are offsets into B.
template <typename MA,typename MB>
inline void SubSet(MA & a,const MB & b,nat32 sr,nat32 sc)
{
 log::Assert(((sr+a.Rows())<=b.Rows())&&((sc+a.Cols())<=b.Cols()),"math::SubSet");

 for (nat32 r=0;r<a.Rows();r++)
 {
  for (nat32 c=0;c<a.Cols();c++)
  {
   a[r][c] = b[r+sr][c+sc];
  }
 }
}

//------------------------------------------------------------------------------
/// Makes a zero matrix, with all entrys set to 0.
template <typename MT>
inline void Zero(MT & mat)
{
 for (nat32 r=0;r<mat.Rows();r++)
 {
  for (nat32 c=0;c<mat.Cols();c++) mat[r][c] = (typename MT::type)(0.0);
 }
}

/// Makes everything below the diagonal zero.
template <typename MT>
inline void ZeroLowerTri(MT & mat)
{
 for (nat32 r=1;r<mat.Rows();r++)
 {
  for (nat32 c=0;c<r;c++) mat[r][c] = (typename MT::type)(0.0);
 }
}

/// Makes everything above the diagonal zero.
template <typename MT>
inline void ZeroUpperTri(MT & mat)
{
 for (nat32 r=0;r<mat.Rows();r++)
 {
  for (nat32 c=r+1;c<mat.Cols();c++) mat[r][c] = (typename MT::type)(0.0);
 }
}

/// Zeroes the diagonal.
template <typename MT>
inline void ZeroDiagonal(MT & mat)
{
 for (nat32 i=0;i<math::Min(mat.Rows(),mat.Cols());i++)
 {
  mat[i][i] = (typename MT::type)(0.0);
 }
}

/// Make an identity matrix, with all entrys 0, except for the diagonal which is set to 1.
template <typename MT>
inline void Identity(MT & mat)
{
 Zero(mat);
 nat32 t = math::Min(mat.Rows(),mat.Cols());
 for (nat32 i=0;i<t;i++) mat[i][i] = (typename MT::type)(1.0);
}

/// Converts a diagonal matrix represented by a vector to an actual matrix.
/// The sizes must be correct on pass in.
template <typename VT, typename MT>
inline void Diag(const VT & vect,MT & mat)
{
 log::Assert((vect.Size()==mat.Rows())&&(vect.Size()==mat.Cols()),"math::Diag");
 Zero(mat);
 for (nat32 i=0;i<vect.Size();i++) mat[i][i] = vect[i];
}

/// Given a vector of length 3 this generates its matching skew-symetric matrix.
/// Implimented as this ia quite a popular operation in CV.
template <typename T>
inline void SkewSymetric33(const Vect<3,T> & in,Mat<3,3,T> & out)
{
 out[0][0] = 0.0;    out[0][1] = -in[2]; out[0][2] = in[1];
 out[1][0] = in[2];  out[1][1] = 0.0;    out[1][2] = -in[0];
 out[2][0] = -in[1]; out[2][1] = in[0];  out[2][2] = 0.0;
}

/// Returns the trace of a matrix, simply the sum of its diagonal components.
template <typename MT>
inline typename MT::type Trace(MT & mat)
{
 nat32 n = math::Min(mat.Rows(),mat.Cols());

 typename MT::type ret = (typename MT::type)0.0;
 for (nat32 i=0;i<n;i++) ret += mat[i][i];
 return ret;
}

/// Generates a Givens rotation matrix, in the plane given by the two components
/// passed in and for the passed in angle.
/// To generate a standard counter-clockwise 2-d matrix then (i,j) = (0,1).
/// If dealing with 3 dimensions then for a counter-clockwise rotation arround
/// the x axis its (2,1), for the y axis its (0,2) and for the z axis its (1,0).
/// (Observer on the axis > 0 side of plane, looking at origin, right handed.)
/// For a clockwise version of any rotation swap the i and j component.
template <typename MT>
inline void Rotation(nat32 i,nat32 j,typename MT::type angle, MT & out)
{
 log::Assert(out.Rows()==out.Cols(),"math::Rotation");
 Identity(out);

 typename MT::type c = Cos(angle);
 typename MT::type s = Sin(angle);

 out[i][i] = c;  out[i][j] = s;
 out[j][i] = -s; out[j][j] = c;
}

/// Creates a Householder Reflection matrix from a Householder vector, this is
/// a matrix that will geometrically mirror any vector it pre-multiplies by the
/// hyperplane through the origin defined as being perpendicular to the given
/// householder vector. If you calculate the householder vector as x + x.Length*e
/// then the result of the householder matrix produced multiplied by x will
/// produce a vector that is equal to -x.Length*e, where e is a vector that starts
/// with 1 and is then filled in for the rest with 0's. (First column of an identity.)
/// (You can swap the + and -, but it produces a singularity where x[0] == x.Length.)
template <typename VT,typename MT>
inline void Reflection(const VT & vect,MT & mat)
{
 log::Assert((vect.Size()==mat.Rows())&&(vect.Size()==mat.Cols()),"math::Reflection");
 typename VT::type mult = (typename VT::type)0.0;
 for (nat32 i=0;i<vect.Size();i++) mult += Sqr(vect[i]);
 mult = ((typename VT::type)2.0)/mult;

 for (nat32 r=0;r<mat.Rows();r++)
 {
  for (nat32 c=0;c<mat.Cols();c++) mat[r][c] = -mult*vect[r]*vect[c];
 }

 for (nat32 i=0;i<vect.Size();i++) mat[i][i] += (typename VT::type)1.0;
}

/// Given a 3-vector this outputs 2 arbitary 3-vectors so that all 3 vectors are
/// perpendicular.
/// Obviously there are many answers, this just gives an arbitary one.
template <typename T>
inline void FinishCordSys(const Vect<3,T> & in,Vect<3,T> & out1,Vect<3,T> & out2)
{
 // Set out1 such that it can't be parallel to in...
  out1[0] = in[2];
  out1[1] = in[0];
  out1[2] = in[1];
  if (out1[0]>out1[1])
  {
   if (out1[2]>out1[0]) out1[2] = -out1[2];
                   else out1[0] = -out1[0];
  }
  else
  {
   if (out1[2]>out1[1]) out1[2] = -out1[2];
                   else out1[1] = -out1[1];
  }


 // Set out2 to the cross product of in and out1, then out to the cross product
 // of the other two...
  CrossProduct(in,out1,out2);
  CrossProduct(out2,in,out1);
}

/// Given a 3-vector orientation and a 4-vector homogenous position this outputs
/// a homogenous plane accordingly.
template <typename T>
inline void MakePlane(const Vect<3,T> & norm,const Vect<4,T> & pos,Vect<4,T> & out)
{
 out[0] = norm[0] * pos[3];
 out[1] = norm[1] * pos[3];
 out[2] = norm[2] * pos[3];
 out[3] = -(norm[0]*pos[0] + norm[1]*pos[1] + norm[2]*pos[2]);
}

//------------------------------------------------------------------------------
/// Converts a normal plucker matrix to its dual plucker matrix, or a dual plucker
/// matrix to its normal plucker matrix. The operation is its own inverse.
/// An in-place operation.
template <typename T>
inline void PluckerConvert(Mat<4,4,T> & plucker)
{
 math::Swap(plucker[0][1],plucker[2][3]);
 math::Swap(plucker[0][2],plucker[1][3]);
 math::Swap(plucker[0][3],plucker[1][2]);

 math::Swap(plucker[1][0],plucker[3][2]);
 math::Swap(plucker[2][0],plucker[3][1]);
 math::Swap(plucker[3][0],plucker[2][1]);
}

/// Creates a Plucker matrix from 2 homogenous coordinates, i.e. a representation
/// of a line in 3D.
template <typename T>
inline void Plucker(const Vect<4,T> & a,const Vect<4,T> & b,Mat<4,4,T> & out)
{
 for (nat32 r=0;r<4;r++)
 {
  for (nat32 c=0;c<4;c++)
  {
   out[r][c] = a[r]*b[c] - b[r]*a[c];
  }
 }
}

/// Create a plucker matrix from a homogenous coordinate and a normal for
/// orientation. Produces a zero matrix if the coordinate is at infinity.
template <typename T>
inline void Plucker(const Vect<4,T> & pos,const Vect<3,T> & norm,Mat<4,4,T> & out)
{
 // Create two planes, such that they contain the line...
  Vect<3,T> tan[2];
  FinishCordSys(norm,tan[0],tan[1]);

  Vect<4,T> plane[2];
  MakePlane(tan[0],pos,plane[0]);
  MakePlane(tan[1],pos,plane[1]);

 // Create a dual plucker matrix from the two planes...
  Plucker(plane[0],plane[1],out);

 // Convert from a dual to normal plucker...
  PluckerConvert(out);
}

/// Outputs the direction of travel of a Plucker matrix, not normalised.
/// The direction returned is arbitary - can go in either direction.
template <typename T>
inline void PluckerDir(const Mat<4,4,T> & plucker,Vect<3,T> & out)
{
 // First calculate the SVD of the dual of the given plucker...
  Mat<4,4,T> u = plucker;
  PluckerConvert(u);
  Vect<4,T> d;
  Mat<4,4,T> v;
  Vect<4,T> tempVect;
  SVD(u,d,v,tempVect);


 // We now know that the two right null space vectors form a basis of the 2D
 // space of left null vectors, this space spaning all points on the line, get
 // algebraic and find two points with w=1, tkae the diffference and we have a
 // direction of travel...
  T d2;
  T d3;
  if (IsZero(u[3][2]))
  {
   d2 = static_cast<T>(1);
   d3 = 0.0;
  }
  else
  {
   if (IsZero(u[3][3]))
   {
    d2 = 0.0;
    d3 = static_cast<T>(1);
   }
   else
   {
    d2 = static_cast<T>(1)/u[3][2];
    d3 = static_cast<T>(-1)/u[3][3];
   }
  }

 out[0] = d2*u[0][2] + d3*u[0][3];
 out[1] = d2*u[1][2] + d3*u[1][3];
 out[2] = d2*u[2][2] + d3*u[2][3];
}

/// Given a plucker matrix and a homogenous coordinate this outputs the
/// homogenous coordinate as projected onto the line, i.e. the point on the line
/// closest to the given point.
template <typename T>
inline void PluckerProj(const Mat<4,4,T> & plucker,const Vect<4,T> & pos,Vect<4,T> & out)
{
 // Create a plane passing through pos thats perpendicular to the line...
  Vect<3,T> norm;
  PluckerDir(plucker,norm);
  Vect<4,T> plane;
  MakePlane(norm,pos,plane);

 // Intercept the plane with the line to get our point...
  MultVect(plucker,plane,out);
}

//------------------------------------------------------------------------------
/// Calculates the cross product of two vectors of size 3.
template <typename T>
inline void CrossProduct(const Vect<3,T> & a,const Vect<3,T> & b,Vect<3,T> & out)
{
 out[0] = a[1]*b[2] - a[2]*b[1];
 out[1] = a[2]*b[0] - a[0]*b[2];
 out[2] = a[0]*b[1] - a[1]*b[0];
}

/// Returns the squared length of the cross product of two vectors of size 3.
template <typename T>
inline T CrossProductLengthSqr(const Vect<3,T> & a,const Vect<3,T> & b)
{
 return math::Sqr(a[1]*b[2] - a[2]*b[1]) +
        math::Sqr(a[2]*b[0] - a[0]*b[2]) +
        math::Sqr(a[0]*b[1] - a[1]*b[0]);
}

/// Returns the length of the cross product of two vectors of size 3.
template <typename T>
inline T CrossProductLength(const Vect<3,T> & a,const Vect<3,T> & b)
{
 return math::Sqrt(math::Sqr(a[1]*b[2] - a[2]*b[1]) +
                   math::Sqr(a[2]*b[0] - a[0]*b[2]) +
                   math::Sqr(a[0]*b[1] - a[1]*b[0]));
}

/// Outputs an arbitary vector that is perpendicular to the given vector.
template <typename T>
inline void Perpendicular(const Vect<3,T> & in,Vect<3,T> & out)
{
 nat32 low;
 if (Abs(in[0])<Abs(in[1])) low = 0;
                       else low = 1;
 if (Abs(in[2])<Abs(in[low])) low = 2;
 
 Vect<3,T> temp(0.0);
 temp[low] = 1.0;
 CrossProduct(in,temp,out);
}

/// Given a square matrix sets it to its own transpose.
template <typename MT>
inline void Transpose(MT & m)
{
 log::Assert(m.Rows()==m.Cols(),"math::Transpose 1");
 for (nat32 r=1;r<m.Rows();r++)
 {
  for (nat32 c=0;c<r;c++)
  {
   typename MT::type temp = m[r][c];
   m[r][c] = m[c][r];
   m[c][r] = temp;
  }
 }
}

/// Transpose for non-square matrices, or simply for when you don't want
/// in-place behaviour.
template <typename MA,typename MB>
inline void Transpose(const MA & in,MB & out)
{
 log::Assert((in.Rows()==out.Cols())&&(in.Cols()==out.Rows()),"math::Transpose 2");
 for (nat32 c=0;c<out.Cols();c++)
 {
  for (nat32 r=0;r<out.Rows();r++)
  {
   out[r][c] = in[c][r];
  }
 }
}

//------------------------------------------------------------------------------
/// Simply assigns one matrix to another. This is easier and fits the pattern
/// than writting all the relevant copy constructors and operator ='s, as all
/// other methods here will work with *any* class that fits the matrix
/// implimentation pattern, regardless of actual type.
template <typename MA,typename MB>
inline void Assign(MA & out,const MB & in)
{
 log::Assert((out.Cols()==in.Cols())&&(out.Rows()==in.Rows()),"math::Assign");
 for (nat32 r=0;r<out.Rows();r++)
 {
  for (nat32 c=0;c<out.Cols();c++) out[r][c] = in[r][c];
 }
}

/// Simply assigns one vector to another.
template <typename VA,typename VB>
inline void AssignVect(VA & out,const VB & in)
{
 log::Assert(out.Size()==in.Size(),"math::AssignVect");
 for (nat32 i=0;i<out.Size();i++) out[i] = in[i];
}

/// Simply assigns one vector to another, where the in vector is homogenous and
/// the out vector non-homogenous.
template <typename VA,typename VB>
inline void AssignVectRemHG(VA & out,const VB & in)
{
 log::Assert((out.Size()+1)==in.Size(),"math::AssignVectRemHG");
 typename VA::type mult = static_cast<typename VA::type>(1)/in[out.Size()];
 for (nat32 i=0;i<out.Size();i++) out[i] = in[i]*mult;
}

/// Simply assigns one vector to another, where the in vector is non-homogenous
/// and the out vector homogenous.
template <typename VA,typename VB>
inline void AssignVectAddHG(VA & out,const VB & in)
{
 log::Assert(out.Size()==(in.Size()+1),"math::AssignVectAddHG");
 for (nat32 i=0;i<in.Size();i++) out[i] = in[i];
 out[in.Size()] = static_cast<typename VA::type>(1);
}

//------------------------------------------------------------------------------
/// Given the equation a b^T = M calculates M, where a and b are vectors.
template <typename VA,typename VB,typename MT>
inline void VectVectTrans(const VA & a,const VB & b,MT & m)
{
 log::Assert((a.Size()==b.Size())&&(a.Size()==m.Rows())&&(m.Rows()==m.Cols()),"math::VectVectTrans");
 for (nat32 r=0;r<a.Size();r++)
 {
  for (nat32 c=0;c<b.Size();c++)
  {
   m[r][c] = a[r]*b[c];
  }
 }
}

/// Given the equation Ax=b calculates b, where x is a vector.
template <typename MA,typename VX,typename VO>
inline void MultVect(const MA & a,const VX & x,VO & out)
{
 log::Assert((a.Cols()==x.Size())&&(a.Rows()==out.Size()),"math::MultVect");
 for (nat32 r=0;r<a.Rows();r++)
 {
  out[r] = (typename MA::type)(0.0);
  for (nat32 c=0;c<a.Cols();c++) out[r] += a[r][c] * x[c];
 }
}

/// Given the equation Ax=b calculates b, where x is a vector. In addition, A is
/// 1 greater than both x and b in the relevent dimensions as in for
/// homogenous coordinates, it takes this into account, extending x with a 1 and
/// dividing through b accordingly.
template <typename MA,typename VX,typename VO>
inline void MultVectEH(const MA & a,const VX & x,VO & out)
{
 log::Assert((a.Cols()-1==x.Size())&&(a.Rows()-1==out.Size()),"math::MultVectEH");

  typename MA::type mult = a[out.Size()][x.Size()];
  for (nat32 i=0;i<x.Size();i++) mult += a[out.Size()][i]*x[i];
  mult = 1.0/mult;

  for (nat32 r=0;r<out.Size();r++)
  {
   out[r] = a[r][x.Size()];
    for (nat32 c=0;c<x.Size();c++) out[r] += a[r][c]*x[c];
   out[r] *= mult;
  }
}

/// Given the equation A^T x=b calculates b, where x is a vector.
template <typename MA,typename VX,typename VO>
inline void TransMultVect(const MA & a,const VX & x,VO & out)
{
 log::Assert((a.Rows()==x.Size())&&(a.Cols()==out.Size()),"math::TransMultVect");
 for (nat32 r=0;r<a.Cols();r++)
 {
  out[r] = (typename MA::type)(0.0);
  for (nat32 c=0;c<a.Rows();c++) out[r] += a[c][r] * x[c];
 }
}

/// Given the equation AB = D calculates D where they are all matrices.
template <typename MA,typename MB,typename MD>
inline void Mult(const MA & a,const MB & b,MD & d)
{
 log::Assert((a.Cols()==b.Rows())&&(a.Rows()==d.Rows())&&(b.Cols()==d.Cols()),"math::Mult");
 for (nat32 r=0;r<d.Rows();r++)
 {
  for (nat32 c=0;c<d.Cols();c++)
  {
   typename MD::type sum = (typename MD::type)0.0;
   for (nat32 i=0;i<a.Cols();i++) sum += a[r][i]*b[i][c];
   d[r][c] = sum;
  }
 }
}

/// Given the equation A^T B = D calculates D where they are all matrices.
template <typename MA,typename MB,typename MD>
inline void TransMult(const MA & a,const MB & b,MD & d)
{
 log::Assert((a.Rows()==b.Rows())&&(a.Cols()==d.Rows())&&(b.Cols()==d.Cols()),"math::TransMult");
 for (nat32 r=0;r<d.Rows();r++)
 {
  for (nat32 c=0;c<d.Cols();c++)
  {
   typename MD::type sum = (typename MD::type)0.0;
   for (nat32 i=0;i<a.Rows();i++) sum += a[i][r]*b[i][c];
   d[r][c] = sum;
  }
 }
}

/// Given the equation A B^T = D calculates D where they are all matrices.
template <typename MA,typename MB,typename MD>
inline void MultTrans(const MA & a,const MB & b,MD & d)
{
 log::Assert((a.Cols()==b.Cols())&&(a.Rows()==d.Rows())&&(b.Rows()==d.Cols()),"math::MultTrans");
 for (nat32 r=0;r<d.Rows();r++)
 {
  for (nat32 c=0;c<d.Cols();c++)
  {
   typename MD::type sum = (typename MD::type)0.0;
   for (nat32 i=0;i<a.Cols();i++) sum += a[r][i]*b[c][i];
   d[r][c] = sum;
  }
 }
}

/// Given the equation a^T * B * d this calculates and returns the scalar.
/// Often used with a and d identical.
template <typename MA,typename MB,typename MD>
inline typename MA::type VectMultVect(const MA & a,const MB & b,MD & d)
{
 log::Assert((a.Size()==b.Rows())&&(b.Cols()==d.Size()),"math::VectMultVect");
 typename MA::type ret = (typename MA::type)(0.0);
  for (nat32 c=0;c<d.Size();c++)
  {
   typename MA::type is = (typename MA::type)(0.0);
    for (nat32 r=0;r<a.Size();r++) is += a[r]*b[r][c];
   ret += is*d[c];
  }
 return ret;
}

/// Given the equation A * Diag(b) = D calculates D, where b is a vector
/// representing a diagonal matrix.
template<typename MA,typename VB>
inline void MultDiag(const MA & a,const VB & b,MA & d)
{
 log::Assert((a.Rows()==d.Rows())&&(b.Size()==a.Cols())&&(b.Size()==d.Cols()),"math::MultDiag");
 for (nat32 r=0;r<d.Rows();r++)
 {
  for (nat32 c=0;c<d.Cols();c++)
  {
   d[r][c] = a[r][c] * b[c];
  }
 }
}

/// Given the equation Diag(b) * A = D calculates D, where b is a vector
/// representing a diagonal matrix.
template<typename MA,typename VB>
inline void DiagMult(const VB & b,const MA & a,MA & d)
{
 log::Assert((b.Size()==a.Rows())&&(a.Rows()==d.Rows())&&(a.Cols()==d.Cols()),"math::DiagMult");
 for (nat32 r=0;r<d.Rows();r++)
 {
  for (nat32 c=0;c<d.Cols();c++)
  {
   d[r][c] = a[r][c] * b[r];
  }
 }
}

//------------------------------------------------------------------------------
/// Returns the trace for the given matrix.
template <typename MT>
inline typename MT::type Trace(const MT & mat)
{
 typename MT::type ret = (typename MT::type)(0.0);
 nat32 mi = math::Max(mat.Rows(),mat.Cols());
 for (nat32 i=0;i<mi;i++) ret += mat[i][i];
 return ret;
}

/// Returns the Frobenius for the given matrix.
template <typename MT>
inline typename MT::type FrobNorm(const MT & mat)
{
 typename MT::type ret = (typename MT::type)(0.0);
 for (nat32 r=0;r<mat.Rows();r++)
 {
  for (nat32 c=0;c<mat.Cols();c++) ret += Sqr(mat[r][c]);
 }
 return Sqrt(ret);
}

/// Returns the Frobenius norm for the difference between two matrices.
template <typename MT>
inline typename MT::type FrobNormDiff(const MT & a,const MT & b)
{
 typename MT::type ret = (typename MT::type)(0.0);
 for (nat32 r=0;r<a.Rows();r++)
 {
  for (nat32 c=0;c<a.Cols();c++) ret += Sqr(a[r][c] - b[r][c]);
 }
 return Sqrt(ret);
}

/// Returns the Frobenius norm for the difference between two matrices.
/// This version considers that the sign of the matrices won't actually match.
template <typename MT>
inline typename MT::type FrobNormDiffSign(const MT & a,const MT & b)
{
 typename MT::type retA = (typename MT::type)(0.0);
 typename MT::type retB = (typename MT::type)(0.0);
 for (nat32 r=0;r<a.Rows();r++)
 {
  for (nat32 c=0;c<a.Cols();c++)
  {
   retA += Sqr(a[r][c] - b[r][c]);
   retB += Sqr(a[r][c] + b[r][c]);
  }
 }
 return Sqrt(math::Min(retA,retB));
}

//------------------------------------------------------------------------------
// Used by the below to calculate the determinant, uses knoledge of the internal
// structure of the class Mat. Its also rather convoluted, and can not be held
// responsable for any headaches it might produce.
template <typename T>
inline T SubDeterminant(T * pos,nat32 size,nat32 stride)
{
 if (size==2) return pos[0]*pos[stride+1] - pos[1]*pos[stride];
 else
 {
  T ret = 0.0;
   for (nat32 i=0;i<size;i++)
   {
    if (i!=0) mem::Swap(&pos[0],&pos[stride*i],size-1);

    T sub = SubDeterminant(&pos[stride],size-1,stride)*pos[stride*i + size-1];
    if ((i+size)%2) ret += sub;
               else ret -= sub;
   }

   for (nat32 i=1;i<size;i++)
   {
    mem::Swap(&pos[(i-1)*stride],&pos[i*stride],size-1);
   }
  return ret;
 }
}

/// Calculates and returns the determinant of the given square matrix.
template <nat32 S,typename T>
inline T Determinant(const Mat<S,S,T> & mat)
{
 return SubDeterminant(const_cast<T*>(&mat[0][0]),S,S);
}

/// Calculates and returns the determinant of the given square matrix.
template <typename T>
inline T Determinant(const Matrix<T> & mat)
{
 log::Assert(mat.Rows()==mat.Cols());
 return SubDeterminant(const_cast<T*>(&mat[0][0]),mat.Rows(),mat.Cols());
}

//------------------------------------------------------------------------------
/// Solves the classical Ax = b for x where A is a lower triangular matrix.
/// Overwrites b with x. A must be a square matrix, x and b are vectors.
/// \param a A lower triangular square matrix of size m x m. All sizing is done from the vector, so can be larger, extra space will be ignored.
/// \param b A length m vector, to be overwritten with x.
template <typename AT,typename BT>
inline void SolveLinearLowerTri(const AT & a,BT & b)
{
 log::Assert((a.Cols()==a.Rows())&&(b.Size()==a.Cols()),"math::SolveLinearLowerTri");
 b[0] = b[0]/a[0][0];
 for (nat32 i=1;i<b.Size();i++)
 {
  typename BT::type t = 0.0;
  for (nat32 j=0;j<i;j++) t += a[i][j] * b[j];
  b[i] = (b[i] - t)/a[i][i];
 }
}


/// Solves the classical Ax = b for x where A is an upper triangular matrix.
/// Overwrites b with x. A must be a square matrix, x and b are vectors.
/// \param a An upper triangular square matrix of size m x m. All sizing is done from the vector, so can be larger, extra space will be ignored.
/// \param b A length m vector, to be overwritten with x.
template <typename AT,typename BT>
inline void SolveLinearUpperTri(const AT & a,BT & b)
{
 log::Assert((a.Cols()==a.Rows())&&(b.Size()==a.Cols()),"math::SolveLinearUpperTri");
 int32 n = b.Size();
 b[n-1] = b[n-1]/a[n-1][n-1];
 for (int32 i=n-2;i>=0;i--)
 {
  typename BT::type t = 0.0;
  for (int32 j=i+1;j<n;j++) t += a[i][j] * b[j];
  b[i] = (b[i] - t)/a[i][i];
 }
}


/// This solves the classical Ax = b for x equation, overwriting b with x.
/// \param a A m by m square matrix, is destroyed by the operation, so send in a copy if you want to use a again latter.
/// \param b A length m vector, goes in as b and exits as x.
template <typename AT,typename BT>
inline void SolveLinear(AT & a,BT & b)
{
 log::Assert((a.Cols()==a.Rows())&&(b.Size()==a.Cols()),"math::SolveLinear");
 // Gauss Elimination via Partial Pivoting...
  for (nat32 i=0;i<a.Rows()-1;i++)
  {
   // Find largest value, set it to be the pivot...
    nat32 pivot = i;
    typename AT::type max = math::Abs(a[i][i]);
    for (nat32 j=i+1;j<a.Rows();j++)
    {
     if (math::Abs(a[j][i])>max)
     {
      pivot = j;
      max = math::Abs(a[j][i]);
     }
    }

   // Swap it in...
    for (nat32 j=i;j<a.Rows();j++) mem::Swap(&a[i][j],&a[pivot][j]);

   // If the pivot is not 0 do the actual calculation...
    if (!math::Equal(a[i][i],(typename AT::type)(0.0)))
    {
     for (nat32 j=i+1;j<a.Rows();j++) a[j][i] /= a[i][i];
     for (nat32 j=i+1;j<a.Rows();j++)
     {
      for (nat32 k=i+1;k<a.Rows();k++) a[j][k] -= a[j][i]*a[i][k];
     }
    }

   // Do the relevent updates to b...
    mem::Swap(&b[i],&b[pivot]);
    for (nat32 j=i+1;j<b.Size();j++) b[j] -= b[i]*a[j][i];
  }

 // Solve the upper triangular problem we now have to get x...
  SolveLinearUpperTri(a,b);
}

//------------------------------------------------------------------------------
/// The Cholesky factorization, given a square symmetric positive definite matrix
/// this factorizes it such that A = G G^T, where G is a lower triangular matrix.
/// It overwrites the lower triangular part including the diagonal of A with the
/// result, the remainder of A is destroyed and left 'random'.
/// (If given a non-square matrix will compute the factorization assuming it is
/// square by Cols(), this allows you to use the n x n part of a m x n matrix, m>n
/// with this. Happens to be a common scenario.) (Only uses the lower triangular
/// part of the matrix, an again useful property.)
/// \returns true on success, false on failure. Failure implies that it is not
/// a positive definate matrix, though indicates nothing about its symetry.
template <typename MT>
inline bit Cholesky(MT & a)
{
 log::Assert(a.Rows()==a.Cols(),"math::Cholesky");
 if ((a[0][0]<=(typename MT::type)0.0)||(Equal(a[0][0],(typename MT::type)0.0))) return false;
 typename MT::type mult = InvSqrt(a[0][0]);
 for (nat32 i=0;i<a.Cols();i++) a[i][0] *= mult;

 for (nat32 j=1;j<a.Cols();j++)
 {
  for (nat32 i=j;i<a.Cols();i++)
  {
   typename MT::type sum = (typename MT::type)0.0;
   for (nat32 k=0;k<j;k++) sum += a[i][k]*a[j][k];
   a[i][j] -= sum;
  }

  if ((a[j][j]<=(typename MT::type)0.0)||(Equal(a[j][j],(typename MT::type)0.0))) return false;
  typename MT::type mult = InvSqrt(a[j][j]);
  for (nat32 i=j;i<a.Cols();i++) a[i][j] *= mult;
 }
 return true;
}

//------------------------------------------------------------------------------
/// This solves the overspecified Ax = b equation for x where A is a m x n
/// matrix with m>n and b is a length m vector whilst x is a length n vector.
/// Uses the Normal equations method. A least squares solution is output.
/// Requires that Rank(A) = n.
/// \param a The input matrix A.
/// \param b The input vector b.
/// \param x The output vector x.
/// \param temp Tempory storage the algorithm requires, should be a n x n matrix.
/// \returns true on success, false on failure. False indicates that not enough
/// information is avaliable or it simply can't solve it due to the limitations
/// of the algorithm.
template <typename MT,typename VT1,typename VT2,typename MT2>
inline bit SolveLeastSquares(const MT & a,const VT1 & b,VT2 & x,MT2 & temp)
{
 log::Assert((a.Cols()==x.Size())&&(a.Rows()==b.Size())&&(temp.Rows()==temp.Cols())&&(a.Cols()==temp.Cols()),"math::SolveLeastSquares");
 // Fill the lower triangular part of temp with A^T A...
  for (nat32 r=0;r<a.Cols();r++)
  {
   for (nat32 c=0;c<=r;c++)
   {
    typename MT::type sum = (typename MT::type)0.0;
     for (nat32 i=0;i<a.Rows();i++) sum += a[i][r]*a[i][c];
    temp[r][c] = sum;
   }
  }

 // Create d, to contain a^T b, put it into x...
  for (nat32 c=0;c<a.Cols();c++)
  {
   typename MT::type sum = (typename MT::type)0.0;
    for (nat32 r=0;r<a.Rows();r++) sum += b[r]*a[r][c];
   x[c] = sum;
  }

 // A Cholesky factorization...
  if (Cholesky(temp)==false) return false;

 // Solve the lower and upper triangular equations to produce the final result...
  SolveLinearLowerTri(temp,x);
   for (nat32 r=0;r<a.Cols();r++) // Specialised transpose.
   {
    for (nat32 c=0;c<=r;c++)
    {
     temp[c][r] = temp[r][c];
    }
   }
  SolveLinearUpperTri(temp,x);

 return true;
}

//------------------------------------------------------------------------------
/// A QR decomposition, A = QR where A is arbitary, Q is orthogonal and R is
/// upper triangular. Implimented using the Givens approach, its calculation of
/// Q could use better techneques.
/// \param a Input as the A matrix, output as the r matrix. mxn where m>=n.
/// \param q Output q matrix, mxm in size.
template <typename MA,typename MQ>
inline void QR(MA & a,MQ & q)
{
 log::Assert((a.Rows()>=a.Cols())&&(q.Rows()==a.Rows())&&(q.Rows()==q.Cols()),"math::QR");

 Identity(q);

 for (nat32 j=0;j<a.Cols();j++)
 {
  for (nat32 i=a.Rows()-1;i>j;i--)
  {
   // Calculate the givens rotation required to zero a victim element...
    typename MA::type c,s;
    if (math::Equal(a[i][j],(typename MA::type)(0.0)))
    {
     c = 1.0; s = 0.0;
    }
    else
    {
     if (math::Abs(a[i][j])>math::Abs(a[i-1][j]))
     {
      typename MA::type r = -a[i-1][j]/a[i][j];
      s = math::InvSqrt(1.0+math::Sqr(r));
      c = s*r;
     }
     else
     {
      typename MA::type r = -a[i][j]/a[i-1][j];
      c = math::InvSqrt(1.0+math::Sqr(r));
      s = c*r;
     }
    }

   // Apply to a to get one zero closer to r...
    for (nat32 k=j;k<a.Cols();k++)
    {
     typename MA::type a1 = a[i-1][k];
     typename MA::type a2 = a[i][k];

     a[i-1][k] = c*a1 - s*a2;
     a[i][k]   = c*a2 + s*a1;
    }

   // Update q with the new givens rotation...
    for (nat32 k=0;k<q.Rows();k++)
    {
     typename MA::type q1 = q[k][i-1];
     typename MA::type q2 = q[k][i];

     q[k][i-1] = c*q1 - s*q2;
     q[k][i]   = c*q2 + s*q1;
    }
  }
 }
}

/// A RQ decomposition, A = RQ where A is arbitary, Q is orthogonal and R is
/// upper triangular. Implimented using the Givens approach, its calculation of
/// Q could use better techneques. Simply the QR transformation the wrong way
/// round. Implimented for the sole purpose of providing a method that I will
/// never use in a camera class;-)
/// \param a Input as the A matrix, output as the r matrix. mxn where m<=n.
/// \param q Output q matrix, nxn in size.
template <typename MA,typename MQ>
inline void RQ(MA & a,MQ & q)
{
 log::Assert((a.Rows()<=a.Cols())&&(q.Cols()==a.Cols())&&(q.Rows()==q.Cols()),"math::RQ");

 Identity(q);

 for (nat32 r=a.Rows()-1;r>0;r--)
 {
  for (nat32 c=0;c<r;c++)
  {
   // Calculate the givens rotation required to zero a victim element...
    typename MA::type gc,gs;
    if (math::Equal(a[r][c],(typename MA::type)(0.0)))
    {
     gc = 1.0; gs = 0.0;
    }
    else
    {
     if (math::Abs(a[r][c])>math::Abs(a[r][c+1]))
     {
      typename MA::type rr = -a[r][c+1]/a[r][c];
      gs = math::InvSqrt(1.0+math::Sqr(rr));
      gc = gs*rr;
     }
     else
     {
      typename MA::type rr = -a[r][c]/a[r][c+1];
      gc = math::InvSqrt(1.0+math::Sqr(rr));
      gs = gc*rr;
     }
    }

   // Apply to a to get one zero closer to r...
    for (nat32 k=0;k<a.Rows();k++)
    {
     typename MA::type a1 = a[k][c+1];
     typename MA::type a2 = a[k][c];

     a[k][c+1] = gc*a1 - gs*a2;
     a[k][c]   = gc*a2 + gs*a1;
    }

   // Update q with the new givens rotation...
    for (nat32 k=0;k<q.Cols();k++)
    {
     typename MA::type q1 = q[c+1][k];
     typename MA::type q2 = q[c][k];

     q[c+1][k] = gc*q1 - gs*q2;
     q[c][k]   = gc*q2 + gs*q1;
    }
  }
 }
}


/// A thin QR decomposition,A = QR where A is a full rank matrix, Q is orthogonal
/// and R is upper triangular. Uses the modified Gram-Schmidt techneque.
/// Note that this is faster than fat techneques, so given a square matrix you
/// should use this, assuming full rank can be presumed.
/// \param a Rows()>=Cols(), as input the a matrix, as output the q matrix.
/// \param r r is the output upper triangular matrix, should be square with the
///          same dimensions equal to a.Cols(). The algorithm does not touch the
///          lower triangular part - if that needs to be null the user must provide.
template <typename MA,typename MR>
inline void ThinQR(MA & a,MR & r)
{
 log::Assert((a.Rows()>=a.Cols())&&(a.Cols()==r.Cols())&&(r.Cols()==r.Rows()),"math::ThinQR");

 for (nat32 k=0;k<r.Cols();k++)
 {
  r[k][k] = 0.0;
  for (nat32 i=0;i<a.Rows();i++) r[k][k] += math::Sqr(a[i][k]);
  r[k][k] = math::Sqrt(r[k][k]);

  typename MA::type invRD = 1.0/r[k][k];
  for (nat32 i=0;i<a.Rows();i++) a[i][k] *= invRD;

  for (nat32 j=k+1;j<r.Cols();j++)
  {
   r[k][j] = 0.0;
   for (nat32 i=0;i<a.Rows();i++) r[k][j] += a[i][k]*a[i][j];
   for (nat32 i=0;i<a.Rows();i++) a[i][j] -= a[i][k]*r[k][j];
  }
 }
}

//------------------------------------------------------------------------------
/// Inverts a square matrix, will fail on singular and very occasionally on
/// non-singular matrices, returns true on success. Uses Gauss-Jordan elimination
/// with partial pivoting.
/// \param mat The matrix to be inverted, is replaced with its inverse.
/// \param temp A tempory matrix used internally.
template <typename MT>
inline bit Inverse(MT & mat,MT & temp)
{
 log::Assert((mat.Rows()==mat.Cols())&&(temp.Rows()==temp.Cols())&&(mat.Cols()==temp.Cols()),"math::Inverse");
 Identity(temp);

 for (nat32 r=0;r<mat.Rows();r++)
 {
  // Find largest pivot and swap in, fail if best we can get is 0...
   typename MT::type max = mat[r][r];
   nat32 index = r;
   for (nat32 i=r+1;i<mat.Rows();i++)
   {
    if (math::Abs(mat[i][r])>math::Abs(max))
    {
     max = mat[i][r];
     index = i;
    }
   }
   if (index!=r) {mat.SwapRows(index,r); temp.SwapRows(index,r);}
   if (math::Equal(max,(typename MT::type)0.0)) return false;

  // Divide through the entire row...
   max = 1.0/max;
   mat[r][r] = 1.0;
   for (nat32 i=r+1;i<mat.Cols();i++) mat[r][i] *= max;
   for (nat32 i=0;i<mat.Cols();i++) temp[r][i] *= max;

  // Row subtract to generate 0's in the current column, so it matches an identity matrix...
   for (nat32 i=0;i<mat.Rows();i++)
   {
    if (i==r) continue;
    real32 factor = mat[i][r];
    mat[i][r] = 0.0;

    for (nat32 j=r+1;j<mat.Cols();j++) mat[i][j] -= factor * mat[r][j];
    for (nat32 j=0;j<mat.Cols();j++) temp[i][j] -= factor * temp[r][j];
   }
 }

 mat = temp;
 return true;
}

//------------------------------------------------------------------------------
/// This is given a 3x3 rotation matrix, from which is calculates an angle-axis
/// representation of the rotation. This is a representation as a vector,
/// representing an axis arround which the rotation occurs with the length of
/// the axis being the radians rotation anti-clockwise. (Anti-clockwise assuming
/// the axis is pointing at you.) See AngAxisToRotMat for
/// the reverse procedure.
template <typename T>
inline void RotMatToAngAxis(Mat<3,3,T> & rm,Vect<3,T> & aa)
{
 T length = math::InvCos(0.5*(Trace(rm) - 1.0));


 if (Equal(length,T(0.0)))
 {
  aa = Vect<3,T>(0.0);
  return;
 }


 if (Equal(length,T(pi)))
 {
  if ((rm[0][0]>rm[1][1])&&(rm[0][0]>rm[2][2]))
  {
   aa[0] = math::Sqrt(rm[0][0] - rm[1][1] - rm[2][2] + 1.0);
   aa[1] = rm[0][1]/aa[0];
   aa[2] = rm[0][2]/aa[0];
  }
  else
  {
   if (rm[1][1]>rm[2][2])
   {
    aa[1] = math::Sqrt(rm[1][1] - rm[0][0] - rm[2][2] + 1.0);
    aa[0] = rm[0][1]/aa[1];
    aa[2] = rm[1][2]/aa[1];
   }
   else
   {
    aa[2] = math::Sqrt(rm[2][2] - rm[0][0] - rm[1][1] + 1.0);
    aa[0] = rm[0][2]/aa[2];
    aa[1] = rm[1][2]/aa[2];
   }
  }

  aa.Normalise();
  aa *= length;
  return;
 }


 aa[0] = rm[2][1] - rm[1][2];
 aa[1] = rm[0][2] - rm[2][0];
 aa[2] = rm[1][0] - rm[0][1];

 aa.Normalise();
 aa *= length;
}

/// This converts an angle-axis to a rotation matrix. Partner in crime to
/// RotMatToAngAxis.
template <typename T>
inline void AngAxisToRotMat(Vect<3,T> & aa,Mat<3,3,T> & rm)
{
 T ang = aa.Length();
 T sA = math::Sin(ang);
 T cA = math::Cos(ang);
 T cAm = 1.0 - cA;

 if (math::Equal(ang,T(0.0))) {Identity(rm); return;}

 ang = 1.0/ang;
 T wx = aa[0]*ang;
 T wy = aa[1]*ang;
 T wz = aa[2]*ang;

 rm[0][0] = cA + wx*wx*cAm;     rm[0][1] = wx*wy*cAm - wz*sA; rm[0][2] = wy*sA + wx*wz*cAm;
 rm[1][0] = wz*sA + wx*wy*cAm;  rm[1][1] = cA + wy*wy*cAm;    rm[1][2] = -wx*sA + wy*wz*cAm;
 rm[2][0] = -wy*sA + wx*wz*cAm; rm[2][1] = wx*sA + wy*wz*cAm; rm[2][2] = cA + wz*wz*cAm;
}

/// This converts an angle-axis to a rotation matrix. Partner in crime to
/// RotMatToAngAxis. This version is slightly different in that it takes a normalised
/// vector and an angle, instead of using the length of the vector as the angle.
template <typename T>
inline void AngAxisToRotMat(Vect<3,T> & aa,real32 ang,Mat<3,3,T> & rm)
{
 T sA = math::Sin(ang);
 T cA = math::Cos(ang);
 T cAm = 1.0 - cA;

 T wx = aa[0];
 T wy = aa[1];
 T wz = aa[2];

 rm[0][0] = cA + wx*wx*cAm;     rm[0][1] = wx*wy*cAm - wz*sA; rm[0][2] = wy*sA + wx*wz*cAm;
 rm[1][0] = wz*sA + wx*wy*cAm;  rm[1][1] = cA + wy*wy*cAm;    rm[1][2] = -wx*sA + wy*wz*cAm;
 rm[2][0] = -wy*sA + wx*wz*cAm; rm[2][1] = wx*sA + wy*wz*cAm; rm[2][2] = cA + wz*wz*cAm;
}

//------------------------------------------------------------------------------
/// This fits a polynomial function precisly to a set of distinct points.
/// Given a set of coordinates this finds the polynomial y = a + b*x + c*x^2 etc
/// that perfectly fits them. Given n points it outputs n polynomial terms.
/// It does this by solving the Vandermonde system, V(x)^t a = y.
/// A fairrly tight O(n^2) algorithm, quite reasonable to use on very large
/// numbers of points, though of debatable value in such situations.
/// \param x The x coordinates of the n points. They must be distinct.
/// \param y The y coordinates of the n points on input. On output the n factors
/// that define the polynomial.
template <typename VT>
void InterPoly(const VT & x,VT & y)
{
 log::Assert(x.Size()==y.Size(),"math::InterPoly");

 for (nat32 k=0;k<x.Size()-1;k++)
 {
  for (nat32 i=x.Size()-1;i>k;i--)
  {
   y[i] = (y[i] - y[i-1])/(x[i] - x[i-k-1]);
  }
 }

 for (int32 k=x.Size()-2;k>=0;k--)
 {
  for (nat32 i=k;i<x.Size()-1;i++)
  {
   y[i] -= y[i+1]*x[k];
  }
 }
}

/// This is the compliment to the InterPoly function, solving the same
/// Vandermonde system but without the transpose, i.e. it solves V(x) z = b for z.
/// \param x The vector from which the Vandermonde matrix is constructed.
/// \param b On input the vector to equate to, b, on output the answer, z.
template <typename VT>
void SolveVandermonde(const VT & x,VT & b)
{
 log::Assert(x.Size()==b.Size(),"math::SolveVandermonde");

 for (nat32 k=0;k<x.Size()-1;k++)
 {
  for (nat32 i=x.Size()-1;i>k;i--)
  {
   b[i] -= x[k]*b[i-1];
  }
 }

 for (int32 k=x.Size()-2;k>=0;k--)
 {
  for (nat32 i=k+1;k<x.Size();k++)
  {
   b[i] /= x[i] - x[i-k-1];
  }

  for (nat32 i=k;k<x.Size()-1;k++)
  {
   b[i] -= b[i+1];
  }
 }
}

//------------------------------------------------------------------------------
// Weird and highly specific matrix operations...

/// Inverts a 2x2 matrix inplace.
template <typename MT>
void Inverse22(MT & mat)
{
 real32 div = mat[0][0]*mat[1][1] - mat[0][1]*mat[1][0];

 math::Swap(mat[0][0],mat[1][1]);
 mat[0][1] = -mat[0][1];
 mat[1][0] = -mat[1][0];

 mat /= div;
}

/// Calculates the square root of a 2x2 matrix, overwrites the input with its
/// square root. Just gives one of the square roots, the 'positive' one.
/// (Though this has minimal meaning.)
template <typename MT>
void Sqrt22(MT & mat)
{
 log::Assert(mat.Rows()==2&&mat.Cols()==2,"math::Sqrt22");
 typename MT::type delta = Sqrt(Determinant(mat));
 typename MT::type trace = Trace(mat);

 mat[0][0] += delta;
 mat[1][1] += delta;

 mat *= static_cast<typename MT::type>(1)/Sqrt(trace + static_cast<typename MT::type>(2)*delta);
}

/// Given a 2x2 matrix this outputs its eigenvector as associated with its
/// largest eigenvalue. Exists primarilly for use with covariance matrices to
/// find a data sets axis of maximum variation.
/// (That is not to say it only works on symetrc matrices, it will work on any.
/// Thats just what I have implimented it for.)
template <typename MT,typename VT>
void MaxEigenVector22(MT & mat,VT & out)
{
 log::Assert(mat.Rows()==2&&mat.Cols()==2&&out.Size()==2,"math::MaxEigenVector22");

 real32 trace = math::Trace(mat);
 real32 det = math::Determinant(mat);

 real32 maxEigenValue = 0.5*trace + math::Sqrt(0.25*math::Sqr(trace) - det);

 if (!math::IsZero(mat[1][0]))
 {
  out[0] = maxEigenValue - mat[1][1];
  out[1] = mat[1][0];
 }
 else
 {
  if (!math::IsZero(mat[0][1]))
  {
   out[0] = mat[0][1];
   out[1] = maxEigenValue - mat[1][1];
  }
  else
  {
   out[0] = 1.0;
   out[1] = 0.0;
  }
 }
}

//------------------------------------------------------------------------------
 };
};
#endif

#include "eos/math/svd.h" // I have too many files that expect svd to be in this file to do otherwise.
