#ifndef EOS_DS_ITERATION_H
#define EOS_DS_ITERATION_H
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
