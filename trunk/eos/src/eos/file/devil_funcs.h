#ifndef EOS_FILE_DEVIL_FUNCS_H
#define EOS_FILE_DEVIL_FUNCS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

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

/// \file devil_funcs.h
/// Provides access to the Devil Library, for image I/O.

#include "eos/types.h"
#include "eos/mem/alloc.h"

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
// Types for all the functions loaded...  
typedef void EOS_STDCALL (*voidTovoid)();
typedef void EOS_STDCALL (*intTovoid)(unsigned int);
typedef int EOS_STDCALL (*voidToint)();
typedef int EOS_STDCALL (*intToint)(int);
typedef unsigned char EOS_STDCALL (*intTobool)(int);
typedef unsigned char EOS_STDCALL (*stringTobool)(const char *);
typedef void EOS_STDCALL (*handFunc)(int,unsigned int *);
typedef int EOS_STDCALL (*cpFunc)(unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,unsigned int,void *);  
typedef unsigned char EOS_STDCALL (*tiFunc)(unsigned int,unsigned int,unsigned int,unsigned char,unsigned int,unsigned int,void *);

 
// The functions...
EOS_VAR voidTovoid   ilInit;
EOS_VAR voidTovoid   ilShutdown;
EOS_VAR voidToint    ilGetError;
EOS_VAR handFunc     ilGenImages;
EOS_VAR intTovoid    ilBindImage;
EOS_VAR handFunc     ilDeleteImages;
EOS_VAR intTobool    ilEnable;
EOS_VAR intTobool    ilDisable;
EOS_VAR intToint     ilGetInteger;
EOS_VAR intTobool    ilOriginFunc;  
EOS_VAR stringTobool ilLoadImage;
EOS_VAR stringTobool ilSaveImage;
EOS_VAR tiFunc       ilTexImage;
EOS_VAR cpFunc       ilCopyPixels;

// Constants...
static const nat32 DEVIL_FORM_L = 0x1909;
static const nat32 DEVIL_FORM_RGB = 0x1907;
static const nat32 DEVIL_FORM_RGBA = 0x1908;
static const nat32 DEVIL_TYPE_BYTE = 0x1400;
static const nat32 DEVIL_TYPE_FLOAT = 0x1406;
static const nat32 DEVIL_FILE_SQUISH = 0x0620;

// Call before use, returns true if you can use the library, false if it hasn't loaded. 
// After it has been called and returns true it will continue to return true.
EOS_FUNC bit DevilActive();

//------------------------------------------------------------------------------
 };
};
#endif
