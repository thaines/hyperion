#ifndef EOS_REND_RENDERER_H
#define EOS_REND_RENDERER_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file renderer.h
/// This file contains all the abstracted parts of a complete rendering system, 
/// be it a scanline renderer or a global illumination renderer with every
/// feature required to ensure a render is never completed in any human lifetime.
/// Actual implimentations of the various parts can be found elsewhere, but this
/// serves to make sure all the parts will play ball.

#include "eos/types.h"
#include "eos/math/quaternions.h"
#include "eos/ds/arrays.h"
#include "eos/ds/lists.h"
#include "eos/bs/colours.h"
#include "eos/bs/geo2d.h"
#include "eos/bs/geo3d.h"
#include "eos/time/progress.h"
#include "eos/svt/field.h"

namespace eos
{
 namespace rend
 {
//------------------------------------------------------------------------------
/// A coordinate system, or Transform. These ushally form a hierachy,
/// convert locations/orientations/rays from world space to local space in both
/// directions.
/// The object has 3 parts to its transformation - 
/// translation (t), rotation (r) and scale (s); when going from world coordinates 
/// to local coordinates they are applied in that order, this being the order
/// a human being would ushally expect.
/// They are maintained seperatly for storage space, efficiency of composition
/// and because scales must not be applied to orientations,
/// additionally gimbal lock is impossible due to the use of quaternions.
/// This class can not actually apply the transform, it can be converted into
/// a OpTran for this purpose.
class EOS_CLASS Transform
{
 public:
  /// Creates a class full of junk.
   Transform() {}
   
  /// &nbsp;
   Transform(const Transform & rhs)
   :t(rhs.t),r(rhs.r),s(rhs.s)
   {}

  /// &nbsp;
   ~Transform() {}
   
   
  /// &nbsp;
   Transform & operator = (const Transform & rhs)
   {t = rhs.t; r = rhs.r; s = rhs.s; return *this;}

  /// Composites two Transforms, given this going from coordinate system A to coordinate
  /// system B and rhs going from system B to system C this replaces this with a system
  /// going from A to C directly.
   Transform & operator *= (const Transform & rhs)
   {
    math::Vect<3> t2;
    r.Apply(rhs.t,t2);
    t2 *= s;
    
    t += t2;
    r *= rhs.r;
    s *= rhs.s;
    
    return *this;
   }
   
  /// Inverts a transform, so instead of going from coordinate system A to system B
  /// it goes from B to A. Used when back tracking from the camera to the root of the
  /// scene to build up transformations.
   void Invert()
   {
    r.Normalise();
    r.Conjugate();
    s = 1.0/s;

    bs::Vert t2 = t; t2 *= -1.0;
    math::Mat<3> mat;
    r.ToMat(mat);
    math::MultVect(mat,t2,t);
    t *= s;
   }


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::rend::Transform";}


  /// The transformation, from world space to local space.
   bs::Vert t;

  /// The rotation, from world space to local space.
   math::Quaternion r;

  /// The scale multiplier, from world space to local space.
   real32 s;
};

//------------------------------------------------------------------------------
/// The Transform object is designed with one set of requirements, these make 
/// it inappropriate for actually applying transforms however.
/// The optimised transform object cannot be editted or composed, but is 
/// constructed from a Transform object and used to efficiently apply large
/// numbers of transforms. It converts a Transform into several matrices, 
/// to additionally suport inverse transformations and orientation
/// transformations for surface normals.
class EOS_CLASS OpTran
{
 public:
  /// Leaves the class un-initialised.
   OpTran() {}

  /// Note that this is lying about it treating rhs as constant,
  /// it Normalises its rotation, overriding the const, before using it.
   OpTran(const Transform & rhs)
   {
    scale = rhs.s;
   
    const_cast<Transform&>(rhs).r.Normalise();
    rhs.r.ToMat(rot);
    
    real32 invS = 1.0/rhs.s;
    for (nat32 r=0;r<3;r++)
    {
     for (nat32 c=0;c<3;c++)
     {
      tran[r][c] = rot[r][c]*rhs.s;
      invTran[c][r] = rot[r][c]*invS;
     }
    }
    
    for (nat32 i=0;i<3;i++)
    {
     tran[i][3] = rhs.t[i];
     invTran[i][3] = -rhs.t[i];  
    }
    
    for (nat32 i=0;i<3;i++)
    {
     tran[3][i] = 0.0;
     invTran[3][i] = 0.0;
    }
    
    tran[3][3] = 1.0;
    invTran[3][3] = 1.0;
   }

  /// &nbsp;
   OpTran(const OpTran & rhs)
   :scale(rhs.scale),rot(rhs.rot),tran(rhs.tran),invTran(rhs.invTran)
   {}

