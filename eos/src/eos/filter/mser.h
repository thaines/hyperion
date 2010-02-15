#ifndef EOS_FILTER_MSER_H
#define EOS_FILTER_MSER_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file mser.h
/// Provides an implimentation of 'Robust Wide Baseline Stereo from maximally
/// Stable Extremal Regions' by Matas, Chum, Urban and Pajdla. Additionally
/// includes the entirety of the follow up paper for selecting affine regions 
/// to generate keys from, and will generate the relevent keys making this a
/// general purpose object recognition module.

#include "eos/types.h"
#include "eos/svt/field.h"
#include "eos/ds/arrays.h"
#include "eos/ds/arrays2d.h"
#include "eos/bs/colours.h"
#include "eos/bs/geo2d.h"
#include "eos/bs/dom.h"
#include "eos/math/matrices.h"
#include "eos/math/mat_ops.h"
#include "eos/time/progress.h"

namespace eos
{
 namespace filter
 {
//------------------------------------------------------------------------------
/// Provides an implimentation of the MSER techneque, given a greyscale field 
/// this calculates all the MSER's, in addition to the center, threshold and 
/// whether its minimal/maximal it also provides the covariance matrix and an 
/// affine frame generated using the maximaly distant contour point as well as
/// the covariance matrix and centre of gravity.
/// This is based on 'Robust Wide Baseline Stereo from Maximally Stable Extremal Regions'
/// by Matas, Chum, Urban and Pajdla.
class EOS_CLASS Mser
{
 public:
  /// &nbsp;
   Mser();
   
  /// &nbsp;
   ~Mser();
   
  
  /// Sets delta, the algorithms one parameter, and an extra 2 parameters,
  /// minimum mser size and maximum mser size as a multiple of pixels in the image.
  /// minArea and delta default to 32, maxAreaMult defaults to 0.1, i.e. 10%.
   void Set(nat32 delta,nat32 minArea = 32,real32 maxAreaMult = 0.1);
   
  /// Sets the image to proccess.
   void Set(const svt::Field<bs::ColourL> & img);

   
  /// &nbsp;
   void Run(time::Progress * prog = null<time::Progress*>());
   
   
  /// Number of MSER's found.
   nat32 Size() const;
  
  /// Returns true if it a maximal, false if its a minimal.
   bit Maximal(nat32 i) const;

  /// A pixel that is a member of the MSER, doing a fill operation with the
  /// threshold and maximal/minimal status from this pixel gives you the
  /// MSER membership.
   const bs::Pos & Pixel(nat32 i) const;

  /// Threshold of a MSER.
   real32 Threshold(nat32 i) const;

  /// Area of a MSER, i.e. number of pixels involved.
   nat32 Area(nat32 i) const;
   
  /// Centre of gravity of a MSER.
   const bs::Pnt & Centre(nat32 i) const;

  /// Returns the covariance matrix.
   const math::Mat<2> & Covariance(nat32 i) const;
   
  /// Returns the matrix part of the affine frame, the centre of 
  /// gravity can be considered as the centre. The +ve x-axis of the 
  /// frame is where the contour point furthest from the centre of 
  /// gravity exists.
   const math::Mat<2> & Affine(nat32 i) const;
   
   
  /// Given a RGB image the same dimensions as the input, ushally just the 
  /// pre-greyscale version, this uses the contained MSERs to render outlines
  /// for all of 'em, a visualisation method. i.e. for each mser it sets to 
  /// RGB(1,1,1) every pixel adjacent to its filled area.
   void VisualiseRegions(svt::Field<bs::ColourRGB> & out) const;
   
  /// Given a RGB image the same dimensions as the input, ushally just the 
  /// pre-greyscale version, this uses the contained MSERs to render squares
  /// with lines from the centre in the direction of the frame for all the 
  /// affine frames generated.
   void VisualiseFrames(svt::Field<bs::ColourRGB> & out) const;
   
  /// This returns an xml element, called "features" which contains all the
  /// msers in order - details all of the various numbers given by this interface.
  /// Remember to call delete on the returned.
   bs::Element * AsXML(str::TokenTable & tt) const;


