#ifndef EOS_IO_SEEKABLE_H
#define EOS_IO_SEEKABLE_H
//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines

/// \file seekable.h
/// Provides the Seekable classes, which are simply the InOut class enhanced with
/// seeking capability - all this allows is the returning of a cursor type which 
/// is of type InOut.

#include "eos/io/inout.h"

namespace eos
{
 namespace io
 {
//------------------------------------------------------------------------------
/// A seekable class is simply a class with some extra properties, such as size,
/// and the the ability to return cursors, which are simply derivatives of InOut.
/// In addition to implimenting the features on display here a a public child type
/// named Cursor (either a class or a typedef) must be provided, so templated code
/// can delcare the correct type.
template <typename ET>
class EOS_CLASS Seekable
{
 public:
  /// This is used to tag which techneque should be used to encode data.
   static inline ET EncodeType() {return ET();}
   typedef ET encodeType;
   
  /// This returns the stream size, i.e. the range within you can seek.
   nat32 Size() const;
   
  /// This returns true if you can set a cursor outside the Size() range and it will
  /// resize to a larger 'thing', false if such seeking will result in an error.
   bit CanResize() const;
   
  /// This sets the size of the 'thing' in question. Note that if 
  /// CanResize()==false this will still ushally work, as CanResize() only 
  /// applys to resizing on demand. Just remember that if CanResize() is 
  /// false then this method is likelly to be slow, and you should probably 
  /// be avoiding it anyway.
  /// \returns The new size, should be what you pass in on success.
   nat32 SetSize(nat32 size);
 
  /// Returns a cursor in the requested position.
  /// \param pos 0..Size()-1 as range if CanResize()==false, otherwise arbitary. A cursor can allways point to the Size() position without resizing a class to be Size()+1, this allows you to make a Cursor(Size()) and then not use it, leaving the class at the same size. If you were to of course use it you would then make the data structure behind the interface grow, but never beyond what you have written.
  /// \returns A cursor, which can be both read and written from. It will be of type type_of(this)::Cursor
   InOut<ET> GetCursor(nat32 pos = 0); 
   
  /// A Typestring method, here as a reminder for children to impliment it.
   static inline cstrconst TypeString();	
};

//------------------------------------------------------------------------------
/// The virtual version of Seekable. Identical in features, though there is no
/// requirement for a Cursor public type to be provided on account of this 
/// classes nature. (Of course, you will have a hard time implimenting the class
/// without making such a type, so it is recomended it be expressed as such anyway,
/// for consistancy.)
template <typename ET>
class EOS_CLASS SeekableVirt : public Deletable
{
 public:
  /// This is used to tag which techneque should be used to encode data.
   static inline ET EncodeType() {return ET();}
   typedef ET encodeType;
   
  /// This returns the stream size, i.e. the range within you can seek.
   virtual nat32 Size() const = 0;
   
  /// This returns true if you can set a cursor outside the Size() range and it will
  /// resize to a larger 'thing', false if such seeking will result in an error.
   virtual bit CanResize() const = 0;
   
  /// This sets the size of the 'thing' in question. Note that if 
  /// CanResize()==false this will still ushally work, as CanResize() only 
  /// applys to resizing on demand. Just remember that if CanResize() is 
  /// false then this method is likelly to be slow, and you should probably 
  /// be avoiding it anyway. Note that if making the data structure larger 
  /// the new data will be of random value.
  /// \returns The new size, should be what you pass in on success.
   virtual nat32 SetSize(nat32 size) = 0;
 
  /// Returns a cursor in the requested position. To get a cursor to append to 
  /// current data request a obj->Cursor(obj->Size()).
  /// \param pos 0..Size()-1 as range if CanResize()==false, otherwise arbitary. A cursor can allways point to the Size() position without resizing a class to be Size()+1, this allows you to make a Cursor(Size()) and then not use it, leaving the class at the same size. If you were to of course use it you would then make the data structure behinf the interface grow, but never beyond what you have written.
  /// \returns A cursor, which can be both read and written from. It will be of type type_of(this)::Cursor
   virtual InOutVirt<ET> GetCursor(nat32 pos = 0) = 0; 	
   
  /// A virtual TypeString method, for the typestring system.
   virtual cstrconst TypeString() const = 0;
};

//------------------------------------------------------------------------------
// We can impliment StreamWrite versions for the virtual version, much like we 
// can for InVirt, and in fact in terms of the InVirt methods.

template <typename T>
inline T & StreamWrite(T & lhs,SeekableVirt<Binary> & rhs,Binary)
{
 lhs << rhs.GetCursor();       
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,SeekableVirt<Text> & rhs,Binary)
{
 lhs << rhs.GetCursor();       
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,SeekableVirt<Binary> & rhs,Text)
{
 lhs << rhs.GetCursor();       
 return lhs;
}

template <typename T>
inline T & StreamWrite(T & lhs,SeekableVirt<Text> & rhs,Text)
{
 lhs << rhs.GetCursor();       
 return lhs;
}

//------------------------------------------------------------------------------
 };
};
#endif
