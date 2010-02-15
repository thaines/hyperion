#ifndef EOS_INF_FIELD_GRAPHS_H
#define EOS_INF_FIELD_GRAPHS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file field_graphs.h
/// This is an extra abstraction layer above and beyond that of the factor
/// graphs system. Instead of expressing individual nodes and connections this
/// allows the expression of entire sets (fields) of nodes and connections from 
/// a library of common node structures and connection patterns. 
/// This allows it to then accelerate convergence using hierachical techneques,
/// by using smaller versions of the fields for earlier iterations
/// to accelerate information transfer across the implied graph.

#include "eos/types.h"

#include "eos/inf/factor_graphs.h"
#include "eos/math/vectors.h"
#include "eos/ds/arrays.h"
#include "eos/ds/lists.h"
#include "eos/ds/sort_lists.h"

namespace eos
{
 namespace inf
 {
//------------------------------------------------------------------------------
/// A helper interface, used by the VariablePattern interface for the transfer 
/// of patterns from one level to the level below.
class EOS_CLASS VariableTransfer
{
 public:
  VariableTransfer(ds::Array<int32> & out):arr(out) {}
 
  /// &nbsp;
   ~VariableTransfer() {}


  /// Its primary virtual method, the VariablePattern is responsible for calling
  /// it for every variable message it wants to transfer, giving the index of 
  /// the from and to variables.
  /// from variables can be repeated, so as to transfer a single variable into
  /// multiple other variables.
   void Transfer(nat32 from,nat32 to)
   {
    arr[to] = from;
   }
   
  /// All 'to' variables must be accounted for, if the pattern does not require 
  /// transfering into particular variables it must call this method to set it
  /// to a flatline initialisation message.
   void Flatline(nat32 to)
   {
    arr[to] = -1;
   }


 private:
  ds::Array<int32> & arr;
};

//------------------------------------------------------------------------------
/// This defines a variable pattern, this is expressed by its type
/// and a parameter vector of nat32
/// that should be matched by all connected FunctionPatterns along with the 
/// type. It has to provide methods that convert the parameter vector
/// into a label count and a variable count. The variable count conversion is 
/// also given the level. Level 0 is the problem actually being solved, each 
/// additional level traditionally reduces the number of variables such that a 
/// solution can be found faster for the smaller problem before it is expanded.
/// An additional method that gives the largest suported level before no more 
/// problem reduction can occur is also to be provided, though on being given 
/// higher levels it should simply repeat the variable count of the maximum level.
/// The final method it has to provide must be able to correctly copy the 
/// messages from a given level to the level below it, on being given a 
/// suitable interface.
class EOS_CLASS VariablePattern : public Deletable
{
 public:
  /// &nbsp;
   ~VariablePattern() {}
   
  /// &nbsp;
   virtual VariablePattern * Clone() const = 0;
   
  /// &nbsp;
   bit operator == (const VariablePattern & rhs) const
   {
    if (str::Compare(typestring(*this),typestring(rhs))!=0) return false;
    const math::Vector<nat32> & p1 = Paras();
    const math::Vector<nat32> & p2 = rhs.Paras();
    return p1==p2;
   }


  /// This returns a const reference to its parameter vector.
  /// This vector should, with the TypeString(), uniquely describe the pattern's
  /// shape such that a connecting FactorPattern can behave itself.
  /// Two variable pattern classes are considered equal if both there 
  /// TypeString() and Paras() match.
   virtual const math::Vector<nat32> & Paras() const = 0;

  /// Returns how many labels the pattern requires for each variable.
   virtual nat32 Labels() const = 0;
   
  /// Returns how many variables are requried by the pattern, on a level by
  /// level basis. Level 0 is the full size, whilst level 1 is after the problem
  /// has been shrunk for the next size smaller hierachy level etc.
   virtual nat32 Vars(nat32 level) const = 0;
   
  /// Returns the maximum level suported by the pattern, note however that this value
  /// is used to find the maximum level from all used patterns, from which inference starts.
  /// In other words, this class should cope with the level being higher.
   virtual nat32 MaxLevel() const = 0;


