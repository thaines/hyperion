#ifndef EOS_MATH_VECTORS_H
#define EOS_MATH_VECTORS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file vectors.h
/// Provides vectors, both templated to a fixed size and variable size, all 
/// templated by numerical type. Operations are provided independently of both 
/// types, so they only have to be coded once.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/math/functions.h"
#include "eos/io/inout.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
/// A Vector class with templated size as well as templated numeral type. 
/// Provides only the most basic of operations as all the real operations are 
/// implimented independently of the Vector class, only requiring it to follow
/// the standard that both vector classes provided adhere to.
template <nat32 S, typename T = real32>
class EOS_CLASS Vect
{
 public:
  /// The data item type, so it can be used by templated code.
   typedef T type;


  /// Does not initialise anything, leaves all values random.
   Vect() {}
   
  /// Initialises all values to the same given value, ushally used to create the 0 vector.
   Vect(T var) {for(nat32 i=0;i<S;i++) d[i] = var;}
   
  /// &nbsp;
   Vect(T var[S]) {for(nat32 i=0;i<S;i++) d[i] = var[i];}
   
  /// &nbsp;
   Vect(const Vect<S,T> & rhs) {for(nat32 i=0;i<S;i++) d[i] = rhs.d[i];}
   
  /// &nbsp;
   ~Vect() {}
  
   
  /// &nbsp;
   Vect<S,T> & operator = (const Vect<S,T> & rhs) {for(nat32 i=0;i<S;i++) d[i] = rhs.d[i]; return *this;}

  /// &nbsp;
   template <typename T2>
   Vect<S,T2> & operator = (const Vect<S,T2> & rhs) {for(nat32 i=0;i<S;i++) d[i] = rhs[i]; return *this;}  
   
  /// &nbsp;
   template <typename T2>
   Vect<S,T> & FromHomo(const Vect<S+1,T2> & rhs) {for (nat32 i=0;i<S;i++) d[i] = rhs[i]/rhs[S]; return *this;}


  /// Uses Equal(...) with the default error measure.
   bit operator == (const Vect<S,T> & rhs) const
   {
    for (nat32 i=0;i<S;i++)
    {
     if (!Equal(d[i],rhs.d[i])) return false;
    }
    return true;
   }
   
  /// See operator==, the rest is obvious.
   bit operator != (const Vect<S,T> & rhs) const {return !(*this==rhs);}


  /// Required as Size is variable for the other Vector class.
   nat32 Size() const {return S;}
 
  /// &nbsp;
   T & operator [] (nat32 i) {return d[i];}
   
  /// &nbsp;
   const T & operator [] (nat32 i) const {return d[i];}
  
   
  /// &nbsp;
   Vect<S,T> & operator *= (const T & rhs) {for(nat32 i=0;i<S;i++) d[i] *= rhs; return *this;}
   
  /// Internally does a 1.0/ before multiplying through, as thats likelly to be more efficient.
   Vect<S,T> & operator /= (const T & rhs) {T m = 1.0/rhs; for(nat32 i=0;i<S;i++) d[i] *= m; return *this;}
  
   
  /// &nbsp;
   Vect<S,T> & operator += (const Vect<S,T> & rhs) {for(nat32 i=0;i<S;i++) d[i] += rhs.d[i]; return *this;}
   
  /// &nbsp;
   Vect<S,T> & operator -= (const Vect<S,T> & rhs) {for(nat32 i=0;i<S;i++) d[i] -= rhs.d[i]; return *this;}
  
  
  /// Negates all entrys in the vector. 
   void Neg() {for(nat32 i=0;i<S;i++) d[i] = -d[i];}

  /// Returns the dot product, a scalar.
   T operator* (const Vect<S,T> & rhs) const {T ret = 0; for(nat32 i=0;i<S;i++) ret += d[i]*rhs.d[i]; return ret;}

   
  /// Returns the length of the vector, squared.
   T LengthSqr() const {T ret = 0.0; for(nat32 i=0;i<S;i++) ret += math::Sqr(d[i]); return ret;}
   
  /// Returns just the length of the vector.
   T Length() const {return math::Sqrt(LengthSqr());}
   
  /// Returns the distance between two vectors.
   T DistanceTo(const Vect<S,T> & rhs)
   {T ret = 0.0; for(nat32 i=0;i<S;i++) ret += math::Sqr(d[i]-rhs.d[i]); return math::Sqrt(ret);}
  
  /// Normalises the vector, setting its length to 1.
   void Normalise() {*this *= math::InvSqrt(LengthSqr());}
   
  /// This normalises a line representation, i.e. scales it so the vector 
  /// excluding the last point is of length 1.
   void NormLine()
   {
    T lenSqr = 0.0;
    for (nat32 i=0;i<S-1;i++) lenSqr += math::Sqr(d[i]);
    *this *= math::InvSqrt(lenSqr);
   }