  /// &nbsp;
   ~OpTran() {}
   
  
  /// &nbsp;
   OpTran & operator = (const OpTran & rhs) 
   {
    scale = rhs.scale;
    rot = rhs.rot;
    tran = rhs.tran;
    invTran = rhs.invTran;
    return *this;
   }

 
  /// This transforms a distance correctly.
   void ToWorld(real32 dist,real32 & out) const
   {
    out = dist*scale;
   }
   
  /// &nbsp;
   void ToWorld(const math::Vect<3> & in,math::Vect<3> & out) const
   {
    out[0] = tran[0][0]*in[0] + tran[0][1]*in[1] + tran[0][2]*in[2] + tran[0][3];
    out[1] = tran[1][0]*in[0] + tran[1][1]*in[1] + tran[1][2]*in[2] + tran[1][3];
    out[2] = tran[2][0]*in[0] + tran[2][1]*in[1] + tran[2][2]*in[2] + tran[2][3];
   }

  /// &nbsp;
   void ToWorld(const math::Vect<4> & in,math::Vect<4> & out) const
   {
    math::MultVect(tran,in,out);
   }
   
  /// &nbsp;
   void ToWorld(const bs::Normal & in,bs::Normal & out) const
   {
    math::MultVect(rot,in,out);
   }
   
  /// &nbsp;
   void ToWorld(const bs::Line & in,bs::Line & out) const
   {
    ToWorld(in.p,out.p);
    ToWorld(in.n,out.n);
   }
   
  /// &nbsp;
   void ToWorld(const bs::Ray & in,bs::Ray & out) const
   {
    ToWorld(in.s,out.s);
    ToWorld(in.n,out.n);
   }
   
  /// &nbsp;
   void ToWorld(const bs::FiniteLine & in,bs::FiniteLine & out) const
   {
    ToWorld(in.s,out.s);
    ToWorld(in.e,out.e);
   }

  /// &nbsp;
   void ToWorld(const bs::Plane & in,bs::Plane & out) const
   {
    ToWorld(in.n,out.n);

    math::Vect<3> loc = in.n;
    loc *= in.d;
    math::Vect<3> nloc;
    ToWorld(loc,nloc);
    out.d = nloc * out.n;
   }

  /// &nbsp;
   void ToWorld(const bs::Sphere & in,bs::Sphere & out) const
   {
    ToWorld(in.c,out.c);
    
    bs::Vert d = in.c;
    d[0] += in.r;
    bs::Vert e;
    ToWorld(d,e);
    e -= out.c;
    out.r = e.Length();
   }

  /// &nbsp;
   void ToWorld(const bs::Box & in,bs::Box & out) const
   {
    ToWorld(in.c,out.c);
    math::MultVect(rot,in.e[0],out.e[0]);
    math::MultVect(rot,in.e[1],out.e[1]);
    math::MultVect(rot,in.e[2],out.e[2]);
   }


  /// This transforms a distance correctly.
   void ToLocal(real32 dist,real32 & out) const
   {
    out = dist/scale;
   }

  /// &nbsp;
   void ToLocal(const math::Vect<3> & in,math::Vect<3> & out) const
   {
    out[0] = invTran[0][0]*in[0] + invTran[0][1]*in[1] + invTran[0][2]*in[2] + invTran[0][3];
    out[1] = invTran[1][0]*in[0] + invTran[1][1]*in[1] + invTran[1][2]*in[2] + invTran[1][3];
    out[2] = invTran[2][0]*in[0] + invTran[2][1]*in[1] + invTran[2][2]*in[2] + invTran[2][3];
   }

  /// &nbsp;
   void ToLocal(const math::Vect<4> & in,math::Vect<4> & out) const
   {
    math::MultVect(invTran,in,out);
   }   
   
  /// &nbsp;
   void ToLocal(const bs::Normal & in,bs::Normal & out) const
   {
    math::TransMultVect(rot,in,out);
   }
   
  /// &nbsp;
   void ToLocal(const bs::Line & in,bs::Line & out) const
   {
    ToLocal(in.p,out.p);
    ToLocal(in.n,out.n);
   }   
   
  /// &nbsp;
   void ToLocal(const bs::Ray & in,bs::Ray & out) const
   {
    ToLocal(in.s,out.s);
    ToLocal(in.n,out.n);
   }
   
  /// &nbsp;
   void ToLocal(const bs::FiniteLine & in,bs::FiniteLine & out) const
   {
    ToLocal(in.s,out.s);
    ToLocal(in.e,out.e);
   }   

  /// &nbsp;
   void ToLocal(const bs::Plane & in,bs::Plane & out) const
   {
    ToLocal(in.n,out.n);

    math::Vect<3> loc = in.n;
    loc *= in.d;
    math::Vect<3> nloc;
    ToLocal(loc,nloc);
    out.d = nloc * out.n;
   }

  /// &nbsp;
   void ToLocal(const bs::Sphere & in,bs::Sphere & out) const
   {
    ToLocal(in.c,out.c);
    
    bs::Vert d = in.c;
    d[0] += in.r;
    bs::Vert e;
    ToLocal(d,e);
    e -= out.c;
    out.r = e.Length();
   }

