#ifndef EOS_INF_FACTOR_GRAPHS_H
#define EOS_INF_FACTOR_GRAPHS_H
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


/// \file factor_graphs.h
/// Provides the factor graph solver object. General usage is to create all the
/// function and random variables, give them to this object and Run it. This 
/// might give you some kind of result, which you might find useful.

#include "eos/types.h"
#include "eos/inf/fg_funcs.h"
#include "eos/inf/fg_vars.h"
#include "eos/time/progress.h"
#include "eos/data/blocks.h"
#include "eos/ds/lists.h"
#include "eos/ds/scheduling.h"

namespace eos
{
 namespace inf
 {
//------------------------------------------------------------------------------
/// This represents a factor graph solver. Due to its massive memory consumption
/// requirements a different data structure should be used for storage, with
/// this reserved for when a solution wants to be found. Construction consists
/// of creating various objects and then passing them into the object, calling
/// lots of methods and then running it. Various but limited abilities to 
/// construct a factor graph and run it multiple times are provided, this is to
/// allow hierachical solutions - you can provide frequency data to initialise
/// one graph from the results of another and/or solve a graph then add extra nodes
/// and solve again to slowelly increase a models complexity.
///
/// Implimented using the sum-product belief propagation algorithm. Ushally all
/// functions are considered to be probability density functions of there variables.
/// Suports any message type, currently only a standard frequency function is 
/// suported but gaussian mixtures and other continuous techneques will join the
/// ranks as implimented.
///
/// Note that due to the rather crazy nature of the optimisation this object 
/// attempts the interface is often untyped, dangerous and plain strange. Get 
/// over it. In the intrest of sanity other objects can be provided with
/// interfaces specialised for solving specific problems that use this object as
/// there inference engine.
///
/// Note that the non-loopy implimentation is far more limited than the loopy 
/// implimentation in terms of scale, as it produces a complete execution schedule
/// seperatly then runs, so it has to store the schedule. This results in basic size 
/// limitations on how large a non-loopy tree can be solved this way, this could be 
/// fixed but isn't as an issue right now.
class EOS_CLASS FactorGraph
{
 public:
  /// You have to set the message passing schedule on construction, so the correct
  /// internal data structures are created as you make the graph.
  /// You also have to set the solving method, the default of ms==false does the 
  /// sum-product algorithm whilst ms==true does the min-sum algorithm.
  /// Note that if using min-sum it is in fact calculating the set that maximises
  /// overall probability rather than individual probability, and that output is 
  /// left in negative log space, so you get a very different answer.
  /// (When selecting the best for sum-product you want the maximum, when selecting
  /// the best for min-sum you want the minimum, unless you convert from -ln space.)
  /// There are two supported schedules:
  /// - non-loopy - Produces the correct result, only works on trees however. Only passes one message per link with no iterations required, making it relativly fast.
  /// - loopy - Does not neccesarily converge and requires iterations so it is rather slow. Works on graphs however, making it considerably more flexable. In practise this is ushally used as most problems can't be expressed as trees.
   FactorGraph(bit ms = false,bit loopy = true);
  
  /// &nbsp;
  ~FactorGraph();


  /// Returns true if the factor graph was constructed to use minimum-sum rather 
  /// than sum-product
   bit DoMS() const {return doMS;}

  /// Returns true if the factor graph was constructed to solve with the loopy method.
   bit DoLoopy() const {return loopy;}


  /// When using loopy belief propagation you have to set the number of iterations,
  /// should be set high enough for information to flow between nodes in the graph.
  /// Ushally set to the longest route between any two nodes. Setting it far lower
  /// can be obtained by using hierachical methods to obtain reasonable estimates 
  /// before iterations start, or by providing short cuts for information to flow
  /// along in the graph structure.
  /// Defaults to 1, which is too low for anything, so you must call this if loopy.
   void SetIters(nat32 its) {iters = its;}


  /// This creates a number of instances of a function, returning a handle to 
  /// refer to that set in future calls. The Function is absorbed and from then
  /// on in owned by this object - it will be deleted when the object dies.
  /// You must not edit it post-absorption.
   nat32 MakeFuncs(Function * func,nat32 instances);
   
