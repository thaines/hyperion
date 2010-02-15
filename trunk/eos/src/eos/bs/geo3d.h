#ifndef EOS_BS_GEO3D_H
#define EOS_BS_GEO3D_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file geo3d.h
/// Provides 3d geometric primatives, such as vertices, planes and spheres.

#include "eos/types.h"
#include "eos/math/functions.h"
#include "eos/math/vectors.h"
#include "eos/math/mat_ops.h"

namespace eos
{
 namespace bs
 {
//------------------------------------------------------------------------------
/// A Vertex, non-homogenous, inherits from and hence provides the 
/// functionality of the Vect class.
class EOS_CLASS Vert : public math::Vect<3>
{
 public:
  /// Leaves the contained data random.
   Vert() {}

  /// &nbsp;
   Vert(real32 x,real32 y,real32 z)
   {
    (*this)[0] = x; (*this)[1] = y; (*this)[2] = z;
   }

  /// &nbsp;
   Vert(const Vert & rhs):math::Vect<3>(rhs) {}

  /// &nbsp;
   Vert(const math::Vect<3> & rhs):math::Vect<3>(rhs) {}

  /// &nbsp;
   Vert(const class Vertex & rhs);

  /// &nbsp;
   ~Vert() {}


  /// &nbsp;
   real32 & X() {return (*this)[0];}

  /// &nbsp;
   real32 & Y() {return (*this)[1];}

  /// &nbsp;
   real32 & Z() {return (*this)[2];}


  /// &nbsp;
   const real32 & X() const {return (*this)[0];}

  /// &nbsp;
   const real32 & Y() const {return (*this)[1];}

  /// &nbsp;
   const real32 & Z() const {return (*this)[2];}


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::bs::Vert";}
};

//------------------------------------------------------------------------------
/// A surface normal, nothing special really, just a seperate type to distinguish
/// it from a normal Vert. This can be used when saving/loading it, as a normal 
/// only has 2 degrees of freedom so we can omit the Z().
class EOS_CLASS Normal : public Vert
{
 public:
  /// Leaves the contained data random.
   Normal() {}

  /// &nbsp;
   Normal(real32 x,real32 y,real32 z)
   {
    (*this)[0] = x; (*this)[1] = y; (*this)[2] = z;
   }

  /// &nbsp;
   Normal(const Vert & rhs):Vert(rhs) {}

  /// &nbsp;
   Normal(const math::Vect<3> & rhs):Vert(rhs) {}

  /// &nbsp;
   ~Normal() {}  


  /// &nbsp;
   Normal & operator = (const Vert & rhs)
   {(*this)[0] = rhs[0]; (*this)[1] = rhs[1]; (*this)[2] = rhs[2]; return *this;}

  /// &nbsp;
   Normal & operator = (const math::Vect<3> & rhs)
   {(*this)[0] = rhs[0]; (*this)[1] = rhs[1]; (*this)[2] = rhs[2]; return *this;}


  /// Sets the normal to be its reflection in the plane specified by the
  /// given plane normal.
   void Reflect(const Normal & plane)
   {
    Normal sub = plane;
    sub *= 2.0 * ((*this) * plane);
    *this -= sub;
   }
  

  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::bs::Normal";}
};

//------------------------------------------------------------------------------
/// A Vertex, homogenous in nature, inherits from and hence provides the 
/// functionality of the Vect class.
class EOS_CLASS Vertex : public math::Vect<4>
{
 public:
  /// Leaves the contained data random.
   Vertex() {}

  /// &nbsp;
   Vertex(real32 x,real32 y,real32 z,real32 w = 1.0)
   {
    (*this)[0] = x; (*this)[1] = y; (*this)[2] = z; (*this)[3] = w;
   }

  /// &nbsp;
   Vertex(const Vert & rhs)
   {
    (*this)[0] = rhs[0]; (*this)[1] = rhs[1]; (*this)[2] = rhs[2]; (*this)[3] = 1.0;
   }

