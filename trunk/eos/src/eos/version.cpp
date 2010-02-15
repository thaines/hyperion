//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines
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
