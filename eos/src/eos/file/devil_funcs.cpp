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

#include "eos/file/devil_funcs.h"

#include "eos/file/dlls.h"

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
// The functions...
EOS_VAR_DEF voidTovoid   ilInit;
EOS_VAR_DEF voidTovoid   ilShutdown;
EOS_VAR_DEF voidToint    ilGetError;
EOS_VAR_DEF handFunc     ilGenImages;
EOS_VAR_DEF intTovoid    ilBindImage;
EOS_VAR_DEF handFunc     ilDeleteImages;
EOS_VAR_DEF intTobool    ilEnable;
EOS_VAR_DEF intTobool    ilDisable;
EOS_VAR_DEF intToint     ilGetInteger;
EOS_VAR_DEF intTobool    ilOriginFunc;  
EOS_VAR_DEF stringTobool ilLoadImage;
EOS_VAR_DEF stringTobool ilSaveImage;
EOS_VAR_DEF tiFunc       ilTexImage;
EOS_VAR_DEF cpFunc       ilCopyPixels;


EOS_FUNC bit DevilActive()
{
 if (ilInit) return true;
 
 Dll devil;
 #ifdef EOS_LINUX
  devil.Load("IL");
 #else
  devil.Load("devil");
 #endif
 if (!devil.Active()) return false;
   
 ilInit = (voidTovoid)devil.Get("ilInit");
 ilShutdown = (voidTovoid)devil.Get("ilShutDown");
 ilGetError = (voidToint)devil.Get("ilGetError");
 ilGenImages = (handFunc)devil.Get("ilGenImages");
 ilBindImage = (intTovoid)devil.Get("ilBindImage");
 ilDeleteImages = (handFunc)devil.Get("ilDeleteImages");
 ilEnable = (intTobool)devil.Get("ilEnable");
 ilDisable = (intTobool)devil.Get("ilDisable");
 ilGetInteger = (intToint)devil.Get("ilGetInteger");
 ilOriginFunc = (intTobool)devil.Get("ilOriginFunc");
 ilLoadImage = (stringTobool)devil.Get("ilLoadImage");
 ilSaveImage = (stringTobool)devil.Get("ilSaveImage");
 ilTexImage = (tiFunc)devil.Get("ilTexImage");
 ilCopyPixels = (cpFunc)devil.Get("ilCopyPixels");
   
 if ((ilInit==0)||(ilShutdown==0)||(ilGetError==0)||(ilGenImages==0)||
     (ilBindImage==0)||(ilDeleteImages==0)||(ilEnable==0)||(ilDisable==0)||
     (ilGetInteger==0)||(ilOriginFunc==0)||(ilLoadImage==0)||(ilSaveImage==0)||
     (ilTexImage==0)||(ilCopyPixels==0))
 {
  ilInit = 0;
  return false;	 
 }
 
 devil.UnloadKeep();
 ilInit();
 return true;	
}

//------------------------------------------------------------------------------
 };
};
