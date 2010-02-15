#ifndef EOS_FILE_DLLS_H
#define EOS_FILE_DLLS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

/// \file dlls.h
/// Provides the ability to load/unload dynamic librarys and extract function
/// pointers from them.

#include "eos/types.h"

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
/// This class represents any one dynamic library at any one time, allowing you 
/// to get pointers to functions in the library in question.
class EOS_CLASS Dll : public Deletable
{
 public:
  /// &nbsp;
   Dll();

  /// &nbsp;
  ~Dll();


  /// Returns true if a dll is currently loaded and therefore GetFunc will work.
   bit Active() const {return handle!=null<void*>();}

 
  /// Loads a dll into the object, if a dll is allready loaded then it will 
  /// unload that first. Set delayed = true to only load symbols when they
  /// are used. This makes loading quicker but first calls of symbols
  /// slower, leaving it false is prefered. Only to be used if loading time
  /// is to be reduced. It can also result in delayed errors, so this option
  /// is ignored in debug mode. Note that it manages the extension itself, 
  /// adjusting it for the current platform, so you request 'my' instead of
  /// my.dll or libmy.dll.
   void Load(cstrconst filename,bool delayed = false);

  /// Unloads the dll currently loaded into the object, or does nothing if 
  /// Active()==false. Note that the moment this is called (or the object 
  /// is deconstructed) all function pointers obtained become invalid.
   void Unload();
   
  /// This Unloads the dll currently loaded into the object, so you can no 
  /// longer call Get, however, unlike Unload it dosn't remove the dll from
  /// memory, so all function pointers will remain valid. This is bad practise,
  /// as you are then essentialy relying on the operating system to garbage 
  /// collect the dll when your program exits, and can't unload it yourself for
  /// the remainder of the programs runtime. There are various situations when 
  /// this makes sense however, i.e. when dynamically loading librarys only so
  /// you can fail nicelly/run with reduced functionality if they don't exist.
   void UnloadKeep() {handle = 0;}

  /// Returns a function pointer given its name, or null if it is not within
  /// the dll in question. It will manage any mangling that has occured, so just
  /// the unblemished name please.
   void * Get(cstrconst name) const;

  
  /// &nbsp;
   static cstrconst TypeString() {return "eos::file::Dll";}


 protected:
  void * handle;
};

//------------------------------------------------------------------------------
 };
};
#endif
