#ifndef EOS_IO_IN_H
#define EOS_IO_IN_H
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


/// \file in.h
/// Contains the standard for objects from which data can be read, as well as
/// standard reverse serialisations for basic types. Covers the >> operator.

#include "eos/io/base.h"

namespace eos
{
 namespace io
 {
//------------------------------------------------------------------------------
/// The In class, used to read stream data into the program.
/// The ET template parameter must either by 'Binary' or 'Text', to indicate the
/// formating requirements for data extracted from the class. Note that the basic 
/// version is not virtual - if you want to impliment such behaviour use the 
/// InVirt class instead. This class has a protected constructor and none of its
/// methods are implimented, this accheives the same effect as making all its 
/// methods pure virtual, without the unwanted overhead.
template <typename ET>
class EOS_CLASS In : public Base
{
 public:
  /// This is used to tag which techneque should be used to encode data.
   static inline ET EncodeType(){return ET();}
   typedef ET encodeType;
 
  /// End of stream, i.e. Avaliable()==0
   bit EOS() const;  
  /// Returns how many bytes are avaliable to be read. In the case of streams 
  /// that can block this will be how many can be read without blocking 
  /// occuring.
   nat32 Avaliable() const;
   
  /// This reads a given number of bytes into out, returning how many bytes
  /// were actually read.
   nat32 Read(void * out,nat32 bytes);
   
  /// This reads ahead of the current location without actually removing
  /// the bytes from the data source. Identical syntax to Read, often used
  /// in conjunction with Skip.
   nat32 Peek(void * out,nat32 bytes) const;
      
  /// This skips reading a given number of bytes, returning how many bytes
  /// were actually skipped. Skip(0) does nothing.
   nat32 Skip(nat32 bytes);


  /// A Typestring method, here as a reminder for children to impliment it.
   static inline cstrconst TypeString();


 protected:  
  In(){} ///< A protected constructor is implimented so you can't construct an instance of this class.
};

//------------------------------------------------------------------------------
/// This class is identical to In, except it is virtual. See In for overview.
template <typename ET>
class EOS_CLASS InVirt : public Deletable, public Base
{
 public:
  /// This is used to tag which techneque should be used to encode data.
   static inline ET EncodeType(){return ET();}   
   typedef ET encodeType;
 
  /// End of stream, i.e. Avaliable()==0
   virtual bit EOS() const = 0;
  /// Returns how many bytes are avaliable to be read. In the case of streams 
  /// that can block this will be how many can be read without blocking 
  /// occuring.
   virtual nat32 Avaliable() const = 0;
   
  /// This reads a given number of bytes into out, returning how many bytes
  /// were actually read.
   virtual nat32 Read(void * out,nat32 bytes) = 0;
   
  /// This reads ahead of the current location without actually removing
  /// the bytes from the data source. Identical syntax to Read, often used
  /// in conjunction with Skip.
   virtual nat32 Peek(void * out,nat32 bytes) const = 0;
   
  /// This skips reading a given number of bytes, returning how many bytes
  /// were actually skipped. Skip(0) does nothing.
   virtual nat32 Skip(nat32 bytes) = 0;

