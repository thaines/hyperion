#ifndef EOS_STEREO_SFG_STEREO_H
#define EOS_STEREO_SFG_STEREO_H
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


/// \file sfg_stereo.h
/// Shaded Factor-Graph Stereo, an algorithm of my own design. Uses factor 
/// graphs and shape from shading in combination with stereo to produce a 
/// disparity map, a surface normal map and an albedo map.

#include "eos/types.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"
#include "eos/time/progress.h"
#include "eos/bs/colours.h"
#include "eos/bs/geo3d.h"
#include "eos/cam/disparity_converter.h"
#include "eos/inf/field_graphs.h"
#include "eos/inf/fig_variables.h"
#include "eos/alg/shapes.h"

namespace eos
{
 namespace stereo
 {
//------------------------------------------------------------------------------
/// This creates the correct relationship between disparity and surface normals,
/// with the surface normals represented as indexes into a alg::HemiOfNorm. Uses
/// a laplace term, as that provides a sort of anti-aliasing.
/// Painfully slow, being an O(d^3) monstrosity.
/// Support is only provided for loopy min-sum belief propagation, no other 
/// run-time modes will work.
class EOS_CLASS DisparityOrient : public inf::Function
{
 public:
  /// &nbsp;
   DisparityOrient();
   
  /// &nbsp;
   ~DisparityOrient();


  /// Sets the various things it needs to know, specifically the number of 
  /// disparities, the depth that each disparity maps to and the HemiOfNorm,
  /// which provides the number of orientation labels and there normals.
  /// The given data structures are all copied.
  /// Also sets the costs, maxCost being the maximum cost allowed, 
  /// with angMult being the multiplier of the angle between two normals used
  /// to decide how much something should cost. angMult is scaled such that 1
  /// is the closest possible angle betwen two nodes on the HemiOfNorm.
  /// Note that labels and hon.Norms() must neither excede 255, as 8 bits are 
  /// used for indexing.
   void Set(real32 maxCost,real32 angMult,nat32 labels,real32 * depth,const alg::HemiOfNorm & hon);


  /// &nbsp;
   nat32 Links() const;

  /// &nbsp;
   void LinkType(nat32 ind,inf::MessageClass & out) const;


  /// &nbsp;
   void ToSP();

