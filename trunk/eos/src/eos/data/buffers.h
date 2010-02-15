#ifndef EOS_DATA_BUFFERS_H
#define EOS_DATA_BUFFERS_H
//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines

/// \file buffers.h
/// Provides a class with identical functionality to Block, except its designed
/// with resizing in mind and is therefore efficient for such tasks.

#include <stdlib.h>

#include "eos/types.h"
#include "eos/io/seekable.h"
#include "eos/mem/preempt.h"
#include "eos/str/functions.h"

namespace eos
{
 namespace data
 {
//------------------------------------------------------------------------------
/// Provides an equivalent to Block, but with efficient resizing, in that it 
/// internally uses a linked list of memory blocks. Provides all the niceties
/// that Block does. In addition it provides full in/out capability.
/// Anything read is taken from the start of the buffer, anything written is 
/// done so to the end of the buffer, meaning this class can be used as a
/// fifo stream of bytes.
class EOS_CLASS Buffer : public io::Seekable<io::Binary>, public io::InOut<io::Binary>
{
 public:
  // Required due to inheriting two types, otherwise its ambiguous...
    static inline io::Binary EncodeType() {return io::Binary();}
    typedef io::Binary encodeType;
 
  static const nat32 blockSize = 64 - sizeof(class Node *); // If changing this remember to adjust the below new/delete operators for Node correctly.
  
 private:
  class Node
  {
   public:
    void * operator new (size_t) {return eos::mem::pre64.Malloc<Node>();}
    void operator delete (void * ptr) {eos::mem::pre64.Free(static_cast<Node*>(ptr));}
   
    Node * next;
    byte data[blockSize];
  };
  
 public:
  /// Basic constructor.
  /// \param size The size of the buffer, the data will contain random values.
  /// \param pre The value of this is irrelevent to the actual behaviour of the buffer. However, if Pad(x) is to be called latter setting this to x will accelerate that call, as it will reserve the required space.
   Buffer(nat32 size = 0,nat32 pre = 0);

  /// &nbsp;
   Buffer(void * data,nat32 size);
   
  /// &nbsp;
   Buffer(const Buffer & rhs);
     
  /// &nbsp;
   ~Buffer();
  
   
  // Operators...
   /// &nbsp;
    Buffer & operator = (const Buffer & rhs);
   
   /// &nbsp;
    Buffer & operator += (const Buffer & rhs); 
   
   /// &nbsp;
    bit operator == (const Buffer & rhs) const;
   
   /// &nbsp;
    bit operator != (const Buffer & rhs) const {return !operator==(rhs);}
   
   /// &nbsp;
    bit operator < (const Buffer & rhs) const;
    
   /// &nbsp;
    bit operator > (const Buffer & rhs) const;
    
   /// &nbsp;
    bit operator <= (const Buffer & rhs) const {return !operator>(rhs);}
    
   /// &nbsp;
    bit operator >= (const Buffer & rhs) const {return !operator<(rhs);}
    
    
   /// This is identical to SetSize, except it uses space at the start of the buffer
   /// instead of the end. When reducing the buffer size it removes data from the 
   /// start of the buffer. When increasing the buffer size it adds new data to the
   /// start of the buffer. New data is arbatory. This is useful for handling blocks
   /// of data with headers, for adding and removing them as moving through a stack.
   /// Will invalidate all Cursors.
   /// \param sze The new size the buffer should be set to.
   /// \returns New size of the buffer, should be sze on success.
    nat32 SetSizePre(nat32 sze);
 

   /// Sub-class for the cursor type.
    class EOS_CLASS Cursor : public eos::io::InOut<eos::io::Binary>
    {
     public:
       Cursor(Buffer & t,Buffer::Node * n,nat32 p,nat32 o):targ(&t),node(n),pos(p),offset(o) {}
       Cursor(const Cursor & rhs):targ(rhs.targ),node(rhs.node),pos(rhs.pos),offset(rhs.offset) {}
      ~Cursor() {}
      
      // Extra interface elements specific to this Cursor type...
       /// &nbsp;
        Cursor & operator = (const Cursor & rhs) {targ = rhs.targ; node = rhs.node; pos = rhs.pos; offset = rhs.offset; return *this;}
       
