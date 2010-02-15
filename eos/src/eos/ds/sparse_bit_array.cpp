//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/ds/sparse_bit_array.h"

#include "eos/file/csv.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
SparseBitArray::SparseBitArray()
:size(0)
{}

SparseBitArray::~SparseBitArray()
{}

void SparseBitArray::Reset()
{
 data.Reset();
 size = 0;
}

void SparseBitArray::Set(nat32 index)
{
 nat32 offset[6];
 
 offset[0] = index>>26;
 Split64< Split64< Split64<Bit256> > > *& targ0 = data.ptr[offset[0]];
 if (targ0==0) targ0 = new Split64< Split64< Split64<Bit256> > >();
 
 offset[1] = (index>>20)&0x3F;
 Split64< Split64<Bit256> > *& targ1 = targ0->ptr[offset[1]];
 if (targ1==0) targ1 = new Split64< Split64<Bit256> >();
 
 offset[2] = (index>>14)&0x3F;
 Split64<Bit256> *& targ2 = targ1->ptr[offset[2]];
 if (targ2==0) targ2 = new Split64<Bit256>();
 
 offset[3] = (index>>8)&0x3F;
 Bit256 *& targ3 = targ2->ptr[offset[3]];
 if (targ3==0) targ3 = new Bit256();
 
 offset[4] = (index>>5)&0x07;
 offset[5] = index&0x1F; 
 
 if (!((targ3->n[offset[4]]>>offset[5])&1))
 {
  size += 1;
  targ3->n[offset[4]] ^= 1<<offset[5];
 }
}

void SparseBitArray::Unset(nat32 index)
{
 nat32 offset[6];
 
 offset[0] = index>>26;
 Split64< Split64< Split64<Bit256> > > * targ0 = data.ptr[offset[0]];
 if (targ0==0) return;
 
 offset[1] = (index>>20)&0x3F;
 Split64< Split64<Bit256> > * targ1 = targ0->ptr[offset[1]];
 if (targ1==0) return;
 
 offset[2] = (index>>14)&0x3F;
 Split64<Bit256> * targ2 = targ1->ptr[offset[2]];
 if (targ2==0) return;
 
 offset[3] = (index>>8)&0x3F;
 Bit256 * targ3 = targ2->ptr[offset[3]];
 if (targ3==0) return; 
 
 offset[4] = (index>>5)&0x07;
 offset[5] = index&0x1F; 
 
 if (targ3->n[offset[4]]>>offset[5])
 {
  size -= 1;
  targ3->n[offset[4]] ^= 1<<offset[5];
 }
}

bit SparseBitArray::operator[](nat32 index) const
{
 nat32 offset[6];
 
 offset[0] = index>>26;
 Split64< Split64< Split64<Bit256> > > * targ0 = data.ptr[offset[0]];
 if (targ0==0) return false;
 
 offset[1] = (index>>20)&0x3F;
 Split64< Split64<Bit256> > * targ1 = targ0->ptr[offset[1]];
 if (targ1==0) return false;
 
 offset[2] = (index>>14)&0x3F;
 Split64<Bit256> * targ2 = targ1->ptr[offset[2]];
 if (targ2==0) return false;
 
 offset[3] = (index>>8)&0x3F;
 Bit256 * targ3 = targ2->ptr[offset[3]];
 if (targ3==0) return false; 
 
 offset[4] = (index>>5)&0x07;
 offset[5] = index&0x1F; 
 return bit(targ3->n[offset[4]]>>offset[5]);
}

nat32 SparseBitArray::Size() const
{
 return size;
}