  /// &nbsp;
   Vertex(const Vertex & rhs):math::Vect<4>(rhs) {}

  /// &nbsp;
   ~Vertex() {}
   
   
  /// &nbsp;
   Vertex & operator = (const Vertex & rhs)
   {for (nat32 i=0;i<4;i++) (*this)[i] = rhs[i]; return *this;}

  /// &nbsp;
   template <typename T>
   Vertex & operator = (const math::Vect<4,T> & rhs)
   {for (nat32 i=0;i<4;i++) (*this)[i] = rhs[i]; return *this;}


  /// &nbsp;
   real32 & X() {return (*this)[0];}

  /// &nbsp;
   real32 & Y() {return (*this)[1];}

  /// &nbsp;
   real32 & Z() {return (*this)[2];}

  /// &nbsp;
   real32 & W() {return (*this)[3];}


  /// &nbsp;
   const real32 & X() const {return (*this)[0];}

  /// &nbsp;
   const real32 & Y() const {return (*this)[1];}

  /// &nbsp;
   const real32 & Z() const {return (*this)[2];}

  /// &nbsp;
   const real32 & W() const {return (*this)[3];}


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::bs::Vertex";}
};

//------------------------------------------------------------------------------
/// A Vertex and Normal glued together - an oriented point in space.
class EOS_CLASS PosDir
{
 public:
  /// &nbsp;
   Vertex pos;

  /// &nbsp;
   Normal dir;
};

//------------------------------------------------------------------------------
/// A 2D texture coordinate, a refinement of the Vect class.
class EOS_CLASS Tex2D : public math::Vect<2>
{
 public:
  /// Leaves the contained data random.
   Tex2D() {}
   
  /// &nbsp;
   Tex2D(real32 u,real32 v) {(*this)[0] = u; (*this)[1] = v;}
   
  /// &nbsp;
   ~Tex2D() {}
   
   
  /// &nbsp;
   real32 & U() {return (*this)[0];}

  /// &nbsp;
   real32 & V() {return (*this)[1];}


  /// &nbsp;
   const real32 & U() const {return (*this)[0];}

  /// &nbsp;
   const real32 & V() const {return (*this)[1];}

  
  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::bs::Tex2D";} 
};

//------------------------------------------------------------------------------
/// Defines a line in terms of a point in 3D space through which it passes and
/// a normal along which it travels, with position 0 on the line being defined
/// as at the given point and position 1 being at the point plus the normal.
/// (In this case n dosn't have to be of normalised length.)
class EOS_CLASS Line
{
 public:
  /// Leaves the contents totaly stochastic.
   Line() {}

  /// &nbsp;
   ~Line() {}


  /// Outputs a vector that is at the given position along the line.
   void GetVertex(real32 pos,Vert & out) 
   {
    out = n;
    out *= pos;
    out += p;
   }


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::bs::Line";}


  /// The vertex through which it passes.
   Vert p;

  /// The normal along which it travels.
   Normal n;
};

//------------------------------------------------------------------------------
/// A half-line, refered to as a Ray as that is ushally what it represents and 
/// is less of a mouthful. Simply a line that is infinite in one direction but
/// finite in another.
class EOS_CLASS Ray
{
 public:
  /// Does no initialisation.
   Ray() {}
   
  /// &nbsp;
   ~Ray() {}


  /// Outputs the location that you obtain if you travel the given distance
  /// from the start. As a ray this is a depth to position function.
   void Travel(real32 dist,Vert & out)
   {
    out = n;
    out *= dist;
    out += s;
   }


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::bs::Ray";}

 
  /// The start of the line - the finite end point.
   Vert s;
   
  /// The direction the line travels from the final point, to infinity.
  /// This should be kept normalised at all time, as its a useful assumption 
  /// for various operations on this class.
   Normal n;
};

//------------------------------------------------------------------------------
/// A line with endpoints.
class EOS_CLASS FiniteLine
{
 public:
  /// Leaves the data random.
   FiniteLine() {}
   
