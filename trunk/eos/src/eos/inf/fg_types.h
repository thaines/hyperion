#ifndef EOS_INF_FG_TYPES_H
#define EOS_INF_FG_TYPES_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file fg_types.h
/// Provides the message types that the factor graph solver fires around.

/// \namespace eos::inf
/// Provides inference capabilities. This currently consists of a factor graph
/// solver that is completly general in its capabilites, suporting every 
/// feature imaginable. Well, at least every imaginable feature that I have had
/// the time and will to impliment at any rate.
/// In addition to this various extensions are provided, for managing specific 
/// classes/types/expressions of problem.

#include "eos/types.h"
#include "eos/mem/functions.h"
#include "eos/math/functions.h"
#include "eos/io/inout.h"

namespace eos
{
 namespace inf
 {
//------------------------------------------------------------------------------
/// Represents a message v-table, the message itself being passed between two 
/// nodes in the factor graph system.
/// Implimented as a C-style class that seperates the content from the v-table,
/// because we will be dealing with millions of 'em and don't want to store a 
/// v-table pointer with each instance, but equally want the flexability of not
/// hardcoding the types. Note that a size parameter is also provided, so the
/// number of labels in a frequency function or the number of gaussian functions
/// in a gaussian mixture for instance, are specified in the 'class' type, 
/// rather than on construction of the class. This allows large blocks of memory
/// to be declared to store millions of messages tightly packed without the
/// extra 8 bytes of these two items per entry. The user of
/// a message type will know the type in advanced and be able to use a proxy 
/// object, which contains a pointer to the actual data, to expose a suitable interface
/// to the message content. All proxy objects are expected to support construction via
/// a constructor which takes (const MessageClass & mc,void * data) as input.
struct EOS_CLASS MessageClass
{
 /// Equality of MessageClass objects is done bytewise - they must be 
 /// identical in every way. This is used for checking that each random
 /// variable is connected to only one message type.
  bit operator == (const MessageClass & rhs) const
  {
   return mem::Compare(this,&rhs)==0;
  }
  
 /// &nbsp;
  bit operator != (const MessageClass & rhs) const {return !(*this==rhs);}


 /// Used it indicate the scale of the object, i.e. for the Frequency type 
 /// its the number of labels. Can be left unused, but is often useful.
  nat32 scale;


 /// Must return the size of the object, so the correct amount of memory can
 /// be allocated. Assignment should be possible with a memcpy using this
 /// many bytes.
  nat32 (*Size)(const MessageClass & mc);
 
 /// The constructor as such, must set the message representation to 'equal
 /// probability of any'. This works when handling probabilities.
 /// (Sets all to 1.)
  void (*Flatline)(const MessageClass & mc,void * obj);

 /// The constructor as such, must set the message representation to 'equal
 /// probability of any'.
 /// This works when handling the negative ln of probabilities.
 /// (Sets all to 0.)
  void (*FlatlineLn)(const MessageClass & mc,void * obj);
  
 /// This should perform objA = objB * objC.
  void (*Mult)(const MessageClass & mc,void * objA,void * objB,void * objC);

 /// This should perform objA *= objB.
  void (*InplaceMult)(const MessageClass & mc,void * objA,void * objB);

 /// This should perform objA += objB.
  void (*InplaceAdd)(const MessageClass & mc,void * objA,void * objB);
  
 /// This should perform objA -= objB.
  void (*InplaceNeg)(const MessageClass & mc,void * objA,void * objB);
  
 /// This should convert from -Ln(probability) to just probability. Used for final output.
 /// Must also normalise the output probability. Must consider that the -ln values will
 /// have been zero meaned, with all the annoying consequences this brings.
  void (*FromNegLn)(const MessageClass & mc,void * obj);
  
 /// This should normalise the message in question. (If normalisation is
 /// automatic then this can do nothing, but a valid function pointer must
 /// still be provided.)
  void (*Norm)(const MessageClass & mc,void * obj);
  