  /// &nbsp;
   void ToLocal(const bs::Box & in,bs::Box & out) const
   {
    ToLocal(in.c,out.c);
    math::TransMultVect(rot,in.e[0],out.e[0]);
    math::TransMultVect(rot,in.e[1],out.e[1]);
    math::TransMultVect(rot,in.e[2],out.e[2]);
   }


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::rend::OpTran";}


 private:
  // The scaler used, for transforming distances...
   real32 scale;
 
  // The 3x3 rotation matrix, representing just the rotation, for taking
  // orientations from world space to local space. Using its transpose its
  // inverse can be applied.
   math::Mat<3> rot;
   
  // The 4x4 full transformation matrix, for taking coordinates from world 
  // space to local space.
   math::Mat<4> tran;
  
  // The 4x4 inverse full transformation matrix, for taking coordinates from
  // local space to world space.
   math::Mat<4> invTran;
};
      
//------------------------------------------------------------------------------
/// A texture coordinate, this is simply a 3 vector for standardisation,
/// regardless as to if its in fact 1D, 2D or indeed 3D, in limited cases it
/// just provides the avaliable coordinates, setting the others to 0.
/// Also provides a material index, this is ushally set to 0 but can have
/// multiple values to idnicate that different materials be used for different
/// parts of an object. (Used modulus number of assigned materials, so it can
/// not overrun.)
class EOS_CLASS TexCoord : public math::Vect<3>
{
 public:
  /// Does no initialisation.
   TexCoord() {}
   
  /// &nbsp;
   TexCoord(real32 u,real32 v = 0.0,real32 w = 0.0) {(*this)[0] = u; (*this)[1] = v; (*this)[2] = w;}
 
  /// &nbsp; 
   ~TexCoord() {}


  /// &nbsp;
   real32 & U() {return (*this)[0];}

  /// &nbsp;
   real32 & V() {return (*this)[1];}

  /// &nbsp;
   real32 & W() {return (*this)[2];}


  /// &nbsp;
   const real32 & U() const {return (*this)[0];}

  /// &nbsp;
   const real32 & V() const {return (*this)[1];}

  /// &nbsp;
   const real32 & W() const {return (*this)[2];}

      
  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::rend::TexCoord";}
   
   
  /// In addition to a texture coordinate a material index is given,
  /// this allows mutliple materials to be assigned to the same object,
  /// assuming it can have its texture details editted to provide different indexes.
   nat32 material;
};

//------------------------------------------------------------------------------
/// A simple wrapper object for storing the relevent details of an intersection.
/// Note that they are all in local object coordinates when it is first 
/// generated by an object, but the database is responsible for then converting
/// it into world coordiantes.
class EOS_CLASS Intersection
{
 public:
  /// This method is given the transformation for the object which it came from,
  /// it then converts from local coordinates to world coordinates.
   void ToWorld(const OpTran & tran);
 
  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::rend::Intersection";} 


  /// The coordinate where the intersection happens.
   bs::Vert point;
   
  /// How far along the ray the intersection point is.
  /// Stored to avoid repeated calculation.
   real32 depth;
   
  /// The surface orientation where the intersection happens.
   bs::Normal norm;
   
  /// The two axis that define the orientation of the point, should be
  /// perpendicular with the surface normal and each other.
  /// This is to define a complete local coordinate system, needed for
  /// bump maps.
   bs::Normal axis[2];   
   
  /// The object texture coordinate where the intersection happens.
   TexCoord coord;
   
  /// The velocity at the intersection point - can be used for 
  /// post-proccessing effects.
   bs::Vert velocity;
};

//------------------------------------------------------------------------------
/// The Boundable class represents things that have a volume, and hence can have
/// various bounding volumes defined for them.
class EOS_CLASS Boundable : public Deletable
{
 public:
  /// &nbsp;
   ~Boundable() {}


  /// This must output a Sphere which contains the entire object, 
  /// in local coordinates.
  /// This is presumed slow to calculate, it is the users responsibility to cache the result.
   virtual void Bound(bs::Sphere & out) const = 0;
  
  /// This must output a bounding box that contains the entire object, in local
  /// coordinates. This is not an axis-aligned representation.
  /// This is presumed slow to calculate, it is the users responsibility to cache the result.
   virtual void Bound(bs::Box & out) const = 0;
   
   
  /// &nbsp;
   virtual cstrconst TypeString() const = 0;   
};

//------------------------------------------------------------------------------
/// An Object, must provide several bounding volumes and do ray intersection
/// tests, providing texture coordinate when an intersection
/// occurs. Whilst standard ray intersection must be suported a bounded line
/// binary check must also be provided, for lighting tests.
/// Must suport intersection with rays emited inside the object, for transparency.
/// Note that if a ray *starts* on the surface of an object it can not intersect
/// the surface where it starts - this would break reflection and transparency 
/// code.
class EOS_CLASS Object : public Boundable
{
 public:
  /// &nbsp;
   ~Object() {}


