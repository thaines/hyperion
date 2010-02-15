#ifndef EOS_INF_FG_FUNCS_H
#define EOS_INF_FG_FUNCS_H
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


/// \file fg_funcs.h
/// Provides the function representation capability for the factor graph solver,
/// and various frameworks and final functions for the user to choose from.

#include "eos/types.h"
#include "eos/inf/fg_types.h"
#include "eos/math/functions.h"
#include "eos/math/vectors.h"
#include "eos/time/progress.h"
#include "eos/data/buffers.h"
#include "eos/ds/arrays.h"

namespace eos
{
 namespace inf
 {
//------------------------------------------------------------------------------
/// This object represents a message set, as given to the Function object so it
/// can produce results. This object is initialised with pointers to various 
/// bits of information, from which it provides an interface that a function 
/// implimentation can use to get at both its input and output.
class EOS_CLASS MessageSet
{
 public:
  // Constructs this object, it requires 3 pointers and an array of message
  // classes.
  // \param mca An array of message classes that synches with both the input and output types.
  // \param base The baseline from which offsets work.
  // \param inOffset Array of offsets to get to the incomming messages for the function.
  // \param outOffset Array of offsets to get to the outgoing messages for the function.
   MessageSet(MessageClass * mca,void * base,nat32 * inOffset,nat32 * outOffset)
   :mc(mca),data((byte*)base),in(inOffset),out(outOffset)
   {}
   
  // Copy constructor, allways useful.
   MessageSet(const MessageSet & rhs)
   :mc(rhs.mc),data(rhs.data),in(rhs.in),out(rhs.out)
   {}

  // &nbsp;
   ~MessageSet() {}
   
   
  // To be used internally only, for changing the baseline when looping over a large number
  // of messages that need doing.
   void SetBase(void * base) {data = (byte*)base;}


  /// Returns an interface to a particular incomming message, given
  /// an index of its position. No type checking is done here, it 
  /// better be the type published by the Function class to appear
  /// at this index, and the index better be in range.
   template <typename T>
   T GetIn(nat32 ind) const {return T(mc[ind],data + in[ind]);}

  /// Returns an interface to a particular outgoing message, given
  /// an index of its position. No type checking is done here, it 
  /// better be the type published by the Function class to appear
  /// at this index, and the index better be in range.
   template <typename T>
   T GetOut(nat32 ind) const {return T(mc[ind],data + out[ind]);}


  /// &nbsp;
   static cstrconst TypeString() {return "eos::inf::MessageSet";}


 private:
  MessageClass * mc;
  byte * data;
  nat32 * in;
  nat32 * out;
};

//------------------------------------------------------------------------------
/// The interface for functions given to a factor graph. Ultimatly implimentors
/// of this interface boil down to specifying a probability density function from 
/// all input parameters, which is then turned into a set of messages to be passed
/// back to the random variables. However, due to the potential of optimisation
/// the interface is exposed at the message creation level. Any and all attempts
/// at optimising message creation should be made, as its the time consuming part
/// of solving a factor graph, and the brute force method is horrific as an inner 
/// loop code lump. Note that this should output normalised messages, as otherwise
/// numbers will tend to explode.
/// Additionally, this has to support both the sum-product and min-sum algorithms,
/// hence the SP and MS namming conventions. SP is the presumed default, as it deals
/// with probabilities directly, if MS is going to be performed the method ToMS is 
/// called when the function is absorbed. This allows an implimentation to convert 
/// data structures if necesary. Because of the negative log probability usage a 
/// probability of 0 can not be suported in this mode. Implimentations must handle 
/// this by setting a suitable value, suggested as a 'cost' of 10.
/// Offsetting all values such that the lowest value is zero should be performed 
/// for the MS version rather than vector normalisation to one.
class EOS_CLASS Function : public Deletable
{
 public:
  /// &nbsp;
   ~Function() {}


  /// This must return how many messages the function takes.
   virtual nat32 Links() const = 0;
  
  /// Given the index of each link, 0..Links()-1, this should output
  /// the message type that travels along that link.
   virtual void LinkType(nat32 ind,MessageClass & out) const = 0;


