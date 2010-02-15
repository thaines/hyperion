#ifndef EOS_DS_ARRAYS_H
#define EOS_DS_ARRAYS_H
//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines

/// \namespace eos::ds
/// Provides a reasonably comprehensive set of generic data structures, for
/// general purpose use. Flat templates are used for ease of use and speed of
/// compilation, meaning most are implimented as pairs of classes.

/// \file arrays.h
/// Contains the array data structure, for sets of data that rarely
/// change in size, with a constant access time. Provides fast sorting and
/// searching capability.

#include "eos/types.h"
#include "eos/typestring.h"

#include "eos/mem/alloc.h"
#include "eos/mem/functions.h"
#include "eos/mem/safety.h"
#include "eos/ds/sorting.h"

#include "eos/ds/lists.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
// Code for the Array flat template...
class EOS_CLASS ArrayCode
{
 protected:
   ArrayCode(nat32 elementSize,nat32 e)
   :elements(e),data(mem::Malloc<byte>(elementSize*e))
   {}

   ArrayCode(nat32 elementSize,nat32 e,void (*Make)(void * ptr))
   :elements(e),data(mem::Malloc<byte>(elementSize*e))
   {for (nat32 i=0;i<e;i++) Make(&data[elementSize*i]);}

   ArrayCode(nat32 elementSize,const ArrayCode & rhs);
  ~ArrayCode() {mem::Free(data);}

  void Del(nat32 elementSize,void (*Term)(void * ptr)); // Calls Term on all elements.

  void Copy(nat32 elementSize,const ArrayCode & rhs); // Call Del before hand.

  nat32 Resize(nat32 elementSize,nat32 newSize,void (*Make)(void * ptr),void (*Term)(void * ptr));

  nat32 Rebuild(nat32 elementSize,nat32 newSize,void (*Make)(void * ptr)); // Allways deletes previous data and copys none over. Call Del first.

  byte * Item(nat32 elementSize,nat32 i) const {return data + elementSize*i;}

  void SortRange(nat32 elementSize,bit (*LessThan)(void * lhs,void * rhs),int32 left,int32 right,byte * temp);
  void SortRange(nat32 elementSize,bit (*LessThan)(void * lhs,void * rhs,void * ptr),int32 left,int32 right,void * ptr,byte * temp);
  int32 Search(nat32 elementSize,bit (*LessThan)(void * lhs,void * rhs),nat32 left,nat32 right,byte * target) const;
  nat32 SearchLargest(nat32 elementSize,bit (*LessThan)(void * lhs,void * rhs),nat32 left,nat32 right,byte * target) const; // Returns index of largest element smaller than or equal to target.


  friend class KdTreeCode;
  friend class CollectorCode;


 nat32 elements;
 byte * data;
};

//------------------------------------------------------------------------------
/// Yea basic array class, as provided by any set of data structures.
template <typename T, typename MT = mem::MakeDont<T>,typename DT = mem::KillNull<T> >
class EOS_CLASS Array : public ArrayCode
{
 public:
  /// &nbsp;
  /// \param sz Size of the array.
  Array(nat32 sz = 0):ArrayCode(sizeof(T),sz,&MakeFunc) {}

  /// Copy constructor, templated on deletion method so it can be assigned
  /// regardless. Just remember that having an object stored in 2 structures
  /// that are going to delete/free/de-allocate it tends to go horribly wrong.
   template <typename MTT,typename DTT>
   Array(const Array<T,MTT,DTT> & rhs):ArrayCode(sizeof(T),rhs) {}

  /// &nbsp;
  ~Array() {Del(sizeof(T),&DelFunc);}


  /// Works on the same principal as the copy constructor, see there for relevent warnings.
   template <typename MTT,typename DTT>
   Array<T,MT,DT> & operator = (const Array<T,MTT,DTT> & rhs) {Del(sizeof(T),&DelFunc); Copy(sizeof(T),rhs); return *this;}

