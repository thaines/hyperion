#ifndef EOS_DATA_BLOCKS_H
#define EOS_DATA_BLOCKS_H
//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines

/// \namespace eos::data
/// Provides general structures and algoirthms for managing arbitary data. This 
/// includes structure for storage and algorithms for checksums, compression and
/// encryption.

/// \file blocks.h
/// This contains the Block class, which is simply a memory block with size 
/// information thats fully integrated into the rest of the system.

#include "eos/types.h"
#include "eos/io/seekable.h"
#include "eos/mem/alloc.h"
#include "eos/mem/functions.h"
#include "eos/str/functions.h"
#include "eos/math/functions.h"

namespace eos
{
 namespace data
 {
//------------------------------------------------------------------------------
/// This is simply an arbitary block of memory with an associated size, which 
/// provides obvious safty. It also provides IO capability, for filling a block,
/// and all the ushall eos capabilities.
/// It can be streamed using << and >>, however it just does the data. For 
/// variable sized blocks you need to store the size independently. When 
/// reading it reads Size() bytes, so set it to the size you want it to read.
/// If dealing with text it will read/write hex.
class EOS_CLASS Block : public eos::io::Seekable<eos::io::Binary>
{
 public:
  /// &nbsp;
   Block(nat32 sze = 0):size(0),data(null<byte*>()) {data = mem::Malloc<byte>(size);}
   
  /// &nbsp;
   Block(const Block & rhs):size(rhs.size),data(null<byte*>())
   {data = mem::Malloc<byte>(size); mem::Copy(data,rhs.data,size);}
   
  /// &nbsp;
   ~Block() {mem::Free(data);}
  
   
  // Random stuff unique to this class...
   /// &nbsp;
    Block & operator = (const Block & rhs)
    {
     size = rhs.size;
     
     mem::Free(data);     
     data = mem::Malloc<byte>(size);
     mem::Copy(data,rhs.data,size);
     
     return *this;
    }
  
   /// &nbsp;
    byte & operator[](nat32 index) {return data[index];}
    
   /// &nbsp;
    const byte & operator[](nat32 index) const {return data[index];}
    
   /// Returns a pointer to the data block, shouldn't really be used 
   /// but can be useful...
    byte * Ptr() {return data;}
    
   /// &nbsp;
    const byte * Ptr() const {return data;}

          
  // From Seekable...
   /// &nbsp;
    nat32 Size() const {return size;}
    
   /// This returns false. If intending to resize on demand use Buffer.
    bit CanResize() const {return false;}
    
   /// Works, but slow. Avoid.
    nat32 SetSize(nat32 sze) 
    {
     if (sze==0)
     {
      mem::Free(data); data = null<byte*>();
      size = 0;
      return 0;	     
     }
     byte * nd = mem::Malloc<byte>(sze);
     if (nd==null<byte*>()) return size;
     mem::Copy(nd,data,math::Min(size,sze));
     if (sze>size) mem::Null(nd+size,sze-size);
     mem::Free(data);
     data = nd;
     size = sze;
     return size;
    }
    
    
   /// Sub-class for the cursor type.
    class Cursor : public eos::io::InOut<eos::io::Binary>
    {
     public:
       Cursor(Block & t,nat32 p):targ(t),pos(p) {}
      ~Cursor() {}
      
      // Extra interface elements specific to this Cursor type...
       /// A simple pass through, so you can get the size of the block at the other end of the Cursor.
        nat32 Size() const {return targ.Size();}
        
       /// A simple pass through, so you can discover if you can resize the block.
        bit CanResize() const {return targ.CanResize();}
        
       /// A simple pass through, so you can resize the block.
        nat32 SetSize(nat32 sze) {return targ.SetSize(sze);}
        
       /// Sets the Cursor to the start of the data block.
        void ToStart() {pos = 0;}
        
       /// Sets the Cursor to a given position.
        void To(nat32 p) {pos = p;}
        
       /// Offsets the Cursor from the current position. Same as Skip except it also does negative values.
        void Offset(int32 o) {pos += o;}
        
       /// Sets the Cursor to the end of the block. 
        void ToEnd() {pos = targ.Size();}
        
       /// Returns the position of the cursor relative to the start of the block.
        nat32 Pos() const {return pos;}
      
        
      // As required by In<Binary>...
       /// &nbsp;
        bit EOS() const {return targ.Size()==pos;} 
        
       /// &nbsp; 
        nat32 Avaliable() const 
        {
         if (pos>=targ.Size()) return 0; 
                          else return targ.Size()-pos;
        }
        
