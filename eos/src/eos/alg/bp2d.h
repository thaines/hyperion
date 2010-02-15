#ifndef EOS_ALG_BP2D_H
#define EOS_ALG_BP2D_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file bp2d.h
/// Provides several loopy belief propagation implimentations, for solving
/// problems formulated in just the right way on 2D grids of samples. All
/// algorithms were implimented based on 'Efficient Belief Propagation for
/// Early Vision' by Felzenszwalb & Huttenlocher. The source code provided by
/// the authors was consulted when it didn't work correctly, though not used
/// for the initial implimentation.

#include "eos/types.h"
#include "eos/svt/var.h"
#include "eos/svt/field.h"
#include "eos/math/mat_ops.h"
#include "eos/time/progress.h"
#include "eos/mem/alloc.h"
#include "eos/ds/arrays2d.h"

namespace eos
{
 namespace alg
 {
//------------------------------------------------------------------------------
/// This defines a message calculator interface that is passed into BP
/// algorithms. It is responsible for calculating both the messages and final
/// belief vector. Implimented like this as major optimisations can be obtained
/// for special cost functions. Also has to suport a hierachical system where
/// the grid size is halfed at each level. It is highly recomended that any
/// reasonable implimentation cache the results of the data cost function.
/// This here is simply a class from which the other variants should inherit,
/// it provides no actual implimentation.
///
/// An implimentor of this class has to represent two functions, D and V.
/// D is the cost of assigning a particular label to a particular node.
/// L is the cost of two neighbours label combination, and must be symetric and
/// independent of the nodes themselves. The system as a whole assignes the
/// labeling that minimises the sum of these two functions, for every node and
/// every 4-way pair of nodes in the grid. The result is not necesarilly the best,
/// but its provably not that far away.
///
/// - PT is an entity type passed to both cost functions, so they can use it to generate
///   there costs. In practise it will ushally be a pointer to the class that is using
///   an instance of this object so the static member cost functions can get at it.
template <typename PT>
class EOS_CLASS MsgBP2D
{
 public:
  typedef PT passthroughType;
 
  /// The constructor, including for child types, is allways given
  /// the same parameters, whether the interface implimentors like it or not.
  /// \param data The pass-through object, stored in this object so it
  ///             can be used by the cost functions.
  /// \param width The width of the grid.
  /// \param height The height of the grid.
  /// \param labels The number of labels that should be considered for assignment to each node.
  /// \param ms True if multiscale behaviour is expected, false otherwise.
  ///           If false level will allways be 0 for calls to Msg, if true
  ///           it can change. This is passed in so it can cache accordingly.
   MsgBP2D(const PT & data,nat32 width,nat32 height,nat32 labels,bit ms) {}
 
 
  /// Given the 3 relevent in messages this must produce the relevent out
  /// message.
  /// \param x The x coordinate of the node calculating the message.
  /// \param y The y coordinate of the node calculating the message.  
  /// \param level The level to interpret this call. Each level halfs the
  ///              resolution of the grid with level 0 being full resolution.
  ///              Level can go all the way upto the point when one of the
  ///              dimensions is 1, i.e. next level and one of them would be zero.
  /// \param in The 3 messages that go into the creation of this message.
  /// \param out The output message that it calculates.
   void Msg2D(nat32 x,nat32 y,nat32 level,real32 * in[3],real32 * out);
           
  /// Given the 4 messages passed to this node this should return the label
  /// that should be choosen. Pritty simple.
  /// The optional out pointer if given should be filled in with the final
  /// costs, only used for analysis.
   nat32 Label(nat32 x,nat32 y,real32 * in[4],real32 * out);
};

//------------------------------------------------------------------------------
/// The most basic of possible MsgBP2D classes, is simply given a D and V function
/// from which it calculates messages. It does cache both given functions however.
/// This is of course slow as its O(labels^2) per message. Different 
/// implimentations provided for far improved runtimes, but only for particular
/// cost metrics.
/// Templated on:
/// - PT As in MsgBP2D.
/// - D is the data cost function, and is the cost of assigning a particular label to a
///   particular entry in the grid.
/// - V is the discontinuity cost function, and is the cost of assigning a particular
///   label combination to two neighbours.
template <typename PT,
          real32 D(const PT & data,nat32 x,nat32 y,nat32 label),
          real32 V(const PT & data,nat32 lab1,nat32 lab2)>
class EOS_CLASS AnyMsgBP2D : public MsgBP2D<PT>
{
 public:
  AnyMsgBP2D(const PT & d,nat32 w,nat32 h,nat32 l,bit ms)
  :MsgBP2D<PT>(d,w,h,l,ms),data(d),width(w),height(h),labels(l)
  {
   // The D cache...
    nat32 levels = math::Min(math::TopBit(width),math::TopBit(height));
    cacheD = mem::Malloc<real32*>(levels);
    
    cacheD[0] = mem::Malloc<real32>(width*height*labels);
    for (nat32 y=0;y<height;y++)
    {
     for (nat32 x=0;x<width;x++)
     {
      for (nat32 lab=0;lab<labels;lab++) cacheD[0][labels*(y*width + x) + lab] = D(data,x,y,lab);
     }
    }
    
    if (ms)
    {
     for (nat32 lev=1;lev<levels;lev++)
     {
      cacheD[lev] = mem::Malloc<real32>((width>>lev)*(height>>lev)*labels);
      for (nat32 y=0;y<(height>>lev);y++)
      {
       for (nat32 x=0;x<(width>>lev);x++)
       {
        for (nat32 lab=0;lab<labels;lab++)
        {
         cacheD[lev][labels*(y*(width>>lev) + x) + lab] = 
               cacheD[lev-1][labels*(((y<<1)+0)*(width>>(lev-1)) + ((x<<1)+0)) + lab] +
               cacheD[lev-1][labels*(((y<<1)+1)*(width>>(lev-1)) + ((x<<1)+0)) + lab] +
               cacheD[lev-1][labels*(((y<<1)+0)*(width>>(lev-1)) + ((x<<1)+1)) + lab] +
               cacheD[lev-1][labels*(((y<<1)+1)*(width>>(lev-1)) + ((x<<1)+1)) + lab];
        }
       }
      }
     }
    }
    else
    {
     for (nat32 l=1;l<levels;l++) cacheD[l] = null<real32*>();
    }
   
   // The V cache...
    cacheV = mem::Malloc<real32>(labels*labels);
    for (nat32 l2=0;l2<labels;l2++)
    {
     for (nat32 l1=0;l1<labels;l1++) cacheV[labels*l2 + l1] = V(data,l1,l2);
    }
    
   // A tempory variable used within methods...
    temp.SetSize(labels);
  }
  