  /// This method is called at the start of a render - it can be used for
  /// building any optimised data structures the other methods may require.
   virtual void Prepare() {}
   
  /// This method is called at the end of a render - it can be used to
  /// clean up stuff.
   virtual void Unprepare() {}


  /// Given a point this must tell you if the point it inside the object. Required to 
  /// correctly handle the camera intersecting transparent objects.
   virtual bit Inside(const bs::Vert & point) const  = 0;

  /// Given a ray this returns true if it intercepts the Object.
  /// When it returns true it must also set dist to how far along
  /// the ray the interception occurs.
   virtual bit Intercept(const bs::Ray & ray,real32 & dist) const = 0;

  /// Given a ray this returns true if it intercepts the Object,
  /// if it returns true it also sets the Intersection object accordingly.
   virtual bit Intercept(const bs::Ray & ray,Intersection & out) const = 0;
   
  /// Given a finite line this returns true if the object intercepts the line at
  /// any point along the line. Used to optimise lighting tests.
   virtual bit Intercept(const bs::FiniteLine & line) const = 0;


  /// Must return the number of material indexes the object will output, 
  /// for user conveniance.
   virtual nat32 Materials() const {return 1;}
};

//------------------------------------------------------------------------------
/// This enumerates the types of texture parameter avaliable, so Texture's and
/// Material's can be linked correctly.
/// The convention exists that textures outputing colour with alpha output the
/// alpha channel as the value channel.
enum TextureType {TextureValue, ///< Outputs a single real32 value.
                  TextureColour, ///< Outputs a rgb colour.
                  TextureOffset, ///< Outputs a 2D offset.
                  TextureVector, ///< Outputs a vector, ushally interpreted as a surface orientation, offset from (1,0,0).
                  TextureAny ///< Can be any type. Ushally setting a texture any for a texture user will then lock down others to be the same type.
                 };

//------------------------------------------------------------------------------
/// Defines an object that uses textures to define parameters across some 
/// dimensionality. Used primarilly for materials and textures themselves, but
/// also used by lights and backgrounds.
class EOS_CLASS TextureUser : public Deletable
{
 public:
  /// &nbsp;
   ~TextureUser() {}


  /// Returns the number of parameters.
   virtual nat32 Parameters() const {return 0;}
    
  /// Returns the type of the parameter at a given index.
   virtual TextureType ParaType(nat32 index) const {return TextureValue;}
   
  /// Returns the name of the parameter at a given index.
   virtual cstrconst ParaName(nat32 index) const {return "error";}
   
  /// Lets the user set the texture at a given index. The texture must suport 
  /// the type in question. It must handle null in some sensible way.
   virtual void ParaSet(nat32 index,class Texture * rhs) {}
   
   
  /// &nbsp;
   virtual cstrconst TypeString() const = 0;   
};

//------------------------------------------------------------------------------
/// An object type optionally used between intersection and material BDRF 
/// calculation, it is given the intersection object and may do whatever it 
/// likes with it. Ushally used to modify texture coordinates based on the some
/// projection scheme other than the objects built in system, but can accheive 
/// other affects.
class EOS_CLASS IntersectionModifier : public TextureUser
{
 public:
  /// &nbsp;
   ~IntersectionModifier();
   
  /// This function must apply the modification as it sees fit. Can do whatever
  /// it likes. Should be consistant however, if given the same intersection 
  /// twice should change it in exactly the same way.
   virtual void Modify(Intersection & inter) const = 0;   
};

//------------------------------------------------------------------------------
/// A Material, given a set of material parameters as texture objects this 
/// expresses a BDRF.
class EOS_CLASS Material : public TextureUser
{
 public:
  /// &nbsp;
   ~Material() {}


  /// Returns true if transparency is suported.
   virtual bit Transparency() const {return false;}

  /// If transparency is suported this outputs the transparency ray resulting
  /// from an intersection.
   virtual void TransparencyRay(const bs::Normal & inDir,const Intersection & ip,bs::Ray & out) const
   {    
    out.s = ip.point;
    out.n = inDir;
   }
  
  /// For transparency this must output the light energy multiplier experianced over 
  /// the given line. It can assume the line is contained wholly within the material.
  /// Defaults to no loss due to distance traveled if not overloaded.
   virtual void TransparencyLoss(const bs::FiniteLine & line,bs::ColourRGB & out) const
   {
    out = bs::ColourRGB(1.0,1.0,1.0);
   }


  /// Returns true if reflections are suported.
   virtual bit Reflection() const {return false;}
   
