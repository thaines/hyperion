//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/sur/mesh.h"

#include "eos/svt/field.h"

namespace eos
{
 namespace sur
 {
//------------------------------------------------------------------------------
Mesh::Mesh(str::TokenTable * t)
:tt(t),
vertPropChanged(false),edgePropChanged(false),facePropChanged(false),
vertPropSize(sizeof(Vertex)),edgePropSize(sizeof(DirEdge)*2),facePropSize(sizeof(Face))
{}

Mesh::~Mesh()
{
 if (verts.Size()!=0)
 {
  ds::SortList<Vertex*>::Cursor targ = verts.FrontPtr();
  while (!targ.Bad())
  {
   sur::Vertex v(*targ);
   ++targ;
   Del(v);
  }
 }

 for (nat32 i=0;i<vertProp.Size();i++) mem::Free(vertProp[i].ini);
 for (nat32 i=0;i<edgeProp.Size();i++) mem::Free(edgeProp[i].ini);
 for (nat32 i=0;i<faceProp.Size();i++) mem::Free(faceProp[i].ini);
}

void Mesh::SetTT(str::TokenTable * t)
{
 tt = t;
}

void Mesh::GetVertices(ds::List<sur::Vertex> & out) const
{
 if (verts.Size()!=0)
 {
  ds::SortList<Vertex*>::Cursor targ = verts.FrontPtr();
  nat32 i = 0;
  while (!targ.Bad())
  {
   out.AddBack(sur::Vertex(*targ));
   (*targ)->ind = i;
   ++i;
   ++targ;
  }
 }
}

void Mesh::GetVertices(ds::Array<sur::Vertex> & out) const
{
 out.Size(verts.Size());
 if (verts.Size()!=0)
 {
  ds::SortList<Vertex*>::Cursor targ = verts.FrontPtr();
  for (nat32 i=0;i<out.Size();i++)
  {
   out[i] = sur::Vertex(*targ);
   (*targ)->ind = i;
   ++targ;
  }
 }
}

void Mesh::GetEdges(ds::List<sur::Edge> & out) const
{
 if (edges.Size()!=0)
 {
  ds::SortList<DirEdge*>::Cursor targ = edges.FrontPtr();
  nat32 i = 0;
  while (!targ.Bad())
  {
   DirEdge * t = math::Min(*targ,(*targ)->partner);
   out.AddBack(sur::Edge(t));
   t->ind = i;
   ++i;
   ++targ;
  }
 }
}

void Mesh::GetEdges(ds::Array<sur::Edge> & out) const
{
 out.Size(edges.Size());
 if (edges.Size()!=0)
 {
  ds::SortList<DirEdge*>::Cursor targ = edges.FrontPtr();
  for (nat32 i=0;i<out.Size();i++)
  {
   DirEdge * t = math::Min(*targ,(*targ)->partner); 
   out[i] = sur::Edge(t);
   t->ind = i;
   ++targ;
  }
 }
}

void Mesh::GetFaces(ds::List<sur::Face> & out) const
{
 if (faces.Size()!=0)
 {
  ds::SortList<Face*>::Cursor targ = faces.FrontPtr();
  nat32 i = 0;
  while (!targ.Bad())
  {
   out.AddBack(sur::Face(*targ));
   (*targ)->ind = i;
   ++i;
   ++targ;
  }
 }
}

void Mesh::GetFaces(ds::Array<sur::Face> & out) const
{
 out.Size(faces.Size());
 if (faces.Size()!=0)
 {
  ds::SortList<Face*>::Cursor targ = faces.FrontPtr();
  for (nat32 i=0;i<out.Size();i++)
  {
   out[i] = sur::Face(*targ);
   (*targ)->ind = i;
   ++targ;
  }
 }
}

sur::Vertex Mesh::NewVertex(const bs::Vert & pos)
{
 Vertex * nv = (Vertex*)(void*)mem::Malloc<byte>(vertPropSize);
 for (nat32 i=0;i<vertProp.Size();i++)
 {
  if (vertProp[i].state!=Prop::Added)
  {
   mem::Copy((byte*)nv + vertProp[i].offset,vertProp[i].ini,vertProp[i].size);
  }
 }

 verts.Add(nv);

 nv->edge = null<DirEdge*>();
 nv->pos = pos;

 return sur::Vertex(nv);
}

sur::Edge Mesh::NewEdge(sur::Vertex a,sur::Vertex b)
{
 sur::Edge ret = a.Link(b);
  if (!ret.Valid())
  {
   DirEdge * a2b = (DirEdge*)(void*)mem::Malloc<byte>(edgePropSize);
   DirEdge * b2a = a2b + 1;
   for (nat32 i=0;i<edgeProp.Size();i++)
   {
    if (edgeProp[i].state!=Prop::Added)
    {
     mem::Copy((byte*)a2b + edgeProp[i].offset,edgeProp[i].ini,edgeProp[i].size);
    }
   }
   edges.Add(a2b);

   a2b->to = b.vert;
   a2b->partner = b2a;
   if (a.vert->edge==null<DirEdge*>())
   {
    a2b->next = a2b;
    a2b->prev = a2b;
    a.vert->edge = a2b;
   }
   else
   {
    a2b->next = a.vert->edge;
    a2b->prev = a.vert->edge->prev;
    a2b->next->prev = a2b;
    a2b->prev->next = a2b;
   }
   a2b->user = null<HalfEdge*>();

   b2a->to = a.vert;
   b2a->partner = a2b;
   if (b.vert->edge==null<DirEdge*>())
   {
    b2a->next = b2a;
    b2a->prev = b2a;
    b.vert->edge = b2a;
   }
   else
   {
    b2a->next = b.vert->edge;
    b2a->prev = b.vert->edge->prev;
    b2a->next->prev = b2a;
    b2a->prev->next = b2a;
   }
   b2a->user = null<HalfEdge*>();

   ret = sur::Edge(a2b);
  }
 return ret;
}

sur::Face Mesh::NewFace(const ds::Array<sur::Vertex> & verts)
{
 log::Assert(verts.Size()>=3,"Mesh::NewFace");

 // Create the face so we know its pointer...
  Face * nf = (Face*)(void*)mem::Malloc<byte>(facePropSize);
  for (nat32 i=0;i<faceProp.Size();i++)
  {
   if (faceProp[i].state!=Prop::Added)
   {
    mem::Copy((byte*)nf + faceProp[i].offset,faceProp[i].ini,faceProp[i].size);
   }
  }
  faces.Add(nf);
  nf->size = verts.Size();

 // Create the edge ring, using the new edge method to provide edge construction...
  HalfEdge * lhe = null<HalfEdge*>();
  for (nat32 i=0;i<verts.Size();i++)
  {
   HalfEdge * nhe = new HalfEdge();

   nhe->face = nf;
   if (lhe) lhe->chain = nhe;
       else nf->edge = nhe;

   sur::Edge em = NewEdge(verts[i],verts[(i+1)%verts.Size()]);
   nhe->edge = (em.edge->to==verts[i].vert)?em.edge->partner:em.edge;

   if (nhe->edge->user==null<HalfEdge*>())
   {
    nhe->edge->user = nhe;
    nhe->next = nhe;
    nhe->prev = nhe;
   }
   else
   {
    nhe->next = nhe->edge->user;
    nhe->prev = nhe->edge->user->prev;
    nhe->next->prev = nhe;
    nhe->prev->next = nhe;
   }

   lhe = nhe;
  }
  lhe->chain = nf->edge;

 // Return a handle to the new face...
  return sur::Face(nf);
}

sur::Face Mesh::NewFace(const ds::List<sur::Vertex> & verts)
{
 ds::Array<sur::Vertex> data(verts.Size());

 ds::List<sur::Vertex>::Cursor targ = verts.FrontPtr();
 for (nat32 i=0;i<data.Size();i++) {data[i] = *targ; ++targ;}

 return NewFace(data);
}

sur::Face Mesh::NewFace(sur::Vertex a,sur::Vertex b,sur::Vertex c)
{
 sur::Vertex v[3];
 v[0] = a;
 v[1] = b;
 v[2] = c;

 // Create the face so we know its pointer...
  Face * nf = (Face*)(void*)mem::Malloc<byte>(facePropSize);
  for (nat32 i=0;i<faceProp.Size();i++)
  {
   if (faceProp[i].state!=Prop::Added)
   {
    mem::Copy((byte*)nf + faceProp[i].offset,faceProp[i].ini,faceProp[i].size);
   }
  }
  faces.Add(nf);
  nf->size = 3;

 // Create the edge ring, using the new edge method to provide edge construction...
  HalfEdge * he[3];
  for (nat32 i=0;i<3;i++) he[i] = new HalfEdge();
  nf->edge = he[0];

  for (nat32 i=0;i<3;i++)
  {
   he[i]->face = nf;
   he[i]->chain = he[(i+1)%3];

   sur::Edge em = NewEdge(v[i],v[(i+1)%3]);
   he[i]->edge = (em.edge->to==v[i].vert)?em.edge->partner:em.edge;

   if (he[i]->edge->user==null<HalfEdge*>())
   {
    he[i]->edge->user = he[i];
    he[i]->next = he[i];
    he[i]->prev = he[i];
   }
   else
   {
    he[i]->next = he[i]->edge->user;
    he[i]->prev = he[i]->edge->user->prev;
    he[i]->next->prev = he[i];
    he[i]->prev->next = he[i];
   }
  }

 // Return a handle to the new face...
  return sur::Face(nf);
}

sur::Face Mesh::NewFace(sur::Vertex a,sur::Vertex b,sur::Vertex c,sur::Vertex d)
{
 sur::Vertex v[4];
 v[0] = a;
 v[1] = b;
 v[2] = c;
 v[3] = d;

 // Create the face so we know its pointer...
  Face * nf = (Face*)(void*)mem::Malloc<byte>(facePropSize);
  for (nat32 i=0;i<faceProp.Size();i++)
  {
   if (faceProp[i].state!=Prop::Added)
   {
    mem::Copy((byte*)nf + faceProp[i].offset,faceProp[i].ini,faceProp[i].size);
   }
  }
  faces.Add(nf);
  nf->size = 4;

 // Create the edge ring, using the new edge method to provide edge construction...
  HalfEdge * he[4];
  for (nat32 i=0;i<4;i++) he[i] = new HalfEdge();
  nf->edge = he[0];

  for (nat32 i=0;i<4;i++)
  {
   he[i]->face = nf;
   he[i]->chain = he[(i+1)%4];

   sur::Edge em = NewEdge(v[i],v[(i+1)%4]);
   he[i]->edge = (em.edge->to==v[i].vert)?em.edge->partner:em.edge;

   if (he[i]->edge->user==null<HalfEdge*>())
   {
    he[i]->edge->user = he[i];
    he[i]->next = he[i];
    he[i]->prev = he[i];
   }
   else
   {
    he[i]->next = he[i]->edge->user;
    he[i]->prev = he[i]->edge->user->prev;
    he[i]->next->prev = he[i];
    he[i]->prev->next = he[i];
   }
  }

 // Return a handle to the new face...
  return sur::Face(nf);
}

void Mesh::Del(sur::Vertex vert)
{
 while (vert.vert->edge) Del(sur::Edge(vert.vert->edge));

 verts.Rem(vert.vert);
 mem::Free(vert.vert);
}

void Mesh::Del(sur::Edge edge)
{
 // Delete faces that use the edge...
  DirEdge * e1 = edge.edge;
  DirEdge * e2 = e1->partner;

  while (e1->user) Del(sur::Face(e1->user->face));
  while (e2->user) Del(sur::Face(e2->user->face));


 // Remove the edge...
  // e1...
   if (e1->next==e1) e2->to->edge = null<DirEdge*>();
   else
   {
    e1->next->prev = e1->prev;
    e1->prev->next = e1->next;
    e2->to->edge = e1->next;
   }

  // e2...
   if (e2->next==e2) e1->to->edge = null<DirEdge*>();
   else
   {
    e2->next->prev = e2->prev;
    e2->prev->next = e2->next;
    e1->to->edge = e2->next;
   }

 // Deletion...
  DirEdge * first = math::Min(e1,e2); // Yeah, think about it... :-P
  edges.Rem(first);
  mem::Free(first); // They share storage.
}

void Mesh::Del(sur::Face face)
{
 // Remove the half edges...
  HalfEdge * targ = face.face->edge;
  for (nat32 i=0;i<face.face->size;i++)
  {
   HalfEdge * victim = targ;
   targ = targ->chain;

   if (victim->next==victim) victim->edge->user = null<HalfEdge*>();
   else
   {
    victim->next->prev = victim->prev;
    victim->prev->next = victim->next;
    victim->edge->user = victim->next;
   }

   delete victim;
  }


 // Remove the face...
  faces.Rem(face.face);
  mem::Free(face.face);
}

void Mesh::Fire(sur::Vertex toDie,sur::Vertex replacement)
{
 LogTime("eos::sur::Mesh::Fire");
 // Special case the replacement vertex having nothing attached to it, 
 // to save time as this is suprisingly comon...
  if (replacement.vert->edge==null<DirEdge*>())
  {
   // Move over the edge and update pointers back to the vertex...
    replacement.vert->edge = toDie.vert->edge;
    if (replacement.vert->edge)
    {
     DirEdge * targ = replacement.vert->edge;
     do
     {
      targ->partner->to = replacement.vert;
      targ = targ->next;
     }
     while (targ!=replacement.vert->edge);
    }
   
   // Delete the old vertex...
    verts.Rem(toDie.vert);
    mem::Free(toDie.vert);
  
   return;
  }


 // Transfer over all the edges, regardless of if they should be transfered or 
 // not, we then use a second pass to fix the errors...
  while (toDie.vert->edge)
  {
   // Get target edge...
    DirEdge * e = toDie.vert->edge;
   
   // Remove from dying vertex...
    if (e->next==e) toDie.vert->edge = null<DirEdge*>();
    else
    {
     e->next->prev = e->prev;
     e->prev->next = e->next;
     toDie.vert->edge = e->next;
    }
   
   // Insert into its replacement...
    e->partner->to = replacement.vert;
    if (replacement.vert->edge)
    {
     e->next = replacement.vert->edge;
     e->prev = e->next->prev;
     
     e->next->prev = e;
     e->prev->next = e;
    }
    else
    {
     replacement.vert->edge = e;
     e->next = e;
     e->prev = e;
    }
  }



 // Terminate...
  verts.Rem(toDie.vert);
  mem::Free(toDie.vert);



 // Loop all edges of the replacement vertex, checking each one is valid.
 // We have two types of badness - loops and duplicates. Duplicates are 
 // merged together whilst loops are fully deleted in a two step process
 // to be safe as they will appear in the list twice...
 // (Faces are handled as we do this.)
  DirEdge * targ = replacement.vert->edge;
  bit notThisTime = true;
  while ((notThisTime)||(targ!=replacement.vert->edge))
  {
   notThisTime = false;
   DirEdge * nextTarg = targ->next;
   {
    // Check for a loop, if a loop remove it this end and set its point
    // to null, so we can detect it when we strike it again...
     bit penguin = true;
     if (targ->to==replacement.vert)
     {
      // Remove this side from the list...
       targ->next->prev = targ->prev;
       targ->prev->next = targ->next;
       if (targ==replacement.vert->edge)
       {
        replacement.vert->edge = targ->next;
        notThisTime = true;
       }
       
      // Mark it so when we see the other end it can be terminated...
       targ->partner->to = null<Vertex*>();
       
      penguin = false;
     }
    
    // Check for a loop which we have already visited once, if so this time we delete it,
    // update and when degenerate delete any using faces...
     if (targ->to==null<Vertex*>())
     {
      // Remove this second side from the list...
       targ->next->prev = targ->prev;
       targ->prev->next = targ->next;
       if (targ==replacement.vert->edge)
       {
        replacement.vert->edge = targ->next;
        if (targ==replacement.vert->edge)
        {
         replacement.vert->edge = null<DirEdge*>();
         nextTarg = null<DirEdge*>();
        }
        else notThisTime = true;
       }
       
      // Remove edge from all using faces, deleting the degenerate...
       while ((targ->user!=null<HalfEdge*>())||(targ->partner->user!=null<HalfEdge*>()))
       {
        HalfEdge * he = (targ->user!=null<HalfEdge*>())?targ->user:targ->partner->user;
        // Remove the current HalfEdge from the structure...      
         if (he==he->next)
         {
          if (targ->user) targ->user = null<HalfEdge*>();
                     else targ->partner->user = null<HalfEdge*>();
         }
         else
         {
          he->next->prev = he->prev;
          he->prev->next = he->next;
          if (targ->user) targ->user = he->next;
                     else targ->partner->user = he->next;
         }
       
        // Remove it from the face...
         Face * face = he->face;
         if (face->edge==he) face->edge = he->chain;
         log::Assert(face->edge!=he);
         face->size -= 1;
         {
          HalfEdge * t = he;
          while (t->chain!=he) t = t->chain;
          log::Assert(t!=he);
          t->chain = he->chain;
         }
         
        // If the face is now degenerate terminate it, ignoring the fact the
        // edges remaining must be in fact the same edge - this algorithm will
        // solve that elsewhere...
         if (face->size==2)
         {
          log::Assert(face->edge==face->edge->chain->chain);
          log::Assert(face->edge!=face->edge->chain);
          // Remove the two face edges...
           HalfEdge * t[2];
           t[0] = face->edge;
           t[1] = face->edge->chain;
           for (nat32 i=0;i<2;i++)
           {
            t[i]->next->prev = t[i]->prev;
            t[i]->prev->next = t[i]->next;
            if (t[i]->edge->user==t[i])
            {
             if (t[i]->next!=t[i]) t[i]->edge->user = t[i]->next;
                              else t[i]->edge->user = null<HalfEdge*>();
            }
           }
          
          // Terminate the face and its two half edges...
           mem::Free(face->edge->chain);
           mem::Free(face->edge);
           faces.Rem(face);
           mem::Free(face);
         }
       }

      
      // Terminate the edge...
      {
       DirEdge * first = math::Min(targ,targ->partner);
       edges.Rem(first);
       mem::Free(first);
      }
      
      penguin = false;
     }
    
    // Check for it being a duplicate, this is unfortunatly O(n^2),
    // but unavoidable really. If a duplicate merge it in and check for any faces that
    // use both sides - they will need correcting and deleting if degenerate...
     if (penguin)
     {
      DirEdge * other = targ->next;
      while (other!=replacement.vert->edge)
      {
       if (other->to==targ->to)
       {
        DirEdge * from[2];
        DirEdge * to[2];
        from[0] = targ;
        from[1] = targ->partner;
        to[0] = other;
        to[1] = other->partner;
       
        // Move everything in targ to other...
         for (nat32 i=0;i<2;i++)
         {
          if (from[i]->user)
          {
           // Correct pointers...
            HalfEdge * he = from[i]->user;
            do
            {
             he->edge = to[i];
             he = he->next;
            } while (he!=from[i]->user);
          
           // Do the transfer...
            if (to[i]->user==null<HalfEdge*>()) to[i]->user = from[i]->user;
            else
            {
             HalfEdge * aa = to[i]->user->prev;
             HalfEdge * ab = to[i]->user;

             HalfEdge * ba = from[i]->user->prev;
             HalfEdge * bb = from[i]->user;
                         
             aa->next = bb;
             bb->prev = aa;
             
             ba->next = ab;
             ab->prev = ba;
            }
          }
         }
    
        
        // Remove targ from the vertex lists, on both sides (Be careful as this
        // can screw with our loop)...
         targ->next->prev = targ->prev;
         targ->prev->next = targ->next;
         if (targ==replacement.vert->edge)
         {
          replacement.vert->edge = targ->next;
          if (targ==replacement.vert->edge)
          {
           replacement.vert->edge = null<DirEdge*>();
           nextTarg = null<DirEdge*>();
          }
          else notThisTime = true;
         }
         
         targ->partner->next->prev = targ->partner->prev;
         targ->partner->prev->next = targ->partner->next;
         if (targ->partner==targ->to->edge)
         {
          if (targ->partner->next!=targ->partner) targ->to->edge = targ->partner->next;
                                             else targ->to->edge = null<DirEdge*>();
         }


        // Terminate targ...
        {
         DirEdge * first = math::Min(targ,targ->partner);
         edges.Rem(first);
         mem::Free(first);
        }

               
        // Check all of others faces for becomming degenerate...
         for (nat32 i=0;i<2;i++)
         {
          HalfEdge * t = to[i]->user;
          if (t)
          {
           do
           {
            HalfEdge * nextT = t->next;
            if (t->chain->edge==t->edge->partner)
            {
             Face * face = t->face;
             // Its degenerate - remove the two edge structures...
              // Remove from pointer ring...
              {
               HalfEdge * spin = t->chain;
               while (spin->chain!=t) spin = spin->chain;
               log::Assert((spin!=t)&&(spin!=t->chain));
               spin->chain = t->chain->chain;
               face->edge = spin;
               
               face->size -= 2;
              }
           
              // Remove from edge linked lists...
               t->next->prev = t->prev;
               t->prev->next = t->next;
               if (t==t->edge->user)
               {
                if (t->next!=t) t->edge->user = t->next;
                else
                {
                 t->edge->user = null<HalfEdge*>();
                 nextT = null<HalfEdge*>();
                }
               }
             
               t->chain->next->prev = t->chain->prev;
               t->chain->prev->next = t->chain->next;
               if (t->chain==t->chain->edge->user)
               {
                if (t->chain->next!=t->chain) t->chain->edge->user = t->chain->next;
                                         else t->chain->edge->user = null<HalfEdge*>();
               }

              // Delete memory...
               mem::Free(t->chain);
               mem::Free(t);
           
             // Check what is left - if its less than 3 edges terminate...
              if (face->size<3)
              {
               // Remove remaining edges, they would be terminated anyway...
                HalfEdge * victim = face->edge->chain;
                face->edge->chain = null<HalfEdge*>();
                while (victim)
                {
                 HalfEdge * nextVictim = victim->chain;
                 {
                  // Remove from edge linked lists...
                   victim->next->prev = victim->prev;
                   victim->prev->next = victim->next;
                   if (victim==victim->edge->user)
                   {
                    if (victim->next!=victim) victim->edge->user = victim->prev;
                                          else victim->edge->user = null<HalfEdge*>();
                   }

                  // Delete memory...
                   mem::Free(victim);
                 }
                 victim = nextVictim;
                }
             
               // Terminate the face...
                faces.Rem(face);
                mem::Free(face);
              }
            }
            t = nextT;
           }
           while (t!=to[i]->user);
          }
         }
        break;
       }
       other = other->next;
      }
     }
   }
   targ = nextTarg;
  }
}

void Mesh::Prune()
{
 LogTime("eos::sur::Mesh::Prune");
 // First prune edges without faces...
 if (edges.Size()!=0)
 {
  ds::SortList<DirEdge*>::Cursor targ = edges.FrontPtr();
  while (!targ.Bad())
  {
   sur::Edge e(*targ);
   ++targ;

   if ((e.edge->user==null<HalfEdge*>())&&(e.edge->partner->user==null<HalfEdge*>())) Del(e);
  }
 }


 // Then prune vertices without edges...
 if (verts.Size()!=0)
 {
  ds::SortList<Vertex*>::Cursor targ = verts.FrontPtr();
  while (!targ.Bad())
  {
   sur::Vertex v(*targ);
   ++targ;

   if (v.vert->edge==null<DirEdge*>()) Del(v);
  }
 }
}

void Mesh::Triangulate()
{
 LogTime("eos::sur::Mesh::Triangulate");
 ds::Array<sur::Face> face;
 GetFaces(face);

 for (nat32 i=0;i<face.Size();i++)
 {
  if (face[i].Size()!=3)
  {
   // We are going to have to triangulate...
    // Get vertices...
     ds::Array<sur::Vertex> vert;
     face[i].GetVertices(vert);

    // Create new faces...
     for (nat32 j=1;j<vert.Size()-1;j++)
     {
      sur::Face n = NewFace(vert[0],vert[j],vert[j+1]);
      Transfer(n,face[i]); // Copy over all properties from the source face.
     }

    // Delete old face...
     Del(face[i]);
  }
 }
}

svt::Node * Mesh::AsSvt(svt::Core & core) const
{
 log::Assert(tt!=null<str::TokenTable*>());
 svt::Node * ret = new svt::Node(core);


 // Create the vertex table, including all extra properties...
  svt::Var * vt = new svt::Var(core);
  vt->AttachParent(ret);

  bs::Vert vertIni(0.0,0.0,0.0);
  vt->Setup1D(verts.Size());
  vt->Add("$vert",vertIni);
  for (nat32 i=0;i<vertProp.Size();i++)
  {
   vt->Add(core.GetTT()(tt->Str(vertProp[i].name)),core.GetTT()(tt->Str(vertProp[i].type)),
           vertProp[i].size,vertProp[i].ini);
  }
  vt->Commit(false);

  ds::Array<HopSkip> hspv(vertProp.Size());
  for (nat32 i=0;i<hspv.Size();i++)
  {
   hspv[i].size = vertProp[i].size;
   hspv[i].fromOffset = vertProp[i].offset;
   vt->GetIndex(core.GetTT()(tt->Str(vertProp[i].name)),hspv[i].toInd);
  }

  {
   svt::Field<bs::Vert> out(vt,"$vert");
   ds::SortList<Vertex*>::Cursor in = verts.FrontPtr();
   for (nat32 i=0;i<out.Size(0);i++)
   {
    out.Get(i) = (*in)->pos;

    for (nat32 j=0;j<hspv.Size();j++)
    {
     mem::Copy((byte*)vt->Ptr(hspv[j].toInd,i),(byte*)(*in) + hspv[j].fromOffset,hspv[j].size);
    }

    ++in;
   }
  }


 // Create the edge table...
  svt::Var * ve = new svt::Var(core);
  ve->AttachParent(ret);

  nat32 natIni = 0;
  ve->Setup1D(edges.Size());
  ve->Add("$a",natIni);
  ve->Add("$b",natIni);
  for (nat32 i=0;i<edgeProp.Size();i++)
  {
   ve->Add(core.GetTT()(tt->Str(edgeProp[i].name)),core.GetTT()(tt->Str(edgeProp[i].type)),
           edgeProp[i].size,edgeProp[i].ini);
  }
  ve->Commit(false);

  ds::Array<HopSkip> hspe(edgeProp.Size());
  for (nat32 i=0;i<hspe.Size();i++)
  {
   hspe[i].size = edgeProp[i].size;
   hspe[i].fromOffset = edgeProp[i].offset;
   ve->GetIndex(core.GetTT()(tt->Str(edgeProp[i].name)),hspe[i].toInd);
  }

  {
   svt::Field<nat32> oa(ve,"$a");
   svt::Field<nat32> ob(ve,"$b");
   ds::SortList<DirEdge*>::Cursor in = edges.FrontPtr();
   for (nat32 i=0;i<oa.Size(0);i++)
   {
    oa.Get(i) = verts.Index((*in)->to);
    ob.Get(i) = verts.Index((*in)->partner->to);

    for (nat32 j=0;j<hspe.Size();j++)
    {
     mem::Copy((byte*)ve->Ptr(hspe[j].toInd,i),(byte*)(*in) + hspe[j].fromOffset,hspe[j].size);
    }

    log::Assert((oa.Get(i)!=nat32(-1))&&(ob.Get(i)!=nat32(-1)),"Mesh::AsSvt");
    ++in;
   }
  }


 // Create the face index table incrimentally...
  svt::Var * vf = new svt::Var(core);
  vf->AttachParent(ret);

  vf->Setup1D(faces.Size());
  vf->Add("$offset",natIni);
  for (nat32 i=0;i<faceProp.Size();i++)
  {
   vf->Add(core.GetTT()(tt->Str(faceProp[i].name)),core.GetTT()(tt->Str(faceProp[i].type)),
           faceProp[i].size,faceProp[i].ini);
  }
  vf->Commit(false);

  ds::Array<HopSkip> hspf(faceProp.Size());
  for (nat32 i=0;i<hspf.Size();i++)
  {
   hspf[i].size = faceProp[i].size;
   hspf[i].fromOffset = faceProp[i].offset;
   vf->GetIndex(core.GetTT()(tt->Str(faceProp[i].name)),hspf[i].toInd);
  }

  nat32 vertOffset = 0;
  {
   svt::Field<nat32> out(vf,"$offset");
   ds::SortList<Face*>::Cursor in = faces.FrontPtr();
   for (nat32 i=0;i<out.Size(0);i++)
   {
    out.Get(i) = vertOffset;
    vertOffset += (*in)->size;

    for (nat32 j=0;j<hspf.Size();j++)
    {
     mem::Copy((byte*)vf->Ptr(hspf[j].toInd,i),(byte*)(*in) + hspf[j].fromOffset,hspf[j].size);
    }

    ++in;
   }
  }


 // Iterate the faces again to create the face vertex list as indexed by the
 // face table...
  svt::Var * vfl = new svt::Var(core);
  vfl->AttachParent(ret);

  vfl->Setup1D(vertOffset);
  vfl->Add("$index",natIni);
  vfl->Commit(false);

  vertOffset = 0;
  {
   svt::Field<nat32> out(vfl,"$index");
   ds::SortList<Face*>::Cursor in = faces.FrontPtr();
   ds::Array<Vertex> temp;
   while (!in.Bad())
   {
    HalfEdge * start = (*in)->edge;
    HalfEdge * targ = start;
    while (true)
    {
     out.Get(vertOffset) = verts.Index(targ->edge->to);
     ++vertOffset;

     targ = targ->chain;
     if (targ==start) break;
    }

    ++in;
   }
  }

 return ret;
}

bit Mesh::FromSvt(svt::Node * root)
{
 log::Assert(tt!=null<str::TokenTable*>());

 // Empty all current data...
 if (verts.Size()!=0)
 {
  ds::SortList<Vertex*>::Cursor targ = verts.FrontPtr();
  while (!targ.Bad())
  {
   sur::Vertex v(*targ);
   ++targ;
   Del(v);
  }
 }


 // Wipe out all current properties...
  vertPropChanged = false;
  edgePropChanged = false;
  facePropChanged = false;

  vertPropSize = sizeof(Vertex);
  edgePropSize = sizeof(DirEdge) * 2;
  facePropSize = sizeof(Face);

  for (nat32 i=0;i<vertProp.Size();i++) mem::Free(vertProp[i].ini);
  for (nat32 i=0;i<edgeProp.Size();i++) mem::Free(edgeProp[i].ini);
  for (nat32 i=0;i<faceProp.Size();i++) mem::Free(faceProp[i].ini);

  vertProp.Size(0);
  edgeProp.Size(0);
  faceProp.Size(0);

  vertByName.Reset();
  edgeByName.Reset();
  faceByName.Reset();


 // Extract and check the various sub-nodes of the root...
  svt::Node * cn[4];
  cn[0] = root->Child();
  if (cn[0]==null<svt::Node*>()) {LogDebug("[sur::Mesh::FromSvt] Insufficient children."); return false;}
  cn[1] = cn[0]->Next();
  if (cn[1]==null<svt::Node*>()) {LogDebug("[sur::Mesh::FromSvt] Insufficient children."); return false;}
  cn[2] = cn[1]->Next();
  if (cn[2]==null<svt::Node*>()) {LogDebug("[sur::Mesh::FromSvt] Insufficient children."); return false;}
  cn[3] = cn[2]->Next();
  if (cn[3]==null<svt::Node*>()) {LogDebug("[sur::Mesh::FromSvt] Insufficient children."); return false;}

  if (str::Compare(cn[0]->TypeString(),"eos::svt::Var")!=0)
  {LogDebug("[sur::Mesh::FromSvt] Wrong child type."); return false;}
  if (str::Compare(cn[1]->TypeString(),"eos::svt::Var")!=0)
  {LogDebug("[sur::Mesh::FromSvt] Wrong child type."); return false;}
  if (str::Compare(cn[2]->TypeString(),"eos::svt::Var")!=0)
  {LogDebug("[sur::Mesh::FromSvt] Wrong child type."); return false;}
  if (str::Compare(cn[3]->TypeString(),"eos::svt::Var")!=0)
  {LogDebug("[sur::Mesh::FromSvt] Wrong child type."); return false;}

  svt::Var * vt =  static_cast<svt::Var*>(cn[0]);
  svt::Var * et = static_cast<svt::Var*>(cn[1]);
  svt::Var * ft = static_cast<svt::Var*>(cn[2]);
  svt::Var * flt = static_cast<svt::Var*>(cn[3]);

  if ((vt->Dims()!=1)||(et->Dims()!=1)||(ft->Dims()!=1)||(flt->Dims()!=1))
  {LogDebug("[sur::Mesh::FromSvt] Too many dimensions."); return false;}

  if (!vt->Exists("$vert","eos::bs::Vert")) {LogDebug("[sur::Mesh::FromSvt] Missing $vert field."); return false;}
  if (!et->Exists("$a","eos::nat32")) {LogDebug("[sur::Mesh::FromSvt] Missing $a field."); return false;}
  if (!et->Exists("$b","eos::nat32")) {LogDebug("[sur::Mesh::FromSvt] Missing $b field."); return false;}
  if (!ft->Exists("$offset","eos::nat32")) {LogDebug("[sur::Mesh::FromSvt] Missing $offset field."); return false;}
  if (!flt->Exists("$index","eos::nat32")) {LogDebug("[sur::Mesh::FromSvt] Missing $index field."); return false;}

  svt::Field<bs::Vert> vf(vt,"$vert");
  svt::Field<nat32> eaf(et,"$a");
  svt::Field<nat32> ebf(et,"$b");
  svt::Field<nat32> ff(ft,"$offset");
  svt::Field<nat32> flf(flt,"$index");


 // Load in all the properties, and call commit to rig the data structure correctly...
  str::TokenTable & svtTT = root->GetCore().GetTT();
  // Verts...
   for (nat32 i=0;i<vt->Fields();i++)
   {
    str::Token name = (*tt)(svtTT.Str(vt->FieldName(i)));
    if (name!=(*tt)("$vert"))
    {
     str::Token type = (*tt)(svtTT.Str(vt->FieldType(i)));
     AddVertProp(name,type,vt->FieldSize(i),vt->FieldDef(i));
    }
   }

  // Edges...
   for (nat32 i=0;i<et->Fields();i++)
   {
    str::Token name = (*tt)(svtTT.Str(et->FieldName(i)));
    if ((name!=(*tt)("$a"))&&(name!=(*tt)("$b")))
    {
     str::Token type = (*tt)(svtTT.Str(et->FieldType(i)));
     AddEdgeProp(name,type,et->FieldSize(i),et->FieldDef(i));
    }
   }

  // Faces...
   for (nat32 i=0;i<ft->Fields();i++)
   {
    str::Token name = (*tt)(svtTT.Str(ft->FieldName(i)));
    if (name!=(*tt)("$offset"))
    {
     str::Token type = (*tt)(svtTT.Str(ft->FieldType(i)));
     AddFaceProp(name,type,ft->FieldSize(i),ft->FieldDef(i));
    }
   }

  // Commit...
   Commit(false);


 // Load in the vertices, keeping an offset to pointer dictionary...
 ds::Array<sur::Vertex> dict(vf.Size(0));
 {
  ds::Array<HopSkip> hsrv(vertProp.Size());
  for (nat32 i=0;i<hsrv.Size();i++)
  {
   hsrv[i].size = vertProp[i].size;
   hsrv[i].fromOffset = vertProp[i].offset;
   vt->GetIndex(svtTT(tt->Str(vertProp[i].name)),hsrv[i].toInd);
  }

  for (nat32 i=0;i<vf.Size(0);i++)
  {
   dict[i] = NewVertex(vf.Get(i));
   for (nat32 j=0;j<hsrv.Size();j++)
   {
    mem::Copy((byte*)(void*)dict[i].vert + hsrv[j].fromOffset,(byte*)vt->Ptr(hsrv[j].toInd,i),hsrv[j].size);
   }
  }
 }


 // Load in edges...
 {
  ds::Array<HopSkip> hsre(edgeProp.Size());
  for (nat32 i=0;i<hsre.Size();i++)
  {
   hsre[i].size = edgeProp[i].size;
   hsre[i].fromOffset = edgeProp[i].offset;
   et->GetIndex(svtTT(tt->Str(edgeProp[i].name)),hsre[i].toInd);
  }

  for (nat32 i=0;i<eaf.Size(0);i++)
  {
   sur::Edge edge = NewEdge(dict[eaf.Get(i)],dict[ebf.Get(i)]);
   for (nat32 j=0;j<hsre.Size();j++)
   {
    mem::Copy((byte*)(void*)edge.edge + hsre[j].fromOffset,(byte*)et->Ptr(hsre[j].toInd,i),hsre[j].size);
   }
  }
 }


 // Load in faces...
 {
  ds::Array<HopSkip> hsrf(faceProp.Size());
  for (nat32 i=0;i<hsrf.Size();i++)
  {
   hsrf[i].size = faceProp[i].size;
   hsrf[i].fromOffset = faceProp[i].offset;
   ft->GetIndex(svtTT(tt->Str(faceProp[i].name)),hsrf[i].toInd);
  }

  ds::Array<sur::Vertex> temp;
  for (nat32 i=0;i<ff.Size(0);i++)
  {
   nat32 start = ff.Get(i);
   nat32 end = (i+1!=ff.Size(0))?ff.Get(i+1):flf.Size(0);
   temp.Size(end-start);
   for (nat32 j=0;j<temp.Size();j++) temp[j] = dict[flf.Get(start+j)];

   sur::Face face = NewFace(temp);
   for (nat32 j=0;j<hsrf.Size();j++)
   {
    mem::Copy((byte*)(void*)face.face + hsrf[j].fromOffset,(byte*)ft->Ptr(hsrf[j].toInd,i),hsrf[j].size);
   }
  }
 }


 return true;
}

void Mesh::Store(file::Wavefront & out) const
{
 if (verts.Size()==0) return;

 // Store the vertices, creating a table going from our lists offsets to
 // the wavefront files offsets...
  ds::Array<nat32> dict(verts.Size());
  {
   ds::SortList<Vertex*>::Cursor targ = verts.FrontPtr();
   for (nat32 i=0;i<dict.Size();i++)
   {
    dict[i] = out.Add((*targ)->pos);
    ++targ;
   }
  }
  
  
 // If we have UV's store them as well...
  ds::Array<nat32> dictUV;
  if (tt&&ExistsVertProp("u")&&ExistsVertProp("v"))
  {
   data::Property<sur::Vertex,real32> propU = GetVertProp<real32>("u");
   data::Property<sur::Vertex,real32> propV = GetVertProp<real32>("v");
  
   dictUV.Size(verts.Size());
   ds::SortList<Vertex*>::Cursor targ = verts.FrontPtr();
   for (nat32 i=0;i<dictUV.Size();i++)
   {
    bs::Tex2D uv;
    uv[0] = propU.Get(sur::Vertex(*targ));
    uv[1] = propV.Get(sur::Vertex(*targ));
   
    dictUV[i] = out.Add(uv);
    ++targ;
   }
  }


 // Now store the faces, simple iteration of vertices...
  if (faces.Size()==0) return;
  ds::SortList<Face*>::Cursor in = faces.FrontPtr();
  while (!in.Bad())
  {
   HalfEdge * start = (*in)->edge;
   HalfEdge * targ = start;
   while (true)
   {
    nat32 uv = 0;
    if (dictUV.Size()) uv = dictUV[verts.Index(targ->edge->to)];
    out.Add(dict[verts.Index(targ->edge->to)],0,uv);
    targ = targ->chain;
    if (targ==start) break;
   }
   out.Face();

   ++in;
  }
}

void Mesh::Store(file::Ply & out) const
{
 if (verts.Size()==0) return;

 // Store the vertices, creating a table going from our lists offsets to
 // the wavefront files offsets...
  ds::Array<nat32> dict(verts.Size());
  {
   data::Property<sur::Vertex,real32> propU;
   data::Property<sur::Vertex,real32> propV;
   if (tt&&ExistsVertProp("u")&&ExistsVertProp("v"))
   {
    propU = GetVertProp<real32>("u");
    propV = GetVertProp<real32>("v");
   }
  
   ds::SortList<Vertex*>::Cursor targ = verts.FrontPtr();
   for (nat32 i=0;i<dict.Size();i++)
   {
    bs::Tex2D uv(0.0,0.0);
    if (propU.Valid()&&propV.Valid())
    {
     uv[0] = propU.Get(sur::Vertex(*targ));
     uv[1] = propV.Get(sur::Vertex(*targ));
    }
   
    dict[i] = out.Add((*targ)->pos,uv);
    ++targ;
   }
  }


 // Now store the faces, simple iteration of vertices...
  if (faces.Size()==0) return;
  ds::SortList<Face*>::Cursor in = faces.FrontPtr();
  while (!in.Bad())
  {
   HalfEdge * start = (*in)->edge;
   HalfEdge * targ = start;
   while (true)
   {
    out.Add(dict[verts.Index(targ->edge->to)]);
    targ = targ->chain;
    if (targ==start) break;
   }
   out.Face();

   ++in;
  }
}

//------------------------------------------------------------------------------
void Mesh::AddVertProp(str::Token name,str::Token type,nat32 size,const void * ini)
{
 log::Assert(tt!=null<str::TokenTable*>());

 vertPropChanged = true;
 nat32 ind = vertProp.Size();
 vertProp.Size(ind+1);

 vertProp[ind].name = name;
 vertProp[ind].type = type;
 vertProp[ind].size = size;
 vertProp[ind].ini = mem::Malloc<byte>(size);
 mem::Copy(vertProp[ind].ini,(byte*)ini,size);

 vertProp[ind].offset = 0;
 vertProp[ind].state = Prop::Added;
}

void Mesh::AddEdgeProp(str::Token name,str::Token type,nat32 size,const void * ini)
{
 log::Assert(tt!=null<str::TokenTable*>());

 edgePropChanged = true;
 nat32 ind = edgeProp.Size();
 edgeProp.Size(ind+1);

 edgeProp[ind].name = name;
 edgeProp[ind].type = type;
 edgeProp[ind].size = size;
 edgeProp[ind].ini = mem::Malloc<byte>(size);
 mem::Copy(edgeProp[ind].ini,(byte*)ini,size);

 edgeProp[ind].offset = 0;
 edgeProp[ind].state = Prop::Added;
}

void Mesh::AddFaceProp(str::Token name,str::Token type,nat32 size,const void * ini)
{
 log::Assert(tt!=null<str::TokenTable*>());

 facePropChanged = true;
 nat32 ind = faceProp.Size();
 faceProp.Size(ind+1);

 faceProp[ind].name = name;
 faceProp[ind].type = type;
 faceProp[ind].size = size;
 faceProp[ind].ini = mem::Malloc<byte>(size);
 mem::Copy(faceProp[ind].ini,(byte*)ini,size);

 faceProp[ind].offset = 0;
 faceProp[ind].state = Prop::Added;
}

void Mesh::RemVertProp(str::Token name)
{
 for (nat32 i=0;i<vertProp.Size();i++) // As the user might be deleting a recently added we can't use the index lookup.
 {
  if (vertProp[i].name==name)
  {
   vertPropChanged = true;
   vertProp[i].state = Prop::Deleted;
  }
 }
}

void Mesh::RemEdgeProp(str::Token name)
{
 for (nat32 i=0;i<edgeProp.Size();i++) // As the user might be deleting a recently added we can't use the index lookup.
 {
  if (edgeProp[i].name==name)
  {
   edgePropChanged = true;
   edgeProp[i].state = Prop::Deleted;
  }
 }
}

void Mesh::RemFaceProp(str::Token name)
{
 for (nat32 i=0;i<faceProp.Size();i++) // As the user might be deleting a recently added we can't use the index lookup.
 {
  if (faceProp[i].name==name)
  {
   facePropChanged = true;
   faceProp[i].state = Prop::Deleted;
  }
 }
}

void Mesh::Commit(bit useDefault)
{
 if (vertPropChanged)
 {
  CommitVertex(useDefault);
  vertPropChanged = false;
 }

 if (edgePropChanged)
 {
  CommitEdge(useDefault);
  edgePropChanged = false;
 }

 if (facePropChanged)
 {
  CommitFace(useDefault);
  facePropChanged = false;
 }
}

bit Mesh::ExistsVertProp(str::Token name) const
{
 return vertByName.Get(name)!=null<nat32*>();
}

bit Mesh::ExistsEdgeProp(str::Token name) const
{
 return edgeByName.Get(name)!=null<nat32*>();
}

bit Mesh::ExistsFaceProp(str::Token name) const
{
 return faceByName.Get(name)!=null<nat32*>();
}

nat32 Mesh::IndexVertProp(str::Token name) const
{
 nat32 * ind = vertByName.Get(name);
 if (ind) return *ind;
     else return nat32(-1);
}

nat32 Mesh::IndexEdgeProp(str::Token name) const
{
 nat32 * ind = edgeByName.Get(name);
 if (ind) return *ind;
     else return nat32(-1);
}

nat32 Mesh::IndexFaceProp(str::Token name) const
{
 nat32 * ind = faceByName.Get(name);
 if (ind) return *ind;
     else return nat32(-1);
}

str::Token Mesh::NameVertProp(nat32 i) const
{
 return vertProp[i].name;
}

str::Token Mesh::TypeVertProp(nat32 i) const
{
 return vertProp[i].type;
}

nat32 Mesh::SizeVertProp(nat32 i) const
{
 return vertProp[i].size;
}

const void * Mesh::DefaultVertProp(nat32 i) const
{
 return vertProp[i].ini;
}

str::Token Mesh::NameEdgeProp(nat32 i) const
{
 return edgeProp[i].name;
}

str::Token Mesh::TypeEdgeProp(nat32 i) const
{
 return edgeProp[i].type;
}

nat32 Mesh::SizeEdgeProp(nat32 i) const
{
 return edgeProp[i].size;
}

const void * Mesh::DefaultEdgeProp(nat32 i) const
{
 return edgeProp[i].ini;
}

str::Token Mesh::NameFaceProp(nat32 i) const
{
 return faceProp[i].name;
}

str::Token Mesh::TypeFaceProp(nat32 i) const
{
 return faceProp[i].type;
}

nat32 Mesh::SizeFaceProp(nat32 i) const
{
 return faceProp[i].size;
}

const void * Mesh::DefaultFaceProp(nat32 i) const
{
 return faceProp[i].ini;
}

void Mesh::Transfer(sur::Vertex & to,const sur::Vertex & from)
{
 mem::Copy(((byte*)(to.vert))+sizeof(Mesh::Vertex),
           ((byte*)(from.vert))+sizeof(Mesh::Vertex),
           vertPropSize-sizeof(Mesh::Vertex));
}

void Mesh::Transfer(sur::Edge & to,const sur::Edge & from)
{
 mem::Copy(((byte*)(to.edge))+sizeof(Mesh::DirEdge)*2,
           ((byte*)(from.edge))+sizeof(Mesh::DirEdge)*2,
           edgePropSize-sizeof(Mesh::DirEdge)*2);
}

void Mesh::Transfer(sur::Face & to,const sur::Face & from)
{
 mem::Copy(((byte*)(to.face))+sizeof(Mesh::Face),
           ((byte*)(from.face))+sizeof(Mesh::Face),
           facePropSize-sizeof(Mesh::Face)); 
}

bit Mesh::SafeContraction(const Edge & edge,const bs::Vert & newPos)
{
 // Iterate all faces that use the vertices at either end of the edge...
  Vertex * vert = edge.edge->to;
  DirEdge * targ = vert->edge;
  while (true)
  {
   // There should only be one face attached to each edge on each side, if not 
   // we have a non-manifold situation and its all fucked anyway...
    if ((targ->user)&&(targ->user->next!=targ->user)) return false;
    if ((targ->partner->user)&&(targ->partner->user->next!=targ->partner->user)) return false;
   
   // Get the face we are considering...
    HalfEdge * faceIn = targ->partner->user; // Into the vertex, chain should get out of the vertex.
    if (faceIn)
    {
     if ((faceIn->edge==edge.edge)||(faceIn->edge==edge.edge->partner))
     {
      // Face where in edge uses the collapsing edge...
      
      // Do nothing - the face will have its out edge using the collapsing edge
      // on the other vertex, and will have already been/about to be analysed
      // - no point doing it twice. (And its easier to analyse in the other case.)
     }
     else
     {
      if ((faceIn->chain->edge==edge.edge)||(faceIn->chain->edge==edge.edge->partner))
      {
       // Face where the out edge uses the collapsing edge...
        if (faceIn->face->size>3)
        {
         // Check that the collapsed version and un-collapsed version match - 
         // have to check with two triangles due to the 4 vertices involved directly...
          if (!SameDir(faceIn->edge->partner->to->pos,faceIn->edge->to->pos,faceIn->chain->edge->to->pos,
                       faceIn->edge->partner->to->pos,newPos               ,faceIn->chain->chain->edge->to->pos)) return false;
          
          if (!SameDir(faceIn->edge->to->pos,faceIn->chain->edge->to->pos,faceIn->chain->chain->edge->to->pos,
                       faceIn->edge->partner->to->pos,newPos             ,faceIn->chain->chain->edge->to->pos)) return false;

         
         // If there are more the 4 vertices we need to check the sides for convex to concave transitions...
          if (faceIn->face->size>4)
          {
           // ***************************************************
          }
        }
      }
      else
      {
       // Face that doesn't use the collapsing edge...
        // Check the basic triangle for changing direction...
         if (!SameDir(faceIn->edge->partner->to->pos,faceIn->edge->to->pos,faceIn->chain->edge->to->pos,
                      faceIn->edge->partner->to->pos,newPos               ,faceIn->chain->edge->to->pos)) return false;
       
        // If the face has more than 3 sides we also need to check the adjacent
        // triangles for changing direction (i.e. convex to concave.)...
        if (faceIn->face->size>3)
        {
         HalfEdge * facePrev = faceIn->chain->chain->chain; // It has to have at least 4 edges.
         while (facePrev->chain!=faceIn) facePrev = facePrev->chain;
         
         if (!SameDir(facePrev->edge->partner->to->pos,facePrev->edge->to->pos,faceIn->edge->to->pos,
                      facePrev->edge->partner->to->pos,facePrev->edge->to->pos,newPos               )) return false;
         
         if (!SameDir(faceIn->edge->to->pos,faceIn->chain->edge->to->pos,faceIn->chain->chain->edge->to->pos,
                      newPos               ,faceIn->chain->edge->to->pos,faceIn->chain->chain->edge->to->pos)) return false;
        }
      }
     }
    }
  
   // Move to the next edge...
    targ = targ->next;
    if (targ==vert->edge)
    {
     if (vert==edge.edge->to)
     {
      vert = edge.edge->partner->to;
      targ = vert->edge;
     }
     else break;
    }
  }

 return true;
}

bit Mesh::Invariant(const sur::Vertex & test) const
{
 if (test.vert->edge)
 {
  DirEdge * targ = test.vert->edge->next;
  for (nat32 i=0;i<edges.Size();i++)
  {
   if (targ->to==test.vert) return false;
   if (targ->partner->to!=test.vert) return false;
  
   if (test.vert->edge==targ) return true;
   targ = targ->next;
  }
  return false;
 }
 return true;
}

void Mesh::CommitVertex(bit useDefault)
{
 // Create a data structure of operations that need to be applied to every
 // vertex, byte code style, simultaneously edit the prop array...
  ds::List<Op> opList;
  nat32 ci = 0;
  nat32 memOffset = sizeof(Vertex);
  for (nat32 i=0;i<vertProp.Size();i++)
  {
   switch (vertProp[i].state)
   {
    case Prop::Stored:
    {
     // Create the operation...
      Op op;
       op.op = false;
       op.offset = memOffset;
       op.size = vertProp[i].size;
       op.oldOffset = vertProp[i].offset;
      opList.AddBack(op);

     // Update the entry...
      vertProp[i].offset = memOffset;
      memOffset += vertProp[i].size;

     // Re-arrange the array...
      if (ci!=i) vertProp[ci] = vertProp[i];
      ++ci;
    }
    break;
    case Prop::Added:
    {
     // Create the operation...
      if (!useDefault)
      {
       Op op;
        op.op = true;
        op.offset = memOffset;
        op.size = vertProp[i].size;
        op.ini = vertProp[i].ini;
       opList.AddBack(op);
      }

     // Update the entry...
      vertProp[i].offset = memOffset;
      memOffset += vertProp[i].size;
      vertProp[i].state = Prop::Stored;

     // Re-arrange the array...
      if (ci!=i) vertProp[ci] = vertProp[i];
      ++ci;
    }
    break;
    case Prop::Deleted:
    {
     // Do nothing and it will not be copied over (Well, free its ini memory.)...
      mem::Free(vertProp[i].ini);
    }
    break;
   }
  }
  vertProp.Size(ci);
  vertPropSize = memOffset;


 // Convert the list of operations into an array of operations - speed of access matters...
  ds::Array<Op> prog(opList.Size());
  {
   ds::List<Op>::Cursor targ = opList.FrontPtr();
   for (nat32 i=0;i<prog.Size();i++)
   {
    prog[i] = *targ;
    ++targ;
   }
  }


 // Go through the entire list and apply the operation sequence to all...
 // (Rebuild the list afterwards, to make iteration possible.)
  if (verts.Size()!=0)
  {
   ds::SortList<Vertex*> newVerts;
   ds::SortList<Vertex*>::Cursor targ = verts.FrontPtr();
   while (!targ.Bad())
   {
    // Create the new data, re-arrange the pointers...
     Vertex * to = (Vertex*)(void*)mem::Malloc<byte>(vertPropSize);
     newVerts.Add(to);
     RepointVertex(*targ,to);

    // Run the mini-program...
     for (nat32 i=0;i<prog.Size();i++)
     {
      if (prog[i].op) mem::Copy(((byte*)(void*)to) + prog[i].offset,prog[i].ini,prog[i].size);
      else mem::Copy(((byte*)(void*)to) + prog[i].offset,(byte*)(void*)(*targ) + prog[i].oldOffset,prog[i].size);
     }

    // Terminate, move to next...
     mem::Free(*targ);
     ++targ;
   }
   verts.Take(newVerts);
  }


 // Re-index the names...
  vertByName.Reset();
  for (nat32 i=0;i<vertProp.Size();i++)
  {
   vertByName.Set(vertProp[i].name,i);
  }
}

void Mesh::CommitEdge(bit useDefault)
{
 // Create a data structure of operations that need to be applied to every
 // edge, byte code style, simultaneously edit the prop array...
  ds::List<Op> opList;
  nat32 ci = 0;
  nat32 memOffset = sizeof(DirEdge)*2;
  for (nat32 i=0;i<edgeProp.Size();i++)
  {
   switch (edgeProp[i].state)
   {
    case Prop::Stored:
    {
     // Create the operation...
      Op op;
       op.op = false;
       op.offset = memOffset;
       op.size = edgeProp[i].size;
       op.oldOffset = edgeProp[i].offset;
      opList.AddBack(op);

     // Update the entry...
      edgeProp[i].offset = memOffset;
      memOffset += edgeProp[i].size;

     // Re-arrange the array...
      if (ci!=i) edgeProp[ci] = edgeProp[i];
      ++ci;
    }
    break;
    case Prop::Added:
    {
     // Create the operation...
      if (!useDefault)
      {
       Op op;
        op.op = true;
        op.offset = memOffset;
        op.size = edgeProp[i].size;
        op.ini = edgeProp[i].ini;
       opList.AddBack(op);
      }

     // Update the entry...
      edgeProp[i].offset = memOffset;
      memOffset += edgeProp[i].size;
      edgeProp[i].state = Prop::Stored;

     // Re-arrange the array...
      if (ci!=i) edgeProp[ci] = edgeProp[i];
      ++ci;
    }
    break;
    case Prop::Deleted:
    {
     // Do nothing and it will not be copied over (Well, free its ini memory.)...
      mem::Free(edgeProp[i].ini);
    }
    break;
   }
  }
  edgeProp.Size(ci);
  edgePropSize = memOffset;


 // Convert the list of operations into an array of operations - speed of access matters...
  ds::Array<Op> prog(opList.Size());
  {
   ds::List<Op>::Cursor targ = opList.FrontPtr();
   for (nat32 i=0;i<prog.Size();i++)
   {
    prog[i] = *targ;
    ++targ;
   }
  }


 // Go through the entire list and apply the operation sequence to all...
 // (Rebuild the list afterwards, to make iteration possible.)
  if (edges.Size()!=0)
  {
   ds::SortList<DirEdge*> newEdges;
   ds::SortList<DirEdge*>::Cursor targ = edges.FrontPtr();
   while (!targ.Bad())
   {
    // Create the new data, re-arrange the pointers...
     DirEdge * to = (DirEdge*)(void*)mem::Malloc<byte>(edgePropSize);
     newEdges.Add(to);
     RepointEdge(*targ,to);

    // Run the mini-program...
     for (nat32 i=0;i<prog.Size();i++)
     {
      if (prog[i].op) mem::Copy(((byte*)(void*)to) + prog[i].offset,prog[i].ini,prog[i].size);
      else mem::Copy(((byte*)(void*)to) + prog[i].offset,(byte*)(void*)(*targ) + prog[i].oldOffset,prog[i].size);
     }

    // Terminate, move to next...
     mem::Free(*targ);
     ++targ;
   }
   edges.Take(newEdges);
  }


 // Re-index the names...
  edgeByName.Reset();
  for (nat32 i=0;i<edgeProp.Size();i++)
  {
   edgeByName.Set(edgeProp[i].name,i);
  }
}

void Mesh::CommitFace(bit useDefault)
{
 // Create a data structure of operations that need to be applied to every
 // face, byte code style, simultaneously edit the prop array...
  ds::List<Op> opList;
  nat32 ci = 0;
  nat32 memOffset = sizeof(Face);
  for (nat32 i=0;i<faceProp.Size();i++)
  {
   switch (faceProp[i].state)
   {
    case Prop::Stored:
    {
     // Create the operation...
      Op op;
       op.op = false;
       op.offset = memOffset;
       op.size = faceProp[i].size;
       op.oldOffset = faceProp[i].offset;
      opList.AddBack(op);

     // Update the entry...
      faceProp[i].offset = memOffset;
      memOffset += faceProp[i].size;

     // Re-arrange the array...
      if (ci!=i) faceProp[ci] = faceProp[i];
      ++ci;
    }
    break;
    case Prop::Added:
    {
     // Create the operation...
      if (!useDefault)
      {
       Op op;
        op.op = true;
        op.offset = memOffset;
        op.size = faceProp[i].size;
        op.ini = faceProp[i].ini;
       opList.AddBack(op);
      }

     // Update the entry...
      faceProp[i].offset = memOffset;
      memOffset += faceProp[i].size;
      faceProp[i].state = Prop::Stored;

     // Re-arrange the array...
      if (ci!=i) faceProp[ci] = faceProp[i];
      ++ci;
    }
    break;
    case Prop::Deleted:
    {
     // Do nothing and it will not be copied over (Well, free its ini memory.)...
      mem::Free(faceProp[i].ini);
    }
    break;
   }
  }
  faceProp.Size(ci);
  facePropSize = memOffset;


 // Convert the list of operations into an array of operations - speed of access matters...
  ds::Array<Op> prog(opList.Size());
  {
   ds::List<Op>::Cursor targ = opList.FrontPtr();
   for (nat32 i=0;i<prog.Size();i++)
   {
    prog[i] = *targ;
    ++targ;
   }
  }


 // Go through the entire list and apply the operation sequence to all...
 // (Rebuild the list afterwards, to make iteration possible.)
  if (faces.Size()!=0)
  {
   ds::SortList<Face*> newFaces;
   ds::SortList<Face*>::Cursor targ = faces.FrontPtr();
   while (!targ.Bad())
   {
    // Create the new data, re-arrange the pointers...
     Face * to = (Face*)(void*)mem::Malloc<byte>(facePropSize);
     newFaces.Add(to);
     RepointFace(*targ,to);

    // Run the mini-program...
     for (nat32 i=0;i<prog.Size();i++)
     {
      if (prog[i].op) mem::Copy(((byte*)(void*)to) + prog[i].offset,prog[i].ini,prog[i].size);
      else mem::Copy(((byte*)(void*)to) + prog[i].offset,(byte*)(void*)(*targ) + prog[i].oldOffset,prog[i].size);
     }

    // Terminate, move to next...
     mem::Free(*targ);
     ++targ;
   }
   faces.Take(newFaces);
  }


 // Re-index the names...
  faceByName.Reset();
  for (nat32 i=0;i<faceProp.Size();i++)
  {
   faceByName.Set(faceProp[i].name,i);
  }
}

void Mesh::RepointVertex(Vertex * from,Vertex * to)
{
 to->edge = from->edge;
 to->pos = from->pos;

 DirEdge * targ = to->edge;
 if (targ)
 {
  do
  {
   targ->partner->to = to;
   targ = targ->next;
  } while (targ!=to->edge);
 }
}

void Mesh::RepointEdge(DirEdge * from,DirEdge * to)
{
 DirEdge * from2 = from + 1;
 DirEdge * to2 = to + 1;

 to->to = from->to;
 to->partner = to2;
 to->next = from->next;
 to->prev = from->prev;
 to->user = from->user;

 to2->to = from2->to;
 to2->partner = to;
 to2->next = from2->next;
 to2->prev = from2->prev;
 to2->user = from2->user;


 // Repoint vertices...
  to->to->edge = to2;
  to2->to->edge = to;


 // Repoint other DirEdges...
  to->next->prev = to;
  to->prev->next = to;

  to2->next->prev = to2;
  to2->prev->next = to2;


 // Repoint HalfEdges...
  if (to->user)
  {
   HalfEdge * targ = to->user;
   do
   {
    targ->edge = to;
    targ = targ->next;
   } while (targ!=to->user);
  }

  if (to2->user)
  {
   HalfEdge * targ = to2->user;
   do
   {
    targ->edge = to2;
    targ = targ->next;
   } while (targ!=to2->user);
  }
}

void Mesh::RepointFace(Face * from,Face * to)
{
 to->edge = from->edge;
 to->size = from->size;

 HalfEdge * targ = to->edge;
 do
 {
  targ->face = to;
  targ = targ->chain;
 } while(targ!=to->edge);
}

void Mesh::CheckDegenFace(Face * face)
{
 if (face->size<3)
 {
  // Handle the fact we could have an edge degeneracy...
   if (face->size==2)
   {
    DirEdge * e1 = face->edge->edge;
    DirEdge * e2 = face->edge->chain->edge;
    if (e1->to!=e2->to) e2 = e2->partner;
    log::Assert((e1->to==e2->to)&&(e1->partner->to==e2->partner->to));

    
    // Transfer everything from e2 to e1...     
     if (e2->user)
     {
      HalfEdge * targ = e2->user;
      do
      {
       targ->edge = e1;
       targ = targ->next;
      }
      while (targ!=e2->user);
    
      if (e1->user)
      {
       e2->user->next = e1->user;
       e2->user->prev = e1->user->prev;
     
       e2->user->next->prev = e2->user;
       e2->user->prev->next = e2->user;
      }
      else
      {
       e1->user = e2->user;
      }
     }
     
     if (e2->partner->user)
     {
      HalfEdge * targ = e2->partner->user;
      do
      {
       targ->edge = e1->partner;
       targ = targ->next;
      }
      while (targ!=e2->partner->user);
    
      if (e1->partner->user)
      {
       e2->partner->user->next = e1->partner->user;
       e2->partner->user->prev = e1->partner->user->prev;
     
       e2->partner->user->next->prev = e2->partner->user;
       e2->partner->user->prev->next = e2->partner->user;     
      }
      else
      {
       e1->partner->user = e2->partner->user;
      }
     }

    // Remove e2 from its endpoints...
     e2->next->prev = e2->prev;
     e2->prev->next = e2->next;
    
     e2->partner->next->prev = e2->partner->prev;
     e2->partner->prev->next = e2->partner->next;
     
     if (e2->to->edge==e2->partner) e2->to->edge = e2->partner->next; // Must work because of the existance of e1.
     if (e2->partner->to->edge==e2) e2->partner->to->edge = e2->next; // "
     
    
    // Terminate e2...
     DirEdge * first = math::Min(e2,e2->partner);
     edges.Rem(first);
     mem::Free(first);
   }
   
  // Remove the half edges...
   HalfEdge * targ = face->edge;
   for (nat32 i=0;i<face->size;i++)
   {
    HalfEdge * victim = targ;
    targ = targ->chain;

    if (victim->next==victim) victim->edge->user = null<HalfEdge*>();
    else
    {
     victim->next->prev = victim->prev;
     victim->prev->next = victim->next;
     victim->edge->user = victim->next;
    }

    delete victim;
   }


  // Terminate...
   faces.Rem(face);
   mem::Free(face); 
 }
}

//------------------------------------------------------------------------------
void Vertex::GetVertices(ds::List<Vertex> & out) const
{
 if (vert->edge==null<Mesh::DirEdge*>()) return;

 Mesh::DirEdge * targ = vert->edge;
 do
 {
  out.AddBack(Vertex(targ->to));
  targ = targ->next;
 } while (targ!=vert->edge);
}

void Vertex::GetVertices(ds::Array<Vertex> & out) const
{
 // Fill up a list...
  ds::List<Vertex> list;
  GetVertices(list);

 // Stick the list into the array...
  out.Size(list.Size());
  ds::List<Vertex>::Cursor targ = list.FrontPtr();
  for (nat32 i=0;i<out.Size();i++)
  {
   out[i] = *targ;
   ++targ;
  }
}

void Vertex::GetEdges(ds::List<Edge> & out) const
{
 if (vert->edge==null<Mesh::DirEdge*>()) return;

 Mesh::DirEdge * targ = vert->edge;
 do
 {
  out.AddBack(Edge(targ));
  targ = targ->next;
 } while (targ!=vert->edge);
}

void Vertex::GetEdges(ds::Array<Edge> & out) const
{
 // Fill up a list...
  ds::List<Edge> list;
  GetEdges(list);

 // Stick the list into an array...
  out.Size(list.Size());
  ds::List<Edge>::Cursor targ = list.FrontPtr();
  for (nat32 i=0;i<out.Size();i++)
  {
   out[i] = *targ;
   ++targ;
  }
}

void Vertex::GetFaces(ds::List<Face> & out) const
{
 if (vert->edge==null<Mesh::DirEdge*>()) return;

 Mesh::DirEdge * targ = vert->edge;
 do
 {
  // Iterate half edges...
   if (targ->user)
   {
    Mesh::HalfEdge * loc = targ->user;
    do
    {
     out.AddBack(Face(loc->face));
     loc = loc->next;
    } while (loc!=targ->user);
   }

  targ = targ->next;
 } while (targ!=vert->edge);
}

void Vertex::GetFaces(ds::Array<Face> & out) const
{
 // Fill list...
  ds::List<Face> list;
  GetFaces(list);

 // Move the face into the array from the list...
  out.Size(list.Size());
  ds::List<Face>::Cursor targ = list.FrontPtr();
  for (nat32 i=0;i<out.Size();i++)
  {
   out[i] = *targ;
   ++targ;
  }
}

Edge Vertex::Link(Vertex other) const
{
 Mesh::DirEdge * targ = vert->edge;
 if (targ)
 {
  do
  {
   if (targ->to==other.vert) return Edge(targ);
   targ = targ->next;
  } while (targ!=vert->edge);
 }

 return Edge();
}

bit Vertex::SafeMove(const bs::Vert & newPos) const
{
 if (vert->edge!=null<Mesh::DirEdge*>())
 {
  Mesh::DirEdge * targ = vert->edge;
  do
  {
   // Iterate half edges...
    if (targ->user)
    {
     Mesh::HalfEdge * loc = targ->user;
     do
     {
      // Ok, we have the HalfEdge of a face that has to be checked...
      
      // Check that face inversion will not happen - calculate the 
      // normal with the original vertex and the replacement vertex,
      // dot them and check its positive...
      // (Loop to handle concavity with faces with more than 3 sides.)
       Mesh::HalfEdge * het = loc;
       for (nat32 j=0;j<loc->face->size;j++)
       {
        bs::Vert b = het->edge->to->pos;
        bs::Vert c = het->chain->edge->to->pos;
       
        bs::Normal orig;
        {
         bs::Vert da = b; da -= vert->pos;
         bs::Vert db = b; db -= c;
         math::CrossProduct(da,db,orig);
        }
       
        bs::Normal rep;
        {
         bs::Vert da = b; da -= newPos;
         bs::Vert db = b; db -= c;
         math::CrossProduct(da,db,rep);
        }
      
        if ((orig*rep)<0.0) return false;
        het = het->chain;
       }
      
      loc = loc->next;
     } while (loc!=targ->user);
    }

   targ = targ->next;
  } while (targ!=vert->edge);
 }
 return true;
}

void * Vertex::PropPtr(Vertex * ptr)
{
 return ptr->vert;
}

//------------------------------------------------------------------------------
nat32 Edge::FaceCount() const
{
 nat32 ret = 0;

 // Self side...
 {
  Mesh::HalfEdge * targ = edge->user;
  if (targ)
  {
   do
   {
    ret += 1;
    targ = targ->next;
   } while (targ!=edge->user);
  }
 }

 // Partner side...
 {
  Mesh::HalfEdge * targ = edge->partner->user;
  if (targ)
  {
   do
   {
    ret += 1;
    targ = targ->next;
   } while (targ!=edge->partner->user);
  }
 }

 return ret;
}

Vertex Edge::Other(Vertex v) const
{
 if (v.vert==edge->to) return Vertex(edge->partner->to);
                  else return Vertex(edge->to);
}

Vertex Edge::VertexA() const
{
 return Vertex(edge->to);
}

Vertex Edge::VertexB() const
{
 return Vertex(edge->partner->to);
}

void Edge::GetVertices(ds::List<Vertex> & out) const
{
 out.AddBack(Vertex(edge->partner->to));
 out.AddBack(Vertex(edge->to));
}

void Edge::GetVertices(ds::Array<Vertex> & out) const
{
 out.Size(2);
 out[0] = Vertex(edge->partner->to);
 out[1] = Vertex(edge->to);
}

void Edge::GetEdges(ds::List<Edge> & out) const
{
 // Current loop...
 {
  Mesh::DirEdge * targ = edge->next;
  while (targ!=edge)
  {
   out.AddBack(Edge(targ));
   targ = targ->next;
  }
 }

 // Partner loop...
 {
  Mesh::DirEdge * targ = edge->partner->next;
  while (targ!=edge->partner)
  {
   out.AddBack(Edge(targ));
   targ = targ->next;
  }
 }
}

void Edge::GetEdges(ds::Array<Edge> & out) const
{
 // Fill a list...
  ds::List<Edge> list;
  GetEdges(list);

 // Copy list into array...
  out.Size(list.Size());
  ds::List<Edge>::Cursor targ = list.FrontPtr();
  for (nat32 i=0;i<out.Size();i++)
  {
   out[i] = *targ;
   ++targ;
  }
}

void Edge::GetFaces(ds::List<Face> & out) const
{
 // Self side...
 {
  Mesh::HalfEdge * targ = edge->user;
  if (targ)
  {
   do
   {
    out.AddBack(Face(targ->face));
    targ = targ->next;
   } while (targ!=edge->user);
  }
 }

 // Partner side...
 {
  Mesh::HalfEdge * targ = edge->partner->user;
  if (targ)
  {
   do
   {
    out.AddBack(Face(targ->face));
    targ = targ->next;
   } while (targ!=edge->partner->user);
  }
 }
}

void Edge::GetFaces(ds::Array<Face> & out) const
{
 // Fill a list...
  ds::List<Face> list;
  GetFaces(list);

 // Copy this list into the array...
 out.Size(list.Size());
 ds::List<Face>::Cursor targ = list.FrontPtr();
 for (nat32 i=0;i<out.Size();i++)
 {
  out[i] = *targ;
  ++targ;
 }
}

Face Edge::AnyFace() const
{
 if (edge->user) return Face(edge->user->face);
 if (edge->partner->user) return Face(edge->partner->user->face);
 return Face();
}

Vertex Edge::Link(Edge other) const
{
 // Check all possible cases...
  if (edge->to==other.edge->to) return Vertex(edge->to);
  if (edge->to==other.edge->partner->to) return Vertex(edge->to);
  if (edge->partner->to==other.edge->to) return Vertex(edge->partner->to);
  if (edge->partner->to==other.edge->partner->to) return Vertex(edge->partner->to);

 // Not found, return a bad Vertex...
  return Vertex();
}

void * Edge::PropPtr(Edge * ptr)
{
 return ptr->edge;
}

//------------------------------------------------------------------------------
void Face::Eq(bs::Normal & norm,real32 & dist)
{
 Mesh::HalfEdge * targ = face->edge;
 for (nat32 i=0;i<face->size;i++)
 {
  Mesh::Vertex * v1 = targ->edge->partner->to;
  Mesh::Vertex * v2 = targ->chain->edge->partner->to;
  Mesh::Vertex * v3 = targ->chain->chain->edge->partner->to;
  
  // Create a normal with the vertices, break if its not degenerate, doing the distance as well...
   bs::Vert a = v2->pos; a -= v1->pos;
   bs::Vert b = v2->pos; b -= v3->pos;
   
   math::CrossProduct(a,b,norm);
   if (!math::IsZero(norm.LengthSqr()))
   {
    norm.Normalise();
    dist = -(norm * v1->pos);
    break;
   }
 
  targ = targ->chain;
 }
}

void Face::GetVertices(ds::List<Vertex> & out) const
{
 Mesh::HalfEdge * targ = face->edge;
 for (nat32 i=0;i<face->size;i++)
 {
  out.AddBack(Vertex(targ->edge->partner->to));
  targ = targ->chain;
 }
}

void Face::GetVertices(ds::Array<Vertex> & out) const
{
 out.Size(face->size);
 Mesh::HalfEdge * targ = face->edge;
 for (nat32 i=0;i<face->size;i++)
 {
  out[i] = Vertex(targ->edge->partner->to);
  targ = targ->chain;
 }
}

void Face::GetEdges(ds::List<Edge> & out) const
{
 Mesh::HalfEdge * targ = face->edge;
 for (nat32 i=0;i<face->size;i++)
 {
  out.AddBack(Edge(targ->edge));
  targ = targ->chain;
 }
}

void Face::GetEdges(ds::Array<Edge> & out) const
{
 out.Size(face->size);
 Mesh::HalfEdge * targ = face->edge;
 for (nat32 i=0;i<face->size;i++)
 {
  out[i] = Edge(targ->edge);
  targ = targ->chain;
 }
}

void Face::GetFaces(ds::List<Face> & out) const
{
 // Collect all the faces into a ds::SortList, to handle duplicates...
  ds::SortList<Mesh::Face*> list;
  Mesh::HalfEdge * targ = face->edge;
  for (nat32 i=0;i<face->size;i++)
  {
   // Same direction...
    Mesh::HalfEdge * loc = targ->next;
    while (loc!=targ)
    {
     list.Add(loc->face);
     loc = loc->next;
    }

   // Reverse direction...
    loc = targ->edge->partner->user;
    if (loc)
    {
     while (true)
     {
      list.Add(loc->face);
      loc = loc->next;
      if (loc==targ->edge->partner->user) break;
     }
    }

   // To next...
    targ = targ->chain;
  }


 // Append the sort list to the list...
 {
  ds::SortList<Mesh::Face*>::Cursor targ = list.FrontPtr();
  while (!targ.Bad())
  {
   out.AddBack(Face(*targ));
   ++targ;
  }
 }
}

void Face::GetFaces(ds::Array<Face> & out) const
{
 // Collect all the faces into a ds::SortList, to handle duplicates...
  ds::SortList<Mesh::Face*> list;
  Mesh::HalfEdge * targ = face->edge;
  for (nat32 i=0;i<face->size;i++)
  {
   // Same direction...
    Mesh::HalfEdge * loc = targ->next;
    while (loc!=targ)
    {
     list.Add(loc->face);
     loc = loc->next;
    }

   // Reverse direction...
    loc = targ->edge->partner->user;
    if (loc)
    {
     while (true)
     {
      list.Add(loc->face);
      loc = loc->next;
      if (loc==targ->edge->partner->user) break;
     }
    }

   // To next...
    targ = targ->chain;
  }


 // Copy the sort list into the array...
 {
  out.Size(list.Size());
  ds::SortList<Mesh::Face*>::Cursor targ = list.FrontPtr();
  for (nat32 i=0;i<out.Size();i++)
  {
   out[i] = Face(*targ);
   ++targ;
  }
 }
}

void Face::Link(Face other,ds::Array<Edge> & out) const
{
 // Iterate all edges of this face checking if they have the face other in
 // them, and storing if so...
  ds::List<Mesh::DirEdge*> list;
  Mesh::HalfEdge * targ = face->edge;
  for (nat32 i=0;i<face->size;i++)
  {
   bit store = false;
    // Same direction...
     Mesh::HalfEdge * loc = targ->next;
     while (loc!=targ)
     {
      store |= loc->face==other.face;
      loc = loc->next;
     }

    // Reverse direction...
     if (!store)
     {
      loc = targ->edge->partner->user;
      if (loc)
      {
       while (true)
       {
        store |= loc->face==other.face;
        loc = loc->next;
        if (loc==targ->edge->partner->user) break;
       }
      }
     }

   // If the other face was found, store it...
    if (store) list.AddBack(targ->edge);

   // To next...
    targ = targ->chain;
  }

 // Convert the list of stored edges into the array...
 {
  out.Size(list.Size());
  ds::List<Mesh::DirEdge*>::Cursor targ = list.FrontPtr();
  for (nat32 i=0;i<out.Size();i++)
  {
   out[i] = Edge(*targ);
   ++targ;
  }
 }
}

void Face::Reverse()
{
 // All we have to do it reverse the direction of every edge - this means moving
 // it to the partner DirEdge and changing arround all the chain pointers...
  Mesh::HalfEdge * targ = face->edge;
  Mesh::HalfEdge * prevTarg = null<Mesh::HalfEdge*>();
  for (nat32 i=0;i<face->size;i++)
  {
   // Reverse the chain...
    Mesh::HalfEdge * nextTarg = targ->chain;
    targ->chain = prevTarg;

   // Remove it from its DirEdge linked list...
    if (targ->next==targ) targ->edge->user = null<Mesh::HalfEdge*>();
    else
    {
     targ->next->prev = targ->prev;
     targ->prev->next = targ->next;
     targ->edge->user = targ->next;
    }

   // And add it to its partner's list...
    targ->edge = targ->edge->partner;

    if (targ->edge->user==null<Mesh::HalfEdge*>())
    {
     targ->edge->user = targ;
     targ->next = targ;
     targ->prev = targ;
    }
    else
    {
     targ->next = targ->edge->user;
     targ->prev = targ->edge->user->prev;
     targ->next->prev = targ;
     targ->prev->next = targ;
    }

   // Move to next...
    prevTarg = targ;
    targ = nextTarg;
  }
  targ->chain = prevTarg;
}

void * Face::PropPtr(Face * ptr)
{
 return ptr->face;
}

//------------------------------------------------------------------------------
 };
};