  ~AnyMsgBP2D()
  {
   nat32 levels = math::Min(math::TopBit(width),math::TopBit(height));
   for (nat32 i=0;i<levels;i++) mem::Free(cacheD[i]);
   mem::Free(cacheD);
   mem::Free(cacheV);
  }
  
  void Msg(nat32 x,nat32 y,nat32 level,real32 * in[3],real32 * out)
  {
   // First calculate the costs for each label of the D function and message alone...    
    for (nat32 i=0;i<labels;i++)
    {
     temp[i] = cacheD[level][labels*(y*(width>>level) + x) + i];
     for (nat32 j=0;j<3;j++) temp[i] += in[j][i];
    }
    
   // Then find the minimum for each field considering the V function...
    for (nat32 j=0;j<labels;j++)
    {
     out[j] = temp[0] + cacheV[labels*j];
     for (nat32 i=1;i<labels;i++)
     {
      real32 val = temp[i] + cacheV[labels*j + i];
      if (val<out[j]) out[j] = val;	     
     }	    
    }
    
   // And zero mean the vector...
    real32 sum = 0.0;
    for (nat32 i=0;i<labels;i++) sum += out[i];
    sum /= real32(labels);
    for (nat32 i=0;i<labels;i++) out[i] -= sum;
  }
  
  nat32 Label(nat32 x,nat32 y,real32 * in[4],real32 * out)
  {
   nat32 ret = 0;
   real32 best = cacheD[0][labels*(y*width + x)];
   for (nat32 i=0;i<4;i++) best += in[i][0];
   if (out) out[0] = best;
   
   for (nat32 i=1;i<labels;i++)
   {
    real32 score = cacheD[0][labels*(y*width + x) + i];
    for (nat32 j=0;j<4;j++) score += in[j][i];
    if (out) out[i] = score;
    if (score<best)
    {
     ret = i;
     best = score;	    
    }
   }
   
   return ret;
  }


  static inline cstrconst TypeString() 
  {
   static GlueStr ret(GlueStr() << "eos::alg::AnyMsgBP2D<" << typestring<PT>() << ">");
   return ret;
  }


 private:
  const PT & data;
  nat32 width;
  nat32 height;
  nat32 labels;
  // Creates math::Min(TopBit(width),TopBit(height)) levels in cacheD.
  real32 ** cacheD; // cacheD[level][labels*(y*(width>>level)+x)+label];
  real32 * cacheV; // cacheV[labels*lab2 + lab1].
  
