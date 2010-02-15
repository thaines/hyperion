//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines

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

#include "eos/ds/arrays2d.h"

#include "eos/math/functions.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
Array2DCode::Array2DCode(nat32 elementSize,const Array2DCode & rhs)
:width(rhs.width),height(rhs.height),data(mem::Malloc<byte>(elementSize*rhs.width*rhs.height))
{
 mem::Copy<byte>(data,rhs.data,elementSize*width*height);
}

void Array2DCode::Del(nat32 elementSize,void (*Term)(void * ptr))
{
 byte * targ = data;
 for (nat32 i=0;i<width*height;i++)
 {
  Term(targ);
  targ += elementSize;	 
 }	
}

void Array2DCode::Copy(nat32 elementSize,const Array2DCode & rhs)
{
 if ((width!=rhs.width)||(height!=rhs.height))
 {
  width = rhs.width;
  height = rhs.height;
  mem::Free(data);
  data = mem::Malloc<byte>(elementSize*width*height);
 }

 mem::Copy<byte>(data,rhs.data,elementSize*width*height);
}

void Array2DCode::Resize(nat32 elementSize, nat32 newW, nat32 newH, void (*Make)(void * ptr), void (*Term)(void * ptr))
{
 if ((width!=newW)||(height!=newH))
 {
  byte * newData = mem::Malloc<byte>(elementSize*newW*newH);
  
  for (nat32 y=0;y<math::Min(height,newH);y++)
  {
   mem::Copy(newData + newW*y*elementSize,data + width*y*elementSize,elementSize*math::Min(width,newW));
   for (nat32 x=newW;x<width;x++) Term(newData + elementSize*x + newW*elementSize*y);
  }
  for (nat32 y=newH;y<height;y++)
  {
   for (nat32 x=0;x<width;x++) Term(newData + elementSize*x + newW*elementSize*y);	  
  }
  
  mem::Free(data);
  data = newData;
  
  for (nat32 y=0;y<math::Min(height,newH);y++)
  {
   for (nat32 x=width;x<newW;x++) Make(newData + elementSize*x + newW*elementSize*y);
  }
  for (nat32 y=height;y<newH;y++)
  {
   for (nat32 x=0;x<newW;x++) Make(newData + elementSize*x + newW*elementSize*y);
  }
  
  width = newW;
  height = newH;
 }	
}

//------------------------------------------------------------------------------
 };
};
