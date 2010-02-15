#ifndef EOS_STEREO_SIMPLE_BP_H
#define EOS_STEREO_SIMPLE_BP_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


/// \file simpleBP.h
/// Provides a simple belief propagation based stereo algorithm. Results arn't
/// that good and its not that fast, but it still beats the socks off any local
/// method.


#include "eos/types.h"
#include "eos/mem/alloc.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"
#include "eos/time/progress.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// A simple stereo algorithm, specifically the one implimented in 'Efficient
/// Belief Propagation for Early Vision' by Felzenszwalb & Huttenlocher.
/// Implimented here for much the same reasons, to test my BP implimentation
/// and compare my output with theres. Also, you can never have too many stereo
/// algorithms.
class EOS_CLASS SimpleBP
{
 public:
  /// &nbsp;
   SimpleBP();
   
  /// &nbsp;
   ~SimpleBP();
   
   
  /// Sets the image pair, it calculates the disparity for the left.
  /// Black and white only folks.
   void SetPair(const svt::Field<real32> & left,const svt::Field<real32> & right);
  
  /// Sets the disparity range to concider, defaults to [-30,30].
   void SetDisp(int32 minDisp,int32 maxDisp);
   
  /// Sets the algorithm parameters. Defaults to the same once given in the paper,
  /// vLimit = 1.7, dLimit = 15/255 and dMult = 255*0.07. (They use 0..255
  /// whilst I use 0..1)
   void SetParas(real32 vLimit,real32 dLimit,real32 dMult);
   
   
  /// Runs the algorithm, provides a progress report.
   void Run(time::Progress * prog = null<time::Progress*>());
   
   
  /// Extracts the disparity output.
   void GetDisparity(svt::Field<int32> & out);
  
 
  /// &nbsp;
   static inline cstrconst TypeString()
   {
    return "eos::stereo::SimpleBP";
   }


 private: 
  // Inputs...
   svt::Field<real32> left;
   svt::Field<real32> right;
  
   int32 minDisp;
   int32 maxDisp;
   
   real32 vLimit;
   real32 dLimit;
   real32 dMult;

 // Outputs...
  svt::Var * output;
  svt::Field<int32> disp;
  
 // The cost functions...
  static real32 D(const SimpleBP & data,nat32 x,nat32 y,nat32 label)
  {
   int32 oPos = x + label + data.minDisp;
   if ((oPos<0)||(oPos>=int32(data.right.Size(0)))) return data.dMult*data.dLimit;
 
   return data.dMult*math::Min(math::Abs(data.left.Get(x,y) - data.right.Get(oPos,y)),data.dLimit);
  }

  static real32 VM(const SimpleBP & data) {return 1.0;}
  static real32 VT(const SimpleBP & data) {return data.vLimit;}
};

//------------------------------------------------------------------------------
 };
};
#endif
