#ifndef EOS_DS_KD_TREE_H
#define EOS_DS_KD_TREE_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file kd_tree.h
/// A Kd tree implimentation with the ushall capability. Note that because it
/// impliments efficient nearest neighbour searching it also requires distance
/// measures beyond basic less-than cpability.

#include "eos/types.h"
#include "eos/typestring.h"

#include "eos/mem/alloc.h"
#include "eos/mem/functions.h"
#include "eos/mem/safety.h"
#include "eos/ds/sorting.h"
#include "eos/ds/lists.h"
#include "eos/ds/arrays.h"
#include "eos/ds/priority_queues.h"
#include "eos/math/vectors.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
// The code for the KD tree, due to the flat template structure.
class EOS_CLASS KdTreeCode
{
 protected:
   KdTreeCode();
  ~KdTreeCode();


  struct Node;
  
  struct NodePtr
  {
   bit operator<(const NodePtr & rhs) const {return dimDist<rhs.dimDist;}

   real64 dimDist; // Squared
   nat32 dim;
   struct KdTreeCode::Node * node;
  };

  struct Node // Malloc-ed, data follows straight afterwards.
  {
   // Constructors...
    static Node * MakeNode(nat32 elementSize,const void * data);
    static Node * CloneNode(nat32 elementSize,Node * other);

   // Destructor... (free's self and children.)
    void Die(void (*Term)(void * ptr));     
    
   // Hunters...
    void * Add(nat32 elementSize,nat32 dimCount,nat32 dim,
               const void * data,real64 (*DI)(const void * item,nat32 dim));
    static Node * MakeMany(nat32 elementSize,nat32 dimCount,nat32 dim,
                           Node ** data,nat32 dataSize,real64 (*DI)(const void * item,nat32 dim));
   
    Node * Nearest(nat32 elementSize,nat32 dimCount,nat32 dim,
                   const void * dummy,real64 (*DI)(const void * item,nat32 dim),real64 & minDist);
    Node * NearestImprove(nat32 elementSize,nat32 dimCount,nat32 dim,
                          const void * dummy,real64 (*DI)(const void * item,nat32 dim),
                          real64 & minDist,Node * bsf);

    Node * ApproxNearest(nat32 elementSize,nat32 dimCount,nat32 dim,
                         const void * dummy,real64 (*DI)(const void * item,nat32 dim),
                         PriorityQueue<KdTreeCode::NodePtr> & ob); // Stores the branches not taken in the priority queue.
  
    void RangeGet(nat32 elementSize,nat32 dimCount,nat32 dim,
                  const math::Vector<real64> & min,const math::Vector<real64> & max,
                  ListCode * out,real64 (*DI)(const void * item,nat32 dim));                         

   // Getter... 
    void * Data() {return (byte*)this + sizeof(*this);}
    void MakeList(nat32 & index,Node ** out);

   Node * left;
   Node * right;	  
  };


  void Del(void (*Term)(void * ptr));
  
  void Copy(nat32 elementSize,const KdTreeCode & rhs); // Call Del before calling this.

  void * Add(nat32 elementSize,nat32 dimCount,const void * data,real64 (*DI)(const void * item,nat32 dim));
  
  void MassFill(nat32 elementSize,nat32 dimCount,const ArrayCode * ac,real64 (*DI)(const void * item,nat32 dim)); // Call Del before calling this.
  void Rebalance(nat32 elementSize,nat32 dimCount,real64 (*DI)(const void * item,nat32 dim));
  
  Node * Nearest(nat32 elementSize,nat32 dimCount,
                 const void * dummy,real64 (*DI)(const void * item,nat32 dim));
  Node * ApproxNearest(nat32 elementSize,nat32 dimCount,
                       const void * dummy,real64 (*DI)(const void * item,nat32 dim),
                       nat32 nodeLimit);
  
  void RangeGet(nat32 elementSize,nat32 dimCount,
                const math::Vector<real64> & min,const math::Vector<real64> & max,
                ListCode * out,real64 (*DI)(const void * item,nat32 dim));


 nat32 size;
 Node * root;
 
 PriorityQueue<NodePtr> ann; // Priority queue used for approximate nearest neighbour code - stored within to save on allocating it each call.
};