  /// If reflection is suported this outputs the reflection ray resulting
  /// from an intersection. Defaults to presuming the surface is planer to
  /// its surface orientation.
   virtual void ReflectionRay(const bs::Normal & inDir,const Intersection & inter,bs::Ray & out) const
   {
    out.s = inter.point;
    out.n = inDir;
    out.n.Reflect(inter.norm);
   }


  /// This must return the BDRF. It is given an Intersection and the incomming and outgoing
  /// normals from which it must determine the multiplier for light reflected this way.
  /// Note that it must suport a full sphere rather than just a hemisphere due to transparency.
   virtual void BDRF(const bs::Normal & inDir,const bs::Normal & outDir,
                     const Intersection & inter,bs::ColourRGB & out) const = 0;
};

//------------------------------------------------------------------------------
/// A Texture, converts a texture coordinate into a parameter. Multiple parameter
/// types are suported. Note that texture coordinate transformations are
/// represented as Texture objects.
class EOS_CLASS Texture : public TextureUser
{
 public:
  /// &nbsp;
   ~Texture() {}


  /// Returns true if it suports the given output type.
   virtual bit Suports(TextureType type) const = 0;


  /// Extracts a value parameter. Should only be called if it suports this.
  /// Called with orientation information as well as texture coordinates,
  /// orientation will ushally not be used, but has the occasional use.
   virtual void GetValue(const TexCoord & coord,
                         const bs::Normal & inDir,const bs::Normal & norm,
                         real32 & out) const {out = 0.0;}

  /// Extracts a colour parameter. Should only be called if it suports this.
  /// Called with orientation information as well as texture coordiantes,
  /// orientation will ushally not be used, but has the occasional use.
   virtual void GetColour(const TexCoord & coord,
                          const bs::Normal & inDir,const bs::Normal & norm,
                          bs::ColourRGB & out) const {out = bs::ColourRGB(0.0,0.0,0.0);}

  /// Extracts a 2D offset parameter. Should only be called if it suports this.
  /// Called with orientation information as well as texture coordinates,
  /// orientation will ushally not be used, but has the occasional use.
   virtual void GetOffset(const TexCoord & coord,
                          const bs::Normal & inDir,const bs::Normal & norm,
                          bs::Pnt & out) const {out = bs::Pnt(0.0,0.0);}
                         
  /// Extracts a vector parameter. Should only be called if it suports this.
  /// Called with orientation information as well as texture coordiantes,
  /// orientation will ushally not be used, but has the occasional use.
   virtual void GetVector(const TexCoord & coord,
                          const bs::Normal & inDir,const bs::Normal & norm,
                          bs::Vert & out) const {out = bs::Vert(0.0,0.0,0.0);}
};

//------------------------------------------------------------------------------
/// A Light, emits light using whatever strategy it chooses.
/// Given a point in space and an object database it must provide how much 
/// energy it provides that point. A point light would simply fire a ray between
/// it and the point and check for intersections with the scene.
/// A pre-rendering method is called, which can construct a shadow map if need be.
class EOS_CLASS Light : public TextureUser
{
 public:
  /// &nbsp;
   ~Light() {}
   
  /// Given an intersection, an outDir and a material plus the scene 
  /// description object the light object is responsible for providing how 
  /// much light gets emited in the outDir of the object as a direct consequence
  /// of the light. This includes quite a lot of work, but is necesary to suport
  /// area lights and because the concept of a seperate light doesn't really fit
  /// into a global illumination renderer anyway.
  /// Note the the given intersection must of had any relevent modifier applied.
   virtual void Calc(const bs::Normal & outDir,const Intersection & inter,const Material & mat,
                     const class RenderableDB & db,bs::ColourRGB & out) const = 0;
};

//------------------------------------------------------------------------------
/// A Background object, very simply if given a ray provides the
/// radiance comming from that direction 'at infinity'.
/// Used for rays that miss all objects.
class EOS_CLASS Background : public TextureUser
{
 public:
  /// &nbsp;
   ~Background() {}
   
  /// This method simply converts a ray into its background colour response.
  /// (The full ray is used as a Background could in fact be another rendering system.)
   virtual void Calc(const bs::Ray & ray,bs::ColourRGB & out) const = 0;
};

//------------------------------------------------------------------------------
/// A Rendererable, consists of an Object, the Transform to get into the 
/// Object's local coordinate space and the Material's required to
/// render it. Just a pointer container.
class EOS_CLASS Renderable
{
 public:
  /// The object to render.
   Object * object;
   
  /// The transform from world to local coordinates for the object.
   OpTran * local;
   
  /// Type provided for use in the mats array.
   struct MatSpec
   {
    Material * mat;
    IntersectionModifier * im;
   }; 
   
  /// The array of materials used by the object. Each material can have an
  /// (optional) intersection modifier associated with it.
   ds::Array<MatSpec> mat;
};

//------------------------------------------------------------------------------
/// RenderableDB, stores a set of objects and manages the ray intersections,
/// exists to optimise ray intersection tests.
class EOS_CLASS RenderableDB : public Deletable
{
 public:
  /// &nbsp;
   ~RenderableDB() {}