  math::Vector<real32> temp;
};

//------------------------------------------------------------------------------
/// This is a MsgBP2D interface implimentor for when the V function returns 0.0
/// when given two identical labels and a constant for any other combination.
/// This allows for a O(labels) runtime to calculate a message. Caches the D 
/// function as you would expect.
/// Templated on:
/// - PT As in MsgBP2D.
/// - D is the data cost function, and is the cost of assigning a particular label to a
///   particular entry in the grid.
/// - VC returns the cost of two neighbours labels not matching, the cost is 0.0
///   if they are equal.
template <typename PT,
          real32 D(const PT & data,nat32 x,nat32 y,nat32 label),
          real32 VC(const PT & data)>
class EOS_CLASS PottsMsgBP2D : public MsgBP2D<PT>
{
 public:
  PottsMsgBP2D(const PT & d,nat32 w,nat32 h,nat32 l,bit ms)
  :MsgBP2D<PT>(d,w,h,l,ms),data(d),width(w),height(h),labels(l)
  {
   // The D cache...
    nat32 levels = math::Min(math::TopBit(width),math::TopBit(height));
    cacheD = mem::Malloc<real32*>(levels);
    
    cacheD[0] = mem::Malloc<real32>(width*height*labels);
    for (nat32 y=0;y<height;y++)
    {
     for (nat32 x=0;x<width;x++)
     {
      for (nat32 lab=0;lab<labels;lab++) cacheD[0][labels*(y*width + x) + lab] = D(data,x,y,lab);
     }
    }
    
    if (ms)
    {
     for (nat32 lev=1;lev<levels;lev++)
     {
      cacheD[lev] = mem::Malloc<real32>((width>>lev)*(height>>lev)*labels);
      for (nat32 y=0;y<(height>>lev);y++)
      {
       for (nat32 x=0;x<(width>>lev);x++)
       {
        for (nat32 lab=0;lab<labels;lab++)
        {
         cacheD[lev][labels*(y*(width>>lev) + x) + lab] = 
               cacheD[lev-1][labels*(((y<<1)+0)*(width>>(lev-1)) + ((x<<1)+0)) + lab] +
               cacheD[lev-1][labels*(((y<<1)+1)*(width>>(lev-1)) + ((x<<1)+0)) + lab] +
               cacheD[lev-1][labels*(((y<<1)+0)*(width>>(lev-1)) + ((x<<1)+1)) + lab] +
               cacheD[lev-1][labels*(((y<<1)+1)*(width>>(lev-1)) + ((x<<1)+1)) + lab];
        }
       }
      }
     }
    }
    else
    {
     for (nat32 l=1;l<levels;l++) cacheD[l] = null<real32*>();
    }
    
   // Cost of difference...
    diffCost = VC(data); 
  }
  
  ~PottsMsgBP2D()
  {
   nat32 levels = math::Min(math::TopBit(width),math::TopBit(height));
   for (nat32 i=0;i<levels;i++) mem::Free(cacheD[i]);
   mem::Free(cacheD);
  }
  
  void Msg(nat32 x,nat32 y,nat32 level,real32 * in[3],real32 * out)
  {
   // First calculate the costs for each label of the D function and message alone...    
    for (nat32 i=0;i<labels;i++)
    {
     out[i] = cacheD[level][labels*(y*(width>>level) + x) + i];
     for (nat32 j=0;j<3;j++) out[i] += in[j][i];
    }
    
   // Find the minimum of the temp array + diffCost...
    real32 minDiff = out[0];
    for (nat32 i=1;i<labels;i++) minDiff = math::Min(minDiff,out[i]);
    minDiff += diffCost;
        
   // Then find the minimum for each field considering the V function...
    for (nat32 i=0;i<labels;i++) out[i] = math::Min(out[i],minDiff);     

   // And zero mean the vector...
    real32 sum = 0.0;
    for (nat32 i=0;i<labels;i++) sum += out[i];
    sum /= real32(labels);
    for (nat32 i=0;i<labels;i++) out[i] -= sum;
  }
  
  nat32 Label(nat32 x,nat32 y,real32 * in[4],real32 * out)
  {
   nat32 ret = 0;
   real32 best = cacheD[0][labels*(y*width + x)];
   for (nat32 i=0;i<4;i++) best += in[i][0];
   if (out) out[0] = best;
   
   for (nat32 i=1;i<labels;i++)
   {
    real32 score = cacheD[0][labels*(y*width + x) + i];
    for (nat32 j=0;j<4;j++) score += in[j][i];
    if (out) out[i] = score;
    if (score<best)
    {
     ret = i;
     best = score;	    
    }
   }
   
   return ret;
  }


  static inline cstrconst TypeString() 
  {
   static GlueStr ret(GlueStr() << "eos::alg::PottsMsgBP2D<" << typestring<PT>() << ">");
   return ret;
  }


 private:
  const PT & data;
  nat32 width;
  nat32 height;
  nat32 labels;
  // Creates math::Min(TopBit(width),TopBit(height)) levels in cacheD.
  real32 ** cacheD; // cacheD[level][labels*(y*(width>>level)+x)+label];
  real32 diffCost;
};

//------------------------------------------------------------------------------
/// This is a MsgBP2D interface implimentor for when the V function return a
/// multiplier of the absolute difference between the two labels,
/// i.e. V = mult*abs(label1-label2).
///
/// Templated on:
/// This allows for a O(labels) runtime to calculate a message. Caches the D 
/// function as you would expect.
/// - PT As in MsgBP2D.
/// - D is the data cost function, and is the cost of assigning a particular label to a
///   particular entry in the grid.
/// - VM returns the multiplier of the distance between two labels.
template <typename PT,
          real32 D(const PT & data,nat32 x,nat32 y,nat32 label),
          real32 VM(const PT & data)>
class EOS_CLASS LinearMsgBP2D : public MsgBP2D<PT>
{
 public:
  LinearMsgBP2D(const PT & d,nat32 w,nat32 h,nat32 l,bit ms)
  :MsgBP2D<PT>(d,w,h,l,ms),data(d),width(w),height(h),labels(l)
  {
   // The D cache...
    nat32 levels = math::Min(math::TopBit(width),math::TopBit(height));
    cacheD = mem::Malloc<real32*>(levels);
    
    cacheD[0] = mem::Malloc<real32>(width*height*labels);
    for (nat32 y=0;y<height;y++)
    {
     for (nat32 x=0;x<width;x++)
     {
      for (nat32 lab=0;lab<labels;lab++) cacheD[0][labels*(y*width + x) + lab] = D(data,x,y,lab);
     }
    }
    
    if (ms)
    {
     for (nat32 lev=1;lev<levels;lev++)
     {
      cacheD[lev] = mem::Malloc<real32>((width>>lev)*(height>>lev)*labels);
      for (nat32 y=0;y<(height>>lev);y++)
      {
       for (nat32 x=0;x<(width>>lev);x++)
       {
        for (nat32 lab=0;lab<labels;lab++)
        {
         cacheD[lev][labels*(y*(width>>lev) + x) + lab] = 
               cacheD[lev-1][labels*(((y<<1)+0)*(width>>(lev-1)) + ((x<<1)+0)) + lab] +
               cacheD[lev-1][labels*(((y<<1)+1)*(width>>(lev-1)) + ((x<<1)+0)) + lab] +
               cacheD[lev-1][labels*(((y<<1)+0)*(width>>(lev-1)) + ((x<<1)+1)) + lab] +
               cacheD[lev-1][labels*(((y<<1)+1)*(width>>(lev-1)) + ((x<<1)+1)) + lab];
        }
       }
      }
     }
    }
    else
    {
     for (nat32 l=1;l<levels;l++) cacheD[l] = null<real32*>();
    }
    
   // Cost of difference...
    linMult = VM(data); 
  }
  
