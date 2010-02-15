#ifndef EOS_STEREO_DSI_MS_H
#define EOS_STEREO_DSI_MS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file dsi_ms.h
/// A simple algorithm, generates a single-pixel sparse DSI via a hierachical 
/// method then clusters via mean-shift to create a stereo-aware segmentation 
/// algorithm. A further class is provided to apply overlap-aware plane fitting
/// followed by plane selection for overlaped areas using this data is also 
/// provided.

#include "eos/types.h"
#include "eos/ds/arrays.h"
#include "eos/ds/arrays_ns.h"
#include "eos/ds/arrays2d.h"
#include "eos/ds/windows.h"
#include "eos/svt/field.h"
#include "eos/bs/colours.h"
#include "eos/time/progress.h"
#include "eos/file/csv.h"
#include "eos/cam/files.h"
#include "eos/bs/geo3d.h"
#include "eos/stereo/dsi.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// This is a rather crazy approach to DSI generation. It uses range matching
/// as its basis, this being influenced in part by Birchfield and Tomasi 1998.
/// It is based on the hierarchical approach of Meerbergen, Vergauwen, Pollefeys
/// and Gool 2002. There approach is a simple stereo algorithm using dynamic 
/// programming in a hierarchy. This uses the same algorithm, but instead of 
/// extracting the one best path it calculates, via a reverse iteration 
/// over the dynamic programming structure, the cost of the cheapest path passing
/// through each disparity. It then passes through to the next level all
/// disparities with a cost within a given margin of the best.
///
/// Allows you to provide an arbitary matching cost for all the pixels by 
/// providing left/right fields, difference functions and a parameter for the 
/// difference function. This allows this class to be used in a large variety of situations.
///
/// Additionally, disparities are allowed in both images simultaneously - this is 
/// necesary once you move to a sparse environment and has the advantage that 
/// it doesn't try to solve areas which have insufficient info as it just sets 
/// them to occluded.
///
/// This results in a sparse set of (sorted) disparities for each pixel, 
/// each with assigned costs. Each disparity should, due to the range matching,
/// be considered to represent a range of possible matches, +/- 0.5 from the 
/// disparity, where somewhere in that range the given matching cost can be 
/// obtained.
class EOS_CLASS SparseDSI : public DSI
{
 public:
  /// &nbsp;
   SparseDSI();

  /// &nbsp;
   ~SparseDSI();


  /// Sets the parameters.
  /// \param occCost The cost of an occlusion. Should be set considering the 
  ///                difference operator of the LuvRange class. Defaults to 1.0.
  /// \param vertCost The cost per unit disparity of a difference between two 
  ///                 scanlines. Because of its sparse nature it takes the minimum
  ///                 cost from all the disparitys in the previous scanline.
  ///                 Defaults to 1.0.
  /// \param vertMult The entire vertical cost - the previous row cost + the 
  ///                 difference cost multiplied by vertCost, is multiplied by this.
  ///                 Defaults to 0.2. Without this the vertical cost swamps the 
  ///                 other costs - must be less than 1.
  /// \param errLim This is the amount of error per pixel allowed for extra 
  ///               matches beyond the best to be considered. This value is 
  ///               multiplied by the number of pixels per scanline before use -
  ///               which is diferent at each level of the hierachy.
  ///               Defaults to 0.1.
   void Set(real32 occCost,real32 vertCost,real32 vertMult,real32 errLim);

  /// Sets the DSC which defines the cost of each match. Its cloned and internaly stored.
   void Set(DSC * dsc);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// The class user should know this, but provided anyway.
   nat32 Width() const {return width;}

  /// The class user should know this, but provided anyway.
   nat32 Height() const {return height;}
  
  /// Returns how many disparities have been assigned to a particular pixel.
   nat32 Size(nat32 x,nat32 y) const;
   
  /// The width of the the right hand image.
   nat32 WidthRight() const {return widthRight;}

