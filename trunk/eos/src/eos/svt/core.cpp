//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/svt/core.h"

#include "eos/svt/type.h"
#include "eos/svt/file.h"

namespace eos
{
 namespace svt
 {
//------------------------------------------------------------------------------
Core::Core(str::TokenTable & t)
:tt(t)
{
 AddType("HON",0,CreateNode,LoadNode);
 AddType("SID","HON",CreateMeta,LoadMeta);
 AddType("MID","SID",CreateVar,LoadVar);  
}

Core::~Core(){}

Type * Core::NewType(str::Token name)
{
 Type *& ptr = types[name];
 if (ptr) return null<Type*>();
 ptr = new Type();
 return ptr;
}

Type * Core::GetType(str::Token name)
{
 Type ** ptr = types.Get(name);
 if (ptr) return *ptr;
     else return null<Type*>();
}

bit Core::IsType(str::Token name,Node * node)
{
 Type ** ptr = types.Get(name);
 if (ptr) return (*ptr)->MemberOf(*node);
     else return true;
}

Node * Core::LoadObject(io::InVirt<io::Binary> & in)
{
 // Peek and find out what we are dealing with...
  struct Head
  {
   cstrchar bm[3];
   cstrchar om[3];
  } head;
  if (in.Peek((byte*)(void*)&head,6)!=6) return null<Node*>();
 
 // Search for the final object type - hope to hell its found as it makes
 // things a lot easier...
  LoadType dummy; dummy.magic = head.om;
  LoadType * targ = ltl.Get(dummy);
  
  if (targ)
  {
   // The easy case - we know this object type...
    return targ->Load(*this,in,null<Node*>());
  }
  else
  {
   // The file contains an unrecognised type - the fallback code has to kick in...
    Node * ret = null<Node*>();
     while (true)
     {
      // Obtain the type we need to load...
       LoadType dummy; dummy.magic = head.bm;
       LoadType * targ = ltl.Get(dummy);
       if (targ==null<LoadType*>()) break;
      
      // Upgrade the previous object to the next level... 
       if (ret) targ->Create(*this,ret);

      // Load it into the object...
       ret = targ->Load(*this,in,ret);
       if (ret==null<Node*>()) break;
      
      // Fetch the next head...
       if (in.Peek((byte*)(void*)&head,6)!=6) {delete ret; ret = null<Node*>(); break;}
     }
     
     // We have loaded all that we can - skip the blocks we can't use...
      while (true)
      {
       // Skip the block...
        nat32 lowSize;
        nat8 highSize;
        if (in.Skip(6)!=6) {delete ret; ret = null<Node*>(); break;}
        if (in.Read(&lowSize,4)!=4) {delete ret; ret = null<Node*>(); break;}
        if (in.Read(&highSize,1)!=1) {delete ret; ret = null<Node*>(); break;}
        if (in.Skip(5)!=5) {delete ret; ret = null<Node*>(); break;}
        
        if (highSize!=0) {delete ret; ret = null<Node*>(); break;}
        if (in.Skip(lowSize-16)!=(lowSize-16)) {delete ret; ret = null<Node*>(); break;}
       
       // Break if this was the final block...
        if ((head.bm[0]==head.om[0])&&(head.bm[1]==head.om[1])&&(head.bm[2]==head.om[2])) break;
       
       // Fetch next head...
        in.Peek((byte*)(void*)&head,6);
      }
     
    return ret;
  }
}
void Core::AddType(cstrconst magic,cstrconst parent,
                   Node * (*Create)(Core & core,Node * parent),
                   Node * (*Load)(Core & core,io::InVirt<io::Binary> & in,Node * self))
{
 LoadType lt;
  lt.magic = magic;
  lt.parent = parent;
  lt.Create = Create;
  lt.Load = Load;
 ltl.Add(lt);
}

//------------------------------------------------------------------------------
 };
};
