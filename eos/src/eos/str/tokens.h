#ifndef EOS_STR_TOKENS_H
#define EOS_STR_TOKENS_H
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


/// \file tokens.h
/// Contains the token system, which maps strings to numbers for speed of 
/// proccessing/transfer.

#include "eos/types.h"
#include "eos/str/strings.h"

namespace eos
{
 namespace str
 {
//------------------------------------------------------------------------------
/// A Token represents a string using an integer, where a TokenTable is used to
/// convert between numbers and integers. This acts as a form of compression/optimisation
/// when passing strings arround the system.
typedef nat32 Token;

static const Token NullToken = 0;

//------------------------------------------------------------------------------
/// The TokenTable class provides a simple 2 way mapping between integers and 
/// strings, allowing numbers to be used instead of strings. Ushally one of these
/// will exist per running program. Reasonably fast at what it does, though can 
/// be a bit of a memory hog. (Internally a 256 way digital tree with data stored
/// as close to the root as possible, and hashing at the top level to mix up strings
/// with similar starts.)
class EOS_CLASS TokenTable
{
 public:
  /// &nbsp;
   TokenTable();
   
  /// &nbsp;
   ~TokenTable();

  /// Returns an unique number for any given string, any two identical strings will
  /// get the same number, any different strings will get different numbers.
   Token operator()(cstrconst str) const;

  /// Identical to the ushal operator except you give it the strings length - 
  /// it does not use the null terminator. useful when tokenising entire buffers
  /// of strings.
   Token operator()(nat32 length,cstrconst str) const;

  /// Identical to the cstrconst version, except it takes the String class.
   Token operator()(const String & rhs) const;
   
  /// Identical to the cstrconst version, except it takes a string cursor, which
  /// it then eats.
   Token operator()(const String::Cursor & rhs) const;

  /// Given a number as returned from above returns a pointer to the relevent
  /// string - do not edit the result. Returns null if the entry dosn't exist.
   cstrconst Str(Token tok) const;

  /// This is the same as operator(), except in the event
  /// of it not allready being defined it just returns false, without
  /// adding it in. If it returns true then out is set to the relevent Token.
   bit Exists(cstrconst str,Token & out) const;

 private:
  // Number to string...
   inline nat32 AssignNum(cstr str) const; // str must last beyond the methods return.
   nat32 max;
   cstr * str[4]; // 3 is low, high not initialised till needed.

  // String to number...
   class Node
   {
    public:
      Node();
     ~Node();

     Node * Get(cstrconst s,nat32 & depth);
     Node * Get(nat32 remainder,cstrconst s,nat32 & depth);

     // Returns the relevent node if it exists, otherwise null - identical to Get
     // except it dosn't create...
      Node * Exists(cstrconst s);

    cstr str;
    nat32 num;
    Node * child; // Points to an array of 256 children.
   };

   Node * top; // Points to an array of 256, hashed by sum of chars.
};


//------------------------------------------------------------------------------
 };
};
#endif
