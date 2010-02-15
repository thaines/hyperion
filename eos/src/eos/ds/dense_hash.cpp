//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines
#include "eos/ds/dense_hash.h"

#include "eos/mem/alloc.h"
#include "eos/mem/functions.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
DenseHashCode::DenseHashCode()
{
 for (nat32 i=0;i<256;i++) data[i] = 0;
}

DenseHashCode::DenseHashCode(const DenseHashCode & rhs)
{
 Copy(rhs);
}

DenseHashCode::~DenseHashCode()
{}

void DenseHashCode::Copy(const DenseHashCode & rhs)
{
 // Note: My brains gone all funny as a result of writting this.
 // I would strongly urge you to reconcider any attempts at deciphering,
 // reading or, god forbid, understanding this. And if it works first
 // time I'll eat something that dosn't taste too good. Such as a cat.
 void ***** const * inD = rhs.data;
 void ****** outD = data;
 for (nat32 i=0;i<256;i++)
 {
  if (*inD)
  {
   void ***** inC = *inD;
   *outD = mem::Malloc<void****>(64);
   void ***** outC = *outD;
   for (nat32 i=0;i<64;i++)
   {
    if (*inC)
    {
     void **** inB = *inC;
     *outC = mem::Malloc<void***>(64);
     void **** outB = *outC;
     for (nat32 i=0;i<64;i++)
     {
      if (*inB)
      {
       void *** inA = *inB;
       *outB = mem::Malloc<void**>(64);
       void *** outA = *outB;
       for (nat32 i=0;i<64;i++)
       {
         if (*inA)
         {
          *outA = mem::Malloc<void*>(64);
          mem::Copy<void*>(*outA,*inA,64);
         } else *outA = null<void**>();
        inA++;
        outA++;
       }
      } else *outB = null<void***>();
      inB++;
      outB++;
     }
    } else *outC = null<void****>();
    inC++;
    outC++;
   }
  } else *outD = null<void*****>();
  inD++;
  outD++;
 }
}

void DenseHashCode::Del(void (*Term)(void * ptr))
{
 void ****** targD = data;
 for (nat32 i=0;i<256;i++)
 {
  if (*targD)
  {
   void ***** targC = *targD;
   for (nat32 i=0;i<64;i++)
   {
    if (*targC)
    {
     void **** targB = *targC;
     for (nat32 i=0;i<64;i++)
     {
      if (*targB)
      {
       void *** targA = *targB;
       for (nat32 i=0;i<64;i++)
       {
         if (*targA)
         {
          void ** targ = *targA;
          for (nat32 i=0;i<64;i++)
          {
           if (*targ) Term(*targ);
           targ++;
          }
          mem::Free(*targA);
         }
        targA++;
       }
       mem::Free(*targB);
      }
      targB++;
     }
     mem::Free(*targC);
    }
    targC++;
   }
   mem::Free(*targD);
  }
  targD++;
 }
}

nat32 DenseHashCode::Size() const
{
 nat32 ret = 0;
 void ****** targD = const_cast<void ******>(data);
 for (nat32 i=0;i<256;i++)
 {
  if (*targD)
  {
   void ***** targC = *targD;
   for (nat32 i=0;i<64;i++)
   {
    if (*targC)
    {
     void **** targB = *targC;
     for (nat32 i=0;i<64;i++)
     {
      if (*targB)
      {
       void *** targA = *targB;
       for (nat32 i=0;i<64;i++)
       {
         if (*targA)
         {
          void ** targ = *targA;
          for (nat32 i=0;i<64;i++)
          {
           if (*targ) ++ret;
           targ++;
          }
         }
        targA++;
       }
      }
      targB++;
     }
    }
    targC++;
   }
  }
  targD++;
 }
 return ret;
}

nat32 DenseHashCode::Memory() const
{
 nat32 ret = sizeof(DenseHashCode);
 void ****** targD = const_cast<void ******>(data);
 for (nat32 i=0;i<256;i++)
 {
  if (*targD)
  {
   ret += sizeof(void****)*64;
   void ***** targC = *targD;
   for (nat32 i=0;i<64;i++)
   {
    if (*targC)
    {
     ret += sizeof(void***)*64;
     void **** targB = *targC;
     for (nat32 i=0;i<64;i++)
     {
      if (*targB)
      {
       ret += sizeof(void**)*64;
       void *** targA = *targB;
       for (nat32 i=0;i<64;i++)
       {
        if (*targA)
        {
         ret += sizeof(void*)*64;
        }
        targA++;
       }
      }
      targB++;
     }
    }
    targC++;
   }
  }
  targD++;
 }
 return ret;
}

