#ifndef EOS_CAM_FILES_H
#define EOS_CAM_FILES_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file cam/files.h
/// Provides classes to represent various file formats I use in relation to
/// camera calibration. Specifically this covers the intrinsic calibration of a
/// single camera and the relationship between two cameras plus each cameras
/// intrinsic properties.

#include "eos/types.h"
#include "eos/cam/cameras.h"
#include "eos/str/strings.h"
#include "eos/bs/dom.h"
#include "eos/bs/geo2d.h"
#include "eos/ds/arrays.h"
#include "eos/svt/field.h"

namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------
/// A simple class, contains 3 details in regards to a particular camera,
/// the intrinsic parameters of the camera, the radial parameters of the camera
/// and the resolution of the images for which these are valid.
/// It provides a Load and Save method, for a .icd file, which is simply these
/// parameters written out as xml.
/// The image is presumed indexed from the bottom left corner.
class EOS_CLASS CameraCalibration
{
 public:
  /// Sets a default configuration, not something that can be practically used.
  /// You provide the image resolution.
   void SetDefault(real64 width,real64 height);

  /// This adjusts the calibration for coordinates from a cropped input.
  /// Changes everything accordingly, including so the default image size is
  /// of the rnage given.
   void Crop(real64 minX,real64 dimX,real64 minY,real64 dimY);


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
   Intrinsic intrinsic;

  /// &nbsp;
   Radial radial;

  /// Size of images for which this is valid. If given different sized images it
  /// is ushally presumed to renormalise coordinates to the size stored here,
  /// i.e. its the same camera but recording at a different resolution.
   math::Vect<2,real64> dim;
};

//------------------------------------------------------------------------------
/// This stores a camera that is known relative to scene world coordinates - in
/// other words it consists of a camera projection matrix, radial distortion
/// parameters and the dimensions of the image. Provides saving and loading,
/// ushally stored as a .cam file.
/// The image is presumed indexed from the bottom left.
class EOS_CLASS CameraFull
{
 public:
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
   
  /// Fills it in from a CameraCalibration on the presumption that the camera 
  /// is in the default position.
   void FromCC(const CameraCalibration & cc);


  /// &nbsp;
   Camera camera;

  /// &nbsp;
   Radial radial;

  /// Size of images for which this is valid. If given different sized images it
  /// is ushally presumed to renormalise coordinates to the size stored here,
  /// i.e. its the same camera but recording at a different resolution.
   math::Vect<2,real64> dim;
};

//------------------------------------------------------------------------------
/// This represents a pair of cameras, contains a CameraCalibration for each camera
/// plus the fundamental matrix that pairs them off.
/// Additonally contains storage for the two projection matrices, and a method
/// to initialise them from the calibration data.
/// Also contains 3x3 matrices for each image to convert them into the relevent
/// coordinates required for the calibration. These will normally be set to the
/// identity matrices as everything is calculated for the specific images.
/// However, if you rectify an image pair using a planar method these can be set
/// to un-rectify prior to triangulation.
/// Suport file i/o of a .pcc file. (Pair of calibrated cameras.)
class EOS_CLASS CameraPair
{
 public:
  /// Fills in all the values to create an ideal stereo rig,
  /// i.e. where they are in the rectified position.
  /// You supply the image size.
   void SetDefault(real64 width,real64 height);

  /// This sets up the camera pair by being given two cameraFull objects, to be
  /// the left and right cameras.
   void FromFull(const CameraFull & left,const CameraFull & right);


  /// Scales the imput left image to a new size.
  /// Adjusts the unrectification matrix and image size accordinly.
   void ScaleLeft(real64 newWidth,real64 newHeight);

  /// Scales the imput right image to a new size.
  /// Adjusts the unrectification matrix and image size accordinly.
   void ScaleRight(real64 newWidth,real64 newHeight);

  /// This crops the left image, accheives this by simply updating the
  /// left rectification matrix and the associated image size.
   void CropLeft(real64 minX,real64 dimX,real64 minY,real64 dimY);

  /// This crops the right image, accheives this by simply updating the
  /// right rectification matrix and the associated image size.
   void CropRight(real64 minX,real64 dimX,real64 minY,real64 dimY);


  /// Calculates a transformation that takes a camera at the origin looking
  /// down the negative z-axis to the position and rotation of the left camera.
  /// Optionally outputs the transformation.
  /// It then uses the transformation to correct this pair such that the left 
  /// camera is at the origin looking down the negative z-axis.
   void LeftToDefault(math::Mat<4,4,real64> * out = 0);

  /// Returns true if, within numerical error, it is rectified.
  /// Slow.
   bit IsRectified() const;

  /// Gives the relationship between disparity and (left camera) depth, only
  /// works if the pair is rectified. The equation for which paramters are
  /// output is depth = a/(disparity+b).
   void GetDispToDepth(real64 & a,real64 & b) const;


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


