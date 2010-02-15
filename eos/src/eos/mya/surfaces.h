#ifndef EOS_MYA_SURFACES_H
#define EOS_MYA_SURFACES_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \namespace eos::mya
/// Provides 'my algorithms', a playground where I develop my own stuff before 
/// its farmed off to a more appropriate module as a complete system.

/// \file surfaces.h
/// Provides an abstract surface representation for parametric stereo, various
/// surface types then inherit from this to provide the required functionality.
/// The surfaces are designed to be given an x and y co-ordinate and to then return
/// the z component, i.e. depth, as a homogenous vector, so it can be at 
/// infinity if need be.

#include "eos/types.h"
#include "eos/math/vectors.h"
#include "eos/str/tokens.h"

namespace eos
{
 namespace mya
 {
//------------------------------------------------------------------------------
/// The surface representation object, represents a particular instance of a 
/// particular type of surface. Provides simple methods to get the depth and the
/// change in depth.
class EOS_CLASS Surface : public Deletable
{
 public:
  /// &nbsp;
   ~Surface() {}

  /// Returns a copy of this object, you must ultimatly call delete on the returned pointer.
   virtual Surface * Clone() const = 0;
   
  /// This returns true if this surface and another surface, which must allways be
  /// the same child type (i.e. child implimentations of this method will 
  /// dynamic_cast to there child type.) are close enough to be concidered to have
  /// no change, accounting for numerical error. Used to indicate when an iterative
  /// procedure refining a surface can stop, i.e. not enough changed between
  /// iterations to bother with more iterations.
   virtual bit operator == (const Surface & rhs) const = 0;

  /// This returns the depth at a given coordinate using this surface.
  /// \param x The x coordinate to sample at.
  /// \param y The y coordinate to sample at.
  /// \param z The output z coordinate, as a homogenous vector. Note that it can be at infinity, i.e. (x,0),x>0; and that it can be (0,0), an invalid value, to indicate an error, i.e. the surface can not be calculated at the given (x,y).
   virtual void Get(real32 x,real32 y,math::Vect<2> & z) const = 0;

  /// This returns the partial differentials of depth at a given coordinate using this surface.
  /// Like the basic Get this outputs in homogenous coordinates to handle infinity and errors.
  /// \param x The x coordinate to sample at.
  /// \param y The y coordinate to sample at.
  /// \param dx The change in x.
  /// \param dy The change in y.
   virtual void GetDelta(real32 x,real32 y,math::Vect<2> & dx,math::Vect<2> & dy) const = 0;

  /// &nbsp;
   virtual cstrconst TypeString() const = 0;
};

//------------------------------------------------------------------------------
/// The surface fitter object, you feed one of these with various data points 
/// till it has enough data, at which point it will return to you a 
/// surface representation or fail due to degenerate data. Will support 
/// overfitting with a surface type specific minimisation strategy.
class EOS_CLASS Fitter : public Deletable
{
 public:
  /// &nbsp;
   ~Fitter() {}


  /// This allows you to empty all contained data and start again, to fit 
  /// to another data set.
   virtual void Reset() = 0;

  /// You use this to add some item of data you have to the fitter object.
  /// \param type The index of the data type you are passing in, the relevent indexes can be obtained from the SurfaceType object.
  /// \param x The x coordinate in the image where the sample has been taken.
  /// \param y The y coordinate in the image where the sample has been taken.
  /// \param data The data associated with the sample as a Vector, how this is proccessed is wholly dependent on type. The vector is allowed to be larger than the number of items required for the type, in such circumstances only use the number of items required, as the rest will be arbitary.
  /// \returns false if it needs more data, true if its got enough data and can probably produce a surface. Note the use of the word probably.
   virtual bit Add(nat32 type,real32 x,real32 y,const math::Vector<real32> & data) = 0;

  /// This extracts a surface from the object if it can, though it can fail. Should
  /// only be called after Add has returned true at least once. Note that this is called
  /// when just the minimal amount of data has been provided in the inner loop of ransac,
  /// so producing a less good solution quickly when Add has returned true just once and a 
  /// better but slower to calculate solution when Add has returned true multiple times
  /// is a prefered behaviour.
  /// \param out If it returns true then this will contain the best fit surface, otherwise it will contain whatever you passed in.
  /// \returns true on success, false on failure, failure implies degenerate data.
   virtual bit Extract(Surface & out) const = 0;


  /// &nbsp;
   virtual cstrconst TypeString() const = 0;
};

//------------------------------------------------------------------------------
/// The surface type object, simply provides instances of the objects related to
/// a particular type, being a simple object factory. It also provides 
/// various nuggets of information about the surface type so you can use an
/// instance effectivly.
class EOS_CLASS SurfaceType : public Deletable
{
 public:
  /// &nbsp;
   ~SurfaceType() {}


  /// This returns a new uninitialised Surface representation. You must delete
  /// the object when you are done with it.
   virtual Surface * NewSurface() const = 0;

  /// This returns a fresh new fitter, ready to help with fitting. You must 
  /// eventually delete the returned object.
   virtual Fitter * NewFitter() const = 0;
   
   
  /// This returns true if the given Surface is of the type represented by
  /// this SurfaceType.
   virtual bit IsMember(Surface * s) const  = 0;
   
  /// This is given a token table and a data type token, it returns true if the
  /// Fitter object can proccess it and false otherwise. If it returns true it
  /// also outputs outType, which is the type you should pass in for the type 
  /// parameter of the Add method of the Fitter object.
   virtual bit Supports(str::TokenTable & tt,str::Token tok,nat32 & outType) const = 0;
   
  /// &nbsp;
   bit Supports(str::TokenTable & tt,cstrconst tok,nat32 & outType) const
   {
    return Supports(tt,tt(tok),outType);
   }


  /// This returns how many degrees of freedom the surface type has. Used to
  /// estimate if enough data is avaliable to reasonably make an estimate, as
  /// otherwise it dosn't bother.
   virtual nat32 Degrees() const = 0;
   
  /// This returns how many degrees of information the given type index, as 
  /// obtained from the Supports method, provides to the fitter object. In
  /// principal if you make a fitter object and then Add enough times that
  /// the sum of the below for each Add called entered equals or excedes
  /// the Degrees method then you would expect to be able to extract a
  /// surface. It is not assumed, however, that this is allways the case.
   virtual nat32 TypeDegrees(nat32 type) const = 0;


  /// &nbsp;
   virtual cstrconst TypeString() const = 0; 
};

//------------------------------------------------------------------------------
 };
};
#endif
