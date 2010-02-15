#ifndef EOS_DATA_CHECKSUMS_H
#define EOS_DATA_CHECKSUMS_H
//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines

/// \file checksums.h
/// Provides a set of checksum calculation classes which can be applied to all 
/// In classes and derivatives. Checksums are taken to cover one way hashes.

#include "eos/types.h"
#include "eos/io/seekable.h"
#include "eos/str/functions.h"

namespace eos
{
 namespace data
 {
//------------------------------------------------------------------------------
/// This class represents a crc16. It is a stream class to which you can write,
/// you generate the crc by creating the class then writting all the data before
/// extracting the crc. Both a method interface and io integration are provided
/// for extracting. Uses the CRC-CCITT 16 standard, though it produces the wrong 
/// value if you feed it no data. (Don't ask.)
class EOS_CLASS Crc16 : public io::Out<io::Binary>
{
 public:
  /// &nbsp;
   Crc16():crc(0xFFFF) {}
   
  /// &nbsp;
   Crc16(const Crc16 & rhs):crc(rhs.crc) {}
   
  /// &nbsp; 
   ~Crc16() {}

  /// Resets the structure as though no data has been fed into it.
   void Reset() {crc = 0xFFFF;}
    
   
  /// &nbsp;
   Crc16 & operator = (const Crc16 & rhs) {crc = rhs.crc; return *this;}
  
  /// &nbsp;
   bit operator == (const Crc16 & rhs) const {return crc==rhs.crc;}
   
  /// &nbsp;
   bit operator != (const Crc16 & rhs) const {return crc!=rhs.crc;}
  
   
  /// Returns the crc for all the data entered so far.
   nat16 Get() const {return crc;}
   
  /// For setting the crc, from another source.
   void Set(nat16 c) {crc = c;}
  
  // For the Out interface...
   /// &nbsp;
    nat32 Write(void * in,nat32 bytes);    
    
   /// &nbsp;
    nat32 Pad(nat32 bytes);

    
  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::data::Crc16";}
  
 private:
  nat16 crc;   
};

//------------------------------------------------------------------------------
/// This class represents a md5. It is a stream class to which you can write,
/// you generate the md5 by creating the class then writting all the data before
/// extracting the md5. Both a method interface and io integration are provided
/// for extracting. Directly implimented from rfc 1321.
class EOS_CLASS Md5 : public io::Out<io::Binary>
{
 public:
  /// &nbsp;
   Md5();
   
  /// &nbsp;
   Md5(const Md5 & rhs);
   
  /// &nbsp; 
   ~Md5() {}
   
  /// Resets the structure as though no data has been fed into it.
   void Reset();
  
   
  /// &nbsp;
   Md5 & operator = (const Md5 & rhs);
  
  /// &nbsp;
   bit operator == (const Md5 & rhs) const;
   
  /// &nbsp;
   bit operator != (const Md5 & rhs) const;
  
   
  /// Returns the md5 for all the data entered so far. Note that there is some 
  /// proccessing involved to calculate the final value to return, so this isn't
  /// a simple getter.
   void Get(nat32 out[4]) const;
  
  // For the Out interface...
   /// &nbsp;
    nat32 Write(void * in,nat32 bytes);    
    
   /// &nbsp;
    nat32 Pad(nat32 bytes);

    
  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::data::Md5";}
  
 private:
  nat32 md5[4]; // The md5 of all data blocks so far, without the end bit.
  nat32 amount; // How much data has been absorbed into the object as a whole, amount%64 to get size of below.
  byte data[64]; // The data collected recently, when full its added into the md5 collected so far. 
  
  static void AddBlock(nat32 block[16],nat32 md5[4]); // block is the chunk of data to be added, md5 is in and out.
};

//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
// IO interface functions for the above classes...
namespace eos
{
 namespace io
 {
//------------------------------------------------------------------------------
// Crc16...
template <typename T>
inline T & StreamWrite(T & lhs,const eos::data::Crc16 & rhs,Binary)
{
 lhs << rhs.Get();
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,eos::data::Crc16 & rhs,Binary)
{
 nat16 temp;
 lhs >> temp;
 rhs.Set(temp);
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,const eos::data::Crc16 & rhs,Text)
{
 cstrchar t[5];
 str::ToHex(rhs.Get(),t);
 lhs << t;
 return lhs;       
}

template <typename T>
inline T & StreamRead(T & lhs,eos::data::Crc16 & rhs,Text)
{
 cstrchar d[4];
 bit err = lhs.Read(d,4)!=4;
 if (err) {lhs.SetError(err); return lhs;}
 
 rhs.Set(str::FromHex<nat16>(d,err));
 
 lhs.SetError(err);	
 return lhs;
}

//------------------------------------------------------------------------------
// Md5 (reading dosn't make sense due to the incrimental nature of md5 calculation)...
template <typename T>
inline T & StreamWrite(T & lhs,const eos::data::Md5 & rhs,Binary)
{
 nat32 md5[4];
 rhs.Get(md5);
       
 lhs << md5[0] << md5[1] << md5[2] << md5[3];
        
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,const eos::data::Md5 & rhs,Text)
{
 nat32 md5[4];
 rhs.Get(md5);

 for (nat32 i=0;i<4;i++)
 {
  cstrchar t[9];
  str::ToHex(md5[i],t);
  lhs << t;
 }
 
 return lhs;       
}

//------------------------------------------------------------------------------	 
 };
};
//------------------------------------------------------------------------------
#endif
