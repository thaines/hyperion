#ifndef EOS_FILTER_KERNEL_H
#define EOS_FILTER_KERNEL_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file kernel.h
/// Simply provides methods to apply kernells to data sets, used to impliment
/// things such as gaussian blurs.

#include "eos/types.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"
#include "eos/mem/alloc.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// This represents a kernel as an arbitary matrix, with the ability to apply 
/// it. Quite slow.
class EOS_CLASS KernelMat
{
 public:
  /// &nbsp;
   KernelMat(nat32 size = 1):half(0xFFFFFFFF),data(null<real32*>()) {SetSize(size);}

  /// &nbsp;
   KernelMat(const KernelMat & rhs):half(0xFFFFFFFF),data(null<real32*>()) {*this = rhs;}

  /// &nbsp;
   ~KernelMat() {delete[] data;}


  /// &nbsp;
   KernelMat & operator = (const KernelMat & rhs);


  /// Returns the current half size, multiply it by 2 and add 1 to get the real
  /// window size.
   nat32 HalfSize() const {return half;}
  
  /// Sets the size of the kernel, the given value is half, the actual window
  /// size is half*2 + 1.
   void SetSize(nat32 half);

  /// This allows you to read/write the value of any entry in the window. They 
  /// all default to 0 after a SetSize/construction. The range of values accepted 
  /// is [-HalfSize()..HalfSize()] in each dimension.
   real32 & Val(int32 x,int32 y);

  /// &nbsp;
   const real32 & Val(int32 x,int32 y) const {return const_cast<KernelMat*>(this)->Val(x,y);}


  /// Applys the kernel to the 2D input and writes it to the same-property output,
  /// all values within half of the edge will be set to 0. The input and output 
  /// fields must not be identical.
   void Apply(const svt::Field<real32> & in,svt::Field<real32> & out) const;
   
   
  /// Replaces the current kernel with an ideal kernel, defined as a a kernel where one
  /// half is 1 and the other 0. The angle is perpendicular to the dividing line, being
  /// on the side of the 1's. It integrates across the line taking into account the
  /// square grid sampling nature of the input.
   void MakeIdeal(real32 angle);
   
  /// Zero means the kernel.
   void ZeroMean();
   
  /// Sets the Frobenius norm of the kernel to 1.
   void FrobNorm();

   
  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::filter::KernelMat";}


 private:
  nat32 half; // The window size is half*2 + 1.
  real32 * data; // (half*2 + 1)^2 in size.
};

//------------------------------------------------------------------------------
/// This represents a kernel as two vectors, a and b, where b * a^T is the 
/// kernel itself. Does the obvious optimisation during calculation and is 
/// highly recomended for speed.
class EOS_CLASS KernelVect
{
 public:
  /// &nbsp;
   KernelVect(nat32 size = 1):half(0xFFFFFF),a(null<real32*>()),b(null<real32*>()) {SetSize(size);}

  /// &nbsp;
   KernelVect(const KernelVect & rhs):half(0xFFFFFF),a(null<real32*>()),b(null<real32*>()) {*this = rhs;}

  /// &nbsp;
   ~KernelVect() {delete[] a; delete[] b;}


 /// &nbsp;
   KernelVect & operator = (const KernelVect & rhs);


  /// Returns the current half size, multiply it by 2 and add 1 to get the real
  /// window size.
   nat32 HalfSize() const {return half;}
  
  /// Sets the size of the kernel, the given value is half, the actual window
  /// size is half*2 + 1.
   void SetSize(nat32 half);
   
   
  /// Converts this Kernel to a Matrix-based kernel, ushally results in a loss of 
  /// efficiency for all but the smallest kernels.
   void ConvertTo(KernelMat & out) const;


  /// Allows you to edit the horizontal vector, a, accepts the range
  /// [-half..half].
   real32 & ValH(int32 x) {return a[half+x];}

  /// &nbsp;
   const real32 & ValH(int32 x) const {return a[half+x];}

  /// Allows you to edit the vertical vector, b, accepts the range
  /// [-half..half].
   real32 & ValV(int32 y) {return b[half+y];}

  /// &nbsp;
   const real32 & ValV(int32 y) const {return b[half+y];}


  /// Applys the kernel to the 2D input and writes it to the same-property output,
  /// all values within half of the edge will be set to 0. The input and output 
  /// fields can be identical, as it uses an intermediate buffer to assist fast 
  /// calculation. If you set transpose true it calculates the results using the
  /// transpose.
  /// Does not handle boundary conditions, simply sets them to zero.
   void Apply(const svt::Field<real32> & in,svt::Field<real32> & out,bit transpose = false) const;
   
  /// Identical to Apply, except it uses repetition of border values to work right 
  /// upto the boundary, rather than setting it to zero.
   void ApplyRepeat(const svt::Field<real32> & in,svt::Field<real32> & out,bit transpose = false) const;
   
  /// Replaces the current kernel with a gaussian, defined by the given standard 
  /// deviation. Does not change the kernel size, thats the users job.
   void MakeGaussian(real32 sd);

   
  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::filter::KernelVect";}


 private:
  nat32 half;
  real32 * a; // half*2+1 in length.
  real32 * b; // "
};

//------------------------------------------------------------------------------
 };
};
#endif
