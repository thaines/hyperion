#ifndef EOS_INF_BIN_BP_2D_H
#define EOS_INF_BIN_BP_2D_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

/// \file bin_bp_2d.h
/// Provides a simple 2d grid belief propagation implimentation where there are
/// only 2 options per node - a binary decision. (This would be better 
/// implimented with graph cuts, but I know bp..)

#include "eos/types.h"

#include "eos/ds/arrays2d.h"
#include "eos/time/progress.h"

namespace eos
{
 namespace inf
 {
//------------------------------------------------------------------------------
/// Does belief propagation on a 2D grid where there are only 2 options per node.
/// Some nodes can be set as fixed. You provide all the relevant costs (-ln(prob))
/// and it solves. Whilst given a maximum number of iterations it detects 
/// convergance and generally doesn't do them all.
/// Being a binary decision on each node its very fast anyway.
/// Uses max-sum bp.
class EOS_CLASS BinBP2D
{
 public:
  /// &nbsp;
   BinBP2D();

  /// &nbsp;
   ~BinBP2D();


  /// Sets the size of the problem. When called results all costs to 0 and no
  /// nodes to being disabled.
   void SetSize(nat32 width,nat32 height);

  /// &nbsp;
   nat32 Width() const {return data.Width();}

  /// &nbsp;
   nat32 Height() const {return data.Height();}


  /// Disables a node, so no messages are passed from it and it isn't simulated.
   void Disable(nat32 x,nat32 y);
   
  /// Sets the cost for a node being assigned true or false.
  /// Should be called for both possibilities.
   void SetCost(nat32 x,nat32 y,bit state,real32 cost);
   
  /// Sets the cost of two adjacent nodes, being at the given coordinate (x,y)
  /// and at (x+1,y), refered to as other.
  /// Should be called for all 4 possibilities.
   void SetCostX(nat32 x,nat32 y,bit givenState,bit otherState,real32 cost);

  /// Sets the cost of two adjacent nodes, being at the given coordinate (x,y)
  /// and at (x,y+1), refered to as other.
  /// Should be called for all 4 possibilities.
   void SetCostY(nat32 x,nat32 y,bit givenState,bit otherState,real32 cost);

  /// Sets the momentum - 0.0 means that none of the previous message is blended
  /// in, 1.0 means that none of the new message is used. Should be less than 1
  /// and defaults to 0.
   void SetMomentum(real32 mom);

  /// Sets the maximum number of iterations and tolerance to decide if its converged or not.
   void SetEnd(real32 tol=1e-3,nat32 maxIters = 10000);

  /// Runs the algorithm so that results may be extracted.
  /// Can be called repeatedly, changing costs between calls.
  /// Will start from previous state on future iterations, which can be an advantage.
  /// Disabling nodes that were not disbled previously simply locks there output
  /// message, it doesn't fully disable them after the first run through.
   void Run(time::Progress * prog = null<time::Progress*>());
   
   
  /// Extracts the result for a single node. Disabled nodes will return false.
   bit State(nat32 x,nat32 y) const;
   
  /// Extracts the result for all nodes. Will resize the array if needed.
  /// All disabled node are set to false.
   void Result(ds::Array2D<bit> & out) const;


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::math::BinBP2D";}


 private:
  real32 tol;
  nat32 maxIters;
  
  real32 momentum;
 
  struct Node
  {
   real32 cost[2]; // Cost associated with the two possible states.
   real32 incX[2][2]; // Cost information with the node (x+1,y). [this state][other state].
   real32 incY[2][2]; // Cost information with the node (x,y+1). [this state][other state].
   
   real32 in[4][2]; // Incomming messages. [from +ve x, +ve y, -ve x,-ve y][this state].

   bit dual; // If true its a binary choice node, if false it has no choice and will be ignored.   
   bit result; // False for state 0, true for state 1.
  }; // 74 bytes.

  ds::Array2D<Node> data;
};

//------------------------------------------------------------------------------
 };
};
#endif