  /// &nbsp;
   ~FiniteLine() {}


  /// Interpolates along the line to output a point on it, a t of 0
  /// will get the start whilst a t of 1 will get the end.
   void Point(real32 t,Vert & out) const
   {
    out = e;
    out -= s;
    out *= t;
    out += s;
   }
   


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::bs::FiniteLine";}

 
  /// The start.
   Vert s;

  /// The end.
   Vert e;
};

//------------------------------------------------------------------------------
/// Defines a plane in terms of a*x + b*y + c = z, a limited definition but has 
/// its uses for particular algorithms etc. Ushally the straight Plane definition
/// will be used.
class EOS_CLASS PlaneABC
{
 public:
  /// Leaves the contained data random.
   PlaneABC() {}

  /// &nbsp;
   PlaneABC(real32 aa,real32 bb,real32 cc):a(aa),b(bb),c(cc) {}

  /// &nbsp;
   PlaneABC(const PlaneABC & rhs):a(rhs.a),b(rhs.b),c(rhs.c) {}

  /// &nbsp;
   ~PlaneABC() {}


  /// &nbsp;
   real32 Z(real32 x,real32 y) const {return a*x + b*y + c;}


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::bs::PlaneABC";}


  /// &nbsp;
   real32 a;

  /// &nbsp;
   real32 b;

  /// &nbsp;
   real32 c;  
};

//------------------------------------------------------------------------------
/// Defines a plane in terms of its normal and distance from the origin, a far 
/// more useful definition than the standard abc one for most actual operations.
/// The extra degree of freedom also allows the direction of facing of the plane
/// to be defined, so the concepts of in front of and behind become meaningful.
/// The definition is that for any point x on the plane dot(n,x) + d = 0.
class EOS_CLASS Plane
{
 public:
  /// Leaves the contained data random.
   Plane() {}

  /// &nbsp;
   Plane(const PlaneABC & rhs)
   {
    // Calculate 3 points on the plane...
     Vert p1(0,0,rhs.Z(0,0));
     Vert p2(1,0,rhs.Z(1,0));
     Vert p3(0,1,rhs.Z(0,1));

    // Generate 2 vectors and use them to create the normal...
     Vert v1(p1); v1 -= p2; v1.Normalise();
     Vert v2(p1); v2 -= p3; v2.Normalise();
     math::CrossProduct(v1,v2,n);

    // Work out d using one of the points...
     d = -(p1*n);
   }

  /// &nbsp;
   Plane(const Plane & rhs):n(rhs.n),d(rhs.d) {}

  /// &nbsp;
   ~Plane() {}


  /// So it acts like a vector.
   nat32 Size() const {return 4;}

  /// So it acts like a vector.
   real32 & operator[] (nat32 i)
   {
    if (i<3) return n[i];
        else return d;
   }


  /// Distance of a point to the plane...
   real32 PointDistance(const Vert & rhs) const {return rhs*n + d;}
   
  /// Intercepts the line created by two given homogenous points with the plane
  /// to give an output homogenous point. Will create points at infinity given a 
  /// parallel line, if both points intercept the plane you should get (0,0,0,0),
  /// though given numerical error it is best to check for that scenario.
   template <typename T>
   void LineIntercept(const math::Vect<4,T> & a,const math::Vect<4,T> & b,math::Vect<4,T> & out) const
   {
    math::Mat<4,4,T> plucker;
    math::Plucker(a,b,plucker);
    
    math::Vect<4,T> pl;
     pl[0] = n[0];
     pl[1] = n[1];
     pl[2] = n[2];
     pl[3] = d;
    math::MultVect(plucker,pl,out);
   }
   
