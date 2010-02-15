#include "eos/file/wavefront.h" // Fucking circular dependancies
#include "eos/file/ply.h"       // "

#ifndef EOS_SUR_MESH_H
#define EOS_SUR_MESH_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \namespace eos::sur
/// Provides assorted surface representations.

/// \file mesh.h
/// Defines a polygonal mesh representation, that is completly 
/// flexable whilst providing all querying and editting capabilities one could
/// require. Rather bloated, eats memory for breakfast, but this is because it 
/// is for editting purposes rather than rendering/storage purposes.
/// Uses an augmented half edge data structure, which has an additional edge
/// type beyond the half edges so it can manage it all.


#include "eos/types.h"

#include "eos/bs/geo3d.h"
#include "eos/ds/arrays.h"
#include "eos/ds/sort_lists.h"
#include "eos/svt/node.h"
#include "eos/str/tokens.h"
#include "eos/data/property.h"

namespace eos
{
 namespace sur
 {
//------------------------------------------------------------------------------
class EOS_CLASS Vertex;
class EOS_CLASS Edge;
class EOS_CLASS Face;

class EOS_CLASS IterVertexEdges;
class EOS_CLASS IterVertexFaces;

class EOS_CLASS MeshTransfer;

//------------------------------------------------------------------------------
/// An editable mesh, suports non-manifold topology and all standard querys and
/// edit operations, as flexable as possible really with constant time for
/// nearly all operations. (Or constant time in the size of the output.)
/// Eats memory for breakfast however, as it needs a hell of a lot of pointers
/// to do all this.
///
/// Additionally allows allows arbitary data to be assigned to 
/// vertices/edges/faces, you can then do as you will with this data.
/// Note that when naming properties all names begining with $ are reserved for
/// internal use, so do not create any properties that start as such.
class EOS_CLASS Mesh
{
 public:
  /// The token table is optional, but must be set if you intend on using the 
  /// property system.
   Mesh(str::TokenTable * tt = null<str::TokenTable*>());
   
  /// &nbsp;
   ~Mesh();
   
  
  /// For if you decide to start using the property system on a class after use.
  /// Note that calling this to change the token table when its allready in use
  /// will do some real strange, though not fatal, shit.
   void SetTT(str::TokenTable * tt);

  /// Will only return a valid value if it has been set.
   str::TokenTable & TT() const {return *tt;}


  /// Returns how many vertices are stored.
   nat32 VertexCount() const {return verts.Size();}

  /// Returns how many edges are stored.
   nat32 EdgeCount() const {return edges.Size();}

  /// Returns how many faces are stored.
   nat32 FaceCount() const {return faces.Size();}
   
  
  /// Appends all vertices contained within to the list.
   void GetVertices(ds::List<sur::Vertex> & out) const;

  /// Resizes the given array and fills it with all vertices contained within.
   void GetVertices(ds::Array<sur::Vertex> & out) const;
   
  /// Appends all edges contained within to the list. 
   void GetEdges(ds::List<sur::Edge> & out) const;

  /// Resizes the given array and fills it with all edges contained within.
   void GetEdges(ds::Array<sur::Edge> & out) const;
  
  /// Appends all faces contained within to the list.
   void GetFaces(ds::List<sur::Face> & out) const; 
   
  /// Resizes the given array and fills it with all faces contained within.
   void GetFaces(ds::Array<sur::Face> & out) const;
   
  
  /// Creates a new vertex, returns a handle to it.
   sur::Vertex NewVertex(const bs::Vert & pos);
   
  /// Creates a new edge, returns a handle to it.
  /// If the edge allready exists it returns the existing edge.
   sur::Edge NewEdge(sur::Vertex a,sur::Vertex b);
   
  /// Creates a new face, creating new edges as needed.
  /// The verts must be suplied in anti-clockwise order to get the face in the
  /// correct direcion.
  /// Do not make faces with less than 3 vertices.
   sur::Face NewFace(const ds::Array<sur::Vertex> & verts);

  /// Creates a new face, creating new edges as needed.
  /// The verts must be suplied in anti-clockwise order to get the face in the
  /// correct direcion.
  /// Do not make faces with less than 3 vertices.
   sur::Face NewFace(const ds::List<sur::Vertex> & verts);
   
  /// Creates a new triangle, vertices must be anti-clockwise.
   sur::Face NewFace(sur::Vertex a,sur::Vertex b,sur::Vertex c);
   
