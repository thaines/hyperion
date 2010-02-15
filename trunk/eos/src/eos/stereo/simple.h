#ifndef EOS_STEREO_SIMPLE_H
#define EOS_STEREO_SIMPLE_H
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


/// \file simple.h
/// Provides some very simple stereo algorithms. Generally used for bootstrap 
/// rather than actual results.

#include "eos/types.h"

#include "eos/time/progress.h"
#include "eos/ds/arrays.h"
#include "eos/ds/arrays2d.h"
#include "eos/stereo/dsi.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// Defines the hierachical match cost interface. This is essentially the same
/// as the DSC object, except it has the hierachy built in. This can be used to
/// express such things as SAD, NCC etc in a way conveniant for a hierachical
/// algorithm to use.
///
/// The interface defines basic functionality to support this, including 
/// supplying the hierachy created from the DSC.
/// Can double as a DSC by exposing its bottom.
class EOS_CLASS HDSC : public Deletable
{
 public:
  /// &nbsp;
   HDSC();
 
  /// &nbsp;
   ~HDSC();

  /// As its passed arround making Clones is a desired capability.
   virtual HDSC * Clone() const = 0;


  /// Allows the user to set the DSC - from this the hierachy is automatically
  /// constructed. (When a child type is cloned this must be called.)
  /// Is cloned internally.
   void Set(DSC * dsc);

  /// Alternative version of set, for if you have masks - if so these will be 
  /// respected throughout.
   void Set(DSC * dsc,const svt::Field<bit> & leftMask,const svt::Field<bit> & rightMask);


  /// Returns how many levels exist in the hierachy.
   nat32 Levels() const;
   
  /// Returns the width of a given hierachy level, left side.
   nat32 WidthLeft(nat32 l) const;

  /// Returns the width of a given hierachy level, right side.
   nat32 WidthRight(nat32 l) const;
   
  /// Returns the height of a given hierachy level, side agnostic.
   nat32 Height(nat32 l) const;


  /// Primary method to be implimented by the interface implimentor - this 
  /// returns the cost of matching two pixels on a given hierachy level.
  /// (Can be given out of range coordinates.)
   virtual real32 Cost(nat32 level,int32 leftX,int32 rightX,int32 bothY) const = 0;

  /// Returns true if the indicated is valid, false otherwise.
   bit LeftMask(nat32 level,nat32 x,nat32 y) const;

  /// Returns true if the indicated is valid, false otherwise.
   bit RightMask(nat32 level,nat32 x,nat32 y) const;


  /// &nbsp;
   virtual cstrconst TypeString() const = 0;  


 protected:
  // This updates the given HDSC to have the same hierachy - as though the same 
  // Set method was called.
   void CloneMe(HDSC & target) const;


  // Returns the DSC, primarilly so you can call its cost function.
   DSC * GetDSC() {return dsc;}
   const DSC * GetDSC() const {return dsc;}

  // Returns the data associated with a pixel in the left image.
  // Usually fed into the DSC cost function.
   byte * Left(nat32 level,nat32 x,nat32 y) const;

  // Returns the data associated with a pixel in the right image.
  // Usually fed into the DSC cost function.
   byte * Right(nat32 level,nat32 x,nat32 y) const;



 private:
  // Basic inputs...
   DSC * dsc;
   svt::Field<bit> lMask;
   svt::Field<bit> rMask;

  // The hierachy...
   ds::ArrayDel< ds::Array2D<bit> > leftMask; // [level].Get(x,y)
   ds::ArrayDel< ds::Array2D<bit> > rightMask;
   ds::ArrayDel< ds::ArrayDel< mem::StackPtr<byte> > > leftCol; // [level][y].Ptr() + x * bytes.
   ds::ArrayDel< ds::ArrayDel< mem::StackPtr<byte> > > rightCol;

  // Helper, once the basic inputs have been set this can be called to build the
  // hierachy... Will terminate any previous data.
   void MakeHierachy();
};

//------------------------------------------------------------------------------
/// An implimentation of Bi-lateral absolute differencing, with the HDSC
/// interface. The RangeLuvDSC class is recomended as the input DSC.
/// This should produce a very good result despite its simplicity due to the
/// bilateral smoothing given it occluding boundary handling capability and
/// better ambiguity handling in smooth regions.
class EOS_CLASS BiLatAD : public HDSC
{
 public:
  /// &nbsp;
   BiLatAD();
   
  /// &nbsp;
   ~BiLatAD();

  /// &nbsp;
   HDSC * Clone() const;


  /// Sets the parameters. This is the window size, the standard deviation for
  /// distance at the radius of the window, and the distance of one standard 
  /// deviation for the DSC returned colour matching differences.
  /// Defaults are 4, 1.5 and 4.0 respectivly.
   void SetParas(nat32 radius,real32 distSd,real32 colScale);
  

  /// &nbsp;
   real32 Cost(nat32 level,int32 leftX,int32 rightX,int32 bothY) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  int32 radius;
  real32 distSd;
  real32 colScale;
};

//------------------------------------------------------------------------------
/// This is a simple hierachical stereo algorithm, primarilly for bootstrap 
/// purposes rather than actual output.
/// Uses a HDSC to provide cost information.
/// Has very few parameters due to its simplicity.
/// (Though actual implimentation is quite complex.)
class EOS_CLASS SimpleStereo : public DSI
{
 public:
  /// &nbsp;
   SimpleStereo();

  /// &nbsp;
   ~SimpleStereo();


  /// Sets the HDSC - this covers almost everything the algorithm needs.
  /// Its cloned internally.
   void Set(const HDSC * hdsc);

  /// Sets the algorithms parameters.
  /// \param limit Maximum number of disparities to assign to each output pixel. Defaults to 2.
   void Set(nat32 limit);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// &nbsp;
   nat32 Width() const;

  /// &nbsp;
   nat32 Height() const;

  /// &nbsp;
   nat32 Size(nat32 x,nat32 y) const;

  /// &nbsp;
   real32 Disp(nat32 x,nat32 y,nat32 i) const;

  /// &nbsp;
   real32 Cost(nat32 x,nat32 y,nat32 i) const;

  /// &nbsp;
   real32 DispWidth(nat32 x,nat32 y,nat32 i) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  static const nat32 expand = 3; // How many entries either side of a disparity on transfer. 
  static const nat32 blockSize = 4*1024*1024; // For the memory managment system used.
 
  // Input...
   HDSC * hdsc;
   nat32 limit;

  // Output...
   struct DispCost
   {
    int32 disp;
    real32 cost;
    
    bit operator < (const DispCost & rhs) const // Reversed, perversly, for selecting the n best.
    {
     return cost > rhs.cost;
    }
   };

   struct SortDC // Sorter class for DispCost, to sort by disp.
   {
    static bit LessThan(const DispCost & lhs,const DispCost & rhs)
    {return lhs.disp<rhs.disp;}
   };

   struct DispList // For runtime.
   {
    nat32 size;
    DispCost * data;
   };

   struct Scanline
   {
    ds::Array<nat32> index; // Has dummy on end, with length of data array in.
    ds::Array<DispCost> data; // Indexed by above.
   };

   ds::ArrayDel<Scanline> data;
};

//------------------------------------------------------------------------------
 };
};
#endif
