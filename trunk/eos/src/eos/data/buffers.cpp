//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines
#include "eos/data/buffers.h"

#include "eos/mem/functions.h"
#include "eos/math/functions.h"
#include "eos/file/csv.h"

namespace eos
{
 namespace data
 {
//------------------------------------------------------------------------------
Buffer::Buffer(nat32 s,nat32 pre)
:size(s),offset(pre%blockSize),first(null<Node*>()),last(null<Node*>())
{
 first = new Node();
 last = first;

 nat32 maxSize = blockSize-offset;
 while (maxSize<size)
 {
  last->next = new Node();
  last = last->next;
  maxSize += blockSize;       
 }

 log::Assert(Invariant());
}

Buffer::Buffer(void * data,nat32 s)
:size(0),offset(0),first(null<Node*>()),last(null<Node*>())
{
 first = new Node();
 last = first;

 Write(data,s);
 log::Assert(Invariant());
}

Buffer::Buffer(const Buffer & rhs)
:size(rhs.size),offset(rhs.offset),first(null<Node*>()),last(null<Node*>())
{
 Node * tin = rhs.first;
 Node ** tout = &first;
  while (tin)
  {
   *tout = new Node();
   last = *tout;
   mem::Copy((*tout)->data,tin->data,blockSize);
   
   tout = &(*tout)->next;
   tin = tin->next;
  }
 *tout = null<Node*>();

 log::Assert(Invariant());
}

Buffer::~Buffer()
{
 log::Assert(Invariant());

 Node * targ = first;
 while (targ)
 {
  Node * victim = targ;
  targ = targ->next;
  delete victim;       
 }
}

Buffer & Buffer::operator = (const Buffer & rhs)
{
 //LogBlock("Buffer & Buffer::operator = (const Buffer & rhs)","-");
 log::Assert(Invariant());

 // We don't empty the current class, instead we overwrite as far as possible 
 // before creating new nodes/terminating excess.
  Node * tin = rhs.first;
  Node ** tout = &first;
  while (tin)
  {
   if (*tout==null<Node*>())
   {
    *tout = new Node();
    (*tout)->next = null<Node*>(); 
   }
   mem::Copy((*tout)->data,tin->data,blockSize);
   
   last = *tout;
   tout = &(*tout)->next;
   tin = tin->next;
  }
  
  if (*tout)
  {
   Node * victim = *tout;
   *tout = null<Node*>();
   
   while (victim)
   {
    Node * targ = victim->next;
    delete victim;
    victim = targ;       
   }
  }
  
  size = rhs.size;
  offset = rhs.offset;

 log::Assert(Invariant());
 return *this;        
}

Buffer & Buffer::operator += (const Buffer & rhs)
{
 //LogBlock("Buffer & Buffer::operator += (const Buffer & rhs)","-");
 log::Assert(Invariant());
 // Unfortunatly the nature of using a linked list of blocks means we can not
 // assume allignment of data blocks, therefore we have to write some rather 
 // nasty code.
 // Essentially there are two possibilities, the blocks allign or they don't,
 // if they align its fast, if not we have to alternate between emptying the in
 // node and filling up the out node.
  nat32 toDo = rhs.size;
  
  nat32 tin_off = rhs.offset;
  Node * tin = rhs.first;
  
  nat32 tout_off = LastOffset();
  Node * tout = last;
  
  if (tin_off==tout_off)
  {
   // The blocks align...
    nat32 amount = blockSize-tout_off;
    mem::Copy(tout->data+tout_off,tin->data+tin_off,amount);
    
    if (amount<toDo)
    {
     toDo -= amount;
     
     while (true)
     {
      tin = tin->next;
      tout->next = new Node();
      tout = tout->next;
     
      mem::Copy(tout->data,tin->data,blockSize);
      
      if (blockSize>=toDo) break;
      toDo -= blockSize;
     }      
    }
  }
  else
  {
   // The blocks don't align...       
    while (true)
    {
     if (tin_off>tout_off)
     {
      // Consume an in node...
       nat32 amount = blockSize-tin_off;
       mem::Copy(tout->data+tout_off,tin->data+tin_off,amount);
       
       if (amount>=toDo) break;
       toDo -= amount;
       
       tout_off += amount;
       tin = tin->next; tin_off = 0;
     }
      
     //if (tin_off<tout_off) // Test not needed, as neccesarilly true at this point.
     {
      // Consume an out node...
       nat32 amount = blockSize-tout_off;
       mem::Copy(tout->data+tout_off,tin->data+tin_off,amount);
       
       if (amount>=toDo) break;
       toDo -= amount;
       
       tin_off += amount;
       tout->next = new Node();
       tout = tout->next; tout_off = 0;           
     }     
    }          
  }

  last = tout;
  size += rhs.size;

 log::Assert(Invariant());
 return *this;
}

bit Buffer::operator == (const Buffer & rhs) const
{
 const nat32 bS = blockSize;
 if (Size()!=rhs.Size()) return false;
 Cursor lc = const_cast<Buffer*>(this)->GetCursor();
 Cursor rc = const_cast<Buffer&>(rhs).GetCursor();
  byte lb[bS];
  byte rb[bS];
  while (!lc.EOS())
  {
   nat32 atc = math::Min(bS,lc.Avaliable());
    lc.Read(lb,atc);
    rc.Read(rb,atc);

   if (mem::Compare(lb,rb,atc)!=0) return false;
  }
 return true;
}

bit Buffer::operator < (const Buffer & rhs) const
{
 const nat32 bS = blockSize;
 Cursor lc = const_cast<Buffer*>(this)->GetCursor();
 Cursor rc = const_cast<Buffer&>(rhs).GetCursor();
  byte lb[bS];
  byte rb[bS];
  while ((!lc.EOS())&&(!rc.EOS()))
  {
   nat32 atc = math::Min(bS,lc.Avaliable(),rc.Avaliable());
    lc.Read(lb,atc);
    rc.Read(rb,atc);

   int32 res = mem::Compare(lb,rb,atc);
   if (res!=0) return res<0;
  }
  if (Size()<rhs.Size()) return true;
                    else return false;
}

bit Buffer::operator > (const Buffer & rhs) const
{
 const nat32 bS = blockSize;
 Cursor lc = const_cast<Buffer*>(this)->GetCursor();
 Cursor rc = const_cast<Buffer&>(rhs).GetCursor();
  byte lb[bS];
  byte rb[bS];
  while ((!lc.EOS())&&(!rc.EOS()))
  {
   nat32 atc = math::Min(bS,lc.Avaliable(),rc.Avaliable());
    lc.Read(lb,atc);
    rc.Read(rb,atc);

   int32 res = mem::Compare(lb,rb,atc);
   if (res!=0) return res>0;
  }
  if (Size()>rhs.Size()) return true;
                    else return false;
}

nat32 Buffer::SetSizePre(nat32 sze)
{
 //LogBlock("nat32 Buffer::SetSizePre(nat32 sze)","sze" << LogDiv() << sze);
 log::Assert(Invariant());
 // We use different code depending on if this is an enlargment or a reduction...
  if (sze>=size) // or equal so the sze==size case goes through here, and gets caught on the easy case.
  {
   // An enlargment...
    nat32 toDo = sze-size;
    if (toDo<=offset)
    {
     // Nice easy case - we have space in the current first node to make the adjustment...
      offset -= toDo; 
    }
    else
    {
     // We need to prepend more nodes...
      toDo -= offset;
      while (true)
      {
       Node * targ = new Node();
       targ->next = first;
       first = targ->next;
       if (blockSize>=toDo) break;
                       else toDo -= blockSize;
      }
      offset = blockSize-toDo;
    }           
  }
  else
  {
   // A reduction...
    // We have to special case setting size to 0, as the normal shrinking code will fail with a 0...
     if (sze==0)
     {
      offset = 0;
      Node * victim = first->next;
      first->next = null<Node*>();
      last = first;
      
      while (victim)
      {
       Node * next = victim->next;
       delete victim;
       victim = next;       
      }
     }
     else
     {
      nat32 toDo = size-sze+offset;
      while (toDo>=blockSize)
      {
       Node * victim = first;
       first = first->next;
       delete victim;
       toDo -= blockSize;       
      }
      offset = toDo;
     }
  }
 size = sze;
 
 log::Assert(Invariant());
 return size; 
}

nat32 Buffer::SetSize(nat32 sze)
{
 //LogBlock("nat32 Buffer::SetSize(nat32 sze)","{sze}" << LogDiv() << sze);
 log::Assert(Invariant());
 // Seperate code depending on if we are increasing in size or decreasing in size...
  if (sze>size)
  {
   // Increasing with size...
    nat32 toDo = sze-size;
    nat32 spareSpace = blockSize-LastOffset();
    if (spareSpace<toDo) // If the change is small enough to fit in the final node, do nothing.
    {
     toDo -= spareSpace;
     while (true)
     {
      last->next = new Node();
      last = last->next;
      if (toDo<=blockSize) break;
                      else toDo -= blockSize;
     }
     last->next = null<Node*>();       
    }
  }
  else
  {
   // Decreasing in size...
    if (LastOffset()<(size-sze)) // If the change is small enough to fit in the final node, do nothing.
    {
     nat32 toDo = sze + offset;
     last = first;
     while (true)
     {
      if (toDo<=blockSize) break;
                      else toDo -= blockSize;
      last = last->next;
     }
     Node * victim = last->next;
     last->next = null<Node*>();
     
     while (victim)
     {
      Node * targ = victim->next;
      delete victim;
      victim = targ;
     }
    }    
  }
  
 size = sze;
 log::Assert(Invariant());
 return size;        
}

Buffer::Cursor Buffer::GetCursor(nat32 pos)
{
 if (pos==0)
 {
  // Start of buffer requested...
   return Cursor(*this,first,0,offset);
 }
 else if (pos>=size)
 {
  // End of buffer requested...
   return Cursor(*this,last,pos,LastOffset()+pos-size);      
 }
 else
 {
  // The middle, a painful case for which the design is weak...
   nat32 os = offset + pos;
   Node * targ = first;
   while (os>=blockSize)
   {
    targ = targ->next;
    os -= blockSize;
   }
   
  return Cursor(*this,targ,pos,os);
 }
}

nat32 Buffer::Read(void * out,nat32 bytes)
{
 nat32 ret = 0;

  while (bytes!=0)
  {
   nat32 toDo = math::Min(bytes,blockSize-offset);
   mem::Copy((byte*)out,first->data+offset,toDo);
   
   bytes -= toDo;
   ret += toDo;
   (byte*&)out += toDo;
   
   if (first!=last)
   {
    Node * victim = first;
    first = first->next;
    offset = 0;
    delete victim;
   }
   else
   {
    if (offset==blockSize) offset = 0;
    break;
   }
  }

 log::Assert(Invariant());
 return ret;
}

nat32 Buffer::Skip(nat32 bytes)
{
 nat32 ret = 0;

  while (bytes!=0)
  {
   nat32 toDo = math::Min(bytes,blockSize-offset);

   bytes -= toDo;
   ret += toDo;
   
   if (first!=last)
   {
    Node * victim = first;
    first = first->next;
    offset = 0;
    delete victim;
   }
   else
   {
    if (offset==blockSize) offset = 0;
    break;
   }
  }

 log::Assert(Invariant());
 return ret;
}

bit Buffer::Invariant() const
{
 // Simply walk the list, check the nodes sum to the correct size...
  nat32 totalFound = 0;
  Node * targ = first;
  while (targ!=last)
  {
   totalFound += blockSize;
   targ = targ->next;
  }

  totalFound += LastOffset();
  totalFound -= offset;

 bit ret = (totalFound==size)&&(last->next==null<Node*>());
  if (ret==false)
  {
   LogAlways("[buffer] Failed invariant {size recorded,size determined,last->next==null}" << LogDiv() << size << LogDiv() << totalFound << LogDiv() << (last->next==null<Node*>()));
  }
 return ret;
}

//------------------------------------------------------------------------------
void Buffer::Cursor::ToStart()
{
 node = targ->first;
 pos = 0;
 offset = targ->offset;       
}

void Buffer::Cursor::To(nat32 p)
{
 // Offset pos by the starting offset for this code, and our target as well, 
 // it makes things simpler...
  pos += targ->offset;
  p += targ->offset;
 
 // Null offset, makes following code simpler...
  pos -= offset;
  // offset = 0;
 
 // If the postion we are seeking to is before the current position we have to
 // return to the start and seek from there. Painful...
  if (p<pos)
  {
   node = targ->first;
   pos = 0;
   // offset = 0;
  }
 
 // Seek from current position till we get to where we want to be...
  while (((pos+blockSize)<p)&&(node->next))
  {
   node = node->next;
   pos += blockSize;
  }
  
 // Set offset to where it should be...
  offset = p-pos; // Note that the behaviour of this when node.next being null breaks out of the above loop is correct, as extensions to data consumption should not happen till a write actually happens.
  pos = p;
 
 // Return pos to the correct position relative to targ.offset...
  pos -= targ->offset;
}

void Buffer::Cursor::Offset(int32 o)
{
 nat32 p = pos;
 if (-o>=int32(p)) p = 0;
              else p += o;

 To(p);
}

void Buffer::Cursor::ToEnd()
{
 node = targ->last;
 pos = targ->size;
 offset = targ->LastOffset();       
}

void Buffer::Cursor::SkipTo(byte b)
{
 byte t;
 while (Peek(&t,1)==1)
 {
  if (b==t) break;
  else Skip(1);
 }
}

nat32 Buffer::Cursor::Avaliable() const
{
 if (pos>=targ->size) return 0;
                 else return targ->size - pos;
}

nat32 Buffer::Cursor::Read(void * out,nat32 bytes)
{
 //LogBlock("nat32 Buffer::Cursor::Read(void * out,nat32 bytes)","{bytes}" << LogDiv() << bytes);
 log::Assert(targ->Invariant());	
 if (EOS()) return 0; // Below code won't manage it, so special cased.
 nat32 startPos = pos;
 
 // Iterate through reading as much as possible from the current node till we 
 // have obtained the order or run out of buffer to read...
  while (true)
  {
   nat32 ma = math::Min(bytes,blockSize-offset,targ->size-pos);
   
   mem::Copy(static_cast<byte*>(out),node->data+offset,ma);
   
   out = static_cast<byte*>(out) + ma;
   bytes -= ma;
   pos += ma;
   offset += ma;
   
   if (offset==blockSize)
   {
    if (node->next==null<Node*>()) break; // This if will only fire when EOS is obtained.
    node = node->next;
    offset = 0;
   } else break;
   if (bytes==0) break; // Rarely actually used, as the above break will ushally get it.
  }
 
 log::Assert(targ->Invariant());
 return pos-startPos;
}

nat32 Buffer::Cursor::Peek(void * out,nat32 bytes) const
{
 //LogBlock("nat32 Buffer::Cursor::Peek(void * out,nat32 bytes) const","{bytes}" << LogDiv() << bytes);
 log::Assert(targ->Invariant());
 if (EOS()) return 0; // Below code won't manage it, so special cased.
 nat32 targPos = pos;
 nat32 targOffset = offset;
 Buffer::Node * targNode = node;
 
 // Iterate through reading as much as possible from the current node till we 
 // have obtained the order or run out of buffer to read...
  while (true)
  {
   nat32 ma = math::Min(bytes,blockSize-targOffset,targ->size-targPos);
   
   mem::Copy(static_cast<byte*>(out),targNode->data+targOffset,ma);
   
   out = static_cast<byte*>(out) + ma;
   bytes -= ma;
   targPos += ma;
   targOffset += ma;
   
   if (targOffset==blockSize)
   {
    if (targNode->next==null<Node*>()) break; // This if will only fire when EOS is obtained.
    targNode = targNode->next;
    targOffset = 0;
   } else break;
   if (bytes==0) break; // Rarely actually used, as the above break will ushally get it.
  }
 
 log::Assert(targ->Invariant());
 return targPos-pos; 
}

nat32 Buffer::Cursor::Skip(nat32 bytes)
{
 Offset(bytes);
 return bytes;
}

nat32 Buffer::Cursor::Write(const void * in,nat32 bytes)
{
 //LogBlock("nat32 Buffer::Cursor::Write(const void * in,nat32 bytes)","{bytes}" << LogDiv() << bytes);
 log::Assert(targ->Invariant());
 if (bytes==0) return 0; // Extending would be the wrong behaviour in this case, so early break.
 
 // Extend nodes upto our current position, for when the user has created a
 // cursor beyond the end of the buffer and the new stuff needs filling in...
  while (offset>=blockSize)
  {
   if (node->next==null<Node*>())
   {
    node->next = new Node();
    node->next->next = null<Node*>();
    targ->last = node->next;
   }
   node = node->next;
   offset -= blockSize;
  }
 
 // We now have a start position, fill up nodes until we are out of data. If we
 // need we also create new nodes...
 nat32 startPos = pos;
  while (true)
  {
   nat32 ma = math::Min(bytes,blockSize-offset);
   
   mem::Copy(node->data+offset,static_cast<const byte*>(in),ma);
   
   in = static_cast<const byte*>(in) + ma;
   bytes -= ma;
   pos += ma;
   offset += ma;

   if (bytes==0) break;
   
   if (node->next==null<Node*>())
   {
    node->next = new Node();
    node->next->next = null<Node*>();
    targ->last = node->next;
   }
      
   node = node->next;
   offset = 0;
  }
  
  if (pos>targ->size) targ->size = pos;
 
 log::Assert(targ->Invariant());  
 return pos-startPos;
}

// This method is identical to the above, except the copy has been replaced with a null and
// the stuff concerned with in is terminated.
nat32 Buffer::Cursor::Pad(nat32 bytes)
{
 //LogBlock("nat32 Buffer::Cursor::Pad(nat32 bytes)","{bytes}" << LogDiv() << bytes);
 log::Assert(targ->Invariant());
 if (bytes==0) return 0; // Extending would be the wrong behaviour in this case, so early break.
 
 // Extend nodes upto our current position, for when the user has created a
 // cursor beyond the end of the buffer and the new stuff needs filling in...
  while (offset>=blockSize)
  {
   if (node->next==null<Node*>())
   {
    node->next = new Node();
    node->next->next = null<Node*>();
   }
   node = node->next;
   offset -= blockSize;
  }
 
 // We now have a start position, fill up nodes until we are out of data. If we
 // need we also create new nodes...
 nat32 startPos = pos;
  while (true)
  {
   nat32 ma = math::Min(bytes,blockSize-offset);
   
   mem::Null(node->data+offset,ma);
   
   bytes -= ma;
   pos += ma;
   offset += ma;
   
   if (bytes==0) break;
   
   if (node->next==null<Node*>())
   {
    node->next = new Node();
    node->next->next = null<Node*>();
   }
   
   node = node->next;
   offset = 0;
  }
  
  if (pos>targ->size) targ->size = pos;
  
 log::Assert(targ->Invariant());
 return pos-startPos;        
}

//------------------------------------------------------------------------------
 };
};