  /// Creates a new quad, the vertices must be anti-clockwise.
   sur::Face NewFace(sur::Vertex a,sur::Vertex b,sur::Vertex c,sur::Vertex d);
   

  /// Deletes the vertex. Any edges and faces that use the vertex will also be deleted.
  /// Any handles to deleted objects will then be left dangling.
   void Del(sur::Vertex vert);
   
  /// Deletes the edge. Any faces that use the edge will also be deleted.
  /// Any handles to deleted objects will then be left dangling.
   void Del(sur::Edge edge);
   
  /// Delete the face. Any handles to the face will be left dangling, including
  /// the one passed in.
   void Del(sur::Face face);
   
   
  /// Deletes a vertex, but for all using edges and faces it swaps in another given
  /// vertex. Any edges that then end up with only two vertices are then deleted,
  /// with using faces collapsed and deleted if left with less than 2 edges.
  /// Note that this does not guard against the situation of a face using the same
  /// vertex twice, it just stops it being adjacent, which of course allows for
  /// some very strange non-manifold geometry.
  /// (If only triangles exist it works perfectly of course.)
   void Fire(sur::Vertex toDie,sur::Vertex replacement);


  /// This removes all vertices and edges that are not in use by a face in the mesh.
   void Prune();
   
  /// This triangulates all faces, so all faces have 3 edges/vertices.
  /// Does not handle concave faces, and the algorithm is the simplest possible,
  /// so it can make bad decisions.
   void Triangulate();


  /// This returns a new svt::Node, with 4 svt::Var's as children that represent this
  /// 3D model. This is designed primarily for storage and some kinds of rendering,
  /// it is certainly not suitable for editting.
  /// You are responsible for calling delete on the returnee.
   svt::Node * AsSvt(svt::Core & core) const;

  /// This is given a svt::Node as returned from the AsSvt() method, it emptys this object 
  /// and replaces it with the contents of the svt db.
  /// Returns true on success, false on failure. On failure you can not be what
  /// the data structure will contain.
   bit FromSvt(svt::Node * node);
   
   
  /// This writes the entire 3d mesh to the wavefront file object.
   void Store(file::Wavefront & out) const;
   
  /// This writes the entire 3d mesh to the ply file object.
   void Store(file::Ply & out) const;


  /// Lets you add a property to the vertices.
   void AddVertProp(str::Token name,str::Token type,nat32 size,const void * ini);

  /// Lets you add a property to the edges.
   void AddEdgeProp(str::Token name,str::Token type,nat32 size,const void * ini);

  /// Lets you add a property to the faces.
   void AddFaceProp(str::Token name,str::Token type,nat32 size,const void * ini);
   
  /// Lets you add a property to the vertices.
   template <typename T>
   void AddVertProp(str::Token name,const T & ini)
   {AddVertProp(name,(*tt)(typestring<T>()),sizeof(T),(void*)&ini);}

  /// Lets you add a property to the edges.
   template <typename T>
   void AddEdgeProp(str::Token name,const T & ini)
   {AddEdgeProp(name,(*tt)(typestring<T>()),sizeof(T),(void*)&ini);}

  /// Lets you add a property to the faces.
   template <typename T>
   void AddFaceProp(str::Token name,const T & ini)
   {AddFaceProp(name,(*tt)(typestring<T>()),sizeof(T),(void*)&ini);}

  /// Lets you add a property to the vertices.
   template <typename T>
   void AddVertProp(cstrconst name,const T & ini)
   {AddVertProp((*tt)(name),(*tt)(typestring<T>()),sizeof(T),(void*)&ini);}

  /// Lets you add a property to the edges.
   template <typename T>
   void AddEdgeProp(cstrconst name,const T & ini)
   {AddEdgeProp((*tt)(name),(*tt)(typestring<T>()),sizeof(T),(void*)&ini);}

  /// Lets you add a property to the faces.
   template <typename T>
   void AddFaceProp(cstrconst name,const T & ini)
   {AddFaceProp((*tt)(name),(*tt)(typestring<T>()),sizeof(T),(void*)&ini);}


  /// Removes a property from the vertices.
   void RemVertProp(str::Token name);

  /// Removes a property from the vertices.
   void RemEdgeProp(str::Token name);

  /// Removes a property from the vertices.
   void RemFaceProp(str::Token name);

