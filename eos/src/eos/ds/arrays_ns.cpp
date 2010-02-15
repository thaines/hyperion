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

#include "eos/ds/arrays_ns.h"

#include "eos/mem/functions.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
ArrayNSCode::ArrayNSCode(nat32 elementSize,const ArrayNSCode & rhs)
:elements(rhs.elements),dataSize(rhs.elements),data(mem::Malloc<byte>(elementSize*rhs.elements))
{
 mem::Copy(data,rhs.data,elementSize*rhs.elements);
}

void ArrayNSCode::Copy(nat32 elementSize,const ArrayNSCode & rhs)
{
 elements = rhs.elements;
 if (elements>dataSize)
 {
  mem::Free(data);
  data = mem::Malloc<byte>(elementSize*elements);
 }

 mem::Copy(data,rhs.data,elementSize*elements);
}

nat32 ArrayNSCode::Resize(nat32 elementSize,nat32 newSize)
{
 if (newSize>dataSize)
 {
  byte * newData = mem::Malloc<byte>(elementSize*newSize);
  mem::Copy(newData,data,elementSize*elements);
 
  mem::Free(data);
  data = newData;
  
  dataSize = newSize;  
 }

 elements = newSize;
 return newSize;
}

//------------------------------------------------------------------------------
 };
};