  ~LinearMsgBP2D()
  {
   nat32 levels = math::Min(math::TopBit(width),math::TopBit(height));
   for (nat32 i=0;i<levels;i++) mem::Free(cacheD[i]);
   mem::Free(cacheD);
  }
  
  void Msg(nat32 x,nat32 y,nat32 level,real32 * in[3],real32 * out)
  {
   // First calculate the costs for each label of the D function and message alone...    
    for (nat32 i=0;i<labels;i++)
    {
     out[i] = cacheD[level][labels*(y*(width>>level) + x) + i];
     for (nat32 j=0;j<3;j++) out[i] += in[j][i];
    }

   // Make two passes over the output array to apply the linear constraint...
    for (nat32 i=1;i<labels;i++) out[i] = math::Min(out[i],out[i-1]+linMult);
    for (int32 i=labels-2;i>=0;i--) out[i] = math::Min(out[i],out[i+1]+linMult);

   // And zero mean the vector...
    real32 sum = 0.0;
    for (nat32 i=0;i<labels;i++) sum += out[i];
    sum /= real32(labels);
    for (nat32 i=0;i<labels;i++) out[i] -= sum;
  }
  
  nat32 Label(nat32 x,nat32 y,real32 * in[4],real32 * out)
  {
   nat32 ret = 0;
   real32 best = cacheD[0][labels*(y*width + x)];
   for (nat32 i=0;i<4;i++) best += in[i][0];
   if (out) out[0] = best;
   
   for (nat32 i=1;i<labels;i++)
   {
    real32 score = cacheD[0][labels*(y*width + x) + i];
    for (nat32 j=0;j<4;j++) score += in[j][i];
    if (out) out[i] = score;
    if (score<best)
    {
     ret = i;
     best = score;	    
    }
   }
   
   return ret;
  }


  static inline cstrconst TypeString() 
  {
   static GlueStr ret(GlueStr() << "eos::alg::LinearMsgBP2D<" << typestring<PT>() << ">");
   return ret;
  }


 private:
  const PT & data;
  nat32 width;
  nat32 height;
  nat32 labels;
  // Creates math::Min(TopBit(width),TopBit(height)) levels in cacheD.
  real32 ** cacheD; // cacheD[level][labels*(y*(width>>level)+x)+label];
  real32 linMult;
};

//------------------------------------------------------------------------------
/// This is a MsgBP2D interface implimentor for when the V function return a
/// multiplier of the absolute difference between the two labels that has been
/// enhanced with a maximum possible value, i.e. V = min(VM*abs(label1-label2),VT).
///
/// Templated on:
/// This allows for a O(labels) runtime to calculate a message. Caches the D 
/// function as you would expect.
/// - PT As in MsgBP2D.
/// - D is the data cost function, and is the cost of assigning a particular label to a
///   particular entry in the grid.
/// - VM returns the multiplier of the distance between two labels.
/// - VT returns the maximum allowed cost.
template <typename PT,
          real32 D(const PT & data,nat32 x,nat32 y,nat32 label),
          real32 VM(const PT & data),
          real32 VT(const PT & data)>
class EOS_CLASS TruncLinearMsgBP2D : public MsgBP2D<PT>
{
 public:
  TruncLinearMsgBP2D(const PT & d,nat32 w,nat32 h,nat32 l,bit ms)
  :MsgBP2D<PT>(d,w,h,l,ms),data(d),width(w),height(h),labels(l)
  {
   // The D cache...
    nat32 levels = math::Min(math::TopBit(width),math::TopBit(height));
    cacheD = mem::Malloc<real32*>(levels);
    
    cacheD[0] = mem::Malloc<real32>(width*height*labels);
    for (nat32 y=0;y<height;y++)
    {
     for (nat32 x=0;x<width;x++)
     {
      for (nat32 lab=0;lab<labels;lab++) cacheD[0][labels*(y*width + x) + lab] = D(data,x,y,lab);
     }
    }
    
    if (ms)
    {
     for (nat32 lev=1;lev<levels;lev++)
     {
      cacheD[lev] = mem::Malloc<real32>((width>>lev)*(height>>lev)*labels);
      for (nat32 y=0;y<(height>>lev);y++)
      {
       for (nat32 x=0;x<(width>>lev);x++)
       {
        for (nat32 lab=0;lab<labels;lab++)
        {
         cacheD[lev][labels*(y*(width>>lev) + x) + lab] = 
               cacheD[lev-1][labels*(((y<<1)+0)*(width>>(lev-1)) + ((x<<1)+0)) + lab] +
               cacheD[lev-1][labels*(((y<<1)+1)*(width>>(lev-1)) + ((x<<1)+0)) + lab] +
               cacheD[lev-1][labels*(((y<<1)+0)*(width>>(lev-1)) + ((x<<1)+1)) + lab] +
               cacheD[lev-1][labels*(((y<<1)+1)*(width>>(lev-1)) + ((x<<1)+1)) + lab];
        }
       }
      }
     }
    }
    else
    {
     for (nat32 l=1;l<levels;l++) cacheD[l] = null<real32*>();
    }
    
   // Cost of difference...
    linMult = VM(data); 
    linTrunc = VT(data);
  }
  
