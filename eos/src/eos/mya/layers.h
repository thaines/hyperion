#ifndef EOS_MYA_LAYERS_H
#define EOS_MYA_LAYERS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file layers.h
/// Provides a representation of an image depth map in terms of a set of 
/// surfaces. Provides various operations on the fitting, including 
/// initialisation.

#include "eos/types.h"
#include "eos/ds/arrays.h"
#include "eos/time/progress.h"
#include "eos/data/randoms.h"
#include "eos/math/functions.h"
#include "eos/svt/field.h"
#include "eos/mya/surfaces.h"
#include "eos/mya/ied.h"
#include "eos/ds/sort_lists.h"

namespace eos
{
 namespace mya
 {
//------------------------------------------------------------------------------
/// This class is given a segmentation of an image and several Ied's calculated
/// from the image. In addition a set of SurfaceType-s are registered with it.
/// It represents a set of sets of segments, called layers, where each layer has an 
/// assigned Surface to represent its depth for the entire area of its combined
/// segments. Layers are indexed by any segment index which is a member of that
/// layer.
/// It also provides both the ability to query and change this data as well as
/// various 'general' algorithms for changing this data, including initialisation.
class EOS_CLASS Layers
{
 public:
  /// &nbsp;
   Layers();
 
  /// &nbsp; 
   ~Layers();


  // Basic setup...
   /// Sets the segmentation to be used. Only call this once.
    void SetSegs(nat32 segCount,const svt::Field<nat32> & segs);

   /// The possible fitting algorithms.
   /// - Prune simply fits with all the given data then prunes outliers. Repeat till convergence/iterout.
   /// - Ransac is the standard algorithm as described in countless places. The best surface found has the Prune method applied to refine the result.
    enum FitAlg {Prune,Ransac};

   /// Sets the fitting algorithm used by the fitting operations, defaults
   /// to Prune. Can be called post-Commit. You also supply a random number
   /// generator, which is used for randsac, if yuor setting the mode to Prune
   /// this can be null.
    void SetFitMethod(FitAlg fa,data::Random * random);


  // Data setup...
   /// This adds an Ied to the object, at least one is required to have been added before
   /// much else can be done. It returns the index by which you must reference this Ied
   /// in the future.
   /// Note that the object passed in must still be deleted by the user, after
   /// having finished using this object, to avoid any crashes.
    nat32 AddIed(Ied * ied);

   /// This sets the 'outlier distance' used by the fitting algorithms, for a given Data
   /// item. Can be called post-Commit. Defaults to 1 for each new data type.
    void SetOutlierDist(nat32 di,real32 cutoff);


  // Surface setup...
   /// This adds a surface type to the set of types that will be considered for fitting,
   /// it returns an index by which you should refer to that type in the future.
   /// Note that the object passed in must still be deleted by the user, after
   /// having finished using this object, to avoid any crashes.
    nat32 AddSurfaceType(SurfaceType * st);
    
   /// Sets the weight of the surface type, the outlier-inlier ratio is multiplied by 
   /// this when deciding which surface to use. A higher number results in a lower
   /// 'chance' of the surface being used, i.e. a surface with weight 2 requires half
   /// the outliers of a surface of weight 1 to be concidered the better choice.
   /// Can be called post-Commit. Defaults to 1 for each new surface type.
    void SetSurfaceWeight(nat32 si,real32 weight);


  // Initialisation...
   /// After calling all the setup methods and before calling any getter/setters/operations
   /// call this, as it preps all the classes data structures and does an initial fitting.
   /// (The initial fitting has each segment in a layer of its own.)
    void Commit(time::Progress * prog = null<time::Progress*>());


  // Getters and setters...
   /// Returns how many segments exist, simply an accessor for the passed in count.
    nat32 SegmentCount() const;
    
   /// Returns how many layers exist.
    nat32 LayerCount() const;

   /// This returns a segment from the layer that will be consistant whichever 
   /// segment-member you pass in. Note that this can change after layer editting 
   /// operations. You can use this for testing if two segments belong to the same
   /// layer.
    nat32 SegToLayer(nat32 seg) const;

