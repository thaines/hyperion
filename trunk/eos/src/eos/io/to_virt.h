#ifndef EOS_IO_TO_VIRT_H
#define EOS_IO_TO_VIRT_H
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


/// \file to_virt.h
/// Provides the techneques to generate virtual io objects that reference
/// non-virtual io objects, so you can pass non-virtual io objects into methods
/// that require virtual behaviour. This allows all objects in this system to be
/// implimented without the virtual overhead and converted as needed.

#include "eos/typestring.h"
#include "eos/io/seekable.h"

namespace eos
{
 namespace io
 {
//------------------------------------------------------------------------------
/// Provides the In to InVirt convertor class, simply use
/// VirtIn(my_non_virt_class), i.e. the constructor for the class with
/// automatic template parameter determination to make a virtual reference to a
/// non-virtual class.
template <typename T>
class EOS_CLASS VirtIn : public InVirt<typename T::encodeType>
{
 public:
  /// &nbsp;
   VirtIn(T & rhs):ptr(rhs){}

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
   static GlueStr ret(GlueStr() << "eos::io::VirtIn<" << typestring(ptr) << ">");
   return ret;
  }

 private:
  T & ptr;
};

//------------------------------------------------------------------------------
/// Provides the Out to OutVirt convertor class, simply use
/// VirtOut(my_non_virt_class), i.e. the constructor for the class with
/// automatic template parameter determination to make a virtual reference to a
/// non-virtual class.
template <typename T>
class EOS_CLASS VirtOut : public OutVirt<typename T::encodeType>
{
 public:
  /// &nbsp;
   VirtOut(T & rhs):ptr(rhs){}

  void SetError(bit set) {ptr.SetError(set);}
  void ClearError() {ptr.ClearError();}
  bit Error() const {return ptr.Error();}
  bit Ok() const {return ptr.Ok();}

  nat32 Write(const void * out,nat32 bytes) {return ptr.Write(out,bytes);}
  nat32 Pad(nat32 bytes) {return ptr.Pad(bytes);}

  cstrconst TypeString() const
  {
   static GlueStr ret(GlueStr() << "eos::io::VirtOut<" << typestring(ptr) << ">");
   return ret;
  }


 private:
  T & ptr;
};

//------------------------------------------------------------------------------
/// Provides the InOut to InOutVirt convertor class, simply use
/// VirtInOut(my_non_virt_class), i.e. the constructor for the class with
/// automatic template parameter determination to make a virtual reference to a
/// non-virtual class.
template <typename T>
class EOS_CLASS VirtInOut : public InOutVirt<typename T::encodeType>
{
 public:
  /// &nbsp;
   VirtInOut(T & rhs):ptr(rhs){}

  void SetError(bit set) {ptr.SetError(set);}
  void ClearError() {ptr.ClearError();}
  bit Error() const {return ptr.Error();}
  bit Ok() const {return ptr.Ok();}

  bit EOS() const {return ptr.EOS();}
  nat32 Avaliable() const {return ptr.Avaliable();}
  nat32 Read(void * out,nat32 bytes) {return ptr.Read(out,bytes);}
  byte Peek() const {return ptr.Peek();}
  nat32 Skip(nat32 bytes) {return ptr.Skip(bytes);}

  nat32 Write(const void * out,nat32 bytes) {return ptr.Write(out,bytes);}
  nat32 Pad(nat32 bytes) {return ptr.Pad(bytes);}

  cstrconst TypeString() const
  {
   static GlueStr ret(GlueStr() << "eos::io::VirtInOut<" << typestring(ptr) << ">");
   return ret;
  }

 private:
  T & ptr;
};

//------------------------------------------------------------------------------
/// Provides the Seekable to SeekableVirt convertor class, simply use
/// VirtSeekable(my_non_virt_class), i.e. the constructor for the class with
/// automatic template parameter determination to make a virtual reference to a
/// non-virtual class.
template <typename T>
class EOS_CLASS VirtSeekable : public SeekableVirt<typename T::encodeType>
{
 public:
  /// &nbsp;
   VirtSeekable(T & rhs):ptr(rhs){}

  nat32 Size() const {return ptr.Size();}
  bit CanResize() const {return ptr.CanResize();}
  nat32 SetSize(nat32 size) {return ptr.SetSize(size);}

  InOutVirt<typename T::encodeType> GetCursor(nat32 pos = 0) {return VirtInOut<typename T::Cursor>(ptr.Cursor(pos));}

  cstrconst TypeString() const
  {
   static GlueStr ret(GlueStr() << "eos::io::VirtSeekable<" << typestring(ptr) << ">");
   return ret;
  }

 private:
  T & ptr;
};

//------------------------------------------------------------------------------
 };
};
#endif