  /// The height of the the right hand image.
   nat32 HeightRight() const {return heightRight;}

  /// Returns the disparity of a given entry at a location.
  /// Disparities are by default sorted, so you get the lower once first.
  /// This makes finding the mode very easy.
   real32 Disp(nat32 x,nat32 y,nat32 i) const;

  /// Returns the cost assigned to a disparity at a location.
  /// Lower costs are better, 0 is the best, this value is 
  /// adjusted to be image width independent.
   real32 Cost(nat32 x,nat32 y,nat32 i) const;
   
  /// Returns the 'disparity width', i.e. each returned disparity is over the 
  /// range of the value returned +/- this value.
   real32 DispWidth(nat32 x,nat32 y,nat32 i) const {return 0.5;}
   
   
  /// Re-sorts the disparities by cost, so that the first index for any pixel 
  /// has the lowest cost, the next the second lowest etc.
   void SortByCost(time::Progress * prog = null<time::Progress*>());


  /// &nbsp;
   cstrconst TypeString() const {return "eos::stereo::SparseDSI";}


 private:
  static const int32 range = 3; // At each level +/- this inclusive is the search range arround each offset.
 
  real32 occCost; // Multiplied by the size of the disparity, to discourage large disparities.
  real32 horizCost; // Falloff from horizontal costs used for pruning purposes.
  real32 vertCost; // Cost per unit disparity for vertical offset.
  real32 vertMult; // Multiplier of vertical cost, to lower it to something reasonable.
  real32 errLim; // This is multiplied by the number of pixels in a scanline before use.
  
  DSC * dsc;


  // The structure used for final storage once the class has finished running.
   struct DispCost
   {
    bit operator < (const DispCost & rhs) const // Sorts by cost.
    {
     return cost < rhs.cost;
    }
   
    int32 disp;
    real32 cost; // Lower is better.
   };
   
  // Structure to represent each line.
   struct Scanline
   {
    Scanline & operator = (const Scanline & rhs)
    {
     data.Size(rhs.data.Size());
     for (nat32 i=0;i<data.Size();i++) data[i] = rhs.data[i];

     index.Size(rhs.index.Size());
     for (nat32 i=0;i<index.Size();i++) index[i] = rhs.index[i];

     return *this;
    }

    ds::Array<DispCost> data;
    ds::Array<nat32> index; // Allways width+1 in length, final entry being size and a dummy.
   };
   
  // All the data, a bit hairy but required to be done this way so it can all work.
   nat32 width;
   nat32 height;
   ds::ArrayDel<Scanline> data; // height scanline objects, each with indexes width+1.
   
  // Can prove useful to store this for future use...
   nat32 widthRight;
   nat32 heightRight;
   


  // The data structure used - arrays of these are created and treated rather
  // like computer programs to be interpreted.
  // The arrays tend to be small, but can get particularly large if large 
  // untextured areas exist.
   struct Node
   {
    int32 left; // Scanline offset in the left image.
    int32 right; // Scanline offset in the right image.    

    real32 cost; // Cost of using this match, filled in cost pass.
    real32 incCost; // Incrimental score so far, filled in first pass.
    real32 decCost; // Decrimental score so far, filled in second pass.
    
    // disparity = right-left.
    // Cost of route through node is incCost+decCost-cost. i.e. incCost and DecCost each include cost.
   };


  // Helper class - used to make decleration of the pass array possible...
   class DoubleNatWindow : public ds::Window<nat32>
   {
    public:
     DoubleNatWindow():ds::Window<nat32>(2) {}
   };


  // Helper methods...
   // The first pass for a level - implicitly calculate the cheapest path, 
   // without storing it because we don't care.
    void FirstPass(nat32 level,ds::ArrayNS<Node> & prog,ds::ArrayDel<DoubleNatWindow> & pass);