  ~TruncLinearMsgBP2D()
  {
   nat32 levels = math::Min(math::TopBit(width),math::TopBit(height));
   for (nat32 i=0;i<levels;i++) mem::Free(cacheD[i]);
   mem::Free(cacheD);
  }
  
  void Msg(nat32 x,nat32 y,nat32 level,real32 * in[3],real32 * out)
  {
   // First calculate the costs for each label of the D function and message alone...    
    for (nat32 i=0;i<labels;i++)
    {
     out[i] = cacheD[level][labels*(y*(width>>level) + x) + i];
     for (nat32 j=0;j<3;j++) out[i] += in[j][i];
    }
    
   // Find the minimum with linTrunc added...
    real32 maxVal = out[0];
    for (nat32 i=1;i<labels;i++) maxVal = math::Min(maxVal,out[i]);
    maxVal += linTrunc;
     
   // Make two passes over the output array to apply the linear constraint...
    for (nat32 i=1;i<labels;i++) out[i] = math::Min(out[i],out[i-1]+linMult);
    for (int32 i=labels-2;i>=0;i--) out[i] = math::Min(out[i],out[i+1]+linMult);
    
   // Apply the truncation...
    for (nat32 i=0;i<labels;i++) out[i] = math::Min(out[i],maxVal);

   // And zero mean the vector...
    real32 sum = 0.0;
    for (nat32 i=0;i<labels;i++) sum += out[i];
    sum /= real32(labels);
    for (nat32 i=0;i<labels;i++) out[i] -= sum;
  }
  
  nat32 Label(nat32 x,nat32 y,real32 * in[4],real32 * out)
  {
   nat32 ret = 0;
   real32 best = cacheD[0][labels*(y*width + x)];
   for (nat32 i=0;i<4;i++) best += in[i][0];
   if (out) out[0] = best;
   
   for (nat32 i=1;i<labels;i++)
   {
    real32 score = cacheD[0][labels*(y*width + x) + i];
    for (nat32 j=0;j<4;j++) score += in[j][i];
    if (out) out[i] = score;
    if (score<best)
    {
     ret = i;
     best = score;	    
    }
   }
   
   return ret;
  }


  static inline cstrconst TypeString() 
  {
   static GlueStr ret(GlueStr() << "eos::alg::TruncLinearMsgBP2D<" << typestring<PT>() << ">");
   return ret;
  }


 private:
  const PT & data;
  nat32 width;
  nat32 height;
  nat32 labels;
  // Creates math::Min(TopBit(width),TopBit(height)) levels in cacheD.
  real32 ** cacheD; // cacheD[level][labels*(y*(width>>level)+x)+label];
  real32 linMult;
  real32 linTrunc;
};

//------------------------------------------------------------------------------
/// This is a basic loopy max product 2d grid BP implimentation. It is included for
/// comparing the hierachical version against, to check its correctness.
/// Avoid using for real problems like the plaugue, unless your planning on
/// making lots of cups of coffee whilst the code runs.
/// Using the given message creation type and a passthrough object this assignes
/// a label to every node on the grid to minimise the cost function generated by
/// the MSGBP type.
/// It uses one optimisation beyond brute force, the checkerboard alternate update
/// schedual.
/// The MSGBP template parameter must be an implimentor of the MsgBP interface,
/// this is responsible for all the actual details and its in the specification
/// of this that you customise the algorithm. (This is just a framework containing
/// most of the code thats independent of the function being optimised.)
/// Note that a border of pixels width 1 arround the image will not be handled 
/// entirely correctly, especially at the corners, so whilst there values shouldn't
/// be totaly bad there reliability will not match the rest.
template <typename MSGBP>
class EOS_CLASS BP2D
{
 public:
  /// &nbsp;
   BP2D():labels(2),pt(null<typename MSGBP::passthroughType const *>()) {}
   
  /// &nbsp;
   ~BP2D() {}


  /// This sets the output field and must be called before Run(). In addition it
  /// sets the number of iterations to width + height, i.e. the minimum required for
  /// information to travel from one corner to the other.
   void SetOutput(svt::Field<nat32> & o)
   {
    out = o;
    iters = out.Size(0) + out.Size(1);	   
   }

