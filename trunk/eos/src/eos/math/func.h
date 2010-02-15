#ifndef EOS_MATH_FUNC_H
#define EOS_MATH_FUNC_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file func.h
/// Provides an object for representing basic real to real functions, such that
/// they can be saved/loaded from files and applied at will.

#include "eos/types.h"

#include "eos/str/strings.h"
#include "eos/ds/arrays.h"
#include "eos/ds/lists.h"
#include "eos/bs/dom.h"
#include "eos/math/vectors.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
/// Represents a function as an addative sequence of terms in a linked list.
/// At the moment it only suports polynomial terms.
/// Can load and save from xml files.
class EOS_CLASS Func
{
 public:
  /// Default function has nothing, i.e. will always return 0.
   Func();
   
  /// &nbsp;
   ~Func();

  
  /// The important bit, applys the function.
   real64 operator () (real64 x) const;
   
  /// Returns the first differential at the given point.
   real64 Diff1(real64 x) const;
  
  /// Does the inverse, using newton iterations starting at 0.
  /// Will happilly get stuck in a local minima:-P
   real64 Inverse(real64 y,real64 tol=1e-6,nat32 maxIters = 1000) const;


  /// Sets the function to y=k*x.
   void SetMult(real32 k=1.0);

  /// &nbsp;
   void Load(const bs::Element & root);

  /// Returns true on success.
   bit Load(cstrconst fn);

  /// Returns true on success.
   bit Load(const str::String & fn);

  /// &nbsp;
   void Save(bs::Element & root) const;

  /// Returns true on success.
   bit Save(cstrconst fn,bit overwrite = false) const;

  /// Returns true on success.
   bit Save(const str::String & fn,bit overwrite = false) const;
   
   
  /// Sets it to the polynomial specified by the given vector.
  /// poly[n] is the x^n term.
   void SetPoly(const Vector<real32> & poly);
   
  /// Sets it to the polynomial specified by the given vector, but with no constant.
  /// poly[n] is the x^(n+1) term.
   void SetPolyNC(const Vector<real32> & poly);


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::math::Func";}


 private:
  // Term types...
   struct Poly
   {
    nat32 power;
    real64 mult;
   };
   
  // Storage...
   ds::List<Poly> terms;
   
  // Optimisation - array that goes upto the highest polynomial term, for fast
  // calculation (Size of array set on load.)...
   mutable ds::Array<real64> power;
};


//------------------------------------------------------------------------------
 };
};
#endif
