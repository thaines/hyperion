#ifndef EOS_DS_ARRAYS2D_H
#define EOS_DS_ARRAYS2D_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file arrays2d.h
/// Contains the 2D array data structure, for sets of data that rarely 
/// change in size, with a constant access time.

#include "eos/types.h"
#include "eos/typestring.h"

#include "eos/io/inout.h"

#include "eos/mem/alloc.h"
#include "eos/mem/functions.h"
#include "eos/mem/safety.h"
#include "eos/ds/sorting.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
// Code for the Array flat template...
class EOS_CLASS Array2DCode
{
 protected:
   Array2DCode(nat32 elementSize,nat32 w,nat32 h,void (*Make)(void * ptr))
   :width(w),height(h),data(mem::Malloc<byte>(elementSize*w*h)) 
   {for (nat32 i=0;i<w*h;i++) Make(&data[elementSize*i]);}
   
   Array2DCode(nat32 elementSize,const Array2DCode & rhs);
  ~Array2DCode() {mem::Free(data);}
  
  void Del(nat32 elementSize, void (*Term)(void * ptr)); // Calls Term on all elements.
  
  void Copy(nat32 elementSize, const Array2DCode & rhs); // Call Del before hand.
  
  void Resize(nat32 elementSize, nat32 newW, nat32 newH, void (*Make)(void * ptr), void (*Term)(void * ptr));

  byte * Item(nat32 elementSize,nat32 x,nat32 y) const {return data + elementSize*x + width*elementSize*y;}


 nat32 width,height;
 byte * data;	
};

//------------------------------------------------------------------------------
/// A 2D array class, has its uses. Very good for World Domination I find.
template <typename T, typename MT = mem::MakeDont<T>, typename DT = mem::KillNull<T> >
class EOS_CLASS Array2D : public Array2DCode
{
 public:
  /// \param w Width of 2D array.
  /// \param h Height of 2D array.
  Array2D(nat32 w = 0,nat32 h = 0):Array2DCode(sizeof(T),w,h,MakeFunc) {}
  
  /// Copy constructor, templated on deletion method so it can be assigned 
  /// regardless. Just remember that having an object stored in 2 structures 
  /// that are going to delete/free/de-allocate it tends to go horribly wrong.
   template <typename DTT>
   Array2D(const Array2D<T,DTT> & rhs):Array2DCode(sizeof(T),rhs) {}
  
  /// &nbsp;
  ~Array2D() {Del(sizeof(T),&DelFunc);}
  
  
  /// Works on the same principal as the copy constructor, see there for relevent warnings.
   template <typename DTT>
   Array2D<T,DT> & operator = (const Array2D<T,DTT> & rhs) {Del(sizeof(T),&DelFunc); Copy(sizeof(T),rhs); return *this;}
  
  
  /// Returns the width of the array.
   nat32 Width() const {return width;}
  
  /// Returns the height of the array.
   nat32 Height() const {return height;}
   
  /// Changes the size of the array. Note that if you
  /// grow the array new items will contain arbitary data, this is particularly
  /// dangerous if they are pointers or an equally sensitive type - make sure to
  /// then initialise them correcly.
  /// \param nW New width.
  /// \param nH New height.  
   void Resize(nat32 nW, nat32 nH) {Array2DCode::Resize(sizeof(T),nW,nH,MakeFunc,DelFunc);}
  
  /// Returns how many bytes of data the structure is using, excluding data at 
  /// the other ends of pointers stored in the contained objects.
   nat32 Memory() const {return width*height*sizeof(T) + sizeof(Array2DCode);}
  
     
  /// &nbsp;
   T & Get(nat32 x, nat32 y) const {return *(T*)Item(sizeof(T),x,y);}
  
  /// Clamps out of range values into the grid, i.e. extends boundary values.
   T & ClampGet(int32 x, int32 y) const
   {
    int32 ax = math::Clamp(x,int32(0),int32(width-1));
    int32 ay = math::Clamp(y,int32(0),int32(height-1));
    return *(T*)Item(sizeof(T),ax,ay);
   }

  
  /// &nbsp;
   static inline cstrconst TypeString() 
   {
    static GlueStr ret(GlueStr() << "eos::ds::Array2D<" << typestring<T>() << "," << typestring<MT>() << "," << typestring<DT>() << ">");
    return ret;
   }  


 private:
  static void MakeFunc(void * ptr)
  {
   MT::Make((T*)ptr);
  }
   
  static void DelFunc(void * ptr)
  {
   DT::Kill((T*)ptr);
  }	
};

//------------------------------------------------------------------------------
/// A 2D array class that allows you to specify the size of the stored type in 
/// bytes, for runtime sized data structures.
template <typename T, typename MT = mem::MakeDont<T>, typename DT = mem::KillNull<T> >
class EOS_CLASS Array2DRS : public Array2DCode
{
 public:
  ///  If constructed with this method then FinishCons must be called.
   Array2DRS():Array2DCode(0,0,0,MakeFunc),es(0) {}
  
