#ifndef EOS_INF_FG_VARS_H
#define EOS_INF_FG_VARS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file fg_vars.h
/// Provides the random variable representation capability for the user of the
/// factor graph solver to use. Also provides internal representation for the 
/// random variables, which user-view random variables are converted into on 
/// commit, to save on storage space and improve cache consistancy.

#include "eos/types.h"
#include "eos/inf/fg_types.h"
#include "eos/inf/fg_funcs.h"
#include "eos/ds/arrays.h"
#include "eos/data/blocks.h"

namespace eos
{
 namespace inf
 {
//------------------------------------------------------------------------------
/// The user-view random variable representation, a user of the FunctionGraph 
/// class builds instances of these for each individual random variable and then
/// adds them to the FunctionGraph class. On combining with the function
/// graph they are changed into a different data structure, which has better 
/// properties for solving a factor graph but impossible for editting. A proxy
/// object is provided to the user after this data changing event to allow them
/// to latter extract a probability distribution for the random variable.
/// Most of the interface is hidden, all use of this object comes from passing
/// it into the FactorGraph object.
class EOS_CLASS Variable
{
 public:
  /// &nbsp;
   Variable();
   
  /// &nbsp;
   ~Variable();

  // Resets the Variable to contain no links.
   void Reset();


  // Structure used to represent a link from the Variable point of view.
   struct Link
   {
    nat32 fsInd;
    FunctionSet * fs;
    nat32 instance;
    nat32 link;
   };

  
  /// If you know how many links your planning on adding calling this first
  /// will pre-allocate the memory and save on thrashing.
   void WillAdd(nat32 links);

  // This links the node to a function, type checks and returns true if it verifies
  // and false if the types don't match and therefore this new link hasn't been added.
   bit AddLink(const Link & link);
   
  /// Returns how many links are contained within.
   nat32 LinkCount() const;
   
  // Outputs the details for the n'th link.
   void LinkDetail(nat32 n,Link & out) const;
   

  // This extracts a mem::Malloc-ed data block that represents the variable,
  // it should latter be mem::Free-ed.
  // The structure of the block is as follows:
  // sizeof(MessageClass) - the message class to use.
  // sizeof(nat32) - the number of links.
  // link = sizeof(void*) - the incomming message to variable pointer.
  //        sizeof(void*) - the outgoing message from variable pointer.  
   void * GetPacked() const;

  // Extracts and returns the number of links that a packed representation 
  // of a variable contains.
   static nat32 Links(void * pv);
   
  // Returns a const reference to the MessageClass of a packed variable.
   static const MessageClass & Class(void * pv);


  // A function that is given a packed variable - it sends one message.
  // Provided with a tempory block of data, if its not big enough it 
  // makes it larger.
  // The sum-product version.
   static void SendOneSP(void * pv,nat32 link);
  
  // A function that is given a packed variable - it sends all but one message.
  // Provided with a tempory block of data, if its not big enough it 
  // makes it larger.
  // The sum-product version.
   static void SendAllButOneSP(void * pv,nat32 link,data::Block & temp);
  
  // A function that is given a packed variable - it sends all messages.
  // Provided with a tempory block of data, if its not big enough it 
  // makes it larger.
  // The sum-product version.
   static void SendAllSP(void * pv,data::Block & temp);
   
  // Calculates the final output probability function, sticking it into the
  // given pointer to the correct kind of message. Simply multiplies all
  // the incomming messages together in other words.
  // The sum-product version.
   static void CalcOutputSP(void * pv,void * msgOut);


  // A function that is given a packed variable - it sends one message.
  // Provided with a tempory block of data, if its not big enough it 
  // makes it larger.
  // The min-sum version.
   static void SendOneMS(void * pv,nat32 link);

  // A function that is given a packed variable - it sends all but one message.
  // Provided with a tempory block of data, if its not big enough it 
  // makes it larger.
  // The min-sum version.
   static void SendAllButOneMS(void * pv,nat32 link);

  // A function that is given a packed variable - it sends all messages.
  // Provided with a tempory block of data, if its not big enough it 
  // makes it larger.
  // The min-sum version.
   static void SendAllMS(void * pv);

  // Calculates the final output probability function, sticking it into the
  // given pointer to the correct kind of message. Simply sums all
  // the incomming messages together in other words.
  // The min-sum version.
   static void CalcOutputMS(void * pv,void * msgOut);


  /// &nbsp;
   static cstrconst TypeString() {return "eos::inf::Variable";}


 private:
  // How much it increases the data array by each time it overflows.
   static const nat32 jumpSize = 16;
 
  // If the number of links to a variable is less than or equal to this
  // it uses the O(n^2) algorithm, otherwise it uses the O(n log n)
  // algorithm.
   static const nat32 algChange = 8;


  // Internally we use an array of details, the array only ever getting larger
  // in incriments, never smaller.
   ds::Array<Link> data;
   nat32 links; // How many links are stored in the above array.

  // The structure that makes up the head of a packed variable...
   struct Packed
   {
    MessageClass mc;
    nat32 size;
    struct Link
    {
     void * in;
     void * out;
    } link[1];
   };
   
  // Structure used in the SendAllButOne and SendAll functions for managing 
  // the nutty diy stack...
   struct Frame
   {
    nat32 min,max; // The range of offsets to consider.
    nat32 centre; // (min+max)/2.
    enum {GoLeft, // Next operation is to calcaulate the left chid.
          GoRight, // Next operation is to calculate the right child.
          GoUp, // Next operation is to return from this virtual function call.
         } op;
   }; // Followed by the left message then the right message.
};

//------------------------------------------------------------------------------
 };
};
#endif
