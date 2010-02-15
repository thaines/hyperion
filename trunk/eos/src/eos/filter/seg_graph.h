#ifndef EOS_FILTER_SEG_GRAPH_H
#define EOS_FILTER_SEG_GRAPH_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file seg_graph.h
/// Given a segmentation this generates a graph-like structure, so you can query
/// which segments are neighbours and get lists of neighbours for each node.

#include "eos/types.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"
#include "eos/ds/arrays.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// Using a 4 way neighbourhood constructs a fast way of querying for the list
/// of neighbouring segments for any given segment.
class EOS_CLASS SegGraph
{
 public:
  /// Constructs a representation of the given segment structure and segment count
  /// to allow the below query methods to be efficient.
   SegGraph(const svt::Field<nat32> & segs,nat32 segments);
   
  /// &nbsp;
   ~SegGraph();


  /// Returns how many segments are avaliable for querying.
   nat32 Segments();

  /// Returns how many neighbours a given segment has.
   nat32 NeighbourCount(nat32 seg);

  /// Returns neighbour n of the given segment.
   nat32 Neighbour(nat32 seg,nat32 n);
   
  /// Returns how many shared edges under 4 connectivity exist for
  /// the given segment on the border with the segment in position n.
   nat32 BorderSize(nat32 seg,nat32 n);


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::filter::SegGraph";}


 private:	
  struct Node
  {
   nat32 size;
   Pair<nat32,nat32> * data;
  };
  ds::Array<Node> sed;
};

//------------------------------------------------------------------------------
 };
};
#endif
