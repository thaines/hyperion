#ifndef EOS_DS_SORTING_H
#define EOS_DS_SORTING_H
//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines

/// \file sorting.h
/// Defines the sorting type, and some default behaviours.

#include "eos/types.h"
#include "eos/typestring.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
/// An empty class, simply provided to unify the children class and make 
/// documentation of the system easier. All Sort derivates impliment a
/// LessThan method that takes a lhs and rhs const reference to the type and
/// returns true if lhs is less than rhs, false otherwise.
class EOS_CLASS Sort
{};

/// This sorts by applying the standard comparison operator.
template <typename T>
class EOS_CLASS SortOp : public Sort
{
 public:
  /// &nbsp;
   static bit LessThan(const T & lhs,const T & rhs) {return lhs<rhs;}
   
  /// &nbsp; 
   static inline cstrconst TypeString() 
   {
    static GlueStr ret(GlueStr() << "eos::ds::SortOp<" << typestring<T>() << ">");
    return ret;
   }
};

/// This sorts by applying the standard comparison operator to the object that
/// T points to, obviously should only be used when T is a pointer type.
template <typename T>
class EOS_CLASS SortPtrOp : public Sort
{
 public:
  /// &nbsp;
   static bit LessThan(const T & lhs,const T & rhs) {return (*lhs)<(*rhs);}
   
  /// &nbsp; 
   static inline cstrconst TypeString() 
   {
    static GlueStr ret(GlueStr() << "eos::ds::SortPtrOp<" << typestring<T>() << ">");
    return ret;
   }
};

//------------------------------------------------------------------------------
 };
};
#endif