  /// Removes a property from the vertices.
   void RemVertProp(cstrconst name) {RemVertProp((*tt)(name));}

  /// Removes a property from the vertices.
   void RemEdgeProp(cstrconst name) {RemEdgeProp((*tt)(name));}

  /// Removes a property from the vertices.
   void RemFaceProp(cstrconst name) {RemFaceProp((*tt)(name));}
   
   
  /// The Add and Rem methods for properties do not cause an imediate affect for
  /// efficiency reasons, so you call the operations you want to do and then 
  /// call this method. This will then go through the entire data structure and
  /// apply every change made. After this all data::Property will be invalid and
  /// all handles will be invalid, unless no changes were made to that particular
  /// class. From the moment you call a Add/Rem method you shouldn't access 
  /// properties again until you have called commit.
   void Commit(bit useDefault = true);


  /// This returns how many properties have been assigned to vertices.
  /// Does not work during an edit/commit call sequence.
   nat32 CountVertProp() const {return vertProp.Size();}

  /// This returns how many properties have been assigned to edges.
  /// Does not work during an edit/commit call sequence.
   nat32 CountEdgeProp() const {return vertProp.Size();}
   
  /// This returns how many properties have been assigned to faces.
  /// Does not work during an edit/commit call sequence.
   nat32 CountFaceProp() const {return vertProp.Size();}


  /// This returns true if the given vertex property exists, false if it does not.
   bit ExistsVertProp(str::Token name) const;

  /// This returns true if the given edge property exists, false if it does not.
   bit ExistsEdgeProp(str::Token name) const;
   
  /// This returns true if the given face property exists, false if it does not.
   bit ExistsFaceProp(str::Token name) const;   

  /// This returns true if the given vertex property exists, false if it does not.
   bit ExistsVertProp(cstrconst name) const {return ExistsVertProp((*tt)(name));}

  /// This returns true if the given edge property exists, false if it does not.
   bit ExistsEdgeProp(cstrconst name) const {return ExistsEdgeProp((*tt)(name));}
   
  /// This returns true if the given face property exists, false if it does not.
   bit ExistsFaceProp(cstrconst name) const {return ExistsFaceProp((*tt)(name));}


  /// Returns the index for a vertex property, nat32(-1) if it doesn't exist.
   nat32 IndexVertProp(str::Token name) const;

  /// Returns the index for a edge property, nat32(-1) if it doesn't exist.
   nat32 IndexEdgeProp(str::Token name) const;
   
  /// Returns the index for a face property, nat32(-1) if it doesn't exist.
   nat32 IndexFaceProp(str::Token name) const;
   
  /// Returns the index for a vertex property, nat32(-1) if it doesn't exist.
   nat32 IndexVertProp(cstrconst name) const {return IndexVertProp((*tt)(name));}

  /// Returns the index for a edge property, nat32(-1) if it doesn't exist.
   nat32 IndexEdgeProp(cstrconst name) const {return IndexEdgeProp((*tt)(name));}
   
  /// Returns the index for a face property, nat32(-1) if it doesn't exist.
   nat32 IndexFaceProp(cstrconst name) const {return IndexFaceProp((*tt)(name));}
   
   
  /// Returns the name of the indexed vector property.
   str::Token NameVertProp(nat32 i) const;
  
  /// Returns the type of the indexed vector property.
   str::Token TypeVertProp(nat32 i) const;
  
  /// Returns the size of the indexed vector property.
   nat32 SizeVertProp(nat32 i) const;
  
  /// Returns the default data for the indexed vector property.
   const void * DefaultVertProp(nat32 i) const;

  /// Returns the name of the indexed edge property.
   str::Token NameEdgeProp(nat32 i) const;
  
  /// Returns the type of the indexed edge property.
   str::Token TypeEdgeProp(nat32 i) const;
  
  /// Returns the size of the indexed edge property.
   nat32 SizeEdgeProp(nat32 i) const;
  
  /// Returns the default data for the indexed edge property.
   const void * DefaultEdgeProp(nat32 i) const;

  /// Returns the name of the indexed face property.
   str::Token NameFaceProp(nat32 i) const;
  
  /// Returns the type of the indexed face property.
   str::Token TypeFaceProp(nat32 i) const;
  
  /// Returns the size of the indexed face property.
   nat32 SizeFaceProp(nat32 i) const;
  