       /// A simple pass through, so you can get the size of the block at the other end of the Cursor.
        nat32 Size() const {return targ->Size();}
        
       /// A simple pass through, so you can discover if you can resize the block.
        bit CanResize() const {return targ->CanResize();}
        
       /// A simple pass through, so you can resize the block.
        nat32 SetSize(nat32 sze) {return targ->SetSize(sze);}
        
       /// Sets the Cursor to the start of the data block.
        void ToStart();
        
       /// Sets the Cursor to a given position.
        void To(nat32 p);
        
       /// Offsets the Cursor from the current position. Same as Skip except it also does negative values.
        void Offset(int32 o);
        
       /// Sets the Cursor to the end of the block. 
        void ToEnd();
        
       /// Returns the position of the cursor relative to the start of the block.
        nat32 Pos() const {return pos;}
        
       /// Skips through to leave the cursor on the given byte, or at the end of
       /// the buffer if it doesn't find one.
        void SkipTo(byte b);
      
        
      // As required by In<Binary>...
       /// &nbsp;
        bit EOS() const {return pos>=targ->size;}
        
       /// &nbsp; 
        nat32 Avaliable() const;
        
       /// &nbsp;
        nat32 Read(void * out,nat32 bytes);
        
       /// &nbsp;
        nat32 Peek(void * out,nat32 bytes) const;
        
       /// &nbsp;
        nat32 Skip(nat32 bytes);      
       
         
      // As required by Out<Binary>...
       /// &nbsp;
        nat32 Write(const void * in,nat32 bytes);  
        
       /// &nbsp;
        nat32 Pad(nat32 bytes);


      /// &nbsp;
       static inline cstrconst TypeString() {return "eos::data::Buffer::Cursor";}
       
       
     private:  
      Buffer * targ;
      Buffer::Node * node;
      nat32 pos;
      nat32 offset; // Offset in the node that equates to the stored pos. Note this is allowed to overrun, ushally by 1 but can be more. This indicates that EOS has happened, and that the next write needs to create a new node(s).
    };
    friend class Cursor;
    
             
  // As required by Seekable<Binary>...
   /// &nbsp;
    nat32 Size() const {return size;}
    
   /// This returns true. If you don't need to resize concider using Block instead.
    bit CanResize() const {return true;}
    
   /// Sets the size of the Buffer, will invalidate any Cursors that are henceforth
   /// outside the new range. Note that whilst fast at increasing the Buffer size this
   /// can be slow at reducing it. The class is in general designed to accumulate data,
   // rather than have it pruned.
    nat32 SetSize(nat32 sze);
   
   /// This method is fast to get the start and end of the buffer, but the middle takes time
   /// and should be avoided where possible.
    Cursor GetCursor(nat32 pos = 0);


  // As required by In<Binary>...
   /// &nbsp;
    bit EOS() const {return size==0;}
        
   /// &nbsp; 
    nat32 Avaliable() const {return size;}
        
   /// &nbsp;
    nat32 Read(void * out,nat32 bytes);
        
   /// &nbsp;
    nat32 Peek(void * out,nat32 bytes) const
    {
     if (size==0) return 0;
     else
     {
      return Cursor(const_cast<Buffer&>(*this),first,0,offset).Peek(out,bytes);
     }
    }
        
   /// &nbsp;
    nat32 Skip(nat32 bytes);

       
   // As required by Out<Binary>...
    /// &nbsp;
     nat32 Write(const void * in,nat32 bytes) 
     {
      if (size==0) return Cursor(*this,first,0,0).Write(in,bytes);
              else return Cursor(*this,last,size,LastOffset()).Write(in,bytes);
      }    
   
