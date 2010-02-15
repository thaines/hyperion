#ifndef EOS_SVT_FIELD_H
#define EOS_SVT_FIELD_H
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


/// \file field.h
/// Contains the templated accesor types for data within the multi-dimensional 
/// var arrays.

#include "eos/types.h"
#include "eos/typestring.h"

#include "eos/ds/arrays.h"
#include "eos/mem/alloc.h"
#include "eos/mem/functions.h"
#include "eos/str/tokens.h"

namespace eos
{
 namespace svt
 {
//------------------------------------------------------------------------------
// As its used below we pre-declare it here...
class Var;

//------------------------------------------------------------------------------
/// An accessor to a particular field within a Var, templated on the type for it
/// to return. It supplies several getters with different numbers of indexes, for
/// accessing 1D upto 4D data structures. More than that is simply not supported
/// by the system. In the event of using a lower dimensional getter than provided
/// it will be as though the extra dimensions have been set to 0. In the case of 
/// using a higher dimension than provided expect it to crash.
template <typename T>
class EOS_CLASS Field
{
 public:
  /// Whilst you can create a field with this constructor do not use 
  /// it until you have passed it into Var, i.e. until Valid().
   Field():var(null<Var*>()),data(null<byte*>()),stride(null<nat32*>()) {}
   
  /// This constructs a field directly from a Var on being given a token of the
  /// field to extract. If the field does not exist the field will not be valid,
  /// so if unsure check.
   Field(Var * var,str::Token f);

  /// This constructs a field directly from a Var on being given a string of the
  /// field to extract. If the field does not exist the field will not be valid,
  /// so if unsure check.
   Field(Var * var,cstrconst f);

  /// &nbsp;
   Field(const Field<T> & rhs):var(rhs.var),data(rhs.data),stride(rhs.stride) {}

  /// &nbsp;
   ~Field() {}


  /// &nbsp;
   Field<T> & operator = (const Field<T> & rhs) {var = rhs.var; data = rhs.data; stride = rhs.stride; return *this;}

  // Undocumented, as only for internal use.
   void Set(Var * v,byte * d, nat32 * s) {var = v; data = d; stride = s;}

  // Undocumented, as only for internal use.
   void Set(const Var * v,byte * d, nat32 * s) const {var = v; data = d; stride = s;}

  /// &nbsp;
   bit operator == (const Field<T> & rhs) {return data==rhs.data;}


  /// Returns true if the field has been set and is ready for action.
   bit Valid() const {return (data!=null<byte*>()) && (stride!=null<nat32*>());}
   
  /// Makes Valid()==false, call this if you delete the Var its pointing to.
   void SetInvalid() {var = null<Var*>(); data = null<byte*>(); stride = null<nat32*>();}


  /// Returns the var which this field links to.
   Var * GetVar() const {return var;}

  /// This allows you to get a field within the field given, for instance if this field 
  /// consists of 3 floating point values this allows you to offset to internal
  /// values and create a field of 1 floating point value.
   template <typename S>
   void SubField(nat32 offset,Field<S> & out) const {out.Set(var,data+offset,stride);}


  /// Returns how many dimensions it has.
   nat32 Dims() const;
  /// Returns the size of a given dimension.
   nat32 Size(nat32 dim) const;
   
  /// Returns a pointer to an array of sizes.
   const nat32 * Sizes() const;

  /// Returns the stride of a given dimension.
   nat32 Stride(nat32 dim) const;
      
  // Returns a pointer to an array of stride sizes.
  // Entry 0 is the gap between each item.
   const nat32 * Strides() const;

  /// Returns the number of items in the data structure.
   nat32 Count() const;


  /// 0 dimension accessor, provided only for completness, in 
  /// principal it could be used.
   T & Get()
   {return *(T*)(data);}
   
  /// &nbsp;
   const T & Get() const
   {return *(T*)(data);}
   
  /// 1 dimension accessor, only use for 1 dimensional data sets.
   T & Get(nat32 x)
   {return *(T*)(data + x*stride[0]);}

  /// &nbsp;
   const T & Get(nat32 x) const
   {return *(T*)(data + x*stride[0]);}  
  
  /// 2 dimensional accessor, only use for 2 dimensional data sets.
   T & Get(nat32 x,nat32 y)
   {return *(T*)(data + x*stride[0] + y*stride[1]);}

  /// &nbsp;
   const T & Get(nat32 x,nat32 y) const
   {return *(T*)(data + x*stride[0] + y*stride[1]);}

  /// 3 dimensional accessor, only use for 3 dimensional data sets.
   T & Get(nat32 x,nat32 y,nat32 z)
   {return *(T*)(data + x*stride[0] + y*stride[1] + z*stride[2]);}

  /// &nbsp;
   const T & Get(nat32 x,nat32 y,nat32 z) const
   {return *(T*)(data + x*stride[0] + y*stride[1] + z*stride[2]);}

  /// 4 dimensional accessor, only use for 4 dimensional data sets.
   T & Get(nat32 x,nat32 y,nat32 z,nat32 t)
   {return *(T*)(data + x*stride[0] + y*stride[1] + z*stride[2] + t*stride[3]);}

  /// &nbsp;
   const T & Get(nat32 x,nat32 y,nat32 z,nat32 t) const
   {return *(T*)(data + x*stride[0] + y*stride[1] + z*stride[2] + t*stride[3]);}  
   