  /// Returns the default data for the indexed face property.
   const void * DefaultFaceProp(nat32 i) const;


  /// Returns a property you may use for the indexed vertex property.
  /// Templated on the type of the property.
   template <typename T>
   data::Property<sur::Vertex,T> GetIndVertProp(nat32 i) const;

  /// Returns a property you may use for the named vertex property.
  /// Templated on the type of the property.
   template <typename T>
   data::Property<sur::Vertex,T> GetVertProp(str::Token name) const;

  /// Returns a property you may use for the named vertex property.
  /// Templated on the type of the property.
   template <typename T>
   data::Property<sur::Vertex,T> GetVertProp(cstrconst name) const;

  /// Returns a property you may use for the indexed edge property.
  /// Templated on the type of the property.
   template <typename T>
   data::Property<sur::Edge,T> GetIndEdgeProp(nat32 i) const;

  /// Returns a property you may use for the named edge property.
  /// Templated on the type of the property.
   template <typename T>
   data::Property<sur::Edge,T> GetEdgeProp(str::Token name) const;

  /// Returns a property you may use for the named edge property.
  /// Templated on the type of the property.
   template <typename T>
   data::Property<sur::Edge,T> GetEdgeProp(cstrconst name) const;

  /// Returns a property you may use for the indexed face property.
  /// Templated on the type of the property.
   template <typename T>
   data::Property<sur::Face,T> GetIndFaceProp(nat32 i) const;

  /// Returns a property you may use for the named face property.
  /// Templated on the type of the property.
   template <typename T>
   data::Property<sur::Face,T> GetFaceProp(str::Token name) const;

  /// Returns a property you may use for the named face property.
  /// Templated on the type of the property.
   template <typename T>
   data::Property<sur::Face,T> GetFaceProp(cstrconst name) const;
   
   
  /// Transfers all vertex properties from one vertex handle to another.
  /// Both must be members of this object.
  /// Does not transfer position.
   void Transfer(sur::Vertex & to,const sur::Vertex & from);

  /// Transfers all edge properties from one edge handle to another.
  /// Both must be members of this object.
   void Transfer(sur::Edge & to,const sur::Edge & from);
   
  /// Transfers all face properties from one face handle to another.
  /// Both must be members of this object.
   void Transfer(sur::Face & to,const sur::Face & from);
   
   
  /// This checks if a contraction is safe, i.e. won't invert faces or 
  /// make concave faces. Returns true if its safe, false if not. Note that if 
  /// things are allready screwed it will usually return false regardless of 
  /// how much more screwed the operation will make it.
   static bit SafeContraction(const Edge & edge,const bs::Vert & newPos);
   
   
  /// Given a vertex this verifies that the invariants hold, for debugging purposes.
  /// Returns true to be put in an assert.
  /// (Might indicate failure by crashing or going into an infinite loop, does not test everything.)
   bit Invariant(const sur::Vertex & test) const;


  /// &nbsp;
   static cstrconst TypeString() {return "eos::sur::Mesh";}


 private:
  struct Vertex;
  struct DirEdge;
  struct HalfEdge;
  struct Face;


  struct Vertex
  {
   DirEdge * edge; // Any DirEdge escaping from the vertex. (edge->to!=this)
   bs::Vert pos;
   nat32 ind; // Index within the data structure. Only valid at certain points.
  };
  
  struct DirEdge
  {
   Vertex * to; // Vertex it its going to, i.e. not the vertex its a member of a doubly linked list for.

   DirEdge * partner; // Going the other way.
   DirEdge * next; // Doubly linked list of all DirEdge's leaving a vertex.
   DirEdge * prev; // "
   
   HalfEdge * user; // Any half edge thats in the same direction as this DirEdge.
   nat32 ind; // Index within the data structure. Only valid at certain points.
  };
  
  struct HalfEdge
  {
   Face * face; // The face which owns this HalfEdge.
   HalfEdge * chain; // For following arround face.

   DirEdge * edge; // Directions must match.
   HalfEdge * next; // Doubly linked list of half edges arround a particular edge in a given direction.
   HalfEdge * prev; // "
  };
  
  struct Face
  {
   HalfEdge * edge; // Any half edge that makes up the faces edge ring.
   nat32 size; // How many vertices/edges it has.
   nat32 ind; // Index within the data structure. Only valid at certain points.
  };


