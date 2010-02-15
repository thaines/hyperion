#ifndef EOS_SFS_LEE_H
#define EOS_SFS_LEE_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

/// \file lee.h
/// Impliments 'Shape from Shading with Perspective Projection' by Lee and Kuo.
/// (Albit without the perspective bit - the orthographic version.)

#include "eos/types.h"

#include "eos/time/progress.h"
#include "eos/bs/geo3d.h"
#include "eos/ds/arrays2d.h"
#include "eos/svt/field.h"
#include "eos/alg/multigrid.h"

namespace eos
{
 namespace sfs
 {
//------------------------------------------------------------------------------
/// Impliments 'Shape from Shading with Perspective Projection' by Lee and Kuo.
/// (Albit without the perspective bit - the orthographic version.)
/// Follows same interface pattern as other SfS algorithms.
class EOS_CLASS Lee
{
 public:
  /// &nbsp;
   Lee();
   
  /// &nbsp;
   ~Lee();


  /// Sets the image to use.
   void SetImage(const svt::Field<real32> & image);

  /// Sets an albedo map to use.
   void SetAlbedo(const svt::Field<real32> & albedo);

  /// Sets the light source direction, this comes as a normal that
  /// points towards the light at its infinite position.
   void SetLight(bs::Normal & norm);
   
  /// Set the parameters to use.
  /// \param iters Number of times to minimise the linearlisation of the reflectance function.
  /// \param initRad The depth map is initialised to an image filling ellipsoid, this is its z radius. Positive for a convex bias, negative for concave bias.
  /// \param startLambda Lambda for first iteration, linearly interpolated to endLambda for the last.
  /// \param endLambda Lambda for last iteration, linearly interpolated to startLambda for the first.
  /// \param multiIters Maximum number of iterations to use when solving the linearisation.
  /// \param tolerance Defines convergance for breaking out of the multigrid iterations early.
  /// \param speed [0,1], weight of linearisation update. 1 is traditional but 0.5 should be fastest.
   void SetParas(nat32 iters = 16,real32 initRad = 25.0,real32 startLambda = 2000.0,real32 endLambda = 0.0,
                 nat32 multiIters = 64,real32 tolerance = 0.001,real32 speed = 0.5);


  /// Blah.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Extracts the calculated needle map.
   void GetNeedle(svt::Field<bs::Normal> & out) const;
   
  /// Extracts the calculated depth map.
   void GetDepth(svt::Field<real32> & out) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  // Parameters...
   nat32 iters;
   real32 initRad;
   real32 startLambda;
   real32 endLambda;
   nat32 multiIters;
   real32 tolerance;
   real32 speed;
   
  // Inputs...
   svt::Field<real32> image;
   svt::Field<real32> albedo;
   bs::Normal toLight;
   
  // Outputs...
   ds::Array2D<real32> result; // Depths are at pixel corners, so this array is larger than the input by 1.
   
  
  // Helper structure...
   struct Site
   {
    real32 z; // Associated current depth.
    real32 dt[6]; // Contains the differentials relevant to a given node, for its 6 using triangles.
   };
   
  // Helper methods...
   // This fills in a multigrid object with the relevant a values and b values...
   // (First layer only for b values.)
   // (Uses current X values for differentials, centre pixel fixed to x of 0.).
   // (Yes, this is the entire complicated part of the algorithm.)
   // Multigrid must have a stencil of 13, with the entrys as set in Run.
   // (Data is a support data structure, passed in to avoid memory thrashing.)
    void PrepMultigrid(ds::ArrayDel< ds::Array2D<Site> > & data,ds::Array2D<real32> & irr,
                       alg::Multigrid2D & mg,real32 lambda,time::Progress * prog);
  
   // Calculates the irradiance given differentials of z. (Albedo is assumed as 1.)
    real32 Ref(real32 p,real32 q) // p=dz/dx, q=dz/dy
    {
     real32 ret = -toLight[0]*p - toLight[1]*q + toLight[2];
     ret /= math::Sqrt(math::Sqr(p) + math::Sqr(q) + 1.0);
     return ret;
    }
    
   // Calculates the irradiance differented wrt p...
    real32 RefDp(real32 p,real32 q)
    {
     real32 lenSqr = math::Sqr(p) + math::Sqr(q) + 1;
     return (-toLight[0]*lenSqr + p*(p*toLight[0] + q*toLight[1] - toLight[2]))
            /
            (lenSqr * math::Sqrt(lenSqr));
    }

   // Calculates the irradiance differented wrt q...
    real32 RefDq(real32 p,real32 q)
    {
     real32 lenSqr = math::Sqr(p) + math::Sqr(q) + 1;
     return (-toLight[1]*lenSqr + q*(p*toLight[0] + q*toLight[1] - toLight[2]))
            /
            (lenSqr * math::Sqrt(lenSqr));
    }
};

//------------------------------------------------------------------------------
 };
};
#endif
