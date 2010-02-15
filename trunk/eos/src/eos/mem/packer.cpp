//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/mem/packer.h"

namespace eos
{
 namespace mem
 {
//------------------------------------------------------------------------------
Packer::Packer(nat32 bs)
:blockSize(bs),top(null<byte*>()),offset(bs)
{}

Packer::~Packer()
{
 Reset();
}

void Packer::Reset()
{
 while (top)
 {
  byte * victim = top;
  top = *(byte**)(void*)victim;
  mem::Free(victim);
 }
 offset = blockSize;
}

void * Packer::NewBlock(nat32 size)
{
 log::Assert(size<(blockSize-sizeof(byte*)));
 if (offset+size<=blockSize)
 {
  void * ret = top+offset;
  offset += size;
  return ret;
 }
 else
 {
  byte * newTop = mem::Malloc<byte>(blockSize);
  *((byte**)(void*)newTop) = top;
  top = newTop;
  offset = sizeof(byte*) + size;
  return top + sizeof(byte*);
 }
}

//------------------------------------------------------------------------------
 };
};