  /// &nbsp;
   static inline cstrconst TypeString() 
   {
    static GlueStr ret(GlueStr() << "eos::math::Vect<" << S << "," << typestring<T>() << ">");
    return ret;
   }
 
 
  // Go figgure, for internal use only.
   T * Ptr() {return d;}
   const T * Ptr() const {return d;}


 private:	
  T d[S];	
};

//------------------------------------------------------------------------------
/// A variable sized Vector class templated numeral type. 
/// Provides only the most basic of operations as all the real operations are 
/// implimented independently of the Vector class, only requiring it to follow
/// the standard that both vector classes provided adhere to.
/// The io stream capability of this class does not save size, so you have to do
/// that yourself.
template <typename T = real32>
class EOS_CLASS Vector
{
 public:
  /// The data item type, so it can be used by templated code.
   typedef T type;

  /// Produces a vector of size 0, call SetSize before doing anything else.
   Vector():size(0),data(null<T*>()) {}

  /// Does not initialise anything, leaves all values random.
  /// \param s The size of the new vector.
   Vector(nat32 s):size(s),data(new T[s]) {}   
   
  /// Initialises each element to the second parameter.
  /// \param s The size of the new vector.
  /// \param val The value that each element of the vector is initialised to.
   Vector(nat32 s,T val):size(s),data(new T[s]) {for (nat32 i=0;i<size;i++) data[i] = val;}
   
  /// &nbsp;
   template <nat32 S,typename T2>
   Vector(const Vect<S,T2> & rhs):size(S),data(new T[S]) {for (nat32 i=0;i<S;i++) data[i] = rhs[i];}
   
   // Gcc bug/misfeature resolution...
    Vector(const Vector<T> & rhs):size(rhs.size),data(new T[rhs.size]) {for(nat32 i=0;i<size;i++) data[i] = rhs.data[i];} 
     
  /// &nbsp;
   template <typename T2>
   Vector(const Vector<T2> & rhs):size(rhs.size),data(new T[rhs.size]) {for(nat32 i=0;i<size;i++) data[i] = rhs.data[i];}
   
  /// &nbsp;
   ~Vector() {delete[] data;}
  

  /// &nbsp;
   template <nat32 S,typename T2>
   Vector<T> & operator = (const Vect<S,T2> & rhs)
   {
    SetSize(S);
    for (nat32 i=0;i<S;i++) data[i] = rhs[i];
    return *this;
   }
  
  // This is here because of a compiler bug/misfeature in gcc, that wasted an entire
  // fucking day of my life. If all your operator='s are templated then it basically
  // thinks there are no operator = and reverts to the default. For classes that use
  // pointers this is really rather bad.
   Vector<T> & operator = (const Vector<T> & rhs)
   {
    SetSize(rhs.size);
    for (nat32 i=0;i<size;i++) data[i] = rhs.data[i];
    return *this;
   }
  
  /// &nbsp;
   template <typename T2>
   Vector<T> & operator = (const Vector<T2> & rhs)
   {
    SetSize(rhs.size);
    for (nat32 i=0;i<size;i++) data[i] = rhs.data[i];
    return *this;
   }


  /// Uses Equal(...) with the default error measure.
   bit operator == (const Vector<T> & rhs) const
   {
    if (size!=rhs.size) return false;
    for (nat32 i=0;i<size;i++)
    {
     if (!Equal(data[i],rhs.data[i])) return false;
    }
    return true;
   }
   
  /// See operator==, the rest is obvious.
   bit operator != (const Vector<T> & rhs) const {return !(*this==rhs);}


  /// &nbsp;
   nat32 Size() const {return size;}
   
  /// Leaves the contents in a random state.
   void SetSize(nat32 ns) {if (ns!=size) {size = ns; delete[] data; data = new T[size];}}
 
  /// &nbsp;
   T & operator [] (nat32 i) {return data[i];}

  /// &nbsp;
   const T & operator [] (nat32 i) const {return data[i];}


  /// &nbsp;
   Vector<T> & operator *= (const T & rhs) {for(nat32 i=0;i<size;i++) data[i] *= rhs; return *this;}

  /// Internally does a 1.0/ before multiplying through, as thats likelly to be more efficient.
   Vector<T> & operator /= (const T & rhs) {T m = 1.0/rhs; for(nat32 i=0;i<size;i++) data[i] *= m; return *this;}

  /// &nbsp;
   Vector<T> & operator += (const Vector<T> & rhs) 
   {
    log::Assert(size==rhs.size);
    for(nat32 i=0;i<size;i++) data[i] += rhs.data[i];
    return *this;
   }
   
  /// &nbsp;
   Vector<T> & operator -= (const Vector<T> & rhs) 
   {
    log::Assert(size==rhs.size);
    for(nat32 i=0;i<size;i++) data[i] -= rhs.data[i];
    return *this;
   }  


  /// Negates all entrys in the vector. 
   void Neg() {for(nat32 i=0;i<size;i++) data[i] = -data[i];}

