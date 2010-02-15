#ifndef EOS_DS_VORONOI_H
#define EOS_DS_VORONOI_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file voronoi.h
/// Provides both Voronoi diagrams and Delaunay tessalations. Actually does
/// Delaunay primarilly, just called Voronoi as he came up with the idea first.

#include "eos/types.h"
#include "eos/typestring.h"
#include "eos/mem/safety.h"
#include "eos/math/mat_ops.h"
#include "eos/ds/arrays.h"

namespace eos
{
 namespace ds
 {
//------------------------------------------------------------------------------
// Code for the Delauney flat template...
// (Warning - this is a rather hotch potch implimentation, as I was working out
// how do it as I coded - there are loads of assumed invariants which I have not
// stated, so changing this code problably means a re-write.)
// (And yes, its a fucking awful mess.)
class EOS_CLASS Delaunay2DCode
{
 protected:
  class Triangle;

  // Structure used to contain a site, the actual data 'T' is appended to the end...
   struct Site // malloc-ed.
   {
    Site * next; // Linked list of Sites, for cleanup and complete list purposes.

    real64 x;
    real64 y;

    nat32 neighbours; // Number of neighbour triangles, excludes old once.
    Triangle * tri; // Pointer to a linked list of triangles that point to it, all of them.

    bit infinite; // If true its an infinite point, consider x and y to be a vector pointing to the infinite points position.

    void * Data() const {return ((byte*)this) + sizeof(Site);}
   };

  // Triangle data structure...
   struct Triangle // new-ed.
   {
    void DelChildren();

    Triangle * FindTri(real64 x,real64 y) const; // Returns the containing triangle for the point, recursive, assumes this triangle contains the given point.

    // Checks if the given vertex in this is close enough to its opposing edge
    // to require a split, if so it does the split...
    // (And recurses as needed.)
     void MabyFlip(Delaunay2DCode * parent,nat32 ind);

    // Little helper function used be various bits of code.
    // Returns true if (x,y) is on the inside assuming a counter-clockwise line arround
    // a triangle. Handles infinite points correctly but only if the second point alone
    // is infinite....
     static bit Side(Site * a,Site * b,real64 x,real64 y);

    Site * vert[3]; // The 3 verticies that make it up, in counter-clockwise order. If vert[i]==null then its an infinite point.

    // Number of child triangles, if 0 then it is part of the final triangulation, otherwise its an
    // old node that just exists for its assistance in searching...
     nat32 children;
     Triangle * child[3]; // Maximum of 3 children is all thats possible.
     bit delChildren; // True if it should delete its children.

    // entry 0 points to the triangle on the other side of the vert[0] to vert[1] edge, etc. If either
    // vert is infinite then its partner is null, otherwise it is not. For nodes with children==0
    // partner allways points to other nodes with children==0...
     Triangle * partner[3];

    Triangle * next[3]; // Next node in the 3 linked lists of triangles, corresponding to the vert[3] array.
   };



  // Constructs it empty...
   Delaunay2DCode();

  // Does not actually destroy anything...
   ~Delaunay2DCode();


  // Emptys the object, calling the relevent termination function on all the data.
   void Del(void (*Term)(void * ptr));

  // Calls Del and then does the constructor.
   void Reset(void (*Term)(void * ptr));



  // Adds a new node in, returning the resulting Site...
   Site * Add(nat32 elementSize,real64 x,real64 y,const void * ptr);

  // Finds the nearest Site to a given position...
   Site * Nearest(real64 x,real64 y) const;


  Triangle root; // The root triangle - the only infinite one, so it can contain all the other triangles.
  Site * first; // Linked list of all the Sites.

