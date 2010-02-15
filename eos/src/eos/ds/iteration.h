#ifndef EOS_DS_ITERATION_H
#define EOS_DS_ITERATION_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file iteration.h
/// Contains helper facilities for iterating over data etc.

#include "eos/types.h"
#include "eos/typestring.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
/// This class is used internally to make iteration over a type by calling a 
/// method of a an object for every thing in question possible. It is simply 
/// templated on the object type, the function of that object to call and the
/// type to be passed in. OT is a class, T is the type of element. FUNC is a 
/// method in OT that will be called for each element,FUNC should take a single
/// parameter of type T&.
template <typename OT, typename T, void (OT::*FUNC)(T&)>
class EOS_CLASS ElemIter
{
 public:
  static void F(void * elem,void * obj)
  {
   (((OT*)obj)->*FUNC)(*(T*)elem);
  }
};

//------------------------------------------------------------------------------
 };
};
#endif
