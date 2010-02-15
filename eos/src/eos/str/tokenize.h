#ifndef EOS_STR_TOKENIZE_H
#define EOS_STR_TOKENIZE_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

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


/// \file tokenize.h
/// Provides the ability to chop up strings into lists of tokens.

#include "eos/types.h"

#include "eos/str/strings.h"
#include "eos/ds/arrays.h"

namespace eos
{
 namespace str
 {
//------------------------------------------------------------------------------
/// This is constructed with a string, it then acts as an indexed array of strings.
class EOS_CLASS Tokenize
{
 public:
  /// Initialises to be without tokens.
   Tokenize();

  /// s is the string to tokenise, del are the delimeters that divide up tokens.
   Tokenize(cstrconst s,cstrconst del = " \t\n\r");

  /// s is the string to tokenise, del are the delimeters that divide up tokens.
   Tokenize(const str::String & s,cstrconst del = " \t\n\r");

  /// &nbsp;
   ~Tokenize();


  /// To replace the current contents.
   void Replace(cstrconst s,cstrconst del = " \t\n\r");

  /// To replace the current contents.
   void Replace(const str::String & s,cstrconst del = " \t\n\r");


  /// &nbsp;
   nat32 Size() const;

  /// &nbsp;
   cstrconst operator[] (nat32 i) const;


  /// &nbsp;
   static cstrconst TypeString() {return "eos::str::Tokenize";}


 private:
  cstr ts;
  void Setup(cstrconst del);
  ds::Array<nat32> toks;
};

//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
// I/O functions...
namespace eos
{
 namespace io
 {


  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::str::Tokenize & rhs,Text)
  {
   lhs << "toks{";
   for (nat32 i=0;i<rhs.Size();i++)
   {
    if (i!=0) lhs << ",";
    lhs << rhs[i];
   }
   lhs << "}";
   return lhs;
  }


 };
};
//------------------------------------------------------------------------------
#endif