   // The second pass for a level - backtrack and calculate the cheapest 
   // possible cost for each pixel/disparity.
    void SecondPass(nat32 level,ds::ArrayNS<Node> & prog,ds::ArrayDel<DoubleNatWindow> & pass);

   // Extract from the final data structure the offset position to use for the 
   // next level. These offset positions for the final level are the sparse DSI 
   // position, just need to be pruned down to size and indexed.
     void ExtractPass(nat32 level,ds::ArrayNS<Node> & prog);

   // Given a prog this converts from an offset prog to a set of disparities, 
   // whilst increasing the size. Called between levels. Will change the size 
   // of prog. (It detects and handles the special case of either side being 
   // of size 1 to extend range if need be.)
    void PrepProg(nat32 level,ds::ArrayNS<Node> & prog);
    
   // This extracts a final sparse dsi storage structure from the Node array, 
   // returns an array of Disps, and outputs into a suitably sized indexing array for that line.
    void Extract(ds::ArrayNS<Node> & prog,ds::Array<nat32> & index,ds::Array<DispCost> & out);

   // Given an input scanline and an output scanline this sets the output to the
   // halfed scanline. When multiple values map to the same value in the half 
   // scanline there minimum cost is recorded.
    static void HalfScanline(const Scanline & in,Scanline & out);
    
   /// Prepares a scanline to have vertical cost called on it by propagating 
   /// scores so each represents the lowest obtainable cost at that point.
    void PropagateScanline(Scanline & sl) const;


  // Helper functions...
   // This returns the zero intercept cost of a node - given any two nodes the
   // one with the smaller zero intercept is the one to be kept as it will 
   // produce the better occlusion scores...
    real32 IncCost(const Node & node)
    {
     return node.incCost - occCost * real32(node.left + node.right);
    }
  
   // This returns the cost of going from node 'from' to node 'to'.
   // Includes the incCost from 'from' but not the to.cost, which is needed for 
   // the final cost.
    real32 IncCost(Node & from,Node & to)
    {
     real32 ret = from.incCost;
     ret += occCost * real32(to.left-from.left + to.right-from.right - 2);
     return ret;
    }
    
   // This calculates the total cost as intercepted with the zero intercept,
   // i.e. this is the cost of taking the best path to this node then occlusion
   // the rest of the way.
    real32 DecCost(const Node & node)
    {
     return node.decCost + occCost * real32(node.left + node.right);
    }
    
   // See previous 2 functions.
    real32 DecCost(Node & from,Node & to)
    {
     real32 ret = from.decCost;
     ret += occCost * real32(from.left-to.left + from.right-to.right - 2);
     return ret;
    }
};

//------------------------------------------------------------------------------
/// This selects from a SparseDSI a single disparity value for every pixel.
/// Uses an extremlly simplistic method to select the best disparity for each 
/// pixel.
/// It first updates all the costs using a window based scheme before simply 
/// selecting the minimum cost.
/// The window simply assumes fronto-parallel planes through each disparity in
/// DSI space and sums costs, of all the disparities it intercepts. When it 
/// misses a pixels disparities it takes the minimum of then plus an extra 
/// distance cost.
class EOS_CLASS DispSelect
{
 public:
  /// &nbsp;
   DispSelect();
   
  /// &nbsp;
   ~DispSelect();

  
  /// Sets the radius, defaults to 1 making for a 3x3 window.
  /// Sets the cost for being out by one disparity. Defautls to 1.0
   void Set(nat32 radius,real32 mult);

  /// Sets the SparseDSI to use.
   void Set(const SparseDSI & sdsi);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Extracts the disparity map.
   void Get(svt::Field<real32> & out);

  
  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::DispSelect";}


 private:
  // In...
   nat32 radius;
   real32 mult;
   const SparseDSI * sdsi;
  
  // Out...
   ds::Array2D<real32> disp;
};