  nat32 sites; // Number of sites contained.
  nat32 triangles; // Number of triangles in the final triangulation.
  nat32 oldTriangles; // Number of old triangles still floating about for indexing purposes, excludes the infinite one.
};

//------------------------------------------------------------------------------
/// Provides an incrimental 2D Delaunay triangulation. Allows for the addition
/// of points to the triangulation in O(log n) time on average, so O(n log n) to
/// build a complete diagram with n sites using this data structure. Implimented
/// from the 'Randomized Incremental Construction of Delaunay and Voronoi Diagrams'
/// by Guibas, Knuth & Sharir. In addition this provides a nearest neighbour
/// capability as a nice side affect of the algorithms data structures.
/// The data structure used internally is essentially an
/// unbalanced tree, which would be extremelly hard to balance, it also stores
/// excess information due to the incrimental nature, so extracting a final
/// result into a graph is recomended for actual storage and certain kinds of
/// processing. It also allows you to use it as a Voronoi diagram, by providing
/// a suitable interface to the data in addition to the Delaunay style interface.
/// Whilst this method is not slower than other possible methods by complexity it
/// does have a poor inner loop, so faster is possible if you just want to create
/// the triangulation without the incrimental capability of this one.
template <typename T, typename DT = mem::KillNull<T> >
class EOS_CLASS Delaunay2D : public Delaunay2DCode
{
 public:
  class Mid; // Predeclaration.

  /// A class to represent each added location in the data structure, allows
  /// access to the contained object and its assigned coordinate, but also
  /// querying of surrounding positions that are linked to it and access to
  /// the Voronoi structure. Just an encapsulated pointer - can be copied raw.
   class Pos
   {
    public:
     Pos(Delaunay2DCode::Site * s):site(s) {}
     Site * GetSite() const {return site;}

     /// &nbsp;
      real64 X() const {return site->x;}

     /// &nbsp;
      real64 Y() const {return site->y;}


     /// A completly arbitary sorting operator. This is used to only consider
     /// each pair of neighbouring Pos once.
      bit operator < (const Pos & rhs) const {return site<rhs.site;};


     /// &nbsp;
      T & operator* () {return *static_cast<T*>(site->Data());}

     /// &nbsp;
      const T & operator* () const {return *static_cast<T*>(site->Data());}

     /// &nbsp;
      T * operator->() {return static_cast<T*>(site->Data());}

     /// &nbsp;
      const T * operator->() const {return static_cast<T*>(site->Data());}


     /// Returns how many neighbouring nodes surround it, its the same
     /// number for both the Delaunay and Voronoi, obviously. Note however
     /// that it includes infinite points, so calls to GetPos/GetMid can
     /// output less data.
      nat32 Neighbours() const {return site->neighbours;}

     /// Outputs an array of all neighbouring Pos in the Delaunay triangulation.
     /// In counter-clockwise order from an arbitary starting point.
      void GetPos(ds::Array<Delaunay2D<T,DT>::Pos> & out);

     /// Outputs an array of all surrounding Mid in the Voronoi tesselation that
     /// make up polygon containing all points closest to this Pos.
     /// In counter-clockwise order from an arbitary starting point.
      void GetMid(ds::Array<Delaunay2D<T,DT>::Mid> & out);

     /// &nbsp;
      static inline cstrconst TypeString()
      {
       static GlueStr ret(GlueStr() << "eos::ds::Delaunay2D<" << typestring<T>() << "," << typestring<DT>() << ">::Pos");
       return ret;
      }


    private:
     Delaunay2DCode::Site * site;
   };

  /// A class to represent the vertices of the Voronoi tesselation, the
  /// points that are equidistant from multiple positions in the data structure.
  /// Note that these can become invalid after an Add operation, though they
  /// have a method to query there validity.
   class Mid
   {
    public:
     Mid(Delaunay2DCode::Triangle * t):tri(t) {}

     /// Returns true if its valid, false if is not. This can change after any add operation.
      bit Valid() const {return (tri!=null<Delaunay2DCode::Triangle*>())&&(tri->children==0);}


     /// Returns true if all the Pos are finite, i.e. no infite points are involved
     /// so it has a centre.
      bit HasCentre() const
      {return (!tri->vert[0]->infinite)&&
              (!tri->vert[1]->infinite)&&
              (!tri->vert[2]->infinite);}

