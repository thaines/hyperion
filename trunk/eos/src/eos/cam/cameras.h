#ifndef EOS_CAM_CAMERAS_H
#define EOS_CAM_CAMERAS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file cam/cameras.h
/// Provides various objects for managing cameras, just extended matrix/vector
/// types.

/// \namespace eos::cam
/// Provides everything you could ever want in regards to cameras - classes to
/// represent them, tools to extract the intrinsic parameters, fundamental matrix
/// calculation, extrinsic parameter calculation, rectification and triangulation
/// of matched points.

#include "eos/types.h"
#include "eos/math/vectors.h"
#include "eos/math/matrices.h"
#include "eos/math/mat_ops.h"
#include "eos/math/iter_min.h"
#include "eos/str/strings.h"
#include "eos/bs/dom.h"
#include "eos/bs/geo2d.h"
#include "eos/bs/geo3d.h"

namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------
/// Represents a cameras intrinsic matrix, a 3x3 upper triangular matrix.
class EOS_CLASS Intrinsic : public math::Mat<3,3,real64>
{
 public:
  /// Initialises to an identity.
   Intrinsic() {Identity(*this);}

  /// &nbsp;
   ~Intrinsic() {}


  /// Adjusts the matrix such that it now works for coordinates that have
  /// had the given x and y added to them.
   void Offset(real64 x,real64 y);

  /// Adjusts the matrix such that it now works for coordinates that have
  /// been multiplied by the given scalers.
   void Scale(real64 xs,real64 ys);


  /// The cameras focal distance multiplied by the number of pixels per unit
  /// distance in the X dimension.
   real64 & FocalX() {return (*this)[0][0];}

  /// The cameras focal distance multiplied by the number of pixels per unit
  /// distance in the Y dimension.
   real64 & FocalY() {return (*this)[1][1];}

  /// The principal points X coordinate, in terms of pixels.
   real64 & PrincipalX() {return (*this)[0][2];}

  /// The principal points Y coordinate, in terms of pixels.
   real64 & PrincipalY() {return (*this)[1][2];}

  /// The skew of the camera, should be 0 for an idea camera.
   real64 & Skew() {return (*this)[0][1];}


  /// Aspect ratio of the camera, just FocalY() / FocalX().
   real64 AspectRatio() const {return (*this)[1][1]/(*this)[0][0];}


  /// &nbsp;
   void Load(const bs::Element & root);

  /// Returns true on success.
   bit Load(cstrconst fn);

  /// Returns true on success.
   bit Load(const str::String & fn);

  /// &nbsp;
   void Save(bs::Element & root);

  /// Returns true on success.
   bit Save(cstrconst fn,bit overwrite = false);

  /// Returns true on success.
   bit Save(const str::String & fn,bit overwrite = false);


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::cam::Intrinsic";}
};

//------------------------------------------------------------------------------
/// Represents the radial distortion parameters of a camera, we use a model
/// where the centre of the distortion and the principal point are not
/// necesarilly co-incident, and model to 4 terms. i.e. using coordinates which have
/// been adjusted to have (0,0) at the radial distortion centre then
/// P' = P * (1 + k[0]*r + k[1]*r*r + k[2]*r*r*r + k[2]*r*r*r*r)
/// where P is the linear model point coordinate and P' is the distorted point we
/// record. r is the length of P, k[...] are the distortion terms, with the centre
/// being implicit in the above equation. We also have an aspect ratio,
/// which we multiply the y coordinate with before use, for if the pixels arn't square.
/// The centre is stored without adjustment for the aspectRatio, i.e. its y
/// coordinate requires multipication as well.
class EOS_CLASS Radial
{
 public:
  /// Leaves the contents random.
   Radial() {}

  /// &nbsp;
   ~Radial() {}


  /// Converts from an un-distorted coordinate to a distorted coordinate.
  /// dis and unDis can be the same variable.
   void Dis(const math::Vect<2,real64> & unDis,math::Vect<2,real64> & dis) const
   {
    real64 ox = unDis[0] - centre[0];
    real64 oy = unDis[1] - centre[1];
    real64 d = math::Sqrt(math::Sqr(aspectRatio*ox) + math::Sqr(oy));
    real64 mult = 1.0 + (k[0] + (k[1] + (k[2] + k[3]*d)*d)*d)*d;
    dis[0] = centre[0] + ox*mult;
    dis[1] = centre[1] + oy*mult;
   }

