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

#include "eos/str/strings.h"

namespace eos
{
 namespace str
 {
//------------------------------------------------------------------------------
bit String::StartsWith(cstrconst s) const
{
 Cursor cur = const_cast<String*>(this)->GetCursor(0);

 while (s[0]!=0)
 {
  cstrchar b;
  if (cur.Read(&b,1)!=1) return false;
  if (b!=s[0]) return false;
  ++s;
 }
 
 return true;
}

bit String::EndsWith(cstrconst s) const
{
 nat32 sLen = str::Length(s);
 if (Size()<sLen) return false;
 
 Cursor cur = const_cast<String*>(this)->GetCursor(Size()-sLen);
 while (s[0]!=0)
 {
  cstrchar b;
  if (cur.Read(&b,1)!=1) return false;
  if (b!=s[0]) return false;
  ++s;
 }
 
 return true;
}

bit String::GetLine(io::InVirt<io::Text> & source)
{
 // Empty this class...
  SetSize(0);
  
 // Peek then read a block at a time until we collide with a '\n', end of file or '\r'...
  static const nat32 buf_size = 128;
  cstrchar buf[buf_size];
  while (true)
  {
   nat32 num = source.Peek(buf,buf_size);
   if (num==0) break;
   
   nat32 toDo = 0;
   for (nat32 i=0;i<num;i++)
   {
    if ((buf[toDo]=='\n')||(buf[toDo]=='\r')) break;
    toDo += 1;
   }
   
   source.Skip(toDo);
   Write(buf,toDo);
   
   if (toDo!=num) break;
  }
 
 // When one of '\n' or '\r' stops it check for the other being the second
 // character, and eat accordingly...
  nat32 tail = source.Peek(buf,2);
  if (tail==0)
  {
   if (Size()==0) return false;
  }
  else
  {
   if (tail==1) source.Skip(1);
   else
   {
    if (buf[0]=='\n')
    {
     if (buf[1]=='\r') source.Skip(2);
                  else source.Skip(1);
    }
    else
    {
     if ((buf[0]=='\n')&&(buf[1]=='\n')) source.Skip(2);
                                    else source.Skip(1);
    }
   }
  }
 return true;
}

bit String::GetLine(io::InVirt<io::Binary> & source,bit noR)
{
 // Empty this class...
  SetSize(0);
  
 // Peek then read a block at a time until we collide with a '\n', end of file or '\r'...
  static const nat32 buf_size = 128;
  cstrchar buf[buf_size];
  while (true)
  {
   nat32 num = source.Peek(buf,buf_size);
   if (num==0) break;
   
   nat32 toDo = 0;
   for (nat32 i=0;i<num;i++)
   {
    if ((buf[toDo]=='\n')||((!noR)&&(buf[toDo]=='\r'))) break;
    toDo += 1;
   }
   
   source.Skip(toDo);
   Write(buf,toDo);
   
   if (toDo!=num) break;
  }
 
 // When one of '\n' or '\r' stops it check for the other being the second
 // character, and eat accordingly...
  nat32 tail = source.Peek(buf,2);
  if (tail==0)
  {
   if (Size()==0) return false;
  }
  else
  {
   if (tail==1) source.Skip(1);
   else
   {
    if (buf[0]=='\n')
    {
     if ((!noR)&&(buf[1]=='\r')) source.Skip(2);
                            else source.Skip(1);
    }
    else
    {
     if ((buf[0]=='\n')&&(buf[1]=='\n')) source.Skip(2);
                                    else source.Skip(1);
    }
   }
  }
 return true;
}

cstr String::ToStr() const
{	
 cstr ret = mem::Malloc<cstrchar>(Size()+1);
  Cursor cur = const_cast<String*>(this)->GetCursor();
  cur.Read(ret,Size());
  ret[Size()] = 0;   
 return ret;
}

int32 String::ToInt32() const // Slow implimentation, fix when matters.
{
 cstr str = ToStr();
 int32 ret = str::ToInt32(str);
 mem::Free(str);
 return ret;
}

real32 String::ToReal32() const // Slow implimentation, fix when matters.
{
 cstr str = ToStr();
 real32 ret = str::ToReal32(str);
 mem::Free(str);
 return ret;
}

bit String::ToBit() const
{
 bit ret = false;
  if (Size()==4)
  {
   cstrchar buf[4];
   Cursor cur = const_cast<String*>(this)->GetCursor();
   cur.Read(buf,Size());
   
   if (((buf[0]=='t')||(buf[0]=='T'))&&
       ((buf[1]=='r')||(buf[1]=='R'))&&
       ((buf[2]=='u')||(buf[2]=='U'))&&
       ((buf[3]=='e')||(buf[3]=='E'))) ret = true;
  }
  else if (Size()==1)
  {
   cstrchar buf[1];
   Cursor cur = const_cast<String*>(this)->GetCursor();
   cur.Read(buf,Size());
   if ((buf[0]=='t')||
       (buf[0]=='T')||
       (buf[0]=='1')) ret = true;
  }
 return ret;       
}

//------------------------------------------------------------------------------
 };
};