  /// Normalises the normal, adjusting the distance as needed.
  /// Makes the distance positive.
   void Normalise()
   {
    real32 mult = math::InvSqrt(n.LengthSqr());
    if (d<0.0) mult *= -1.0;
    n *= mult;
    d *= mult;
   }


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::bs::Plane";}


  /// The normal of the plane.
   Normal n;

  /// The distance along the normal from the origin to the plane.
   real32 d;
};

//------------------------------------------------------------------------------
/// Defines a Sphere, standard centre radius representation.
class EOS_CLASS Sphere
{
 public:
  /// Does no actual construction.
   Sphere() {}
   
  /// &nbsp;
   ~Sphere() {} 


  /// Returns true if the given vertex is inside the sphere.
   bit Inside(const Vert & v) const
   {
    Vert vert = v;
    vert -= c;
    return vert.LengthSqr()<math::Sqr(r);
   }

  /// Returns true if the given ray intercepts the sphere.
  /// Can also output the distance along the line to the intercept point.
   bit Intercept(const Ray & ray,real32 * dist = null<real32*>()) const
   {
    real32 lambda = (ray.n * c) - (ray.n * ray.s);
        
    Normal offset = ray.n;
    offset *= lambda;
    offset += ray.s;
    offset -= c;
    
    real32 cDstSqr = offset.LengthSqr();
    if (cDstSqr<math::Sqr(r))
    {
     if ((lambda>0.0)&&(!math::Equal(lambda,real32(0.0))))
     {
      // Must intercept sphere...
       if (dist)
       {
        real32 halfDist = r*math::Cos((math::pi*math::Sqrt(cDstSqr))/(2.0*r));
        *dist = lambda-halfDist;
        if ((*dist<=0.0)||(math::Equal(*dist,real32(0.0)))) *dist = lambda+halfDist;
       }
       return true; 
     }
     else
     {
      // We know the line intercepts, and we know where, what we don't know
      // is if the half-line intercepts...
       real32 halfDist = r*math::Cos((math::pi*math::Sqrt(cDstSqr))/(2.0*r));
       lambda += halfDist;

       if ((lambda>0.0)&&(!math::Equal(lambda,real32(0.0))))
       {
        if (dist) *dist = lambda;
        return true;
       }
       else return false;
     }
    }
    else return false;
   }
   
  /// Returns true if the given ray intercepts the sphere, if it does intercept
  /// it outputs the point of interception closest to the start of the ray. 
  /// Note that if the ray starts inside the sphere this will be where the ray 
  /// exits the sphere.
   bit Intercept(const Ray & ray,real32 & dist,bs::Vert & where) const
   {
    real32 lambda = (ray.n * c) - (ray.n * ray.s);
        
    where = ray.n;
    where *= lambda;
    where += ray.s;
    where -= c;
    
    real32 cDstSqr = where.LengthSqr();
    if (cDstSqr<math::Sqr(r))
    {
     real32 halfDist = r*math::Cos((math::pi*math::Sqrt(cDstSqr))/(2.0*r));
     
     dist = lambda-halfDist;
     if ((dist<=0.0)||(math::Equal(dist,real32(0.0))))
     {
      dist = lambda+halfDist;
      if ((dist<=0.0)||(math::Equal(dist,real32(0.0)))) return false;
     }

     where = ray.n;
     where *= dist;
     where += ray.s;
     return true;
    }
    else return false;
   }
   

  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::bs::Sphere";}


  /// Centre of sphere.
   Vert c;
   
  /// Radius of sphere.
   real32 r;
};

//------------------------------------------------------------------------------
/// Defines a Box, a cuboid with arbitary rotation.
/// The representation is not anywhere near as compact as it could be, but its 
/// efficient for various intersection tests, and actualy allows for 
/// representation of some non-box but potentially useful shapes.
/// Essentially based on the idea of defining a local coordinate system in which
/// the box sits axis aligned going from (0,0,0) to (1,1,1).
class EOS_CLASS Box
{
 public:
 
