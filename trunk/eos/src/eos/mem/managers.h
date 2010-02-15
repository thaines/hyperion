#ifndef EOS_MEM_MANAGERS_H
#define EOS_MEM_MANAGERS_H
//------------------------------------------------------------------------------
// Copyright 2005 Tom Haines

/// \file managers.h
/// Contains memory managers, in the form of shared object mangers and 
/// object factorys.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/str/tokens.h"
#include "eos/ds/sort_lists.h"

namespace eos
{
 namespace mem
 {
//------------------------------------------------------------------------------
// Suport class so the below can be as flat as a pancake. And twice as tasty.
class EOS_CLASS ObjectShareCode
{
 protected:
   ObjectShareCode(str::TokenTable & tokTab):tt(tokTab) {}
  ~ObjectShareCode();	




 str::TokenTable & tt;
};

//------------------------------------------------------------------------------
/// This class allows you to share objects between parts of a system, you provide
/// a type and 'object creator' (Ushally code to load a particular file type) to
/// the templated type, you can then request an object by name, if its allready
/// been run through the loader then its returned, otherwise its passed to the
/// loader which will do its thing. A typical example of using this would be
/// for loading textures into a computer game. Objects are not deleted untill
/// they are all returned to the system, and then only when the garbage collector
/// is called. You allways inherit from this class to provide it with the extra 
/// functionality required.
template <typename T>
class EOS_CLASS ObjectShare : public ObjectShareCode
{
 public:
  /// &nbsp;
   ObjectShare(str::TokenTable & tokTab):ObjectShareCode(tokTab) {}

  /// &nbsp;
   ~ObjectShare() {}


  /// Returns a reference to the shared TokenTable, which is used to encode 
  /// all the resource identifiers.
   str::TokenTable & GetTT() const {return tt;}


  /// Returns an object, using make to create it only if neccesary. When done with
  /// it call Release. It can return null if Make fails for whatever reason.
   T * Acquire(str::Token tok);

  /// Returns true if the object is allready loaded, and it is going to take minimal
  /// time for a call to Acquire to complete.
   bit Avaliable(str::Token tok);

  /// When you are done with an object call this to indicate your disinterest. If it
  /// realises it now has no friends it will commit suicide the next time the dustman
  /// calls.
   void Release(T * obj);


  /// Deletes any objets with a reference count of 0. The poor bastards.
  /// You can provide a limit of how many objects it calls Destroy(..) on
  /// before stopping, if set to 0 it will clean all. This allows you to
  /// force a fast return for this method.
   void GC(nat32 lim = 0);


  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::mem::ObjectShare<" << typestring<T>() << ">");
    return ret;
   }


 protected:
  /// You provide an implimentation of this to load the objects in question.
   virtual T * Make(cstrconst name) = 0;
   
  /// You provide an implimentation of this to kill the objects in question.
   virtual void Destroy(T * obj) = 0;
};

//------------------------------------------------------------------------------
/// An object factory, provides the indexing to find the relevent construction
/// methods, an implimentor inherits from this object and provides a function for
/// each type. The registering of types is protected, a child can however change
/// that easily enough. T is the type of object it returns whilst PT is the pass
/// through type which is provided to all functions so they can make the object
/// in question.
template <typename T,typename PT>
class EOS_CLASS ObjectFactory
{
 public:
  /// &nbsp;
   ObjectFactory(str::TokenTable & tokTab):tt(tokTab) {}
  
  /// &nbsp;
   ~ObjectFactory() {}
   
  /// Returns a reference to the shared TokenTable, which is used to encode 
  /// all the resource identifiers.
   str::TokenTable & GetTT() const {return tt;}


  /// Returns a new instance of the object in question. If no handler is avaliable
  /// it returns null.
   T * Make(str::Token name) const
   {
    Node dummy; dummy.name = name;
    Node * targ = data.Get(dummy);
    if (targ) return (targ->F)(name,targ->pt);
         else return null<T*>();
   }

  /// Returns a new instance of the object in question. If no handler is avaliable
  /// it returns null.
   T * Make(cstrconst name) const
   {
    return Make(tt(name));
   }
   
  /// Returns true if it has a handler for the given object, and can therefore 
  /// correctly handle a Make with the same token.
   bit Avaliable(str::Token name) const
   {
    Node dummy; dummy.name = name;
    return (data.Get(dummy)!=null<T*>());
   }

  /// Returns true if it has a handler for the given object, and can therefore 
  /// correctly handle a Make with the same token.
   bit Avaliable(cstrconst name) const
   {
    return Avaliable(tt(name));
   }


  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::mem::ObjectFactory<" << typestring<T>() << ">");
    return ret;
   }


 protected:
  /// &nbsp;
   str::TokenTable & tt;
 
  /// Allows you to register an object construction function. Note that the pt
  /// given to this method is copied and stored within, so it better support that.
  /// (It will ushally be a pointer, so no problem there.)
  /// If the name is allready registered it will just be overwritten.
   void Register(str::Token name,T * (*F)(str::Token name,PT pt),PT pt)
   {
    Node nn;
     nn.name = name;
     nn.F = F;
     nn.pt = pt;
    data.Add(nn);
   }
   
  /// Allows you to remove a registered object.
   void Unregister(str::Token name)
   {
    Node dummy;
     dummy.name = name;
    data.Rem(dummy);
   }

 
 private:
  struct Node
  {
   bit operator < (const Node & rhs) const {return name<rhs.name;}
   
   str::Token name;
   T * (*F)(str::Token name,PT pt);
   PT pt;
  };
 
  ds::SortList<Node> data;
};

//------------------------------------------------------------------------------
 };
};
#endif