  /// This is called after all editting, if SP is going to be used rather than MS.
  /// If called the method can presume that no MS methods will be called
  /// and only SP methods, otherwise visa-versa. This is ushally used to 
  /// convert the internal data from negative log probabilities to probabilitys.
   virtual void ToSP() {}

  /// This is called to calculate a single output message from the rest,
  /// used only when calculating an exact non-loopy solution on a tree.
  /// mtc is the index of the message to calculate.
  /// Sum-product varient.
   virtual void SendOneSP(nat32 instance,const MessageSet & ms,nat32 mtc) = 0;

  /// This is called to calculate every output message but one from the
  /// in messages, used only when calculating an exact non-loopy solution
  /// on a tree.
  /// mti is the index of the message to not send. This is the compliment
  /// of SendOne, and will invariably be called after SendOne with mti==mtc
  /// for each instance of a function.
  /// Sum-product varient.
   virtual void SendAllButOneSP(nat32 instance,const MessageSet & ms,nat32 mti) = 0;

  /// This is called to calculate every output message, used exclusivly for
  /// in-exact loopy solving of graphs. The reality is the other two are 
  /// provided for completness - this is the one that is mostly used and 
  /// should therefore see the lions share of optimisation efforts.
  /// Sum-product varient.
   virtual void SendAllSP(nat32 instance,const MessageSet & ms) = 0;


  /// This is called after all editting, if MS is going to be used rather than SP.
  /// If called the method can presume that no SP methods will be called
  /// and only MS methods, otherwise visa-versa. This is ushally used to 
  /// convert the internal data from probabilitys to negative log probabilities.
   virtual void ToMS() {}

  /// This is called to calculate a single output message from the rest,
  /// used only when calculating an exact non-loopy solution on a tree.
  /// mtc is the index of the message to calculate.
  /// Min-sum varient.
   virtual void SendOneMS(nat32 instance,const MessageSet & ms,nat32 mtc) = 0;

  /// This is called to calculate every output message but one from the
  /// in messages, used only when calculating an exact non-loopy solution
  /// on a tree.
  /// mti is the index of the message to not send. This is the compliment
  /// of SendOne, and will invariably be called after SendOne with mti==mtc
  /// for each instance of a function.
  /// Min-sum varient.
   virtual void SendAllButOneMS(nat32 instance,const MessageSet & ms,nat32 mti) = 0;

  /// This is called to calculate every output message, used exclusivly for
  /// in-exact loopy solving of graphs. The reality is the other two are 
  /// provided for completness - this is the one that is mostly used and 
  /// should therefore see the lions share of optimisation efforts.
  /// Min-sum varient.
   virtual void SendAllMS(nat32 instance,const MessageSet & ms) = 0;


  /// &nbsp; 
   virtual cstrconst TypeString() const = 0;
};

//------------------------------------------------------------------------------
// An internal use only class, this contains a set of instances of a function &
// the data for each instance, i.e. the incomming and outgoing messages.
// Provides an interface for getting pointers to particular messages and for doing
// message passing rounds on particular instances or all instances. Only
// contains one function, used for all instances, so the FactorGraph object will
// ushally contain one of these for each function with each one containning all
// instances of that function required.
class EOS_CLASS FunctionSet
{
 public:
  // Constructor, requires a function object to use and how many instances the
  // user wants of that function, it does the rest.
  // The Function object passed in is from that moment on owned by this object,
  // it will be deleted when this object is deleted.
  // Note that this constructor will do one hell of a malloc, and can fail as
  // a result, the active method is provided to check for this.
   FunctionSet(Function * func,nat32 instances = 1);
   
  // &nbsp;
   ~FunctionSet();

   
  // Returns true if the object has constructed properly, false if we have run
  // out of memory.
   bit Active() const {return data!=null<byte*>();}

  // Returns a pointer to the contained function object.
   Function * GetFunc() const {return func;}
   
  // Returns the number of instances.
   nat32 Instances() const {return instances;}

  // This sets all messages to equal probability, must be called before actual message
  // passing comences.
   void Flatline();
   
  // As above, for MS mode.
   void FlatlineLn();
   