  /// Adds a Renderable to the DB. Ownership remains with the user.
   virtual void Add(Renderable * rb) = 0;


  /// Calls its namesake on all contained objects, can also do work of its own.
   virtual void Prepare(time::Progress * prog = null<time::Progress*>()) = 0;
   
  /// Calls its namesake on all contained objects, can also do work of its own.
   virtual void Unprepare(time::Progress * prog = null<time::Progress*>()) = 0;


  /// Given a point this must append to the given list all objects it is contained
  /// within.
   virtual void Inside(const bs::Vert & point,ds::List<Renderable*> & out) const = 0;
  
  /// Given a Ray this gives you the details of the object it intercepts. 
  /// Returns true to indicate success, false to indicate its exited the scene
  /// and you must sample the background.
  /// The intersection must be converted into world coordinates.
   virtual bit Intercept(const bs::Ray & ray,Renderable *& objOut,Intersection & intOut) const = 0;
  
  /// Given a line this returns true if it intercepts any objects, 
  /// and gives you details of the first interception, or false if there is 
  /// nothing between the two end points.
  /// The intersection must be converted into world coordinates.
   virtual bit Intercept(const bs::FiniteLine & line,Renderable *& objOut,Intersection & intOut) const = 0;  
   
   
  /// &nbsp;
   virtual cstrconst TypeString() const = 0;
};

//------------------------------------------------------------------------------
/// A TaggedRay, the fundamental entity that gets shot arround the system.
/// An extension of the bs::Ray with various extra details,
/// such as intercepted object, texture coordinate, velocity and irradiance.
class EOS_CLASS TaggedRay : public bs::Ray
{
 public:
  /// &nbsp;
   TaggedRay() {}

  /// &nbsp;
   ~TaggedRay() {}


  /// The object the ray hit, or null if it hit the background instead.
   Renderable * hit;
  
  /// The irradiance recieved from the Ray - it is with this the final image
  /// is created.
   bs::ColourRGB irradiance;
   
  /// The intersection details, incase they are useful. (Because it gives
  /// access to the material and texture coordinates this can be valuable 
  /// for post proccessing.)
   Intersection inter;
   
  /// Weighting for the ray, this should be respected when they are combined.
  /// Used to create particular blur affects, but ushally just set to 1.0.
  /// Set by the Sampler, not the Renderer.
   real32 weight;
};

//------------------------------------------------------------------------------
/// A Renderer, does an actual render, can represent multiple techneques, such as
/// a simple scan line method, a raytracer or a global illumination renderer.
/// Responsible for taking the BDRF's of intersecting rays to create a fully 
/// tagged RayImage object.
class EOS_CLASS Renderer : public Deletable
{
 public:
  /// &nbsp;
   ~Renderer() {}


  /// This is given a tagged ray where only the ray and weight parts is filled in - 
  /// it is then responsible for filling in the rest.
  /// It is given a RenderableDB, Light list and Background (for fallback) to this end.
  /// It is additionally suplied with a list of objects that it is inside of at the 
  /// start of the pass.
   virtual void Cast(TaggedRay & ray,
                     const RenderableDB & db,
                     const ds::List<Renderable*> & inside,
                     const ds::List<Light*,mem::KillDel<Light> > & ll,
                     const Background & bg) const = 0;


  /// &nbsp;
   virtual cstrconst TypeString() const = 0; 
};

//------------------------------------------------------------------------------
/// The RayImage stores tagged rays for every pixel.
/// The results of rendering are collected in this object, which is then passed
/// through Postproccessor(s) and the ToneMapper object.
class EOS_CLASS RayImage
{
 public:
  /// You give it the width and height of the image and the expected number of
  /// rays for each pixel, though you can store more rays per pixel it is 
  /// expected that most pixels would be the expected number,
  /// due to anti-aliasing. blockRays is the number of extra rays allocated 
  /// each time a pixel runs out of space.
   RayImage(nat32 width,nat32 height,nat32 expRays,nat32 blockRays = 4);
   
  /// &nbsp;
   ~RayImage();
   
  
  /// &nbsp;
   nat32 Width() const;
   
  /// &nbsp;
   nat32 Height() const;


  /// Returns the number of tagged rays assigned to a given pixel.
   nat32 Rays(nat32 x,nat32 y) const;

  /// Returns the sum of the weightings of the rays, maintained incrimentally as it has its uses.
   real32 RaysWeightSum(nat32 x,nat32 y) const;
   
  /// Returns a reference to a particular tagged ray.
   TaggedRay & Ray(nat32 x,nat32 y,nat32 index);

  /// &nbsp;
   const TaggedRay & Ray(nat32 x,nat32 y,nat32 index) const;