bit DenseHashCode::Exists(nat32 index) const
{
 nat8 offset[5];
  offset[0] = index>>24;
  void ***** a = data[offset[0]]; if (a==0) return false;
  offset[1] = (index>>18)&0x3F;
  void **** b = a[offset[1]]; if (b==0) return false;
  offset[2] = (index>>12)&0x3F;
  void *** c = b[offset[2]]; if (c==0) return false;
  offset[3] = (index>>6)&0x3F;
  void ** d = c[offset[3]]; if (d==0) return false;
  offset[4] = index&0x3F;
  void * e = d[offset[4]];

 return e!=0;
}

void *& DenseHashCode::Get(nat32 index)
{
 nat8 offset[5];
  offset[0] = index>>24;
  void *****& a = data[offset[0]];
  if (a==0)
  {
   a = mem::Malloc<void****>(64);
   mem::Null(a,64);
  }

  offset[1] = (index>>18)&0x3F;
  void ****& b = a[offset[1]];
  if (b==0)
  {
   b = mem::Malloc<void***>(64);
   mem::Null(b,64);
  }

  offset[2] = (index>>12)&0x3F;
  void ***& c = b[offset[2]];
  if (c==0)
  {
   c = mem::Malloc<void**>(64);
   mem::Null(c,64);
  }

  offset[3] = (index>>6)&0x3F;
  void **& d = c[offset[3]];
  if (d==0)
  {
   d = mem::Malloc<void*>(64);
   mem::Null(d,64);
  }

  offset[4] = index&0x3F;
 return d[offset[4]];
}

void DenseHashCode::Optimise()
{
 // This goes through and deletes any branches of the tree that only points to
 // nulls. And yes, the code is an absolute bitch to understand...
  void ****** targD = data;
  for (nat32 i=0;i<256;i++)
  {
   if (*targD)
   {
    bit delD = true;
    void ***** targC = *targD;
    for (nat32 i=0;i<64;i++)
    {
     if (*targC)
     {
      bit delC = true;
      void **** targB = *targC;
      for (nat32 i=0;i<64;i++)
      {
       if (*targB)
       {
       	bit delB = true;
        void *** targA = *targB;
        for (nat32 i=0;i<64;i++)
        {
          if (*targA)
          {
           bit delA = true;
           void ** targ = *targA;
           for (nat32 i=0;i<64;i++)
           {
            if (*targ) {delA = false; break;}
            targ++;
           }
           if (delA)
           {
            mem::Free(*targA);
            *targA = 0;
           } else delB = false;
          }
         targA++;
        }
        if (delB)
        {
         mem::Free(*targB);
         *targB = 0;
        } else delC = false;
       }
       targB++;
      }
      if (delC)
      {
       mem::Free(*targC);
       *targC = 0;
      } else delD = false;
     }
     targC++;
    }
    if (delD)
    {
     mem::Free(*targD);
     *targD = 0;
    }
   }
   targD++;
  }
}

void DenseHashCode::Clean(nat32 index)
{
 // This checks if the index in question can be 'cleaned'.
 // A sniper rifle is the prefered techneque, however a really
 // hard hit on the head with a broom has been known to work...

 // Work down the chain to our destination...
  nat8 offset[4];
   offset[0] = index>>24;
   void ***** a = data[offset[0]]; if (a==0) return;
   offset[1] = (index>>18)&0x3F;
   void **** b = a[offset[1]]; if (b==0) return;
   offset[2] = (index>>12)&0x3F;
   void *** c = b[offset[2]]; if (c==0) return;
   offset[3] = (index>>6)&0x3F;
   void ** d = c[offset[3]]; if (d==0) return;

  // Now work up the chain deleting data as we go if possible, once we find some
  // live data we have to stop, as we can't delete any more.
   // Level C...
    for (nat32 i=0;i<64;i++)
    {
     if (*d) return;
     d++;
    }
    mem::Free(c[offset[3]]);
    c[offset[3]] = 0;

   // Level B...
    for (nat32 i=0;i<64;i++)
    {
     if (*c) return;
     c++;
    }
    mem::Free(b[offset[2]]);
    b[offset[2]] = 0;

   // Level A...
    for (nat32 i=0;i<64;i++)
    {
     if (*b) return;
     b++;
    }
    mem::Free(a[offset[1]]);
    a[offset[1]] = 0;

   // The unholy level...
    for (nat32 i=0;i<64;i++)
    {
     if (*a) return;
     a++;
    }
    mem::Free(data[offset[0]]);
    data[offset[0]] = 0;
}

//------------------------------------------------------------------------------
 };
};
