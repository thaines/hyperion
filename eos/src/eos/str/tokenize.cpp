//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/str/tokenize.h"

#include "eos/str/functions.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace str
 {
//------------------------------------------------------------------------------
Tokenize::Tokenize()
:ts(null<cstr>())
{}

Tokenize::Tokenize(cstrconst s,cstrconst del)
{
 ts = str::Duplicate(s);
 Setup(del);
}

Tokenize::Tokenize(const str::String & s,cstrconst del)
{
 ts = s.ToStr();
 Setup(del);
}

Tokenize::~Tokenize()
{
 mem::Free(ts);
}

void Tokenize::Replace(cstrconst s,cstrconst del)
{
 mem::Free(ts);
 ts = str::Duplicate(s);
 Setup(del);
}

void Tokenize::Replace(const str::String & s,cstrconst del)
{
 mem::Free(ts);
 ts = s.ToStr();
 Setup(del);
}

nat32 Tokenize::Size() const
{
 return toks.Size();
}

cstrconst Tokenize::operator[] (nat32 i) const
{
 return ts + toks[i];
}

void Tokenize::Setup(cstrconst del)
{
 LogTime("eos::str::Tokenize::Setup");
 // Create bit map to indicate if each character is a delimeter...
  bit map[256];
  for (nat32 i=0;i<256;i++) map[i] = false;
  for (nat32 i=0;del[i]!=0;i++) map[nat32(del[i])] = true;


 // First pass to count the tokens and null all delimeters...
  nat32 count = 0;
  cstr targ = ts;
  bit lastDel = true;
  while (targ[0]!=0)
  {
   if (map[nat32(targ[0])])
   {
    lastDel = true;
    targ[0] = 0;
   }
   else
   {
    if (lastDel) count += 1;
    lastDel = false;
   }
   ++targ;
  }


 // Second pass to build an index to the tokens...
  toks.Size(count);
  targ = ts;
  for (nat32 i=0;;i++)
  {
   // Skip nulls...
    while (targ[0]==0) ++targ;

   // Store the pos...
    toks[i] = targ-ts;

   // Skip to dels if not last...
    if (i+1==count) break;
    while (targ[0]!=0) ++targ;
  }
}

//------------------------------------------------------------------------------
 };
};
