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

#include "eos/str/tokens.h"

#include "eos/mem/alloc.h"
#include "eos/str/functions.h"

#include "eos/file/csv.h" // Delete me!

namespace eos
{
 namespace str
 {
//------------------------------------------------------------------------------
TokenTable::TokenTable()
{
 str[0] = 0; str[1] = 0; str[2] = 0; str[3] = 0;
 max = 0;
 top = new Node[256];

 (*this)("NullToken"); // So requests of NullToken return something recognisable as such.
}

TokenTable::~TokenTable()
{
 // This kills the str data structure.
 // If you ask how it works, I will have no choice but to apply it, to you...
  nat8 depth = 3;
  nat16 width[4] = {1,1,1,0};

  while(str[depth])
  {
   if (depth==3)
   {
    if (str[depth][width[depth]])
    {
     mem::Free(str[depth][width[depth]]);
     width[depth]++;
    }
    else
    {
     mem::Free(str[depth]);
     depth--;
     width[depth]++;
    }
   }
   else
   {
    if (str[depth][width[depth]])
    {
     str[depth+1] = (cstr*)str[depth][width[depth]];
     depth++;
     width[depth] = 0;
    }
    else
    {
     mem::Free(str[depth]);
     depth--;
     width[depth]++;
    }
   }

   while (width[depth]==256)
   {
    mem::Free(str[depth]);
    depth--;
    width[depth]++;
   }
  }

 delete[] top;
}

Token TokenTable::operator()(cstrconst s) const
{
 // First calculate a basic hash value - a sum of the bytes in the string...
  nat8 hash = 0;
  cstrconst targ = s;
  while (*targ) {hash += (nat8)*targ; targ++;}

 // Now we get its node...
  nat32 depth = 0;
  Node * node = top[hash].Get(s,depth);

 // Detect if its a new node, and setup as needed...
  if (node->str==0)
  {
   cstr str = str::Duplicate(s);
   node->str = str+depth;
   node->num = AssignNum(str);
  }

 // Return the value req...
  return node->num;
}

Token TokenTable::operator()(nat32 length,cstrconst s) const
{
 // First calculate a basic hash value - a sum of the bytes in the string...
  nat8 hash = 0;
  for (nat32 i=0;i<length;i++) {hash += (nat8)s[i];}

 // Now we get its node...
  nat32 depth = 0;
  Node * node = top[hash].Get(length,s,depth);

 // Detect if its a new node, and setup as needed...
  if (node->str==0)
  {
   cstr str = mem::Malloc<cstrchar>(length+1);
    mem::Copy(str,s,length);
    str[length] = 0;
   node->str = str+depth;
   node->num = AssignNum(str);
  }

 // Return the value req...
  return node->num;
}

Token TokenTable::operator()(const String & rhs) const // Inefficient by laziness, set on fire when needed/bored.
{
 cstr str = rhs.ToStr();
  Token ret = (*this)(str);
 mem::Free(str);
 return ret;
}

Token TokenTable::operator()(const String::Cursor & rhs) const // Slow.
{
 cstr s = mem::Malloc<cstrchar>(rhs.Avaliable()+1);
 rhs.Peek(s,rhs.Avaliable());
 s[rhs.Avaliable()] = 0;
 
 Token ret = (*this)(s);
 
 mem::Free(s);
 
 return ret;
}

cstrconst TokenTable::Str(Token num) const
{
 cstrconst ret;
 if (num>=max) ret = null<cstrconst>();
 else
 {
  if (!(num&0xFFFFFF00)) ret = str[3][num];
  else
  {
   if (!(num&0xFFFF0000)) ret = ((cstr*)str[2][num>>8])[num&0xFF];
   else
   {
    if (!(num&0xFF000000)) ret = ((cstr*)((cstr*)str[1][num>>16])[(num>>8)&0xFF])[num&0xFF];
    else ret = ((cstr*)((cstr*)((cstr*)str[0][num>>24])[(num>>16)&0xFF])[(num>>8)&0xFF])[num&0xFF];
   }
  }
 }
 return ret;
}

nat32 TokenTable::AssignNum(cstr s) const
{
 if (max<=0xFF)
 {
  if (str[3]==0)
  {
   const_cast<TokenTable*>(this)->str[3] = mem::Malloc<cstr>(sizeof(cstr)*256);
   mem::Null<cstr>(str[3],256);
  }

  str[3][max] = s;
 }
  else if (max<=0xFFFF)
 {
  if (str[2]==0)
  {
   const_cast<TokenTable*>(this)->str[2] = mem::Malloc<cstr>(256);
   mem::Null<cstr>(str[2],256);
   ((cstr*&)str[2][0]) = str[3];
  }

  if (str[2][max>>8]==0)
  {
   (cstr*&)(str[2][max>>8]) = mem::Malloc<cstr>(256);
   mem::Null<cstr>((cstr*)(str[2][max>>8]),256);
  }

  ((cstr*&)str[2][max>>8])[max&0xFF] = s;
 }
  else if (max<=0xFFFFFF)
 {
  if (str[1]==0)
  {
   const_cast<TokenTable*>(this)->str[1] = mem::Malloc<cstr>(256);
   mem::Null<cstr>(str[1],256);
   ((cstr*&)str[1][0]) = str[2];
  }

  if (str[1][max>>16]==0)
  {
   (cstr*&)(str[1][max>>16]) = mem::Malloc<cstr>(256);
   mem::Null<cstr>((cstr*)(str[1][max>>16]),256);
  }

  if (((cstr*)str[1][max>>16])[(max>>8)&0xFF]==0)
  {
   (cstr*&)(((cstr*)str[1][max>>16])[(max>>8)&0xFF]) = mem::Malloc<cstr>(256);
   mem::Null<cstr>((cstr*)(((cstr*)str[1][max>>16])[(max>>8)&0xFF]),256);
  }

  ((cstr*)((cstr*)str[1][max>>16])[(max>>8)&0xFF])[max&0xFF] = s;
 }
  else
 {
  if (str[0]==0)
  {
   const_cast<TokenTable*>(this)->str[0] = mem::Malloc<cstr>(256);
   mem::Null<cstr>(str[0],256);
   ((cstr*&)str[0][0]) = str[1];
  }

  if (str[0][max>>24]==0)
  {
   (cstr*&)(str[0][max>>24]) = mem::Malloc<cstr>(256);
   mem::Null<cstr>((cstr*)(str[0][max>>24]),256);
  }

  if (((cstr*)str[0][max>>24])[(max>>16)&0xFF]==0)
  {
   (cstr*&)(((cstr*)str[0][max>>24])[(max>>16)&0xFF]) = mem::Malloc<cstr>(256);
   mem::Null<cstr>((cstr*)(((cstr*)str[0][max>>24])[(max>>16)&0xFF]),256);
  }

  if (((cstr*)((cstr*)str[0][max>>24])[(max>>16)&0xFF])[(max>>8)&0xFF]==0)
  {
   (cstr*&)(((cstr*)((cstr*)str[0][max>>24])[(max>>16)&0xFF])[(max>>8)&0xFF]) = mem::Malloc<cstr>(256);
   mem::Null<cstr>((cstr*)(((cstr*)((cstr*)str[0][max>>24])[(max>>16)&0xFF])[(max>>8)&0xFF]),256);
  }

  ((cstr*)((cstr*)((cstr*)str[0][max>>24])[(max>>16)&0xFF])[(max>>8)&0xFF])[max&0xFF] = s;
 }

 return const_cast<TokenTable*>(this)->max++;
}

bit TokenTable::Exists(cstrconst s,Token & out) const
{
 // First calculate a basic hash value - a sum of the bytes in the string...
  nat8 hash = 0;
  cstrconst targ = s;
  while (*targ) {hash += (nat8)*targ; targ++;}

 // Now check for its existance...
  Node * node = top[hash].Exists(s);
  if (node)
  {
   out = node->num;
   return true;
  }
  else return false;
}

//------------------------------------------------------------------------------
TokenTable::Node::Node()
:str(null<cstr>()),child(null<Node*>())
{}

TokenTable::Node::~Node()
{
 delete[] child;
}

TokenTable::Node * TokenTable::Node::Get(nat32 remainder,cstrconst s,nat32 & depth)
{
 if (str==null<cstr>()) return this;
 if (mem::Compare(str,s,remainder)==0) return this;

 if (child==null<Node*>()) child = new Node[256];

 if (remainder==0)
 {
  nat32 d = 1;
  Node * node = child[nat8(*str)].Get(str+1,d);
  node->str = str+d;
  node->num = num;

  str = null<cstr>();
  return this;
 }

 depth++;
 return child[nat8(*s)].Get(remainder-1,s+1,depth);
}

TokenTable::Node * TokenTable::Node::Get(cstrconst s,nat32 & depth)
{
 if (str==null<cstr>()) return this;
 if (str::Compare(str,s)==0) return this;

 if (child==null<Node*>()) child = new Node[256];

 if (*s==0)
 {
  nat32 d = 1;
  Node * node = child[nat8(*str)].Get(str+1,d);
  node->str = str+d;
  node->num = num;

  str = null<cstr>();
  return this;
 }

 depth++;
 return child[nat8(*s)].Get(s+1,depth);
}

TokenTable::Node * TokenTable::Node::Exists(cstrconst s)
{
 if (str::Compare(str,s)==0) return this;
 if (child==null<Node*>()) return null<Node*>();
 if (*s==0) return null<Node*>();

 return child[nat8(*s)].Exists(s+1);
}

//------------------------------------------------------------------------------
 };
};