 /// The -ln equivalent to Norm, either zero mean or offset such that lowest value is zero.
  void (*Drop)(const MessageClass & mc,void * obj);  
};

//------------------------------------------------------------------------------
/// Empty type, provided to group the Message instance proxy classes in the
/// documentation, contributes nothing in terms of functionality.
class EOS_CLASS Message
{};

//------------------------------------------------------------------------------
/// The proxy message type for a frequency function, as used to represent
/// discrete random variables.
class EOS_CLASS Frequency : public Message
{
 public:
  /// This creates a MessageClass suitable for use with this object, you provide
  /// how many labels you want to be avaliable.
   static void MakeClass(nat32 labels,MessageClass & out);
 
  /// If constructed using this do not use till a correctly constructed one has been assigned.
   Frequency()
   {}
 
  /// &nbsp;
   Frequency(const MessageClass & mcc,void * obj)
   :mc(&mcc),data(static_cast<real32*>(obj))
   {}
  
  /// &nbsp;
   Frequency(const Frequency & rhs)
   :mc(rhs.mc),data(rhs.data)
   {}


  /// &nbsp;
   Frequency & operator = (const Frequency & rhs)
   {
    mc = rhs.mc;
    data = rhs.data;
    return *this;
   }


  /// Accessor for the probability of each label, 0..labels()-1.
   real32 & operator[] (nat32 i) {return data[i];}

  /// Accessor for the probability of each label, 0..labels()-1.
   const real32 & operator[] (nat32 i) const {return data[i];}


  /// Component-wise multipication.
   Frequency & operator *= (const Frequency & rhs)
   {
    for (nat32 i=0;i<mc->scale;i++) data[i] *= rhs.data[i];
    return *this;
   }
  
  /// Normalises such that the sum of the frequencies is 1.
   void Norm()
   {
    real32 sum = 0.0;
    for (nat32 i=0;i<mc->scale;i++) sum += data[i];
    sum = 1.0/sum;
    for (nat32 i=0;i<mc->scale;i++) data[i] *= sum;
   }
   
  /// Zero means the frequencies.
   void ZeroMean()
   {
    real32 sum = 0.0;
    for (nat32 i=0;i<mc->scale;i++) sum += data[i];
    sum /= real32(mc->scale);
    for (nat32 i=0;i<mc->scale;i++) data[i] -= sum;
   }
   
  // Ajusts the frequencies such that the lowest value is 0.0.
  /// (Called drop as this ushally reduces the numbers - 
  /// used as a normalisation scheme.)
   void Drop()
   {
    real32 mv = data[0];
    for (nat32 i=1;i<mc->scale;i++) mv = math::Min(mv,data[i]);
    for (nat32 i=0;i<mc->scale;i++) data[i] -= mv;
    /*real32 sum = 0.0;
    for (nat32 i=0;i<mc->scale;i++) sum += data[i];
    sum /= real32(mc->scale);
    for (nat32 i=0;i<mc->scale;i++) data[i] -= sum;*/
   }


  /// Returns the index of the label with the greatest frequency.
  /// If multiple labels are the maximum then the first one found
  /// gets it.
   nat32 Highest() const
   {
    nat32 ret = 0;
    real32 best = data[0];
     for (nat32 i=1;i<mc->scale;i++)
     {
      if (data[i]>best)
      {
       best = data[i];
       ret = i;
      }
     }
    return ret;
   }
   
  /// Returns the index of the label with the lowest 'frequency'.
  /// Useful if dealing with -ln results directly.
   nat32 Lowest() const
   {
    nat32 ret = 0;
    real32 best = data[0];
     for (nat32 i=1;i<mc->scale;i++)
     {
      if (data[i]<best)
      {
       best = data[i];
       ret = i;
      }
     }
    return ret;
   }   
   
