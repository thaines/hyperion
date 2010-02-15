//------------------------------------------------------------------------------
// Copyright 2004 Tom Haines
#include "eos/ds/arrays.h"

#include "eos/math/functions.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
ArrayCode::ArrayCode(nat32 elementSize,const ArrayCode & rhs)
:elements(rhs.elements),data(mem::Malloc<byte>(elementSize*rhs.elements))
{
 mem::Copy<byte>(data,rhs.data,elementSize*elements);
}

void ArrayCode::Del(nat32 elementSize,void (*Term)(void * ptr))
{
 byte * targ = data;
 for (nat32 i=0;i<elements;i++)
 {
  Term(targ);
  targ += elementSize;	 
 }	
}

void ArrayCode::Copy(nat32 elementSize,const ArrayCode & rhs)
{
 if (elements!=rhs.elements)
 {
  elements = rhs.elements;
  mem::Free(data);
  data = mem::Malloc<byte>(elementSize*elements);	 
 }

 mem::Copy<byte>(data,rhs.data,elementSize*elements);
}

nat32 ArrayCode::Resize(nat32 elementSize,nat32 newSize,void (*Make)(void * ptr),void (*Term)(void * ptr))
{
 if (elements!=newSize)
 {
  byte * newData = mem::Malloc<byte>(elementSize*newSize);
  mem::Copy(newData,data,elementSize*math::Min(elements,newSize));
  for (nat32 i=newSize;i<elements;i++) Term(Item(elementSize,i));
  mem::Free(data);
  data = newData;
  for (nat32 i=elements;i<newSize;i++) Make(Item(elementSize,i));
  elements = newSize;
 }
 return elements;	
}

nat32 ArrayCode::Rebuild(nat32 elementSize,nat32 newSize,void (*Make)(void * ptr))
{
 elements = newSize;
 mem::Free(data);
 data = mem::Malloc<byte>(elementSize*elements);
 for (nat32 i=0;i<elements;i++) Make(Item(elementSize,i));
 return elements;
}

void ArrayCode::SortRange(nat32 elementSize,bit (*LessThan)(void * lhs,void * rhs),int32 left,int32 right,byte * temp)
{
 // The below code comes almost directly out of Weiss:-)
  if (left+10<=right)
  {
   // This is a quicksort...
    // First setup the pivot so it is in position right-1...
     int32 center = (left+right)/2;
     if (LessThan(Item(elementSize,center),Item(elementSize,left))) mem::Swap(Item(elementSize,center),Item(elementSize,left),elementSize);
     if (LessThan(Item(elementSize,right),Item(elementSize,left))) mem::Swap(Item(elementSize,right),Item(elementSize,left),elementSize);
     if (LessThan(Item(elementSize,right),Item(elementSize,center))) mem::Swap(Item(elementSize,right),Item(elementSize,center),elementSize);

     mem::Swap(Item(elementSize,right-1),Item(elementSize,center),elementSize);

    // Now for the actual quicksort code...
     int32 i = left;
     int32 j = right - 1;
     while (true)
     {
      while (LessThan(Item(elementSize,++i),Item(elementSize,right-1))) {}
      while (LessThan(Item(elementSize,right-1),Item(elementSize,--j))) {}

      if (i<j) mem::Swap(Item(elementSize,i),Item(elementSize,j),elementSize);
          else break;
     }

    // Return the pivot to the center...
     mem::Swap(Item(elementSize,i),Item(elementSize,right-1),elementSize);

    // And now for the recursive step...
     SortRange(elementSize,LessThan,left,i-1,temp);
     SortRange(elementSize,LessThan,i+1,right,temp);
  }
  else
  {
   // This is an insertion sort...    
    for (int32 p = left+1;p<=right;p++)
    {
     mem::Copy(temp,Item(elementSize,p),elementSize);
     int32 j;
     for (j=p;(j>left)&&LessThan(temp,Item(elementSize,j-1));j--){}
     if (j!=p)
     {
      mem::Move(Item(elementSize,j+1),Item(elementSize,j),elementSize*(p-j));
      mem::Copy(Item(elementSize,j),temp,elementSize);
     }
    }
  }
}