  /// Converts from an un-distorted coordinate to a distorted coordinate.
  /// dis and unDis can be the same variable, the output variable will be
  /// normalised such that w=1.
   void Dis(const math::Vect<3,real64> & unDis,math::Vect<3,real64> & dis) const
   {
    real64 ox = (unDis[0]/unDis[2]) - centre[0];
    real64 oy = (unDis[1]/unDis[2]) - centre[1];
    real64 d = math::Sqrt(math::Sqr(aspectRatio*ox) + math::Sqr(oy));
    real64 mult = 1.0 + (k[0] + (k[1] + (k[2] + k[3]*d)*d)*d)*d;
    dis[0] = centre[0] + ox*mult;
    dis[1] = centre[1] + oy*mult;
    dis[2] = 1.0;
   }

  /// Converts from distorted to un-distorted coordinates. Due to the nature
  /// of the function it has to use a LM based method, though convergence is
  /// very quick as its an ideal function. This is still a very slow method
  /// however.
  /// dis and unDis can be the same variable.
   void UnDis(const math::Vect<2,real64> & dis,math::Vect<2,real64> & unDis) const
   {
    real64 ox = dis[0] - centre[0];
    real64 oy = dis[1] - centre[1];

    math::Vect<1,real64> d;
    d[0] = 1.0;

    UnDisStruct uds(k);
    uds.result = math::Sqrt(math::Sqr(aspectRatio*ox) + math::Sqr(oy));

    math::LM(d,uds,&UnDisF);

    d[0] /= uds.result;
    unDis[0] = centre[0] + ox*d[0];
    unDis[1] = centre[1] + oy*d[0];

    /*math::Vect<2,real64> disTemp;
    Dis(unDis,disTemp);
    LogDebug("{dis,unDis,re-dis}" << LogDiv() << dis << LogDiv() << unDis << LogDiv() << disTemp);
    LogDebug("{d[0],result}" << LogDiv() << d[0] << LogDiv() << uds.result);*/
   }

  /// Converts from distorted to un-distorted coordinates. Due to the nature
  /// of the function it has to use a LM based method, though convergence is
  /// very quick as its an ideal function. This is still a very slow method
  /// however.
  /// dis and unDis can be the same variable.
  /// The output will be normalised so that w=1.
   void UnDis(const math::Vect<3,real64> & dis,math::Vect<3,real64> & unDis) const
   {
    math::Vect<2,real64> disNorm,unDisNorm;
    disNorm[0] = dis[0]/dis[2];
    disNorm[1] = dis[1]/dis[2];

    UnDis(disNorm,unDisNorm);

    unDis[0] = unDisNorm[0];
    unDis[1] = unDisNorm[1];
    unDis[2] = 1.0;
   }


  /// Given the image size the parameters were calculated for and a new image
  /// size this transforms the parameters so they will work with coordinates for
  /// the new size.
  /// This is for simple scaling of image size only, i.e. same camera, different
  /// resolution, ushally used to reduce the quantity of data to be proccessed.
  /// in is the original size, out is the size to change to.
   void ChangeSize(const bs::Pnt & in,const bs::Pnt & out);

  /// Adjusts the parameters such that it now works for coordinates that have
  /// had the given x and y added to them.
  /// This adjustment happens in un-distorted space, so distort before hand if
  /// need be.
   void Offset(real64 x,real64 y);

  /// Adjusts the matrix such that it now works for coordinates that have
  /// been multiplied by the given scalers.
  /// This adjustment happens in un-distorted space, so distort before hand if
  /// need be.
   void Scale(real64 xs,real64 ys);


  /// &nbsp;
   void Load(const bs::Element & root);

  /// Returns true on success.
   bit Load(cstrconst fn);

  /// Returns true on success.
   bit Load(const str::String & fn);

  /// &nbsp;
   void Save(bs::Element & root);

  /// Returns true on success.
   bit Save(cstrconst fn,bit overwrite = false);

  /// Returns true on success.
   bit Save(const str::String & fn,bit overwrite = false);


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::cam::Radial";}


  /// The aspect ratio, multiply the y component of each coordinate with this
  /// to get square pixels.
   real64 aspectRatio;

  /// The radial distortion centre.
   math::Vect<2,real64> centre;

  /// The terms of the distortion, if they are all 0 then we have no distortion.
   math::Vect<4,real64> k;