 // Make all the suport classes friends with this class, so they can play with it...
  friend class sur::Vertex;
  friend class sur::Edge;
  friend class sur::Face;
  
 // Other support classes, in other files, that needs access to all this...
  friend class sur::IterVertexEdges;
  friend class sur::IterVertexFaces;
  friend class sur::MeshTransfer;


 // Storage for all the contained things, so we can iterate them and delete them...
  ds::SortList<Vertex*> verts;
  ds::SortList<DirEdge*> edges; // For each pair only contains one entry.
  ds::SortList<Face*> faces;
  
  
 // Structure for storing a variable as attached to a vertex/edge/face...
  struct Prop
  {
   str::Token name;
   str::Token type;
   nat32 size;
   byte * ini; // Malloc'ed.

   nat32 offset; // Offset into any given set of fields to get to this one, allways from start of data structure.

   enum State {Stored, // It is currently in the data structure, no change needed.
               Added, // It does not currently exist and must be added next Commit.
               Deleted // It is to be deleted at the next Commit, but is still stored.
              } state;
  };
  
 // The data for the property system...
  str::TokenTable * tt;

  bit vertPropChanged;
  bit edgePropChanged;
  bit facePropChanged;
  
  nat32 vertPropSize;
  nat32 edgePropSize;
  nat32 facePropSize;

  ds::Array<Prop> vertProp;
  ds::Array<Prop> edgeProp;
  ds::Array<Prop> faceProp;
  
  ds::SparseHash<nat32> vertByName;
  ds::SparseHash<nat32> edgeByName;
  ds::SparseHash<nat32> faceByName;


 // Structure used in the conversion from/to svt...
  struct HopSkip
  {
   nat32 size;
   nat32 fromOffset;
   nat32 toInd;
  };
 
 
 // Structure used to represent operations in the below stuff...
  struct Op
  {
   bit op; // false==offset copy, true==memory block copy.
   nat32 offset;
   nat32 size;
   union
   {
    nat32 oldOffset; // op==false.
    byte * ini; // op==true.
   };
  };
 
 // Suport methods for the property system, used by the commit method...
  // The commit method is broken into 3, for obvious reasons...
   void CommitVertex(bit useDefault);
   void CommitEdge(bit useDefault);
   void CommitFace(bit useDefault);

  // These edit all pointers in other objects given the old pointer so that they 
  // point to the new pointer. Also copies over all data from the previous 
  // data, so all you have to worry about is copying over properties.
  // (This is for replacement, not merging, as merging requires degenracy checking.)
   void RepointVertex(Vertex * from,Vertex * to);
   void RepointEdge(DirEdge * from,DirEdge * to); // Does the partner as well, must be given the first in memory of each.
   void RepointFace(Face * from,Face * to);
   
  // This is given a face, it checks if its degenerate, if it is it deletes it.
  // It also handles the fact that this will usually mean duplicate storage of an
  // edge, and merge one edge into its identical twin...
   void CheckDegenFace(Face * face);
   
  // Given two sets of 3 vertices this returns true if the triangles described
  // have the same facing, false if they are pointing in diferent directions...
  // (Also returns false if either triangle is degenerate.)
   static bit SameDir(const bs::Vert & aP1,const bs::Vert & aP2,const bs::Vert & aP3,
                      const bs::Vert & bP1,const bs::Vert & bP2,const bs::Vert & bP3)
   {
    bs::Normal dirA;
    {
     bs::Vert d1 = aP2; d1 -= aP1;
     bs::Vert d2 = aP2; d2 -= aP3;
     math::CrossProduct(d1,d2,dirA);
    }
    if (math::IsZero(dirA.LengthSqr())) return false;

    bs::Normal dirB;
    {
     bs::Vert d1 = bP2; d1 -= bP1;
     bs::Vert d2 = bP2; d2 -= bP3;
     math::CrossProduct(d1,d2,dirB);
    }
    if (math::IsZero(dirB.LengthSqr())) return false;
   
    return (dirA*dirB)>0.0;
   }
};

//------------------------------------------------------------------------------
/// Proxy interface for a vertex as stored in a Mesh object.
class EOS_CLASS Vertex
{
 public:
  /// Creates a !Valid() handle.
   Vertex():vert(null<Mesh::Vertex*>()) {}
 
  /// &nbsp;
   Vertex(const Vertex & rhs):vert(rhs.vert) {}