void ArrayCode::SortRange(nat32 elementSize,bit (*LessThan)(void * lhs,void * rhs,void * ptr),int32 left,int32 right,void * ptr,byte * temp)
{
 // The below code comes almost directly out of Weiss:-)
  if (left+10<=right)
  {
   // This is a quicksort...
    // First setup the pivot so it is in position right-1...
     int32 center = (left+right)/2;
     if (LessThan(Item(elementSize,center),Item(elementSize,left),ptr)) mem::Swap(Item(elementSize,center),Item(elementSize,left),elementSize);
     if (LessThan(Item(elementSize,right),Item(elementSize,left),ptr)) mem::Swap(Item(elementSize,right),Item(elementSize,left),elementSize);
     if (LessThan(Item(elementSize,right),Item(elementSize,center),ptr)) mem::Swap(Item(elementSize,right),Item(elementSize,center),elementSize);

     mem::Swap(Item(elementSize,right-1),Item(elementSize,center),elementSize);

    // Now for the actual quicksort code...
     int32 i = left;
     int32 j = right - 1;
     while (true)
     {
      while (LessThan(Item(elementSize,++i),Item(elementSize,right-1),ptr)) {}
      while (LessThan(Item(elementSize,right-1),Item(elementSize,--j),ptr)) {}

      if (i<j) mem::Swap(Item(elementSize,i),Item(elementSize,j),elementSize);
          else break;
     }

    // Return the pivot to the center...
     mem::Swap(Item(elementSize,i),Item(elementSize,right-1),elementSize);

    // And now for the recursive step...
     SortRange(elementSize,LessThan,left,i-1,ptr,temp);
     SortRange(elementSize,LessThan,i+1,right,ptr,temp);
  }
  else
  {
   // This is an insertion sort...
    for (int32 p = left+1;p<=right;p++)
    {
     mem::Copy(temp,Item(elementSize,p),elementSize);
     int32 j;
     for (j=p;(j>left)&&LessThan(temp,Item(elementSize,j-1),ptr);j--){}
     if (j!=p)
     {
      mem::Move(Item(elementSize,j+1),Item(elementSize,j),elementSize*(p-j));
      mem::Copy(Item(elementSize,j),temp,elementSize);
     }
    }
  }
}

int32 ArrayCode::Search(nat32 elementSize,bit (*LessThan)(void * lhs,void * rhs),nat32 left,nat32 right,byte * target) const
{
 if ((!LessThan(Item(elementSize,left),target))&&(!LessThan(target,Item(elementSize,left)))) return left;
 if ((!LessThan(Item(elementSize,right),target))&&(!LessThan(target,Item(elementSize,right)))) return left;

 if (left+1>=right) return -1;
 nat32 center = (left + right)/2;

 if (LessThan(Item(elementSize,center),target)) return Search(elementSize,LessThan,center+1,right-1,target);
 else if (LessThan(target,Item(elementSize,center))) return Search(elementSize,LessThan,left+1,center-1,target);
 else return center;
}

nat32 ArrayCode::SearchLargest(nat32 elementSize,bit (*LessThan)(void * lhs,void * rhs),nat32 left,nat32 right,byte * target) const
{
 if (LessThan(Item(elementSize,right),target)) return right;
 if (left+1>=right) return left;
 
 nat32 center = (left + right)/2;
 if (LessThan(Item(elementSize,center),target)) return SearchLargest(elementSize,LessThan,center+1,right-1,target);
 else if (LessThan(target,Item(elementSize,center))) return SearchLargest(elementSize,LessThan,left+1,center-1,target);
 else return center;
}

//------------------------------------------------------------------------------
 };
};
