#ifndef EOS_IO_OUT_H
#define EOS_IO_OUT_H
//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines

/// \file out.h
/// Contains the standard for objects to which data can be written, as well as
/// standard serialisations for basic types. Covers the << operator.

#include "eos/io/base.h"
#include "eos/io/in.h"
#include "eos/str/functions.h"

#include <stdlib.h>

// The block size for writting from an in stream to an out stream...
// Used outside the io library in a couple of places.
#define EOS_IO_BLOCK_SIZE 512*1024

namespace eos
{
 namespace io
 {
//------------------------------------------------------------------------------
/// The Out class, used to write stream data out of the program.
/// The ET template parameter must either by 'Binary' or 'Text', to indicate the
/// formating requirements for data fed into the class. Note that the basic 
/// version is not virtual - if you want to impliment such behaviour use the 
/// OutVirt class instead. This class has a protected constructor and none of its
/// methods are implimented, this accheives the same effect as making all its 
/// methods pure virtual, without the unwanted overhead.
template <typename ET>
class EOS_CLASS Out : public Base
{
 public:
  /// This is used to tag which techneque should be used to encode data.
   static inline ET EncodeType() {return ET();}
   typedef ET encodeType;
     
  /// This writes a given number of bytes from in, returning how many bytes
  /// were actually written.
   nat32 Write(const void * in,nat32 bytes);    
 
  /// This writes a given number of nulls/spaces to the stream, for creating empty 
  /// space. Returns how many have been written. (nulls for binary, spaces for text.)
   nat32 Pad(nat32 bytes);

  /// A Typestring method, here as a reminder for children to impliment it.
   static inline cstrconst TypeString();
   
 protected:	
  Out(){} ///< A protected constructor is implimented so you can't construct an instance of this class.	
};

//------------------------------------------------------------------------------
/// This class is identical to Out, except virtual. See Out for overview.
template <typename ET>
class EOS_CLASS OutVirt : public Deletable, public Base
{
 public:
  /// This is used to tag which techneque should be used to encode data.
   static inline ET EncodeType() {return ET();}
   typedef ET encodeType;
   
  /// This writes a given number of bytes from in, returning how many bytes
  /// were actually written.
   virtual nat32 Write(const void * in,nat32 bytes) = 0;
 
  /// This writtes a given number of nulls to the stream, for creating empty 
  /// space. Returns how many nulls have been written.
   virtual nat32 Pad(nat32 bytes) = 0;
   