  /// &nbsp;
   ~Vertex() {}

  /// &nbsp;
   Vertex & operator = (const Vertex & rhs) {vert = rhs.vert; return *this;}

  /// Returns true if valid, false if not. If false do not call any methods.
   bit Valid() {return vert!=null<Mesh::Vertex*>();}

  /// Returns true if they are handles to the same vertex.
   bit operator == (const Vertex & rhs) const {return vert==rhs.vert;}

  /// Returns true if they are handles to the different vertices.
   bit operator != (const Vertex & rhs) const {return !(*this==rhs);}

  /// Returns the index of this vertex from the last time a GetVertices method was called.
  /// Only valid if no editting operations have since been called.
  /// Your allowed to edit the number and screw with it yourself, remembering that a GetVertices will change it.
   nat32 & Index() {return vert->ind;}

  /// &nbsp;
   const nat32 & Index() const {return vert->ind;}  


  /// Returns the position of the vertex. You may edit it directly.
   bs::Vert & Pos() {return vert->pos;}
   
  /// &nbsp;
   const bs::Vert & Pos() const {return vert->pos;}


  /// Appends to the list all vertices that are connected to this vertex by
  /// following a single edge.
   void GetVertices(ds::List<Vertex> & out) const; 

  /// Resizes the given array and fills it with all vertices that are connected
  /// to this vertex by a single edge transition.
   void GetVertices(ds::Array<Vertex> & out) const;   

  /// Appends to the list all edge that are using this vertex.
   void GetEdges(ds::List<Edge> & out) const;

  /// Resizes the given array and fills it with all edges that are using this
  /// vertex.
   void GetEdges(ds::Array<Edge> & out) const;

  /// Appends all faces that are using this vertex to the given list.
   void GetFaces(ds::List<Face> & out) const;

  /// Resizes the given array and fills it with all faces that are using this
  /// vertex.
   void GetFaces(ds::Array<Face> & out) const;


  /// Given another Vertex this returns the Edge connecting them. If no edge
  /// exists this the returned Edge handle will not be active.
   Edge Link(Vertex other) const;
   
   
  /// Checks if moving the vertex to the given position would cause problems 
  /// with the using faces, i.e. invert them or make them concave.
   bit SafeMove(const bs::Vert & newPos) const;


  /// &nbsp;
   static cstrconst TypeString() {return "eos::sur::Vertex";}


 private:
  friend class Mesh;
  friend class Edge;
  friend class Face;
  
  friend class sur::IterVertexEdges;
  friend class sur::IterVertexFaces;
  friend class sur::MeshTransfer;
  
  Vertex(Mesh::Vertex * v):vert(v) {}
  static void * PropPtr(Vertex * ptr);

  Mesh::Vertex * vert;
};

//------------------------------------------------------------------------------
/// Proxy interface for an edge as stored in a Mesh object.
class EOS_CLASS Edge
{
 public:
  /// Creates a !Valid() handle.
   Edge():edge(null<Mesh::DirEdge*>()) {}
 
  /// &nbsp;
   Edge(const Edge & rhs):edge(rhs.edge) {}

  /// &nbsp;
   ~Edge() {}

  /// &nbsp;
   Edge & operator = (const Edge & rhs) {edge = rhs.edge; return *this;}

  /// Returns true if valid, false if not. If false do not call any methods.
   bit Valid() {return edge!=null<Mesh::DirEdge*>();}
   
  /// Returns true if they are handles to the same edge.
   bit operator == (const Edge & rhs) const {return edge==rhs.edge;}

  /// Returns the index of this vertex from the last time a GetEdges method was called.
  /// Only valid if no editting operations have since been called.
  /// Your allowed to edit the number and screw with it yourself, remembering that a GetEdges will change it.  
   nat32 & Index() {return math::Min(edge,edge->partner)->ind;}
   
  /// &nbsp;
   const nat32 & Index() const {return edge->ind;}


  /// Returns how many faces the edge has.
   nat32 FaceCount() const;
   
  /// Given one of the edges vertexes this returns the other.
  /// Give it a Vertex which it isn't using at your peril.
   Vertex Other(Vertex v) const;
   
  /// Returns one of the Vertices, quite arbitary.
   Vertex VertexA() const;

  /// Returns the other Vertex to VertexA().
   Vertex VertexB() const;

  /// Appends to the list the two vertices it uses.
   void GetVertices(ds::List<Vertex> & out) const;