  /// A virtual TypeString method, for the typestring system.
   virtual cstrconst TypeString() const = 0;
};

//------------------------------------------------------------------------------

/// The >> operator. Designed such that you
/// can add to the types suported by it, but due to the Text/Binary distinction
/// you do not supply extra versions of the >> operator, instead you provide
/// versions of the StreamRead templated function. An example implimentation of
/// a StreamRead implimentation is:<br><code>
/// template < typename T ><br>
/// inline T & StreamRead(T & lhs,my_type & rhs,Binary)<br>
/// {<br>
/// &nbsp;lhs.SetError(lhs.Read(&rhs,sizeof(rhs))!=sizeof(rhs));<br>
/// &nbsp;return lhs;<br>
/// }<br></code>
/// You ushally impliment two versions - a 'Binary' and 'Text' version, note it
/// passed as a type as the 3rd parameter.
template <typename P,typename T>
inline P & operator >> (P & lhs,T & rhs)
{
 return StreamRead(lhs,rhs,P::EncodeType());   
}

//------------------------------------------------------------------------------
// All the StreamRead functions for the basic types, binary versions...
template <typename T>
inline T & StreamRead(T & lhs,nat8 & rhs,Binary)
{
 lhs.SetError(lhs.Read(&rhs,sizeof(rhs))!=sizeof(rhs));
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,nat16 & rhs,Binary)
{
 lhs.SetError(lhs.Read(&rhs,sizeof(rhs))!=sizeof(rhs));
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,nat32 & rhs,Binary)
{
 lhs.SetError(lhs.Read(&rhs,sizeof(rhs))!=sizeof(rhs));
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,nat64 & rhs,Binary)
{
 lhs.SetError(lhs.Read(&rhs,sizeof(rhs))!=sizeof(rhs));
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,int8 & rhs,Binary)
{
 lhs.SetError(lhs.Read(&rhs,sizeof(rhs))!=sizeof(rhs));
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,int16 & rhs,Binary)
{
 lhs.SetError(lhs.Read(&rhs,sizeof(rhs))!=sizeof(rhs));
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,int32 & rhs,Binary)
{
 lhs.SetError(lhs.Read(&rhs,sizeof(rhs))!=sizeof(rhs));
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,int64 & rhs,Binary)
{
 lhs.SetError(lhs.Read(&rhs,sizeof(rhs))!=sizeof(rhs));
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,real32 & rhs,Binary)
{
 lhs.SetError(lhs.Read(&rhs,sizeof(rhs))!=sizeof(rhs));
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,real64 & rhs,Binary)
{
 lhs.SetError(lhs.Read(&rhs,sizeof(rhs))!=sizeof(rhs));
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,bit & rhs,Binary)
{
 lhs.SetError(lhs.Read(&rhs,1)!=1);
 return lhs;
}

//------------------------------------------------------------------------------
// Some helper functions used by the below functions...

/// This function skips whitespace - spaces, tabs and all newlines, apply to
/// an In or InVirt stream.
template <typename T>
inline void SkipWS(T & lhs)
{
 while (true)
 {
  byte val; lhs.Peek(&val,1);
  if ((val==' ')||(val=='\n')||(val=='\t')||(val=='\r'))
  {
   lhs.Skip(1);
  }
  else break;
 }
}

/// This function reads in an integer, you can decide if it accepts negatives,
/// and the maximum number of characters it will read in, including proceding 
/// zeros. This allows a string such as 000100089 to encode 0,100,89 in nat8s
/// for example. Not meant to be used directly.
/// \param lhs The In object to read data from.
/// \param doNeg true to accept negative numbers, false if you only like positives.
/// \param charLimit The maximum number of characters to read in. Excludes signs.
/// \returns The number read in. Will have set an error if occured, will return 0 in such scenarios.
template <typename T,typename RET>
inline RET ReadInt(T & lhs,bit doNeg,nat8 charLimit)
{
 RET ret = 0;
 bit error = true;
 
 /// Transilate negatives...
  byte targ;
  if (lhs.Peek(&targ,1)!=1) {lhs.SetError(true); return static_cast<RET>(0);}
  bit neg; 
  if (targ=='+')
  {
   lhs.Skip(1);
   if (lhs.Peek(&targ,1)!=1) {lhs.SetError(true); return static_cast<RET>(0);}
   neg = false;
   error = false;
  }
  else if ((targ=='-')&&(doNeg))
  {
   lhs.Skip(1);
   if (lhs.Peek(&targ,1)!=1) {lhs.SetError(true); return static_cast<RET>(0);}
   neg = true;
   error = false;
  }
  else neg = false;
 
 // Transilate digits...
  while ((targ>='0')&&(targ<='9')&&(charLimit!=0))
  {
   ret *= 10;
   ret += targ - '0';
   error = false;
  
   lhs.Skip(1);
   if (lhs.Peek(&targ,1)!=1) break;
   charLimit--;
  }
 
 lhs.SetError(error);
 if (neg) return -ret;
     else return ret;	
}

// This translates a real number, excluding exponents - used by below to do the full
// wack...
template <typename T,typename RET>
inline RET ReadRealSimple(T & lhs,byte & targ)
{
 RET ret = 0.0;
 bit error = true;
 
 // Transilate negatives...
  bit neg;
  if (targ=='+')
  {
   lhs.Skip(1);
   if (lhs.Peek(&targ,1)!=1) {lhs.SetError(true); return static_cast<RET>(0);}
   neg = false;	 
  }
  else 
  {
   if (targ=='-')
   {
    lhs.Skip(1);
    if (lhs.Peek(&targ,1)!=1) {lhs.SetError(true); return static_cast<RET>(0);}
    neg = true; 
   }
   else neg = false;
  }
 
 // Transilate the first set of digits...
  while ((targ>='0')&&(targ<='9'))
  {
   ret *= static_cast<RET>(10);
   ret += static_cast<RET>(targ - '0');
   error = false;
  
   lhs.Skip(1);
   if (lhs.Peek(&targ,1)!=1) break;
  }
   
 // The decimal digits...
  if (targ=='.')
  {
   lhs.Skip(1);
   if (lhs.Peek(&targ,1)==1)
   {
    // Transilate the post decimal digits...
     RET mult = static_cast<RET>(1);
     while ((targ>='0')&&(targ<='9'))
     {
      mult *= static_cast<RET>(0.1);
      ret *= static_cast<RET>(10);
      ret += static_cast<RET>(targ - '0');
      error = false;
  
      lhs.Skip(1);
      if (lhs.Peek(&targ,1)!=1) break;
     }
     ret *= mult;
    }
  }
  
 lhs.SetError(error);
 if (neg) return -ret;
     else return ret;
}

/// This function reads in an integer from an In stream. Not meant to be used
/// directly. Will set an error if nothing contributes to the number, otherwise
/// it returns with the stream pointing to the first character thats not part of
/// the number.
template <typename T,typename RET>
inline RET ReadReal(T & lhs)
{
 byte targ; 
 if (lhs.Peek(&targ,1)!=1) {lhs.SetError(true); return static_cast<RET>(0);}
 RET ret = ReadRealSimple<T,RET>(lhs,targ);
 
 if ((targ=='e')||(targ=='E'))
 {
  lhs.Skip(1);
  if (lhs.Peek(&targ,1)!=1) {lhs.SetError(true); return static_cast<RET>(0);}
  
  ret *= math::Pow(static_cast<RET>(10),ReadRealSimple<T,RET>(lhs,targ));
 }

 return ret;
}

//------------------------------------------------------------------------------
// All the StreamRead functions for the basic types, text versions...
template <typename T>
inline T & StreamRead(T & lhs,nat8 & rhs,Text)
{
 SkipWS(lhs);
 if (lhs.EOS()) lhs.SetError(true);
 else
 {
  rhs = ReadInt<T,nat8>(lhs,false,3);
 }
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,nat16 & rhs,Text)
{
 SkipWS(lhs);
 if (lhs.EOS()) lhs.SetError(true);
 else
 {
  rhs = ReadInt<T,nat16>(lhs,false,5);
 }
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,nat32 & rhs,Text)
{
 SkipWS(lhs);
 if (lhs.EOS()) lhs.SetError(true);
 else
 {
  rhs = ReadInt<T,nat32>(lhs,false,10);
 }
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,nat64 & rhs,Text)
{
 SkipWS(lhs);
 if (lhs.EOS()) lhs.SetError(true);
 else
 {
  rhs = ReadInt<T,nat64>(lhs,false,20);
 }
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,int8 & rhs,Text)
{
 SkipWS(lhs);
 if (lhs.EOS()) lhs.SetError(true);
 else
 {
  rhs = ReadInt<T,int8>(lhs,true,3);
 }
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,int16 & rhs,Text)
{
 SkipWS(lhs);
 if (lhs.EOS()) lhs.SetError(true);
 else
 {
  rhs = ReadInt<T,int16>(lhs,true,5);
 }
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,int32 & rhs,Text)
{
 SkipWS(lhs);
 if (lhs.EOS()) lhs.SetError(true);
 else
 {
  rhs = ReadInt<T,int32>(lhs,true,10);
 }
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,int64 & rhs,Text)
{
 SkipWS(lhs);
 if (lhs.EOS()) lhs.SetError(true);
 else
 {
  rhs = ReadInt<T,int64>(lhs,true,19);
 }
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,real32 & rhs,Text)
{
 SkipWS(lhs);
 if (lhs.EOS()) lhs.SetError(true);
 else
 {
  rhs = ReadReal<T,real32>(lhs);
 }
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,real64 & rhs,Text)
{
 SkipWS(lhs);
 if (lhs.EOS()) lhs.SetError(true);
 else
 {
  rhs = ReadReal<T,real64>(lhs);
 }
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,bit & rhs,Text)
{
 SkipWS(lhs);
 if (lhs.EOS()) lhs.SetError(true);
 else
 {
  byte targ = lhs.Peek();	 
  if ((targ=='f')||(targ=='F'))
  {
   lhs.Skip(1);
   rhs = false;
   
   targ = lhs.Peek();
   if ((targ=='a')||(targ=='A'))
   {
    lhs.Skip(1);
    targ = lhs.Peek();
    if ((targ=='l')||(targ=='L'))
    {
     lhs.Skip(1);
     targ = lhs.Peek();
     if ((targ=='s')||(targ=='S'))
     {
      lhs.Skip(1);
      targ = lhs.Peek();
      if ((targ=='e')||(targ=='e'))
      {
       lhs.Skip(1);    
      } else lhs.SetError(true);     
     } else lhs.SetError(true);     
    } else lhs.SetError(true);
   }
  }
  else if ((targ=='t')||(targ=='T'))
  {
   lhs.Skip(1);
   rhs = true;
   
   targ = lhs.Peek();
   if ((targ=='r')||(targ=='R'))
   {
    lhs.Skip(1);
    targ = lhs.Peek();
    if ((targ=='u')||(targ=='U'))
    {
     lhs.Skip(1);
     targ = lhs.Peek();
     if ((targ=='e')||(targ=='e'))
     {
      lhs.Skip(1);
     } else lhs.SetError(true);     
    } else lhs.SetError(true);
   }   	  
  }
  else if ((targ=='n')||(targ=='N'))
  {
   lhs.Skip(1);
   rhs = false;
   targ = lhs.Peek();
   if ((targ=='o')||(targ=='O'))
   {
    lhs.Skip(1);
   }     
  }
  else if ((targ=='y')||(targ=='Y'))
  {
   lhs.Skip(1);
   rhs = true;
   
   targ = lhs.Peek();
   if ((targ=='e')||(targ=='E'))
   {
    lhs.Skip(1);
    targ = lhs.Peek();
    if ((targ=='s')||(targ=='S'))
    {
     lhs.Skip(1);     
    } else lhs.SetError(true);
   }   	  
  }
  else if (targ=='0')
  {
   lhs.Skip(1);
   rhs = false;	  
  }
  else if (targ=='1')
  {
   lhs.Skip(1);
   rhs = true;	  
  }
  else lhs.SetError(true);	 
 }
 return lhs;
}

//------------------------------------------------------------------------------
 };
};
#endif