  /// This is called to transfer all variables from level+1 to level, it must
  /// obtain full coverage. It is given an interface to perform this duty with.
  /// If given a level beyond MaxLevel() it must still apply the relevent transfer,
  /// even though this would be an 'identity mapping'.
   virtual void Transfer(nat32 level,VariableTransfer & vt) const = 0;


  /// &nbsp;
   virtual cstrconst TypeString() const = 0;
};

//------------------------------------------------------------------------------
// Helper structure, used as the interface between the FactorConstruct class
// and the FieldGraph system...
struct PatLink
{
 bit operator < (const PatLink & rhs) const
 {
  if (var!=rhs.var) return var<rhs.var;
  if (fp!=rhs.fp) return fp<rhs.fp;
  if (unique!=rhs.unique) return unique<rhs.unique;
  if (pipe!=rhs.pipe) return pipe<rhs.pipe;

  return false;
 }

 // Key...
  nat32 var; // Variable index within the variable pattern.
  nat32 fp; // factor pattern index which created link.
  nat32 pipe; // pipe within factor pattern which link is associated with.
  nat32 unique; // unique number, PatLink's only match if this matches, provided by factor pattern.

 // Data... (As required to index into the factor graph and extract the appropriate message.)
  nat32 func;
  nat32 inst;
  nat32 link;
};

//------------------------------------------------------------------------------
/// A helper interface, used by the FactorPattern implimentor to express how
/// factors are to be linked to variables for a particular level.
class EOS_CLASS FactorConstruct : public Deletable
{
 public:
  // Constructor...
   FactorConstruct(FactorGraph & fg,nat32 fpp,nat32 pipes)
   :curr(fg),fp(fpp),linkStore(pipes)
   {}

  /// &nbsp;
   ~FactorConstruct() {}


  // Setup - extended construction, only relevent if prev...
   void SetPipe(nat32 ind,ds::SortList<PatLink> * lStore)
   {
    linkStore[ind] = lStore;
   }


  /// This is identical to the method of the same name in the FactorGraph class.
  /// It should be used to obtain indexes to the function objects required to 
  /// construct the relations.
   nat32 MakeFuncs(Function * func,nat32 instances)
   {
    return curr.MakeFuncs(func,instances);
   }
   
  /// Once you have created the relevent function then this method will link 
  /// them to the variables, you need to call this for each link required set.
  /// This is not a pass through, it stores the results and latter passes 'em 
  /// on. This is required so that the interface can be oriented arround factors
  /// rather than variables, unlike the FactorGraph class.
  /// (It also does a whole load of work at the precise point of construction.)
  /// \param func The function index, as returned from the MakeFuncs method.
  /// \param inst Which instance within the function set to use.
  /// \param link Which link for the specific function is to be connected.
  /// \param pipe Which pipe out of the FactorPattern to use.
  /// \param ind The index of the variable within the pipe to connect to.
  /// \param unique A unique number specific to the ind/pipe combination for 
  ///               this function pattern, this is used when transfering from 
  ///               one level to the next. Required as pattern transfer is by
  ///               variable index, not by function/instance/link as in the
  ///               factor graph object. To make the transfer the unique 
  ///               identifier alongside the function index and pipe is used
  ///               as the key when matching the links going into a single variable.
   void Link(nat32 func,nat32 inst,nat32 link,nat32 pipe,nat32 ind,nat32 unique)
   {
    // Store the link to be created into the relevent array...
     PatLink pl;
      pl.var = ind;
      pl.fp = fp;
      pl.pipe = pipe;
      pl.unique = unique;
      pl.func = func;
      pl.inst = inst;
      pl.link = link;
     linkStore[pipe]->Add(pl);
   }


 private:
  FactorGraph & curr;
  nat32 fp; // Index of factor pattern.

  ds::Array<ds::SortList<PatLink>*> linkStore; // Data structure to store the given in for each pipe.
};

//------------------------------------------------------------------------------
/// This specifies the pattern of how a set of functions connect to one or more
/// VariablePattern-s. Instances of this class are first initialised, they then
/// have all, some or none of the pipes variable patterns set.
/// Once setup they are passed to the FieldGraph and the field graph is given 
/// the links. The FieldGraph then finishes setting pipe types and verifies they
/// all match to create a valid problem to solve.
/// Its main job though is the Construct method, which is used to actually create
/// the factor graph represented by the various pattern objects.
class EOS_CLASS FactorPattern : public Deletable
{
 public:
  /// &nbsp;
   ~FactorPattern() {}