     /// Outputs the centre of this Mid, note that this involves some calculation.
     /// Do not call if any of its Pos are infinite, as its no longer calculable.
     /// Will return true on success, false indicates failure, ushally because
     /// the triangle is essentially a line due to points being inserted on top
     /// of one another.
      bit Centre(math::Vect<2,real64> & out) const
      {
       math::Mat<3,3,real64> mat;

       mat[0][0] = tri->vert[0]->x; mat[0][1] = tri->vert[0]->y; mat[0][2] = 1.0;
       mat[1][0] = tri->vert[1]->x; mat[1][1] = tri->vert[1]->y; mat[1][2] = 1.0;
       mat[2][0] = tri->vert[2]->x; mat[2][1] = tri->vert[2]->y; mat[2][2] = 1.0;
       real64 a = Determinant(mat);
       if (math::Equal(a,real64(0.0))) return false;
       a = 0.5/a;

       real32 vl0 = math::Sqr(tri->vert[0]->x) + math::Sqr(tri->vert[0]->y);
       real32 vl1 = math::Sqr(tri->vert[1]->x) + math::Sqr(tri->vert[1]->y);
       real32 vl2 = math::Sqr(tri->vert[2]->x) + math::Sqr(tri->vert[2]->y);

       mat[0][0] = vl0; mat[0][1] = tri->vert[0]->y; mat[0][2] = 1.0;
       mat[1][0] = vl1; mat[1][1] = tri->vert[1]->y; mat[1][2] = 1.0;
       mat[2][0] = vl2; mat[2][1] = tri->vert[2]->y; mat[2][2] = 1.0;
       out[0] = a*Determinant(mat);

       mat[0][0] = tri->vert[0]->x; mat[0][1] = vl0; mat[0][2] = 1.0;
       mat[1][0] = tri->vert[1]->x; mat[1][1] = vl1; mat[1][2] = 1.0;
       mat[2][0] = tri->vert[2]->x; mat[2][1] = vl2; mat[2][2] = 1.0;
       out[1] = a*Determinant(mat);

       return true;
      }

     /// If the Mid has no centre then you might still want to output the vector
     /// to infinity that goes from a neighbouring Mid to this one. Given a
     /// neighbouring Mid with a centre this outputs the direction that a line
     /// from that Mid's centre to infinity should follow.
      void Direction(Mid n,math::Vect<2,real64> & dir) const
      {
       // Identify the two neighbouring points by finding the partner index...
        nat32 partInd = 0;
        for (nat32 i=1;i<3;i++) {if (tri->partner[i]==n.tri) {partInd = i; break;}}

       // Output a vector perpendicular to the line between them...
        dir[0] = -(tri->vert[(partInd+1)%3]->y - tri->vert[partInd]->y);
        dir[1] = (tri->vert[(partInd+1)%3]->x - tri->vert[partInd]->x);
        dir.Normalise();
      }


     /// Each mid point is where 3 Pos meet, given an index from 0..2 this returns if
     /// that particular point is infinite.
      bit Infinite(nat32 i) const {return tri->vert[i]->infinite;}

     /// If !Infinite(i) then this returns the Pos for that point. i should be 0..2.
      Pos GetPos(nat32 i) const {return Pos(tri->vert[i]);}

     /// Returns one of the 3 partners connected to it, will return an Invalid point if
     /// there is no partner for a given index. i can be 0..2.
      Mid GetNeighbour(nat32 i) const {return Mid(tri->partner[i]);}


     /// Allows you to ask for the next voronoi vertex clockwise arround a given point,
     /// which must be obtainable from GetPos.
      Mid Clockwise(Pos pos) const
      {
       nat32 ind = 0;
       for (nat32 i=1;i<3;i++) {if (tri->vert[i]==pos.GetSite()) {ind = i; break;}}
       return Mid(tri->partner[ind]);
      }

