#ifndef EOS_STEREO_HEBP_H
#define EOS_STEREO_HEBP_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

/// \file hebp.h
/// This uses the EBP algorithm in a hierachical framework, so no bootstrap 
/// algorithm is required to generate the DSR.

#include "eos/types.h"

#include "eos/stereo/ebp.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// This uses the EBP algorithm in a hierarchy so that a DSR is not required.
/// Simply runs the algorithm as multiple resolutions starting at a low 
/// resolution bootstrapping each run with the previous lower resolution run.
class EOS_CLASS HEBP : public DSI
{
 public:
  /// &nbsp;
   HEBP();
   
  /// &nbsp;
   ~HEBP();


  /// Sets the parameters for the EBP algorithm.
   void Set(real32 occCostBase,real32 occCostMult,real32 occLimMult,nat32 iters,nat32 outCount);

  /// Sets the DSC which defines the cost of each match. Its cloned and internaly stored.
   void Set(DSC * dsc);

  /// Optional, sets the DSC which defines the difference between adjacent
  /// pixels, for assigning occlusion costs.
  /// Its cloned and internaly stored.
   void SetOcc(DSC * dsc);

  /// Optional, sets the masks.
   void Set(const svt::Field<bit> & leftMask,const svt::Field<bit> & rightMask);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// &nbsp;
   nat32 Width() const;

  /// &nbsp;
   nat32 Height() const;
   
  /// &nbsp;
   nat32 Size(nat32 x, nat32 y) const;
   
  /// &nbsp;
   real32 Disp(nat32 x, nat32 y, nat32 i) const;

  /// &nbsp;   
   real32 Cost(nat32 x, nat32 y, nat32 i) const;

  /// &nbsp;   
   real32 DispWidth(nat32 x, nat32 y, nat32 i) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  static const nat32 search_range = 3;
  static const nat32 spread_range = 1;
 
  // Input...
   real32 occCostBase;
   real32 occCostMult;
   real32 occLimMult;
   nat32 iters;
   nat32 outCount;
   
   DSC * dsc;
   DSC * dscOcc;
   svt::Field<bit> leftMask;
   svt::Field<bit> rightMask;

  // Output (Just pass through for the EBP of the last layer.)...
   EBP * ebp;
};

//------------------------------------------------------------------------------
 };
};
#endif