  /// Resizes the given array and fills it with the two vertices that are 
  /// being used by this edge.
   void GetVertices(ds::Array<Vertex> & out) const;   
  
  /// Appends to the list all edges that are connected to this edge.
   void GetEdges(ds::List<Edge> & out) const;

  /// Resizes the given array and fills it with all edges that are connected to
  /// this edge via the vertices at either end.
   void GetEdges(ds::Array<Edge> & out) const;

  /// Appends to the list all faces that are using this edge.
   void GetFaces(ds::List<Face> & out) const;

  /// Resizes the given array and fills it with all faces that are using this 
  /// edge. Will be 0,1 or 2 assuming manifold geometry, but can be more as this
  /// structure does suport arbitary meshes.
   void GetFaces(ds::Array<Face> & out) const;
   
  /// Returns an arbitary face that uses the edge, or a bad face if it has no useing faces.
   Face AnyFace() const;


  /// Given another Edge this returns the Vertex they share.
  /// If such a Vertex does not exist then the returned Vertex will be !Valid().
   Vertex Link(Edge other) const;
   
  /// Returns true if the edge is a boundary edge, i.e. it doesn't have 2 faces
  /// with the same direction. Note that not having any faces makes it not a boundary
  /// edge, as its not bounding anything!
   bit Boundary() const
   {
    bit a = edge->user==null<Mesh::HalfEdge*>();
    bit b = edge->partner->user==null<Mesh::HalfEdge*>();
    return a!=b;
   }


  /// &nbsp;
   static cstrconst TypeString() {return "eos::sur::Edge";}


 private:
  friend class Mesh;
  friend class Vertex;
  friend class Face;
  
  friend class sur::IterVertexEdges;
  friend class sur::IterVertexFaces;
  friend class sur::MeshTransfer;
  
  Edge(Mesh::DirEdge * e)
  :edge(e)
  {
   if (edge->partner<edge) edge = edge->partner;
  }
  static void * PropPtr(Edge * ptr);

  Mesh::DirEdge * edge;
};

//------------------------------------------------------------------------------
/// Proxy interface for a face as stored in a Mesh object.
class EOS_CLASS Face
{
 public:
  /// Creates a !Valid() handle.
   Face():face(null<Mesh::Face*>()) {}
 
  /// &nbsp;
   Face(const Face & rhs):face(rhs.face) {}

  /// &nbsp;
   ~Face() {}

  /// &nbsp;
   Face & operator = (const Face & rhs) {face = rhs.face; return *this;}

  /// Returns true if valid, false if not. If false do not call any methods.
   bit Valid() {return face!=null<Mesh::Face*>();}

  /// Returns true if they are handles to the same face.
   bit operator == (const Face & rhs) const {return face==rhs.face;}

  /// Returns the index of this vertex from the last time a GetFaces method was called.
  /// Only valid if no editting operations have since been called.
  /// Your allowed to edit the number and screw with it yourself, remembering that a GetFaces will change it.    
   nat32 & Index() {return face->ind;}

  /// &nbsp;
   const nat32 & Index() const {return face->ind;}
   
   
  /// Outputs the normal and distance from the origin of the face, assumes the 
  /// face is planar.
   void Eq(bs::Normal & norm,real32 & dist);
  
  
  /// Returns how many vertices/edges are used by the face.
   nat32 Size() const {return face->size;}


  /// Appends the vertices the face is using to the given list, in 
  /// anti-clockwise order.
  /// If you get the array/list for both the vertices and the edges then
  /// edge[0] goes from vert[0] to vert[1], and so on.
   void GetVertices(ds::List<Vertex> & out) const; 

  /// Resizes the given array and fills it with all the vertices being used by
  /// this face, in anti-clockwise order.
  /// If you get the array/list for both the vertices and the edges then
  /// edge[0] goes from vert[0] to vert[1], and so on.  
   void GetVertices(ds::Array<Vertex> & out) const;   
  
  /// Appends the edges the face is using to the given list, in anti-clockwise
  /// order.
  /// If you get the array/list for both the vertices and the edges then
  /// edge[0] goes from vert[0] to vert[1], and so on.  
   void GetEdges(ds::List<Edge> & out) const;

