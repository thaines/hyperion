#ifndef EOS_IO_CONVERSION_H
#define EOS_IO_CONVERSION_H
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


/// \file io/conversion.h
/// Converts io streams from one type to another.

#include "eos/types.h"

#include "eos/typestring.h"
#include "eos/io/in.h"
#include "eos/io/out.h"

namespace eos
{
 namespace io
 {
//------------------------------------------------------------------------------
/// You provide this with an in io stream and a type, regardless of its current
/// type it is encapsulated to become this new type.
/// Works as a reference to the construction stream, so make sure that object
/// remain and note that they will remain in synch.
/// Template parameter T is the class to suffer a type change, ET is the type it
/// is to be changed into.
template <typename T,typename ET>
class EOS_CLASS ConvertIn : public In<ET>
{
 public:
  /// &nbsp;
   ConvertIn(T & rhs):ptr(rhs){}

  void SetError(bit set) {ptr.SetError(set);}
  void ClearError() {ptr.ClearError();}
  bit Error() const {return ptr.Error();}
  bit Ok() const {return ptr.Ok();}

  bit EOS() const {return ptr.EOS();}
  nat32 Avaliable() const {return ptr.Avaliable();}
  nat32 Read(void * out,nat32 bytes) {return ptr.Read(out,bytes);}
  nat32 Peek(void * out,nat32 bytes) const {return ptr.Peek(out,bytes);}
  nat32 Skip(nat32 bytes) {return ptr.Skip(bytes);}

  cstrconst TypeString() const
  {
   static GlueStr ret(GlueStr() << "eos::io::ConvertIn<" << typestring(ptr) << ">");
   return ret;
  }


 private:
  T & ptr;
};

//------------------------------------------------------------------------------
/// You provide this with an out io stream and a type, regardless of its current
/// type it is encapsulated to become this new type.
/// Works as a reference to the construction stream, so make sure that object
/// remain and note that they will remain in synch.
/// Template parameter T is the class to suffer a type change, ET is the type it
/// is to be changed into.
template <typename T,typename ET>
class EOS_CLASS ConvertOut : public Out<ET>
{
 public:
  /// &nbsp;
   ConvertOut(T & rhs):ptr(rhs){}

  void SetError(bit set) {ptr.SetError(set);}
  void ClearError() {ptr.ClearError();}
  bit Error() const {return ptr.Error();}
  bit Ok() const {return ptr.Ok();}

  nat32 Write(const void * out,nat32 bytes) {return ptr.Write(out,bytes);}
  nat32 Pad(nat32 bytes) {return ptr.Pad(bytes);}

  cstrconst TypeString() const
  {
   static GlueStr ret(GlueStr() << "eos::io::ConvertOut<" << typestring(ptr) << ">");
   return ret;
  }


 private:
  T & ptr;
};

//------------------------------------------------------------------------------
 };
};
#endif
