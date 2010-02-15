//-----------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/filter/seg_graph.h"

#include "eos/ds/arrays2d.h"

namespace eos
{
 namespace filter
 {
//-----------------------------------------------------------------------------
SegGraph::SegGraph(const svt::Field<nat32> & segs,nat32 segments)
{
 sed.Size(segments);

 // As its only a tempory data structure we simply use a matrix
 // representation, to indicate linking edges...
  ds::Array2D<nat32> mat(segments,segments);
  for (nat32 y=0;y<mat.Height();y++)
  {
   for (nat32 x=0;x<mat.Width();x++) mat.Get(x,y) = 0;
  }
 
 // Pass to store all links...
  for (nat32 y=0;y<segs.Size(1);y++)
  {
   for (nat32 x=0;x<segs.Size(0);x++)
   {
    if (x!=segs.Size(0)-1) mat.Get(segs.Get(x,y),segs.Get(x+1,y)) += 1;
    if (y!=segs.Size(1)-1) mat.Get(segs.Get(x,y),segs.Get(x,y+1)) += 1;
   }
  }
  
 // Add the matrix to its transpose, ignoring the diagonal...
  for (nat32 y=0;y<mat.Height();y++)
  {
   for (nat32 x=y+1;x<mat.Width();x++)
   {
    mat.Get(x,y) += mat.Get(y,x);
    mat.Get(y,x) = mat.Get(x,y);
   }
  }  
 
 // Re-map the data into the stored data structure, which has obvious size advantages...
  for (nat32 y=0;y<mat.Height();y++)
  {
   nat32 count = 0;
   for (nat32 x=0;x<y;x++)
   {
    if (mat.Get(x,y)>0) ++count;
   }
   for (nat32 x=y+1;x<mat.Width();x++)
   {
    if (mat.Get(x,y)>0) ++count;
   }
   
   sed[y].size = count;
   sed[y].data = new Pair<nat32,nat32>[count];
   
   nat32 pos = 0;
   for (nat32 x=0;x<y;x++)
   {
    if (mat.Get(x,y)>0)
    {
     sed[y].data[pos].first = x;
     sed[y].data[pos].second = mat.Get(x,y);     
     ++pos;    
    }
   }
   for (nat32 x=y+1;x<mat.Width();x++)
   {
    if (mat.Get(x,y)>0)
    {
     sed[y].data[pos].first = x;
     sed[y].data[pos].second = mat.Get(x,y);     
     ++pos;
    }
   }
  }	
}

SegGraph::~SegGraph()
{
 for (nat32 i=0;i<sed.Size();i++) delete[] sed[i].data;
}

nat32 SegGraph::Segments()
{
 return sed.Size();	
}

nat32 SegGraph::NeighbourCount(nat32 seg)
{
 return sed[seg].size;	
}

nat32 SegGraph::Neighbour(nat32 seg,nat32 n)
{
 return sed[seg].data[n].first;	
}

nat32 SegGraph::BorderSize(nat32 seg,nat32 n)
{
 return sed[seg].data[n].second;
}

//-----------------------------------------------------------------------------
 };
};
