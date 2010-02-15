#ifndef EOS_INF_MODEL_SEG_H
#define EOS_INF_MODEL_SEG_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file model_seg.h
/// Belief propagation based segmentation algorithm.

#include "eos/types.h"

#include "eos/time/progress.h"
#include "eos/ds/arrays2d.h"
#include "eos/ds/collectors.h"
#include "eos/svt/field.h"
#include "eos/mem/packer.h"

#include "eos/file/csv.h"

namespace eos
{
 namespace inf
 {
//------------------------------------------------------------------------------
/// This is a segmentation algorithm, except it segments by 'models' where each
/// node can have the probability of it fitting a given model calculated.
/// Adjacent nodes have a cost of being assigned to a different model.
/// The nodes are arranged on a 2D grid, with this providing the adjacency
/// information and the opportunity for various optimisations.
///
/// This would be a simple (min sum) belief propagation algorithm except the number of
/// labels will generally be prohibitive, so it uses the cost cap, which makes most
/// labels too expensive for any given node, except where adjacent nodes suport that
/// label. It then dynamically decides which labels are worth storing for each
/// node to minimise memory consumption. The entire algorithm is implimented as
/// a hideous data rewrite system basically.
class EOS_CLASS ModelSeg
{
 public:
  /// &nbsp;
   ModelSeg();

  /// &nbsp;
   ~ModelSeg();


  /// Sets the size of the problem - the size of the grid and the number of
  /// models to choose from for each node.
   void SetSize(nat32 width,nat32 height,nat32 models);
   
  /// Optionally sets a mask, nodes with a false are excluded from the 
  /// calculation.
   void SetMask(const svt::Field<bit> & mask);

  /// Sets the parameters.
  /// \param diffCost Cost of assigning differing models to adjacent pixels.
  ///                 The basis of the optimisation - don't set too high.
  ///                 This is multiplied by a per-adjacency value.
  ///                 Defaults to 1.0.
  /// \param costCap Maximum cost that may be assigned to a given node for
  ///                assigning a given model. Defaults to 1.0.
  /// \param iters Number of iters per level of hierachy. Defaults to 10.
   void SetParas(real32 diffCost,real32 costCap,nat32 iters);
   
  /// Sets the cost of fitting a given model to a given node coordinate.
  /// You do not have to fill in all values, all omited values are assumed
  /// to cost costCap. Any value that is more than costCap will be dropped.
  /// You may not call this for any (x,y,model) tuple more than once.
   void AddCost(nat32 x,nat32 y,nat32 model,real32 cost);
   
  /// Sets the multiplier of the diffCost on a per-adjacency basis, this 
  /// consists of a multiplier for the +ve x and +ve y directions.
  /// The entire array defaults to 1.
  /// Set per-pixel, obviously for edge pixels not all given values will be
  /// used.
  /// Call after SetSize.
   void SetMult(nat32 x,nat32 y,real32 mx,real32 my);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());
   
   
  /// Returns true if a model has been assigned to the given index, false if 
  /// it has not. (Because its masked usually.) 
   bit Masked(nat32 x,nat32 y) const;

  /// Returns the model index choosen for a given index.
   nat32 Model(nat32 x,nat32 y) const;  
  
  /// Returns the confidence of a particular assignment. The difference between
  /// the best place cost and second place cost.
   real32 Confidence(nat32 x,nat32 y) const;


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::inf::ModelSeg";}


 private:
  static const nat32 blockSize = 4*1024*1024; // Block size used by the assortment of memory allocators.
 
  // Input...
   nat32 width;
   nat32 height;
   nat32 models;
   
   real32 costCap;
   real32 diffCost;
   nat32 iters;
   
   struct Dir
   {
    real32 mx,my;
   };
   ds::Array2D<Dir> diffCostMult;
   
   svt::Field<bit> mask;
   
   struct NodeModCost
   {
    nat32 x;
    nat32 y;
    nat32 model;
    real32 cost;
    
    bit operator < (const NodeModCost & rhs) const
    {
     if (y!=rhs.y) return y<rhs.y;
     if (x!=rhs.x) return x<rhs.x;
     return model<rhs.model;
    }
   };
   ds::Collector<NodeModCost> userCosts;


  // Outputs...
   struct ModCost // Used below where cost has its normal meaning, for output its confidence.
   {
    nat32 model;
    real32 cost;
   };
   ds::Array2D<ModCost> out;


  // This structure represents a message, either the input provided by the 
  // user or the message from one node to another. Simply a list of
  // ModCost's...
  // (Which must be sorted.)
   struct Msg
   {
    nat32 size;
    real32 baseCost; // Cost of models not included in the model list.

    ModCost * Data() {return (ModCost*)(void*)(this+1);} 
    const ModCost * Data() const {return (ModCost*)(void*)(this+1);}

    // Writes to the log info on this message:
     void Log(cstrconst openWith) const
     {
      LogDebug(openWith << " {size,baseCost}" << LogDiv() << size << LogDiv() << baseCost);
      for (nat32 i=0;i<size;i++)
      {
       LogDebug("{n,model,cost}" <<
                LogDiv() << i << LogDiv() << Data()[i].model << LogDiv() << Data()[i].cost);
      }
     }
   };
   
  // Part of the index data structure - pointers to the user provided message
  // and 4 incomming messages. Also has flags to indicate if it should send
  // messages, to encode boundary and mask conditions.
  // Directions encoded as 0 = +ve x; 1 = +ve y; 2 = -ve x; 3 = -ve y.
   struct Node
   {
    bit send[4]; // Flag for each diection, to indicate if a message should be sent.
    real32 cap[4]; // Message cost cap for each of the 4 directions, applied to outputs.
    Msg * in[4]; // Incomming message from each direction, can include nulls.
    Msg * user; // Message set by user. Can be null.    
   };


  // Helper functions...
   // Transfers user messages from a lower level to a higher level...
    void TransferUserUp(ds::Array2D<Node> & from,ds::Array2D<Node> & to,mem::Packer & alloc);
    
   // Transfers messages down the hierachy, used to go from one level of the
   // hierachy to the next.
   // The two allocators are used to allocate messages in a checkerboard pattern,
   // to support the memory usage optimisation...
    void TransferMsgDown(ds::Array2D<Node> & from,ds::Array2D<Node> & to,
                         mem::Packer & allocA,mem::Packer & allocB);

   // Does a single round of message passing. iter is the iteration number, and
   // is used to indicate whether to use black or white squares on the giant 
   // chess board via its lowest bit. It will create new incomming messages for
   // all nodes of the other colour, hence the packer.
   // Calls of this will alternate in which packer is used, to allow memory to
   // be freed when overwritten by a latter message pass.
    void PassMessages(ds::Array2D<Node> & index,nat32 iter,mem::Packer & alloc);
};

//------------------------------------------------------------------------------
 };
};
#endif