  /// The -ve corner of the cuboid, i.e. if the cuboid fills up an axis aligned
  /// volume from (0,0,0) to (1,1,1) in a 'box coordinate system' then this
  /// is the position of the (0,0,0) point.
   Vert c;

  /// Gives the 3 axis, one each for x,y and z respectivly. The lengths of the axis are
  /// the lengths of the sides of the cuboid.
  /// All edges should be perpendicular for it to represent a box.
   math::Vect<3> e[3];
};

//------------------------------------------------------------------------------
// All the stuff that has to be defined out-of-order due to circular dependencies...
inline Vert::Vert(const class Vertex & rhs)
{
 real32 we = 1.0/rhs[3];
 (*this)[0] = rhs[0]*we;
 (*this)[1] = rhs[1]*we;
 (*this)[2] = rhs[2]*we;
}

//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
// I/O functions...
namespace eos
{
 namespace io
 {


  template <typename T>
  inline T & StreamRead(T & lhs,eos::bs::Line & rhs,Binary)
  {
   lhs >> rhs.p >> rhs.n;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::Line & rhs,Binary)
  {
   lhs << rhs.p << rhs.n;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::Line & rhs,Text)
  {
   lhs << rhs.p << "->" << rhs.n;
   return lhs;
  }


  template <typename T>
  inline T & StreamRead(T & lhs,eos::bs::Ray & rhs,Binary)
  {
   lhs >> rhs.s >> rhs.n;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::Ray & rhs,Binary)
  {
   lhs << rhs.s << rhs.n;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::Ray & rhs,Text)
  {
   lhs << rhs.s << ">>" << rhs.n;
   return lhs;
  }


  template <typename T>
  inline T & StreamRead(T & lhs,eos::bs::FiniteLine & rhs,Binary)
  {
   lhs >> rhs.s >> rhs.e;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::FiniteLine & rhs,Binary)
  {
   lhs << rhs.s << rhs.e;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::FiniteLine & rhs,Text)
  {
   lhs << rhs.s << "<->" << rhs.e;
   return lhs;
  }


  template <typename T>
  inline T & StreamRead(T & lhs,eos::bs::PlaneABC & rhs,Binary)
  {
   lhs >> rhs.a >> rhs.b >> rhs.c;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::PlaneABC & rhs,Binary)
  {
   lhs << rhs.a << rhs.b << rhs.c;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::PlaneABC & rhs,Text)
  {
   lhs << "<" << rhs.a << "," << rhs.b << "," << rhs.c << ">";
   return lhs;
  }


  template <typename T>
  inline T & StreamRead(T & lhs,eos::bs::Plane & rhs,Binary)
  {
   lhs >> rhs.n >> rhs.d;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::Plane & rhs,Binary)
  {
   lhs << rhs.n << rhs.d;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::Plane & rhs,Text)
  {
   lhs << rhs.n << ":" << rhs.d;
   return lhs;
  }


  template <typename T>
  inline T & StreamRead(T & lhs,eos::bs::Sphere & rhs,Binary)
  {
   lhs >> rhs.c >> rhs.r;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::Sphere & rhs,Binary)
  {
   lhs << rhs.c << rhs.r;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::Sphere & rhs,Text)
  {
   lhs << rhs.c << ">" << rhs.r;
   return lhs;
  }


  template <typename T>
  inline T & StreamRead(T & lhs,eos::bs::Box & rhs,Binary)
  {
   lhs >> rhs.c >> rhs.e[0] >> rhs.e[1] >> rhs.e[2];
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::Box & rhs,Binary)
  {
   lhs << rhs.c << rhs.e[0] << rhs.e[1] << rhs.e[2];
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::bs::Box & rhs,Text)
  {
   lhs << rhs.c << "{" << rhs.e[0] << "," << rhs.e[1] << "," << rhs.e[2] << "}";
   return lhs;
  }


 };      
};
//------------------------------------------------------------------------------
#endif