  /// &nbsp;
   static cstrconst TypeString() {return "eos::filter::Mser";}


 private:
  nat32 minArea;
  nat32 maxArea;
  real32 maxAreaMult;

  nat32 delta;
  svt::Field<bs::ColourL> img;
  
  struct Node
  {
   bit maximal;
   bs::Pos pixel;
   real32 threshold;
   nat32 area;
   bs::Pnt centre;
   math::Mat<2> covariance;
   math::Mat<2> affine;
  };
  ds::Array<Node> data;

  
  // The forest data structure used for merging...
   struct Turnip
   {
    nat32 x;
    nat32 y;
    real32 l;
    nat32 bucket;

    Turnip * next; // Linked list of nodes at any particular level, so they can be iterated in the right order.

    Turnip * parent; // Points to the parent node. Path shortening is applied.
    Turnip * previous; // These form a linked list of nodes from any given head, through the clustering route - by taking areas along this route you get the relevant graph.
    Turnip * nextOther; // Points to the node that lost at each merge step, so all can be iterated for means/covariances etc.
    Turnip * prevOther; // (Its a doubly linked list, never null)
    
    nat32 area; // Total area if you sum all nodes following previous and other, excluding parents obviously. Set to 0 to indicate that the node has yet to enter the forest.


    Turnip * Head()
    {
     if (parent)
     {
      parent = parent->Head();
      return parent;
     }
     else return this;	    
    }
    
    void Collapse() // Disolves the previous into the nextOther list, nulls the previous essnetially.
    {
     if (previous)
     {
      previous->prevOther->nextOther = nextOther;
      nextOther->prevOther = previous->prevOther;
   
      previous->prevOther = this;
      nextOther = previous;
   
      previous = null<Turnip*>();
      nextOther->Collapse();
     }
    }
    
    // Dissolves the previous into the nextOther list whilst until previous->bucket==bucket.
    void Compress()
    {
     while ((previous)&&(previous->bucket==bucket))
     {
      previous->prevOther->nextOther = nextOther;
      nextOther->prevOther = previous->prevOther;
   
      previous->prevOther = this;
      nextOther = previous;
   
      previous = previous->previous;
      nextOther->previous = null<Turnip*>();
     }
    }
    
    void CalcTolMean(bit dir,real32 & tol,
                     nat32 & n,real32 & baseX,real32 & baseY,real32 & sumX,real32 & sumY) const
    {
     const Turnip * targ = this;
     while (true)
     {
      if (dir) tol = math::Min(tol,targ->l);
         else  tol = math::Max(tol,targ->l);
         
      sumX += real32(targ->x)-baseX;
      sumY += real32(targ->y)-baseY;
      ++n;
     
      if (math::Abs(sumX)+math::Abs(sumY)>1000000.0)
      {
       baseX += sumX/real32(n);	     
       baseY += sumY/real32(n);
       
       n = 0;
       sumX = 0.0;
       sumY = 0.0;
      }
      
      if (targ->nextOther==this) break;
      targ = targ->nextOther;
     }
     
     if (previous) previous->CalcTolMean(dir,tol,n,baseX,baseY,sumX,sumY);
    }
    
    void CalcCovar(const bs::Pnt & centre,math::Mat<2> & covar) const
    {
     const Turnip * targ = this;
     while (true)
     {
      real32 relX = targ->x - centre[0];
      real32 relY = targ->y - centre[1];

      covar[0][0] += math::Sqr(relX);
      covar[0][1] += relX*relY;
      covar[1][1] += math::Sqr(relY);

      if (targ->nextOther==this) break;
      targ = targ->nextOther;
     }
     
     if (previous) previous->CalcCovar(centre,covar);
    }
    
    void CalcExt(const bs::Pnt & centre,const math::Mat<2> & affineInv,real32 & best,bs::Pnt & furthest) const
    {
     const Turnip * targ = this;
     while (true)
     {
      bs::Pnt pos[2];

      pos[0][0] = targ->x - centre[0];
      pos[0][1] = targ->y - centre[1];
      math::MultVect(affineInv,pos[0],pos[1]);

      real32 lenSqr = pos[1].LengthSqr();
      if (lenSqr>best)
      {
       best = lenSqr;
       furthest = pos[1];	     
      }

      if (targ->nextOther==this) break;
      targ = targ->nextOther;
     }
     
     if (previous) previous->CalcExt(centre,affineInv,best,furthest);
    }
   };
   