  /// Removes a particular ray, will change the indexes of any rays after it.
   void RemRay(nat32 x,nat32 y,nat32 index);
   
  /// Adds a ray, returns the index of the new Ray.
   nat32 AddRay(nat32 x,nat32 y,const TaggedRay & in);


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::rend::RayImage";} 


 private:
  nat32 width;
  nat32 height;
  nat32 expRays;
  nat32 blockRays;
  
  nat32 sizeOfThing;
 
  // Storage is accheieved using singularly linked lists containing a number of
  // TaggedRay objects. The first node in each list is declared in one mother 
  // data structure.
   // The linked list node...
    struct Node // malloced
    {
     Node * next;

     TaggedRay * First() {return (TaggedRay*)(void*)(this+1);}
    };

   // The head data for each list, allwys followed by a Node...
    struct Head // malloced
    {
     nat32 rays;
     real32 weightSum;
     
     Node * Start() {return (Node*)(void*)(this+1);}
    };

   // The data structure that contains it all...
    byte * data;

    Head * Entry(nat32 x,nat32 y) const
    {
     return (Head*)(void*)(data + sizeOfThing*(width*y + x));
    }
};

//------------------------------------------------------------------------------
/// The Viewer object specifies image resolution and must provide Rays
/// for the corners of pixels.
/// (Corners so anti-aliasing can be done in a seperate module.)
/// The fact the camera directly provides rays rather than any projection matrix
/// allows for some vary strange and/or detailed camera models.
/// For example radial distortion can be modeled, as could realistic optical
/// systems if a postprocessing filter is additionally used to add other 
/// defects in. Origin is bottom left.
class EOS_CLASS Viewer : public TextureUser
{
 public:
  /// &nbsp;
   ~Viewer() {}


  /// &nbsp;
   virtual nat32 Width() const = 0;

  /// &nbsp;
   virtual nat32 Height() const = 0;
   

  /// Should return true if a fixed centre exists, as in all rays are emitted 
  /// from the same point. Allows reuse of the list of objects that 
  /// it starts off intersecting, as otherwise it has to be recalculated for
  /// every ray. Used to tell the renderer that it can do a specific 
  /// optimisation basically.
  /// If it returns true it should also output that centre point.
   virtual bit ConstantStart(bs::Vert & out) const {return false;}
   
  /// This is given a pixel coordinate, [0..Width()]x[0..Height()] and must 
  /// output the ray at the bottom left corner of that pixel. Because of 
  /// this you can give it coordinates with components equal to 
  /// Width() or Height() to obtain the corners of pixels on the edge.
  /// The Sampler module then takes these 4 rays for each pixel and decides 
  /// which rays to cast by linearly interpolating between them.
  /// The fact it takes real values means this is expected to interpolate
  /// between pixel corners as well.
   virtual void ViewRay(real32 x,real32 y,bs::Ray & out) const = 0;
};

//------------------------------------------------------------------------------
/// The Sampler object decides which rays to sample for each given pixel,
/// and is responsible for implimenting anti-aliasing. It also provides weights
/// for each ray, so tricks like applying a suitable blur for TV can be
/// implimented here.
/// This actually drives a render, being given all the relevent objects and the
/// progress bar, so it can report what percentage of rays have been cast/pixels
/// done etc.
class EOS_CLASS Sampler : public Deletable
{
 public:
  /// &nbsp;
   ~Sampler() {}


  /// An optimisation method - should return the expected number of rays per pixel,
  /// used to initialise the RayImage.
   virtual nat32 Samples() const = 0;

  /// A single method - given a Job it simply updates it such that the contained
  /// RayImage is complete. Provided with a progress bar as this can take some time.
  /// It is responsible for calling the Renderer for each ray it wants analysed
  /// and storing it in the relevent pixel(s) for the RayImage.
   virtual void Render(class Job & job,time::Progress * prog = null<time::Progress*>()) = 0;


  /// &nbsp;
   virtual cstrconst TypeString() const = 0;    
};

//------------------------------------------------------------------------------
/// The Postproccessor is given the RayImage, which it can then edit as it sees
/// fit. Multiple postprocessors can be applied before tone mapping finally
/// occurs.
class EOS_CLASS PostProcessor : public TextureUser
{
 public:
  /// &nbsp;
   ~PostProcessor() {}
   