  /// Assignment from a linked list, conveniant.
   template <typename DTT>
   Array<T,MT,DT> & operator = (const List<T,DTT> & rhs)
   {
    Resize(sizeof(T),rhs.Size(),&MakeFunc,&DelFunc);
    typename List<T,DTT>::Cursor targ = rhs.FrontPtr();
    nat32 i = 0;
    while (!targ.Bad())
    {
     (*this)[i] = *targ;
     ++targ;
     ++i;
    }
    return *this;
   }

  /// Returns the size of the array.
   nat32 Size() const {return elements;}

  /// Changes the size of the array, returning the new size. Note that if you
  /// grow the array new items will contain arbitary data, this is particularly
  /// dangerous if they are pointers or an equally sensitive type - make sure to
  /// then initialise them correcly. (Unless you have set a MakeFunc.)
   nat32 Size(nat32 size) {return Resize(sizeof(T),size,&MakeFunc,&DelFunc);}

  /// Returns how many bytes of data the structure is using, excluding data at
  /// the other ends of pointers stored in the contained objects.
   nat32 Memory() const {return elements*sizeof(T) + sizeof(ArrayCode);}


  /// This provides access to the aray in the traditional C sense, this is
  /// no faster than using the object directly, and so should only be used
  /// for compatability and other silly reasons.
   T * Ptr() {return (T*)data;}

  /// &nbsp;
   T & operator[] (nat32 i) const {return *(T*)Item(sizeof(T),i);}


  /// Sorts the array by a given sorting functor.
   template <typename SO>
   void Sort()
   {
    struct Func
    {
     static bit F(void * lhs,void * rhs)
     {
      return SO::LessThan(*((T*)lhs),*((T*)rhs));
     }
    };
    byte temp[sizeof(T)];
    ArrayCode::SortRange(sizeof(T),Func::F,0,elements-1,temp);
   }

  /// Sort this array using the < operator, i.e. the sort mostly done.
   void SortNorm()
   {
    Sort< SortOp<T> >();
   }

  /// Sorts a given range in an array by a given sorting functor.
  /// The range given is inclusive, i.e. SortRange(0,Size()-1) is identical
  /// to sort.
   template <typename SO>
   void SortRange(nat32 start,nat32 end)
   {
    if (end<=start) return;
    struct Func
    {
     static bit F(void * lhs,void * rhs)
     {
      return SO::LessThan(*((T*)lhs),*((T*)rhs));
     }
    };
    byte temp[sizeof(T)];
    ArrayCode::SortRange(sizeof(T),Func::F,int32(start),int32(end),temp);
   }

  /// Sort a given range in the array using the items < operator.
   void SortRangeNorm(nat32 start,nat32 end)
   {
    SortRange< SortOp<T> >(start,end);
   }

  /// This finds an item under the assumption the array is allready sorted, by the same
  /// sorting functor as then given to this method. Will return the index of an entry
  /// matching the dummy on success or -1 if not found.
   template <typename SO>
   int32 Search(const T & dummy)
   {
    struct Func
    {
     static bit F(void * lhs,void * rhs)
     {
      return SO::LessThan(*((T*)lhs),*((T*)rhs));
     }
    };
    return ArrayCode::Search(sizeof(T),Func::F,0,elements-1,(byte*)&dummy);
   }

  /// This finds an item under the assumption the array is allready sorted, by the
  /// objects less than operator. Will return the index of an entry matching the
  /// dummy on success or -1 if not found.
   int32 SearchNorm(const T & dummy)
   {
    return Search< SortOp<T> >(dummy);
   }

