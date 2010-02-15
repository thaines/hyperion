#ifndef EOS_ALG_GREEDY_MERGE_H
#define EOS_ALG_GREEDY_MERGE_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file greedy_merge.h
/// Given a graph this merges nodes with a best first approach until no can be
/// merged. The user provides things such as costing methods and merging methods.

#include "eos/types.h"

#include "eos/time/progress.h"
#include "eos/ds/arrays.h"
#include "eos/ds/lists.h"
#include "eos/ds/layered_graphs.h"

namespace eos
{
 namespace alg
 {
//------------------------------------------------------------------------------
/// Interface that must be implimented to use the GreedyMerge class, provides
/// problem specific methods.
class EOS_CLASS GreedyMergeInterface : public Deletable
{
 public:
  /// &nbsp;
   ~GreedyMergeInterface() {}
   
  /// Returns the cost of a single segment. Costs must be independent between
  /// unmerged nodes so they can be summed to get the cost for the entire
  /// graph.
  /// Given the arbitary data associated with the node.
   virtual real32 Cost(const Deletable * a) const = 0;
   
  /// Returns the cost of two segments for if they were merged.
   virtual real32 Cost(const Deletable * a,const Deletable * b) const  = 0;
   
  /// Merges two segments returning a new segment. The new segment will be
  /// automatically deleted by the GreedyMerge class latter.
   virtual Deletable * Merge(const Deletable * a,const Deletable * b) const  = 0;
};

//------------------------------------------------------------------------------
/// You provide this with a graph and associated data. You also provide
/// an interface of methods, these provide the cost of things and the
/// ability to merge them. It then greedilly merges things until no further
/// merge would reduce the summed cost. As output you get the resulting graph
/// and a mapping from the input graph to the output graph.
class EOS_CLASS GreedyMerge
{
 public:
  /// &nbsp;
   GreedyMerge();
   
  /// &nbsp;
   ~GreedyMerge();
   
   
  /// Sets the GreedyMergeInterface that defines costs and does merges.
  /// The interface will be owned by this object and deleted when it gets deleted.
   void Set(GreedyMergeInterface * gmi);

  /// Sets the number of nodes in the graph - they will be numbered from 0..num-1.
   void SetNodeCount(nat32 num);

  /// Sets the arbitary data associated with each node and passed to the gmi for
  /// cost calculation/merging etc.
  /// The data is owned by this object from this point on, and will be delted if
  /// the node is merged.
   void SetNodeData(nat32 ind,Deletable * data);
   
  /// Adds an edge between two nodes, to indicate they are candidates for merging.
   void AddEdge(nat32 a,nat32 b);
   
   
  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());
   
  
  /// Returns the number of output nodes that exist.
   nat32 Nodes() const;
   
  /// Returns the arbitary data associated with each output node.
   const Deletable * Data(nat32 ind) const;

  /// Given an input node index returns its output node index.
   nat32 InToOut(nat32 in) const;
   
  /// Returns the number of edges in the output.
   nat32 Edges() const;
   
  /// Returns the indexed edge.
   void GetEdge(nat32 index,nat32 & outA,nat32 & outB);


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::alg::GreedyMerge";}


 private:
  struct Edge
  {
   nat32 a,b;
  };

  // Input...
   GreedyMergeInterface * gmi;
   ds::ArrayPtr<Deletable> in;
   ds::List<Edge> inEdge;
  
  // Output...
   ds::ArrayPtr<Deletable> out;
   ds::Array<nat32> inToOut;
   ds::Array<Edge> outEdge;
   
  // Runtime...
   struct NodeData
   {
    real32 cost;
    nat32 index; // Used in the output constructing step.
    Deletable * data;
   };
   typedef ds::LayeredGraph<NodeData,Nothing,Nothing,Nothing> LG;
   
   struct WorkItem
   {
    real32 costDec; // Amount cost decreases by application.
    LG::NodeHand a;
    LG::NodeHand b;
    
    bit operator < (const WorkItem & rhs) const {return costDec > rhs.costDec;}
   };
};

//------------------------------------------------------------------------------
 };
};
#endif
