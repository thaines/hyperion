#ifndef EOS_SUR_MESH_SUP_H
#define EOS_SUR_MESH_SUP_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

/// \file mesh_sup.h
/// Mesh support tools, to assist in manipulating meshes.

#include "eos/types.h"
#include "eos/sur/mesh.h"

namespace eos
{
 namespace sur
 {
//------------------------------------------------------------------------------
/// Instances of this class are given a 'from' and a 'to' Mesh, possibly the same
/// Mesh. It then provides the ability to transfer properties between the meshes,
/// making sure to jiggle as need be to match the names where size matches.
/// (But not necesarilly type - be warned.) It also provides the ability to 
/// interpolate multiple weighted vertices from the 'from' mesh to create a new
/// vertex in the 'to' mesh. When doing this it also interpolates any properties
/// that are of type 'real32' with the same weighting as the position calculation.
/// The from and to mesh must use the same token table.
///
/// Note that a structure is constructed with this class, if either of the two
/// involved meshes then have Commit() called again the structure will become
/// wrong and a new one of these will need constructing, unless you want a crash.
/// Do not use it whilst adding/deleting fields before you have commited.
class EOS_CLASS MeshTransfer
{
 public:
  /// &nbsp;
   MeshTransfer(const Mesh & from,Mesh & to);
  
  /// &nbsp;
   ~MeshTransfer();
   
   
  /// Transfers the properties of a vector from the 'from' mesh to the 'to mesh.
   void Transfer(sur::Vertex to,const sur::Vertex from);
  
  /// Transfers the properties of an edge from the 'from' mesh to the 'to mesh.
   void Transfer(sur::Edge to,const sur::Edge from);
   
  /// Transfers the properties of a face from the 'from' mesh to the 'to mesh.
   void Transfer(sur::Face to,const sur::Face from);


  /// Constructs a new vector in the to mesh from a set of vectors in the from mesh.
  /// Each vector is given a weighting. These are used to interpolate position 
  /// and all real32 properties.
  /// Returns the new vertex handle.
  /// The non interpolatable properties are copied from the first entry.
   sur::Vertex Interpolate(nat32 n,sur::Vertex * v,real32 * weight);
   
   
  /// &nbsp;
   static cstrconst TypeString() {return "eos::sur::MeshTransfer";}


 private:
  Mesh & to;
 
  // A transfer consists of a sequence of operations, represented by an array of these. Each translates to a mem::Copy.
   struct Op
   {
    nat32 toOffset; // Offset in target to transfer to.
    nat32 size; // How large the transfer should be.
    bit type; // If false from is a fixed pointer, if true its an offset into the from memory.
    union
    {
     nat32 fromOffset;
     byte * fromPtr;
    };
   
    void Do(byte * to,byte * from)
    {
     if (type) mem::Copy(to+toOffset,from+fromOffset,size);
          else mem::Copy(to+toOffset,fromPtr,size);
    }
    
    void DoTra(byte * to,byte * from)
    {
     if (type) mem::Copy(to+toOffset,from+fromOffset,size);
    }
        
    bit operator < (const Op & rhs) const
    {
     return toOffset < rhs.toOffset;
    }
   };
  
  // Array of transfers for each type...
   ds::Array<Op> vertOp;
   ds::Array<Op> edgeOp;
   ds::Array<Op> faceOp;


  // Information for doing interpolation - offsets of matching real32's in the
  // two sides, for vertices alone as only they are interpolatable...
   struct Match
   {
    nat32 fromOffset;
    nat32 toOffset;
   };
   ds::Array<Match> vertReal;



  // Helper methods...
   // This is given a from property list,its index, a to property list and an output
   // op array - it fills the array accordingly...
    void Build(const ds::Array<Mesh::Prop> & from,const ds::SparseHash<nat32> & index,
               const ds::Array<Mesh::Prop> & to,ds::Array<Op> & out);
    
   // This optimises an op array to make it as efficiant as possible...
    void Optimise(ds::Array<Op> & ops);
};

//------------------------------------------------------------------------------
 };
};
#endif
