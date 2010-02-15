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

#include "eos/file/dlls.h"

#ifdef EOS_LINUX
 #include <dlfcn.h>
#else
 #include <windows.h>
#endif

#include "eos/mem/alloc.h"
#include "eos/str/functions.h"

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
Dll::Dll()
:handle(0) 
{}

Dll::~Dll()
{
 Unload();
}

void Dll::Load(cstrconst filename,bool delayed)
{
 Unload();

 #ifdef EOS_WIN32
  handle = (void*)LoadLibrary(filename);
 #else
  cstr fn = mem::Malloc<cstrchar>(str::Length(filename) + 8 + 1); // 6 = strlen "lib" + ".so.0", +2 for trying '.0' if it fails.
  str::Copy(fn,"lib");
  str::Append(fn,filename);
  str::Append(fn,".so");

  int flag = RTLD_NOW;
  #ifdef EOS_RELEASE
   if (delayed) flag = RTLD_LAZY;
  #endif

  handle = dlopen(fn,flag | RTLD_GLOBAL);
  if (handle==null<void*>())
  {
   str::Append(fn,".0");
   handle = dlopen(fn,flag | RTLD_GLOBAL);
  }

  mem::Free(fn);
 #endif
}

void Dll::Unload()
{
 #ifdef EOS_WIN32
  if (handle) FreeLibrary((HMODULE)handle);
 #else
  if (handle) dlclose(handle);  
 #endif
 handle = 0;
}

void * Dll::Get(cstrconst name) const
{
 #ifdef EOS_WIN32
  return (void*)GetProcAddress((HMODULE)handle,name);
 #else
  return dlsym(handle,name);  
 #endif
}

//------------------------------------------------------------------------------
 };
};
