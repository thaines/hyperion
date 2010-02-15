//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines
#include "eos/mem/alloc.h"

namespace eos
{
 namespace mem
 {
//------------------------------------------------------------------------------
EOS_FUNC void * EOS_STDCALL BasicMalloc(nat32 size)
{
 return ::malloc(size);
}

EOS_FUNC void EOS_STDCALL BasicFree(void * ptr)
{
 ::free(ptr);
}

//------------------------------------------------------------------------------
 };
};
