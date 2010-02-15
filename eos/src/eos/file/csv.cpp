//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/file/csv.h"

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
Csv::Csv(cstrconst fn,bit overwrite)
:startOfRow(true),file(fn,overwrite?way_ow:way_append,mode_write)
{}

Csv::Csv(const str::String & fn,bit overwrite)
:startOfRow(true),file(fn,overwrite?way_ow:way_append,mode_write)
{}

Csv::Csv(const Dir & dir,cstrconst fn,bit overwrite)
:startOfRow(true),file(dir,fn,overwrite?way_ow:way_append,mode_write)
{}

Csv::Csv(const Dir & dir,const str::String & fn,bit overwrite)
:startOfRow(true),file(dir,fn,overwrite?way_ow:way_append,mode_write)
{}

Csv::~Csv()
{}

bit Csv::Active()
{
 return file.Active();
}

nat32 Csv::Write(const void * inv,nat32 bytes)
{
 const byte * in = (byte*)inv;

 if (startOfRow)
 {
  startOfRow = false;
  file.GetCursor(file.Size()).Write("\"",1);
 }

 nat32 ret = 0;
 nat32 base = 0;
  for (nat32 i=0;i<bytes;i++)
  {
   if (in[i]=='"')
   {
    ret += file.GetCursor(file.Size()).Write(in+base,i-base);
    file.GetCursor(file.Size()).Write("\\\"",2);
    base = i;
   }
   else if (in[i]=='\n')
   {
    ret += file.GetCursor(file.Size()).Write(in+base,i-base);
    file.GetCursor(file.Size()).Write("\\n",2);
    base = i;    
   }
   else if (in[i]=='\\')
   {
    ret += file.GetCursor(file.Size()).Write(in+base,i-base);
    file.GetCursor(file.Size()).Write("\\\\",2);
    base = i;    
   }   
  }
 ret += file.GetCursor(file.Size()).Write(in+base,bytes-base);
 return ret;
}

nat32 Csv::Pad(nat32 bytes)
{
 return file.GetCursor(file.Size()).Pad(bytes);
}

void Csv::FieldEnd()
{
 if (startOfRow)
 {
  startOfRow = false;
  file.GetCursor(file.Size()).Write("\"",1);
 }
 file.GetCursor(file.Size()).Write("\",\"",3);
}

void Csv::RowEnd()
{
 if (startOfRow) file.GetCursor(file.Size()).Write("\"",1); 
 file.GetCursor(file.Size()).Write("\"\n",2);
 startOfRow = true;
}

void Csv::Flush()
{
 file.Flush();
}

//------------------------------------------------------------------------------
 };
};