       /// &nbsp;
        nat32 Read(void * out,nat32 bytes)
        {
         if (pos>=targ.Size()) return 0;
         nat32 ret = math::Min(targ.Size()-pos,bytes);
          mem::Copy((byte*)out,targ.Ptr()+pos,ret);
          pos += ret;
         return ret;	    
        }
        
       /// &nbsp;
        nat32 Peek(void * out,nat32 bytes) const 
        {
	 if (pos>=targ.Size()) return 0;
         nat32 ret = math::Min(targ.Size()-pos,bytes);
          mem::Copy((byte*)out,targ.Ptr()+pos,ret);
         return ret;
	}
        
       /// &nbsp;
        nat32 Skip(nat32 bytes)
        {
         if (pos>=targ.Size()) return 0;
         nat32 ret = math::Min(targ.Size()-pos,bytes);
          pos += ret;
         return ret;	    
        }      
        
        
      // As required by Out<Binary>...
       /// &nbsp;
        nat32 Write(void * in,nat32 bytes)
        {
         if (pos>=targ.Size()) return 0;
         nat32 ret = math::Min(targ.Size()-pos,bytes);
          mem::Copy(targ.Ptr()+pos,(byte*)in,ret);
          pos += ret;
         return ret;                
        }   
        
       /// &nbsp;
        nat32 Pad(nat32 bytes)
        {
         if (pos>=targ.Size()) return 0;
         nat32 ret = math::Min(targ.Size()-pos,bytes);
          mem::Null(targ.Ptr()+pos,ret);
          pos += ret;
         return ret;                
        }
       
        
      /// &nbsp;
       static inline cstrconst TypeString() {return "eos::data::Block::Cursor";}     
     
       
     private:
      Block & targ;
      nat32 pos;
    };
    
   /// &nbsp;
    Cursor GetCursor(nat32 pos = 0)
    {
     return Cursor(*this,pos);      
    }
   
    
  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::data::Block";}
   
      
 private:
  nat32 size;
  byte * data;
};

//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
// IO interface functions for Block class.
namespace eos
{
 namespace io
 {
//------------------------------------------------------------------------------
template <typename T>
inline T & StreamWrite(T & lhs,const eos::data::Block & rhs,Binary)
{
 lhs.SetError(lhs.Write(rhs.Ptr(),rhs.Size())!=rhs.Size());
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,eos::data::Block & rhs,Binary)
{
 lhs.SetError(lhs.Read(rhs.Ptr(),rhs.Size())!=rhs.Size());
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,const eos::data::Block & rhs,Text)
{
 byte hex[EOS_IO_BLOCK_SIZE*2];
 cstrconst hl = "0123456789ABCDEF";
 
 const byte * pos = rhs.Ptr();
 nat32 toDo = rhs.Size();
 while (toDo!=0)
 {
  nat32 dn = math::Min(toDo,nat32(EOS_IO_BLOCK_SIZE));
  for (nat32 i=0;i<dn;i++)
  {
   hex[i*2]   = hl[pos[i]>>4];
   hex[i*2+1] = hl[pos[i]&0x0F];         
  }
  
  bit res = lhs.Write(hex,dn*2)!=dn*2;
  if (res) {lhs.SetError(true); break;}
  
  pos += dn;
  toDo -= dn;
 }

 return lhs;       
}

template <typename T>
inline T & StreamRead(T & lhs,eos::data::Block & rhs,Text)
{
 // Buffer to use...
  byte hex[EOS_IO_BLOCK_SIZE*2];
  
 // Loop bitch, loop...
  nat32 done = 0;
  while (done!=rhs.Size())
  {
   nat32 amount = done*2;
   if (amount>EOS_IO_BLOCK_SIZE*2) amount = EOS_IO_BLOCK_SIZE*2;
   
   nat32 ad = lhs.Read(hex,amount);
    for (nat32 i=0;i<ad;i+=2)
    {
     int8 val1 = str::HexToVal(hex[ad]);
     int8 val2 = str::HexToVal(hex[ad+1]);
     if ((val1==-1)||(val2==-1)) {ad=0; break;}

     rhs.Ptr()[done] = (val1<<4) | val2;
     ++done;   
    }
   if (ad!=amount) {lhs.SetError(true); break;}
  }
	
 return lhs;
}

//------------------------------------------------------------------------------	 
 };
};
//------------------------------------------------------------------------------
#endif