  /// This returns how many VariablePatterns it expects to be linked to.
   virtual nat32 PipeCount() const = 0;

  /// Returns true if the pipe index given has been set, i.e. its 
  /// VariablePattern has been set.
   virtual bit PipeIsSet(nat32 ind) const = 0;
   
  /// Returns a reference to the VariablePattern of a particular pipe.
  /// Should only be called if PipeIsSet returns true.
   virtual const VariablePattern & PipeGet(nat32 ind) const  = 0;
   
  /// Allows you to set the VariablePattern of a particular pipe.
  /// When you set a pattern all other links should be updated to reflect 
  /// the change so as to be consistant, i.e. VariablePatterns that no longer
  /// fit the new pattern should be unset or set to new VariablePatterns that
  /// match the new guy.
  /// Returns true if the VariablePattern is accepted, false if it is rejected
  /// and no change is made due to being in-compatible.
   virtual bit PipeSet(nat32 ind,const VariablePattern & vp) = 0;


  /// This is given an interface, using this interface this method must then
  /// create the relevent function object(s) (it does the construction) and 
  /// then call a set of methods to indicate how the function object(s) are to 
  /// be linked in with the variable objects. This construction is of course 
  /// dependent on the current hierachy level.
   virtual void Construct(nat32 level,FactorConstruct & fc) const = 0;


  /// &nbsp;
   virtual cstrconst TypeString() const = 0;
};

//------------------------------------------------------------------------------
/// A field graph is essentially an abstraction of the FactorGraph system such
/// that you work with groups of factors and variables rather than individuals.
/// By working with groups the code becomes pattern aware, and can apply
/// hierachical optimisation methods as a direct result. The essential idea is 
/// that you provide a graph structure where each FactorPattern and each
/// VariablePattern represents a large number of factors/variables. The nodes are
/// far more specific than ushall as they also specify the pattern of how nodes
/// are inter-related and can be connected to form factor graphs.
/// This makes this module considerably more limited, as special development is
/// required for each possible scenario, its value comes in the optimisation, as
/// this entire system is dedicated to accelerating special forms of factor graph.
/// Like the FactorGraph module the focus is on specifying factors, with details
/// with regards to the variables then infered.
/// Due to the fact that the connection pattern of one node limits and often 
/// specifies the pattern of another node you will often not need to specify
/// details, such as sizes, to most nodes, as they can be infered from the
/// surrounding nodes.
class EOS_CLASS FieldGraph : public Deletable
{
 public:
  /// Field graphs are allways loopy, but the min-sum/max-product option still
  /// exists. Set ms==true for the min-sum, otherwise your using max-product.
   FieldGraph(bit ms = false);

  /// &nbsp;
   ~FieldGraph();


  /// Returns true if its doing min-sum, false if its doing max-product.
   bit DoMS() const {return doMS;}

  /// Sets the number of iterations per level that are performed, extra iterations
  /// can be applied at the highest and lowest levels.
  /// \param iters How many iterations per level to do, defaults to 6.
  /// \param extraHigh How many extra iterations to do for the highest level, defaults to 0.
  /// \param extraLow How many extra iteration to do for the lowest, and final, level, defaults to 0.
   void SetIters(nat32 iters,nat32 extraHigh = 0,nat32 extraLow = 0);
   
  /// Sets the maximum level to consider - it will not go higher than this. Best left alone this method.
   void SetMaxLevel(nat32 maxLevel);


  /// Adds a FactorPattern, returning its handle number.
  /// The given pointer is claimed by the FieldGraph, and shall never be heard 
  /// of again.
   nat32 NewFP(FactorPattern * fp);

  /// Creates a new VariablePattern node, returning its handle. No information is provided,
  /// as all details are infered by the factorPatterns which are connected to it.
   nat32 NewVP();

  /// This lays a pipe from a factor pattern to a variable pattern.
  /// \param fp FactorPattern handle.
  /// \param pipe Index of the pipe to use from the FactorPattern.
  /// \param vp VariablePattern handle.
   void Lay(nat32 fp,nat32 pipe,nat32 vp);