 private:
  // Helper for below...
   struct UnDisStruct
   {
    UnDisStruct(const math::Vect<4,real64> & kk):k(kk) {}

    const math::Vect<4,real64> & k;
    real64 result;
   };


  // Function for doing the error calculation for the UnDis method...
   static void UnDisF(const math::Vect<1,real64> & pv,math::Vect<1,real64> & ev,const UnDisStruct & pd)
   {
    real64 d = pv[0];
    real64 mult = 1.0 + (pd.k[0] + (pd.k[1] + (pd.k[2] + pd.k[3]*d)*d)*d)*d;
    ev[0] = mult*d - pd.result;
   }
};

//------------------------------------------------------------------------------
/// Given an intrinsic matrix and (optionally) radial parameters this calculates
/// its 35mm equivalent focal length using the horizontal.
/// Also requires the dimensions of the image to which the matrix applies.
EOS_FUNC real32 FocalLength35mmHoriz(real64 width,real64 height,
                                     const Intrinsic & intrinsic,
                                     const Radial * radial = null<Radial*>());

/// Given an intrinsic matrix and (optionally) radial parameters this calculates
/// its 35mm equivalent focal length using the vertical.
/// Also requires the dimensions of the image to which the matrix applies.
EOS_FUNC real32 FocalLength35mmVert(real64 width,real64 height,
                                    const Intrinsic & intrinsic,
                                    const Radial * radial = null<Radial*>());

/// Given an intrinsic matrix and (optionally) radial parameters this calculates
/// its 35mm equivalent focal length using the diagonal.
/// Also requires the dimensions of the image to which the matrix applies.
EOS_FUNC real32 FocalLength35mmDiag(real64 width,real64 height,
                                    const Intrinsic & intrinsic,
                                    const Radial * radial = null<Radial*>());

//------------------------------------------------------------------------------
/// This represents the extrinsic parameters of a camera, just a 3x4 matrix.
/// No functionality provided, at least not at the moment, just for representation.
class EOS_CLASS Extrinsic : public math::Mat<3,4,real64>
{};

//------------------------------------------------------------------------------
/// This represents a camera projection matrix, 3x4.
class EOS_CLASS Camera : public Extrinsic
{
 public:
  /// Initialises it to an extended identity.
   Camera() {Identity(*this);}

  /// Initialises it from an intrinsic matrix alone - sets the rotation matrix
  /// to the identity and the translation matrix to 0.
   Camera(const Intrinsic & intr)
   {
    math::Zero(*this);
    math::SetSub(*this,intr,0,0);
   }

  /// Initialises it from an intrinsic matrix, rotation matrix and translation
  /// matrix.
   Camera(const Intrinsic & intr,const math::Mat<3,3> & rot,math::Vect<3> & trans)
   {
    math::Mat<3,3,real64> temp;
    Mult(intr,rot,temp);
    math::Mat<3,4,real64> temp2;
    Identity(temp2);
    temp2[0][3] = -trans[0];
    temp2[1][3] = -trans[1];
    temp2[2][3] = -trans[2];
    Mult(temp,temp2,*this);
   }

  /// &nbsp;
   ~Camera() {}


  /// Extracts the intrinsic matrix and rotation matrix that together with the camera
  /// centre construct this projection matrix.
   void Decompose(Intrinsic & intr,math::Mat<3,3,real64> & rot) const
   {
    // Do the calculation...
     for (nat32 r=0;r<3;r++)
     {
      for (nat32 c=0;c<3;c++)
      {
       intr[r][c] = (*this)[r][c];
      }
     }

     math::RQ(intr,rot);
     math::ZeroLowerTri(intr);


    // Go through and check the signs - the intrinsic matrices diagonal should
    // be entirely positive...
     for (nat32 i=0;i<3;i++)
     {
      if (intr[i][i]<0.0)
      {
       for (nat32 r=0;r<3;r++) intr[r][i] *= -1.0;
       for (nat32 c=0;c<3;c++) rot[i][c] *= -1.0;
      }
     }


    // Intrinsic matrix is homogenous, so we can just multiply it at will -
    // make entry [2][2] equal to 1, as expected by the class...
     intr *= 1.0/intr[2][2];
   }

