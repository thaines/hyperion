#ifndef EOS_FILE_EXIF_H
#define EOS_FILE_EXIF_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

/// \file exif.h
/// Reads a jpeg file, gets its exif data and

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/file/files.h"
#include "eos/ds/arrays_resize.h"
#include "eos/str/strings.h"
#include "eos/io/functions.h"

namespace eos
{
 namespace file
 {
//------------------------------------------------------------------------------
/// Loads a jpeg file and provides access to its exif data - provides a
/// conveniant interface for a subset of tags that I tend to find useful.
class EOS_CLASS Exif
{
 public:
  /// &nbsp;
   Exif();

  /// &nbsp;
   ~Exif();


  /// Loads a file and copies its (simple) exif info into this class, returns 
  /// true on success, false on failure.
   bit Load(cstrconst fn);
   
  /// Loads a file and copies its (simple) exif info into this class, returns 
  /// true on success, false on failure.
   bit Load(const str::String & fn);


  /// Returns how many tags have been loaded - good for checking that something exists.
   nat32 TagCount() const {return tags.Size();}
   
  /// Returns the index of the given name if it exists, or -1 if it doesn't.
   int32 GetName(nat16 name) const;
   
  /// Returns the tag at the given index.
   nat16 Name(nat32 ind) const {return tags[ind].tag;}

  /// Returns the type at the given index.
   nat16 Type(nat32 ind) const {return tags[ind].type;}
   
  /// Returns the count at the given index.
   nat32 Count(nat32 ind) const {return tags[ind].count;}
   
  /// Returns a type 1, a byte.
   byte GetByte(nat32 ind,nat32 which=0) const {return tags[ind].data[which];}
   
  /// Returns a type 2, a string.
   cstrconst GetStr(nat32 ind) const {return cstrconst(tags[ind].data);}
   
  /// Returns a type 3, a nat16.
   nat16 GetNat16(nat32 ind,nat32 which=0) const 
   {return io::ToCurrent(io::Nat16FromBS(tags[ind].data+2*which),motorola);}
   
  /// Returns a type 4, a nat32.
   nat32 GetNat32(nat32 ind,nat32 which=0) const 
   {return io::ToCurrent(io::Nat32FromBS(tags[ind].data+4*which),motorola);}
   
  /// Returns a type 5, an unsigned rational, as a float.
   real32 GetUR(nat32 ind,nat32 which=0) const
   {
    nat32 a = io::ToCurrent(io::Nat32FromBS(tags[ind].data+8*which),motorola);
    nat32 b = io::ToCurrent(io::Nat32FromBS(tags[ind].data+4+8*which),motorola);
    return real32(a)/real32(b);
   }

  /// Returns a type 6, an int8.
   int8 GetInt8(nat32 ind,nat32 which=0) const {return *(int8*)(void*)&tags[ind].data[which];}
   
  /// Returns a type 8, an int16.
   int16 GetInt16(nat32 ind,nat32 which=0) const 
   {return io::ToCurrent(io::Int16FromBS(tags[ind].data+2*which),motorola);}

  /// Returns a type 9, an int32.
   int32 GetInt32(nat32 ind,nat32 which=0) const 
   {return io::ToCurrent(io::Int32FromBS(tags[ind].data+4*which),motorola);}

  /// Returns a type 10, a signed rational, as a float.
   real32 GetSR(nat32 ind,nat32 which=0) const
   {
    int32 a = io::ToCurrent(io::Int32FromBS(tags[ind].data+8*which),motorola);
    int32 b = io::ToCurrent(io::Int32FromBS(tags[ind].data+4+8*which),motorola);
    return real32(a)/real32(b);
   }
   

  /// Returns true if camera make is avaliable.
   bit HasMake() const {return GetName(0x010f)!=-1;}
   
  /// If HasMake then this returns it.
   cstrconst GetMake() const {return GetStr(GetName(0x010f));}

  /// Returns true if camera model is avaliable.
   bit HasModel() const {return GetName(0x0110)!=-1;}
   
  /// If HasModel then this returns it.
   cstrconst GetModel() const {return GetStr(GetName(0x0110));}

  /// Returns true if the date time string of when the photo was taken is avaliable.
   bit HasDateTime() const {return GetName(0x0132)!=-1;}
   
  /// If HasDateTime then this returns it.
   cstrconst GetDateTime() const {return GetStr(GetName(0x0132));}

  /// Returns true if exposure time is avaliable.
   bit HasExposureTime() const {return GetName(0x829a)!=-1;}
   
  /// If HasExposureTime() is true you can call this to get it in seconds.
   real32 GetExposureTime() const {return GetUR(GetName(0x829a));}

  /// Returns true if F-Stop is avaliable.
   bit HasFStop() const {return GetName(0x829d)!=-1;}
   
  /// If HasFStop() is true you can call this to get it.
   real32 GetFStop() const {return GetUR(GetName(0x829d));}

  /// Returns true if ISO is avaliable.
   bit HasISO() const {return GetName(0x8827)!=-1;}
   
  /// If HasISO() is true you can call this to get it.
   nat16 GetISO() const {return GetNat16(GetName(0x8827));}
   
  /// Returns true if Focal Length (Zoom) is avaliable.
   bit HasFocalLength() const {return GetName(0x920a)!=-1;}
   
  /// If HasFocalLength() is true you can call this to get it, unit is milli-meter.
   real32 GetFocalLength() const {return GetUR(GetName(0x920a));}
   
  /// Returns true if Flash usage is avaliable.
   bit HasFlash() const {return GetName(0x8827)!=-1;}
   
  /// If HasFlash() is true you can call this to get it - false for no flash, true for flash.
   bit GetFlash() const {return GetNat16(GetName(0x8827))==1;} 


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::file::Exif";}


 private:
  // Represents an entry of the IFD, a single property extracted from the file basically.
   struct Tag
   {
    nat16 tag;
    nat16 type;
    nat32 count;
    byte * data; // Unlike the other this is not corrected for endiness.
   };
  
  // Tells you if the data is using big endian rather than little endian...
   bit motorola;
   
  // Array of IFD's that have been loaded from current file...
   ds::ArrayResize<Tag> tags;
   
  // Helper - zeros the length of ifds and deletes all data.
   void Reset();
};

//------------------------------------------------------------------------------
 };
};
#endif
