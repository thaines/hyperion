#ifndef EOS_INF_FIG_FACTORS_H
#define EOS_INF_FIG_FACTORS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file fig_factors.h
/// Provides implimentation of factor patterns for the field graph system.

#include "eos/types.h"
#include "eos/inf/field_graphs.h"
#include "eos/inf/fig_variables.h"

namespace eos
{
 namespace inf
 {
//------------------------------------------------------------------------------
/// This allows you to set the probability of each labal for each variable in a
/// Grid2D.
class EOS_CLASS GridFreq2D : public FactorPattern
{
 public:
  /// You have to set the size and label count on construction, as before use
  /// you need to fill in all such data.
   GridFreq2D(nat32 labels,nat32 width,nat32 height);

  /// &nbsp;
   ~GridFreq2D();


  /// Sets the probability of a particular label at a particular coordinate.
  /// Each variable is normalised before use, you must set every last value however.
  /// If you are using this you should not be using SetCost.
   void SetFreq(nat32 x,nat32 y,nat32 label,real32 freq);

  /// Sets the -ln probability of a particular label at a particular coordinate.
  /// Each variable is droped before use, you must set every last value however.
  /// If you are using this you should not be using SetFreq.
   void SetCost(nat32 x,nat32 y,nat32 label,real32 cost);
   

  /// &nbsp; 
   nat32 PipeCount() const;

  /// &nbsp;
   bit PipeIsSet(nat32 ind) const;

  /// &nbsp;
   const VariablePattern & PipeGet(nat32 ind) const;

  /// &nbsp;
   bit PipeSet(nat32 ind,const VariablePattern & vp);


  /// &nbsp;
   void Construct(nat32 level,FactorConstruct & fc) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  nat32 width;
  nat32 height;
  nat32 labels;
  Grid2D * vp;

  bit ms;
  mutable nat32 calcLevel; // The highest level thats been calculated, starts at 0 as that level is set by SetFreq.
  ds::Array<real32*> data; // data[level], data.Size() is MaxLevel()+1, repetition is used above this.
};

//------------------------------------------------------------------------------
/// Given two identically sized 2D grids this applys a relationship between 
/// matched variables using a joint distribution set on a case by case basis.
class EOS_CLASS GridJointPair2D : public FactorPattern
{
 public:
  /// You setup its details on construction, as it has to create storage for it 
  /// all.
  /// \param labels0 Number of labels for pipe 0.
  /// \param labels1 Number of labels for pipe 1.
  /// \param width Width of both grids.
  /// \param height Height of both grids.
  /// \param ms True if you are going to set all the values as costs, false 
  ///           to use probabilities.
   GridJointPair2D(nat32 labels0,nat32 labels1,nat32 width,nat32 height,bit ms = false);

  /// &nbsp;
   ~GridJointPair2D();


  /// Returns a reference to a given probability/cost, so you can set it. 
  /// You must set every last one.
   real32 & Val(nat32 x,nat32 y,nat32 lab0,nat32 lab1);


  /// &nbsp; 
   nat32 PipeCount() const;

  /// &nbsp;
   bit PipeIsSet(nat32 ind) const;

  /// &nbsp;
   const VariablePattern & PipeGet(nat32 ind) const;

  /// &nbsp;
   bit PipeSet(nat32 ind,const VariablePattern & vp);


  /// &nbsp;
   void Construct(nat32 level,FactorConstruct & fc) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  nat32 labels0;
  nat32 labels1;
  nat32 width;
  nat32 height;
  bit ms;

  Grid2D * pipe[2]; // The two pipes.

  // Array of joint distributions for each level. Each level is indexed by 
  // height,width,labels1,labels0 in that order, with the width and height 
  // adjusted accordingly for the level. Everything is stored in min-sum mode,
  // but can be converted to probabilities if required...
   mutable ds::Array<real32*> data;
};

//------------------------------------------------------------------------------
/// Puts the differential of the 2d grid thats on pipe 0 into pipe 1,
/// and/or vice-versa depending on how you want to think about it.
/// pipe[1].labels == pipe[0].labels*2-1.
/// The sizes of the two pipes grides are kept identical, even though this 
/// wastes a row/column in pipe 1's grid, as this avoids needing a different variable
/// pattern for the second pipe.
class EOS_CLASS GridDiff2D : public FactorPattern
{
 public:
  /// If dir is false then it does dx, if true it does dy.
   GridDiff2D(bit dir);

  /// &nbsp;
   ~GridDiff2D();


  /// Sets the lambda used by the DifferencePotts function to indicate chance of
  /// differential being wrong.
   void SetLambda(real32 lambda);


  /// &nbsp; 
   nat32 PipeCount() const;

  /// &nbsp;
   bit PipeIsSet(nat32 ind) const;

  /// &nbsp;
   const VariablePattern & PipeGet(nat32 ind) const;

  /// &nbsp;
   bit PipeSet(nat32 ind,const VariablePattern & vp);


  /// &nbsp;
   void Construct(nat32 level,FactorConstruct & fc) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  bit dir; // false==dx, true==dy.
  real32 lambda; 
  Grid2D * pipe[2]; // The two pipes.
};

//------------------------------------------------------------------------------
/// Smooths a grid using a Potts equality factor, in only one direction at a time.
class EOS_CLASS GridSmoothPotts2D : public FactorPattern
{
 public:
  /// If dir is false then it does dx, if true it does dy.
  /// If con is true you set smoothing parameters for the entire field, 
  /// if false you set a unique value for every instance.
  /// width and height are only used if con==false, otherwise there ignored.
   GridSmoothPotts2D(bit dir,bit con = true,nat32 width = 0,nat32 height = 0);

