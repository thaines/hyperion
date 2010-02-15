#ifndef EOS_MEM_PACKER_H
#define EOS_MEM_PACKER_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

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


/// \file packer.h
/// Abstracts the idea of declaring a single memory block and then splitting it
/// up for many tasks. When that block is used up a new block is allocated and 
/// it continues. Frees all memory only when the object is destroyed - there is
/// no free method. Ideal for a certain class of heavy-weight algorithm.

#include "eos/types.h"

namespace eos
{
 namespace mem
 {
//------------------------------------------------------------------------------
/// This impliments the idea of code that wants to declare lots of small chunks
/// of memory quickly, do some work, then free them all at once.
/// This fits a lot of heavy algorithms.
/// You choose a block size, it then eats out of the block for each allocation
/// until the next allocation would pass the end of the block. It then allocates
/// a new block and uses that up, and so on.
/// There is no free, all the memory is freeded when you call reset or the
/// object is deleted.
/// Obviously the size of the blocks must greatly exceed the size of requested 
/// blocks, otherwise this class is pointless. (And fatal, if its doesn't at 
/// least exceed.)
class EOS_CLASS Packer
{
 public:
  /// Sets the size of each new block.
   Packer(nat32 blockSize);
   
  /// &nbsp;
   ~Packer();
     
  /// Resets it, freeing all memory and preparing it to start again.
   void Reset();


  /// Returns a new memory block, num is the number wanted.
   template <typename T>
   T * Malloc(nat32 num = 1)
   {
    return (T*)NewBlock(sizeof(T)*num);
   }


  /// This returns a pointer to a memory block, of which you can use upto the
  /// amount requested. The idea is you might not need this much however, so 
  /// after discovering how much you do need, and probably filling in that
  /// memory, you then call Loc with the final block size, to confirm the
  /// final usage. During this two part allocation no other methods for this
  /// object may be called.
   template <typename T>
   T * Mal(nat32 num = 1)
   {
    T * ret = (T*)NewBlock(sizeof(T)*num);
    offset -= sizeof(T)*num;
    return ret;
   }
   
  /// Second part to Mal, see Mal for details.
   template <typename T>
   void Loc(nat32 num = 1)
   {
    offset += sizeof(T)*num;
   }


 private:
  nat32 blockSize;
  byte * top; // First 4 bytes of each block are eatten by pointer to previous block. For destruction.
  nat32 offset;
  
  void * NewBlock(nat32 size);
};

//------------------------------------------------------------------------------
 };
};
#endif