  /// &nbsp;
   void SendOneSP(nat32 instance,const inf::MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneSP(nat32 instance,const inf::MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllSP(nat32 instance,const inf::MessageSet & ms);


  /// &nbsp;
   void ToMS();

  /// &nbsp;
   void SendOneMS(nat32 instance,const inf::MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneMS(nat32 instance,const inf::MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllMS(nat32 instance,const inf::MessageSet & ms);


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  real32 maxCost;
  real32 angMult;

  nat32 labels;
  real32 * depth;
  alg::HemiOfNorm hon;
  
  
  // This structure represents the cost for the given indexing combination, used
  // to create a sparse data structure representing all the valid combinations.
   struct Node
   {
    real32 cost;
    nat16 index[4];
   };
   
  // This structure represents an instruction, message calculation consists of
  // interpreting a 'program' that is a sequence of these instructions.
   struct Inst
   {
    real32 cost;
    nat8 unwind[4];
    nat16 index[4];
   }; // 16 bytes.
  
  // The 'lookup table', or interpreted 'program'.
  // Simply a sequence of instructions that needs to be iterated to create any 
  // actual output. By changing the interpreter the same instruction sequence
  // will create all four possible output messages...
   nat32 ltSize;
   Inst * lt;
   
  // 'Marginal distributions' of the lookup table, used to remove bias...
   real32 * dist[4];
};

//------------------------------------------------------------------------------
/// Uses DisparityOrient to calculate a discrete surface orientation layer
/// from a disparity layer. Slow.
class EOS_CLASS GridDisparityOrient2D : public inf::FactorPattern
{
 public:
  /// &nbsp;
   GridDisparityOrient2D();
   
  /// &nbsp;
   ~GridDisparityOrient2D();


  /// Sets the various parameters, that are then passed through to the 
  /// DisparityOrient. Copys are made.
   void Set(real32 maxCost,real32 angMult,
            const cam::DispConv & dc,int32 minDisp,int32 maxDisp,
            const alg::HemiOfNorm & hon);
  
  
  /// &nbsp;
   nat32 PipeCount() const;

  /// &nbsp;
   bit PipeIsSet(nat32 ind) const;

  /// &nbsp;
   const inf::VariablePattern & PipeGet(nat32 ind) const;

  /// &nbsp;
   bit PipeSet(nat32 ind,const inf::VariablePattern & vp);


  /// &nbsp;
   void Construct(nat32 level,inf::FactorConstruct & fc) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  real32 maxCost;
  real32 angMult;

  nat32 labels;
  real32 * depth;
  alg::HemiOfNorm hon;
  
  inf::Grid2D * pipe[2];
};

//------------------------------------------------------------------------------
/// Given two surface orientation labels indicated by disparity this applys a 
/// capped constant albedo constraint between the orientations using the SFS 
/// constraint. Is given irradiance info so it can do this.
class EOS_CLASS AlbedoOrient : public inf::Function
{
 public:
  /// &nbsp;
   AlbedoOrient();
   
  /// &nbsp;
   ~AlbedoOrient();


  /// Sets all the relevant parameters.
  /// \param angAlias Used for 'anti-aliasing' when transfering from 
  ///                 orientation to cone constraint cos angle. Essentially split 
  ///                 between either side of a position.
  /// \param maxCost Cost of not meeting constraint.
  /// \param angRes Resolution of the cone-cos(angle) table used.
  /// \param toLight Normalised normal pointing at light source.
  /// \param hon Specifies the number and directions of the surface orientations.
  ///            A copy is made for internal use.
  /// \param irr The image irradiance. You should declare there to be 
  ///            (width-1)*height + width*(height-1) instances, as that is what 
  ///            this assumes based on irr.
   void Set(real32 angAlias,real32 maxCost,nat32 angRes,
            const bs::Normal & toLight,const alg::HemiOfNorm & hon,
            const svt::Field<real32> & irr);


  /// &nbsp;
   nat32 Links() const;

  /// &nbsp;
   void LinkType(nat32 ind,inf::MessageClass & out) const;


  /// &nbsp;
   void ToSP();

  /// &nbsp;
   void SendOneSP(nat32 instance,const inf::MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneSP(nat32 instance,const inf::MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllSP(nat32 instance,const inf::MessageSet & ms);


  /// &nbsp;
   void ToMS();

  /// &nbsp;
   void SendOneMS(nat32 instance,const inf::MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneMS(nat32 instance,const inf::MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllMS(nat32 instance,const inf::MessageSet & ms);


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  real32 angAlias;
  real32 maxCost;
  nat32 angRes;
  bs::Normal toLight;
  alg::HemiOfNorm hon;

  svt::Field<real32> irr;


  // Lookup table, used both ways, converts each orientation into 2 
  // cone-cos(angle) indexes with anti-aliasing costs...
   struct ConeCosAng
   {
    int32 index[2];
    real32 cost[2];
   } * orToCCA;
   
  // Data structure used during run-time - stored here to avoid the snail that 
  // is malloc from eatting performance for breakfast...
   real32 * cca[2];
};

//------------------------------------------------------------------------------
/// Factor Pattern for the AlbedoOrient Function. Applys a SfS based smoothing 
/// term to a 2D grid field representing surface orientations.
class EOS_CLASS GridAlbedoOrient2D : public inf::FactorPattern
{
 public:
  /// &nbsp;
   GridAlbedoOrient2D();
   
  /// &nbsp;
   ~GridAlbedoOrient2D();


  /// Sets the various parameters, that are then passed through to the 
  /// AlbedoOrient. Copys are made. 
   void Set(real32 angAlias,real32 maxCost,nat32 angRes,
            const bs::Normal & toLight,const alg::HemiOfNorm & hon,
            const svt::Field<real32> & irr);
  
  
  /// &nbsp;
   nat32 PipeCount() const;

  /// &nbsp;
   bit PipeIsSet(nat32 ind) const;

  /// &nbsp;
   const inf::VariablePattern & PipeGet(nat32 ind) const;

  /// &nbsp;
   bit PipeSet(nat32 ind,const inf::VariablePattern & vp);


  /// &nbsp;
   void Construct(nat32 level,inf::FactorConstruct & fc) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  real32 angAlias;
  real32 maxCost;
  nat32 angRes;
  bs::Normal toLight;
  alg::HemiOfNorm hon;
  svt::Field<real32> irr;
  
  inf::Grid2D * pipe;

  // Cache of down-sized iradiance maps...
   struct Node
   {
    svt::Var * var;
    svt::Field<real32> irr;
   };
   mutable ds::Array<Node> cache;
};

//------------------------------------------------------------------------------
/// This is a brute force implimentation of a SFS based smoothing method over 
/// 2x2 regions. Essentially should be roughly equivalent to DisparitySmoothSFS,
/// though this doesn't suffer from as much approximation and hence should 
/// produce better results. If your not dead before it finishes running that is.
/// Has the same restriction in that it only suports loopy MS.
class EOS_CLASS BruteDisparitySmoothSFS : public inf::Function
{
 public:
  /// &nbsp;
   BruteDisparitySmoothSFS();

  /// &nbsp;
   ~BruteDisparitySmoothSFS();


  /// Sets various costs used, must be called before use.
  /// \param dir If false then it uses the positive diagonal direction, if true
  ///            the negative.
  /// \param labels Number of labels used, i.e. disparities avaliable.
  /// \param depth Depth that each disparity label matches to, labels size.
  /// \param toLight Normal to infinitly distant point light source.
  /// \param cost Sets the cost of not fitting the SfS constraint, as this is
  ///             essentially a Potts term. The cost of matching the SfS
  ///             constraint is the typical zero.
  /// \param stride Ushally the width of the image-1, to account for the 2x2 
  ///               size of this factor.
  /// \param irr Irradiance field, treated as a 2D array indexed by instance 
  ///            using the stride measurement to divide the instance to get a 2D
  ///            coordinate for the top left of the 2x2 square.
  ///            The data structure behind this must be preserved until this has 
  ///            finished.
  /// \param irrRes How many levels irradiance is broken up into, does not include 0.
  ///               255 is the typical value, it 'only' changes the size of the 
  ///               lookup table, not exection time directly. Though a larger value
  ///               makes for a larger lookup table, which means more cache misses
  ///               and a longer creation time for the table, which is not insignificant.
   void Set(bit dir,nat32 labels,real32 * depth,const bs::Normal & toLight,
            real32 cost,nat32 stride,const svt::Field<real32> & irr,nat32 irrRes);
            
  
  /// Optimisation method, this extracts the lookup table, returning a buffer you
  /// are then responsible for delete[] -ing at a latter time. This can then be used 
  /// to set the lookup table of another brute that has the same initialisation 
  /// parameters. Set must be called before use.
   byte * GetLT() const;
   
  /// The partner to GetLT(), this sets a lookup table, only applicable for lookup 
  /// tables extracted from brutes with identical parameters. 
  /// (irr does not count here) Set must be called before use.
   void SetLT(byte * lt);


  /// &nbsp;
   nat32 Links() const;

  /// &nbsp;
   void LinkType(nat32 ind,inf::MessageClass & out) const;


  /// &nbsp;
   void ToSP();

  /// &nbsp;
   void SendOneSP(nat32 instance,const inf::MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneSP(nat32 instance,const inf::MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllSP(nat32 instance,const inf::MessageSet & ms);


  /// &nbsp;
   void ToMS();

  /// &nbsp;
   void SendOneMS(nat32 instance,const inf::MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneMS(nat32 instance,const inf::MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllMS(nat32 instance,const inf::MessageSet & ms);


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  real32 * depth;
  bs::Normal toLight; 
  nat32 labels;
  real32 cost;
  nat32 stride;
  svt::Field<real32> irr;
  nat32 irrRes;
  bit dir;
  
  // Lookup table, extremely hairy.
  // lt points to irrRes*2-1 seperate lookup tables, one for each irradiance 
  // combination across the diagonal. That is, the first half covers the top 
  // value being higher, the second half covers the top value being lowerer 
  // than the bottom value. 0 is not included in the array for obvious reasons 
  // and the minus one is for the overlap where both values are the same.
  //
  // Each seperate lookup table is then labels to the power of 4 (!) bits, 
  // rounded up to the nearest byte as necesary. These match every combination 
  // of disparities, with the bit set if this combination matches the albedo 
  // constraint, false if it does not.
   byte ** lt;
   
  // Helper methods, for creating the lookup table...
  // (All addative, so you can add effects together.)
  // (Multiply the opposite angle by the irrRatio to get the outC angle.)
   void CalcLT(real32 irrRatio,nat32 outC,byte * out);
};

//------------------------------------------------------------------------------
/// This is a factor, applys a sfs based smoothing to a 2x2 area of disparities.
/// This is a MS only implimentation, can not work with SP.
/// It also only suports Loopy, as is obvious by its loopy design!
class EOS_CLASS DisparitySmoothSFS : public inf::Function
{
 public:
  /// &nbsp;
   DisparitySmoothSFS();

  /// &nbsp;
   ~DisparitySmoothSFS();


  /// Sets various costs used, must be called before use.
  /// \param labels Number of labels used, i.e. disparities avaliable.
  /// \param depth Depth that each disparity label matches to, labels size.
  /// \param toLight Normal to infinitly distant point light source.
  /// \param cost Sets the cost of not fitting the SfS constraint, as this is
  ///             essentially a Potts term. The cost of matching the SfS
  ///             constraint is the typical zero.
  /// \param angRes Sets the resolution of the cos angle part of the lookup 
  ///               tables used. Should be set fairrly high, to reduce aliasing
  ///               and because this is relativly cheap to set high from a 
  ///               processing point of view. Does increase memory usage however,
  ///               making cache misses more likelly.
  /// \param stride Ushally the width of the image-1, to account for the 2x2 
  ///               size of this factor.
  /// \param irr Irradiance field, treated as a 2D array indexed by instance 
  ///            using the stride measurement to divide the instance to get a 2D
  ///            coordinate for the top left of the 2x2 square.
  ///            The data structure behind this must be preserved until this has 
  ///            finished.
   void Set(nat32 labels,real32 * depth,const bs::Normal & toLight,
            real32 cost,nat32 angRes,nat32 stride,
            const svt::Field<real32> & irr);


  /// &nbsp;
   nat32 Links() const;

  /// &nbsp;
   void LinkType(nat32 ind,inf::MessageClass & out) const;


  /// &nbsp;
   void ToSP();

  /// &nbsp;
   void SendOneSP(nat32 instance,const inf::MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneSP(nat32 instance,const inf::MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllSP(nat32 instance,const inf::MessageSet & ms);


  /// &nbsp;
   void ToMS();

  /// &nbsp;
   void SendOneMS(nat32 instance,const inf::MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneMS(nat32 instance,const inf::MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllMS(nat32 instance,const inf::MessageSet & ms);


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  real32 * depth;
  bs::Normal toLight; 
  nat32 angRes;
  nat32 labels;
  real32 cost;
  nat32 stride;
  svt::Field<real32> irr;

  // Lookup tables used to accelerate behaviour...
   // This lookup table is used to convert disparities into angles from toLight,
   // i.e. the first part of the cone constraint.
   // 4 versions are provided, for each of the corners that can be involved.
   // [0]: Indexed by disparity(c+dy),disparity(c+dx),disparity(c).
   // [1]: Indexed by disparity(c+dy),disparity(c-dx),disparity(c).
   // [2]: Indexed by disparity(c-dy),disparity(c+dx),disparity(c).
   // [3]: Indexed by disparity(c-dy),disparity(c-dx),disparity(c).
    real32 * dispToAng[4];
   // This converts two disparities and an angle into a third disparity, the 
   // centre disparity. A partial inverse of the above essentially.
   // A value of nat8(-1) is used to indicate the impossible, which should then
   // be ignored. (A side affect of nat8 is a limit of 256 disparity levels. 
   // That would take forever to run though, so no worries.)
   // [0]: Indexed by disparity(c-dy),disparity(c-dx),angle.
   // [1]: Indexed by disparity(c-dy),disparity(c+dx),angle.
   // [2]: Indexed by disparity(c+dy),disparity(c-dx),angle.
   // [3]: Indexed by disparity(c+dy),disparity(c+dx),angle.
    nat8 * angToDisp[4];
};

//------------------------------------------------------------------------------
/// This is a factor pattern designed to smooth a disparity field by directly
/// applying the concept of peice-wise continuous albedo, as calculated from
/// the cone constraint. This is applied as a Potts term, requiring the
/// disparity to be such that a sfs interpretation results in constant albedo.
class EOS_CLASS DisparityAlbedoCost : public inf::FactorPattern
{
 public:
  /// &nbsp;
   DisparityAlbedoCost();

  /// &nbsp;
   ~DisparityAlbedoCost();

  
  /// Sets all of the parameters required, must be called.
  /// Provides the cost of not matching and the direction to the light source.
  /// Provides the number of discreet steps to divide cos angles into and the 
  /// irradiance map for the image. Sets the pattern size effectivly.
  /// Requires a position independent disparity to depth convertor, this method generates
  /// depths for each label on the spot, so the convertor can be deleted straight after 
  /// this method returns.
   void Set(real32 cost,const bs::Normal & toLight,
            nat32 angRes,const svt::Field<real32> & irr,
            const cam::DispConv & dc,int32 minDisp,int32 maxDisp);

   /// Switches on brute force mode.
    void EnableBrute(nat32 irrRes);


  /// &nbsp; 
   nat32 PipeCount() const;

  /// &nbsp;
   bit PipeIsSet(nat32 ind) const;

  /// &nbsp;
   const inf::VariablePattern & PipeGet(nat32 ind) const;

  /// &nbsp;
   bit PipeSet(nat32 ind,const inf::VariablePattern & vp);


  /// This is one hell of a method.
   void Construct(nat32 level,inf::FactorConstruct & fc) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  real32 cost;
  bs::Normal toLight;
  
  nat32 angRes;
  svt::Field<real32> irr;
  
  nat32 labels;
  real32 * dispToDepth;
  
  bit doBrute;
  nat32 irrRes;

  inf::Grid2D * vp;


  struct Node
  {
   svt::Var * var;
   svt::Field<real32> field;
  };
  mutable Node * sIrr;
  
  // Tempory storage of lookup table, to save on re-calculation,
  // for the brute force method...
   mutable byte * lt;
};

//------------------------------------------------------------------------------
/// This calculates disparity and normal maps for an image pair, given 
/// additionally the light source direction and albedo as well as irradiance.
/// Uses a factor graph approach where each of the three fields is encoded as 
/// random variables with various functions expressing 'constraints' between the
/// variables.
class EOS_CLASS SfgsDisparity1
{
 public:
  /// &nbsp;
   SfgsDisparity1();

  /// &nbsp;
   ~SfgsDisparity1();


  /// Sets the range of disparities to consider. Defaults to -30 to 30.
   void SetDispRange(int32 minDisp,int32 maxDisp);

  /// Sets the parameters that control the matching cost.
   void SetMatchParas(real32 alphaMult,real32 alphaMax,
                      real32 betaMult,real32 betaMax,
                      real32 gammaMult,real32 gammaMax,
                      real32 deltaMult,real32 deltaMax,
                      real32 oobd,real32 max);

  /// Sets the differential parameters.
   void SetDiffParas(real32 errLevel);
  
  /// Sets the smoothing parameters.
   void SetSmoothParas(real32 smoothMult,real32 smoothMax);

  /// Switches on the smoothing override, which attaches the smoothing to the
  /// disparity directly rather than to the disparity.
  /// Defaults to false, i.e. off.
   void SmoothOverride(bit override);
   
  /// Switches on constant-albedo based smoothing, also sets its parameters. 
  /// Painful. Defaults to off.
   void SmoothAlbedo(bit smoothAlbedo,real32 cost,nat32 angRes,bit brute,nat32 irrRes);
   
  /// Switches on surface orientation calculation and sets its parameters.
  /// Defaults to off.
   void SmoothOrient(bit smoothOrient,nat32 subDivs,real32 maxCost,real32 angMult,real32 eqCost);
   
  /// Enables an enhancement to SmoothOrient where a SFS based constant albedo term is used.
   void SmoothOrientSFS(bit smoothSFS,real32 angAlias,real32 maxCost,nat32 angRes);

  /// Sets disparity convertors, for if using the peicewise constant albedo
  /// smoothing method.
   void SetDispConv(const cam::DispConv & left,const cam::DispConv & right);


  /// This sets the left and right images to use as inputs.
   void SetIr(const svt::Field<bs::ColourLuv> & left,const svt::Field<bs::ColourLuv> & right);
  
  /// This sets the left and right albedo maps to use.
  /// Note that they must not contain any zeros or negative numbers, as it will 
  /// all go to hell if they do.
   void SetAlbedo(const svt::Field<real32> & left,const svt::Field<real32> & right);

  /// This sets the light source direction to use, for both
  /// the left and right image incase there is rotation between them.
   void SetLight(const bs::Normal & left,const bs::Normal & right);


  /// This sets the field to output the disparity to, for both the left and right images.
   void SetDisp(svt::Field<real32> & left,svt::Field<real32> & right);
   
  /// Use this to optionally get a confidence measure for each disparity, simply the 
  /// probability difference between the highest probability value and the second 
  /// highest. A higher value indicates a higher quality fit.
   void SetConfidence(svt::Field<real32> & left,svt::Field<real32> & right);
   
  /// This can optionally be called to set fields to output the left and right differential
  /// of disparities to. Must be the same sizes as the other fields.
  /// This is the differential with regards to x.
   void SetDispDx(svt::Field<real32> & left,svt::Field<real32> & right);

  /// This can optionally be called to set fields to output the left and right differential
  /// of disparities to. Must be the same sizes as the other fields.
  /// This is the differential with regards to y.
   void SetDispDy(svt::Field<real32> & left,svt::Field<real32> & right);
   
  /// Sets the needle map for output, only then used as output when running in orient mode.
   void SetNeedle(const svt::Field<bs::Normal> & left,const svt::Field<bs::Normal> & right);


  /// This runs the algorithm, after this has returned the output fields will have
  /// been set it the most probable solution it has found.
   void Run(time::Progress * prog = null<time::Progress*>());
  

  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::stereo::SfgsDisparity1";}


 protected:
  int32 minDisp,maxDisp; // Indicates the number of steps in the disparity, the delta can be infered from this.
  
  real32 alphaMult,alphaMax;
  real32 betaMult,betaMax;
  real32 gammaMult,gammaMax;
  real32 deltaMult,deltaMax;
  real32 oobd; // Used for the sdsi with out of bound disparities.
  real32 max; // Maximum applied to all maximums.
  
  real32 diffErrLevel;
    
  real32 smoothMult,smoothMax;
  bit smoothOverride;

  bit albedoOverride;
  real32 saCost;
  nat32 saAngRes;
  bit bruteAlbedo;
  nat32 bruteIrrRes;
  
  bit orientOverride;
  nat32 orientsubDivs;
  real32 orientMaxCost;
  real32 orientAngMult;
  real32 orientEqCost;
  
  bit orientOverrideSFS;
  real32 orientSFSangAlias;
  real32 orientSFSmaxCost;
  nat32 orientSFSangRes;

  const cam::DispConv * dc[2];

  svt::Field<bs::ColourLuv> ir[2];
  svt::Field<real32> albedo[2];
  bs::Normal light[2];

  svt::Field<real32> disp[2];
  svt::Field<real32> confidence[2];
  svt::Field<real32> dispDx[2];
  svt::Field<real32> dispDy[2];
  svt::Field<bs::Normal> needle[2];

 // Actual Run method, only does one image so run itself calls this twice.
  void RunSingle(nat32 which,time::Progress * prog);
};

//------------------------------------------------------------------------------
/// This calculates the albedo map of an image given its irradiance map and its
/// needle map. Uses a markov random field encoding under a white light 
/// assumption and the assumption the occluding boundaries are rare.
class EOS_CLASS SfgsAlbedo1
{
 public:
  /// &nbsp;
   SfgsAlbedo1();

  /// &nbsp;
   ~SfgsAlbedo1();
   
  
  /// Sets how many levels to divide albedo into, 100 is the default.
   void SetLabelCount(nat32 labels);
  
  /// Sets the algorithm parameters for the albedo equality component.
  /// This uses a laplacian equality constraint with a corruption component.
  /// The corruption component for albedos higher than the estimated albedo
  /// is set here, as albedo is likelly to be overestimated in areas of 
  /// specularity the corruption component is increased for values less 
  /// than albedo as surface orientation gets near to the toLight vector.
  /// This is modeled as a squared falloff, with a corruption of 1 when
  /// the surface orientation and toLight vector are equal; which drops to 
  /// the minimum corruption at a given difference angle.
   void SetAlbedoParas(real32 sd,real32 minCorruption,real32 corruptionAngle);
  
  /// Sets the algorithm parameters for the neighbour equality component.
  /// A potts term is used where the probability ratio of equal-labels with non-equal labels is
  /// max(maxProb - alpha*{difference in surface orientation} - beta*{eucledian colour difference},minProb).
   void SetNeighbourParas(real32 minProb,real32 maxProb,real32 alpha,real32 beta);
 
  /// Sets the light source direction, must be normalised.
   void SetLight(const bs::Normal & toLight);


  /// Sets the irradiance map.
   void SetIr(const svt::Field<bs::ColourLuv> & ir);
  
  /// Sets the needle map, must be normalised.
   void SetNeedle(const svt::Field<bs::Normal> & needle);
  
  /// Sets the albedo map, this is the output.
   void SetAlbedo(svt::Field<real32> & albedo);


  /// Runs the algorithm. Sets the albedo map from all the other information.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::stereo::SfgsAlbedo1";}


 protected:
  nat32 labels;
 
  real32 albedoSd;
  real32 albedoMinCorruption;
  real32 albedoCorruptionAngle;
  
  real32 neighbourMinProb;
  real32 neighbourMaxProb;
  real32 neighbourAlpha;
  real32 neighbourBeta;

  bs::Normal toLight;


  svt::Field<bs::ColourLuv> ir;
  svt::Field<bs::Normal> needle;

  svt::Field<real32> albedo;
};

//------------------------------------------------------------------------------
/// This uses the SfgsDisparity1 and SfgsAlbedo1 classes to impliment a stereo 
/// algorithm, by iterating over the two steps. (Assumes albedo to be equal to 
/// irradiance for the first pass with SfgsDisparity1.)
class EOS_CLASS Sfgs1
{
 public:
  /// &nbsp;
   Sfgs1();
   
  /// &nbsp;
   ~Sfgs1();


  /// &nbsp;
   void SetDispRange(int32 minDisp,int32 maxDisp);
   
  /// &nbsp;
   void SetMatchParas(real32 alphaMult,real32 alphaMax,real32 betaMult,real32 betaMax,real32 gammaMult,real32 gammaMax,real32 deltaMult,real32 deltaMax,real32 oobd,real32 max);
   
  /// &nbsp;
   void SetDiffParas(real32 errLevel);
   
  /// &nbsp;
   void SetSmoothParas(real32 smoothMult,real32 smoothMax);
   
  /// &nbsp;
   void EnableSmoothOverride();
   
  /// &nbsp;
   void EnableAlbedoSmoothOverride(real32 cost,nat32 angRes,bit brute,nat32 irrRes);
   
  /// &nbsp;
   void EnableSmoothOrient(nat32 subDivs,real32 maxCost,real32 angMult,real32 eqCost);
   
  /// &nbsp;
   void EnableSmoothOrientEx(real32 angAlias,real32 maxCost,nat32 angRes);


  /// &nbsp;
   void SetAlbedoLabelCount(nat32 labels);
   
  /// &nbsp;   
   void SetAlbedoParas(real32 sd,real32 minCorruption,real32 corruptionAngle);
   
  /// &nbsp;
   void SetNeighbourParas(real32 minProb,real32 maxProb,real32 alpha,real32 beta);
   
  /// This is additional to this class, a smoothing applied to the needle maps before
  /// use, set to 0 to disable, which is the default...
   void SetNeedleSmooth(real32 sd);

  
  /// &nbsp;
   void SetIr(const svt::Field<bs::ColourLuv> & left,const svt::Field<bs::ColourLuv> & right);
   
  /// &nbsp;
   void SetLight(const bs::Normal & left,const bs::Normal & right);


  /// &nbsp;
   void SetAlbedo(svt::Field<real32> & left,svt::Field<real32> & right);
   
  /// &nbsp;
   void SetDisp(svt::Field<real32> & left,svt::Field<real32> & right);
   
  /// &nbsp;
   void SetNeedle(svt::Field<bs::Normal> & left,svt::Field<bs::Normal> & right);


  /// &nbsp;
   void SetConfidence(svt::Field<real32> & left,svt::Field<real32> & right);
   
  /// &nbsp;
   void SetDispDx(svt::Field<real32> & left,svt::Field<real32> & right);
   
  /// &nbsp;
   void SetDispDy(svt::Field<real32> & left,svt::Field<real32> & right);


  /// Defaults to 3 iterations. Note that disparity calculation happens iCount+1
  /// times, as its the first and last called.
   void SetIterCount(nat32 iCount);

  /// Provides pointers to objects that will convert a depth map into a 
  /// needle map, this conversion is abstracted out for flexability.
  /// Two are of course required, one for each image.
  /// Once called the given pointers will be owned by this.
   void SetNeedleCreator(cam::DispConv * dcLeft,cam::DispConv * dcRight);


  /// Runs the algorithm.
   void Run(time::Progress * prog = null<time::Progress*>());


  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::stereo::Sfgs1";}


 private:  
  nat32 iCount;
  cam::DispConv * dcLeft;
  cam::DispConv * dcRight;

  bs::Normal leftLight;
  svt::Field<bs::ColourLuv> leftIr;
  svt::Field<bs::Normal> leftNeedle;
  svt::Field<real32> leftDisp;
  svt::Field<real32> leftAlbedo;

  bs::Normal rightLight;
  svt::Field<bs::ColourLuv> rightIr;
  svt::Field<bs::Normal> rightNeedle;
  svt::Field<real32> rightDisp;
  svt::Field<real32> rightAlbedo;
  
  real32 needleSmooth;
  

  SfgsDisparity1 disp;
  SfgsAlbedo1 alb;
};

//------------------------------------------------------------------------------
 };
};
#endif
