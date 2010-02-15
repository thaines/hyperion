#ifndef EOS_STEREO_EBP_H
#define EOS_STEREO_EBP_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

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


/// \file ebp.h
/// A belief propagation implimentation, you provide it with a DSI to indicate,
/// per pixel, the search range. Heavilly optimised.

#include "eos/types.h"
#include "eos/time/progress.h"
#include "eos/ds/arrays2d.h"
#include "eos/svt/field.h"
#include "eos/stereo/dsi.h"
#include "eos/stereo/dsr.h"
#include "eos/mem/packer.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// A belief propagation implimentation. Nothing more than the 
/// approach given in 'Efficient Belief Propagation For Early Vision'.
/// However, it takes a DSR as input, and searches disparities in the ranges
/// given.
/// This makes things rather more complicated.
class EOS_CLASS EBP : public DSI
{
 public:
  /// &nbsp;
   EBP();
   
  /// &nbsp;
   ~EBP();


  /// Sets the parameters:
  /// \param occCostBase The basic cost of occlusion. Defaults to 1.0.
  /// \param occCostMult The matching cost of two pixels is calculated and 
  ///                    multiplied by this. It is then added to occCostbase to
  ///                    get the occCost for that pixel pair. Defaults to -0.1
  /// \param occLimMult The maximum occlusion cost expressed as a multiplier of 
  ///                   the occCost. Defaults to 2.0.
  /// \param iters Number of iterations to do per layer. Defaults to 8.
  ///              Does not count for the top level of the hierachy, where for 
  ///              initalisation it calculates and does the number required to
  ///              obtain convergance.
  /// \param outCount Number of disparities to output for each pixel. Defaults to 1.
   void Set(real32 occCostBase,real32 occCostMult,real32 occLimMult,nat32 iters,nat32 outCount);

  /// Sets the search range object, which provides a per-pixel range of
  /// locations to search. Its cloned and internaly stored.
   void Set(DSR * dsr);

  /// Sets the DSC which defines the cost of each match. Its cloned and internaly stored.
   void Set(const DSC * dsc);

  /// Sets the DSC which defines the difference between adjacent pixels for
  /// calculating the occlusion cost. Its cloned and internaly stored.
  /// If not set then it defaults back to the other DSC for this purpose.
   void SetOcc(const DSC * dsc);


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
  static const nat32 blockSize = 16*1024*1024; // For the memory managment system used.
 
  // In...
   real32 occCostBase;
   real32 occCostMult;
   real32 occLimMult;
   int32 iters;
   nat32 outCount;
   
   DSR * dsr;
   DSC * dsc;
   DSC * dscOcc;
   
  // Out...
   struct Match
   {
    int32 disp;
    real32 cost;
    
    bit operator < (const Match & rhs) const
    {
     return cost > rhs.cost; // Perversly reversed for use by the heap in selecting the best.
    }
   };
   
   struct DispSort
   {
    static bit LessThan(const Match & lhs,const Match & rhs)
    {return lhs.disp<rhs.disp;}
   };

   struct Scanline 
   {
    ds::Array<nat32> index; // Comes with dummy at end.
    ds::Array<Match> data;
   };
  
   ds::ArrayDel<Scanline> disp;


  // Runtime...
   // Note: Structure of runtime storage.
   // Each pixel has the same storage pattern, which is of course variable size.
   // First is the structure block which indicates the structure of all further blocks.
   // Then is 5 following message blocks, the first is the message derived from
   // matching costs, then the 4 directions of the incomming messages.
   // Each of these message blocks is an array of floats, size and meaning specified
   // by the structure block.
   //
   // The structure block consists of:
   // 4 byte message passing flags, to indicate if messages should be passed out in each direction.
   // - byte for each direction, 0 to not pass, -1 to pass.
   // 4 byte number of floats in each message block.
   // 4 byte number of following sections to indicate structure of message block.
   //
   // Each following section consists of a 4 byte disparity for the first entry 
   // in the run, and a 4 byte run length.
   
   // Structures to assist with runtime storage...
    struct Section
    {
     int32 startDisp;
     nat32 runLength;
    };
   
    struct DispIter
    {
     nat32 msgSize;
     Section * sec;
     nat32 offset;
     real32 * pos; // In match cost message - offset by msgSize to get others.

     real32 & Value(nat32 i) {return *(pos + msgSize*i);} // i==0 for pixel cost, 1..4 for directions.
     int32 Disparity() {return sec->startDisp + int32(offset);}
     
     void ToNext()
     {
      ++offset; ++pos;
      if (offset==sec->runLength)
      {
       ++sec;
       offset = 0;
      }
     }
     
     void ToPrev()
     {
      --pos;
      if (offset==0)
      {
       --sec;
       offset = sec->runLength-1;
      }
      else --offset;
     }
    };
       
    struct Pixel
    {
     nat32 passFlags;
     nat32 msgSize; // Number of floats in each message.
     nat32 sections;
     real32 occCost[4]; // Occlusion cost for the 4 directions for the pixel.
     
     byte & Pass(nat32 i) {return ((byte*)(void*)(&passFlags))[i];}
     nat32 TotalBytes() {return sizeof(Pixel) + sections * sizeof(Section) + 5 * msgSize * sizeof(real32);}
     Section * GetSec(nat32 i) {return ((Section*)(void*)(this+1)) + i;}
     real32 * Start() {return (real32*)(void*)(((Section*)(void*)(this+1)) + sections);}

     void MakeIter(DispIter & iter)
     {
      iter.msgSize = msgSize;
      iter.sec = (Section*)(void*)(this+1);
      iter.offset = 0;
      iter.pos = Start();
     }
    };


   // Structures used for various 'lil steps...
    struct CostDisp // Sorts by cost.
    {
     int32 disp;
     real32 cost;
     
     bit operator < (const CostDisp & rhs) const {return cost<rhs.cost;}
    };


   // Method to do a single iteration, given the index of a layer.
   // Also given the iteration number, to suport a checkerboard message passing pattern.
    void Iter(ds::Array2D<Pixel*> & index,nat32 iter);
    
   // This method unions upto 4 Pixels, returning a new pixel allocated with 
   // the given mem::Packer. Given pointers can be null, they will not then be
   // included in the result. If all 4 are null it returns null.
   // The cost messages are summed, whilst the passing messages are simply
   // nulled.
    Pixel * Union(Pixel * pix[4],mem::Packer * memAlloc);
    
   // This extracts into a to pixel the message states from another, assuming 
   // the from pixel was created by unioning the to pixel with others previously.
   // (The reverse of union basically.)
    void GetMessages(Pixel * to,Pixel * from);
};

//------------------------------------------------------------------------------
 };
};
#endif
