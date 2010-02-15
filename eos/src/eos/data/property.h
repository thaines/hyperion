#ifndef EOS_DATA_PROPERTY_H
#define EOS_DATA_PROPERTY_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file property.h
/// Defines a templated class for accessing properties of a class. 
/// Such properties could of been optionaly added at run-time via overloaded
/// class allocation. For providing a typesafe interface to something 
/// inherantly un-typesafe.

#include "eos/types.h"

namespace eos
{
 namespace data
 {
//------------------------------------------------------------------------------
// Function which returns the pointer given to it as a void*...
template <typename T>
void * DoNothing(T * t) {return t;}

//------------------------------------------------------------------------------
/// A property acts as an accessor for 'something' in a given class - it is 
/// internally nothing more than a templated class that has an offset in bytes
/// from the pointers given to the relevant data. Provided to encapsulate the
/// extremely unsafe nature of anything using such a system.
/// Templated type T is the class it may be called on, templated class P is the
/// type of the property itself.
template <typename T,typename P>
class EOS_CLASS Property
{
 public:
  /// Leaves it as a (dangerous) dud - make sure to assign to it before use!
   Property():offset(0),F(DoNothing<T>) {}

  /// &nbsp;
   Property(const Property<T,P> & rhs):offset(rhs.offset),F(rhs.F) {}

  /// Constructs it - this should only be called by a class that 'knows' the 
  /// structure of T. An Offset of 0 is used to indicate a dud, so you can't 
  /// have a data blob that is entirelly dedicated to Propertys.
  /// os is the offset, func is called on the object pointer before use, 
  /// the offset is then in bytes from the returned value. The default
  /// returns what it is given.
   Property(nat32 os,void * (*func)(T * t) = DoNothing<T>)
   :offset(os),F(func) {}

  /// &nbsp;
   ~Property() {}


  /// &nbsp;
   Property & operator = (const Property & rhs) {offset = rhs.offset; F = rhs.F; return *this;}


  /// Returns true if the class is suposed to be safe to use.
   bit Valid() const {return offset!=0;}


  /// This provides access to the property - you give it an instance of T and it
  /// returns a reference to a P.
   P & Get(T & t) const 
   {
    void * start = F(&t);
    void * pos = (byte*)start + offset;
    return *(P*)pos;
   }

  /// This provides access to the property - you give it an instance of T and it
  /// returns a reference to a P.
   const P & Get(const T & t) const
   {
    void * start = F(const_cast<T*>(&t));
    void * pos = (byte*)start + offset;
    return *(P*)pos;
   }


 private:
  nat32 offset;
  void * (*F)(T * t);
};

//------------------------------------------------------------------------------
 };
};
#endif