  /// Returns the probability gap between the highest and second highest probability,
  /// can make a reasonable confidence measure.
   real32 Confidence() const
   {
    real32 best[2];
    best[0] = data[0];
    best[1] = data[1];
    if (best[0]<best[1]) Swap(best[0],best[1]);
    
    for (nat32 i=2;i<mc->scale;i++)
    {
     if (data[i]>best[1])
     {
      if (data[i]>best[0])
      {
       best[1] = best[0];
       best[0] = data[i];
      }
      else
      {
       best[1] = data[i]; 
      }
     }
    }
    
    return best[0] - best[1];
   }
   
   
  /// An interpolating version of Highest(), fits a parabola arround the
  /// Highest() value and returns the index that maximises.
  /// Only appropriate for 1D continuous variables that have been discretised.
  /// This is not a mathematically sound trick, but it works and thats all that matters.
   real32 Maximum() const
   {
    // Get the highest, moving away from edges so we have values above and below to fit to...
     nat32 highest = Highest();
     nat32 centre = highest;
     if (centre==0) centre = 1;
     else if (centre==Labels()-1) centre = Labels()-2;
     
    // Fit the parabola and find the maxima...
     real32 a = data[centre];
     real32 c = 0.5*(data[centre-1]+data[centre+1]) - a;
     real32 b = data[centre+1] - a - c;
     
     if (!math::Equal(c,real32(0.0)))
     {
      real32 offset = -b/(2.0*c);
      if (centre==highest)
      {
       if (math::Abs(offset)<=0.5) return centre + offset;
      }
      else
      {
       if (highest==0)
       {
        if ((offset<=-0.5)&&(offset>=-1.5)) return centre + offset;
       }
       else
       {
        if ((offset<=1.5)&&(offset>=0.5)) return centre + offset;
       }
      }
     }
     
    return highest; // Fallback for if things go wrong.
   }


  /// Returns the number of labels. In principal you shouldn't have to 
  /// use this, but there allways exceptions and its often neater this way.
   nat32 Labels() const {return mc->scale;}


 private:
  const MessageClass * mc;
  real32 * data;
  
  // The functions used by the MessageClass creator...
   static nat32 Size(const MessageClass & mc);
   static void Flatline(const MessageClass & mc,void * obj);
   static void FlatlineLn(const MessageClass & mc,void * obj);
   static void Mult(const MessageClass & mc,void * objA,void * objB,void * objC);
   static void InplaceMult(const MessageClass & mc,void * objA,void * objB);
   static void InplaceAdd(const MessageClass & mc,void * objA,void * objB);
   static void InplaceNeg(const MessageClass & mc,void * objA,void * objB);
   static void FromNegLn(const MessageClass & mc,void * obj);
   static void Norm(const MessageClass & mc,void * obj);   
   static void Drop(const MessageClass & mc,void * obj);   
};

//------------------------------------------------------------------------------
// The proxy message type for a 1D density function, as used to represent
// continuous random variables. This is based on gaussian mixtures.

// Code me!

//------------------------------------------------------------------------------
// The proxy message type for a 2D density function, as used to represent
// continuous random vectors. This is based on gaussian mixtures.

// Code me!

//------------------------------------------------------------------------------
// The proxy message type for a 3D density function, as used to represent
// continuous random vectors. This is based on gaussian mixtures.

// Code me!

//------------------------------------------------------------------------------
// The proxy message type for a 1D density function, as used to represent
// continuous random variables. This is based on mixtures of circular normal
// (von Mises) distributions.

// Code me!

//------------------------------------------------------------------------------
 };
};
//------------------------------------------------------------------------------
// IO stream integration for the above classes, just text output for logging...
namespace eos
{
 namespace io
 {


  template <typename T>
  inline T & StreamWrite(T & lhs,const inf::Frequency & rhs,Text)
  {
   lhs << "<";
   for (nat32 i=0;i<rhs.Labels();i++)
   {
    lhs << rhs[i];
    if (i+1!=rhs.Labels()) lhs << ",";
   }
   lhs << ">";
   return lhs;
  }


 };
};

//------------------------------------------------------------------------------
#endif