//------------------------------------------------------------------------------
/// Given a SparseDSI object this creates a mirror structure of 3D line segments
/// for each pixel, also provides the centre. Copys over the costs, so you can 
/// then delete the source SparseDSI and use this directly. This essentially 
/// takes you from disparity to actual 3D, whilst mantaining the idea that 
/// disparities represent ranges +/- 0.5 rather than specific points.
class EOS_CLASS SparsePos
{
 public:
  /// Cap defines the maximum number of positions to calculate per pixel.
  /// It selects the disparity entrys with the lowest cost, as you would expect.
   SparsePos(nat32 cap = 0xFFFFFFFF);
   
  /// &nbsp;
   ~SparsePos();


  /// Sets the cap.
   void Set(nat32 c) {cap = c;}

  /// Sets the SparsePos from a DSI, allows for progress reporting as this
  /// isn't exactly the fastest method ever devised.
  /// NOTE: sdsi must be sorted by cost, i.e. SortByCost(); must have been called.
   void Set(const DSI & sdsi,const cam::CameraPair & pair,time::Progress * prog = null<time::Progress*>());


  /// The class user should know this, but provided anyway.
   nat32 Width() const;

  /// The class user should know this, but provided anyway.
   nat32 Height() const;


  /// Returns how many segments have been assigned to a particular pixel.
   nat32 Size(nat32 x,nat32 y) const;

  /// Returns the start position of the line segment at a given pixel with a 
  /// given index.
  /// Sorted so that the most distant position comes first.
   const bs::Vertex & Start(nat32 x,nat32 y,nat32 i) const;

  /// Returns the end position of the line segment at a given pixel with a 
  /// given index.
  /// Sorted so that the most distant position comes first.
   const bs::Vertex & End(nat32 x,nat32 y,nat32 i) const;
   
  /// Returns the centre of the line segment at a given pixel with a 
  /// given index.
  /// Sorted so that the most distant position comes first.
   const bs::Vertex & Centre(nat32 x,nat32 y,nat32 i) const;

  /// Returns the cost assigned to a position at a given index.
   real32 Cost(nat32 x,nat32 y,nat32 i) const;


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::SparsePos";}


 private:
  nat32 cap;
 
  // Represents a position & its cost...
   struct Node
   {
    bs::Vertex start;
    bs::Vertex centre; // Just the half way between start and end.
    bs::Vertex end;
    real32 cost;
   };

  // Represents a Row...
   struct Row
   {
    ds::Array<nat32> index; // Allways width+1, to make size determination easy.
    ds::Array<Node> data; // Indexed by the above index for x coordinate.
   };

  // The rows that make up the left image...
   nat32 width;
   nat32 height;
   ds::ArrayDel<Row> data;
};

//------------------------------------------------------------------------------
/// Given a SparsePos and an image segmentation this assigns a plane to each 
/// segment by fitting to the positions avaliable. Also provides a planes to 
/// disparity map capability.
class EOS_CLASS SparsePlane
{
 public:
  /// &nbsp;
   SparsePlane();

  /// &nbsp;
   ~SparsePlane();


  /// Sets the input 3D positions with costs.
   void Set(const SparsePos & spos);

  /// Sets the segmentation.
   void Set(const svt::Field<nat32> & seg);
   
  /// If set to true segment 0 is assumed to be a dump for bad segments, 
  /// and is automatically assigned a plane at infinity rather than having any 
  /// work done to it.
   void Duds(bit enable);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Obtains how many segments were found in the segmentation map.
   nat32 Segments() const;

  /// Obtains the plane for a given segment. Will be normalised.
   const bs::Plane Seg(nat32 i) const;


  /// Outputs a map of 3D positions for every pixel.
   void PosMap(const cam::CameraPair & pair,svt::Field<bs::Vertex> & out) const;

  /// Outputs a disparity map.
   void DispMap(const cam::CameraPair & pair,svt::Field<real32> & out) const;


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::SparsePlane";}