  /// This is essentially a search that never fails, because if it can't find the item it
  /// returns the index of the largest item smaller than it, and if all items are larger
  /// than it then it returns the first item. Useful when its the position returned that
  /// matters, not the data at the other end, for finding where data fits in a set.
   template <typename SO>
   int32 SearchLargest(const T & dummy)
   {
    struct Func
    {
     static bit F(void * lhs,void * rhs)
     {
      return SO::LessThan(*((T*)lhs),*((T*)rhs));
     }
    };
    return ArrayCode::SearchLargest(sizeof(T),Func::F,0,elements-1,(byte*)&dummy);
   }

  /// This is essentially a search that never fails, because if it can't find the item it
  /// returns the index of the largest item smaller than it, and if all items are larger
  /// than it then it returns the first item. Useful when its the position returned that
  /// matters, not the data at the other end, for finding where data fits in a set.
  /// This version uses the objects < operator.
   int32 SearchLargestNorm(const T & dummy)
   {
    return SearchLargest< SortOp<T> >(dummy);
   }


  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::ds::Array<" << typestring<T>() << "," << typestring<MT>() << "," << typestring<DT>() << ">");
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
/// An array class that allows you to specify the size of the array elements in
/// bytes. This is useful when you are using runtime-sized structures.
/// Note that there is no checking for you using two of these with the same type
/// but different sizes - treat such differences as though they are in fact
/// different types.
template <typename T, typename MT = mem::MakeDont<T>,typename DT = mem::KillNull<T> >
class EOS_CLASS ArrayRS : public ArrayCode
{
 public:
  /// If constructed with this method then FinishCons must be called.
   ArrayRS():ArrayCode(0,0,&MakeFunc),es(0) {}
   
  /// For if constructed with the no-parameter constructor.
   void FinishCons(nat32 eSize,nat32 sz = 0)
   {
    es = eSize;
    Resize(es,sz,&MakeFunc,&DelFunc);
   }

  /// &nbsp;
  /// \param eSize Size of each element in the array, independent of the type.
  ///              (Traditionally the given type would be smaller than this.)
  /// \param sz Size of the array.
   ArrayRS(nat32 eSize,nat32 sz = 0):ArrayCode(eSize,sz,&MakeFunc),es(eSize) {}

  /// Copy constructor, templated on deletion method so it can be assigned
  /// regardless. Just remember that having an object stored in 2 structures
  /// that are going to delete/free/de-allocate it tends to go horribly wrong.
   template <typename MTT,typename DTT>
   ArrayRS(const ArrayRS<T,MTT,DTT> & rhs):ArrayCode(rhs.es,rhs),es(rhs.es) {}

  /// &nbsp;
  ~ArrayRS() {Del(es,&DelFunc);}


  /// Works on the same principal as the copy constructor, see there for relevent warnings.
   template <typename MTT,typename DTT>
   ArrayRS<T,MT,DT> & operator = (const ArrayRS<T,MTT,DTT> & rhs) {Del(es,&DelFunc); Copy(es,rhs); return *this;}

  /// Returns the size of the array.
   nat32 Size() const {return elements;}

  /// Changes the size of the array, returning the new size. Note that if you
  /// grow the array new items will contain arbitary data, this is particularly
  /// dangerous if they are pointers or an equally sensitive type - make sure to
  /// then initialise them correcly.
   nat32 Size(nat32 size) {return Resize(es,size,&MakeFunc,&DelFunc);}

  /// This resizes both the array and the size of the elements. Nothing gets
  /// copied over, so all elements will be random after this.
  /// \param eSize Size of each element in the array, independent of the type.
  ///              (Traditionally the given type would be smaller than this.)
  /// \param sz Size of the array.
   nat32 Rebuild(nat32 eSize,nat32 sz)
   {
    Del(es,&DelFunc);
    es = eSize;
    return ArrayCode::Rebuild(es,sz,&MakeFunc);
   }

  /// Returns how many bytes of data the structure is using, excluding data at
  /// the other ends of pointers stored in the contained objects.
   nat32 Memory() const {return elements*es + sizeof(ArrayCode);}