  /// A virtual TypeString method, for the typestring system.
   virtual cstrconst TypeString() const = 0;
};

//------------------------------------------------------------------------------

/// The << operator. Designed such that you
/// can add to the types suported by it, but due to the Text/Binary distinction
/// you do not supply extra versions of the << operator, instead you provide
/// versions of the StreamWrite templated function. An example implimentation of
/// a StreamWrite implimentation is:<br><code>
/// template < typename T ><br>
/// inline T & StreamWrite(T & lhs,my_type & rhs,Binary)<br>
/// {<br>
/// &nbsp;lhs.SetError(lhs.Write(&rhs,sizeof(rhs))!=sizeof(rhs));<br>
/// &nbsp;return lhs;<br>
/// }<br></code>
/// You ushally impliment two versions - a 'Binary' and 'Text' version, note it
/// passed as a type as the 3rd parameter.
template <typename P,typename T>
inline P & operator << (P & lhs,const T & rhs)
{		
 return StreamWrite(lhs,rhs,P::EncodeType());   
}

//------------------------------------------------------------------------------
// All the StreamWrite functions for the basic types, binary versions...
template <typename T>
inline T & StreamWrite(T & lhs,nat8 rhs,Binary)
{
 lhs.SetError(lhs.Write(&rhs,sizeof(rhs))!=sizeof(rhs));
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,nat16 rhs,Binary)
{
 lhs.SetError(lhs.Write(&rhs,sizeof(rhs))!=sizeof(rhs));
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,nat32 rhs,Binary)
{
 lhs.SetError(lhs.Read(&rhs,sizeof(rhs))!=sizeof(rhs));
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,const nat64 & rhs,Binary)
{
 lhs.SetError(lhs.Read(&rhs,sizeof(rhs))!=sizeof(rhs));
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,int8 rhs,Binary)
{
 lhs.SetError(lhs.Write(&rhs,sizeof(rhs))!=sizeof(rhs));
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,int16 rhs,Binary)
{
 lhs.SetError(lhs.Write(&rhs,sizeof(rhs))!=sizeof(rhs));
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,int32 rhs,Binary)
{
 lhs.SetError(lhs.Write(&rhs,sizeof(rhs))!=sizeof(rhs));
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,const int64 & rhs,Binary)
{
 lhs.SetError(lhs.Write(&rhs,sizeof(rhs))!=sizeof(rhs));
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,real32 rhs,Binary)
{
 lhs.SetError(lhs.Write(&rhs,sizeof(rhs))!=sizeof(rhs));
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,const real64 & rhs,Binary)
{
 lhs.SetError(lhs.Write(&rhs,sizeof(rhs))!=sizeof(rhs));
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,bit rhs,Binary)
{
 lhs.SetError(lhs.Write(&rhs,1)!=1);
 return lhs;
}

//------------------------------------------------------------------------------
// Helper functions for the below code...

/// Internally used function, supplied incase it becomes useful, slightly more
/// efficient in design than standard solutions on account of the strange 
/// interface. Note that the buffer length is not checked - make sure that
/// it can't overflow. This does *not* create a null - its designed for 
/// scenarios where its not needed.
/// \param val The value to become a string.
/// \param bufTop The *top* of a buffer. i.e. buffer+length of buffer
/// \param size String length is outputted here.
/// \returns A pointer to where the string starts in the buffer - won't neccesarily be at the start.
template <typename T>
inline cstr IntToCstr(T val,cstr bufTop,nat32 & size)
{
 cstr start = bufTop;
 bool neg;
 if (val<0)
 {
  val = -val; 
  neg = true;
 } else neg = false;

 do
 {
  --bufTop;
  *bufTop = '0' + val%10;   
  val = val/10;
 }
 while (val!=0);
  
 if (neg)
 {
  --bufTop;
  *bufTop = '-';
 }
  
 size = start-bufTop;
 return bufTop;	
}

/// See IntToCstr, variant for natural numbers.
template <typename T>
inline cstr NatToCstr(T val,cstr bufTop,nat32 & size)
{
 cstr start = bufTop;

 do
 {
  --bufTop;
  *bufTop = '0' + val%10;   
  val = val/10;
 }
 while (val!=0);
  
 size = start-bufTop;
 return bufTop;	
}

//------------------------------------------------------------------------------
// All the StreamWrite functions for the basic types, text versions...
// Includes additional suport for character strings.
template <typename T>
inline T & StreamWrite(T & lhs,nat8 rhs,Text)
{
 nat32 size;
 cstrchar buf[3];
 cstr str = NatToCstr(rhs,buf+3,size);
 lhs.SetError(lhs.Write(str,size)!=size);
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,nat16 rhs,Text)
{
 nat32 size;
 cstrchar buf[5];
 cstr str = NatToCstr(rhs,buf+5,size);
 lhs.SetError(lhs.Write(str,size)!=size);
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,nat32 rhs,Text)
{
 nat32 size;
 cstrchar buf[10];
 cstr str = NatToCstr(rhs,buf+10,size);
 lhs.SetError(lhs.Write(str,size)!=size);
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,const nat64 & rhs,Text)
{
 nat32 size;
 cstrchar buf[20];
 cstr str = NatToCstr(rhs,buf+20,size);
 lhs.SetError(lhs.Write(str,size)!=size);
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,int8 rhs,Text)
{
 nat32 size;
 cstrchar buf[4];
 cstr str = IntToCstr(rhs,buf+4,size);
 lhs.SetError(lhs.Write(str,size)!=size);
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,int16 rhs,Text)
{
 nat32 size;
 cstrchar buf[6];
 cstr str = IntToCstr(rhs,buf+6,size);
 lhs.SetError(lhs.Write(str,size)!=size);
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,int32 rhs,Text)
{
 nat32 size;
 cstrchar buf[11];
 cstr str = IntToCstr(rhs,buf+11,size);
 lhs.SetError(lhs.Write(str,size)!=size);
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,const int64 & rhs,Text)
{
 nat32 size;
 cstrchar buf[20];
 cstr str = IntToCstr(rhs,buf+20,size);
 lhs.SetError(lhs.Write(str,size)!=size);
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,real32 rhs,Text)
{
 cstrchar str[32];
 gcvt(rhs,8,str);
 nat32 size = 0; while (str[size]!=0) size++;
 lhs.SetError(lhs.Write(str,size)!=size);
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,const real64 & rhs,Text)
{
 cstrchar str[32];
 gcvt(rhs,24,str);
 nat32 size = 0; while (str[size]!=0) size++;
 lhs.SetError(lhs.Write(str,size)!=size);	
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,bit rhs,Text)
{
 if (rhs) lhs.SetError(lhs.Write((void*)"true",4)!=4);
     else lhs.SetError(lhs.Write((void*)"false",5)!=5);
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,cstrconst rhs,Text)
{	
 nat32 size = str::Length(rhs); 
 lhs.SetError(lhs.Write((void*)rhs,size)!=size);
 return lhs;
}

//------------------------------------------------------------------------------
// This allows the virtual in can be written to an out stream at this level
// due to its virtual behaviour. I written as text it is converted to hex...
template <typename T>
inline T & StreamWrite(T & lhs,const InVirt<Binary> & rhs,Binary)
{
 byte data[EOS_IO_BLOCK_SIZE];
 
 while (true)
 {
  nat32 amount = rhs.Avaliable();
  if (amount==0) break;
  if (amount>EOS_IO_BLOCK_SIZE) amount = EOS_IO_BLOCK_SIZE;
  
  amount = rhs.Read(data,amount);
  lhs.SetError(lhs.Write(data,amount)!=amount);	 
 }

 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,const InVirt<Text> & rhs,Binary)
{
 byte data[EOS_IO_BLOCK_SIZE];
 
 while (true)
 {
  nat32 amount = rhs.Avaliable();
  if (amount==0) break;
  if (amount>EOS_IO_BLOCK_SIZE) amount = EOS_IO_BLOCK_SIZE;
  
  amount = rhs.Read(data,amount);
  lhs.SetError(lhs.Write(data,amount)!=amount);	 
 }

 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,const InVirt<Binary> & rhs,Text)
{
 byte data[EOS_IO_BLOCK_SIZE];
 byte hex[EOS_IO_BLOCK_SIZE*2];
 cstrconst hl = "0123456789ABCDEF";
 
 while (true)
 {
  nat32 amount = rhs.Avaliable();
  if (amount==0) break;
  if (amount>EOS_IO_BLOCK_SIZE) amount = EOS_IO_BLOCK_SIZE;
  
  amount = rhs.Read(data,amount);
  
  for (nat32 i=0;i<amount;i++)
  {
   hex[i*2]   = hl[data[i]>>4];
   hex[i*2+1] = hl[data[i]&0x0F];
  }
  
  bit res = lhs.Write(hex,amount*2)!=amount*2;
  if (res) {lhs.SetError(true); break;}
 }

 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,const InVirt<Text> & rhs,Text)
{
 byte data[EOS_IO_BLOCK_SIZE];
 
 while (true)
 {
  nat32 amount = rhs.Avaliable();
  if (amount==0) break;
  if (amount>EOS_IO_BLOCK_SIZE) amount = EOS_IO_BLOCK_SIZE;
  
  amount = rhs.Read(data,amount);
  bit res = lhs.Write(data,amount)!=amount;
  if (res) {lhs.SetError(true); break;}	 
 }

 return lhs;
}

//------------------------------------------------------------------------------
/// Tempory, an object that allows streaming to a FILE *, a tempory way of making
/// console (vis stdout) and file logging easy.
class StreamFile : public io::Out<io::Text>
{
 public:
  StreamFile(FILE * file):f(file) {}
  
  nat32 Write(void * in,nat32 bytes)
  {
   return fwrite(in,bytes,1,f);
  }    

  nat32 Pad(nat32 bytes)
  {
   for (nat32 i=0;i<bytes;i++) fputc(' ',f);
   return bytes;
  }

  static inline cstrconst TypeString() {return "eos::io::StreamFile";}


 private:
  FILE * f;
};

//------------------------------------------------------------------------------
 };
//------------------------------------------------------------------------------
};
#endif