     /// Allows you to ask for the next voronoi vertex anti-clockwise arround a given point,
     /// which must be obtainable from GetPos.
      Mid AntiClockwise(Pos pos) const
      {
       nat32 ind = 0;
       for (nat32 i=1;i<3;i++) {if (tri->vert[i]==pos.GetSite()) {ind = i; break;}}
       return Mid(tri->partner[(ind+2)%3]);
      }


     /// &nbsp;
      static inline cstrconst TypeString()
      {
       static GlueStr ret(GlueStr() << "eos::ds::Delaunay2D<" << typestring<T>() << "," << typestring<DT>() << ">::Mid");
       return ret;
      }


    private:
     Triangle * tri;
   };


  /// &nbsp;
   Delaunay2D() {}

  /// &nbsp;
   ~Delaunay2D()
   {Del(&DelFunc);}


  // Emptys it all and sets up to be re-filled.
   void Reset() {Delaunay2DCode::Reset(&DelFunc);}


  /// Returns the number of locations stored withing the structure.
   void PosCount() const {return sites;}

  /// Returns the number of vertices as a consequence of the dual structure,
  /// the Voronoi tessalation.
   void MidCount() const {return triangles;}

  /// Returns how much memory the structure is consuming in bytes.
   nat32 Memory() const {return sizeof(Delaunay2D) + sizeof(Delaunay2DCode::Site)*sites + sizeof(Delaunay2DCode::Triangle)*(triangles+oldTriangles);}


  /// Adds a new pos, adjusting the triangulation as required required.
   Pos Add(const math::Vect<2,real32> & pos,const T & obj)
   {return Pos(Delaunay2DCode::Add(sizeof(T),pos[0],pos[1],&obj));}

  /// Adds a new pos, adjusting the triangulation as required required.
   Pos Add(const math::Vect<2,real64> & pos,const T & obj)
   {return Pos(Delaunay2DCode::Add(sizeof(T),pos[0],pos[1],&obj));}

  /// Adds a new pos, adjusting the triangulation as required required.
   Pos Add(real32 x,real32 y,const T & obj)
   {return Pos(Delaunay2DCode::Add(sizeof(T),x,y,&obj));}

  /// Adds a new pos, adjusting the triangulation as required required.
   Pos Add(real64 x,real64 y,const T & obj)
   {return Pos(Delaunay2DCode::Add(sizeof(T),x,y,&obj));}


  /// Given a coordinate returns the nearest Pos. Do not call when PosCount()==0
   Pos Nearest(const math::Vect<2,real32> & pos)
   {return Pos(Delaunay2DCode::Nearest(pos[0],pos[1]));}

  /// Given a coordinate returns the nearest Pos. Do not call when PosCount()==0
   Pos Nearest(const math::Vect<2,real64> & pos)
   {return Pos(Delaunay2DCode::Nearest(pos[0],pos[1]));}

  /// Given a coordinate returns the nearest Pos. Do not call when PosCount()==0
   Pos Nearest(real32 x,real32 y)
   {return Pos(Delaunay2DCode::Nearest(x,y));}

  /// Given a coordinate returns the nearest Pos. Do not call when PosCount()==0
   Pos Nearest(real64 x,real64 y)
   {return Pos(Delaunay2DCode::Nearest(x,y));}


  /// Given a coordinate returns the Mid representing the triangle that it is 
  /// in - you can then query the 3 Pos of the Mid to get the actual traingle,
  /// being aware that any or all of the Pos could be at infinity.
   Mid Triangle(const math::Vect<2,real32> & pos)
   {return Mid(root.FindTri(pos[0],pos[1]));}

  /// Given a coordinate returns the Mid representing the triangle that it is 
  /// in - you can then query the 3 Pos of the Mid to get the actual traingle,
  /// being aware that any or all of the Pos could be at infinity.
   Mid Triangle(const math::Vect<2,real64> & pos)
   {return Mid(root.FindTri(pos[0],pos[1]));}

