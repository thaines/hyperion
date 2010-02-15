//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/ds/graphs.h"

#include "eos/math/functions.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
Vertex::Vertex(Graph & g)
:graph(g),graphCursor(g.VertexList())
{
 graphCursor.AddBefore(this); log::Assert(*graphCursor==this);
}

Vertex::~Vertex()
{
 // First kill off the edges...
  while (edges.Size()!=0)
  {
   delete edges.Front();
  }

 // Then kill of the connection from it to the graph...
  graphCursor.RemNext();
}

Edge * Vertex::Connection(Vertex * vert) const
{
 List<Edge*>::Cursor targ = const_cast<Vertex*>(this)->edges.FrontPtr();
 while (!targ.Bad())
 {
  if (((*targ)->From()==vert)||((*targ)->To()==vert)) return (*targ);
  ++targ;
 }
 return null<Edge*>();
}

void Vertex::Merge(Vertex * victim)
{
 List<Edge*>::Cursor targ = victim->EdgeList();
 while (!targ.Bad())
 {
  Vertex * other = (*targ)->From();
  if (other==victim) other = (*targ)->To();

  if ((other!=this)&&(other!=victim)&&(Connection(other)==null<Edge*>()))
  {
   Edge * edge = *targ;
   ++targ;
   if (edge->From()==victim) edge->SetFrom(this);
                        else edge->SetTo(this);
  }
  else ++targ;
 }
 delete victim;
}

//------------------------------------------------------------------------------
Edge::Edge(Graph & g,Vertex * f,Vertex * t)
:graph(g),graphCursor(g.EdgeList()),
from(f),fromCursor(f->EdgeList()),
to(t),toCursor(t->EdgeList())
{
 graphCursor.AddBefore(this); log::Assert(*graphCursor==this);
 fromCursor.AddBefore(this); log::Assert(*fromCursor==this);
 toCursor.AddBefore(this); log::Assert(*toCursor==this);
}

Edge::~Edge()
{
 log::Assert(*graphCursor==this); graphCursor.RemNext();
 log::Assert(*fromCursor==this); fromCursor.RemNext();
 log::Assert(*toCursor==this); toCursor.RemNext();
}

void Edge::SetFrom(Vertex * f)
{
 log::Assert(*fromCursor==this); fromCursor.RemNext();
 from = f;
 fromCursor = from->EdgeList();
 fromCursor.AddBefore(this); log::Assert(*fromCursor==this);
}

void Edge::SetTo(Vertex * t)
{
 log::Assert(*toCursor==this); toCursor.RemNext();
 to = t;
 toCursor = to->EdgeList();
 toCursor.AddBefore(this); log::Assert(*toCursor==this);
}

//------------------------------------------------------------------------------
Graph::Graph()
{}

Graph::~Graph()
{
 while (verts.Size()!=0)
 {
  delete verts.Front();
 }
}

nat32 Graph::Memory() const
{
 nat32 ret = sizeof(Graph);
  ret += verts.Size() * (sizeof(Vertex) + 3*sizeof(void*));
  ret += edges.Size() * (sizeof(Edge) + 9*sizeof(void*)); // This line is based on intimate knoledge of the list structure.
 return ret;
}

//------------------------------------------------------------------------------
 };
};
