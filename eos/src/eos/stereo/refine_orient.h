#ifndef EOS_STEREO_REFINE_ORIENT_H
#define EOS_STEREO_REFINE_ORIENT_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file refine_orient.h
/// This refines a disparity map given a needle map. Uses kernel density
/// estimation to estimate the ideal disparity assuming surrounding disparities
/// are correct, iterates till convergance.

#include "eos/types.h"
#include "eos/stereo/dsi_ms.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// Given an initial disparity map this refines it using a point-based surface
/// model. Uses surface orientation information to generate a smooth surface 
/// from the often stepped surface of the input. Mostly run two-step, firstly
/// without surface orientation information, where is will plane fit each
/// segment to get orientation, and then secondly with a given surface 
/// orientation calculated using the normal map of the first fitting as 
/// bootstrap.
/// The surface model simply have a 3d position for each pixel, it initialises 
/// from the disparity map and then iterates towards a smoother surface by 
/// treating each pixel with its orientation as a plane, interceptin the line of
/// neighbouring pixels and providing information on where the neighbours should
/// be. The sparse dsi provides costs to weight there influence.
class EOS_CLASS RefineOrient
{
 public:
  /// &nbsp;
   RefineOrient();

  /// &nbsp;
   ~RefineOrient();


  /// Sets the radius of the window used for gathering information, the
  /// damping of motion, the cost of a disparity difference and the number of 
  /// iterations used.
  /// Radius defaults to 2, damping to 1.0, stepCost to 0.5 and iters to 100.
   void Set(nat32 radius,real32 damping,real32 stepCost,nat32 iters = 100);

  /// Sets the input 3D positions with costs, you also provide the disparities 
  /// with costs as used to initialise the SparsePos as disparity is an easier
  /// measure for fidning costs.
   void Set(const DSI & dsi,const SparsePos & spos);

  /// Sets the segmentation.
   void Set(const svt::Field<nat32> & seg);
   
  /// Sets the needle map, sets the orientation of each and every plane.
  /// This is optional, if not set it plane fits each segment to obtain normals.
   void Set(const svt::Field<bs::Normal> & needle);

  /// Sets the camera configuration, so it can switch between disparity/position space.
   void Set(const cam::CameraPair & pair);
   
  /// If set to true segment 0 is assumed to be a dump for bad segments, 
  /// and is automatically assigned a location at infinity rather than having any 
  /// work done to it.
   void Duds(bit enable);


  /// Runs the algorithm.
  /// Can be called a second time after setting the needle map, at which point 
  /// it will start from its converged position the first time.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Outputs a map of 3D positions for every pixel.
   void PosMap(svt::Field<bs::Vertex> & out) const;

  /// Outputs a needle map.
  /// Calculated by central differencing the pixels, will be rather noisy,
  /// so it also allows for smoothing - set the given parameter to the standard 
  /// deviation of the gaussian to use, or 0.0 to not smooth.
   void NeedleMap(svt::Field<bs::Normal> & out,real32 sd = 0.0) const;

  /// Outputs a disparity map.
   void DispMap(svt::Field<real32> & out) const;


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::RefineOrient";}


 private:
  // Input...
   nat32 radius;
   real32 damping;
   real32 stepCost;
   nat32 iters;
   const DSI * dsi;
   const SparsePos * spos;
   svt::Field<nat32> seg;
   svt::Field<bs::Normal> needle;
   cam::CameraPair pair;
   bit duds;
   
   static const real32 dispWindow = 2.5;

  // Output...
   ds::Array2D<bs::Vertex> data; // Assumed to contain data if size isn't (0,0).
   
  // Helper method, this calculates the new position of a node given its last...
   void CalcPos(nat32 x,nat32 y,
                const ds::Array2D<bs::Vertex> & pos,const ds::Array2D<bs::Normal> & norm,
                const math::Vect<4> & centre,const ds::Array2D<real32> & weight,
                bs::Vertex & out) const;
};

//------------------------------------------------------------------------------
 };
};
#endif
