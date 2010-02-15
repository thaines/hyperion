#ifndef EOS_DS_COLLECTORS_H
#define EOS_DS_COLLECTORS_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file collectors.h
/// Simple data structure for collecting data of some kind, designed for speed
/// by using the mem::Packer memory manager.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/ds/arrays.h"
#include "eos/mem/packer.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
// Code for flat template...
class EOS_CLASS CollectorCode
{
 protected:
   CollectorCode(nat32 elementSize,nat32 blockSize);
  ~CollectorCode();
  
  void Add(nat32 elementSize,const void * data);
  void Fill(nat32 elementSize,ArrayCode & out) const;
  
  void Reverse();


 // Variables...
  nat32 size;
  mem::Packer packer;
 
  struct Node
  {
   Node * next;
   void * Data() {return (void*)(this+1);}
  };
  Node * last;
  
 private:
  static void NoOp(void * ptr)
  {}
};

//------------------------------------------------------------------------------
/// Fairly simple class for collecting data records with.
/// Designed for speed and large data sets, it uses a mem::Packer for allocations.
/// Once collected the only function is to then spit the data into a normal
/// array - it really is only for the collecting phase. Its trick is in using 
/// the Packer to avoid doing a malloc for each new item like a normal linked
/// list would have to.
/// The contained records must be simple - this does not suport constructors/destructors.
template <typename T>
class EOS_CLASS Collector : public CollectorCode
{
 public:
  /// On construction you must provide the number of records to allocate at a
  /// time - the implicit size of memory blocks requested.
   Collector(nat32 blockSize = 256):CollectorCode(sizeof(T),blockSize) {}
   
  /// &nbsp;
   ~Collector() {}

  /// &nbsp;
   void Reset() {size = 0; packer.Reset(); last = null<Node*>();}


  /// &nbsp;
   void Add(const T & rhs) {CollectorCode::Add(sizeof(T),&rhs);}
   
  /// &nbsp;
   nat32 Size() const {return size;}
   
  /// Will resize the array as needed.
  /// The data will be in the order added.
   void Fill(ds::Array<T> & out) const {CollectorCode::Fill(sizeof(T),out);}
   
   
  /// Cursor class, allows you to iterate the stored structures, in reverse order.
   class EOS_CLASS Cursor
   {
    public:
     // Not for public consumption...
      Cursor(Node * start):targ(start) {}
    
     /// &nbsp;
      Cursor():targ(null<Node*>()) {}
      
     /// &nbsp;
      Cursor(const Cursor & rhs):targ(rhs.targ) {}
      
     /// &nbsp;
      ~Cursor() {}
    
     /// &nbsp;
      Cursor & operator = (const Cursor & rhs) {targ = rhs.targ; return *this;}

     
     /// Returns true if the class is unsafe to use.
      bit Bad() const {return targ==null<Node*>();}

     /// Returns true if the class is safe to use.
      bit Good() const {return targ!=null<Node*>();}

     /// Moves to next item.
      Cursor & operator++ () {targ = targ->next; return *this;}
     
     
     /// &nbsp;
      T * Ptr() {return (T*)targ->Data();} 

     /// &nbsp;
      const T * Ptr() const {return (T*)targ->Data();} 

     /// &nbsp;
      T & operator* () {return *(T*)targ->Data();}

     /// &nbsp;
      const T & operator* () const {return *(T*)targ->Data();}
         
     /// &nbsp;
      T * operator->() {return (T*)targ->Data();}
  
     /// &nbsp;
      const T * operator->() const {return (T*)targ->Data();}

     
    private:
     Node * targ;
   };
   
   
  /// Returns a cursor to the most recent added item, can only use the cursor 
  /// to head backwards as the data structure is one way only.
   Cursor Ptr() const {return Cursor(last);}
   
  /// The iteration system will iterate the objects in reverse order, this 
  /// is not so good for cache coherance, so it can be worth the effort of
  /// calling the below, as it will reverse the stored order such that iteration
  /// is in added order. Can be called to reverse the order whenever the user 
  /// chooses.
   void Reverse() {CollectorCode::Reverse();}
   
   
  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::ds::Collector<" << typestring<T>() << ">");
    return ret;
   }   
};

//------------------------------------------------------------------------------
 };
};
#endif