  /// Returns how many FactorPatterns have been created, the handles will go from 
  /// 0 to CountFP()-1.
   nat32 CountFP() const {return countFP;}
   
  /// Returns how many VariablePatterns have been created, the handles will go from 
  /// 0 to CountVP()-1.
   nat32 CountVP() const {return countVP;}


  /// Solves the specified monstrosity. For all its tricks this will not be fast.
  /// (And the progress bar won't be linear.)
  /// Returns true on success, false on failure. Failure will ushally occur due
  /// to bad pipes being layed, i.e. incompatibility between nodes.
  /// This is not checked until run is called.
   bit Run(time::Progress * prog = null<time::Progress*>());

  /// Post Run this method can be used to extract results. 
  /// Note that conversion via FromNegLn is automatic, just remember that if 
  /// DoMS()==true then the probabilitys have been scaled by an arbitary convex 
  /// function, so interpolation doesn't really make sense.
  /// \param vp VariablePattern handle.
  /// \param ind Index into the VariablePattern, it is assumed the user 
  ///            understands a linear indexing scheme of level 0 for whatever 
  ///            pattern is represented.
   Frequency Get(nat32 vp,nat32 ind)
   {
    return Frequency(res[vp].mc,&res[vp].res[ind*res[vp].mc.scale]);
   }

  /// After Run has been called it will have calculated the type of each 
  /// variable pattern, this can then be queryed. Can be useful if only for 
  /// getting the acceptable ranges of ind for the Get method.
   const VariablePattern & VP(nat32 vp) const
   {
    return *res[vp].vp;
   }


  /// &nbsp;
   static cstrconst TypeString() {return "eos::inf::FieldGraph";}


 private:
  bit doMS;
  nat32 maximumLevel;

  nat32 iters;
  nat32 extraHigh;
  nat32 extraLow;
  
  nat32 countFP;
  nat32 countVP;
  ds::Array<FactorPattern*> fp;
  
  // This stores all the calls to Lay, it is not proccessed heavily until 
  // the actual call to run...
   struct WorkOrder
   {
    nat32 fp;
    nat32 pipe;
    nat32 vp;
   };
   ds::List<WorkOrder> wol;

  // This post-run data structure stores the data necesary to sate the Get and VP methods.
   struct VariableResult
   {
    VariablePattern * vp;
    MessageClass mc; // needed as otherwise we can't return Frequency objects, also caches the label count to save querying vp as that can be slow.
    real32 * res; // Array of floating point values, vp->Labels()*vp->Vars(0) in size. Go figgure. Malloced.
   };
   ds::Array<VariableResult> res;


  // The below stuff all exists to help the Run method...
   // This updates (or not if its allready set.) the VariablePattern of a pipe
   // for a FactorPattern. It returns true if the update was successful, 
   // or false if a conflict occured, i.e. that pipe can not be that variablePattern...
   // The variable set is set to the number of pipes now set that wern't set before.
    static bit SetVarPat(FactorPattern & fp,nat32 pipe,const VariablePattern & vp,nat32 & set);
    
   // This structure contains a factor graph and other information required to be passed
   // from one level to the next...
    struct LevelData
    {
     LevelData(bit doMS):fg(doMS,true) {}

     FactorGraph fg;
     
     // For each variable pattern we store a dictionary of links created,
     // with the details required for several operations...
      ds::Array<ds::SortList<PatLink>,
                mem::MakeNew<ds::SortList<PatLink> >,
                mem::KillOnlyDel<ds::SortList<PatLink> > > vpl;
                
     // For each variable pattern we also store an array of variable index 
     // to actual variable as stored in factor graph...
      ds::Array<ds::Array<nat32>,
                mem::MakeNew<ds::Array<nat32> >,
                mem::KillOnlyDel<ds::Array<nat32> > > piv;
    };
    
   // This actually does the real work, kept in here for neatness, only does a single level,
   // on being given the previous level so it can be transfered in...
   // (level is the level of curr, prev is level+1)
    void DoLevel(nat32 level,LevelData * prev,LevelData & curr,time::Progress * prog);
};

//------------------------------------------------------------------------------
 };
};
#endif