    /// &nbsp;
     nat32 Pad(nat32 bytes) {return Cursor(*this,last,size,LastOffset()).Pad(bytes);}

    
  /// Returns how much memory the class is using, takes into account unused space etc.
   nat32 Memory() const
   {return sizeof(Buffer) + ((size+offset+blockSize-1)/blockSize)*sizeof(Node);}

    
  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::data::Buffer";}
 
     
  // Undocumented interface, used exclusivly by the io interface functions.
  // Simply allows iteration of the data blocks of a buffer object.
   // This returns a 'handle' to the first data block...
    void * IterFirst() const
    {
     return first;       
    }
   // This is given a handle and outputs both a pointer to some data and the
   // size of the data...
   // It will output a handle of 0 when done.
    void IterNext(void *& handle,byte *& data,nat32 & dataSize) const
    {
     data = static_cast<Node*>(handle)->data;
     dataSize = blockSize;
     if (handle==first)
     {
      data += offset;
      dataSize -= offset;    
     }
     if (handle==last)
     {
      dataSize -= blockSize-LastOffset();       
     }
     handle = static_cast<Node*>(handle)->next;     
    }


 private:
  nat32 size;
  nat32 offset; // Offset in the first node to get to the data.
  Node * first; // Node at the front of the data.
  Node * last; // Node at the end of the data. Exists to accelerate getting a cursor to the end, as this is a common operation.
  
  // Helper function, returns how many bytes have been consumed in last, i.e. the offset to start writting at to append data.
   nat32 LastOffset() const
   {
    nat32 ret = (size+offset)%blockSize;
    if ((ret==0)&&(size!=0)) ret = blockSize;
    return ret;
   }
   
  // An integrity check function - called by asserts at various points, 
  // just to be sure the system works...
  // Returns true if the sun is shinning.
   bit Invariant() const;
};

//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
// IO interface functions for Buffer class...
namespace eos
{
 namespace io
 {
//------------------------------------------------------------------------------
template <typename T>
inline T & StreamWrite(T & lhs,const eos::data::Buffer & rhs,Binary)
{
 void * hand = const_cast<eos::data::Buffer&>(rhs).IterFirst();
 while (hand)
 {
  byte * ptr;
  nat32 ptrSize;
  const_cast<eos::data::Buffer&>(rhs).IterNext(hand,ptr,ptrSize);
  lhs.SetError(lhs.Write(ptr,ptrSize)!=ptrSize);       
 }
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,eos::data::Buffer & rhs,Binary)
{
 void * hand = rhs.IterFirst();
 while (hand)
 {
  byte * ptr;
  nat32 ptrSize;
  rhs.IterNext(hand,ptr,ptrSize);
  lhs.SetError(lhs.Read(ptr,ptrSize)!=ptrSize);       
 } 
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,const eos::data::Buffer & rhs,Text)
{
 byte hex[eos::data::Buffer::blockSize*2];
 cstrconst hl = "0123456789ABCDEF";
 
 void * hand = rhs.IterFirst();
 while (hand)
 {
  byte * ptr;
  nat32 ptrSize;
  rhs.IterNext(hand,ptr,ptrSize);
  for (nat32 i=0;i<ptrSize;i++)
  {
   hex[i*2]   = hl[ptr[i]>>4];
   hex[i*2+1] = hl[ptr[i]&0x0F];         
  }
  lhs.SetError(lhs.Write(hex,ptrSize*2)!=ptrSize*2);       
 }  

 return lhs;       
}

template <typename T>
inline T & StreamRead(T & lhs,eos::data::Buffer & rhs,Text)
{
 // Buffer to use...
  byte hex[eos::data::Buffer::blockSize*2];
  
 // Loop bitch, loop...
  void * hand = rhs.IterFirst();
  while (hand)
  {
   byte * ptr;
   nat32 ptrSize;
   rhs.IterNext(hand,ptr,ptrSize);
   
   nat32 ad = lhs.Read(hex,ptrSize*2);
   for (nat32 i=0;i<ad;i+=2)
   {
    int8 val1 = str::HexToVal(hex[ad]);
    int8 val2 = str::HexToVal(hex[ad+1]);
    if ((val1==-1)||(val2==-1)) {ad=0; break;}

    ptr[ad/2] = (val1<<4) | val2;   
   }
   
   if (ptrSize*2!=ad)
   {
    lhs.SetError(true);
    break;       
   }
  }
	
 return lhs;
}
//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
#endif
