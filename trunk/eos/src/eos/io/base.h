#ifndef EOS_IO_BASE_H
#define EOS_IO_BASE_H
//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines

/// \file io/base.h
/// Contains the base class of the io system. This provides error reporting
/// capability and some other basic capabilities for the io module.

/// \namespace eos::io
/// Provides the input/output standard to which a large number of objects 
/// within the system comply. The system is highly templated and designed to
/// result in both minimal and fast code when compiled. Used by file io, 
/// network io and the logging system, as well as others.

#include "eos/types.h"
#include "eos/math/functions.h"

namespace eos
{
 namespace io
 {
//------------------------------------------------------------------------------
/// This class is provided for its children as part of the io stream tagging 
/// system. Exists so you can write an i/o handler to manage both Binary and 
/// Text formatting in the same function - would only be used when you want to
/// use the text stream formating for binary streams as well.
struct EOS_CLASS DualFormat
{
 /// &nbsp;
  static cstrconst TypeString() {return "eos::io::DualFormat";}
};

/// The Binary class is a support class used when defining io streams to 
/// indicate that the stream expects data to be in a binary format. Used for the
/// tagging principle of template partial specialisation. You never use this 
/// class directly.
struct EOS_CLASS Binary : public DualFormat
{
 /// &nbsp;
  static cstrconst TypeString() {return "eos::io::Binary";}
};

/// The Text class is a support class used when defining io streams to indicate
/// that the stream expects data to be in a textual format. Used for the
/// tagging principle of template partial specialisation. You never use this 
/// class directly.
struct EOS_CLASS Text : public DualFormat 
{
 /// &nbsp;
  static cstrconst TypeString() {return "eos::io::Text";}
};

//------------------------------------------------------------------------------
/// Function which converts a DualFormat to the byte that the Pad() method in
/// the Out interface should output for that format type.
template <typename ET>
inline byte GetPadByte(ET);

template<>
inline byte GetPadByte<Binary>(Binary) {return 0;}

template<>
inline byte GetPadByte<Text>(Text) {return ' ';}

//------------------------------------------------------------------------------
/// This is inherited by all io streams, provides the error handling capability.
/// Not to be used directly.
class EOS_CLASS Base
{
 public:
  /// The base constructor, deliberatly initialises the class to be in a state 
  /// of no error.
   inline Base()
   :err(false)
   {}
 
  /// Call with true to switch the error mode to on, false to do nothing. 
  /// Note that SetError(false) does NOT clear the error bit, and in fact does
  /// absolutly nothing.
   inline void SetError(bit set) 
   {
    err = err||set;
   }
  
  /// Call this method to remove any detected errors.
   inline void ClearError()
   {
    err = false;   
   }
  
  /// Returns true if an error has been set since ClearError was last called.
   inline bit Error() const
   {
    return err;
   }
  
  /// The reverse of Error(). Returns true if no error has occured since the 
  /// last call of ClearError().
   inline bit Ok() const
   {
    return !err;   
   }
  
 private:
  bit err;
};

//------------------------------------------------------------------------------
 };
};
#endif