   /// Outputs an array of segment indexes, where each segment is in a different layer,
   /// covering all layers. Will resize the array to Size()==LayerCount(). These equate
   /// to the layer numbers output by SegToLayer.
    void GetLayers(ds::Array<nat32> & out) const;
   
   /// As a helper to the BestFitLayer method this sets the flags of the given array
   /// to the current setup of the given segments layer. The given array must allready
   /// be SegmentCount() size.
   /// \param layer Which layer to take the flags from.
   /// \param foi The flag array to write the output to. This must be the correct size on pass in.
   /// \param setFalse If true then it sets the flags to false when neccesary, if false it only 
   /// sets the relevent flags to true, does not set false. Allows you to '|' sets of flags easilly.
    void GetLayerFlags(nat32 layer,ds::Array<bit> & foi,bit setFalse = true) const;

   /// Returns a pointer to the Surface of a particular layer as indicated by a
   /// segment in that layer. Can be null if no surface could be reasonably fit.
    Surface * LayerToSurf(nat32 layer) const;
    
   /// Returns how many pixels make up a given segment.
    nat32 SegmentSize(nat32 seg) const;
    
   /// Returns how many pixels make up a layer.
    nat32 LayerSize(nat32 layer) const;
    
   /// Returns the fitting cost of the last fitting to the given layer.
   /// (Simply the number of outliers multiplied by the surface weight.)
    real32 FitCost(nat32 layer) const;
    
   /// Sets the layer associated with a segment. You must call Refit postcall
   /// to refit the layers. Essentially removes seg from its current layer and 
   /// puts it into the layer represented by the segment layer.
    void SetSeglayer(nat32 seg,nat32 layer);    

   /// Clusters two layers into one, note that after calling this you must
   /// call the Refit method to refit.
    void MergeLayers(nat32 lay1,nat32 lay2);
    
   /// Given a segment this seperates it from any current layer, making it 
   /// into its own layer. Postcall you must call Refit to get the surfaces
   /// back.
    void Seperate(nat32 seg);


  // Operations...
   /// This updates the structure, after calling this surface pointers
   /// will no longer be valid. All layers that have changed get refit with a new 
   /// surface. It only recalculates where required, so its as efficient as can be, but
   /// its still going to be time consuming after multiple edits.
    void Refit(time::Progress * prog = null<time::Progress*>());

   /// This returns the best fit surface for a single segment, do not delete the result, it
   /// is owned by this object. Can return null if no surface could be reasonably fit. 
   /// (Insufficient data etc.)
   /// Outputs a fitting cost. It internally caches calculated results, so calling it twice
   /// with the same parameters the second time should be a lot faster.
    Surface * SegFit(nat32 seg,real32 & cost) const;
    
   // Identical to SegFit except it returns the cost only, no surface, for testing
   // hypothetical configurations. Uses the caching system, so the surface is actually
   // generated, so further calls to SegFit will be optimised.
    real32 SegFitCost(nat32 seg) const;

   /// This returns the best fit surface for a set of segments, do not delete the
   /// result, its owned by this object. The given array must be Size()==SegmentCount().
   /// Can return null if no surface could be reasonably fit. (Insufficient data etc.)
   /// Caches calculated results, so duplicate calculation is not a risk.
   /// \param foi An array of flags, one for each segment, true indicates inclusion to the layer, false exclusion.
   /// \param cost Output variable, the cost of the fitting, an indication of how good it is.   
    Surface * LayerFit(const ds::Array<bit> & foi,real32 & cost) const;

   // Identical to LayerFit except it returns the cost only, no surface, for testing
   // hypothetical configurations. Uses the caching system, so the surface is actually
   // generated, so further calls to LayerFit will be optimised.
    real32 LayerFitCost(const ds::Array<bit> & foi) const;
    