  // Returns the message class for a given index, conveniant.
   const MessageClass & GetClass(nat32 index) const {return mc[index];}


  // Returns a pointer to the block of data associated with a particular 
  // message for a particular instance, the message being in for the 
  // function, out for the random variable to which its connected.
   void * ToFunc(nat32 instance,nat32 link) const {return data + instance*stride + in[link];}

  // Returns a pointer to the block of data associated with a particular 
  // message for a particular instance, the message being out for the 
  // function, in for the random variable to which its connected.
   void * FromFunc(nat32 instance,nat32 link) const {return data + instance*stride + out[link];}


  // This does a SendOne call with a given instance for the given link index.
   void SendOneSP(nat32 instance,nat32 link)
   {
    MessageSet ms(mc,data + instance*stride,in,out);
    func->SendOneSP(instance,ms,link);
   }
  
  // This does a SendButOne call with a given instance ignoring a given link index.
   void SendAllButOneSP(nat32 instance,nat32 link)
   {
    MessageSet ms(mc,data + instance*stride,in,out);
    func->SendAllButOneSP(instance,ms,link);
   }
  
  // This does a SendAll call on every instance contained within, loopy belief 
  // propagation will spend the vast majority of its time within this function.
   void SendAllSP(time::Progress * prog)
   {
    prog->Push();
     MessageSet ms(mc,data,in,out);
     byte * targ = data;
     for (nat32 i=0;i<instances;i++)
     {
      prog->Report(i,instances);
      ms.SetBase(targ);
      func->SendAllSP(i,ms);
      targ += stride;
     }
    prog->Pop();
   }
   
   
  // This does a SendOne call with a given instance for the given link index.
   void SendOneMS(nat32 instance,nat32 link)
   {
    MessageSet ms(mc,data + instance*stride,in,out);
    func->SendOneMS(instance,ms,link);
   }
  
  // This does a SendButOne call with a given instance ignoring a given link index.
   void SendAllButOneMS(nat32 instance,nat32 link)
   {
    MessageSet ms(mc,data + instance*stride,in,out);
    func->SendAllButOneMS(instance,ms,link);
   }
  
  // This does a SendAll call on every instance contained within, loopy belief 
  // propagation will spend the vast majority of its time within this function.
   void SendAllMS(time::Progress * prog)
   {
    prog->Push();
     MessageSet ms(mc,data,in,out);
     byte * targ = data;
     for (nat32 i=0;i<instances;i++)
     {
      prog->Report(i,instances);
      ms.SetBase(targ);
      func->SendAllMS(i,ms);
      targ += stride;
     }
    prog->Pop();
   }   
   
   
  // Returns the memory consumption of the class in bytes.
   nat32 Memory() const {return stride*instances + (sizeof(MessageClass)+sizeof(nat32)*2)*links + sizeof(*this);}


  // &nbsp;
   static cstrconst TypeString() {return "eos::inf::FunctionSet";}


 private:
  Function * func;
  nat32 instances;
  
  nat32 links;
  MessageClass * mc;

  byte * data;
  nat32 stride; // Gap in bytes between instances in the above buffer.

  nat32 * in;
  nat32 * out;
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
/// A simple function to be connected to a single Frequency variable, it provides
/// a probability frequency message to the variable to indicate the
/// probability of each possible label the variable can take.
class EOS_CLASS Distribution : public Function
{
 public:
  /// You provide how many possible labels exist for the variable as well as 
  /// how many instances to store frequency functions for. Note that you can have 
  /// more instances declared than specified here, it will just modulus the request
  /// instance by the quantity stored.
   Distribution(nat32 instances,nat32 labels);
   
  /// &nbsp;
   ~Distribution();


  /// Sets the probability frequency for a given label in a given instance. 
   void SetFreq(nat32 instance,nat32 label,real32 freq);
   
  /// Sets the probability frequency for a given instance.
   void SetFreq(nat32 instance,const real32 * freqs);

  /// Sets the probability frequency for a given instance.
   void SetFreq(nat32 instance,const math::Vector<real32> & freqs);


  /// Sets the -ln probability frequency for a given label in a given instance. 
   void SetCost(nat32 instance,nat32 label,real32 cost);
   
