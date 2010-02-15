#ifndef EOS_SVT_TYPE_H
#define EOS_SVT_TYPE_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file type.h
/// Provides a type enforcment/checking system.

#include "eos/types.h"
#include "eos/svt/var.h"
#include "eos/ds/lists.h"

namespace eos
{
 namespace svt
 {
//------------------------------------------------------------------------------
/// This provides a class that represents a set of constraints on a Node and 
/// subtypes, such that you can check membership against any Node given to an
/// instance of this class. This allows for a typing scheme within the SVT, where
/// type is simply a matter of conforming to a particular structure.
/// The interface consists of one method, MemberOf, to do the check; all the 
/// other methods exist to create the Type appropriatly in the first place, and 
/// should not be called after the initial setup.
class EOS_CLASS Type
{
 public:
  /// &nbsp;
   Type();

  /// &nbsp;
   ~Type();


  /// This is the entire and only point of this class, the rest is just floss.
  /// Returns true if the given Node is a member of this class, false otherwise.
   bit MemberOf(const Node & rhs);


  /// The Mode given to constraints, can either be to set a Fixed value, a 
  /// Minimum value or a Maximum value. (None is used internally only.)
  /// Note that when applying constraints you can either run in min/max mode or
  /// fixed mode - in fixed mode multiple fixed values can be given, if any 
  /// match it passes, in min/max mode all given range checks must pass. Do not
  /// combine these checks.
   enum Mode {None,Fixed,Minimum,Maximum};


  /// Call this to indicate it must be at least a Meta, so the basic Node type 
  /// will not be accepted. Remember to call this for any Meta-based constraints
  /// to be applied.
   void RequireMeta();

  /// Call this to indicate it must be at least a Var, so Node and Meta
  /// will not be accepted. Remember to call this for any Var-based constraints
  /// to be applied.  
   void RequireVar();

  
  /// This applies a constraint on the existance of a key, with a given name.
   void RequireKey(str::Token key);

  /// This applies the same constraint as RequireKey but additionally requires a 
  /// particular type.
   void RequireKeyType(str::Token key,Meta::Type type);


  /// This constrains the number of dimensions allowed by a var.
  /// mode indicates how to constrain the dimensions, val is the number as 
  /// appropriate for the mode.
   void ConstrainDims(Mode mode,nat32 val);

  /// This constrains the size of a particular dimension, it also by extension 
  /// requires at least that many dimensions. dim is the dimension to constrain,
  /// mode how to constrain it and val the number that goes with the constraint 
  /// mode.
   void ConstrainSize(nat32 dim,Mode mode,nat32 val);
   

  /// This enforces the existance of a field, by name alone.
   void RequireField(str::Token name);

  /// This enforces the existance of a field, by name, type and size.
   void RequireFieldType(str::Token name,str::Token type,nat32 size);


 private:
  enum {Cnode,Cmeta,Cvar} cls; // Which class is required as a minimum.

  struct KeyReq
  {
   str::Token key;
   bit typeCheck; // If true check below, false and don't.
   Meta::Type type;
  };
  ds::List<KeyReq> krList;

  struct DimReq
  {
   bit size; // If true its a size constraint, if false a dim constraint.
   nat32 dim; // If a size constraint this is which dimension to affect.
   Mode mode; // What the constraint is, None if its an existencial constraint.
   nat32 value; // The number associated with the constraint, if relevent.
  };
  ds::List<DimReq> drList;

  struct FieldReq
  {
   str::Token name;
   str::Token type; // If set to NullToken any accepted, and size is ignored.
   nat32 size;
  };
  ds::List <FieldReq> frList;
};

//------------------------------------------------------------------------------
 };
};
#endif
