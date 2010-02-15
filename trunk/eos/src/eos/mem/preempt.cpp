//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines
#include "eos/mem/preempt.h"

namespace eos
{
 namespace mem
 {
//------------------------------------------------------------------------------

EOS_VAR_DEF PreAlloc<8,pre8_size> pre8;
EOS_VAR_DEF PreAlloc<16,pre16_size> pre16;
EOS_VAR_DEF PreAlloc<32,pre32_size> pre32;
EOS_VAR_DEF PreAlloc<64,pre64_size> pre64;

//------------------------------------------------------------------------------
 };
};