  /// This provides access to the aray in the traditional C sense, this is
  /// no faster than using the object directly, and so should only be used
  /// for compatability and other silly reasons.
   T * Ptr() {return (T*)data;}

  /// &nbsp;
   T & operator[] (nat32 i) const
   {
    log::Assert(i<elements,"Array index out of bounds.");
    return *(T*)Item(es,i);
   }


  /// Sorts the array by a given sorting functor.
   template <typename SO>
   void Sort()
   {
    struct Func
    {
     static bit F(void * lhs,void * rhs)
     {
      return SO::LessThan(*((T*)lhs),*((T*)rhs));
     }
    };
    byte * temp = mem::Malloc<byte>(es);
     SortRange(es,Func::F,0,elements-1,temp);
    mem::Free(temp);
   }

  /// Sorts the array. Unlike the normal Sort(), this version is given a sorting
  /// function which can take extra data. This is often required with this version,
  /// as sorting will be dependent on the runtime decided structure of the data in
  /// question.
   template <typename PT>
   void Sort(bit (*Func)(T * lhs,T * rhs,PT * ptr),PT * ptr)
   {
    byte * temp = mem::Malloc<byte>(es);
    SortRange(es,(bit(*)(void*,void*,void*))Func,0,elements-1,ptr,temp);
    mem::Free(temp);
   }

  /// This finds an item under the assumption the array is allready sorted, by the same
  /// sorting functor as then given to this method. Will return the index of an entry
  /// matching the dummy on success or -1 if not found.
   template <typename SO>
   int32 Search(const T & dummy)
   {
    struct Func
    {
     static bit F(void * lhs,void * rhs)
     {
      return SO::LessThan(*((T*)lhs),*((T*)rhs));
     }
    };
    return ArrayCode::Search(es,Func::F,0,elements-1,(byte*)&dummy);
   }

  /// This is essentially a search that never fails, because if it can't find the item it
  /// returns the index of the largest item smaller than it, and if all items are larger
  /// than it then it returns the first item. Useful when its the position returned that
  /// matters, not the data at the other end, for finding where data fits in a set.
   template <typename SO>
   int32 SearchLargest(const T & dummy)
   {
    struct Func
    {
     static bit F(void * lhs,void * rhs)
     {
      return SO::LessThan(*((T*)lhs),*((T*)rhs));
     }
    };
    return ArrayCode::SearchLargest(es,Func::F,0,elements-1,(byte*)&dummy);
   }


  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::ds::ArrayRS<" << typestring<T>() << "," << typestring<MT>() << "," << typestring<DT>() << ">");
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
/// This is an array class that automatically uses mem::MakeNew and
/// mem::KillOnlyDel on its subject, this makes it safe to use this version with
/// objects that need to be both constructed and destructed. Requires a
/// parameterless constructor off course.
template <typename T>
class EOS_CLASS ArrayDel : public Array<T,mem::MakeNew<T>,mem::KillOnlyDel<T> >
{
 public:
  /// &nbsp;
   ArrayDel(nat32 sz = 0):Array<T,mem::MakeNew<T>,mem::KillOnlyDel<T> >(sz) {}

  /// &nbsp;
   ~ArrayDel() {}
};

//------------------------------------------------------------------------------
/// This is an array class that automatically uses mem::MakeNull and
/// mem::KillDel on its subject, this makes it ideal for pointers, as it will
/// null them on construction and delete them when done.
/// The T parameter should be constructed without the pointer indicator - it
/// will add this itself.
template <typename T>
class EOS_CLASS ArrayPtr : public Array<T*,mem::MakeNull<T*>,mem::KillDel<T> >
{
 public:
  /// &nbsp;
   ArrayPtr(nat32 sz = 0):Array<T*,mem::MakeNull<T*>,mem::KillDel<T> >(sz) {}

  /// &nbsp;
   ~ArrayPtr() {}
};

//------------------------------------------------------------------------------
 };
};
#endif
