#ifndef EOS_DS_SPARSE_HASH_H
#define EOS_DS_SPARSE_HASH_H
//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines

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


/// \file sparse_hash.h
/// A hash table designed for sparse data entrys, where clumping is unlikelly.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/mem/safety.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
// The code for the SparseHash flat template...
class EOS_CLASS SparseHashCode
{
 protected:
   SparseHashCode(nat32 elementSize,nat32 startSize = 15);
   SparseHashCode(nat32 elementSize,const SparseHashCode & rhs);
  ~SparseHashCode(); // Doesn't actually clean up - must call Del to do that.
  
  void Del(nat32 elementSize,void (*Term)(void * ptr)); // The actual deconstructor, Term is called and given a pointer to every valid object, for cleaning up. (If contains a ptr remember thats a ptr to a ptr.)
  void Reset(nat32 elementSize,nat32 startSize = 15); // Sets the structure to be empty, call Del first.

  void Copy(nat32 elementSize,const SparseHashCode & rhs); // You must call Del before calling this, copys by mem::Copy, so fancy stuff can't be stored within except by pointer.

  void Set(nat32 elementSize,nat32 index,void * in);
  void * Get(nat32 elementSize,nat32 index) const; // Returns a pointer to the indexed element, or null if it doesn't exist.
  void * GetCreate(nat32 elementSize,nat32 index); // Creates it if it dosn't exist - never returns null. The memory created will be unset however.
  void Unset(nat32 elementSize,nat32 index,void (*Term)(void * ptr));


  nat32 size; // Size of array - when it runs out its multiplied by 2 and has 1 added.
  nat32 elements; // How many elements exist in the system - when equal to or greater than size the hash is increased.
  nat32 nodes; // How many nodes exist, beyond the once in the size data structure.
  
  nat32 Memory(nat32 elementSize) const // Returns how many bytes of data the indexing of the structure is using.
  {
   return (size+nodes) * (sizeof(Node)+elementSize) + sizeof(SparseHashCode);
  }


 private:
  struct Node
  {
   Node * next;       
   nat32 index;
   bit valid; // true if it contains valid data, false if it doesn't.
  }; // This structure is allways followed directly by the data of the node.
 
  Node * data;
 
  void Inflate(nat32 elementSize); // This doubles+1 the size of the hash table. Slow.
};

//------------------------------------------------------------------------------
/// A hash class designed arround the ushall block of data with hanging off 
/// expansion when it runs out of space. Whilst approximatly O(n) it will 
/// occasionally have to re-hash, and is hence unpredictable on a call by
/// call basis. The T given can be anything, the DT will only work however if it
/// is given a pointer, so leave it as KillNull unless T is a type of pointer for
/// which a Kill can be sensibly done. Indexes using nat32, and can be used as an
/// array.
template <typename T,typename DT = mem::KillNull<T> >
class EOS_CLASS SparseHash : public SparseHashCode
{
 public:
  /// &nbsp;
   SparseHash(nat32 startSize = 15):SparseHashCode(sizeof(T),startSize) {}
   
  /// The copy constructor is templated on the type of DT, so any DT type can be
  /// assigned to any other. Note that assignments should only be a deleting type
  /// and a non-deleting type, as between two deleting types will result in double
  /// deletion. Which is bad.
   template <typename DTT>
   SparseHash(const SparseHash<T,DTT> & rhs):SparseHashCode(sizeof(T),rhs) {}
   
  /// &nbsp;
   ~SparseHash() {Del(sizeof(T),&DelFunc);}
 
        
  /// This works on the same design principals as the copy constructor.
   template <typename DTT>
   SparseHash<T,DT> operator = (const SparseHash<T,DTT> & rhs) {Del(sizeof(T),&DelFunc); Copy(sizeof(T),rhs); return *this;}
   
  
  /// Resets the hash table to contain no data.
   void Reset(nat32 startSize = 15) {Del(sizeof(T),&DelFunc); SparseHashCode::Reset(sizeof(T),startSize);}
   
  /// Returns how many items are in the table.
   nat32 Size() const {return elements;}
   
  /// Returns how much memory the table is consuming.
   nat32 Memory() const {return SparseHashCode::Memory(sizeof(T));}
   
   
  /// This sets a value in the table, copying the given data in.
   void Set(nat32 i,const T & in) {SparseHashCode::Set(sizeof(T),i,(void*)&in);}
   
  /// This returns a pointer to an item in the table, or null if the index given
  /// does not map anywhere.
   T * Get(nat32 i) const {return (T*)SparseHashCode::Get(sizeof(T),i);}
   
  /// This unsets the given index, assuming it exists, and deletes it etc as 
  /// relevent to the template definition, so it no longer appears to exist 
  /// and any associated resources are freed.
   void Unset(nat32 i){SparseHashCode::Unset(sizeof(T),i,&DelFunc);}
  

  /// This tests if an item exists, returning true if it does and false otherwise.
   bit Exists(nat32 i) const {return SparseHashCode::Get(sizeof(T),i)!=null<void*>();}   
  
  /// This provides an array like interface to the structure, in the vent the 
  /// requested index doesn't allready exist it is created, just note that it will
  /// be created with garbage data.
   T & operator [] (nat32 i) {return *((T*)GetCreate(sizeof(T),i));}
   

  /// &nbsp;
   static inline cstrconst TypeString() 
   {
    static GlueStr ret(GlueStr() << "eos::ds::SparseHash<" << typestring<T>() << "," << typestring<DT>() << ">");
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
