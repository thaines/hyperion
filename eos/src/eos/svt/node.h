#ifndef EOS_SVT_NODE_H
#define EOS_SVT_NODE_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file node.h
/// The root type of the svt hierachy.

#include "eos/types.h"
#include "eos/svt/core.h"

namespace eos
{
 namespace svt
 {
//------------------------------------------------------------------------------
/// The root type of the hierachy system, provides features shared by all
/// nodes, specifically access to the core and the hierachy they form. Note
/// that children form a circular list, so don't attempt any iterating till null.
/// (do {... targ = targ->Next();} while (!targ->First()) should be used to iterate all.)
/// See the \link svt_io SVT IO page \endlink for details on the results of
/// IO with this class.
class EOS_CLASS Node : public Deletable
{
 public:
  /// &nbsp;
   Node(Core & c)
   :core(c),parent(null<Node*>()),child(null<Node*>()),next(this),last(this) 
   {}
   
  // This rather unushal constructor takes a pointer to another Node, 
  // shallow copys it and then emptys the passed in object such that it is empty.
  // This makes it quite efficient, as it only has to alter a bunch of pointers.
  // Undocumented as not really for direct use.
   Node(Node * victim);

  /// Deleting a Node will detach it from its parent and delete all its children, leaving
  /// its parent and siblings alive.
   ~Node() {Detach(); KillChildren();}

  
  /// &nbsp;
   Core & GetCore() const {return core;}
   
  /// &nbsp;
   nat32 ChildCount() const;


  /// Returns the parent of the Node, or null if the Node has no parent.
   Node * Parent() const {return parent;}

  /// Returns the first child of the Node, or null if the node has no children.
   Node * Child() const {return child;}

  /// Returns the next sibling, who shares the same parent, or null if none
  /// exists.
   Node * Next() const {return next;}

  /// Returns the previosu sibling, who shares the same parent, or null if none
  /// exists.
   Node * Last() const {return last;}


  /// Returns true if this is the first child of its parent, used to iterate all
  /// nodes.
   bit First() const
   {
    if (parent) return parent->child==this;
           else return true;
   }

  /// Removes itself and its children from connected nodes, so Parent(),Next() and 
  /// Last() all return null. Children will remain, use KillChildren() to remove
  /// perminantly or Detach() them each in turn, if you don't like them.
   void Detach();

  /// Makes the Node kill all of its children. You don't even have to ask twice.
   void KillChildren();


  /// Attaches the node, detaching it if neccesary, such that the given node it 
  /// its parent. If the parent allready has children it will become the last 
  /// child.
   void AttachParent(Node * parent);

  /// Attaches the given node as the child of this node, making it the first 
  /// node out of any children.
   void AttachChild(Node * child);   

  /// Detaches this node and moves it so its parent is the same as the given Node
  /// and it appears before the node in the sibling list.
   void AttachBefore(Node * next);

  /// Detaches this node and moves it so it is after the given node in the sibling
  /// list with the parent of the given node.
   void AttachAfter(Node * last);


  /// The Memory() method I am so fond of is made virtual here, so size can
  /// be determined. Only includes current node, to get size of object plus 
  /// children the TotalMemory() method is provided.
   virtual nat32 Memory() const {return sizeof(Node);}

  /// This extends Memory() to include children as well, so the size of an 
  /// entire structure can be determined.
   nat32 TotalMemory() const;
   
  /// Similar to Memory(), this returns how many bytes the object would consume if 
  /// serialised, including the BOF header.
   nat32 WriteSize() const;
   
  /// This is a virtual version of WriteSize, so the correct version is called.
  /// Both versions are required to get header sizes right when writting.
   virtual nat32 TotalWriteSize() const;


  /// Supports the write method, returns the 3 character magic string for this object.
   virtual cstrconst MagicString() const;
  
  /// This Writes the object, including parent blocks, to a stream.
  /// Returns how many bytes have been written.
   virtual nat32 Write(io::OutVirt<io::Binary> & out) const;
   
  /// This reads from the given stream into this object, assuming this object to 
  /// be 'just constructed' and empty. Does not call any parent type Read's, 
  /// i.e. does the block alone.
  /// (Not virtual, so the parent Read's methods can be called, and you should
  /// dam well know the type anyway as it has just been constructed.)
   void ReadBlock(io::InVirt<io::Binary> & in);
   

  /// &nbsp;
   inline virtual cstrconst TypeString() const {return "eos::svt::Node";};


 protected:
  Core & core;

 private:
  Node * parent;
  Node * child;
  Node * next;
  Node * last;  
};

//------------------------------------------------------------------------------
 };
};
#endif
