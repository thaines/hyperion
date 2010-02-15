//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

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

#include "eos/alg/greedy_merge.h"

#include "eos/file/csv.h"
#include "eos/ds/priority_queues.h"

namespace eos
{
 namespace alg
 {
//------------------------------------------------------------------------------
GreedyMerge::GreedyMerge()
:gmi(null<GreedyMergeInterface*>())
{}

GreedyMerge::~GreedyMerge()
{
 delete gmi;
}

void GreedyMerge::Set(GreedyMergeInterface * g)
{
 delete gmi;
 gmi = g;
}

void GreedyMerge::SetNodeCount(nat32 num)
{
 in.Size(num);
}

void GreedyMerge::SetNodeData(nat32 ind,Deletable * data)
{
 delete in[ind];
 in[ind] = data;
}

void GreedyMerge::AddEdge(nat32 a,nat32 b)
{
 Edge e;
 e.a = a;
 e.b = b;
 
 inEdge.AddBack(e);
}

void GreedyMerge::Run(time::Progress * prog)
{
 LogBlock("eos::alg::GreedyMerge::Run","-");
 prog->Push();
 
 // Build an editable graph structure and fill in its nodes, calculate costs for every node...
  prog->Report(0,4);
  LG graph;  

  prog->Push();
  ds::Array<LG::NodeHand> nh(in.Size());
  for (nat32 i=0;i<nh.Size();i++)
  {
   LogTime("eos::alg::GreedyMerge::Run node");
   prog->Report(i,nh.Size());

   nh[i] = graph.AddNode();
   nh[i]->cost = gmi->Cost(in[i]);
   nh[i]->data = in[i];
  }
  prog->Pop();



 // Create A work queue, fill it in with every possible work item...
  prog->Report(1,4);
  ds::PriorityQueue<WorkItem> workQueue(inEdge.Size()*2);
  graph.AddLayer();
  
  prog->Push();
  {
   ds::List<Edge>::Cursor targ = inEdge.FrontPtr();
   for (nat32 i=0;i<inEdge.Size();i++)
   {
    LogTime("eos::alg::GreedyMerge::Run edge");
    prog->Report(i,inEdge.Size());
    
    // Add edge to graph...
     graph.AddEdge(0,nh[targ->a],nh[targ->b]);
     
    // Calculate cost of merging the nodes associated with the edge, compare to 
    // cost of not merging and store work item if its an improvment...
     real32 mergedCost = gmi->Cost(nh[targ->a]->data,nh[targ->b]->data);
     real32 unmergedCost = nh[targ->a]->cost + nh[targ->b]->cost;
     if (mergedCost<unmergedCost)
     {
      WorkItem wi;
      wi.costDec = unmergedCost - mergedCost;
      wi.a = nh[targ->a];
      wi.b = nh[targ->b];
      
      workQueue.Add(wi);
     }

    ++targ;
   }
  }
  prog->Pop();



 // Keep eatting the work queue, grabbing the best job to do at each bite.
 // When a job is eatten add on all consequental jobs, keep going until the
 // queue is empty.
 // Note that work items can be invalidated by earlier work items...
  prog->Report(2,4);
  prog->Push();
  nat32 done = 0;
  while (workQueue.Size()!=0)
  {
   LogTime("eos::alg::GreedyMerge::Run job");
   prog->Report(done,math::Min(done + workQueue.Size(),in.Size()-1));

   // Remove the best operation...
    WorkItem wi = workQueue.Peek();
    workQueue.Rem();

   // Check its still valid...
    if ((!wi.a.Current())||(!wi.b.Current())) continue;
    ++done;

   // Apply it...
    LG::NodeHand nn = graph.MergeNodes(wi.a,wi.b);
    nn->cost = wi.a->cost + wi.b->cost - wi.costDec;
    nn->data = gmi->Merge(wi.a->data,wi.b->data);
   
    delete wi.a->data;
    delete wi.b->data;
   
   // Generate all follow through work items...
    LG::EdgeIter targ = nn.LayerFront().Targ().EdgeFront();
    while (!targ.Bad())
    {
     LogTime("eos::alg::GreedyMerge::Run new_job");
     LG::NodeHand na = targ.Targ().A().GetNode();
     LG::NodeHand nb = targ.Targ().B().GetNode();
     
     real32 mergedCost = gmi->Cost(na->data,nb->data);
     real32 unmergedCost = na->cost + nb->cost;
     if (mergedCost<unmergedCost)
     {
      WorkItem wi;
      wi.costDec = unmergedCost - mergedCost;
      wi.a = na;
      wi.b = nb;
      
      workQueue.Add(wi);
     }
    
     ++targ;
    }
  }
  prog->Pop();



 // Extract the final graph, clean up...
  prog->Report(3,4);
  
  // First pass - count outputs and assign indices...
  {
   nat32 count = 0;
   LG::NodeIter targ = graph.NodeFront();
   while (!targ.Bad())
   {
    targ.Targ()->index = count;
    ++count;
    ++targ;
   }
   
   out.Size(count);
  }
  
  // Second pass - extract outputs into there indices...
  {
   LG::NodeIter targ = graph.NodeFront();
   while (!targ.Bad())
   {
    delete out[targ.Targ()->index];
    out[targ.Targ()->index] = targ.Targ()->data;
    ++targ;
   }
  }
  
  // First pass to count all the edges...
  {
   LG::EdgeIter targ = graph.EdgeFront();
   nat32 count = 0;
   while (!targ.Bad())
   {
    count += 1;
    ++targ;
   }
   
   outEdge.Size(count);
  }

  // Second pass to get them...
  {
   LG::EdgeIter targ = graph.EdgeFront();
   nat32 count = 0;
   while (!targ.Bad())
   {
    LG::NodeHand na = targ.Targ().A().GetNode();
    LG::NodeHand nb = targ.Targ().B().GetNode();
    
    outEdge[count].a = na->index;
    outEdge[count].b = nb->index;
   
    count += 1;
    ++targ;
   }
  }

  // Fill in original to output mapping...
   inToOut.Size(nh.Size());
   for (nat32 i=0;i<nh.Size();i++)
   {
    LG::NodeHand targ = nh[i].GetCurrent();
    inToOut[i] = targ->index;
   }
  
  // Null input Deletable pointer - have been screwed with by now...
   for (nat32 i=0;i<in.Size();i++) in[i] = null<Deletable*>();


 prog->Pop();
}
nat32 GreedyMerge::Nodes() const
{
 return out.Size();
}

const Deletable * GreedyMerge::Data(nat32 ind) const
{
 return out[ind];
}

nat32 GreedyMerge::InToOut(nat32 i) const
{
 return inToOut[i];
}

nat32 GreedyMerge::Edges() const
{
 return outEdge.Size();
}

void GreedyMerge::GetEdge(nat32 index,nat32 & outA,nat32 & outB)
{
 outA = outEdge[index].a;
 outB = outEdge[index].b;
}

//------------------------------------------------------------------------------
 };
};
