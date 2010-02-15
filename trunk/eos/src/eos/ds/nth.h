#ifndef EOS_DS_NTH_H
#define EOS_DS_NTH_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file nth.h
/// The very definition of esoteric. A data structure for finding the minimum 
/// out of several sets of the n-th smallest item within each data set.

#include "eos/types.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
/// Given a several sets numbered 0..m-1 this finds the minimum nth out of all 
/// m sets. The nth of each set is defined as the n-th smallest item in the set 
/// where is n is provided by the user. You provide the numbers to the data 
/// structure sequentially, each addition taking log n, resulting in a runtime
/// of O(t log n) where t is the total number of items. Heavilly optimised 
/// however, so will usually do better than this - can realistically get close 
/// to O(t).
/// (Note that if every set has less than n items in then it will output 
/// infinity.)
/// (Implimented using lots of little heaps.)
/// (Implimented such that with one set only the slightest of costs is taken 
/// compared to a dedicated single set implimentation.)
class EOS_CLASS MultiNth
{
 public:
  /// sets is how many sets, nth is which item we want, with 0 being the first
  /// and so on.
   MultiNth(nat32 sets = 0,nat32 nth = 0);

  /// &nbsp;
   ~MultiNth();


  /// Resets it, removing all data.
  /// (If given same parameters as previous will not re-alloc.)
   void Reset(nat32 sets,nat32 nth);


  /// Adds a number to a given set - call this for all number in all sets before 
  /// getting the answer.
   void Add(nat32 set,real32 num);

  /// Returns the answer, incrimental in nature, and can be called any time. 
  /// Will return +ve infinity until at least one set has had n items added.
   real32 Result() const;


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::ds::MultNth";}


 private:
  nat32 sets; // How many sets to consider.
  nat32 nth; // Which item to extract from each set, stored 1 based, so 1 is item 0 etc.
  
  real32 min; // Answer given data so far.
  nat32 * size; // sets, size of each set.
  real32 * data; // sets*nth, as sets heaps.
};

//------------------------------------------------------------------------------
 };
};
#endif
