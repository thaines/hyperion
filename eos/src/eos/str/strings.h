#ifndef EOS_STR_STRINGS_H
#define EOS_STR_STRINGS_H
//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines

/// \file strings.h
/// Contains the string class provided by the eos system.

#include "eos/types.h"
#include "eos/io/seekable.h"
#include "eos/data/buffers.h"

namespace eos
{
 namespace str
 {
//------------------------------------------------------------------------------
/// The String class, does exactly what you would expect. Essentially a 
/// derivative of eos::data::Buffer that has been converted to present a Text
/// instead of Binary interface. Provides all the features you would expect from
/// a string class, including a basic level of internationalisation suport.
class EOS_CLASS String : public io::Seekable<io::Text>, public io::InOut<io::Text>
{
 public:
  // Required due to inheriting two types, otherwise its ambiguous...
    static inline io::Text EncodeType() {return io::Text();}
    typedef io::Text encodeType; 
 
  /// &nbsp;
   String() {}
  
  /// &nbsp; 
   String(cstrconst str) {*this << str;}
   
  /// &nbsp;
   String(const String & rhs):buf(rhs.buf) {}
   
  /// &nbsp;
   ~String() {}
  
  
  // A bunch of operators...
   /// &nbsp;
    String & operator = (const String & rhs) {buf = rhs.buf; return *this;}
    
   /// &nbsp;
    String & operator = (cstrconst rhs) {buf.SetSize(0); *this << rhs; return *this;}
    
   /// Equivalent to *this << rhs, only provided because this is slightly faster 
   /// than << and for stylistic reasons.
    String & operator += (const String & rhs) {buf += rhs.buf; return *this;}

    
   /// &nbsp;
    bit operator == (const String & rhs) const {return buf==rhs.buf;}
    
   /// &nbsp;
    bit operator != (const String & rhs) const {return buf!=rhs.buf;}
    
   /// &nbsp;
    bit operator == (cstrconst rhs) const {return buf==String(rhs).buf;}
    
   /// &nbsp;
    bit operator != (cstrconst rhs) const {return buf!=String(rhs).buf;}
   
    
   /// &nbsp; 
    bit operator < (const String & rhs) const {return buf<rhs.buf;}
    
   /// &nbsp;
    bit operator > (const String & rhs) const {return buf>rhs.buf;}
    
   /// &nbsp;
    bit operator <= (const String & rhs) const {return buf<=rhs.buf;}
    
   /// &nbsp;
    bit operator >= (const String & rhs) const {return buf>=rhs.buf;}


  // Strange shit...
   /// Returns true if the string starts with a particular string.
    bit StartsWith(cstrconst s) const;

   /// Returns true if the string ends with a particular string, 
   /// useful for checking file extensions.
    bit EndsWith(cstrconst s) const;
    
   /// This extracts a line of text from a virtual in stream, leaving the
   /// stream at the start of the next line or the end of the file.
   /// This assigns the line to this, deleting all present data.
   /// Handles all styles of line-ends.
   /// Returns true on success, false if source is all dried up.
    bit GetLine(io::InVirt<io::Text> & source);
    
   /// Identical to the io::Text version, except that it takes binary data.
   /// Good for when text data is encapsulated in a binary file.
   /// An extra feature is given in noR - if this is given then '\r'
   /// is not considered to be a line ending character. (i.e. windows compatability off.)
   /// The prime reason to use this is for integrated binary/text
   /// data, where eatting '\r' after a string followed by binary could screw 
   /// alignment.
    bit GetLine(io::InVirt<io::Binary> & source,bit noR = false);


  // Some useful convertors...
   /// You must call mem::Free on the result of this call.
    cstr ToStr() const;

   /// &nbsp;
    int32 ToInt32() const;

   /// &nbsp;
    real32 ToReal32() const;
    
   /// "t", "true" (Both case insensitive) and "1" produce true, everything else is false.
    bit ToBit() const;


  /// The string Cursor, as required by Seekable.
   class EOS_CLASS Cursor : public io::InOut<io::Text>
   {
    public:
      Cursor(const data::Buffer::Cursor & rhs):cur(rhs) {}
      Cursor(const Cursor & rhs):cur(rhs.cur) {}
     ~Cursor() {}
      
     // Extra interface elements specific to this Cursor type...
      /// &nbsp;
       Cursor & operator = (const Cursor & rhs) {cur = rhs.cur; return *this;}
      
      /// A simple pass through, so you can get the size of the block at the other end of the Cursor.
       nat32 Size() const {return cur.Size();}
       
      /// A simple pass through, so you can discover if you can resize the block.
       bit CanResize() const {return cur.CanResize();}
       
      /// A simple pass through, so you can resize the block.
       nat32 SetSize(nat32 sze) {return cur.SetSize(sze);}
       
      /// Sets the Cursor to the start of the data block.
       void ToStart() {cur.ToStart();}
       