//------------------------------------------------------------------------------
/// A default support function for the KD tree, to allow out-of-the-box suport for
/// vectors and things that inherit from vectors. Simply provides the sorting 
/// capability by indexing the templated object with the [] operator using the 
/// requested dimension and returning the results.
template <typename T>
real64 VectorIndexKD(const T * item,nat32 dim)
{
 return real64((*item)[dim]);
}

//------------------------------------------------------------------------------
/// A KD tree. It allows for any dimensionality and nearest neighbour querying.
/// Because of the (eucledian) nearest neigbour suport the sorting function provided has to 
/// return more information than which one is greater, and in fact is just a 
/// query for an objects coordinate in a particular dimension. (A default 
/// coordinate query that uses the [] operator with the dimension is provided, 
/// this works with math::Vect etc.) In addition to an accurate nearest neighbour
/// search it also provides an approximate nearest neighbour, highly recommended
/// once you pass about 8 dimensional space as you might as well brute force it otherwise.
/// Of course its approximate, but will ushally get something fairly close to
/// the nearest. Its the best bin first techneque of 'Shape Indexing Using
/// Approximate Nearest-Neighbour Search in High-Dimensional Spaces' by Beis & Lowe.
/// In addition the ability to construct a tree given a large amount of data is
/// provided, this will then produce a nicelly balanced tree. A re-balancing 
/// method is also provided, and a standard all nodes in a bounding hyper-box getter.
/// The templated parameters are:
/// - T - The type of the contained objects.
/// - DC - The number of dimensions. (Whilst not as efficient as it could be setting this to 1 does produce yea basic binary tree, it dosn't have too great a loss of speed either.)
/// - SO - The dimension position accessor, for 'sorting' the nodes.
/// - DT - type for cleaning up the nodes when done with them.
template <typename T,nat32 DC,real64 (*SO)(const T * item,nat32 dim) = VectorIndexKD<T>,typename DT = mem::KillNull<T> >
class EOS_CLASS KdTree : public KdTreeCode
{
 public:
  /// &nbsp;
   KdTree() {}
   
  /// &nbsp;
   ~KdTree() {Del(DelFunc);}

  /// Resets the tree to contain nothing.
   void MakeEmpty() {Del(DelFunc);}


  /// &nbsp;
   template <typename DTT>
   KdTree<T,DC,SO,DT> & operator = (const KdTree<T,DC,SO,DTT> & rhs) {Del(DelFunc); Copy(sizeof(T),rhs);}
   
  /// This sets the KdTree to contain the entire contents of the given array, the difference between
  /// this and looping and Add-ing each element in turn is this goes to the extra effort to create
  /// a balanced tree. Note that the time cost of calling Add for each item then Rebalance is only
  /// the cost of doing the insertions into the tree, so if more conveniant such a techneque will 
  /// not infer that great an extra cost. Any previous contents in the tree will be terminated.
   template <typename MTT,typename DTT>
   void Build(const Array<T,MTT,DTT> & rhs) {Del(DelFunc); MassFill(sizeof(T),DC,&rhs,SortFunc);}
  
   
  /// Returns the number of nodes in the tree.
   nat32 Size() const {return size;}
  
  /// Returns how much memory the object is using in bytes, including the footprint of the nodes
  /// assuming they don't point to further data.
   nat32 Memory() const {return size*(sizeof(Node)+sizeof(T)) + sizeof(size) + sizeof(root) + ann.Memory();}

   
  /// Adds a node to the tree, returns a reference to the new node.
   T & Add(const T & in) {return *(T*)KdTreeCode::Add(sizeof(T),DC,&in,SortFunc);}
  
  /// Rebalances the tree, should be called after a large number of adds.
   void Rebalance() {KdTreeCode::Rebalance(sizeof(T),DC,SortFunc);}
   
  
  /// Returns the nearest node to the given dummy node.
   T & Nearest(const T & dummy) {return *(T*)(KdTreeCode::Nearest(sizeof(T),DC,&dummy,SortFunc)->Data());}
  
