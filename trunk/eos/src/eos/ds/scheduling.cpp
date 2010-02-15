//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/ds/scheduling.h"

#include "eos/mem/functions.h"
#include "eos/mem/preempt.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
SchedulerCode::SchedulerCode()
:verts(0),edges(0),first(null<Vert*>())
{}

SchedulerCode::~SchedulerCode()
{}

void SchedulerCode::Del(void (*Term)(void * ptr))
{
 while (first)
 {
  Vert * victim = first;
  first = first->next;
  
  while (victim->first)
  {
   Edge * toDie = victim->first;
   victim->first = victim->first->next;
   
   delete toDie;
  }
  
  Term(victim->Data());
  mem::Free(victim);
 }
}

void * SchedulerCode::MakeVert(nat32 elementSize,const void * data)
{
 Vert * nv = (Vert*)mem::Malloc<byte>(sizeof(Vert)+elementSize);
  nv->first = null<Edge*>();
  nv->incomming = 0;
  nv->inLeft = 0;

 nv->next = first;
 nv->prev = null<Vert*>();
 if (first) first->prev = nv;
 first = nv;
 ++verts;
  
 mem::Copy<byte>((byte*)nv->Data(),(byte*)data,elementSize);
  
 return nv;
}

void SchedulerCode::MakeEdge(void * first,void * second)
{
 Vert * from = (Vert*)first;
 Vert * to = (Vert*)second;

 Edge * ne = new Edge();
  ne->to = to;
  ne->next = from->first;
  from->first = ne;
  
 to->incomming += 1; 
 ++edges;
}

void SchedulerCode::DelVert(void * handle,void (*Term)(void * ptr))
{
 Vert * targ = (Vert*)handle;
 
 if (targ->next) targ->next->prev = targ->prev;
 if (targ->prev) targ->prev->next = targ->next;
            else first = targ->next;
            
 while (targ->first)
 {
  Edge * et = targ->first;
  targ->first = targ->first->next;
  
  et->to->incomming -= 1;
  delete et;
 }

 Term(targ->Data());
 mem::Free(targ);
}

void SchedulerCode::DelEdge(void * first,void * second)
{
 Vert * from = (Vert*)first;
 Vert * to = (Vert*)second;
 
 Edge * et = from->first;
 Edge * trail = null<Edge*>();
 while (et)
 {
  if (et->to==to)
  {
   if (trail) trail->next = et->next;
         else from->first = et->next;
   
   to->incomming -= 1;
   delete et;
   
   break;
  }
  
  trail = et;
  et = et->next;
 }
}

bit SchedulerCode::Sort()
{
 // Change the meaning of lists, set the first list to be our output, then
 // create another list of not yet avaliable vertices, todo, into which all current
 // vertices belong. In addition, we need a pointer to the working node in first,
 // i.e. where we are in our queue of nodes to process...
  Vert * targ;
  Vert * todo = first;
  first = null<Vert*>();
  
 // Iterate the entire list and extract into first all vertices that have
 // incomming==0...
 {
  Vert * i = todo;
  while (i!=null<Vert*>())
  {
   if (i->incomming==0)
   {
    Vert * it = i; i = i->next;
    
    if (it->prev) it->prev->next = it->next;
             else todo = it->next;
    if (it->next) it->next->prev = it->prev;
    
    it->next = first;
    it->prev = null<Vert*>();
    if (first) first->prev = it;
    first = it;
   }
   else 
   {
    i->inLeft = i->incomming;
    i = i->next;
   }
  }
 }

 // Set targ to first and keep looping over the queue until we reach the end, 
 // moving nodes from todo to the next position in first (So it in fact runs as
 // a stack.) when there inLeft reaches 0...
  targ = first;
  while (targ)
  {
   Edge * et = targ->first;
   while (et)
   {
    et->to->inLeft -= 1;
    if (et->to->inLeft==0)
    {
     if (et->to->next) et->to->next->prev = et->to->prev;
     if (et->to->prev) et->to->prev->next = et->to->next;
                  else todo = et->to->next;
     
     et->to->next = targ->next;
     et->to->prev = targ;
     if (et->to->next) et->to->next->prev = et->to;
     targ->next = et->to;
    }
    et = et->next;
   }
   
   targ = targ->next;
  }
 
 // Check that todo is empty, if not then there is a cycle and we have failed,
 // move todo into first to prevent a memory leak and return an error...
  if (todo==null<Vert*>()) return true;
  else
  {
   while (todo)
   {
    Vert * targ = todo;
    todo = todo->next;
    
    targ->next = first;
    targ->prev = null<Vert*>();
    if (first) first->prev = targ;
    first = targ;
   }
   return false;
  }
}

//------------------------------------------------------------------------------
 };
};