  /// Resizes the given array and fills it with all edges that are being used
  /// by this face, in anti-clockwise order.
  /// If you get the array/list for both the vertices and the edges then
  /// edge[0] goes from vert[0] to vert[1], and so on.
   void GetEdges(ds::Array<Edge> & out) const;

  /// Appends to the given list all faces that
  /// are adjacent to this face, i.e. share an edge.
  /// This operation is one of the few badly supported by the data 
  /// structure, and it has to use a binary tree to remove duplicates from the 
  /// output.
   void GetFaces(ds::List<Face> & out) const;

  /// Resizes the given array and fills it with all faces that
  /// are adjacent to this face, i.e. share an edge.
  /// This operation is one of the few badly supported by the data 
  /// structure, and it has to use a binary tree to remove duplicates from the 
  /// output.
   void GetFaces(ds::Array<Face> & out) const;


  /// Fills the given array up with all shared edges between this face and the given.
  /// If they have no adjacencies then the list will be empty.
  /// Replaces any previous array contents.
   void Link(Face other,ds::Array<Edge> & out) const;
   
  /// Swaps the orientation of the face, so it points the other way.
   void Reverse();


  /// &nbsp;
   static cstrconst TypeString() {return "eos::sur::Face";}


 private:
  friend class Mesh;
  friend class Vertex;
  friend class Edge;
  
  friend class sur::IterVertexEdges;
  friend class sur::IterVertexFaces;  
  friend class sur::MeshTransfer;

  Face(Mesh::Face * f):face(f) {}
  static void * PropPtr(Face * ptr);

  Mesh::Face * face;
};

//------------------------------------------------------------------------------
template <typename T>
inline data::Property<sur::Vertex,T> Mesh::GetIndVertProp(nat32 i) const
{
 return data::Property<sur::Vertex,T>(vertProp[i].offset,sur::Vertex::PropPtr);
}

template <typename T>
inline data::Property<sur::Vertex,T> Mesh::GetVertProp(str::Token name) const
{
 nat32 * ind = vertByName.Get(name);
 if (ind) return data::Property<sur::Vertex,T>(vertProp[*ind].offset,sur::Vertex::PropPtr);
     else return data::Property<sur::Vertex,T>();
}

template <typename T>
inline data::Property<sur::Vertex,T> Mesh::GetVertProp(cstrconst name) const
{
 nat32 * ind = vertByName.Get((*tt)(name));
 if (ind) return data::Property<sur::Vertex,T>(vertProp[*ind].offset,sur::Vertex::PropPtr);
     else return data::Property<sur::Vertex,T>();
}

template <typename T>
inline data::Property<sur::Edge,T> Mesh::GetIndEdgeProp(nat32 i) const
{
 return data::Property<sur::Edge,T>(edgeProp[i].offset,sur::Edge::PropPtr);
}

template <typename T>
inline data::Property<sur::Edge,T> Mesh::GetEdgeProp(str::Token name) const
{
 nat32 * ind = edgeByName.Get(name);
 if (ind) return data::Property<sur::Edge,T>(edgeProp[*ind].offset,sur::Edge::PropPtr);
     else return data::Property<sur::Edge,T>();
}

template <typename T>
inline data::Property<sur::Edge,T> Mesh::GetEdgeProp(cstrconst name) const
{
 nat32 * ind = edgeByName.Get((*tt)(name));
 if (ind) return data::Property<sur::Edge,T>(edgeProp[*ind].offset,sur::Edge::PropPtr);
     else return data::Property<sur::Edge,T>();
}

template <typename T>
inline data::Property<sur::Face,T> Mesh::GetIndFaceProp(nat32 i) const
{
 return data::Property<sur::Face,T>(faceProp[i].offset,sur::Face::PropPtr);
}

template <typename T>
inline data::Property<sur::Face,T> Mesh::GetFaceProp(str::Token name) const
{
 nat32 * ind = faceByName.Get(name);
 if (ind) return data::Property<sur::Face,T>(faceProp[*ind].offset,sur::Face::PropPtr);
     else return data::Property<sur::Face,T>();
}

template <typename T>
inline data::Property<sur::Face,T> Mesh::GetFaceProp(cstrconst name) const
{
 nat32 * ind = faceByName.Get((*tt)(name));
 if (ind) return data::Property<sur::Face,T>(faceProp[*ind].offset,sur::Face::PropPtr);
     else return data::Property<sur::Face,T>();
}

//------------------------------------------------------------------------------
 };
};
#endif