  /// Returns the dot product, a scalar
   T operator * (const Vector<T> & rhs) const {log::Assert(Size()==rhs.Size());T ret = 0; for (nat32 i=0;i<Size();i++) ret += (*this)[i]*rhs[i]; return ret;}
  
  /// Returns the dot product, a scalar
   template <nat32 S>
   T operator * (const Vect<S,T> & rhs) const {log::Assert(Size()==S);T ret = 0; for (nat32 i=0;i<S;i++) ret += (*this)[i]*rhs[i]; return ret;}   


  /// Returns the length of the vector, squared.
   T LengthSqr() const {T ret = 0.0; for(nat32 i=0;i<size;i++) ret += math::Sqr(data[i]); return ret;}

  /// Returns just the length of the vector.
   T Length() const {return math::Sqrt(LengthSqr());}

  /// Normalises the vector, setting its length to 1.
   void Normalise() {*this *= math::InvSqrt(LengthSqr());}


  /// &nbsp;
   static inline cstrconst TypeString() 
   {
    static GlueStr ret(GlueStr() << "eos::math::Vector<" << typestring<T>() << ">");
    return ret;
   }


 private:
  nat32 size;
  T * data;
};

//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
// IO stream integration for the above classes...
namespace eos
{
 namespace io
 {

  template <typename T, nat32 VS, typename VT>
  inline T & StreamRead(T & lhs,math::Vect<VS,VT> & rhs,Binary)
  {
   for (nat32 i=0;i<rhs.Size();i++)
   {
    lhs >> rhs[i];
   }
   return lhs;
  }
  
  template <typename T, nat32 VS, typename VT>
  inline T & StreamRead(T & lhs,math::Vect<VS,VT> & rhs,Text)
  {
   SkipWS(lhs);
   cstrchar p;
   if ((lhs.Peek(&p,1)!=1)||(p!='(')) {lhs.SetError(true); return lhs;}
   lhs.Skip(1);
   for (nat32 i=0;i<rhs.Size();i++)
   {
    lhs >> rhs[i];
    if (i+1!=rhs.Size())
    {
     if ((lhs.Peek(&p,1)!=1)||(p!=',')) {lhs.SetError(true); return lhs;}	  
     lhs.Skip(1);
    }
   }
   if ((lhs.Peek(&p,1)!=1)||(p!=')')) {lhs.SetError(true); return lhs;}
   lhs.Skip(1);
   return lhs;
  }
  
  template <typename T, nat32 VS, typename VT>
  inline T & StreamWrite(T & lhs,const math::Vect<VS,VT> & rhs,Binary)
  {
   for (nat32 i=0;i<rhs.Size();i++)
   {
    lhs << rhs[i];	 
   }
   return lhs;
  }
  
  template <typename T, nat32 VS, typename VT>
  inline T & StreamWrite(T & lhs,const math::Vect<VS,VT> & rhs,Text)
  {
   lhs << "(";
   for (nat32 i=0;i<rhs.Size();i++)
   {
    lhs << rhs[i];	 
    if (i+1!=rhs.Size()) lhs << ",";
   }
   lhs << ")";
   return lhs;
  }

 
  template <typename T, typename VT>
  inline T & StreamRead(T & lhs,math::Vector<VT> & rhs,Binary)
  {
   for (nat32 i=0;i<rhs.Size();i++)
   {
    lhs >> rhs[i];
   }
   return lhs;
  }
  
  template <typename T, typename VT>
  inline T & StreamRead(T & lhs,math::Vector<VT> & rhs,Text)
  {   
   SkipWS(lhs);
   cstrchar p;
   if ((lhs.Peek(&p,1)!=1)||(p!='(')) {lhs.SetError(true); return lhs;}
   lhs.Skip(1);
   for (nat32 i=0;i<rhs.Size();i++)
   {
    lhs >> rhs[i];
    if (i+1!=rhs.Size())
    {
     if ((lhs.Peek(&p,1)!=1)||(p!=',')) {lhs.SetError(true); return lhs;}	  
     lhs.Skip(1);
    }
   }
   if ((lhs.Peek(&p,1)!=1)||(p!=')')) {lhs.SetError(true); return lhs;}
   lhs.Skip(1);
   return lhs;
  }
  
  template <typename T, typename VT>
  inline T & StreamWrite(T & lhs,const math::Vector<VT> & rhs,Binary)
  {
   for (nat32 i=0;i<rhs.Size();i++)
   {
    lhs << rhs[i];	 
   }
   return lhs;
  }
  
  template <typename T, typename VT>
  inline T & StreamWrite(T & lhs,const math::Vector<VT> & rhs,Text)
  {
   lhs << "(";
   for (nat32 i=0;i<rhs.Size();i++)
   {
    lhs << rhs[i];	 
    if (i+1!=rhs.Size()) lhs << ",";
   }
   lhs << ")";
   return lhs;
  }

 };
};

//------------------------------------------------------------------------------
#endif
