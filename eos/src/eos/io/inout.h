#ifndef EOS_IO_INOUT_H
#define EOS_IO_INOUT_H
//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines

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


/// \file inout.h
/// Contains the standard for objects from which data can be both read and 
/// written, combines the in and out types.

#include "eos/io/in.h"
#include "eos/io/out.h"

namespace eos
{
 namespace io
 {
//------------------------------------------------------------------------------
/// A merging of the In and Out io classes, supplying both the in and out 
/// interface. Provided to do disambguation and to provide a common base class
/// for such implimentations.
template <typename ET>
class EOS_CLASS InOut : public In<ET>, public Out<ET>
{
 public:
  // Disambiguation...
   static inline ET EncodeType() {return ET();}
   typedef ET encodeType;
  
   inline void SetError(bit set) {In<ET>::SetError(set);} 
   inline void ClearError() {In<ET>::ClearError();}
   inline bit Error() const {return In<ET>::Error();}
   inline bit Ok() const {return In<ET>::Ok();}
};

//------------------------------------------------------------------------------
/// The virtual version of InOut. See InOut for reference.
template <typename ET>
class EOS_CLASS InOutVirt : public InVirt<ET>, public OutVirt<ET>
{
 public:
  // Disambiguation...
   static inline ET EncodeType(){return ET();}
   typedef ET encodeType;
   
   inline void SetError(bit set) {InVirt<ET>::SetError(set);}  
   inline void ClearError() {InVirt<ET>::ClearError();}
   inline bit Error() const {return InVirt<ET>::Error();}
   inline bit Ok() const {return InVirt<ET>::Ok();}	
};

//------------------------------------------------------------------------------
 };
};
#endif