  /// Swaps the cameras, so the left becomes the right and the right becomes the left.
   void Swap()
   {
    math::Swap(left,right);
    math::Transpose(fun);
    math::Swap(lp,rp);
    math::Swap(unRectLeft,unRectRight);
    math::Swap(leftDim,rightDim);
   }

  /// Initialises lp and rp using the other variables.
  /// Works in two modes, you can provide it with nothing and it generates a
  /// bunch of false coorespondences that match the fundamental matrix,
  /// and chooses the configuration which puts the majority
  /// of these points in front of the camera. Alternativly you can provide it
  /// with a list of actual points, in which case it will use them instead.
  /// If the number of points in front of the camera is a tie it resolves it
  /// by choosing the camera for which the nearest point is furthest away.
  /// Passed in matches should be un-distorted by any camera model used and
  /// corrected for any image scale change.
  /// Returns true on success.
   bit MakeProjection(ds::Array<Pair<bs::Point,bs::Point> > * matches);


  /// Given a 2d position in the left image and a disparity this triangulates
  /// the associated 3d homogenous point.
  /// Returns true on success, false on failure.
  /// Only for rectified pairs.
   bit Triangulate(real32 x,real32 y,real32 disp,math::Vect<4,real64> & out) const;
  
  /// Version of Triangulate for non-homogenous 32 bit vectors.
    bit Triangulate(real32 x,real32 y,real32 disp,math::Vect<3,real32> & out) const
    {
     math::Vect<4,real64> o;
     if (Triangulate(x,y,disp,o)==false) return false;
     for (nat32 i=0;i<3;i++) out[i] = o[i]/o[3];
     return true;
    }

  /// Given a 3d homogenous point this projects back to calculate the 2d
  /// position for the left image and its disparity, in rectified space.
  /// rectLeft and rectRight can be optionally given. As they are not directly
  /// stored in this class they have to otherwise be calculated every time.
  /// If calling this method multiple times then inverting unRectLeft and
  /// unRectRight yourself and passing them in will save a lot of time, as
  /// inversion is the slowest thing this method can do.
  /// Only for rectified pairs.
   void Project(const math::Vect<4,real64> & pos,real32 & outX,real32 & outY,real32 & outDisp,
                const math::Mat<3,3,real64> * rectLeft = 0,
                const math::Mat<3,3,real64> * rectRight = 0) const;
                
  /// Given a 3D homogenous point this projects back to calculate the 2d
  /// position for the left image and right image.
  /// rectLeft and rectRight can be optionally given. As they are not directly
  /// stored in this class they have to otherwise be calculated every time.
  /// If calling this method multiple times then inverting unRectLeft and
  /// unRectRight yourself and passing them in will save a lot of time, as  
   void Project(const math::Vect<4,real64> & pos,
                math::Vect<2,real64> & outLeft,math::Vect<2,real64> & outRight,
                const math::Mat<3,3,real64> * rectLeft = 0,
                const math::Mat<3,3,real64> * rectRight = 0) const;


  /// Given a disparity map this outputs a map of 3D positions.
   void Convert(const svt::Field<real32> & disp,svt::Field<bs::Vertex> & pos) const;

  /// Given a disparity map this outputs a needle map.
   void Convert(const svt::Field<real32> & disp,svt::Field<bs::Normal> & needle) const;

  /// Given a map of 3D positions this outputs a disparity map. The map must make snese,
  /// i.e. the points must be on the projection lines of there relevant pixels.
   void Convert(const svt::Field<bs::Vertex> & pos,svt::Field<real32> & disp) const;


  /// Some of the contained matrices are homogenous in nature, making arbitary
  /// multipliers safe - this simply goes through and divides each one by its
  /// frobnus norm, to make algorithms that then use all this data numerically
  /// stable.
   void Normalise();


  /// &nbsp;
   CameraCalibration left;

  /// &nbsp;
   CameraCalibration right;

  /// Will always match up with lp and rp, i.e. not transformed by unRectLeft or unRectRight.
   Fundamental fun;

  /// Thie distance between the two cameras, for scaling things correctly.
  /// Can just be set to 1 if unknown to get a solution within a scale factor.
   real64 gap;


  /// Will not include any rectifying transform, which is kept seperate.
   Camera lp;

  /// Will not include any rectifying transform, which is kept seperate.
   Camera rp;


  /// Matrix to un-rectify the left image.
   math::Mat<3,3,real64> unRectLeft;

  /// Size of the left image, if the unRectLeft is the identity then this will
  /// be the same as left.dim, but if not then this is the size of the rectified image.
   math::Vect<2,real64> leftDim;

  /// Matrix to unrectify the right image.
   math::Mat<3,3,real64> unRectRight;

  /// Size of the left image, if the unRectRight is the identity then this will
  /// be the same as right.dim, but if not then this is the size of the rectified image.
   math::Vect<2,real64> rightDim;
};

//------------------------------------------------------------------------------
 };
};
#endif