  /// Returns the nearest approximate node to the given dummy node.
  /// Uses the techneque given in 'Shape Indexing Using Approximate
  /// nearest-Neighbour Search in High-Dimensional Spaces' by Beis
  /// & Lowe. This best bin first method essentially has a cut off 
  /// as to how many nodes it will check, checking them in an approximate
  /// best candidate first manor. nodeLimit is the number of nodes to 
  /// cut off after. Once you pass arround 10 dimensions or so this
  /// is your only sensible option for nearest neighbour searching,
  /// as at that point normal NN will probably end up searching the
  /// entire tree.
  /// Obviously, it dosn't allways produce the best result.
   T & ApproxNearest(const T & dummy,nat32 nodeLimit)
   {return *(T*)(KdTreeCode::ApproxNearest(sizeof(T),DC,&dummy,SortFunc,nodeLimit)->Data());}

  /// Extracts all nodes in a given cuboid volume into a linked list.
   template <typename DTT>
   void GetRange(const math::Vector<real64> & min,const math::Vector<real64> & max,List<T,DTT> & out)
   {RangeGet(sizeof(T),DC,min,max,(ListCode*)(&out),SortFunc);}


  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::ds::KdTree<" << typestring<T>() << "," << DC << "," << typestring<DT>() << ">");
    return ret;
   }


 private:
  static real64 SortFunc(const void * item,nat32 dim)
  {
   return SO((T*)item,dim);
  }
  
  static void DelFunc(void * ptr)
  {
   DT::Kill((T*)ptr);
  } 
};

//------------------------------------------------------------------------------
// Code for inplace version of kd-tree...
class EOS_CLASS InplaceKdTreeCode
{
 protected:
   InplaceKdTreeCode(); // Constructed to be size 0.
  ~InplaceKdTreeCode();


  // Structure for storing each node...
   struct Node // Malloced.
   {
    Node * self; // Used during the Build process, ignore at all other times. (For finding the mode.)
    Node * left;
    Node * right;
    // Followed by dimensions times sizeof(real32) bytes as position in space.
    // Followed by data for the contained object.
    
    real32 & Pos(nat32 ind) {return ((real32*)(this+1))[ind];}
    void * Obj(nat32 dims) {return ((byte*)(this+1)) + sizeof(real32)*dims;}
    
    // Methods...
     void NearestMan(nat32 dims,nat32 elementSize,const real32 * pos,nat32 depth,real32 & dist,Node *& bsf);
     void NearestEuc(nat32 dims,nat32 elementSize,const real32 * pos,nat32 depth,real32 & dist,Node *& bsf);
   };


  // Methods...
   void Resize(nat32 dims,nat32 elementSize,nat32 newSize);
   
   void SelfSwap(nat32 nodeSize,int32 aInd,int32 bInd)
   {
    Node * a = (Node*)(void*)(data + nodeSize*aInd);
    Node * b = (Node*)(void*)(data + nodeSize*bInd);
    math::Swap(a->self,b->self);
   }
   
   Node * Build(nat32 dims,nat32 elementSize,nat32 depth,nat32 min,nat32 max);
   void BuildAll(nat32 dims,nat32 elementSize);
   
   Node * NearestMan(nat32 dims,nat32 elementSize,const real32 * pos,real32 * dist);
   Node * NearestEuc(nat32 dims,nat32 elementSize,const real32 * pos,real32 * dist);
   Node * Index(nat32 dims,nat32 elementSize,nat32 index);
   

  // Variables...
   nat32 dataSize; // Size of data block, number of nodes.
   bit shrink; // If true shrink the data block by re-allocation, else just keep the older but oversized data block.

   nat32 size; // # nodes that are in data block and in use. shrink==true implies size==dataSize, else size<=dataSize.
   byte * data; // The data block, size of node (inc extras) * dataSize bytes.

   Node * top; // Pointer to root of kd-tree, null if not built.
};

