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

#include "eos/ds/nth.h"

#include "eos/math/functions.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
MultiNth::MultiNth(nat32 s,nat32 n)
:sets(0),nth(0),min(math::Infinity<real32>()),
size(null<nat32*>()),data(null<real32*>())
{
 Reset(s,n);
}

MultiNth::~MultiNth()
{
 mem::Free(size);
 mem::Free(data);
}

void MultiNth::Reset(nat32 s,nat32 n)
{
 n += 1;

 if (s!=sets)
 {
  mem::Free(size);
  size = mem::Malloc<nat32>(s);
 }
 
 if ((s*n)!=(sets*nth))
 {
  mem::Free(data);
  data = mem::Malloc<real32>(s*n);
 }
 
 sets = s;
 nth = n;

 min = math::Infinity<real32>();
 for (nat32 i=0;i<sets;i++) size[i] = 0;
}

void MultiNth::Add(nat32 set,real32 num)
{
 // Break out quick for irrelevant numbers...
  if (num>min) return;
  
 // Grab the offset pointer to the relevant mini-heap, such that the first entry
 // is at index 1...
  real32 * heap = data + set*nth - 1;
 
 // Two different code paths, depending on if the sets heap is full or not...
  if (size[set]==nth)
  {
   // Heap is full - by invariant and num<=min check the given number must be 
   // smaller or equal to the head of the heap - replace the head and re-heapify...
   // If no swaps are required then an update of min might be required.
    nat32 loc = 1;
    heap[loc] = num;
    
    while (true)
    {
     // Find the largest child...
      nat32 indLeft = loc*2;
      if (indLeft>nth) break;
      nat32 indRight = indLeft+1;
      
      nat32 nLoc = indLeft;
      if ((indRight<=nth)&&(heap[indRight]>heap[indLeft])) nLoc = indRight;
      
     // If largest child is larger than the current node swap, else break...
      if (heap[nLoc]>heap[loc])
      {
       math::Swap(heap[loc],heap[nLoc]);
       loc = nLoc;
      }
      else break;
    }
    
    min = math::Min(min,heap[1]);
  }
  else
  {
   // Heap not full, add new number in and swap to correct position...
    size[set] += 1;
    nat32 loc = size[set];
    heap[loc] = num;
    
    while (loc!=1)
    {
     nat32 parent = loc/2;
     if (heap[loc]>heap[parent])
     {
      math::Swap(heap[parent],heap[loc]);
      loc = parent;
     }
     else break;
    }
    
   // Check if heap is now full, if so update min accordingly...
    if (size[set]==nth) min = math::Min(min,heap[1]);
  }
}

real32 MultiNth::Result() const
{
 return min;
}

//------------------------------------------------------------------------------
 };
};
