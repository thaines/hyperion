//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/inf/fg_vars.h"

#include "eos/mem/alloc.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace inf
 {
//------------------------------------------------------------------------------
Variable::Variable()
:data(jumpSize),links(0)
{}

Variable::~Variable()
{}

void Variable::Reset()
{
 links = 0;
}

void Variable::WillAdd(nat32 lc)
{
 if (data.Size()<lc) data.Size(lc);
}

bit Variable::AddLink(const Link & link)
{
 // Verify that types match...
  if (links!=0)
  {
   MessageClass ma; data[links-1].fs->GetFunc()->LinkType(data[links-1].link,ma);
   MessageClass mb; link.fs->GetFunc()->LinkType(link.link,mb);
   if (ma!=mb) return false;
  }
 
 // Add it in, assuming we have got this far...
  if (links==data.Size()) data.Size(links+jumpSize);
   data[links] = link;
   ++links;
 return true;
}

nat32 Variable::LinkCount() const
{
 return links;
}

void Variable::LinkDetail(nat32 n,Link & out) const
{
 out = data[n];
}

void * Variable::GetPacked() const
{
 Packed * ret = (Packed*)mem::Malloc<byte>(sizeof(Packed) + sizeof(Packed::Link)*(links-1));
 
 data[0].fs->GetFunc()->LinkType(data[0].link,ret->mc);
 ret->size = links;
 
 for (nat32 i=0;i<links;i++)
 {
  ret->link[i].in = data[i].fs->FromFunc(data[i].instance,data[i].link);
  ret->link[i].out = data[i].fs->ToFunc(data[i].instance,data[i].link); 
 }
 
 return ret;
}

nat32 Variable::Links(void * pv)
{
 Packed * var = (Packed*)pv;
 return var->size;
}

const MessageClass & Variable::Class(void * pv)
{
 Packed * var = (Packed*)pv;
 return var->mc;
}

void Variable::SendOneSP(void * pv,nat32 l)
{
 Packed * var = (Packed*)pv;
 
 // Flatline the relevent output...
  var->mc.Flatline(var->mc,var->link[l].out);
 
 // Multiply in all messages before...
  for (nat32 i=0;i<l;i++) var->mc.InplaceMult(var->mc,var->link[l].out,var->link[i].in);
  
 // Multiply in all messages after...
  for (nat32 i=l+1;i<var->size;i++) var->mc.InplaceMult(var->mc,var->link[l].out,var->link[i].in);
}

void Variable::SendAllButOneSP(void * pv,nat32 l,data::Block & temp)
{
 Packed * var = (Packed*)pv;
 
 // Before we begin, flatline all the outputs...
  for (nat32 i=0;i<l;i++) var->mc.Flatline(var->mc,var->link[i].out);
  for (nat32 i=l+1;i<var->size;i++) var->mc.Flatline(var->mc,var->link[i].out);
  
 // Now choose an algorithm depending on which is probably faster...
  if (var->size<=algChange)
  {
   // The nice simple O(n^2) algorithm...
    for (nat32 i=0;i<l;i++)
    {
     // Multiply in all messages before...
      for (nat32 j=0;j<i;j++) var->mc.InplaceMult(var->mc,var->link[j].out,var->link[i].in);
  
     // Multiply in all messages after...
      for (nat32 j=i+1;j<var->size;j++) var->mc.InplaceMult(var->mc,var->link[j].out,var->link[i].in);
    }
    
    // Skip l.
    
    for (nat32 i=l+1;i<var->size;i++)
    {
     // Multiply in all messages before...
      for (nat32 j=0;j<i;j++) var->mc.InplaceMult(var->mc,var->link[j].out,var->link[i].in);
  
     // Multiply in all messages after...
      for (nat32 j=i+1;j<var->size;j++) var->mc.InplaceMult(var->mc,var->link[j].out,var->link[i].in);
    }    
  }
  else
  {
   // The bloody complicated (roughly) O(n log n) algorithm...
    // First work out how big we need the stack to be, and make sure its that large...
     nat32 msgSize = var->mc.Size(var->mc);
     nat32 frameSize = sizeof(Frame) + msgSize*2;
     nat32 leftOffset = sizeof(Frame);
     nat32 rightOffset = sizeof(Frame) + msgSize;
     nat32 maxDepth = 1 + math::TopBit(var->size);
     nat32 dataNeeded = maxDepth*frameSize;
     
     if (temp.Size()<dataNeeded) temp.SetSize(dataNeeded);
     
    // Setup the stack with the starting information needed...
     Frame * stack = (Frame*)temp.Ptr();
      stack->min = 0;
      stack->max = var->size-1;
      stack->op = Frame::GoLeft;
     nat32 depth = 0;
    
    // Now run over the stack until we pop off the top and have hence done all calculations...
     while (true)
     {
      Frame * targ = (Frame*)(((byte*)stack) + frameSize*depth);
      if (targ->op==Frame::GoLeft)
      {
       targ->centre = (targ->min + targ->max)/2;
              
       if (targ->min==targ->centre)
       {
        mem::Copy<byte>(((byte*)targ)+leftOffset,(byte*)var->link[targ->min].in,msgSize);
        targ->op = Frame::GoRight;
       }
       else
       {
        Frame * child = (Frame*)(((byte*)targ) + frameSize);
        child->min = targ->min;
        child->max = targ->centre;
        
        targ->op = Frame::GoRight;
        child->op = Frame::GoLeft;
        ++depth;
       }
      }
      else if (targ->op==Frame::GoRight)
      {
       if (targ->centre+1==targ->max)
       {
        mem::Copy<byte>(((byte*)targ)+rightOffset,(byte*)var->link[targ->max].in,msgSize);
        targ->op = Frame::GoUp;
       }
       else
       {
        Frame * child = (Frame*)(((byte*)targ) + frameSize);
        child->min = targ->centre+1;
        child->max = targ->max;
        
        targ->op = Frame::GoUp;
        child->op = Frame::GoLeft;
        ++depth;
       }       
      }
      else // Frame::GoUp...
      {
       // Multiply the left hand side with the members of the right hand side, and visa versa...
       // (Avoiding member l.)
        if (l<=targ->centre)
        {
         if (l<targ->min)
         {
          for (nat32 i=targ->min;i<=targ->centre;i++) var->mc.InplaceMult(var->mc,var->link[i].out,((byte*)targ)+rightOffset);
         }
         else
         {
          for (nat32 i=targ->min;i<l;i++) var->mc.InplaceMult(var->mc,var->link[i].out,((byte*)targ)+rightOffset);
          for (nat32 i=l+1;i<=targ->centre;i++) var->mc.InplaceMult(var->mc,var->link[i].out,((byte*)targ)+rightOffset);                   
         }
         for (nat32 i=targ->centre+1;i<=targ->max;i++) var->mc.InplaceMult(var->mc,var->link[i].out,((byte*)targ)+leftOffset);
        }
        else
        {
         for (nat32 i=targ->min;i<=targ->centre;i++) var->mc.InplaceMult(var->mc,var->link[i].out,((byte*)targ)+rightOffset);
         if (l>targ->max)
         {
          for (nat32 i=targ->centre+1;i<=targ->max;i++) var->mc.InplaceMult(var->mc,var->link[i].out,((byte*)targ)+leftOffset);
         }
         else
         {
          for (nat32 i=targ->centre+1;i<l;i++) var->mc.InplaceMult(var->mc,var->link[i].out,((byte*)targ)+leftOffset);
          for (nat32 i=l+1;i<=targ->max;i++) var->mc.InplaceMult(var->mc,var->link[i].out,((byte*)targ)+leftOffset);
         }
        }
        
       // If we are popping off the top break, if not multiply the left and right together
       // and put them in the parent frame...
        if (depth==0) break;
        else
        {
         Frame * parent = (Frame*)(((byte*)targ) - frameSize);
         if (parent->op==Frame::GoRight)
         {
          var->mc.Mult(var->mc,((byte*)parent)+leftOffset,((byte*)targ)+leftOffset,((byte*)targ)+rightOffset);
         }
         else
         {
          var->mc.Mult(var->mc,((byte*)parent)+rightOffset,((byte*)targ)+leftOffset,((byte*)targ)+rightOffset);
         }
        }
      }
     }
  }
}

void Variable::SendAllSP(void * pv,data::Block & temp)
{
 Packed * var = (Packed*)pv;
 
 // Before we begin, flatline all the outputs...
  for (nat32 i=0;i<var->size;i++) var->mc.Flatline(var->mc,var->link[i].out);

 // Now choose an algorithm depending on which is probably faster...
  if (var->size<=algChange)
  {
   // The nice simple O(n^2) algorithm...
    for (nat32 i=0;i<var->size;i++)
    {
     // Multiply in all messages before...
      for (nat32 j=0;j<i;j++) var->mc.InplaceMult(var->mc,var->link[j].out,var->link[i].in);
  
     // Multiply in all messages after...
      for (nat32 j=i+1;j<var->size;j++) var->mc.InplaceMult(var->mc,var->link[j].out,var->link[i].in);
    }
  }
  else
  {
   // The bloody complicated (roughly) O(n log n) algorithm...
    // First work out how big we need the stack to be, and make sure its that large...
     nat32 msgSize = var->mc.Size(var->mc);
     nat32 frameSize = sizeof(Frame) + msgSize*2;
     nat32 leftOffset = sizeof(Frame);
     nat32 rightOffset = sizeof(Frame) + msgSize;
     nat32 maxDepth = 1 + math::TopBit(var->size);
     nat32 dataNeeded = maxDepth*frameSize;
     
     if (temp.Size()<dataNeeded) temp.SetSize(dataNeeded);
     
    // Setup the stack with the starting information needed...
     Frame * stack = (Frame*)temp.Ptr();
      stack->min = 0;
      stack->max = var->size-1;
      stack->op = Frame::GoLeft;
     nat32 depth = 0;
    
    // Now run over the stack until we pop off the top and have hence done all calculations...
     while (true)
     {
      Frame * targ = (Frame*)(((byte*)stack) + frameSize*depth);
      if (targ->op==Frame::GoLeft)
      {
       targ->centre = (targ->min + targ->max)/2;
              
       if (targ->min==targ->centre)
       {
        mem::Copy<byte>(((byte*)targ)+leftOffset,(byte*)var->link[targ->min].in,msgSize);
        targ->op = Frame::GoRight;
       }
       else
       {
        Frame * child = (Frame*)(((byte*)targ) + frameSize);
        child->min = targ->min;
        child->max = targ->centre;
        
        targ->op = Frame::GoRight;
        child->op = Frame::GoLeft;
        ++depth;
       }
      }
      else if (targ->op==Frame::GoRight)
      {
       if (targ->centre+1==targ->max)
       {
        mem::Copy<byte>(((byte*)targ)+rightOffset,(byte*)var->link[targ->max].in,msgSize);
        targ->op = Frame::GoUp;
       }
       else
       {
        Frame * child = (Frame*)(((byte*)targ) + frameSize);
        child->min = targ->centre+1;
        child->max = targ->max;
        
        targ->op = Frame::GoUp;
        child->op = Frame::GoLeft;
        ++depth;
       }       
      }
      else // Frame::GoUp...
      {
       // Multiply the left hand side with the members of the right hand side, and visa versa...
        for (nat32 i=targ->min;i<=targ->centre;i++) var->mc.InplaceMult(var->mc,var->link[i].out,((byte*)targ)+rightOffset);
        for (nat32 i=targ->centre+1;i<=targ->max;i++) var->mc.InplaceMult(var->mc,var->link[i].out,((byte*)targ)+leftOffset);
       
       // If we are popping off the top break, if not multiply the left and right together
       // and put them in the parent frame...
        if (depth==0) break;
        else
        {
         Frame * parent = (Frame*)(((byte*)targ) - frameSize);
         if (parent->op==Frame::GoRight)
         {
          var->mc.Mult(var->mc,((byte*)parent)+leftOffset,((byte*)targ)+leftOffset,((byte*)targ)+rightOffset);
         }
         else
         {
          var->mc.Mult(var->mc,((byte*)parent)+rightOffset,((byte*)targ)+leftOffset,((byte*)targ)+rightOffset);
         }
        }
      }
     }
  }
}

void Variable::CalcOutputSP(void * pv,void * msgOut)
{
 Packed * var = (Packed*)pv;
 
 // Flatline the output...
  var->mc.Flatline(var->mc,msgOut);
 
 // Multiply in all messages into the final result...
  for (nat32 i=0;i<var->size;i++) var->mc.InplaceMult(var->mc,msgOut,var->link[i].in);
  
 // Normalise...
  var->mc.Norm(var->mc,msgOut);
}

void Variable::SendOneMS(void * pv,nat32 l)
{
 Packed * var = (Packed*)pv;
 
 // Flatline the relevent output...
  var->mc.FlatlineLn(var->mc,var->link[l].out);
 
 // Sum in all messages before...
  for (nat32 i=0;i<l;i++) var->mc.InplaceAdd(var->mc,var->link[l].out,var->link[i].in);
  
 // Sum in all messages after...
  for (nat32 i=l+1;i<var->size;i++) var->mc.InplaceAdd(var->mc,var->link[l].out,var->link[i].in);
}

void Variable::SendAllButOneMS(void * pv,nat32 l)
{
 Packed * var = (Packed*)pv;
 
 nat32 sumEntry = 0;
 if (sumEntry==l) sumEntry = 1;
 if (var->size<=sumEntry) return;

 // Sum all into the sumEntry entry...
  var->mc.FlatlineLn(var->mc,var->link[sumEntry].out);
  for (nat32 i=0;i<var->size;i++) var->mc.InplaceAdd(var->mc,var->link[sumEntry].out,var->link[i].in);

 // Set all but l and sumEntry to sumEntry minus there message, so they are correct...
  for (nat32 i=0;i<var->size;i++)
  {
   if ((i!=l)&&(i!=sumEntry))
   {
    mem::Copy<byte>((byte*)var->link[i].out,(byte*)var->link[sumEntry].out,var->mc.Size(var->mc));
    var->mc.InplaceNeg(var->mc,var->link[i].out,var->link[i].in);
   }
  }
 
 // Correct the sumEntry... 
  var->mc.InplaceNeg(var->mc,var->link[sumEntry].out,var->link[sumEntry].in);
}

void Variable::SendAllMS(void * pv)
{
 Packed * var = (Packed*)pv;

 // Sum all but the first into the first entry, to make a correct first entry...
  var->mc.FlatlineLn(var->mc,var->link[0].out);
  if (var->size==1) return;  
  for (nat32 i=1;i<var->size;i++) var->mc.InplaceAdd(var->mc,var->link[0].out,var->link[i].in);

 // Make the second entry the sum of all of them by adding in the first...
  mem::Copy<byte>((byte*)var->link[1].out,(byte*)var->link[0].out,var->mc.Size(var->mc));
  var->mc.InplaceAdd(var->mc,var->link[1].out,var->link[0].in);

 // Iterate every other entry and make correct by subtraction...
  for (nat32 i=2;i<var->size;i++)
  {
   mem::Copy<byte>((byte*)var->link[i].out,(byte*)var->link[1].out,var->mc.Size(var->mc));
   var->mc.InplaceNeg(var->mc,var->link[i].out,var->link[i].in);
  }

 // Correct the second entry by subtraction...
  var->mc.InplaceNeg(var->mc,var->link[1].out,var->link[1].in);
  
 /*for (nat32 i=0;i<var->size;i++)
 {
  Frequency in = Frequency(var->mc,var->link[i].in);
  Frequency out = Frequency(var->mc,var->link[i].out);
  
  LogDebug("[var] in " << i << LogDiv() << in);
  LogDebug("[var] out " << i << LogDiv() << out);
 }*/
}

void Variable::CalcOutputMS(void * pv,void * msgOut)
{
 Packed * var = (Packed*)pv;
 
 // Flatline the output...
  var->mc.FlatlineLn(var->mc,msgOut);
 
 // Sum all messages into the final result...
  for (nat32 i=0;i<var->size;i++) var->mc.InplaceAdd(var->mc,msgOut,var->link[i].in);
  
 // Normalise...
  var->mc.Drop(var->mc,msgOut);
}

//------------------------------------------------------------------------------
 };
};