//------------------------------------------------------------------------------
/// An inplace kd-tree, where you tell it how many items you have and it 
/// provides internal storage, you then assign all the objects and store the
/// positions of the objects and call the Build method, at which point it 
/// generates a balanced kd-tree in O(n log n) time.
///
/// Note that the coordinates are external to the objects, in storage provided 
/// by this data structure. This structure can be switched into non-shrinking
/// mode, so that it doesn't reallocate when the buffer is resized to something
/// smaller. Because its inplace you can also access the items as an array, for
/// iterating them all if need be.
///
/// You can resize the structure, add more items if it got larger and re-build,
/// without any items indexes changing, so this doubles as a usable array with
/// associated n dimensional coordinates.
///
/// Note that this has no construction/destruction of data, it is assumed that
/// you will do that yourself. This has been done as this is a class designed
/// for speed in very particular situations, where such stuff will only cause
/// slow down.
///
/// The templated parameters are:
/// - T - the type of the contained object.
/// - DC - the number of dimensions.
template <typename T,nat32 DC>
class EOS_CLASS InplaceKdTree : public InplaceKdTreeCode
{
 public:
  /// sz is the starting size of the array.
   InplaceKdTree(nat32 sz = 0) {if (sz!=0) Resize(DC,sizeof(T),sz);}

  /// &nbsp;  
   ~InplaceKdTree() {}

  
  /// &nbsp;
   nat32 Size() const {return size;}
   
  /// Returns the new size.
   nat32 Size(nat32 sz) {Resize(DC,sizeof(T),sz); return size;}
   
  /// Number of bytes used, excluding any data pointe to.
   nat32 Memory() const {return sizeof(*this) + dataSize * (sizeof(Node) + DC*sizeof(real32) + sizeof(T));}

  
  /// &nbsp;
   T & operator[] (nat32 ind) {return *(T*)(Index(DC,sizeof(T),ind)->Obj(DC));}

  /// &nbsp;
   const T & operator[] (nat32 ind) const {return *(T*)(Index(DC,sizeof(T),ind)->Obj(DC));}
   
   
  /// Outputs the position of a given index.
   void GetPos(nat32 ind,math::Vect<DC,real32> & pos)
   {
    Node * targ = Index(DC,sizeof(T),ind);
    for (nat32 i=0;i<DC;i++) pos[i] = targ->Pos(i);
   }
  
  /// Sets the position of a given index, will un-build the kd-tree.
   void SetPos(nat32 ind,const math::Vect<DC,real32> & pos)
   {
    Node * targ = Index(DC,sizeof(T),ind);
    for (nat32 i=0;i<DC;i++) targ->Pos(i) = pos[i];
   }

  
  /// If true then resizing the array smaller will re-allocate a smaller block 
  /// and delete the old larger one. If false it will simply reuse the large block 
  /// and waste memory, but save time.
  /// Defaults to true.
   bit Shrinks() const {return shrink;}
   
  /// Alows you to set whether it shrinks its internal memory or not.
   bit Shrinks(bit s) {shrink = s; return shrink;}

  
  /// Returns true if the kd-tree has bee built, and the nearest methods can be used.
   bit Built() const {return top!=null<Node*>();}
  
  /// Builds the kd-tree.
   void Build() {LogTime("eos::ds::InplaceKdTree::Build"); BuildAll(DC,sizeof(T));}


  /// Given a coordinate finds the index of the nearest node using manhattan distance.
   nat32 NearestMan(const math::Vect<DC,real32> & pos,real32 * dist = null<real32*>())
   {
    LogTime("eos::ds::InplaceKdTree::NearestMan");
    byte * targ = (byte*)(void*)InplaceKdTreeCode::NearestMan(DC,sizeof(T),pos.Ptr(),dist);
    nat32 nodeSize = sizeof(Node) + DC*sizeof(real32) + sizeof(T);
    return (targ-data)/nodeSize;
   }
   
  /// Given a coordinate finds the index of the nearest node using euclidean distance.
   nat32 NearestEuc(const math::Vect<DC,real32> & pos,real32 * dist = null<real32*>())
   {
    LogTime("eos::ds::InplaceKdTree::NearestEuc");
    byte * targ = (byte*)(void*)InplaceKdTreeCode::NearestEuc(DC,sizeof(T),pos.Ptr(),dist);
    nat32 nodeSize = sizeof(Node) + DC*sizeof(real32) + sizeof(T);
    return (targ-data)/nodeSize;    
   }


  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::ds::InplaceKdTree<" << typestring<T>() << "," << DC << ">");
    return ret;
   }
};

//------------------------------------------------------------------------------
 };
};
#endif