  // Helper methods/functions...
   void DoList(ds::Array2D<Turnip> & forest,Turnip * targ,ds::List<Node> & store,bit dir);
   void Merge(Turnip & ta,Turnip & tb,ds::List<Node> & store,bit dir);
   void Analyse(Turnip & head,ds::List<Node> & store,bit dir);
   void Store(const Turnip & head,ds::List<Node> & store,bit dir);


  // Data structure used by the Visualise method...
   struct Cabbage
   {
    nat32 x;
    nat32 y;
    nat32 ind;
    Cabbage * next;
   };
};

//------------------------------------------------------------------------------
/// This structure represents a MserKey, that is a 1323 length feature vector.
/// This is a 21x21 pixel rgb image extracted from the affine region of a mser
/// that has been normalised.
/// Because it however represents an image patch this structure is provided
/// so as to also provide the normalisation parameters used and some 
/// conveniance methods for extracting the original image.
class EOS_CLASS MserKey : public math::Vect<1323,real32>
{
 public:
  /// Index vector, for synchronisation. When created by MserKeys this is set to
  /// the index of the key, so you can get further information.
   nat32 index;
 
  /// Mean for the 3 channels, red, green and blue.
   real32 mean[3];
   
  /// Standard deviation for the 3 channels, red, green and blue.
   real32 sd[3];


  /// Returns the offset into the vector for a particular triplet associated
  /// with a coordinate. Both x and y must be [0..20].
   static nat32 Offset(nat32 x,nat32 y) {return 3*(y*21 + x);}
   
  /// Outputs the unnormalised colour of a given pixel into the given bs::ColourRGB.
   void Extract(nat32 x,nat32 y,bs::ColourRGB & out) const
   {
    nat32 offset = Offset(x,y);
    out.r = math::Clamp<real32>(((*this)[offset]*sd[0]) + mean[0],0.0,1.0);
    out.g = math::Clamp<real32>(((*this)[offset]*sd[1]) + mean[1],0.0,1.0);
    out.b = math::Clamp<real32>(((*this)[offset]*sd[2]) + mean[2],0.0,1.0);
   }
};

//------------------------------------------------------------------------------
/// This inherits from Mser, to also generate keys for each mser output which 
/// it extends the interface to allow access to. The keys are simply 625 long
/// vectors with euclidean distance being the dissimilarity measure.
/// This is based on the paper 
/// 'Object Recognition using Local Affine Frames on Distinguished Regions'
/// by Obdrzalek and Matas.
class EOS_CLASS MserKeys : public Mser
{
 public:
  /// &nbsp;
   MserKeys();
   
  /// &nbsp;
   ~MserKeys();


  /// Pass through, must be called.
   void Set(const svt::Field<bs::ColourL> & img)
   {
    Mser::Set(img);	   
   }

  /// Sets the rgb version of the image to proccess.
  /// You must also call this with the bs::ColourL type - whilst they must be
  /// the same image this class does not do the conversion itself.
   void Set(const svt::Field<bs::ColourRGB> & img);

   
  /// &nbsp;
   void Run(time::Progress * prog = null<time::Progress*>());   

   
  /// An extra getter, to get the keys, this works in unision with the getters
  /// provided by te parent class.
   const MserKey & Key(nat32 i) const;
   
   
  /// This returns a new svt::Var that contains all the keys as images, side by
  /// side in the x axis, if saved alongside the .xml file you could re-generate
  /// the mser data.
  /// It will contain one field of bs::ColourRGB, called rgb.
  /// Remember to delete the returnee.
   svt::Var * KeyImage(svt::Core & core) const;


  /// &nbsp;
   static cstrconst TypeString() {return "eos::filter::MserKeys";}


 private:
  svt::Field<bs::ColourRGB> img;
  ds::Array<MserKey> key;
};

//------------------------------------------------------------------------------
 };
};
#endif
