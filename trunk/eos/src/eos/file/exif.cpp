//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines
#include "eos/file/exif.h"

#include "eos/mem/alloc.h"
#include "eos/mem/safety.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
Exif::Exif()
{}

Exif::~Exif()
{
 Reset();
}

bit Exif::Load(cstrconst fn)
{
 Reset();

 File<io::Binary> fo;
 if (fo.Open(fn,way_edit,mode_read)==false) {LogError("Exif: Can't open file"); return false;}
 
 Cursor<io::Binary> f = fo.GetCursor();
 
 // Read in the jpeg header...
  byte jpegHead[2];
  if (f.Read(jpegHead,2)!=2) {LogError("Exif: Can't read jpeg header"); return false;}
  if ((jpegHead[0]!=0xFF)||(jpegHead[1]!=0xD8)) {LogError("Exif: Jpeg header is wrong"); return false;}
 
 // Now check if exif data exists...
  byte exifHead[2];
  if (f.Read(exifHead,2)!=2) {LogError("Exif: Can't read exif header"); return false;}
  if ((exifHead[0]!=0xFF)||(exifHead[1]!=0xE1)) {LogError("Exif: Exif header is wrong"); return false;}
  
 // Get its size - Motorola format...
  nat16 size;
  if (f.Read(&size,2)!=2) {LogError("Exif: Can't read exif size"); return false;}
  size = io::FromBigEndian(size);
  if (size<2) {LogError("Exif: Read exif size impossibly small"); return false;}
  size -= 2; // For the data already read.
 
 // Get the exif identifier... 
  byte exifIdent[6];
  if (f.Read(exifIdent,6)!=6) {LogError("Exif: Exif ident can't be read"); return false;}
  if ((exifIdent[0]!='E')||
      (exifIdent[1]!='x')||
      (exifIdent[2]!='i')||
      (exifIdent[3]!='f')||
      (exifIdent[4]!=0)||
      (exifIdent[5]!=0)) {LogError("Exif: Exif ident is wrong"); return false;}
  if (size<6) {LogError("Exif: Read exif size to small"); return false;}
  size -= 6;
  
 // Now, because we are going to have to do offsets, we read in the entire
 // block, such that indexes into this block match the offsets...
  mem::StackPtr<byte,mem::KillFree<byte> > tiff = mem::Malloc<byte>(size);
  if (f.Read(tiff.Ptr(),size)!=size) {LogError("Exif: Failed to read exif block"); return false;}
  
  nat32 offset = 0;


 // Read in the tiff header - note the byte convention...
  // Endiness...
   if (tiff[offset]!=tiff[offset+1]) {LogError("Exif: Bad endiness indicator"); return false;}
   motorola = false;
   if (tiff[offset]!='I')
   {
    if (tiff[offset]=='M') motorola = true;
                      else {LogError("Exif: Unrecognised endiness"); return false;}
   }
   offset += 2;
   
  // 42...
   if (io::ToCurrent(io::Nat16FromBS(&tiff[offset]),motorola)!=42)
      {LogError("Exif: The tiff header has no 42"); return false;}
   offset += 2;
   
  // Offset to first IFD...
   offset = io::ToCurrent(io::Nat32FromBS(&tiff[offset]),motorola);


 // We now need to process all IFD's...
  bit doneSub = false;
  while (true)
  {
   // Get the number of tags...
    nat16 count = io::ToCurrent(io::Nat16FromBS(&tiff[offset]),motorola);
    offset += 2;
    if (offset+count*12+4>=size)
       {LogError("Exif:IFD data outside of data block"); Reset(); return false;}
  
   // Now process each tag, adding them each to the array...
    for (nat32 i=0;i<count;i++)
    {
     nat32 to = tags.Size();
     tags.Size(to+1);
     tags[to].data = null<byte*>();
  
     // Read tag number, format and component count...
      tags[to].tag = io::ToCurrent(io::Nat16FromBS(&tiff[offset]),motorola); offset += 2;
      tags[to].type = io::ToCurrent(io::Nat16FromBS(&tiff[offset]),motorola); offset += 2;
      tags[to].count = io::ToCurrent(io::Nat32FromBS(&tiff[offset]),motorola); offset += 4;
    
      if ((tags[to].type==0)||(tags[to].type>12))
         {LogError("Exif: Unrecognised tag type - " << tags[to].type); Reset(); return false;}
      if (tags[to].count==0) {LogError("Exif: Tag entry count of 0 makes no sense"); Reset(); return false;}
    
      static const nat32 type_size[13] = {0,1,1,2,4,8,1,1,2,4,8,4,8};
      nat32 data_size = type_size[tags[to].type] * tags[to].count;
   
     // Now handle the fiddly data/pointer system...
      tags[to].data = mem::Malloc<byte>(data_size);
      if (data_size<=4)
      {
       mem::Copy(tags[to].data,&tiff[offset],data_size);
      }
      else
      {
       nat32 pd = io::ToCurrent(io::Nat32FromBS(&tiff[offset]),motorola);
       if (pd+data_size>=size) {LogError("Exif: Tag data index outside of data block"); Reset(); return false;}
       mem::Copy(tags[to].data,&tiff[pd],data_size);
      }
      offset += 4;
    }
   
   // Either move to the sub IFD or quit...
    if (doneSub) break;
    else
    {
     doneSub = true;

     // Need to do the sub-exif IFD...
      offset = 0;
      for (nat32 i=0;i<tags.Size();i++)
      {
       if (tags[i].tag==0x8769)
       {
        if ((tags[i].type!=4)||(tags[i].count!=1))
           {LogError("Exif: ExifOffset is badly formated"); Reset(); return false;}
        offset = io::ToCurrent(io::Nat32FromBS(tags[i].data),motorola);
        break;
       }
      }
      if (offset==0) break;
    }
  }


 return true;
}

bit Exif::Load(const str::String & fn)
{
 cstr fnStr = fn.ToStr();
 bit ret = Load(fnStr);
 mem::Free(fnStr);
 return ret;
}

int32 Exif::GetName(nat16 name) const
{
 for (nat32 i=0;i<tags.Size();i++)
 {
  if (tags[i].tag==name) return i;
 }
 return -1;
}

void Exif::Reset()
{
 for (nat32 i=0;i<tags.Size();i++) mem::Free(tags[i].data);
 tags.Size(0);
}

//------------------------------------------------------------------------------
 };
};
