#ifndef EOS_SVT_CORE_H
#define EOS_SVT_CORE_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


/// \namespace eos::svt
/// The Single Variable Type system aims to provide a data structure which can
/// be used for all variables in complicated systems. Essentially the svt 
/// namespace provides the ability to create and edit a sophisticated database
/// of data, where the majority is in the form of multi-dimensional arrays of
/// details. At its centre is the Core, which simply makes sure all the objects 
/// are using the same token table, and that the there are no Type clashes. A
/// svt hierachy can then be formed from a set of objects, all of which derive
/// from Node. A Branch is ushally at the Root, and is simply a list of Nodes,
/// a Leaf is simply data, in key to data form for storing individual values etc.
/// The most important class is Var, which inherits from Leaf and uses it for meta
/// data, but ultimatly consists of a multi-dimensional array of many fields. To
/// access a field a Field object is requested from a Var, which caches all the 
/// details needed to find it so as to accelerate accessing them. In addition to 
/// all this a Type object is provided, which can specify a set of constraints for
/// a Var to be a member of that type, you can then test Var objects for membership.

/// \file core.h
/// Contains the central object which manages a svt database.

#include "eos/types.h"
#include "eos/io/inout.h"
#include "eos/str/tokens.h"
#include "eos/ds/sparse_hash.h"
#include "eos/ds/sort_lists.h"

namespace eos
{
 namespace svt
 {
//------------------------------------------------------------------------------
// The Type class is used below, use a pre-decleration as its error city otherwise...
class Type;

// Ditto on the node class.
class Node;

//------------------------------------------------------------------------------
/// The Core of the SVT system, almost every object within it has a pointer to 
/// this. It provide a database of types and access to a central token table,
/// so the tokens can be compared between all parts of the system. It is 
/// responsable for the creation and deletion of types, so there is no chance
/// of conflicts.
/// It additionally suports IO, with each object type suported by the system 
/// registered here. (The default Node/Meta/Var chain is included by default.)
/// It essentially acts like an object factory when loading SVT objects.
class EOS_CLASS Core
{
 public:
  /// &nbsp;
   Core(str::TokenTable & t);

  /// &nbsp;
   ~Core();

  /// Returns a reference to the shared TokenTable, which is used to encode 
  /// all keys for data arround the system.
   str::TokenTable & GetTT() const {return tt;}


  /// Allocates and returns a new type, returns null if it allready exists.
   Type * NewType(str::Token name);

  /// Returns a type, or null if it doesn;t exist.
   Type * GetType(str::Token name);

  /// Tests if a node matches a type, returns true if it is of that type, 
  /// false if it is not. If given a type that does not exist it returns 
  /// true, on the grounds that not existing means any data will do.
   bit IsType(str::Token name,Node * node);


  /// Loads an object from a file, returning the new Node. 
  /// Handles loading of child types correctly, it also does fallback if
  /// the type is unrecognised to the best it can manage given the types 
  /// it knows about. Expects the stream to start at the beginning of 
  /// the head of the first block of the object, will leave the stream 
  /// at the end of the object.
   Node * LoadObject(io::InVirt<io::Binary> & in);
   
  /// Registers a type so the system can load it, all types have to 
  /// inherit from Node.
  /// \param magic The 3 character magic string that represents the object type.
  /// \param parent The 3 character magic string of its parent type, or null if
  ///               it has no parent. You should never pass null, as the automatically
  ///               registered Node is the only type without a parent.
  /// \param Create Constructs a new instance of the object. It is optionally handed 
  ///               an instance of the parent type of the type it is to create, if this
  ///               is given it is to replace and delete its parent, such that it has 
  ///               all of its parents properties etc. All objects that point to the parent
  ///               must be editted such that the only object to be re-allocated is
  ///               parent.
  /// \param Load Loads and returns the object from the stream, leaving the steam pointing 
  ///             to the next block. Can return null on error. If self is passed in as null 
  ///             it can presume the stream starts at the beginning of the object block type,
  ///             i.e. it must call the loaders for its parent types. If self is not null it 
  ///             must presume that it to be an allready constructed instance of the object type
  ///             being loaded, into which it must insert the loaded data. It must only load the
  ///             one block that constitutes this object in this case, and not any parent blocks
  ///             as they will have allready been done.
   void AddType(cstrconst magic,cstrconst parent,
                Node * (*Create)(Core & core,Node * parent),
                Node * (*Load)(Core & core,io::InVirt<io::Binary> & in,Node * self));


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::svt::Core";}


 protected:
  str::TokenTable & tt;
  ds::SparseHash<Type*,mem::KillDel<Type> > types;
  
  struct LoadType
  {
   bit operator < (const LoadType & rhs) const 
   {
    if (magic[0]!=rhs.magic[0]) return magic[0] < rhs.magic[0];
    if (magic[1]!=rhs.magic[1]) return magic[1] < rhs.magic[1];
    return magic[2] < rhs.magic[2];
   }

   cstrconst magic;
   cstrconst parent;
   Node * (*Create)(Core & core,Node * parent);
   Node * (*Load)(Core & core,io::InVirt<io::Binary> & in,Node * self);
  };
  
  ds::SortList<LoadType> ltl;
};

//------------------------------------------------------------------------------
 };
};
#endif
