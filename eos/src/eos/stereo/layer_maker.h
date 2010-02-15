#ifndef EOS_STEREO_LAYER_MAKER_H
#define EOS_STEREO_LAYER_MAKER_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file layer_maker.h
/// Impliments a mean shift clustering of planes fitted to segments to create 
/// layers of combined segments, which are presumed to be of the same plane.
/// The planes are then re-fitted to match this theory.

#include "eos/types.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"
#include "eos/stereo/plane_seg.h"
#include "eos/time/progress.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// This class represents a set of layers fitted to segments, and is essentially
/// section 4 of the paper 'A layered stereo matching algorithm using image
/// segmentation and global visibility constraints' by Bleyer & Gelautz. The
/// fitting is done by first clustering the segments using mean shift and a
/// plane distance measure, then grouping all segments that are suitably close 
/// to form layers, to which planes are then fitted. The class allows for the 
/// querrying of the data generated and then also the editting of which segments
/// belong to which layers, after which a method can be called to recalculate 
/// the planes fitted to layers with the new set of assigned segments.
class EOS_CLASS LayerMaker
{
 public:
  /// &nbsp;
   LayerMaker();

  /// &nbsp;
   ~LayerMaker();


  /// This initialises the class, should be called once and only once 
  /// straight after, or near enough, construction. radius is the 
  /// radius of the window used in the mean shift, 0.6-0.8
  /// being the range of values given in the paper.
   void Setup(svt::Core & core,PlaneSeg & planeSeg,real32 radius,time::Progress * prog = null<time::Progress*>());

  /// After editting by assignment to SegToLayer call this to rebuild the planes.
   void Rebuild(PlaneSeg & planeSeg,time::Progress * prog = null<time::Progress*>());
   
  /// This merges together layers, in the same way that Setup merges together segments.
   void LayerMerge(svt::Core & core,const svt::Field<nat32> & segs,PlaneSeg & planeSeg,real32 radius,time::Progress * prog = null<time::Progress*>());

  /// Returns the number of segments in existance.
   nat32 Segments();

  /// Returns the index of the layer for which the given segment belongs, you
  /// can use this to edit the fact.
   nat32 & SegToLayer(nat32 seg);

  /// Returns the number of layers in existance.
   nat32 Layers();

  /// Returns the plane for a given layer.
   const bs::PlaneABC & Plane(nat32 layer);
   
   
  /// Extracts the image segmented by layer, a useful diagnostic. The given 
  /// maps must have the same dimensions.
  /// \param seg The segmentation for the image.
  /// \param laySeg The output, a segmentation of the image by layer.
   void GetLayerSeg(const svt::Field<nat32> & seg,svt::Field<nat32> & laySeg);


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::LayerMaker";}


 private:
  // An array of all segments, each having the index of there respective
  // layer number...
   ds::Array<nat32> sed;

  // An array of all layers, containning the fitted plane for each one...
   ds::Array<bs::PlaneABC> layer;

 
 // The distance function used by mean shift...
  static bit DistFunc(nat32 fvSize,real32 * fv1,real32 * fv2,real32 pt);

 // Structure used for the cluster step - just yea basic forest...
  struct Node
  {
   Node * parent;
   nat32 layer; // Which layer it belongs to +1, set to 0 to indicate currently unassigned.

   Node * Head()
   {
    if (parent==null<Node*>()) return this;
    parent = parent->Head();
    return parent;
   }
  };
};

//------------------------------------------------------------------------------
 };
};
#endif