 private:
  bit duds;

  // Input...
   const SparsePos * spos;
   svt::Field<nat32> seg;

  // Output...
   ds::Array<bs::Plane> plane;
};

//------------------------------------------------------------------------------
/// Given a SparsePos and an image segmentation this assigns a plane to each 
/// pixel by fitting to the positions within the same segment in a small window.
/// Will create a disparity map from this on request.
class EOS_CLASS LocalSparsePlane
{
 public:
  /// &nbsp;
   LocalSparsePlane();

  /// &nbsp;
   ~LocalSparsePlane();


  /// Sets the radius of the window used for plane fitting. Defaults to 3.
   void Set(nat32 radius);

  /// Sets the input 3D positions with costs.
   void Set(const SparsePos & spos);

  /// Sets the segmentation.
   void Set(const svt::Field<nat32> & seg);

  /// Sets the camera configuration, so it can switch between disparity/position space.
   void Set(const cam::CameraPair & pair);
   
  /// If set to true segment 0 is assumed to be a dump for bad segments, 
  /// and is automatically assigned a plane at infinity rather than having any 
  /// work done to it.
   void Duds(bit enable);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Outputs a map of 3D positions for every pixel.
   void PosMap(svt::Field<bs::Vertex> & out) const;

  /// Outputs a needle map.
   void NeedleMap(svt::Field<bs::Normal> & out) const;

  /// Outputs a disparity map.
   void DispMap(svt::Field<real32> & out) const;


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::LocalSparsePlane";}


 private:
  bit duds;

  // Input...
   int32 radius; // Window is radius*2 + 1 in size.
   const SparsePos * spos;
   svt::Field<nat32> seg;
   cam::CameraPair pair;

  // Output...
   ds::Array2D<bs::PosDir> data;
};

//------------------------------------------------------------------------------
/// Just like LocalPlane this is given a SparsePos and an image segmentation
/// which it uses to assign a plane to each pixel by fitting to the positions
/// within the same segment in a small window.
/// This version is additionally provided with a normal map, the planes are then
/// restricted to being perpendicular to the normals in the map.
/// With smoothly varying normals this will ushally produce a smooth surface, it
/// is also more robust as kernel density estimation is used, making outliers
/// irrelevant.
class EOS_CLASS OrientSparsePlane
{
 public:
  /// &nbsp;
   OrientSparsePlane();

  /// &nbsp;
   ~OrientSparsePlane();


  /// Sets the radius of the window used for plane fitting. Defaults to 3.
   void Set(nat32 radius);

  /// Sets the input 3D positions with costs.
   void Set(const SparsePos & spos);

  /// Sets the segmentation.
   void Set(const svt::Field<nat32> & seg);
   
  /// Sets the needle map, sets the orientation of each and every plane.
   void Set(const svt::Field<bs::Normal> & needle);

  /// Sets the camera configuration, so it can switch between disparity/position space.
   void Set(const cam::CameraPair & pair);
   
  /// If set to true segment 0 is assumed to be a dump for bad segments, 
  /// and is automatically assigned a plane at infinity rather than having any 
  /// work done to it.
   void Duds(bit enable);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// Outputs a map of 3D positions for every pixel.
   void PosMap(svt::Field<bs::Vertex> & out) const;

  /// Outputs a needle map.
   void NeedleMap(svt::Field<bs::Normal> & out) const;

  /// Outputs a disparity map.
   void DispMap(svt::Field<real32> & out) const;


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::stereo::OrientSparsePlane";}


 private:
  bit duds;

  // Input...
   int32 radius; // Window is radius*2 + 1 in size.
   const SparsePos * spos;
   svt::Field<nat32> seg;
   svt::Field<bs::Normal> needle;
   cam::CameraPair pair;

  // Output...
   ds::Array2D<bs::PosDir> data;
};

//------------------------------------------------------------------------------
 };
};
#endif
