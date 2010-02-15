#ifndef EOS_STEREO_SAD_SEG_STEREO_H
#define EOS_STEREO_SAD_SEG_STEREO_H
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


/// \file sad_seg_stereo.h
/// Provides a very simple stereo algorithm, used to initialise a vastly more
/// complicated once.

#include "eos/types.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"
#include "eos/time/progress.h"
#include "eos/ds/arrays.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// This impliments a semi-sparse stereo algorithm based on multiple window 
/// sizes of sad, using segmentation and left-right consistancy checks as well
/// as reliability checking. It is the boot strap algorithm from the paper
/// 'A layered stereo matching algorithm using image segmentation and global
/// visibility constraints', by Michael Bleyer & Margrit Gelautz.
/// Uses the standard algorithm structure of a class where you call some
/// methods to prepare the algorithm, then call Run() to do the work and finally
/// use other methods to extract the results.
class EOS_CLASS SadSegStereo
{
 public:
  /// &nbsp;
   SadSegStereo();

  /// &nbsp;
   ~SadSegStereo();

  /// This supplies the left segmentation, you also supply the number of
  /// segments so it can construct its funky data structures. You must
  /// call this before Run().
   void SetSegments(nat32 count,const svt::Field<nat32> & segs);

  /// This adds a pair of 2D fields to the calculation, an entry for each
  /// channel. This must be called at least once before Run.
  /// All fields must be the same size, but between the left and right 
  /// set the width can be different.
   void AddField(const svt::Field<real32> & left,const svt::Field<real32> & right);

  /// Sets the range of disparity values to search through. They default to
  /// [-30..30] if this method is not called.
   void SetRange(int32 minDisp,int32 maxDisp);

  /// Sets the maximum absolute difference that can occur between two pixels,
  /// is used for out-of-bound situations. Should just be a large number, 
  /// defaults to 1e2.
   void SetMaxAD(real32 mad);

  /// Sets the minimum cluster size, and cluster of pixels with equal value 
  /// smaller than this will be concidered invalid. Defaults to 0.
   void SetMinCluster(nat32 th);

  /// Sets the maximum radius pass, it will start with a radius of 1, then go
  /// upto this value inclusive, defaults to 3. (i.e. 3x3 window to 7x7 window.)
   void SetMaxRadius(nat32 mr);


  /// This takes the given inputs and calculates and stores the output, provides a
  /// progress bar capability as it can take some time.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// This obtains the disparity values, note that not all values will be
  /// correct so you need to get the validity mask as well.
   void GetDisparity(svt::Field<real32> & disp);

  /// This obtains a mask to indicate which disparity values are correct and
  /// which are not. true indicates valid, false not valid.
   void GetMask(svt::Field<bit> & mask);


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::SadSegStereo";}


 private:
  // Input...
   int32 minDisp;
   int32 maxDisp;

   real32 maxDiff;
   nat32 maxRad;
   nat32 minSeg;

   nat32 segments;
   svt::Field<nat32> segs;

   ds::Array< Pair< svt::Field<real32>, svt::Field<real32> > > in;

  // Output...
   svt::Var * out;

  // Internal...
   int32 minRange; // These exist because the checking nature of the algorithm requires us to often go beyond the range passed in.
   int32 maxRange; // Essentially the actual range we use for sad stereo, we just only match inside the minDisp/maxDisp range.
  
   struct Seg // Details for each segment, used in the guts of the algorithm.
   {
    int32 min,max; // Minimum and maximum validated disparities.
    nat32 validCount; // valid pixels in segment.
    nat32 invalidCount; // Invalid pixels in segment.
  
    bit Valid() {return validCount > invalidCount;}
   };
   ds::Array<Seg> sed;

   struct Node // Used by the KillIsolated method to make it suitably fast.
   {
    Node * parent;
    nat32 size;

    Node * Head()
    {
     if (parent==null<Node*>()) return this;
     parent = parent->Head();
     return parent;
    }
   };

   void DoPass(time::Progress * prog,nat32 radius);
   void KillIsolated(svt::Field<bit> & valid,svt::Field<real32> & disp);
};

//------------------------------------------------------------------------------
 };
};
#endif