  // Result extractors...
   /// This extracts the depth for a particular point in the image, interpolates between layers
   /// where neccesary. Will return limit when things go wrong.
   /// \param limit The maximum depth to return, any value greater than this will be set to this, especially infinity.
   /// \param x The x coordinate to sample at.
   /// \param y The y coordinate to sample at.
   /// \returns A depth value, [0..limit]
    real32 GetDepth(real32 limit,real32 x,real32 y);
   
   /// This extracts the disparity for a particular point in the image, interpolates between
   /// layers where neccesary. Will return 0 when things go wrong.
   /// \param mult The focal length, assumed identical for both cameras, multiplied by the distance between camera centers.
   /// \param x The x coordinate to sample at.
   /// \param y The y coordinate to sample at.
   /// \returns A disparity, you can negate mult to reverse the sign of them.
    real32 GetDisp(real32 mult,real32 x,real32 y);


   /// This extracts a depth map for the entire image, by simply sampling the relevent surfaces.
   /// Any depth greater than the given limit or in error is set to the limit.
   /// \param limit Maximum depth returned
   /// \param depth The output.
    void GetDepthMap(real32 limit,svt::Field<real32> & depth) const;
 
   /// This extracts a disparity map for the entire image, converts from depth, this assumes
   /// no distortion in the image. Erronous samples are converted to a disparity of 0.
   /// \param mult mult is the focal length (Assumed equal for both cameras) multiplied by the distance between the two camera centers. Simply the muliplier in the 1 over transformation this actually maps to.
   /// \param disp The output.
    void GetDispMap(real32 mult,svt::Field<real32> & disp) const;
    
   /// This extracts a layer map, simply a segmentation map but for the layers created
   /// rather than the segments from which there constructed.
    void GetLayerMap(svt::Field<nat32> & layers) const;
    
   /// This extracts a segmentation by surface type, with segment 0 being no surface, segment 1 
   /// being the the first surface type added and so on.
    void GetSurfaceMap(svt::Field<nat32> & surfaces) const;
    
   // Extracts a validity map, outputs false at locations where no surface is defined and
   /// true where a surface is defined.
    void GetValidity(svt::Field<bit> & valdity) const;


  /// Transilates from image coordinates to the normalised coordinates used
  /// internaly by this class, for if you want to use the Surface objects directly.
   void IntCoord(const math::Vect<2> & in,math::Vect<2> & out);
  
  
  /// &nbsp;
   static inline cstrconst TypeString() {return "eos::mya::Layers";}


 private:
  // Constants...
   static const nat32 ranScale = 8; // It won't do ransac unless it has this many times as much information as required at a minimum to do a basic fitting.
   static const nat32 maxPruneIter = 64; // Maximum number of iterations it will do before giving up on improving the surface as part of the prune and ransac fitting algorithms.
   static const real32 ransacProb = 0.99; // Probability of success for the ransac algorithm.
   static const nat32 maxRansacIter = 10000; // Maximum number of ransac iterations, to keep things moving.
 
  // An internal token table, used for comunicating between plugin surface types and data sources...
   mutable str::TokenTable tt;
 
  // Basics...
   svt::Field<nat32> segs;
   FitAlg fa;
   data::Random * random;

  // Data sources...
   struct NodeSample
   {
    real32 x;
    real32 y;
    math::Vector<real32> vec;
   };
 
   struct NodeIed // Dual use structure, used to store data sources and to cache data during proccessing.
   {
    Ied * ied;
    real32 cutoff;
    
    // A cache of data to optimise the fitting algorithm, essentially the NodeSample's
    // for all valid pixels for each segment.
     typedef ds::Array<NodeSample,mem::MakeNew<NodeSample>,mem::KillOnlyDel<NodeSample> > SampleArray;
     ds::Array<SampleArray,mem::MakeNew<SampleArray>,mem::KillOnlyDel<SampleArray> > surfData;    
     
    // Some data used by the fitting algorithm during runtime, simply for conveniance of passing arround...
     mutable nat32 ind; // Type index, to be used when passing to the surface.
     mutable nat32 sampleCount; // Number of samples.
     mutable nat32 sampleSum; // Sum of samples upto but excluding here, for weighted random selection of NodeIed.
   };
   ds::Array<NodeIed,mem::MakeNew<NodeIed>,mem::KillOnlyDel<NodeIed> > da;
   