  /// Given a coordinate returns the Mid representing the triangle that it is 
  /// in - you can then query the 3 Pos of the Mid to get the actual traingle,
  /// being aware that any or all of the Pos could be at infinity.
   Mid Triangle(real32 x,real32 y)
   {return Mid(root.FindTri(x,y));}

  /// Given a coordinate returns the Mid representing the triangle that it is 
  /// in - you can then query the 3 Pos of the Mid to get the actual traingle,
  /// being aware that any or all of the Pos could be at infinity.
   Mid Triangle(real64 x,real64 y)
   {return Mid(root.FindTri(x,y));}


  /// Obtains an array of all Pos in the data structure.
  /// Sets the array size as needed.
   void GetAllPos(ds::Array<Pos> & out)
   {
    out.Size(sites);
    Site * targ = first;
    for (nat32 i=0;i<sites;i++)
    {
     out[i] = Pos(targ);
     targ = targ->next;
    }
   }

  /// Obtains an array of all Mid's, including those that have infinite points.
  /// Sets the array size as needed.
   void GetAllMid(ds::Array<Mid> & out)
   {
    out.Size(triangles);
    nat32 op = 0;
    struct F
    {
     static void Func(Delaunay2DCode::Triangle * targ,ds::Array<Mid> & out,nat32 & op)
     {
      if (targ->children==0)
      {
       out[op] = Mid(targ);
       op += 1;
      }
      else
      {
       if (targ->delChildren)
       {
        for (nat32 i=0;i<targ->children;i++) Func(targ->child[i],out,op);
       }
      }
     }
    };
    F::Func(&root,out,op);
    log::Assert(op==triangles,"eos::ds::Delaunay2D::GetAllMid");
   }


  /// &nbsp;
   static inline cstrconst TypeString()
   {
    static GlueStr ret(GlueStr() << "eos::ds::Delaunay2D<" << typestring<T>() << "," << typestring<DT>() << ">");
    return ret;
   }

 private:
  static void DelFunc(void * ptr)
  {
   DT::Kill((T*)ptr);
  }
};

//------------------------------------------------------------------------------
// Inline functions for the Delaunay2D class...
template <typename T, typename DT>
inline void Delaunay2D<T,DT>::Pos::GetPos(ds::Array<Delaunay2D<T,DT>::Pos> & out)
{
 if (out.Size()<site->neighbours) out.Size(site->neighbours);
 Delaunay2DCode::Triangle * targ = site->tri;
 nat32 oi = 0;
 while (targ)
 {
  // Identify the relevent vertex, so we can goto the next one...
   nat32 ind = 0;
   for (nat32 i=1;i<3;i++) {if (targ->vert[i]==site) {ind = i; break;}}

  // If the triangle is a root triangle grab one of its vertices and add to output...
   if ((targ->children==0)&&(!targ->vert[(ind+1)%3]->infinite))
   {
    out[oi] = Pos(targ->vert[(ind+1)%3]);
    ++oi;
   }

  // Goto the next one...
   targ = targ->next[ind];
 }
 out.Size(oi);
}

template <typename T, typename DT>
inline void Delaunay2D<T,DT>::Pos::GetMid(ds::Array<Delaunay2D<T,DT>::Mid> & out)
{
 out.Size(site->neighbours);
 Delaunay2DCode::Triangle * targ = site->tri;
 nat32 oi = 0;
 while (targ)
 {
  // If its a root triangle add it to the output...
   if (targ->children==0)
   {
    out[oi] = Mid(targ);
    ++oi;
   }

  // Identify the relevent vertex, so we can goto the next one...
   nat32 ind = 0;
   for (nat32 i=1;i<3;i++) {if (targ->vert[i]==site) {ind = i; break;}}

  // Goto the next one...
   targ = targ->next[ind];
 }
 log::Assert(oi==site->neighbours,"eos::ds::Delaunay2D::Pos::GetMid");
}

//------------------------------------------------------------------------------
 };
};
#endif
