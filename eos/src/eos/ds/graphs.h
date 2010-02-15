#ifndef EOS_DS_GRAPHS_H
#define EOS_DS_GRAPHS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file ds/graphs.h
/// Provides a graph data structure, where you can store data at both vertices 
/// and edges. A little different from other data structures provided, in that
/// this uses inheritance rather than templates to extend its functionality.
/// This is neccesary due to the nature of algorithms that run on graphs. It also
/// uses otehr data structures defined in this structure.

#include "eos/types.h"
#include "eos/typestring.h"

#include "eos/mem/alloc.h"
#include "eos/ds/lists.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
// Classes provided below...
class Vertex;
class Edge;
class Graph;

//------------------------------------------------------------------------------
/// The vertex object, for representing the vertices in a Graph. To add 
/// functionality you inherit from it. Basic by design.
class EOS_CLASS Vertex : public Deletable
{
 public:
  /// &nbsp;
   Vertex(Graph & graph);

  /// On deletion the vertex will remove itself from its connected graph, 
  /// deleting all edges to which it is connected.
   ~Vertex();


  /// &nbsp;
   Graph & GetGraph() const {return graph;}


  /// Returns how many edges it has.
   nat32 Edges() const {return edges.Size();}

  /// Returns a Cursor to a list of edge pointers, so you can iterate all edges. 
  /// Note that you must not edit any of the list items (add/delete) otherwise 
  /// things will go tits up. The Cursor will be at the start of the edge list.
   List<Edge*>::Cursor EdgeList() {return edges.FrontPtr();}

  /// Returns the relevent Edge if this Vertex is connected to the given Vertex,
  /// null if they are not connected. Relativly slow as it has to iterate the
  /// Edge list.
   Edge * Connection(Vertex * vert) const;

  /// Essentially a merge, where the victim data structure is deleted and this data
  /// structure is kept, with the additon of extra edges from the victim.
  /// This is given a victim node (Can not be this node), which is then deleted.
  /// All edges for that vertex are changed to point to this vertex instead,
  /// except where an edge allready exists in which case the one allready in this
  /// vertex is kept and the one for the victim dies.
   void Merge(Vertex * victim);


  /// &nbsp;
   virtual cstrconst TypeString() const {return "eos::ds::Vertex";}


 private:
  Graph & graph;
  List<Vertex*>::Cursor graphCursor; // So it can delete itself from the graph quickly.

  // List of pointers to edges. Note that each edge keeps Cursor-s to the 
  // relevent pointers to it, so it can delete them efficiently.
   List<Edge*> edges; 
};

//------------------------------------------------------------------------------
/// The edge object, for representing the edges in a Graph. To add
/// functionality you inherit from it. Can be interpreted as either a directed 
/// or undirected edge as you feel like. Basic by design.
class EOS_CLASS Edge : public Deletable
{
 public:
  /// You should not connect two vertices that are allready connected, 
  /// whilst it will work it will result in you having to disconnect
  /// vertices twice before they are actually disconnected.
   Edge(Graph & graph,Vertex * from,Vertex * to);

  /// This will clean up the connections correctly, removing it from the graph.
   ~Edge();


  /// &nbsp;
   Graph & GetGraph() const {return graph;}

  /// &nbsp;
   Vertex * From() const {return from;}

  /// &nbsp;
   Vertex * To() const {return to;}

  /// Updates the data structure correctly.
   void SetFrom(Vertex * from);

  /// Updates the data structure correctly.
   void SetTo(Vertex * to);


  /// &nbsp;
   virtual cstrconst TypeString() const {return "eos::ds::Edge";}


 private:
  Graph & graph;
  List<Edge*>::Cursor graphCursor; // So it can delete itself from the graph quickly.

  Vertex * from; // Vetex it is comming from.
  List<Edge*>::Cursor fromCursor; // So it can delete the offending entry quickly, it likes a quick death.

  Vertex * to; // Vertex it is pointing at.
  List<Edge*>::Cursor toCursor; // So it can delete the offending entry quickly, it likes a quick death.
};

//------------------------------------------------------------------------------
/// The graph object, contains a whole load of Vertex and Edge objects to 
/// represent a graph. You can insert your own types of vertices/edges into
/// this object, and even mix and match. Provides a central deletion ability, a
/// central access ability to iterate all bits, and something that can be passed
/// as a whole to graph algorithms.
class EOS_CLASS Graph : public Deletable
{
 public:
  /// &nbsp;
   Graph();

  /// &nbsp;
   ~Graph();


  /// Returns how many vertices are in the graph.
   nat32 Vertices() const {return verts.Size();}

  /// Returns how many edges are in the graph.
   nat32 Edges() const {return edges.Size();}

  /// Returns how much memory the graph is using, assuming it does not contain more
  /// memory in inherited versions of Vertex and Edge. You will have to
  /// count any added data yourself.
   nat32 Memory() const;

  /// Returns a list of all vertices in the graph. Do not add to/delete from the list, 
  /// this will probably cause a crash. Starts at the start of the list.
   List<Vertex*>::Cursor VertexList() {return verts.FrontPtr();}
 
  /// Returns a list of all edges in the graph. Do not add to/delete from the list, 
  /// this will probably cause a crash. Starts at the start of the list.
   List<Edge*>::Cursor EdgeList() {return edges.FrontPtr();}


  /// &nbsp;
   static inline cstrconst TypeString(){return "eos::ds::Graph";}


 private:
  List<Vertex*> verts; // A list of all vertices contained within.
  List<Edge*> edges; // A list of all edges contained within.
};

//------------------------------------------------------------------------------
 };
};
#endif