  /// Outputs the camera centre, as a homogenous coordinate, which happens to be
  /// the right null vector of this matrix.
   void Centre(math::Vect<4,real64> & trans) const
   {
    LogTime("eos::cam::Camera::Centre");
    math::Mat<4,4,real64> temp;
    math::SetSub(temp,*this,0,0);
    for (nat32 i=0;i<4;i++) temp[3][i] = 0.0;
    RightNullSpace(temp,trans);
    trans.Normalise();
    if (!math::IsZero(trans[3])) trans /= trans[3];
   }

  /// Outputs the inverse of the camera matrix - this will convert an image
  /// point into a point somewhere on the line that may be inhabited by that
  /// point.
   void GetInverse(math::Mat<4,3,real64> & out) const
   {
    LogTime("eos::cam::Camera::GetInverse");

    math::Mat<4,4> temp;
    math::SetSub(temp,*this,0,0);
    for (nat32 i=0;i<4;i++) temp[3][i] = 0.0;

    math::PseudoInverse(temp);

    math::SubSet(out,temp,0,0);
   }

  /// Outputs the direction the camera is looking in, this vector is most useful for backface culling.
   void Dir(math::Vect<3,real64> & dir) const
   {
    Intrinsic intr;
    math::Mat<3,3,real64> rot;
    Decompose(intr,rot);

    dir[0] = rot[2][0];
    dir[1] = rot[2][1];
    dir[2] = rot[2][2];
   }


  /// Given a 3d coordinate this calculates the depth of the coordinate from the
  /// camera.
  /// Remember that this will be negative if its in front of the camera, as we
  /// are using a right handed coordinate system.
   real64 Depth(const math::Vect<4,real64> & pos) const
   {
    math::Vect<3,real64> proj;
    math::MultVect(*this,pos,proj);

    math::Mat<3,3,real64> m;
    math::SubSet(m,*this,0,0);

    return (real32(math::Sign(math::Determinant(m)))*proj[2])
           /
           (pos[3] * math::Sqrt(math::Sqr(m[2][0])+math::Sqr(m[2][1])+math::Sqr(m[2][2])));
   }

  /// Calculates and returns the aspect ratio of the matrix as efficiently as
  /// I know how. This only exists because it is needed in a LM process, where
  /// speed really, really matters.
   real64 AspectRatio() const
   {
    Intrinsic intr;
    math::Mat<3,3,real64> temp;
    for (nat32 r=0;r<3;r++)
    {
     for (nat32 c=0;c<3;c++)
     {
      intr[r][c] = (*this)[r][c];
     }
    }
    math::RQ(intr,temp);
    return intr[1][1]/intr[0][0];
   }


  /// Outputs the matrix that converts from camera coordinates to world coordinates.
  /// Camera coordinates are camera at origin, camera looking down z-axis in -ve
  /// direction. (Right handed coordinate system.)
   void ToWorld(math::Mat<4,4,real64> & out) const
   {
    Intrinsic intr;
    math::Mat<3,3,real64> rot;
    math::Vect<4,real64> trans;

    Decompose(intr,rot);
    Centre(trans);
    trans /= trans[3];

    out[0][0] = rot[0][0]; out[0][1] = rot[1][0]; out[0][2] = rot[2][0]; out[0][3] = trans[0];
    out[1][0] = rot[0][1]; out[1][1] = rot[1][1]; out[1][2] = rot[2][1]; out[1][3] = trans[1];
    out[2][0] = rot[0][2]; out[2][1] = rot[1][2]; out[2][2] = rot[2][2]; out[2][3] = trans[2];
    out[3][0] = 0.0;       out[3][1] = 0.0;       out[3][2] = 0.0;       out[3][3] = 1.0;

    LogDebug("[cam.toworld] {intr,rot,trans,out}" << LogDiv() << intr << LogDiv() << rot << LogDiv() << trans << LogDiv() << out);
   }

  /// Outputs the matrix that converts from world coordinates to camera coordinates.
   void ToLocal(math::Mat<4,4,real64> & out) const
   {
    Intrinsic intr;
    math::Mat<3,3,real64> rot;
    math::Vect<4,real64> trans;

    Decompose(intr,rot);
    Centre(trans);
    trans /= trans[3];

    out[0][0] = rot[0][0]; out[0][1] = rot[0][1]; out[0][2] = rot[0][2]; out[0][3] = 0.0;
    out[1][0] = rot[1][0]; out[1][1] = rot[1][1]; out[1][2] = rot[1][2]; out[1][3] = 0.0;
    out[2][0] = rot[2][0]; out[2][1] = rot[2][1]; out[2][2] = rot[2][2]; out[2][3] = 0.0;
    out[3][0] = 0.0;       out[3][1] = 0.0;       out[3][2] = 0.0;       out[3][3] = 1.0;

    for (nat32 i=0;i<3;i++)
    {
     for (nat32 j=0;j<3;j++) out[j][3] -= rot[j][i]*trans[i];
    }

    LogDebug("[cam.tolocal] {intr,rot,trans,out}" << LogDiv() << intr << LogDiv() << rot << LogDiv() << trans << LogDiv() << out);
   }


