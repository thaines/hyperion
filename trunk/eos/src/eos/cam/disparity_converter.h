#ifndef EOS_CAM_DISPARITY_CONVERTER_H
#define EOS_CAM_DISPARITY_CONVERTER_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file disparity_converter.h
/// Provides an abstraction of and instances of the idea of converting disparity
/// into depth and needle maps.

#include "eos/types.h"
#include "eos/cam/cameras.h"
#include "eos/cam/files.h"
#include "eos/bs/geo3d.h"
#include "eos/svt/field.h"

namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------
/// An interface to provide methods that convert from disparity to depth,
/// 3d position and surface orientation. Simply 3 virtual methods in a class,
/// required because there are various ways of doing it, depending on how the 
/// input was captured/processed.
/// Implimenting objects are generally constructed for a specific image, so they 
/// might include masks or other per-pixel information in there construction 
/// proccess.
class EOS_CLASS DispConv : public Deletable
{
 public:
  /// &nbsp;
  ~DispConv() {}


 // So they can be cloned. Delete the returned when done.
  virtual DispConv * Clone() const = 0;
  
 /// This should return true if disparity to depth is position independent,
 /// false otherwise.
  virtual bit PosInd() const = 0;
  
 /// This should return the depth for the given disparity at the given position,
 /// if PosInd() the position given should not effect the output.
  virtual real32 DispToDepth(real32 x,real32 y,real32 disp) const = 0;
  
 /// This outputs the 3D coordinates of a given {x,y,disp} tuple.
  virtual void DispToPos(real32 x,real32 y,real32 disp,bs::Vertex & pos) const = 0;
  
 /// This should return the disparity given the depth at a given position,
 /// if PosInd() the position given should not effect the output.
  virtual real32 DepthToDisp(real32 x,real32 y,real32 depth) const = 0;
  
 /// This converts a 3D position to a disparity and a x,y position in the first image.
  virtual void PosToDisp(const bs::Vertex & pos,real32 & x,real32 & y,real32 & disp) const = 0;
  
 /// Converts an image location into a ray, consisting of a start positon and offset vector.
 /// (Start is ushally the centre of the camera, i.e. invariant to x/y.)
  virtual void Ray(real32 x,real32 y,bs::Vertex & start,bs::Vertex & offset) const = 0;


 /// This will set values in the depth map to infinity if need be.
  virtual void Convert(svt::Field<real32> & disp,svt::Field<real32> & depth) const = 0;
  
 /// Remember that this will output infinitly distant points.
  virtual void Convert(svt::Field<real32> & disp,svt::Field<bs::Vertex> & pos) const = 0;
  
 /// Infinitly deep values always have a surface normal pointing directly at the camera.
  virtual void Convert(svt::Field<real32> & disp,svt::Field<bs::Normal> & needle) const = 0;
  

 /// &nbsp;
  virtual cstrconst TypeString() const = 0;
};

//------------------------------------------------------------------------------
// For not rectified input.

//------------------------------------------------------------------------------
/// An implimentor of DispConv, this is designed to work with planar rectified 
/// input.
/// It is given a CameraPair for which this is the left image.
/// This camera pair should include the un-rectification matrices.
/// Can also be given a mask to indicate which areas are game for calculation.
class EOS_CLASS DispConvPlane : public DispConv
{
 public:
  /// &nbsp;
   DispConvPlane();
   
  /// &nbsp;
   ~DispConvPlane();


  /// &nbsp;
   DispConv * Clone() const;

   
  /// Sets the camera pair object, this expects to handle the left image.
  /// Must be called before use.
  /// You also have to provide the size of the left and right images, so 
  /// appropriate scaling may be done.
   void Set(const cam::CameraPair & pair,nat32 leftWidth,nat32 leftHeight,nat32 rightWidth,nat32 rightHeight);
   
  /// Sets masks for both images, it will set to 0/origin/pointing to camera anything
  /// that is outside the masks.
   void Set(const svt::Field<bit> & left,const svt::Field<bit> & right);


 /// &nbsp;
  bit PosInd() const;
  
 /// &nbsp;
  real32 DispToDepth(real32 x,real32 y,real32 disp) const;

 /// &nbsp;
  void DispToPos(real32 x,real32 y,real32 disp,bs::Vertex & pos) const;
  
 /// &nbsp;
  real32 DepthToDisp(real32 x,real32 y,real32 depth) const;

 /// &nbsp;
  void PosToDisp(const bs::Vertex & pos,real32 & x,real32 & y,real32 & disp) const;
 
 /// &nbsp; 
  void Ray(real32 x,real32 y,bs::Vertex & start,bs::Vertex & offset) const;


 /// &nbsp;
  void Convert(svt::Field<real32> & disp,svt::Field<real32> & depth) const;
  
 /// &nbsp;
  void Convert(svt::Field<real32> & disp,svt::Field<bs::Vertex> & pos) const;
  
 /// &nbsp;
  void Convert(svt::Field<real32> & disp,svt::Field<bs::Normal> & needle) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  cam::CameraPair pair;
  svt::Field<bit> leftMask;
  svt::Field<bit> rightMask;
  
 // Helper stuff...
  // Calculates the non-homogenous position of the given disparity/coordinate,
  // returns false if the point is at infinity or the calculation fails.
  // Used for the normal field calculation.
   bit CalcPos(nat32 x,nat32 y,real32 disp,nat32 width,nat32 height,bs::Vert & out) const;
   
 // Cached data, for acceleration purposes...
  mutable bit rectValid; // true if the below are the inverses of the unrectification matrices.
  mutable math::Mat<3,3,real64> rectLeft;
  mutable math::Mat<3,3,real64> rectRight;
};

//------------------------------------------------------------------------------
// For polar rectified input.

//------------------------------------------------------------------------------
 };
};
#endif