  /// This optionally sets a field into which the costs of each label will be output.
  /// This must be 3 dimensional - (x,y,label)
   void SetDebugOutput(svt::Field<real32> & o)
   {
    dOut = o;
   }

  /// This sets the number of labels to work with, must be called.
   void SetLabels(nat32 l)
   {
    labels = l;	   
   }
   
  /// This sets the passthrough object, unless the cost functions can work without
  /// it this must be called.
   void SetPT(const typename MSGBP::passthroughType & ptt)
   {
    pt = &ptt;	   
   }
  
  /// This sets the number of iterations, incase you are not happy with width+height
  /// iterations. Call after setting he output field, as that action overwrites.
   void SetIterations(nat32 ic)
   {
    iters = ic;
   }
  
  /// Runs the algorithm, providing a progress report. The given output field will
  /// contain the results once this returns.
   void Run(time::Progress * prog = null<time::Progress*>())
   {
    prog->Push();    
    
    // Create the structure to contain the messages, and initialise them...
     prog->Report(0,iters+3);     
     // index as [labels*(4*(out.Size(0)*y + x) + dir) + label]...
     real32 * md = mem::Malloc<real32>(out.Size(0)*out.Size(1)*4*labels);
     for (nat32 i=0;i<out.Size(0)*out.Size(1)*4*labels;i++) md[i] = 0.0;          
     
     enum Dir {north,east,south,west}; // +ve x = east; +ve y = north.
     
    // Zeroed out message, used for the border...
     real32 * zeroMsg = mem::Malloc<real32>(labels);
     for (nat32 i=0;i<labels;i++) zeroMsg[i] = 0.0;
    
    // Construct the message creation object...
     prog->Report(1,iters+3);
     MSGBP msg(*pt,out.Size(0),out.Size(1),labels,false);
    
    // Do the iterations, updating the messages in the checkboard pattern...
     for (nat32 i=0;i<iters;i++)
     {
      prog->Report(i+2,iters+3);
      for (nat32 y=0;y<out.Size(1);y++)
      {
       for (nat32 x=((y+i)%2);x<out.Size(0);x+=2)
       {
        real32 * rm[3];
        
        if (x!=0) rm[0] = &md[labels*(4*(out.Size(0)*y + x-1) + east)];
             else rm[0] = zeroMsg;
        if (x!=out.Size(0)-1) rm[1] = &md[labels*(4*(out.Size(0)*y + x+1) + west)];
                         else rm[1] = zeroMsg;
        if (y!=0) rm[2] = &md[labels*(4*(out.Size(0)*(y-1) + x) + north)];
             else rm[2] = zeroMsg;
	msg.Msg(x,y,0,rm,&md[labels*(4*(out.Size(0)*y + x) + north)]);
	       
        if (x!=0) rm[0] = &md[labels*(4*(out.Size(0)*y + x-1) + east)];
             else rm[0] = zeroMsg;
        if (x!=out.Size(0)-1) rm[1] = &md[labels*(4*(out.Size(0)*y + x+1) + west)];
                         else rm[1] = zeroMsg;
        if (y!=out.Size(1)-1) rm[2] = &md[labels*(4*(out.Size(0)*(y+1) + x) + south)];
                         else rm[2] = zeroMsg;
	msg.Msg(x,y,0,rm,&md[labels*(4*(out.Size(0)*y + x) + south)]);
	       
        if (x!=0) rm[0] = &md[labels*(4*(out.Size(0)*y + x-1) + east)];
             else rm[0] = zeroMsg;
        if (y!=0) rm[1] = &md[labels*(4*(out.Size(0)*(y-1) + x) + north)];
             else rm[1] = zeroMsg;
        if (y!=out.Size(1)-1) rm[2] = &md[labels*(4*(out.Size(0)*(y+1) + x) + south)];
                         else rm[2] = zeroMsg;
	msg.Msg(x,y,0,rm,&md[labels*(4*(out.Size(0)*y + x) + east)]);
	
        if (x!=out.Size(0)-1) rm[0] = &md[labels*(4*(out.Size(0)*y + x+1) + west)];
                         else rm[0] = zeroMsg;
        if (y!=0) rm[1] = &md[labels*(4*(out.Size(0)*(y-1) + x) + north)];
             else rm[1] = zeroMsg;
        if (y!=out.Size(1)-1) rm[2] = &md[labels*(4*(out.Size(0)*(y+1) + x) + south)];
                         else rm[2] = zeroMsg;
	msg.Msg(x,y,0,rm,&md[labels*(4*(out.Size(0)*y + x) + west)]);
       }
      }
     }
    
    // Extract the final labeling...
     prog->Report(iters+2,iters+3);
     real32 * costOut = null<real32*>();
     if (dOut.Valid()) costOut = mem::Malloc<real32>(labels);
     
     for (nat32 y=0;y<out.Size(1);y++)
     {
      for (nat32 x=0;x<out.Size(0);x++)
      {
       real32 * rm[4];
        if (x!=0) rm[0] = &md[labels*(4*(out.Size(0)*y + x-1) + east)];
             else rm[0] = zeroMsg;
        if (x!=out.Size(0)-1) rm[1] = &md[labels*(4*(out.Size(0)*y + x+1) + west)];
                         else rm[1] = zeroMsg;
        if (y!=0) rm[2] = &md[labels*(4*(out.Size(0)*(y-1) + x) + north)];
             else rm[2] = zeroMsg;
        if (y!=out.Size(1)-1) rm[3] = &md[labels*(4*(out.Size(0)*(y+1) + x) + south)];
                         else rm[3] = zeroMsg;

       out.Get(x,y) = msg.Label(x,y,rm,costOut);

       if (costOut)
       {
        for (nat32 l=0;l<labels;l++) dOut.Get(x,y,l) = costOut[l];
       }
      }
     }
     
     if (costOut) mem::Free(costOut);
     mem::Free(zeroMsg);
    
    prog->Pop();
   }
  