  /// This creates a link between a function variable and a Variable that,
  /// unsuprisingly, represent a random variable. You declare the Variable
  /// object yourself, you then call this method on it several times to add
  /// all the links before finally using MakeVar to add the Variable
  /// to the mix and get a handle to it.
  /// Note that of course you can only link each function/instance/index 
  /// combination to one Variable as otherwise things will go screwy. There is
  /// no checking that this is observed.
  ///
  /// Note that you do not need to link every function/instance/link combination
  /// created, any that are ignored will be left as an even probability of any,
  /// which is ushally the correct behaviour. This is often helpful for dealing
  /// with boundary conditions.
  ///
  /// \param variable The variable to make the link to.
  /// \param function The handle returned by MakeFuncs for the function set you want to link to.
  /// \param instance The specific function instance within the function set.
  /// \param link The index of the link in the function to link to, see the Function object for these.
  /// \returns true if the link is created, false otherwise. It can fail if the operation would result in multiple message types being assigned to the variable.
   bit MakeLink(Variable * variable,nat32 function,nat32 instance,nat32 link);
   
  /// Once you have added all the links you require to a Variable object
  /// you can add it in. This method is rather strange in that it dosn't
  /// absorb the Variable object but resets it instead. This means that 
  /// ushall setup procedure is to make a single Variable object, then 
  /// iterate over calling MakeLink several times then calling MakeVar,
  /// until all variables wanted are in the system. You then delete the
  /// Variable object. Returns a handle to the link, so you can latter
  /// request its probability function.
   nat32 MakeVar(Variable * var);


  /// This allows you to set the starting message going into a function, 
  /// only useful for loopy belief propagation. Used in hierachical 
  /// techneques to set the messages based on the previous results of a
  /// simpler model. Returns true if it works, false if it fails due to
  /// the passed in MessageClass not matching. It of course can't check 
  /// that the message pointer is actually of the MessageClass type.
  /// \param function The function set handle returned by MakeFuncs.
  /// \param instance The instance of the function within the set.
  /// \param link The index of the variable link.
  /// \param type The type of the message.
  /// \param message Pointer to the block of data that represents the message.
  /// \returns true if the types matches and the update was done, false if they didn't and nothing happened.
   bit SetMsg(nat32 function,nat32 instance,nat32 link,const MessageClass & type,const void * message);


  /// Runs the algorithm to produce a probability function for each and 
  /// every random variable. This will consume serious amounts of time.
  /// Whilst you can't delete added functions and variables from this
  /// object you can Run is a number of iterations, add more detail to
  /// the model then run it again, and it will use the probability 
  /// functions found in the previous pass for the variables that 
  /// existed then. If doing that you might want to save some time by
  /// not calculating the actual probability functions - you can do this
  /// by setting multipass to true, just remember to set it false for the
  /// last pass so you can extract results. multipass also changes the
  /// final state its left in to make sure all information passes between
  /// two subsequant Runs(), which wouldn't happen otherwise.
  /// multipass is ignored if non-loopy BP is used, as such behaviour would
  /// not then make sense.
  /// For loopy BP it will allways return true, for non-loopy it can fail 
  /// however and return false if it detects that you have given it a graph
  /// rather than a tree.
   bit Run(time::Progress * prog = null<time::Progress*>(),bit multipass = false);
   
  /// Resets all the messages. Useful if you want to run again with the same
  /// structure but with a new data set behind some data-sensitive function.
   void Reset();


  /// Extracts the final probability function for a given random variable handle,
  /// must only be called after a call to Run with multipass set to false.
  /// Simply returns a raw pointer to the result.
   void * GetProb(nat32 var) {return &output[outIndex[var]];}
   
  /// A conveniance version of GetProb, converts it straight into a message type
  /// object...
   template <typename T>
   T Prob(nat32 var) {return T(Variable::Class(vars[var]),&output[outIndex[var]]);}

  /// Extracts the final message to a function so it can be put into another
  /// graph using the SetMsg method with which it partners. Works best to call
  /// this after a multipass run rather than not, as it misses an iteration
  /// if not.
   const void * GetMsg(nat32 function,nat32 instance,nat32 link) const;

  /// Proves useful in collaboration with SetMsg and GetMsg, returns a const 
  /// reference to the type of a function variable link.
   const MessageClass & GetType(nat32 function,nat32 link) const;


