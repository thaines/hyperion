#ifndef EOS_MEM_PREEMPT_H
#define EOS_MEM_PREEMPT_H
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


/// \file preempt.h
/// This contains a system for providing memory blocks quickly, works by 
/// declaring large blocks of memory then dishing out chunks as required.
/// A default set of standard memory block providers are created automatically,
/// and used by certain parts of the system for speed.

#include "eos/types.h"
#include "eos/mem/alloc.h"

namespace eos
{
 namespace mem
 {
//------------------------------------------------------------------------------
/// This is declared with a particular block size and a particular number of 
/// blocks, it then dishes these blocks out as requested. Does this using one
/// large memory block, for speed. In the event of more blocks being requested
/// than it can provide it simply falls back to malloc.
/// Templated by block size then block count. Results in BS*BC bytes of memory
/// being permanatly used. BS must be 8 or greater and should be a multiple of 8.
/// BS is the size of the memory blocks it provides, BC is the number of these 
/// blocks allocated in advance.
template <nat32 BS,nat32 BC>
class EOS_CLASS PreAlloc
{
 private:
  struct Node
  {
   Node * next;
   byte extra[BS-sizeof(Node*)];
  };
 
  Node * data;
  Node * top; // Top of a stack of nodes that are avaliable.
  
 public:
  /// &nbsp;
   PreAlloc()
   {
    data = eos::mem::Malloc<Node>(BC);
    top = data;
    for (nat32 i=0;i<BC-1;i++) data[i].next = &data[i+1];    
    data[BC-1].next = null<Node*>();
   }
  
  /// &nbsp;
   ~PreAlloc()
   {
    while (top)
    {
     if ((top<(data+BC))&&(top>=data)) top = top->next;
     else
     {
      Node * toDie = top;
      top = top->next;
      eos::mem::Free(toDie);
     }
    }
    eos::mem::Free(data);
   }
 
  /// This mallocs a pointer of the requested size, templated by return type and
  /// designed to return null if the type you give it is bigger than what 
  /// this returns.
   template <typename T>
   T * Malloc()
   {
    T * ret;
    if (sizeof(T)<=sizeof(Node))
    {
     if (top)
     {
      ret = (T*)(void*)top;
      top = top->next;	     
     }
     else
     {
      ret = (T*)(void*)eos::mem::Malloc<Node>();
     }	    
    } else ret = null<T*>();
    return ret;	   
   }
   
  /// Frees a memory block that was allocated by this object.
   template <typename T>
   void Free(T * ptr)
   {
    if (ptr)
    {
     Node * d = (Node*)(void*)ptr;
     d->next = top;
     top = d;
    }
   }
};

//------------------------------------------------------------------------------
/// Size of the 8 byte system wide memory allocator.
static const nat32 pre8_size = 128*1024;

/// A system wide memory allocator for 8 byte structures.
EOS_VAR PreAlloc<8,pre8_size> pre8;

/// Size of the 16 byte system wide memory allocator.
static const nat32 pre16_size = 128*1024;

/// A system wide memory allocator for 16 byte structures.
EOS_VAR PreAlloc<16,pre16_size> pre16;

/// Size of the 32 byte system wide memory allocator.
static const nat32 pre32_size = 32*1024; 

/// A system wide memory allocator for 32 byte structures.
EOS_VAR PreAlloc<32,pre32_size> pre32;

/// Size of the 64 byte system wide memory allocator.
static const nat32 pre64_size = 16*1024; 

/// A system wide memory allocator for 64 byte structures.
EOS_VAR PreAlloc<64,pre64_size> pre64;

//------------------------------------------------------------------------------
 };
};
#endif