      /// Sets the Cursor to a given position.
       void To(nat32 p) {cur.To(p);}
       
      /// Offsets the Cursor from the current position. Same as Skip except it also does negative values.
       void Offset(int32 o) {cur.Offset(o);}
       
      /// Sets the Cursor to the end of the block. 
       void ToEnd() {cur.ToEnd();}
       
      /// Returns the position of the cursor relative to the start of the block.
       nat32 Pos() const {return cur.Pos();}
       
      /// Skips the cursor forward until the given character is found, leaving
      /// it on the given character, or until the end is reached.
       void SkipTo(cstrchar c) {cur.SkipTo(c);}
     
       
     // As required by In<Binary>...
      /// &nbsp;
       bit EOS() const {return cur.EOS();}
       
      /// &nbsp; 
       nat32 Avaliable() const {return cur.Avaliable();}
       
      /// &nbsp;
       nat32 Read(void * out,nat32 bytes) {return cur.Read(out,bytes);}
       
      /// &nbsp;
       nat32 Peek(void * out,nat32 bytes) const {return cur.Peek(out,bytes);}
       
      /// &nbsp;
       nat32 Skip(nat32 bytes) {return cur.Skip(bytes);}
      
        
     // As required by Out<Binary>...
      /// &nbsp;
       nat32 Write(const void * in,nat32 bytes) {return cur.Write(in,bytes);}  
       
      /// &nbsp;
       nat32 Pad(nat32 bytes)
       {
        static const cstrchar t[] = "                                "; // 32 spaces.
        nat32 toDo = bytes;
        while (toDo>0) toDo -= Write((void*)t,math::Min(toDo,nat32(sizeof(t))));
        return bytes;
       }

 
     /// &nbsp;
      static inline cstrconst TypeString() {return "eos::str::String::Cursor";}


     /// Not to be used, for direct access to the internal buffer, used in the io
     /// integration methods.
      data::Buffer::Cursor & BufferCursor() {return cur;}
     
       
    private:
     data::Buffer::Cursor cur;      
   };
  
       
  // From Seekable...
   /// &nbsp;
    nat32 Size() const {return buf.Size();}
    
   /// &nbsp;
    bit CanResize() const {return buf.CanResize();}
    
   /// &nbsp;
    nat32 SetSize(nat32 size) {return buf.SetSize(size);}
    
   /// &nbsp;
    Cursor GetCursor(nat32 pos = 0) {return Cursor(buf.GetCursor(pos));}


  // From In...
   /// &nbsp;
    bit EOS() const {return buf.EOS();}
        
   /// &nbsp; 
    nat32 Avaliable() const {return buf.Avaliable();}
        
   /// &nbsp;
    nat32 Read(void * out,nat32 bytes) {return buf.Read(out,bytes);}
        
   /// &nbsp;
    nat32 Peek(void * out,nat32 bytes) const {return buf.Peek(out,bytes);}
        
   /// &nbsp;
    nat32 Skip(nat32 bytes) {return buf.Skip(bytes);}


  // From Out...
   /// &nbsp;
    nat32 Write(const void * in,nat32 bytes) {return buf.Write(in,bytes);}
    
   /// &nbsp;
    nat32 Pad(nat32 bytes)
    {
     static const cstrchar t[] = "                                "; // 32 spaces.
     nat32 toDo = bytes;
     while (toDo>0) toDo -= Write((void*)t,math::Min(toDo,nat32(sizeof(t))));
     return bytes;
    }
  
  /// &nbsp;
   nat32 Memory() const {return buf.Memory();} 

    
  /// Not to be used, for direct access to the internal buffer, used in the io
  /// integration methods.
   data::Buffer & Buffer() {return buf;}
   
  /// Not to be used, for direct access to the internal buffer, used in the io
  /// integration methods... 
   const data::Buffer & Buffer() const {return buf;}
  
  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::str::String";}
 
 private:
  data::Buffer buf;
};

//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
// IO integration for the string class...
namespace eos
{
 namespace io
 {
//------------------------------------------------------------------------------
template <typename T>
inline T & StreamWrite(T & lhs,const eos::str::String & rhs,Binary)
{
 StreamWrite(lhs,rhs.Buffer(),Binary());       
 return lhs;
}

template <typename T>
inline T & StreamRead(T & lhs,eos::str::String & rhs,Binary)
{
 StreamRead(lhs,rhs.Buffer(),Binary());      
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,const eos::str::String & rhs,Text)
{
 StreamWrite(lhs,rhs.Buffer(),Binary());
 return lhs;       
}

template <typename T>
inline T & StreamRead(T & lhs,eos::str::String & rhs,Text)
{
 StreamRead(lhs,rhs.Buffer(),Binary());	
 return lhs;
}

//------------------------------------------------------------------------------	 
 };
};
//------------------------------------------------------------------------------
#endif
