#ifndef EOS_IO_INOUT_H
#define EOS_IO_INOUT_H
//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines

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
