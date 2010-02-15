#ifndef EOS_TYPESTRING_H
#define EOS_TYPESTRING_H
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


/// \file typestring.h
/// Contains the templated function typestring<type>(), which returns a string 
/// to represent the type of given to it. It is used primarilly by the logging
/// system within templated classes.
/// To achieve this it requires that everything impliments a 
/// static cstrconst TypeString() method, with exception of base types where
/// it is overloaded to behave itself.

#include "eos/types.h"

namespace eos
{
//------------------------------------------------------------------------------

/// This function is for use within templated code, generally for the purpose
/// of the logging system. Called with a typename as its templated parameter
/// it returns a constant string that represents the type in question, as it
/// would be typed into code, fully qualified. Requires that all classes provide
/// a static cstrconst TypeName() member.
/// (If you omit this requirememnt you can't call the function, obviously,
/// however the logging system is so pervasive that you probably won't be able
/// to use the class for a debug build without it.)
/// \returns A constant string representing the class. Ushally in the fully qualified form used to declare an instance of the given type.
template <typename T> 
inline cstrconst typestring()
{
 return T::TypeString();
}

/// A version of typestring that can be more conveniant than the basic variety,
/// with the advantage that it works on virtual types.
template <typename T> 
inline cstrconst typestring(const T & t)
{
 return t.TypeString();
}


template <>
inline cstrconst typestring<nat8>()
{
 return "eos::nat8";
}

template <>
inline cstrconst typestring<nat16>()
{
 return "eos::nat16";        
}

template <>
inline cstrconst typestring<nat32>()
{
 return "eos::nat32";        
}

template <>
inline cstrconst typestring<nat64>()
{
 return "eos::nat64";        
}


template <>
inline cstrconst typestring<int8>()
{
 return "eos::int8";        
}

template <>
inline cstrconst typestring<int16>()
{
 return "eos::int16";        
}

template <>
inline cstrconst typestring<int32>()
{
 return "eos::int32";        
}

template <>
inline cstrconst typestring<int64>()
{
 return "eos::int64";        
}


template <>
inline cstrconst typestring<real32>()
{
 return "eos::real32";        
}

template <>
inline cstrconst typestring<real64>()
{
 return "eos::real64";        
}


template <>
inline cstrconst typestring<bit>()
{
 return "eos::bit";        
}

template <>
inline cstrconst typestring<cstr>()
{
 return "eos::cstr";        
}

template <>
inline cstrconst typestring<cstrconst>()
{
 return "eos::cstrconst";        
}

//------------------------------------------------------------------------------
/// \class GlueStr
/// The GlueStr class is a helper class used to assist in the creation of 
/// returnable strings for when defining the TypeString() method for templated
/// classes. Its generally used in the following form:<br><br>
/// <code>cstrconst my_class::TypeString()<br>
/// {<br>
/// &nbsp;static GlueStr ret(GlueStr() << "my_class<" << typestring(T) << ">");<br>
/// &nbsp;return ret;<br>
/// }</code><br><br>
/// and is optimised specifically for this one scenario.

class EOS_CLASS GlueStr
{
 public:
  /// Constructor without parameters, initialises the class to contain the string "".
   GlueStr()
   :ret(null<cstr>()),n(0)
   {}
  
  /// Constructor in terms of another GlueStr. Works by directly copying the rhs string over, this
  /// leaves the rhs as an empty GlueStr. In essence this is to allow for declaring the GlueStr 
  /// object on a single line using the '<<' operator with a tempory object, without causing unnecesary
  /// work, as required for the decleration of static objects.
   GlueStr(GlueStr & rhs) // Works by transfering the data over - matches up with how this object is meant to be used.
   :n(0)
   {
    if (rhs.n!=0) rhs.Fold();
    this->ret = rhs.ret;
    rhs.ret = null<cstr>();
   }
  
  /// Destructor, to delete any internal data.
   ~GlueStr()
   {
    delete[] ret;   
   }
  
  /// Provides a constant string pointer, which is owned by the object.
   operator cstrconst ()
   {
    if (n!=0) Fold();
    if (ret) return ret;   
        else return "error";
   }
  
  /// For concaternating constant strings together to create the relevent string,
  /// used to create the tempory GlueStr to be stored as required.
   GlueStr & operator << (cstrconst str)
   {
    if (n==nMax) Fold();
    rhs[n] = str;
    ++n;
    return *this;
   }
 
  /// &nbsp;
   inline static cstrconst TypeString()
   {
    return "eos::GlueStr";   
   } 
   
 private:
  void Fold();  
 
  cstr ret;
  
  static const nat32 nMax = 3; // Set as 3 on the grounds that the pattern will ushally be "class<" << templated class << ">"
  nat32 n; // Number of rhs slots filled.   
  cstrconst rhs[nMax]; // A number of constant strings given, yet to be folded into the return string, ret.
};

//------------------------------------------------------------------------------
};
#endif