  /// Does the affect, whatever it may be.
   virtual void Apply(RayImage & ri,time::Progress * prog = null<time::Progress*>()) const = 0;
};

//------------------------------------------------------------------------------
/// The ToneMapper takes the RayImage and generates a final image in standard
/// 0..1 range. It is responsible for combining rays as well as the actual tone
/// mapping, though a weighted average would ushally be used.
/// It is given all the information at once, so effects like bloom and real lens
/// flare can be simulated. Note that this runs after the postprocessor(s),
/// and therefore it will tone map all changes they have made.
class EOS_CLASS ToneMapper : public TextureUser
{
 public:
  /// &nbsp;
   ~ToneMapper() {}
   
  
  /// Given a RayImage and a svt::Field for output this does the tone mapping.
   virtual void Apply(const RayImage & ri,svt::Field<bs::ColourRGB> & out,
                      time::Progress * prog = null<time::Progress*>()) const = 0;
};

//------------------------------------------------------------------------------
/// A Job, represents all the details that go into a render - 
/// the scene description, the Renderer and the (optional) convertion from RayImage
/// to normal image. Manages construction/destruction of all objects, 
/// so the same materials can be used repeatedly etc - its main purpose really
/// is that of grand destructor.
class EOS_CLASS Job
{
 public:
  /// &nbsp;
   Job();
   
  /// &nbsp;
   ~Job();


  /// This must be called before anything else, the objects passed in 
  /// become the property of this object. It must never be called more
  /// than once - it is a double constructor.
  /// \param db The database, into which all other objects are inserted.
  /// \param renderer The rendering method to be used.
  /// \param bg Background, used by all rays that miss the scene.
  /// \param viewer The camera view to be rendered.
  /// \param sampler The anti-aliasing scheme to be used.
  /// \param tm The tone mapper used to create the final image. Can be null if 
  ///           actual image output is not going to be requested.
   void Setup(RenderableDB * db,Renderer * renderer,
              Background * bg,Viewer * viewer,
              Sampler * sampler,ToneMapper * tm);
              
  
  /// &nbsp;
   RenderableDB & DB() {return *db;}
   
  /// &nbsp;
   Renderer & Rend() {return *renderer;}
   
  /// &nbsp;
   Background & BG() {return *bg;}
   
  /// &nbsp;
   Viewer & Camera() {return *viewer;}
   
  /// &nbsp;
   Sampler & Samp() {return *sampler;}
   
  /// &nbsp;
   ds::List<Light*,mem::KillDel<Light> > & LightList() {return lights;}


  /// This adds a post-processor, after all current post-processors.
  /// The post-processor will then become the property of the Job.
   void Add(PostProcessor * pp);

  /// Adds a Light to the light list. It will be deleted when the Job is deleted.
   void Add(Light * light);
   
  /// Adds a Renderable to the database. The various things it points to will 
  /// ushally be registered with this class, though that is optional.
  /// After adding the Job owns the Renderable.
   void Add(Renderable * obj);


  /// Registers a OpTran so it can be deleted when the Job is done.
   void Register(OpTran * tran);

  /// Registers an Object so it can be deleted when the Job is done.
   void Register(Object * obj);

  /// Registers a IntersectionModifier so it can be deleted when the Job is done.
   void Register(IntersectionModifier * im);

  /// Registers a Material so it can be deleted when the Job is done.
   void Register(Material * mat);

  /// Registers a Texture so it can be deleted when the Job is done.
   void Register(Texture * tex);  


  /// Renders the scene. This version does not produce an image,
  /// it exists for if you want the RayImage rather than something
  /// that has been tone mapped.
   void Render(time::Progress * prog = null<time::Progress*>());
   
  /// Renders the scene. This generates an image, requiring that
  /// a ToneMapper was specified.
   void Render(svt::Field<bs::ColourRGB> & out,time::Progress * prog = null<time::Progress*>());

  /// After rendering you can use this to access the RayImage, 
  /// incase you need to extract such information.
  /// (Includes depth, velocity and other juicy bits of info.)
   RayImage & RI() {return *ri;}


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::rend::Job";}


 private:
  RenderableDB * db;
  Renderer * renderer;
  Background * bg;
  Viewer * viewer;
  Sampler * sampler;
  ToneMapper * tm;

  ds::List<PostProcessor*,mem::KillDel<PostProcessor> > posts;
  ds::List<Light*,mem::KillDel<Light> > lights;
  ds::List<Renderable*,mem::KillDel<Renderable> > objs;

  ds::List<OpTran*,mem::KillDel<OpTran> > trans;
  ds::List<Object*,mem::KillDel<Object> > objects;
  ds::List<IntersectionModifier*,mem::KillDel<IntersectionModifier> > ims;
  ds::List<Material*,mem::KillDel<Material> > materials;
  ds::List<Texture*,mem::KillDel<Texture> > textures;
  
  RayImage * ri;
};

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
  inline T & StreamRead(T & lhs,eos::rend::Transform & rhs,Binary)
  {
   lhs >> rhs.t >> rhs.r >> rhs.s;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::rend::Transform & rhs,Binary)
  {
   lhs << rhs.t << rhs.r << rhs.s;
   return lhs;
  }
 
  template <typename T>
  inline T & StreamWrite(T & lhs,const eos::rend::Transform & rhs,Text)
  {
   lhs << rhs.t << "~" << rhs.r << "~" << rhs.s;
   return lhs;
  }


 };      
};
//------------------------------------------------------------------------------
#endif
