#ifndef EOS_DS_SPARSE_BIT_ARRAY_H
#define EOS_DS_SPARSE_BIT_ARRAY_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file sparse_bit_array.h
/// Provide a sparse array of bits, where they default to false but can be set
/// to true and unset. Designed to be extremely fast.

#include "eos/types.h"
#include "eos/mem/preempt.h"
#include "eos/mem/functions.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
/// Represents an array of bits, with false being the default state.
/// Bits can be set and unset, you can also reset all bits to false again. Uses
/// a sparse data structure, with efficient memory allocation.
/// Designed with the expectation that bits will clump, as blocks of bits are 
/// used.
class EOS_CLASS SparseBitArray
{
 public:
  /// All bits start false.
   SparseBitArray();

  /// &nbsp;
   ~SparseBitArray();


  /// Resets all bits to be false. Run time is very roughly O(n) where n 
  /// is the number of bits set true. It has an inner loop so tight though
  /// that you would need extremelly tight values for this to become time
  /// consuming.
   void Reset();

  /// Sets a bit true. Approximatly O(1).
   void Set(nat32 index);

  /// Unsets a bit. Approximatly O(1).
   void Unset(nat32 index);

  /// Returns the state of the bit at a particular index. O(1).
   bit operator[](nat32 index) const;


  /// Returns how many bits have been set to true. O(1).
   nat32 Size() const;

  /// Given an index updates it to the lowest index greater than or equal to the
  /// given index that is set to true. Can be used to construct a list of the 
  /// indexes of set bits. Returns true when index has been updated to an index
  /// that is true, false if no such index exists, i.e. end of loop.
  /// Approximatly O(1).
   bit NextInc(nat32 & index) const;


  /// &nbsp;
   cstrconst TypeString() const {return "eos::ds::SparseBitArray";}


 private:
  // Structure of bits, with helper methods...
   struct Bit256
   {
    Bit256() {mem::Null(n,8);}
   
    void * operator new(size_t size) {log::Assert(size==32); return mem::pre32.Malloc<Bit256>();}
    void operator delete(void * ptr) {mem::pre32.Free(ptr);}
   
    void SetFalse() {for (nat32 i=0;i<8;i++) n[i] = 0;}

    bit NextInc(nat32 & index) const;
    void Reset() {mem::Null(n,8);}

    nat32 n[8]; // 32 bytes. We count the low (&0x01) bits as being first in the sequence, for each nat32.
   };

  // Structure used for the below...
   template <typename T>
   struct Split64
   {
     Split64() {for (nat32 i=0;i<64;i++) ptr[i] = null<T*>();}
    ~Split64() {for (nat32 i=0;i<64;i++) delete ptr[i];}
    
    void Reset() {for (nat32 i=0;i<64;i++) {if (ptr[i]) ptr[i]->Reset();}}
   
    T * ptr[64]; // 256 bytes on a 32 bit computer.
   };
  
  // Data storage structure...
   nat32 size;
   Split64< Split64< Split64< Split64<Bit256> > > > data;
};

//------------------------------------------------------------------------------
 };
};
#endif