bit SparseBitArray::NextInc(nat32 & index) const
{
 nat32 offset[5];
 offset[0] = index>>26;
 offset[1] = (index>>20)&0x3F;
 offset[2] = (index>>14)&0x3F;
 offset[3] = (index>>8)&0x3F;
 offset[4] = index&0xFF;

 for (nat32 i0=offset[0];i0<64;i0++)
 {
  Split64< Split64< Split64<Bit256> > > * targ0 = data.ptr[i0];
  if (targ0)
  {
   for (nat32 i1=offset[1];i1<64;i1++)
   {
    Split64< Split64<Bit256> > * targ1 = targ0->ptr[i1];
    if (targ1)
    {
     for (nat32 i2=offset[2];i2<64;i2++)
     {
      Split64<Bit256> * targ2 = targ1->ptr[i2];
      if (targ2)
      {
       for (nat32 i3=offset[3];i3<64;i3++)
       {
        Bit256 * targ3 = targ2->ptr[i3]; 
        if (targ3)
        {
         // We have a 256 bit block to check, and an offset to check from...          
          bit ret = targ3->NextInc(offset[4]);
          if (ret)
          {
           index = (i0<<26) | (i1<<20) | (i2<<14) | (i3<<8) | offset[4];
           return true;
          }
        }
        offset[4] = 0;
       }
      }
      offset[3] = 0;
     }
    }
    offset[2] = 0;
   }
  }
  offset[1] = 0;
 }
 return false;
}

//------------------------------------------------------------------------------
// Helper for below...
bit NextIncByte(byte b,nat32 & index)
{
 if ((b&0x0F)&&(index<4))
 {
  if ((b&0x03)&&(index<2))
  {
   if ((b&0x01)&&(index==0))
   {
    return true;
   }
   if (b&0x02)
   {
    index = 1;
    return true;
   }
  }
  if (b&0x0C)
  {
   if ((b&0x04)&&(index<3))
   {
    index = 2;
    return true;
   }
   if (b&0x08)
   {
    index = 3;
    return true;
   }
  }
 }
 if (b&0xF0)
 {
  if ((b&0x30)&&(index<6))
  {
   if ((b&0x10)&&(index<5))
   {
    index = 4;
    return true;
   }
   if (b&0x20)
   {
    index = 5;
    return true;
   }
  }
  if (b&0xC0)
  {
   if ((b&0x40)&&(index<7))
   {
    index = 6;
    return true;
   }
   if (b&0x80)
   {
    index = 7;
    return true;
   }
  }
 }
 return false;
}

bit SparseBitArray::Bit256::NextInc(nat32 & index) const
{
 nat32 offset = index&0x1F;
 for (nat32 i=(index>>5);i<8;i++)
 {
  if ((n[i]&0x0000FFFF)&&(offset<16))
  {
   if ((n[i]&0x000000FF)&&(offset<8))
   {
    bit ret = NextIncByte(byte(n[i]&0xFF),offset);
    if (ret)
    {
     index = (i<<5) | offset;
     return true;
    }
   }
   if (n[i]&0x0000FF00)
   {
    nat32 temp;
    if (offset>8) temp = offset - 8;
             else temp = 0;
    bit ret = NextIncByte(byte((n[i]>>8)&0xFF),temp);
    if (ret)
    {
     index = (i<<5) | (temp+8);
     return true;
    }    
   }
  }
  if (n[i]&0xFFFF0000)
  {
   if ((n[i]&0x00FF0000)&&(offset<24))
   {
    nat32 temp;
    if (offset>16) temp = offset - 16;
              else temp = 0;
    bit ret = NextIncByte(byte((n[i]>>16)&0xFF),temp);
    if (ret)
    {
     index = (i<<5) | (temp+16);
     return true;
    }   
   }
   if (n[i]&0xFF000000)
   {
    nat32 temp;
    if (offset>24) temp = offset - 24;
              else temp = 0;
    bit ret = NextIncByte(byte((n[i]>>24)&0xFF),temp);
    if (ret)
    {
     index = (i<<5) | (temp+24);
     return true;
    }
   }
  }

  offset = 0;
 }

 return false;
}

//------------------------------------------------------------------------------
 };
};