  /// For if constructed with the no-parameter constructor.
   void FinishCons(nat32 eSize,nat32 w = 0,nat32 h = 0)
   {
    es = eSize;
    Array2DCode::Resize(es,w,h,MakeFunc,DelFunc);
   }
 
  /// \param eSize Size of each element, in bytes. Usually larger than sizeof(T).
  /// \param w Width of 2D array.
  /// \param h Height of 2D array.
   Array2DRS(nat32 eSize,nat32 w = 0,nat32 h = 0):Array2DCode(eSize,w,h,MakeFunc),es(eSize) {}
  
  /// Copy constructor, templated on deletion method so it can be assigned 
  /// regardless. Just remember that having an object stored in 2 structures 
  /// that are going to delete/free/de-allocate it tends to go horribly wrong.
   template <typename DTT>
   Array2DRS(const Array2DRS<T,DTT> & rhs):Array2DCode(rhs.es,rhs),es(rhs.es) {}
  
  /// &nbsp;
  ~Array2DRS() {Del(es,&DelFunc);}
  
  
  /// Works on the same principal as the copy constructor, see there for relevent warnings.
   template <typename DTT>
   Array2DRS<T,DT> & operator = (const Array2DRS<T,DTT> & rhs) {Del(es,&DelFunc); Copy(es,rhs); return *this;}
  
  
  /// Returns the width of the array.
   nat32 Width() const {return width;}
  
  /// Returns the height of the array.
   nat32 Height() const {return height;}
   
  /// Changes the size of the array. Note that if you
  /// grow the array new items will contain arbitary data, this is particularly
  /// dangerous if they are pointers or an equally sensitive type - make sure to
  /// then initialise them correcly.
  /// \param nW New width.
  /// \param nH New height.  
   void Resize(nat32 nW, nat32 nH) {Array2DCode::Resize(es,nW,nH,MakeFunc,DelFunc);}
  
  /// Returns how many bytes of data the structure is using, excluding data at 
  /// the other ends of pointers stored in the contained objects.
   nat32 Memory() const {return width*height*es + sizeof(Array2DCode);}
  
     
  /// &nbsp;
   T & Get(nat32 x, nat32 y) const {return *(T*)Item(es,x,y);}
  
  /// Clamps out of range values into the grid, i.e. extends boundary values.
   T & ClampGet(int32 x, int32 y) const
   {
    int32 ax = math::Clamp(x,int32(0),int32(width-1));
    int32 ay = math::Clamp(y,int32(0),int32(height-1));
    return *(T*)Item(es,ax,ay);
   }

  
  /// &nbsp;
   static inline cstrconst TypeString() 
   {
    static GlueStr ret(GlueStr() << "eos::ds::Array2DRS<" << typestring<T>() << "," << typestring<MT>() << "," << typestring<DT>() << ">");
    return ret;
   }  


 private:
  static void MakeFunc(void * ptr)
  {
   MT::Make((T*)ptr);
  }
   
  static void DelFunc(void * ptr)
  {
   DT::Kill((T*)ptr);
  }
  
  nat32 es; // Size of each element in the array.	
};

//------------------------------------------------------------------------------
/// This is a 2d array class that automatically uses mem::MakeNew and 
/// mem::KillOnlyDel on its subject, this makes it safe to use this version with
/// objects that need to be both constructed and destructed. Requires a 
/// parameterless constructor off course.
template <typename T>
class EOS_CLASS ArrayDel2D : public Array2D<T,mem::MakeNew<T>,mem::KillOnlyDel<T> >
{
 public:
  /// &nbsp;
  ArrayDel2D(nat32 w = 0,nat32 h = 0):Array2D<T,mem::MakeNew<T>,mem::KillOnlyDel<T> >(w,h) {}
  
  /// &nbsp;
  ~ArrayDel2D() {}
};

//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
// I/O functions...
namespace eos
{
 namespace io
 {

  template <typename T,typename TT>
  inline T & StreamWrite(T & lhs,const eos::ds::Array2D<TT> & rhs,Text)
  {
   lhs << "[" << rhs.Width() << "," << rhs.Height() << "]={";
   for (nat32 y=0;true;y++)
   {
    lhs << "{";
    for (nat32 x=0;true;x++)
    {
     lhs << rhs.Get(x,y);
     if ((x+1)==rhs.Width()) break;
     lhs << ",";
    }
    lhs << "}";
    if ((y+1)==rhs.Height()) break;
    lhs << ",";
   }
   lhs << "}";
   return lhs;
  }

 };
};
//------------------------------------------------------------------------------
#endif