  /// &nbsp;
   void Load(const bs::Element & root);

  /// Returns true on success.
   bit Load(cstrconst fn);

  /// Returns true on success.
   bit Load(const str::String & fn);

  /// &nbsp;
   void Save(bs::Element & root);

  /// Returns true on success.
   bit Save(cstrconst fn,bit overwrite = false);

  /// Returns true on success.
   bit Save(const str::String & fn,bit overwrite = false);


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::cam::Camera";}
};

//------------------------------------------------------------------------------
/// Represents the fundamental matrix, provides no functionality, just a type.
/// If x is in image a and x' is in image b and they match then x'^T this x = 0.
class EOS_CLASS Fundamental : public math::Mat<3,3,real64>
{
 public:
  /// Extracts the epipole in image a.
   void EpipoleA(math::Vect<3,real64> & ea) const;

  /// Extracts the epipole in image b.
   void EpipoleB(math::Vect<3,real64> & eb) const;

  /// Decomposes the fundamental matrix to [e']_X M, where e' is the epipole
  /// in the second image and M is a full-rank matrix. This is disturbingly
  /// tricky to do, so as to ensure that M is of full rank, and is currently
  /// implimented to involve a random number generator.
  /// WARNING - I don't think this currently works.
   void Decompose(math::Vect<3,real64> & eb,math::Mat<3,3,real64> & m) const;

  /// Outputs two camera matrices that match the fundamental matrix.
  /// These projections will be within a homography of the real values.
  /// Note that by construction these result in a fairly degenerate situation -
  /// the second camera centre will be at infinity.
   void GetCameraPair(Camera & ca,Camera & cb) const;

  /// Sets the fundamental matrix from a pair of cameras.
   void SetCameraPair(const Camera & left,const Camera & right);

  /// Given intrinsic matrices for the left and right images this corrects them
  /// so that there multiplication of this fundamental matrix produces a correct
  /// essential matrix. Attempts to minimise the change it makes to the
  /// intrinsic matrices.
  /// Note that the intrinsic matrices won't necesary remain upper triangular
  /// as a result of this. (Yeah, it sucks. Its a simple approach.)
   void MatchTo(Intrinsic & left,Intrinsic & right) const;
   
  /// Given a left and right projection matrix that almost match this 
  /// fundamental matrix this adjusts the projection matrices such that
  /// they precisly match the fundamental matrix. For compensating for
  /// numerical error and badly matching intrinsic matrices.
   void MatchTo(Camera & left,Camera & right) const;


  /// Given the image size the parameters were calculated for and a new image
  /// size this transforms the parameters so they will work with coordinates for
  /// the new size.
  /// This is for simple scaling of image size only, i.e. same camera, different
  /// resolution, ushally used to reduce the quantity of data to be proccessed.
  /// in is the original size, out is the size to change to.
   void ChangeSize(const bs::Pnt & inA,const bs::Pnt & inB,
                   const bs::Pnt & outA,const bs::Pnt & outB);


  /// &nbsp;
   void Load(const bs::Element & root);

  /// Returns true on success.
   bit Load(cstrconst fn);

  /// Returns true on success.
   bit Load(const str::String & fn);

  /// &nbsp;
   void Save(bs::Element & root);

  /// Returns true on success.
   bit Save(cstrconst fn,bit overwrite = false);

  /// Returns true on success.
   bit Save(const str::String & fn,bit overwrite = false);
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
  inline T & StreamWrite(T & lhs,const eos::cam::Radial & rhs,Text)
  {
   lhs << rhs.aspectRatio << ":<" << rhs.centre[0] << "," << rhs.centre[1] << ">:";
   lhs << rhs.k[0] << ":" << rhs.k[1] << ":" << rhs.k[2] << ":" << rhs.k[3];
   return lhs;
  }


 };
};
//------------------------------------------------------------------------------
#endif
