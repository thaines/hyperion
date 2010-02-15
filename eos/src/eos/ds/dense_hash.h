#ifndef EOS_DS_DENSE_HASH_H
#define EOS_DS_DENSE_HASH_H
//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines

/// \file dense_hash.h
/// Contains a dense hash table, for when hashed items are likelly to clump 
/// together, easier to think about as a dynamic array that can have large gaps 
/// between groups of entrys.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/mem/safety.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
// The code for the flat template below...
class EOS_CLASS DenseHashCode
{
 protected:
   DenseHashCode();
   DenseHashCode(const DenseHashCode & rhs);
  ~DenseHashCode(); // Does not clean up - 'caller' must call Del.

  void Copy(const DenseHashCode & rhs); // Call Del before calling this - copys in the data.

  // This is essentially the deconstructor - will call the given function on every
  // single object stored within...
   void Del(void (*Term)(void * ptr));
   
  nat32 Size() const; // Slow!
  
  nat32 Memory() const; // Returns how much memory the structure is using, slow.

  bit Exists(nat32 index) const; // Returns true if the given index is not null.
  void *& Get(nat32 index); // Gets a reference to the stored pointer for a particular entry.

  void Optimise(); // Goes through and clean's up any nodes that contain no valuable information.
  void Clean(nat32 index); // A focused version of above.

 private:
  void ***** data[256]; // Yeah, it does look silly, but it is right;-)
};

//------------------------------------------------------------------------------
/// The dense hash is far less flexable than other data structures, but rather
/// useful. It indexes using 32 bit integers (This correspondes to the Token type.)
/// and uses a digital tree to enforce non-amorphic constant time access. Its 
/// best viewed as a dynamic array that never resizes and handles sparse values 
/// without going nuts. It only indexes pointers to objects, to enforce a constant
/// sized item. Note that it is internally a digital tree that 
/// can use a lot of memory quickly, a maximum of 1024 bytes per entry if the 
/// clumping simply isn't happening. (Its 256 way at the top, then 64 way for 
/// the next 4 levels. Assuming 4 byte pointers, so twice that on a 64 bit 
/// architecture.) Should be reserved for large clumps of data where speed of 
/// access is the critical element, not suitable for small scale, should be used
/// when at least thousands of entrys are expected.
template <typename T,typename DT = mem::KillNull<T> >
class EOS_CLASS DenseHash : public DenseHashCode
{
 public:
  /// &nbsp;
   DenseHash() {}
  
  /// The copy constructor is templated on the type of DT, so any DT type can be
  /// assigned to any other. Note that assignments should only be a deleting type
  /// and a non-deleting type,as between two deleting types will result in double
  /// deletion. Which is bad.
   template <typename DTT>
   DenseHash(const DenseHash<T,DTT> & rhs):DenseHashCode(rhs) {}
   
  /// &nbsp;
   ~DenseHash() {Del(&DelFunc);}
   
   
  /// This works on the same design principals as the copy constructor.
   template <typename DTT>
   DenseHash<T,DT> operator = (const DenseHash<T,DTT> & rhs) {Del(&DelFunc); Copy(rhs); return *this;}
 
   
  /// The interface - simply acts like an array, it creates memory to store pointers
  /// when you index them, once accessed the memory for a pointer remains in use 
  /// unless you use methods to clean it up. You are returned a reference to the pointer,
  /// so you can do whatever you like, including setting it to null.
   T *& operator [] (nat32 i) {return (T*&)Get(i);}
   
  /// Returns true if an index is not null, false otherwise. Prefered over using standard []
  /// because it will not create memory to store the index in the event the index does not exist.
   bit Exists(nat32 i) const {return DenseHashCode::Exists(i);}
   
  /// Returns how many non-null pointers are in the data structure. Note that it has to
  /// iterate the entire structure, and is hence slow.
   nat32 Size() const {return DenseHashCode::Size();}
  
  /// Returns the total memory consumed by the data structure. Slow to calculate, for 
  /// profiling and to make sure you know how much is being wasted in indexing.
  /// (Only includes the indexing data structure, not the data it points to.)
   nat32 Memory() const {return DenseHashCode::Memory();}
   
  /// This prunes all unnecesary memory consumed by the data structure. Slow. It is
  /// however faster than calling Clean every time you set an index to null, assuming 
  /// you do this a lot. For simple programs you can probably forget about this, but 
  /// for servers it is wise to call this every now and again, and probably give the 
  /// administrator the option to call it if they want. If you use the Clean method
  /// whenever setting an index to null this is unnecesary.
   void Optimise() {DenseHashCode::Optimise();}
  
  /// See Optimise() for relevent coments to this. This removes any memory used by an
  /// index, to be called after an index is set to null to minimise memory consumption.
  /// Optimise is faster if lots of deletion happens, but this is constant time. For a 
  /// system where few nullifications happen this is probably the correct approach, in a
  /// system with lots of churn Optimise will generally be prefered. Again, this is only
  /// neccesary for programs with long run times.
   void Clean(nat32 i) {DenseHashCode::Clean(i);}
  
   
  /// &nbsp;
   static inline cstrconst TypeString() 
   {
    static GlueStr ret(GlueStr() << "eos::ds::DenseHash<" << typestring<T>() << "," << typestring<DT>() << ">");
    return ret;
   }
 
   
 protected:
  static void DelFunc(void * ptr)
  {
   DT::Kill((T*)ptr); 
  }
};

//------------------------------------------------------------------------------
 };
};
#endif