  /// If you are using the ms variant all the final messages output above will
  /// remain in negative log mode, so you can build hierachies. This converts 
  /// all messages from negative ln to basic probability. Only call once after
  /// running, once called you can not run again as the internal data would be
  /// wrong. You can choose not to call this and simply use the -ln probability
  /// values directly if you so wish.
  /// Does nothing if not solving using the min-sum method.
  /// Note that the normalisation used removes the ability to actually recover
  /// correct probabilities, this simply assignes them so they are proportional,
  /// with the most likelly the highest probability and the least likelly the lowest.
  /// It is not necesarilly linear however, so interpolating is technically the
  /// wrong thing to do, doesn't mean it doesn't work to some extent however.
   void FromNegLn();


  /// &nbsp;
   static cstrconst TypeString() {return "eos::inf::FactorGraph";}


 private:
  static const nat32 funcInc = 16; // How big an increase we use when the array overflows.
  static const nat32 varInc = 4096; // How big an increase we use when the array overflows.

  bit doMS;
  bit loopy;
  nat32 iters;


  // The actual data structure, these arrays are often larger than needed for
  // efficiency, so seperate counters are required...
   nat32 funcCount;
   ds::Array<FunctionSet*> funcs;
   
   nat32 varCount;
   ds::Array<void*> vars;
   
  // This structure is only constructed if we are in non-loopy mode, so we can 
  // schedule the message passes correctly... (A linked list of all links.)
   struct Link
   {
    nat32 var;
    nat32 varLink;
    nat32 func;
    nat32 funcInst;
    nat32 funcLink;
   };
   ds::List<Link> links;
   
  // Data structure for storing the final output probability functions...
   nat32 * outIndex; // Array indexed by probability function of offsets into output for that message.
   byte * output; // Large data blob of tightly packed probability functions.


  // Internal functions, the real Run methods, one for each mode...
   bit RunNonLoopySP(time::Progress * prog);  
   void RunLoopySP(time::Progress * prog,bit multipass);
   void CalcOutputSP();

   bit RunNonLoopyMS(time::Progress * prog);  
   void RunLoopyMS(time::Progress * prog,bit multipass);
   void CalcOutputMS();

   
  // Extra structures used by the non-loopy message passing arrangment...
   struct MsgJob : public Link
   {
    enum {ToVar,ToFunc} dir;
   };

   struct State
   {
    enum {None,One,All} done;
    nat32 link; // For the One to All transition, so we know what to not.
   };
   
   // Creates a single data structure array of jobs, where you can request the in 
   // the and out job of each function/instance/link triplet.
   class JobStore
   {
    public:
     JobStore(const ds::Array<FunctionSet*> & funcs,nat32 funcCount)
     {
      offset = new nat32[funcCount];
      size = new nat32[funcCount];
      
      // Work out sizes and offsets...
       offset[0] = 0;
       size[0] = funcs[0]->GetFunc()->Links();
       for (nat32 i=1;i<funcCount;i++)
       {
        size[i] = funcs[i]->GetFunc()->Links();
        offset[i] = offset[i-1] + funcs[i-1]->Instances() * size[i-1] * 2;
       }
       nat32 totalSize = offset[funcCount-1] + funcs[funcCount-1]->Instances() * size[funcCount-1] * 2;
      
      // Create and set all the Jobs to be Bad...
       data = new ds::Scheduler<MsgJob>::Job[totalSize];
       for (nat32 i=0;i<totalSize;i++) data[i] = ds::Scheduler<MsgJob>::NullJob();
     }
      
     ~JobStore()
     {
      delete[] offset;
      delete[] size;           
      delete[] data;
     }
     
     ds::Scheduler<MsgJob>::Job & GetToVar(nat32 func,nat32 inst,nat32 link) {return data[offset[func] + inst*size[func]*2 + link*2];}     
     ds::Scheduler<MsgJob>::Job & GetToFunc(nat32 func,nat32 inst,nat32 link) {return data[offset[func] + inst*size[func]*2 + link*2 + 1];}


    private:
     nat32 * offset;
     nat32 * size; // Number of jobs per function for each fuction group.
     ds::Scheduler<MsgJob>::Job * data;
   };
};

//------------------------------------------------------------------------------
 };
};
#endif