  /// Sets the -ln probability frequency for a given instance.
   void SetCost(nat32 instance,const real32 * costs);

  /// Sets the -ln probability frequency for a given instance.
   void SetCost(nat32 instance,const math::Vector<real32> & costs);


  /// &nbsp;
   nat32 Links() const;
  
  /// &nbsp;
   void LinkType(nat32 ind,MessageClass & out) const;


  /// &nbsp;
   void ToSP();

  /// &nbsp;
   void SendOneSP(nat32 instance,const MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneSP(nat32 instance,const MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllSP(nat32 instance,const MessageSet & ms);


  /// &nbsp;
   void ToMS();

  /// &nbsp;
   void SendOneMS(nat32 instance,const MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneMS(nat32 instance,const MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllMS(nat32 instance,const MessageSet & ms);


  /// &nbsp; 
   cstrconst TypeString() const;

   
 private:
  nat32 labels;
  nat32 instances;
  
  struct Prob
  {
   bit ms; // If false the below is a probability, if true a -ln probability.
   real32 val;
  };
  
  Prob * data; // labels*instances in size, [labels*ins + lab].
};

//------------------------------------------------------------------------------
/// A joint distribution function, this takes a variable number of 
/// frequency probability functions as specified by construction, is given a
/// table that indicates the probability of any given combination of variables
/// If you set the number of variables to 1 it becomes equivalent to the 
/// Distribution function.
/// Note: ToMS function does no normalisation, it dam well should, before conversion only.
class EOS_CLASS JointDistribution : public Function
{
 public:
  /// You provide how many links to variables exist and how many instances of 
  /// the distribution to store. You can declare more instances than stored in
  /// the function as it uses the requested instance modulus the number of 
  /// instances stored here.
   JointDistribution(nat32 instances,nat32 links);
   
  /// &nbsp;
   ~JointDistribution();


  /// Sets the number of labels for all the links, must be Links() in size,
  /// remember that this creates a data structure of real32's for you to 
  /// fill in, in Links()+1 dimensions with these being the sizes of those
  /// dimensions. (Instances being the final dimension.) In other words,
  /// don't make it too big!
  /// When you call this you reset all distributions, must be called at
  /// least once before *any* use, including linking in so it can verify
  /// the label counts.
   void SetLabels(nat32 * labSize);
   
  /// &nbsp;
   void SetLabels(const math::Vector<nat32> & labSize);
   
   
  /// Returns a reference to the probability of the given combination of
  /// labels, so you can both set and read it. You also have to provide 
  /// which instance you want. labInd must be Links() in size.
   real32 & Get(nat32 * labInd,nat32 instance);
   
  /// &nbsp;
   real32 & Get(const math::Vector<nat32> & labInd,nat32 instance);
   
  
  /// If you don't want to use the Get interface a more conveniant techneque
  /// is to overload the Evaluate method and call Build. The Evaluate method
  /// must return the frequency for the paramters given in an array of labels.
   virtual real32 Evaluate(nat32 * labs,nat32 instance);
  
  /// If you don't want to use the Get interface a more conveniant techneque
  /// is to overload the Evaluate method and call Build. The Build method uses
  /// Evaluate for every combination to fill in the joint distribution structure.
  /// (Internally it uses the Get method - it just manages the trouble of looping 
  /// over everything for you.)
   void Build();

 
  /// &nbsp;
   nat32 Links() const;
  
  /// &nbsp;
   void LinkType(nat32 ind,MessageClass & out) const;


  /// &nbsp;
   void SendOneSP(nat32 instance,const MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneSP(nat32 instance,const MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllSP(nat32 instance,const MessageSet & ms);


  /// &nbsp;
   void ToMS();

  /// &nbsp;
   void SendOneMS(nat32 instance,const MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneMS(nat32 instance,const MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllMS(nat32 instance,const MessageSet & ms);


  /// &nbsp; 
   cstrconst TypeString() const;


 private:
  nat32 links;
  nat32 instances;
  nat32 * labels; // links in size, number of labels for each dimension.
  nat32 * stride; // cumulative multipication of above.
  
  real32 * data;
  
  real32 Marginalise(nat32 link,nat32 skipLink,nat32 skipValue,const MessageSet & ms,real32 mmsd,real32 * targ);
  real32 MarginaliseMS(nat32 link,nat32 skipLink,nat32 skipValue,const MessageSet & ms,real32 mmsd,real32 * targ);
};

//------------------------------------------------------------------------------
/// A join distribution for just 2 variables, as a common case having a seperate
/// class for this can provide further optimisation. This additionally has 
/// better suport for various things. Normalisation is applied before use of the
/// obtained data.
class EOS_CLASS DualDistribution : public Function
{
 public:
  /// You provide the number of instances and the label counts for the pairings 
  /// of variables. The ms param should be left false to use probability, true 
  /// to use costs. The distribution must then be wholly entered in this form.
   DualDistribution(nat32 instances,nat32 labels0,nat32 labels1, bit ms = false);

  /// &nbsp;
   DualDistribution();


  /// Returns a reference to a specific probability/cost, so you can set it.
   real32 & Val(nat32 instance,nat32 lab0,nat32 lab1);


  /// &nbsp;
   nat32 Links() const;

  /// &nbsp;
   void LinkType(nat32 ind,MessageClass & out) const;


  /// &nbsp;
   void ToSP();

  /// &nbsp;
   void SendOneSP(nat32 instance,const MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneSP(nat32 instance,const MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllSP(nat32 instance,const MessageSet & ms);


  /// &nbsp;
   void ToMS();

  /// &nbsp;
   void SendOneMS(nat32 instance,const MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneMS(nat32 instance,const MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllMS(nat32 instance,const MessageSet & ms);


  /// &nbsp; 
   cstrconst TypeString() const;


 private:
  nat32 instances;
  nat32 labels0;
  nat32 labels1;
  bit ms;

  real32 * data; // Indexed by instances,labels1,labels0, in that order.
};

//------------------------------------------------------------------------------
/// This sets two random variables as equal, using a potts term. You set the 
/// ratio between the probability of equality to the probability assigned to 
/// each other label. i.e. a value of 1 provides no constraint, a value of 2 
/// means that equality is twice as likelly as any other possible labeling.
/// Note that a value less than 1 for the ratio will not work in Min-Sum mode,
/// but will work for Sum-Product mode. A value of 0 will never work.
class EOS_CLASS EqualPotts : public Function
{
 public:
  /// You give it the number of discrete labels for the pair of random variables
  /// it will be linking.
   EqualPotts(nat32 instances,nat32 labels);

  /// &nbsp;
   ~EqualPotts();


  /// Sets the parameters for the pdf. You must call this at least once before use
  /// for each instance.
  /// \param instance Which instance to set the parameters for.
  /// \param equal Probability of two equal labels over probability of 
  ///              two labels that are not equal.
  ///              They all default to 1, i.e. no affect.
   void Set(nat32 instance,real32 equal);


  /// &nbsp;
   nat32 Links() const;

  /// &nbsp;
   void LinkType(nat32 ind,MessageClass & out) const;


  /// &nbsp;
   void SendOneSP(nat32 instance,const MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneSP(nat32 instance,const MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllSP(nat32 instance,const MessageSet & ms);


  /// &nbsp;
   void ToMS();

  /// &nbsp;
   void SendOneMS(nat32 instance,const MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneMS(nat32 instance,const MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllMS(nat32 instance,const MessageSet & ms);


  /// &nbsp; 
   cstrconst TypeString() const;


 private:
  nat32 instances;
  nat32 labels;
  
  real32 * inst; // equality probability ratio for each instance.
};

//------------------------------------------------------------------------------
/// This sets two random variables as equal, using a robust and fast (non) PDF.
/// The 'PDF' simply consists of a gaussian with a minimum probability, i.e
/// pdf = Max(Gaussian(sd,difference),corruption). This obviously integrates 
/// to infinity, so it is not a real PDF, but it works in practise, with the 
/// important bonus of being fast to calculate.
class EOS_CLASS EqualGaussian : public Function
{
 public:
  /// You give it the number of discrete labels for the pair of random variables
  /// it will be linking.
   EqualGaussian(nat32 instances,nat32 labels);

  /// &nbsp;
   ~EqualGaussian();
   
  
  /// Sets the parameters for the pdf. You must call this at least once before use
  /// for each instance.
  /// \param instance Which instance to set the parameters for.
  /// \param corruption Sets the base line probability of an arbitary value.
  ///                   If you set this to 0.0 you switch off an entire chunk of 
  ///                   the algorithm and speed things up, any other value has 
  ///                   equal hit however. This is set as a percentage of the 
  ///                   maximum value of the gaussian component.
  /// \param sd Sets the standard deviation of the gaussian.
  ///           Setting this low speeds things up.
  /// \param range Sets how many sd's of range to put the cutoff at. Setting this
  ///              low speeds things up with a loss of accuracy due to ignoring 
  ///              weaker terms. Note this if you set this higher than the range
  ///              implied by corruption then it is ignored, only really matters
  ///              when corruption is low/0.0.
   void Set(nat32 instance,real32 corruption,real32 sd,real32 range = 2.0);


  /// &nbsp;
   nat32 Links() const;
  
  /// &nbsp;
   void LinkType(nat32 ind,MessageClass & out) const;


  /// &nbsp;
   void SendOneSP(nat32 instance,const MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneSP(nat32 instance,const MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllSP(nat32 instance,const MessageSet & ms);


  /// &nbsp;
   void ToMS();

  /// &nbsp;
   void SendOneMS(nat32 instance,const MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneMS(nat32 instance,const MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllMS(nat32 instance,const MessageSet & ms);


  /// &nbsp; 
   cstrconst TypeString() const;


 private:
  nat32 instances;
  nat32 labels;
  
  struct Inst
  {  
   bit doCorruption; // true if corruption!=0.0.
   real32 corruption;
  
   real32 sd;
   nat32 range; // For the gaussian part go from -range to +range.
   real32 * gauss; // A lookup table of gaussian values minus corruption. range+1 in size, to include 0.
  };
  
  Inst * inst;
};

//------------------------------------------------------------------------------
/// This sets two random variables as equal, using a robust and fast (non) PDF.
/// The 'PDF' simply consists of a laplacian with a minimum probability, i.e
/// pdf = Max(Laplace(sd,difference),corruption). This obviously integrates 
/// to infinity, so it is not a real PDF, but it works in practise, with the 
/// important bonus of being fast to calculate.
class EOS_CLASS EqualLaplace : public Function
{
 public:
  /// You give it the number of discrete labels for the pair of random variables
  /// it will be linking.
   EqualLaplace(nat32 instances,nat32 labels);

  /// &nbsp;
   ~EqualLaplace();
   
  
  /// Sets the parameters for the pdf. You must call this at least once before use
  /// for each instance.
  /// \param instance Which instance to set the parameters for.
  /// \param corruption Sets the base line probability of an arbitary value.
  ///                   If you set this to 0.0 you switch off an entire chunk of 
  ///                   the algorithm and speed things up, any other value has 
  ///                   equal hit however. This is set as a percentage of the 
  ///                   maximum value of the laplacian component.
  /// \param dev Sets the deviation parameter of the laplacian distribution.
  ///            Setting this low speeds things up.
  /// \param range Sets how many devs of range to put the cutoff at. Setting this
  ///              low speeds things up with a loss of accuracy due to ignoring 
  ///              weaker terms. Note this if you set this higher than the range
  ///              implied by corruption then it is ignored, only really matters
  ///              when corruption is low or zero.
   void Set(nat32 instance,real32 corruption,real32 dev,real32 range = 2.0);
   
  /// This sets the parameters of an instance in -ln space, max being the maximum cost 
  /// and mult being the multiplier to convert difference into cost.
   void SetMS(nat32 instance,real32 mult,real32 max);


  /// &nbsp;
   nat32 Links() const;
  
  /// &nbsp;
   void LinkType(nat32 ind,MessageClass & out) const;


  /// &nbsp;
   void ToSP();

  /// &nbsp;
   void SendOneSP(nat32 instance,const MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneSP(nat32 instance,const MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllSP(nat32 instance,const MessageSet & ms);


  /// &nbsp;
   void ToMS();

  /// &nbsp;
   void SendOneMS(nat32 instance,const MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneMS(nat32 instance,const MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllMS(nat32 instance,const MessageSet & ms);


  /// &nbsp; 
   cstrconst TypeString() const;


 private:
  nat32 instances;
  nat32 labels;
  
  struct Inst
  {
   bit ms; // true if it contains -ln values, false for the normal variety.
   real32 rMult;

   bit doCorruption; // true if corruption!=0.0.
   real32 corruption;

   real32 sd;
   nat32 range; // For the laplacian part go from -range to +range.
   real32 * laplace; // A lookup table of laplacian values minus corruption. range+1 in size, to include 0.
  };
  
  Inst * inst;
};

//------------------------------------------------------------------------------
/// Sets two random variables as equal, using a Circular Normal Distribution.
/// A minimum cost value is also provided as a multiplier of the maximum value
/// of the distribution. It assumes that the labels given are equally 
/// distributed over 360 degrees.
class EOS_CLASS EqualVonMises : public Function
{
 public:
  /// &nbsp;
   EqualVonMises(nat32 instances,nat32 labels);

  /// &nbsp;
   ~EqualVonMises();


  /// Sets the K value and cutMult, on a per instance basis.
  /// cutMult is the minimum probability as a multiplier of the maximum.
  /// Must be called for all instances as there are no defaults.
   void Set(nat32 instance,real32 k,real32 cutMult);


  /// &nbsp;
   nat32 Links() const;
  
  /// &nbsp;
   void LinkType(nat32 ind,MessageClass & out) const;


  /// &nbsp;
   void ToSP();

  /// &nbsp;
   void SendOneSP(nat32 instance,const MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneSP(nat32 instance,const MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllSP(nat32 instance,const MessageSet & ms);


  /// &nbsp;
   void ToMS();

  /// &nbsp;
   void SendOneMS(nat32 instance,const MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneMS(nat32 instance,const MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllMS(nat32 instance,const MessageSet & ms);


  /// &nbsp; 
   cstrconst TypeString() const; 


 private:
  nat32 instances;
  nat32 labels;
  
  real32 diffMult; // Converts a label difference into an angular difference, ready for use.
  
  struct Para
  {
   real32 k; // k value for each instance. No conversion required for sp/ms. 
   real32 minProb; // Multiplier of k where the cutoff exists.
   real32 maxCost; // Maximum cost in ms mode. Equalivalent to above - conversion on set.
  };
  Para * para; 
};

//------------------------------------------------------------------------------
/// This sets random variable 0 to be equal to random variable 1 minus random 
/// variable 2. An almost-hard constraint, the factor function being 1 when it is
/// satisifed and a user set value when it is not. Given the 3 random variables 
/// as a,b and c then the function is a = (b-c)+labels-1; This gives a range of 
/// labels*2-1 for the first random variable.
class EOS_CLASS DifferencePotts : public Function
{
 public:
  /// You give it the number of discrete labels for the last two random variables
  /// of the tripplet it will be linking, the first random variable will then
  /// have labels*2-1 discrete possibilities.
   DifferencePotts(nat32 labels,real32 lambda = 0.0);

  /// &nbsp;
   ~DifferencePotts();


  /// &nbsp;
   nat32 Links() const;
  
  /// &nbsp;
   void LinkType(nat32 ind,MessageClass & out) const;


  /// &nbsp;
   void SendOneSP(nat32 instance,const MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneSP(nat32 instance,const MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllSP(nat32 instance,const MessageSet & ms);


  /// &nbsp;
   void ToMS();

  /// &nbsp;
   void SendOneMS(nat32 instance,const MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneMS(nat32 instance,const MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllMS(nat32 instance,const MessageSet & ms);


  /// &nbsp; 
   cstrconst TypeString() const;


 private:
  int32 labels;
  real32 lambda;
};

//------------------------------------------------------------------------------
/// This abstracts the idea of a Potts function to an arbitary number of 
/// links and an arbitary true/false function.
/// (Well, I say Potts, but it is more capable than that.)
/// It does this at the expense of the sum-product implimentation - this can
/// only be used for min-sum based solutions.
/// A user of this class specifies the cost of a mis-match, and then specifies 
/// all combinations that do not suffer that mis-match cost.
/// Runtime is linear in the number of exception combinations, so the key is in
/// keeping these to a minimum.
class EOS_CLASS GeneralPotts : public Function
{
 public:
  /// Provided with the number of links then an array indicating how many 
  /// labels are used for each link. The array must be links size.
   GeneralPotts(nat32 instances,nat32 links,nat32 * labels);

  /// &nbsp;
   ~GeneralPotts();


  /// This sets the cost associated with the vast majority of combinations,
  /// it must be larger than any given exception for that exception to 
  /// actually matter. They all default to 1.
  /// Remember that a cost is simply a negative ln probability.
   void SetCost(nat32 instance,real32 cost);
  
  /// This adds an exception, a particular combination that has a cost less 
  /// than the ushall cost. If you add the same exception twice only the 
  /// one with the least cost will remain. Runtime is linear in the number 
  /// of exceptions, so the fewer the better.
  /// \param instance The instance to which the exception applies.
  /// \param cost The cost of this exception, should be less than the 
  ///             instances cost, as otherwise will have no affect.
  /// \param label An array, links in size, of the label for each link to 
  ///              which this exception applies.
   void SetException(nat32 instance,real32 cost,nat32 * label);
   
  /// Returns how many exceptions have been entered with SetException.
   nat32 ExceptionCount() const;


  /// &nbsp;
   nat32 Links() const;
  
  /// &nbsp;
   void LinkType(nat32 ind,MessageClass & out) const;


  /// &nbsp;
   void SendOneSP(nat32 instance,const MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneSP(nat32 instance,const MessageSet & ms,nat32 mti);

  /// &nbsp;
   void SendAllSP(nat32 instance,const MessageSet & ms);  


  /// &nbsp;
   void ToMS();

  /// &nbsp;
   void SendOneMS(nat32 instance,const MessageSet & ms,nat32 mtc);

  /// &nbsp;
   void SendAllButOneMS(nat32 instance,const MessageSet & ms,nat32 mti);   

  /// &nbsp;
   void SendAllMS(nat32 instance,const MessageSet & ms);


  /// &nbsp; 
   cstrconst TypeString() const {return "eos::inf::GeneralPotts";}


 private:
  nat32 instances; 
  nat32 links;
  nat32 * labels; // links size.
  
  real32 * cost; // instances size.

  // We collect the exceptions in the simplest way possible - by using an 
  // expanding data block. Each exception is encoded as instance(nat32), 
  // cost(real32) then finally links labels(nat32)...
  // (You can work out how many have been stored by division by exception size.)
   data::Buffer excBuf;

  // This contains an exception, happens to match the storage structure used by exeBuf.
   struct Exc
   {
    nat32 instance;
    nat32 cost;

    nat32 & Label(nat32 i) {return ((nat32*)(void*)(this+1))[i];}
    const nat32 & Label(nat32 i) const {return ((nat32*)(void*)(this+1))[i];}
   };
   
  // This runtime-sized array contains the all the Exe objects once proccessing
  // has started, for efficiency reasons. Sorted.
   ds::ArrayRS<Exc> exc;

  // Indexing for the above array by instance, contains the starting index and 
  // number of items for each instance...
   struct InstDet
   {
    nat32 start;
    nat32 size;
   };
   ds::Array<InstDet> excInd;
   
  // Helper function, used for sorting 'em...
   static bit SortFunc(Exc * lhs,Exc * rhs,nat32 * links)
   {
    if (lhs->instance!=rhs->instance) return lhs->instance < rhs->instance;
    for (nat32 i=0;i<*links;i++)
    {
     if (lhs->Label(i)!=rhs->Label(i)) return lhs->Label(i) < rhs->Label(i);
    }
    return false;
   }
   
  // Helper data structure, provides a suitably sized array for processing 
  // things...
   struct Node
   {
    Frequency in;
    Frequency out;
    real32 min; // Minimum value of freq.
   };
   Node * n;
};

//------------------------------------------------------------------------------
 };
};
#endif
