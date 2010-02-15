#ifndef EOS_DS_FALLOFF_H
#define EOS_DS_FALLOFF_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file falloff.h
/// Given a set of points in n-dimensional space with costs assigned and a 
/// convex falloff in 'cost' of using that point then this allows you to query
/// the minimum cost at any given point of all provided points.
/// Designed for speed.

#include "eos/types.h"
#include "eos/math/distance.h"
#include "eos/math/vectors.h"
#include "eos/ds/arrays.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
// Code for the flat template Falloff...
class EOS_CLASS FalloffCode
{
 protected:
  // Data structure used...
   struct Node
   {
    Node * left;
    Node * right;
    real32 cost;

    real32 * Pos() {return (real32*)(void*)(this+1);}
   
    bit operator < (const Node & rhs) const {return cost<rhs.cost;}
   };
  
 
 // Blah...
   FalloffCode(nat32 dim);
  ~FalloffCode();

  
  // Sets the size of the storage array, resetting it ready for use.
   void SetSize(nat32 sz);

  // Set the positions and costs of the various points in the space.
   void Set(nat32 ind,real32 cost); // Sets cost.
   void Set(nat32 dim,nat32 ind,const real32 * pos); // Sets position via pointer to array.
   void Set(nat32 ind,nat32 ord,real32 val); // Sets position by one ord at a time.


  // Builds the kd tree from the contained data, pruning nodes that cost too 
  // much and are overwhelmed by neighbours.
   void Build(nat32 dim,const math::Distance & dist);


  // Given a coordinate this finds the cost of that position.
   real32 Cost(nat32 dim,const math::Distance & dist,const real32 * pos) const;
   
  // Helper for above...
   static void CostRec(nat32 dim,const math::Distance & dist,Node * targ,nat32 depth,const real32 * pos,real32 & out);


 // Actual storage, not shrunk to remove pruned nodes...
  ArrayRS<Node> data;
};

//------------------------------------------------------------------------------
/// Given a set of points in n-dimensional space with assigned costs and a
/// distance measure the cost at any location in space can be defined as the 
/// minimum of the cost plus the distance for every single point going to the
/// location. This class efficiently creates a data structure from a point set
/// and allows you to then sample this cost at any point. Internally uses a 
/// kd tree, with a non-shrinking memory block so it can be re-used efficiently.
///
/// Works a two phase system - first you fill in an internal array with all the
/// data, you then call Build and can start finding the minimum cost.
/// At any time you can then resize it, fill it up and Build again.
template <nat32 D,typename C = math::EuclideanDistance>
class EOS_CLASS Falloff : public FalloffCode
{
 public:
  /// &nbsp;
   Falloff():FalloffCode(D) {}

  /// &nbsp;
   ~Falloff() {}
   
   
  /// Returns the size of the array.
   nat32 Size() const {return data.Size();}
  
  /// Sets the size of the array.
   nat32 Size(nat32 sz) {SetSize(sz); return sz;}


  /// Sets the cost associated with an entry.
   void Set(nat32 ind,real32 cost) {FalloffCode::Set(ind,cost);} 
   
  /// Sets the position vector associated with an entry.
   void Set(nat32 ind,const math::Vect<D,real32> & v) {FalloffCode::Set(D,ind,v.Ptr());}
   
  /// Sets a single part of the position vector associated with an entry.
   void Set(nat32 ind,nat32 ord,real32 val) {FalloffCode::Set(ind,ord,val);}


  /// Once you have filled in the entire array call this to build the data
  /// structure to allow querying to begin.
   void Build() {FalloffCode::Build(D,dist);}
   
   
  /// Returns the cost at the given coordinate.
   real32 Cost(const math::Vect<D,real32> & v) const {return FalloffCode::Cost(D,dist,v.Ptr());}


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::ds::Falloff<...>";}


  /// This is made public, so you may call any of its public methods.
  /// This is because most distance measures provide, at the very least, a
  /// multiplier of distance, and possibly other parameters.
  /// Once built changing any parameters will break things.
   C dist;
};

//------------------------------------------------------------------------------
 };
};
#endif