  // Surface types...
   struct NodeSurface
   {
    SurfaceType * type;
    real32 weight;
   };
   ds::Array<NodeSurface> sa;


  // An internal data structure used during fitting in collaboration with the
  // da object - an array of numbers where each number indicates a segment thats
  // a member of the layer to which we are fitting.
   mutable ds::Array<nat32> segSet;

  // Assignment of segment numbers to layers, represented by a forest...
   struct Node
   {
    Node * Head()
    {
     if (parent==null<Node*>()) return this;
     parent = parent->Head();
     return parent;
    }

    Node * parent;
    nat32 seg; // Which segment it maps to. Could be worked out by memory position, but thats messy.
    nat32 segSize; // How many pixels are in the segment.

    // All the below only apply when parent==null...
     Surface * surface;
     bit editted; // true if its been editted, i.e. must be Refit().
     nat32 pixels; // How many pixels are in the layer.
     real32 cost; // surface_weight*outlierCount - less is better.
   };
   ds::Array<Node> data; // Entry for each segment, data.Size() is # of segments.
   nat32 layers; // How many layers exist, tracked incrimentally as its direct calculation would be slow.

  // The caching system, to stop duplicate calculation...
   // This represents an item used to store a layer-configuration and its
   // resulting surface and score...
    class CacheNode
    {
     public:
       CacheNode(nat32 sCount)
       :segCount(sCount),bits(mem::Malloc<byte>((sCount+7)>>3)),surface(null<Surface*>())
        {mem::Null(bits,(sCount+7)>>3);}
      
      ~CacheNode() {mem::Free(bits); delete surface;}
      
      bit operator < (const CacheNode & rhs) const 
      {return math::CompareBitSeq(bits,rhs.bits,(segCount+7)>>3);}
     
     
      // Adds a member to the key, which starts with no members on construction...
       void AddMember(nat32 seg) {log::Assert(seg<segCount); bits[seg>>3] |= 1<<(seg&0x07);}
       
      // Adds a whole load of members to the key, or's with current data...
       void AddMembers(const ds::Array<bit> & foi)
       {
        log::Assert(segCount==foi.Size());
        for (nat32 i=0;i<segCount;i++) {if (foi[i]) AddMember(i);}
       }
      
      // Sets the score...
       void SetScore(real32 sco) {score = sco;}
       
      // Sets the surface...
       void SetSurface(Surface * surf) {delete surface; surface = surf;}      
      
      // Gets the score...
       real32 GetScore() const {return score;}
       
      // Gets the surface...
       Surface * GetSurface() const {return surface;}

     private:
      nat32 segCount;
      byte * bits; // segCount/8, rounded up in size, bit for each segment indicating membership. All excess bits are zero'ed.
      
      real32 score;
      Surface * surface;
    };
    
  // Hash table of scores for each layer configuration tried, a sort list is used
  // as its the only structure with the required features, though better structures
  // do exist (A potential future optimisation possibility.)...
   mutable ds::SortList<CacheNode*,ds::SortPtrOp<CacheNode*>,mem::KillDel<CacheNode> > cache;
   

  // Internal stuff...
   // Finds the best fit surface, called when the da structure has been updated with
   // the relevent data arrays and sizes for the area being fitted in question.
   // It outputs its score, as a measure of confidence, where 0 implies a perfect fit,
   // higher numbers imply a worse fit.
    Surface * SegFitInt(real32 & score) const;
   
   // Fits data to a surface, for a given SurfaceType with given model data
   // with weights and SurfaceType-model indexes passed in.
   // out is filled with the number of outliers for the returned surface, so
   // surface comparisons can be made to decide on the best. (As a real so a weighting can
   // be applied without conversion.)
    Surface * SegFitInt(FitAlg lfa,NodeSurface & st,const ds::Array<NodeIed*> & ds,real32 & out) const;
};

//------------------------------------------------------------------------------
 };
};
#endif