  /// &nbsp;
   ~GridSmoothPotts2D();


  /// If you are running in con==true mode then this set the parameter for the
  /// entire field.
   void Set(real32 equal);
   
  /// If you are running in con==false mode then this sets the parameter on an 
  /// entry by entry basis. You give the lower coordinate of the term your setting,
  /// i.e. if you give (x,y) then the smoothing term you are setting is between
  /// (x,y) and (x+1,y) or (x,y+1) depending on dir. You should not therefore call
  /// it for the last row/column depending on mode, though no harm is done if you do.
   void Set(nat32 x,nat32 y,real32 equal);


  /// &nbsp; 
   nat32 PipeCount() const;

  /// &nbsp;
   bit PipeIsSet(nat32 ind) const;

  /// &nbsp;
   const VariablePattern & PipeGet(nat32 ind) const;

  /// &nbsp;
   bit PipeSet(nat32 ind,const VariablePattern & vp);


  /// &nbsp;
   void Construct(nat32 level,FactorConstruct & fc) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  bit dir;
  bit con;
  nat32 width;
  nat32 height;
  Grid2D * vp;
  
  // For con==true mode...
   real32 equal;
  
  // For con==false mode...
   mutable nat32 calcLevel; // The highest level thats been calculated, starts at 0 as that level is set by Set.
   ds::Array<real32*> data; // data[level], data.Size() is MaxLevel()+1, repetition is used above this.
};

//------------------------------------------------------------------------------
/// Smooths a grid using a Laplace equality factor, in only one direction at a time.
class EOS_CLASS GridSmoothLaplace2D : public FactorPattern
{
 public:
  /// If dir is false then it does dx, if true it does dy.
  /// If con is true you set smoothing parameters for the entire field, 
  /// if false you set a unique value for every instance.
  /// width and height are only used if con==false, otherwise there ignored.
   GridSmoothLaplace2D(bit dir,bit con = true,nat32 width = 0,nat32 height = 0);

  /// &nbsp;
   ~GridSmoothLaplace2D();


  /// If you are running in con==true mode then this set the parameters for the
  /// entire field.
   void Set(real32 corruption,real32 sd);

  /// If you are running in con==true mode then this sets the parameters for the
  /// entire field. This version works in -ln space.
   void SetMS(real32 mult,real32 max);
   
  /// If you are running in con==false mode then this sets the parameters on an 
  /// entry by entry basis. You give the lower coordinate of the term your setting,
  /// i.e. if you give (x,y) then the smoothing term you are setting is between
  /// (x,y) and (x+1,y) or (x,y+1) depending on dir. You should not therefore call
  /// it for the last row/column depending on mode, though no harm is done if you do.
   void Set(nat32 x,nat32 y,real32 corruption,real32 sd);
   
  /// Set Set for details, this is the -ln version. You must use either Set or SetMS,
  /// you can not mix and match the two calls for a single object.
   void SetMS(nat32 x,nat32 y,real32 mult,real32 max);


  /// &nbsp; 
   nat32 PipeCount() const;

  /// &nbsp;
   bit PipeIsSet(nat32 ind) const;

  /// &nbsp;
   const VariablePattern & PipeGet(nat32 ind) const;

  /// &nbsp;
   bit PipeSet(nat32 ind,const VariablePattern & vp);


  /// &nbsp;
   void Construct(nat32 level,FactorConstruct & fc) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  bit dir;
  bit con;
  nat32 width;
  nat32 height;
  Grid2D * vp;
  bit ms; // false if normal mode, true if min-sum mode.

  struct Para
  {
   real32 corruption;
   real32 sd;
  };
  
  // For con==true mode...
   Para para;
  
  // For con==false mode...
   mutable nat32 calcLevel; // The highest level thats been calculated, starts at 0 as that level is set by Set.
   ds::Array<Para*> data; // data[level], data.Size() is MaxLevel()+1, repetition is used above this.
};

//------------------------------------------------------------------------------
/// Smooths a grid with a circular normal distribution equality factor.
/// Only does one direction at a time.
class EOS_CLASS GridSmoothVonMises2D : public FactorPattern
{
 public:
  /// &nbsp;
   GridSmoothVonMises2D();
   
  /// &nbsp;
   ~GridSmoothVonMises2D();


  /// Sets the k value and cutMult for the entire smoothing field.
   void Set(real32 k,real32 cutMult);
  

  /// &nbsp; 
   nat32 PipeCount() const;

  /// &nbsp;
   bit PipeIsSet(nat32 ind) const;

  /// &nbsp;
   const VariablePattern & PipeGet(nat32 ind) const;

  /// &nbsp;
   bit PipeSet(nat32 ind,const VariablePattern & vp);


  /// &nbsp;
   void Construct(nat32 level,FactorConstruct & fc) const;


  /// &nbsp;
   cstrconst TypeString() const;


 private:
  real32 k;
  real32 cutMult;
  
  Grid2D * vp;
};

//------------------------------------------------------------------------------
 };
};
#endif
