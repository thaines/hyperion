//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines
#include "eos/sur/simplify.h"

#include "eos/ds/sort_lists.h"
#include "eos/ds/kd_tree.h"
#include "eos/sur/mesh_iter.h"
#include "eos/sur/mesh_sup.h"
#include "eos/math/iter_min.h"


namespace eos
{
 namespace sur
 {
//------------------------------------------------------------------------------
// Position accessor for putting Mesh::Vertex objects into a KD tree...
real64 VertexPos(const Vertex * v,nat32 dim)
{
 return v->Pos()[dim];
}

// Structure used for a forest...
struct RemDupNode
{
 nat32 parent; // Self for none, or index.
 nat32 size;
};

//------------------------------------------------------------------------------
EOS_FUNC void RemDups(Mesh * mesh,real32 range,time::Progress * prog)
{
 prog->Push();
 // Put all the vertices into a KD tree...
  prog->Report(0,4);
  ds::KdTree<Vertex,3,VertexPos> kd;
  ds::Array<Vertex> vert;
  {
   mesh->GetVertices(vert);
   for (nat32 i=0;i<vert.Size();i++) kd.Add(vert[i]);
   kd.Rebalance();
  }


 // Create a forest to store the merges - can't do them at the same time as
 // finding them as that causes problems...
  ds::Array<RemDupNode> forest(vert.Size());
  for (nat32 i=0;i<forest.Size();i++)
  {
   forest[i].parent = i;
   forest[i].size = 1;
  }


 // For each vertex check the KD tree for nearby vertices and store if need be...
  prog->Report(1,4);
  prog->Push();
  math::Vector<real64> min(3);
  math::Vector<real64> max(3);
  for (nat32 i=0;i<vert.Size();i++)
  {
   prog->Report(i,vert.Size());
   if (vert[i].Valid())
   {
    // Get the vertexes in the relevant range, should include once self...
     ds::List<Vertex> list;
     for (nat32 j=0;j<3;j++)
     {
      min[j] = vert[i].Pos()[j] - range;
      max[j] = vert[i].Pos()[j] + range;
     }
     kd.GetRange(min,max,list);
   
    // Iterate and merge them all...
     ds::List<Vertex>::Cursor targ = list.FrontPtr();
     real32 div = 1.0;
     while (!targ.Bad())
     {
      if ((*targ)!=vert[i])
      {
       // Its a merge - combine in the forest...
        // Get parents...
         nat32 parA = forest[targ->Index()].parent;
         nat32 parB = forest[vert[i].Index()].parent;
        
         while (forest[parA].parent!=parA) parA = forest[parA].parent;
         while (forest[parB].parent!=parB) parB = forest[parB].parent;
         
        // If different then merge...
         if (parA!=parB)
         {
          forest[parB].parent = parA;
          forest[parA].size += forest[parB].size;
         }
      }
      ++targ;
     }
     vert[i].Pos() /= div;
   }
  }
  prog->Pop();


 // Iterate the forest and do the merges...
  prog->Report(2,4);
  prog->Push();
  for (nat32 i=0;i<forest.Size();i++)
  {
   prog->Report(i,forest.Size());
   if (forest[i].parent!=i)
   {
    nat32 parent = forest[i].parent;
    while (forest[parent].parent!=parent) parent = forest[parent].parent;
    
    vert[parent].Pos() += vert[i].Pos();
    mesh->Fire(vert[i],vert[parent]);
   }
  }
  prog->Pop();
  
  
 // Iterate again and divide through the positions to set them to the means...
  prog->Report(3,4);
  prog->Push();
  for (nat32 i=0;i<forest.Size();i++)
  {
   prog->Report(i,forest.Size());
   if ((forest[i].parent==i)&&(forest[i].size!=1))
   {
    vert[i].Pos() /= real32(forest[i].size);
   }
  }
  prog->Pop();  
 
 prog->Pop();
}



EOS_FUNC void AddEdges(Mesh * mesh,real32 range,time::Progress * prog)
{
 prog->Push();
 // Put all the vertices into a KD tree...
  prog->Report(0,2);
  ds::KdTree<Vertex,3,VertexPos> kd;
  ds::Array<Vertex> vert;
  {
   mesh->GetVertices(vert);
   for (nat32 i=0;i<vert.Size();i++) kd.Add(vert[i]);
   kd.Rebalance();
  }
 
 // For each vertex check the KD tree for nearby vertices and make edges if need
 // be...
  prog->Report(1,2);
  prog->Push();
  math::Vector<real64> min(3);
  math::Vector<real64> max(3);
  for (nat32 i=0;i<vert.Size();i++)
  {
   prog->Report(i,vert.Size());
   
   // Get the vertexes in the relevant range, should include once self...
    ds::List<Vertex> list;
    for (nat32 j=0;j<3;j++)
    {
     min[j] = vert[i].Pos()[j] - range;
     max[j] = vert[i].Pos()[j] + range;
    }
    kd.GetRange(min,max,list);
   
   // Iterate and make sure there are edges for all...
    ds::List<Vertex>::Cursor targ = list.FrontPtr();
    while (!targ.Bad())
    {
     if ((*targ)!=vert[i]) // Not the same vertex.
     {
      mesh->NewEdge(*targ,vert[i]); // Only creates edges if they don't already exist.
     }
     ++targ;
    }
  }
  prog->Pop();
 
 prog->Pop();
}

//------------------------------------------------------------------------------
Simplify::Simplify()
:mesh(null<Mesh*>()),mergeDist(0.01),edgeCost(1.0)
{}

Simplify::~Simplify()
{}

void Simplify::Set(Mesh * m)
{
 mesh = m;
}

void Simplify::SetMerge(real32 dist)
{
 mergeDist = dist;
}

void Simplify::SetEdge(real32 cost)
{
 edgeCost = cost;
}

void Simplify::Run(nat32 targVerts,time::Progress * prog)
{
 LogTime("eos::sur::Simplify::Run");
 log::Assert(mesh!=null<Mesh*>());
 prog->Push(); 
 nat32 steps = 3;
 nat32 step = 0;
 
 // If needed add edges between near vertices...
  if (!math::IsZero(mergeDist))
  {
   steps += 2;
   prog->Report(step++,steps); 
   AddEdges(mesh,mergeDist,prog);
  }


 // Calculate the error matrix for each vertex...
  prog->Report(step++,steps);
  prog->Push();
  ds::Array<Vertex> startVerts(mesh->VertexCount());
  mesh->GetVertices(startVerts);
  
  ds::Array<ErrorV> vertError(startVerts.Size()); // This is mantained throughout the algorithm.
  for (nat32 i=0;i<startVerts.Size();i++)
  {
   LogTime("eos::sur::Simplify::Run vert");
   prog->Report(i,startVerts.Size());
   vertError[i].Zero();
   
   // Iterate all faces using the target vertex and add in there contribution...
    IterVertexFaces iter(startVerts[i]);
    while (iter.Valid())
    {
     // Get the plane equation...
      bs::Normal norm;
      real32 dist;
      iter.Targ().Eq(norm,dist);
     
     // Update the matrix...
      vertError[i].r0[0] += math::Sqr(norm[0]);
      vertError[i].r0[1] += norm[0]*norm[1];
      vertError[i].r0[2] += norm[0]*norm[2];
      vertError[i].r0[3] += norm[0]*dist;
      vertError[i].r1[0] += math::Sqr(norm[1]);
      vertError[i].r1[1] += norm[1]*norm[2];
      vertError[i].r1[2] += norm[1]*dist;
      vertError[i].r2[0] += math::Sqr(norm[2]);
      vertError[i].r2[1] += norm[2]*dist;
      vertError[i].r3[0] += math::Sqr(dist);
      
     iter.Next();
    }
    
   
   // Iterate the edges that use the target vertex, for each one that is on
   // a boundary add in its edge constraint effect...
    if (!math::IsZero(edgeCost))
    {
     IterVertexEdges iter(startVerts[i]);
     while (iter.Valid())
     {
      Edge edge = iter.Targ();
      if (edge.Boundary())
      {
       // Get the relevant normals...
        bs::Normal faceNorm;
        {
         real32 dist;
         edge.AnyFace().Eq(faceNorm,dist);
        }
    
        bs::Normal edgeNorm;
        for (nat32 j=0;j<3;j++) edgeNorm[j] = edge.VertexA().Pos()[j] - edge.VertexB().Pos()[j];
    
       // Calculate the plane parameters...
        bs::Normal norm;
        math::CrossProduct(faceNorm,edgeNorm,norm);
        if (!math::IsZero(norm.LengthSqr()))
        {
         norm.Normalise();
         real32 dist = -(norm * edge.VertexA().Pos());
     
         // Update the matrix...
          vertError[i].r0[0] += edgeCost*math::Sqr(norm[0]);
          vertError[i].r0[1] += edgeCost*norm[0]*norm[1];
          vertError[i].r0[2] += edgeCost*norm[0]*norm[2];
          vertError[i].r0[3] += edgeCost*norm[0]*dist;
          vertError[i].r1[0] += edgeCost*math::Sqr(norm[1]);
          vertError[i].r1[1] += edgeCost*norm[1]*norm[2];
          vertError[i].r1[2] += edgeCost*norm[1]*dist;
          vertError[i].r2[0] += edgeCost*math::Sqr(norm[2]);
          vertError[i].r2[1] += edgeCost*norm[2]*dist;
          vertError[i].r3[0] += edgeCost*math::Sqr(dist);
        }
      }
      iter.Next();
     }
    }
  }
  prog->Pop();


 
 // Create the data structure - this is a touch complicated...
  ds::SortList<ContractE,IndexSortContractE> byIndex; // Contains actual data.
  ds::SortList<ContractE*,CostSortContractE> byCost; // Contains pointers to actual data, as stored in above.



 // Calculate the edge data...
 // This involves filling the above data structure with its initial data set.
 prog->Report(step++,steps);
 {
  prog->Push();
  ds::Array<Edge> startEdges(mesh->EdgeCount());
  mesh->GetEdges(startEdges);
  
  for (nat32 i=0;i<startEdges.Size();i++)
  {
   LogTime("eos::sur::Simplify::Run edge");
   prog->Report(i,startEdges.Size());
   // Construct the contraction...
    ContractE ce;
    ce.edge = startEdges[i];
    Vertex a = ce.edge.VertexA();
    Vertex b = ce.edge.VertexB();
    ce.FillIn(a.Pos(),vertError[a.Index()],b.Pos(),vertError[b.Index()]);
    
   // Add to data structure, but only if safe...
    if (ce.Safe())
    {
     byIndex.Add(ce);
     byCost.Add(byIndex.Get(ce));
    }
  }
  prog->Pop();
 }



 // Do the actual algorithm - do edge contractions till the mesh is simple enough...
  prog->Report(step++,steps);
  prog->Push();
  MeshTransfer mTra(*mesh,*mesh);
  nat32 startSize = mesh->VertexCount();
  while ((mesh->VertexCount()>targVerts)&&(byCost.Size()>0))
  {
   LogTime("eos::sur::Simplify::Run contract");
   prog->Report(startSize-mesh->VertexCount(),startSize-targVerts);
   // Get and remove the lowest cost contraction...
    ContractE job = *byCost.First();
    byCost.Rem(&job);
    byIndex.Rem(job);
    
    Vertex vert[2];
    vert[0] = job.edge.VertexA();
    vert[1] = job.edge.VertexB();


   // Remove from the data structure all contractions involving the two vertices...
    for (nat32 i=0;i<2;i++)
    {
     IterVertexEdges iter(vert[i]);
     while(iter.Valid())
     {
      ContractE dummy; dummy.edge = iter.Targ();
      ContractE * targ = byIndex.Get(dummy);
      if (targ)
      {
       byCost.Rem(targ);
       byIndex.Rem(*targ);
      }
      iter.Next();
     }
    }


   // Do the contraction, update the vertex error array...
    // Work out the interpolation weights...
     real32 weight[2];
     for (nat32 i=0;i<2;i++) weight[i] = job.newPos.DistanceTo(vert[(i+1)%2].Pos());

    // Create the new vertex...
     Vertex nv = mTra.Interpolate(2,vert,weight);
     nv.Pos() = job.newPos;
    
    // Update the vertex array...
     nv.Index() = math::Min(vert[0].Index(),vert[1].Index());
     vertError[nv.Index()] += vertError[math::Max(vert[0].Index(),vert[1].Index())];
    
    // Merge the two old vertices into this new vertex...
     for (nat32 i=0;i<2;i++) mesh->Fire(vert[i],nv);


   // Iterate the associate edges, recalculate there costs, reinsert into data structure...
   {
    IterVertexEdges iter(nv);
    while (iter.Valid())
    {
     ContractE nce;
     nce.edge = iter.Targ();

     Vertex a = nce.edge.VertexA();
     Vertex b = nce.edge.VertexB();
       
     nce.FillIn(a.Pos(),vertError[a.Index()],b.Pos(),vertError[b.Index()]);
     
     if (nce.Safe())
     {
      byIndex.Add(nce);      
      byCost.Add(byIndex.Get(nce));
     }

     iter.Next();
    }
   }
  }
  prog->Pop();
  


 // If needed remove unused edges...
  if (!math::IsZero(mergeDist))
  {
   prog->Report(step++,steps);
   mesh->Prune();
  }

 prog->Pop();
}
   
void Simplify::RunP(real32 num,time::Progress * prog)
{
 log::Assert(mesh!=null<Mesh*>());
 Run(nat32(real32(mesh->VertexCount())*num),prog);
}

void Simplify::ContractE::FillIn(const bs::Vert & pa,const Simplify::ErrorV & ea,
                                 const bs::Vert & pb,const Simplify::ErrorV & eb)
{
 // Error matrix for combined vertices...
  ErrorV err = ea;
  err += eb;
    
 // Try the first approach - go direct to the position by screwing with matrices...
  // Construct the matrix to be inverted...
   math::Mat<4,4> mat;
   mat[0][0] = err.r0[0]; mat[0][1] = err.r0[1]; mat[0][2] = err.r0[2]; mat[0][3] = err.r0[3];
   mat[1][0] = err.r0[1]; mat[1][1] = err.r1[0]; mat[1][2] = err.r1[1]; mat[1][3] = err.r1[2];
   mat[2][0] = err.r0[2]; mat[2][1] = err.r1[1]; mat[2][2] = err.r2[0]; mat[2][3] = err.r2[1];
   mat[3][0] = 0.0;       mat[3][1] = 0.0;       mat[3][2] = 0.0;       mat[3][3] = 1.0;
   
  // Try and invert it, on success do the other bits...
   math::Mat<4,4> temp;
   if (math::Inverse(mat,temp))
   {
    newPos[0] = mat[0][3]/mat[3][3];
    newPos[1] = mat[1][3]/mat[3][3];
    newPos[2] = mat[2][3]/mat[3][3];
    
    cost = err.Cost(newPos);
    if (math::IsFinite(cost)) return;
   }


 // First approach has failed, now try the third approach, as initialisation for an attempt on the second...
  bs::Vert half = pa;
  half += pb;
  half *= 0.5;
  
  real32 ca = err.Cost(pa);
  real32 ch = err.Cost(half);
  real32 cb = err.Cost(pb);

  if (ca<ch)
  {
   if (ca<cb) {newPos = pa; cost = ca;}
         else {newPos = pb; cost = cb;}
  }
  else
  {
   if (ch<cb) {newPos = half; cost = ch;}
         else {newPos = pb;   cost = cb;}
  }

  
 // This isn't part of the original algorithms specs, but screw it, should work better...
 // (This is bloody slow, relying on the first approach working most times.)
  cost = math::LM(newPos,err,ErrorV::CostFunc);
}

bit Simplify::ContractE::Safe() const
{
 return Mesh::SafeContraction(edge,newPos);
}

//------------------------------------------------------------------------------
 };
};