  /// &nbsp;
   static inline cstrconst TypeString() 
   {
    static GlueStr ret(GlueStr() << "eos::alg::BP2D<" << typestring<MSGBP>() << ">");
    return ret;
   } 


 private:
  // Number of labels...
   nat32 labels; 
   
  // The number of iterations to do - needs to be a lot to give the data time to move arround...
   nat32 iters;
   
  // The passthrough...
   typename MSGBP::passthroughType const * pt;
   
  // This field is where the output will go, it also determines the dimensions
  // of the output grid.
   svt::Field<nat32> out;
   
  // Debug output, optional. Contains the cost associated with every label.
   svt::Field<real32> dOut;
};

//------------------------------------------------------------------------------
/// This is a hierachical loopy max product BP implimentation. As fast as they
/// come, especially if given a specialist message calculator with suitable speed up.
/// Using the given message creation type and a passthrough object this assignes
/// a label to every node on the grid to minimise the cost function generated by
/// the MSGBP type.
/// The MSGBP template parameter must be an implimentor of the MsgBP interface,
/// this is responsible for all the actual details and its in the specification
/// of this that you customise the algorithm. (This is just a framework containing
/// most of the code thats independent of the function being optimised.)
/// Note that a border of pixels width 1 arround the image will not be handled 
/// entirely correctly, especially at the corners, so whilst there values shouldn't
/// be totaly bad there reliability will not match the rest.
template <typename MSGBP>
class EOS_CLASS HBP2D
{
 public:
  /// &nbsp;
   HBP2D():labels(2),maxLevels(32),levelIters(6),pt(null<typename MSGBP::passthroughType const *>()) {}
   
  /// &nbsp;
   ~HBP2D() {}


  /// This sets the output field and must be called before Run().
   void SetOutput(svt::Field<nat32> & o)
   {
    out = o;   
   }
   
  /// This optionally sets a field into which the costs of each label will be output.
  /// This must be 3 dimensional - (x,y,label)
   void SetDebugOutput(svt::Field<real32> & o)
   {
    dOut = o;
   }
  
  /// This sets the number of labels to work with, must be called.
   void SetLabels(nat32 l)
   {
    labels = l;	   
   }
   
  /// This sets the passthrough object, unless the cost functions can work without
  /// it this must be called.
   void SetPT(const typename MSGBP::passthroughType & ptt)
   {
    pt = &ptt;	   
   }
  
  /// This sets the number of iterations at each level and the maaximum number of
  /// levels. The maxLevels defaults to 32, which as the width/height are 32 bit
  /// numbers equates to the most possible, levelIters defaults to 6 which appears
  /// to be a good value. You are unlikelly to want to change these.
   void SetIterations(nat32 maxL,nat32 levelI)
   {
    maxLevels = maxL;
    levelIters = levelI;
   }
  
