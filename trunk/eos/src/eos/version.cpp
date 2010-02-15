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

#include "eos/version.h"

namespace eos
{
//------------------------------------------------------------------------------
EOS_FUNC void GetVersion(Version & out)
{
 out.interface = EOS_VERSION_INTERFACE;
 out.release = EOS_VERSION_RELEASE;
 out.minor = EOS_VERSION_MINOR;
}

EOS_FUNC cstrconst GetCopyright()
{
 return "Copyright 2005-2008 Tom Haines";
}

EOS_FUNC cstrconst GetBuild()
{
 return "eos, version " EOS_VERSION_STRING ", compiled " __DATE__ " " __TIME__;
}

//------------------------------------------------------------------------------
EOS_FUNC bit IsDebug()
{
 #ifdef EOS_DEBUG
 return true;
 #endif
 
 #ifdef EOS_RELEASE
 return false;
 #endif 
}
EOS_FUNC Os GetOs()
{
 #ifdef EOS_WIN32
 return win32;
 #endif
 
 #ifdef EOS_LINUX
 return lin;
 #endif      
}

EOS_FUNC Chipset GetChipset()
{
 #ifdef EOS_64BIT
 return x86_64;
 #else
 return x86;
 #endif
}

EOS_FUNC bit WithAsm()
{
 #ifdef EOS_ASM
 return true;
 #else
 return false;
 #endif       
}

//------------------------------------------------------------------------------
};