  /// n dimensional accessor, actually uses the Dims() function, amazing.
  /// Must be given a pointer to Dims() nat32-s.
   T & Get(nat32 * pos)
   {
    byte * ret = data; 
    nat32 dims = Dims();
    for (nat32 i=0;i<dims;i++) ret += pos[i]*stride[i];
    return *(T*)ret;
   }
   
  /// &nbsp;
   const T & Get(nat32 * pos) const
   {
    byte * ret = data; 
    nat32 dims = Dims();
    for (nat32 i=0;i<dims;i++) ret += pos[i]*stride[i];
    return *(T*)ret;
   }
   
   
  /// The array can be any size, it ignores extra dimensions, presumes missing dimensions to be zero.
   T & Get(const ds::Array<nat32> & pos)
   {
    byte * ret = data;
    nat32 top = math::Min(Dims(),pos.Size());	   
    for (nat32 i=0;i<top;i++) ret += pos[i]*stride[i];
    return *(T*)ret;
   }

  /// The array can be any size, it ignores extra dimensions, presumes missing dimensions to be zero.
   const T & Get(const ds::Array<nat32> & pos) const
   {
    byte * ret = data;
    nat32 top = math::Min(Dims(),pos.Size());	   
    for (nat32 i=0;i<top;i++) ret += pos[i]*stride[i];
    return *(T*)ret;
   }


  /// Copys all the data in the given field into this field. The given field must
  /// have the same number of dimensions and equal or larger sizes.
   void CopyFrom(const Field<T> & rhs)
   {
    // Setup position variable...
     nat32 posTemp[4];
     nat32 dims = Dims();
     nat32 * pos = posTemp;
     if (dims>4) pos = mem::Malloc<nat32>(dims);
     for (nat32 i=0;i<dims;i++) pos[i] = 0;
 
    // Iterate every last field entry...
     bit cont = true;
     while (cont)
     {
      byte * from = rhs.data; for (nat32 i=0;i<dims;i++) from += pos[i]*rhs.stride[i];
      byte * to = data; for (nat32 i=0;i<dims;i++) to += pos[i]*stride[i];
      mem::Copy(to,from,sizeof(T));
      
      ++pos[0];

      for (nat32 i=0;i<dims;i++)
      {
       if (pos[i]<Size(i)) break;
       if (i==(dims-1)) {cont = false; break;}
       pos[i] = 0;
       ++pos[i+1];
      }
     }   

    // Clean up...
     if (Dims()>4) mem::Free(pos);
   }

  /// Copys all the data from a given load of bytes into this field, you give it a pointer 
  /// to the stride from the start of one item to the next (in bytes), it then copys in 
  /// the relevent number of items.
   void CopyFrom(T * d,nat32 s);


  /// &nbsp;
   static inline cstrconst TypeString() 
   {
    static GlueStr ret(GlueStr() << "eos::svt::Field<" << typestring<T>() << ">");
    return ret;
   }


 private:
  Var * var; // Pointer to the relevent Var, so it can returns dims and size.
  byte * data; // Pointer to the first item.
  nat32 * stride; // Pointer to an array of numbers, dim[0] is the stride, dim[1] is the stride * the size of the first dimension, etc.
};

//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
// This little section is to get arround a circular linkage problem...
#include "eos/svt/var.h"

template <typename T>
eos::svt::Field<T>::Field(eos::svt::Var * var,eos::str::Token f)
:var(null<Var*>()),data(null<byte*>()),stride(null<nat32*>())
{var->ByName(f,*this);}

template <typename T>
eos::svt::Field<T>::Field(eos::svt::Var * var,eos::cstrconst f)
:var(null<Var*>()),data(null<byte*>()),stride(null<nat32*>())
{var->ByName(f,*this);}
   
template <typename T>
inline eos::nat32 eos::svt::Field<T>::Dims() const 
{return var->Dims();}

template <typename T>
inline eos::nat32 eos::svt::Field<T>::Size(eos::nat32 dim) const 
{return var->Size(dim);}

template <typename T>
inline const eos::nat32 * eos::svt::Field<T>::Sizes() const
{return var->Sizes();}

template <typename T>
inline eos::nat32 eos::svt::Field<T>::Stride(eos::nat32 dim) const 
{return var->Stride(dim);}

template <typename T>
inline const eos::nat32 * eos::svt::Field<T>::Strides() const
{return var->Strides();}

template <typename T>
inline eos::nat32 eos::svt::Field<T>::Count() const
{return var->Count();}

template <typename T>
void eos::svt::Field<T>::CopyFrom(T * d,eos::nat32 s)
{
 // Setup position variable...
  eos::nat32 posTemp[4];
  eos::nat32 dims = Dims();
  eos::nat32 * pos = posTemp;
  if (dims>4) pos = eos::mem::Malloc<nat32>(dims);
  for (eos::nat32 i=0;i<dims;i++) pos[i] = 0;

 // Iterate every last field entry...
  eos::nat32 count = var->Count();
  for (eos::nat32 i=0;i<count;i++)
  {
   byte * to = data; for (eos::nat32 j=0;j<dims;j++) to += pos[j]*stride[j];
   eos::mem::Copy(to,(eos::byte*)d,sizeof(T));
   
   ++pos[0];
   (eos::byte*&)d += s;

   for (eos::nat32 j=0;j<dims-1;j++)
   {
    if (pos[j]<Size(j)) break;
    pos[j] = 0;
    ++pos[j+1];
   }
  }   

 // Clean up...
  if (Dims()>4) eos::mem::Free(pos);    
}
   
//------------------------------------------------------------------------------
#endif