  /// Runs the algorithm, providing a progress report. The given output field will
  /// contain the results once this returns.
   void Run(time::Progress * prog = null<time::Progress*>())
   {
    prog->Push();    
    
    // Create the structure to contain the messages, and initialise them...
     nat32 levelCount = math::Min(maxLevels,math::TopBit(out.Size(0)),math::TopBit(out.Size(1)));
     prog->Report(0,levelCount*levelIters+3);
     
     // index as [labels*(4*(out.Size(0)*y + x) + dir) + label]...
     real32 * md = mem::Malloc<real32>(out.Size(0)*out.Size(1)*4*labels);
     for (nat32 i=0;i<out.Size(0)*out.Size(1)*4*labels;i++) md[i] = 0.0;     
     
     enum Dir {north,east,south,west}; // +ve x = east; +ve y = north.
     
    // Zeroed out message, used for the border...
     real32 * zeroMsg = mem::Malloc<real32>(labels);
     for (nat32 i=0;i<labels;i++) zeroMsg[i] = 0.0;     
    
    // Construct the message creation object...
     prog->Report(1,levelCount*levelIters+3);
     MSGBP msg(*pt,out.Size(0),out.Size(1),labels,true);


    // Do the iterations, updating the messages in the checkerboard pattern...     
     for (nat32 i=0;i<levelCount;i++)
     {
      nat32 level = (levelCount-1) - i;
      nat32 width = out.Size(0)>>level;
      nat32 height = out.Size(1)>>level;
      for (nat32 j=0;j<levelIters;j++)
      {
       prog->Report(2+i*levelIters+j,levelCount*levelIters+3);
       // Do the messages for this level...
       	for (nat32 y=0;y<height;y++)
        {
         for (nat32 x=((y+j)%2);x<width;x+=2)
         {
	  real32 * rm[3];
        
          if (x!=0) rm[0] = &md[labels*(4*(out.Size(0)*y + x-1) + east)];
               else rm[0] = zeroMsg;
          if (x!=width-1) rm[1] = &md[labels*(4*(out.Size(0)*y + x+1) + west)];
                     else rm[1] = zeroMsg;
          if (y!=0) rm[2] = &md[labels*(4*(out.Size(0)*(y-1) + x) + north)];
               else rm[2] = zeroMsg;
	  msg.Msg(x,y,level,rm,&md[labels*(4*(out.Size(0)*y + x) + north)]);
	         
          if (x!=0) rm[0] = &md[labels*(4*(out.Size(0)*y + x-1) + east)];
               else rm[0] = zeroMsg;
          if (x!=width-1) rm[1] = &md[labels*(4*(out.Size(0)*y + x+1) + west)];
                     else rm[1] = zeroMsg;
          if (y!=height-1) rm[2] = &md[labels*(4*(out.Size(0)*(y+1) + x) + south)];
                      else rm[2] = zeroMsg;
	  msg.Msg(x,y,level,rm,&md[labels*(4*(out.Size(0)*y + x) + south)]);
	         
          if (x!=0) rm[0] = &md[labels*(4*(out.Size(0)*y + x-1) + east)];
               else rm[0] = zeroMsg;
          if (y!=0) rm[1] = &md[labels*(4*(out.Size(0)*(y-1) + x) + north)];
               else rm[1] = zeroMsg;
          if (y!=height-1) rm[2] = &md[labels*(4*(out.Size(0)*(y+1) + x) + south)];
                      else rm[2] = zeroMsg;
	  msg.Msg(x,y,level,rm,&md[labels*(4*(out.Size(0)*y + x) + east)]);
	  
          if (x!=width-1) rm[0] = &md[labels*(4*(out.Size(0)*y + x+1) + west)];
                     else rm[0] = zeroMsg;
          if (y!=0) rm[1] = &md[labels*(4*(out.Size(0)*(y-1) + x) + north)];
               else rm[1] = zeroMsg;
          if (y!=height-1) rm[2] = &md[labels*(4*(out.Size(0)*(y+1) + x) + south)];
                      else rm[2] = zeroMsg;
	  msg.Msg(x,y,level,rm,&md[labels*(4*(out.Size(0)*y + x) + west)]);
         }
        }      
      }
      
      // If level 0 we break out here - isn't another level to scale upto...
       if (level==0) break;
      
      // Scale up the messages for the next level...
       nat32 newWidth = out.Size(0)>>(level-1);
       nat32 newHeight = out.Size(1)>>(level-1);
       for (int32 y=newHeight-1;y>=0;y--)
       {
	for (int32 x=newWidth-1;x>=0;x--)
	{
         real32 * to = &md[labels*4*(out.Size(0)*y + x)];
         real32 * from = &md[labels*4*(out.Size(0)*(y>>1) + (x>>1))];
	 if (to!=from) mem::Copy(to,from,4*labels);
	}
       }
     }
 
    
    // Extract the final labeling...
     prog->Report(levelCount*levelIters+2,levelCount*levelIters+3);
     real32 * costOut = null<real32*>();
     if (dOut.Valid()) costOut = mem::Malloc<real32>(labels);
     
     for (nat32 y=0;y<out.Size(1);y++)
     {
      for (nat32 x=0;x<out.Size(0);x++)
      {
       real32 * rm[4];
        if (x!=0) rm[0] = &md[labels*(4*(out.Size(0)*y + x-1) + east)];
             else rm[0] = zeroMsg;
        if (x!=out.Size(0)-1) rm[1] = &md[labels*(4*(out.Size(0)*y + x+1) + west)];
                         else rm[1] = zeroMsg;
        if (y!=0) rm[2] = &md[labels*(4*(out.Size(0)*(y-1) + x) + north)];
             else rm[2] = zeroMsg;
        if (y!=out.Size(1)-1) rm[3] = &md[labels*(4*(out.Size(0)*(y+1) + x) + south)];                     
                         else rm[3] = zeroMsg;

       out.Get(x,y) = msg.Label(x,y,rm,costOut);

       if (costOut)
       {
        for (nat32 l=0;l<labels;l++) dOut.Get(x,y,l) = costOut[l];
       }
      }
     }
     
     if (costOut) mem::Free(costOut);
     mem::Free(zeroMsg);
    
    prog->Pop();
   }
  

  /// &nbsp;
   static inline cstrconst TypeString() 
   {
    static GlueStr ret(GlueStr() << "eos::alg::BP2D<" << typestring<MSGBP>() << ">");
    return ret;
   } 


 private:
  // Number of labels...
   nat32 labels;
   
  // The number of iterations to do and maximum number of levels to do...
   nat32 maxLevels;
   nat32 levelIters;
   
  // The passthrough...
   typename MSGBP::passthroughType const * pt;
   
  // This field is where the output will go, it also determines the dimensions
  // of the output grid.
   svt::Field<nat32> out;
   
  // Debug output, optional. Contains the cost associated with every label.
   svt::Field<real32> dOut;   
};

//------------------------------------------------------------------------------
 };
};
#endif
