#ifndef EOS_MATH_ITER_MIN_H
#define EOS_MATH_ITER_MIN_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

/// \file iter_min.h
/// Provides methods to do non-linear minimisation of functions, i.e. iterative
/// techneques.

#include "eos/types.h"
#include "eos/math/mat_ops.h"
#include "eos/math/complex.h"
#include "eos/time/progress.h"
#include "eos/ds/lists.h"
#include "eos/ds/arrays.h"
#include "eos/ds/arrays2d.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
/// An implimentation of the Levenberg Marquardt algorithm.
/// Given e = f(p) for error vector e and parameter vector p and a starting
/// p that is hopefully reasonably close to the actual answer (Prefably
/// calculated with a linear method.) this finds a p so as to minimise the 
/// 2 norm of the error vector. An iterative algorithm, based on
/// Newtons method. It can converge to a local minima if badly initialised. You can
/// choose to provide a method to calculate the Jacobian if you want, if not it
/// does so numerically. Providing such a method ushally only provides only major 
/// improvement to runtime, not the quality of the results, and is often not worth
/// the implimentation effort.
///
/// This version is for vectors of a known size, and only uses the stack for memory
/// allocation. An error vector size by parameter vector sized jacobian is 
/// internally calculated. If this is going to be too big for the stack use the
/// variable sized version.
///
/// Template parameters:
/// - MS - The size of the error vector.
/// - PS - The size of the parameter vector.
/// - OT - An auxilary type, passed into the functions, for if they require extra information. Ushally a measurment vector.
/// - RT - The number type to use throughout.
///
/// \param pv The parameter vector. On call this is the starting p, on return this
/// contains the final p, the answer it has converged to.
/// \param oi An arbitary data structure that is passed into the functions, incase
/// they need other information in addition to the parameter vector. Passed constant as
/// it does not make sense for this data to change. Will ushally take the form of
/// the measurment vector for the function to map to, so it can be subtracted from
/// the functions result before return.
/// \param F The function in question, takes a parameter vector and an error 
/// vector, writes out to the error vector as calculated from the parameter
/// vector. The entire point is to find a parameter vector which minimises the error
/// vectors 2 norm.
/// \param DF The differential of the function in question. As expected considering F,
/// produces a jacobian. If set to null, its default, the jacobian is calcualted
/// numerically.
/// \param CF This function is called on the parameters after every iteration, it can
/// apply a consistancy constraint to 'pull back' the result to something reasonable
/// when dealing with over-parameterisations. Set to null to do nothing. Except for 
/// the initialisation parameters this will have allways been called before either F
/// or DF has been, excluding for calculation of Jacobians.
/// \param maxIter The maximum number of iterations to do before giving up.
/// \returns The 2 norm length of the final error vector, an indication of how good
/// the answer is. (The residual.)
template <nat32 MS,nat32 PS,typename OT,typename RT>
inline RT LM(Vect<PS,RT> & pv,const OT & oi,
             void (*F)(const Vect<PS,RT> & pv,Vect<MS,RT> & err,const OT & oi),
             void (*DF)(const Vect<PS,RT> & pv,Mat<MS,PS,RT> & jac,const OT & oi) = 0,
             void (*CF)(Vect<PS,RT> & pv,const OT & oi) = 0,
             nat32 maxIter = 1000)
{
 // Variables needed...
  Vect<MS,RT> err;
  RT errNorm; // Actually the 2 norm squared, we only care about comparisons of.
  Mat<MS,PS,RT> jacobian;
  RT lambda = 10e-3; // The value that varies depending on if things get better or not.  
  
  Vect<PS,RT> pvT;
  Vect<MS,RT> errT;  
  Mat<PS,PS,RT> lhs;
  Vect<PS,RT> rhs;

 // Calculate the error vector and its norm squared... 
  F(pv,err,oi);
  errNorm = err.LengthSqr();
      
 // Iterate...
  static const RT maxLambda = 1e10;
  for (nat32 k=0;k<maxIter;k++)
  {   
   // Calculate the jacobian...
    if (DF) DF(pv,jacobian,oi);
    else
    {
     // Calculate numerically...
      for (nat32 c=0;c<jacobian.Cols();c++)
      {
       pvT = pv;
       RT delta = Max(Abs(10e-4 * pv[c]),10e-6);             
       pvT[c] += delta;
       F(pvT,errT,oi);
       
       delta = 1.0/delta;
       for (nat32 r=0;r<jacobian.Rows();r++)
       {
        jacobian[r][c] = (errT[r] - err[r])*delta;
       }
      }
    }
   
   while (lambda<=maxLambda)
   {
    // Calculate the lhs of the normal equation...
     TransMult(jacobian,jacobian,lhs);
     for (nat32 i=0;i<lhs.Rows();i++) lhs[i][i] *= 1.0 + lambda;
     
    // Pseudo inverse it...
     PseudoInverse(lhs);
    
    // Calculate the negated update vector and inturn the new parameter vector...
     TransMultVect(jacobian,err,rhs);
     MultVect(lhs,rhs,pvT);
     for (nat32 i=0;i<pvT.Size();i++) pvT[i] = pv[i] - pvT[i];
     if (CF) CF(pvT,oi);
    
    // Test if the update vector improves things, if so store and
    // start another main iteration otherwise start another minor
    // iteration, adjust lambda as required...
     F(pvT,errT,oi);
     RT errNormT = errT.LengthSqr();  
     if (errNormT<errNorm)
     {
      errNorm = errNormT;
      pv = pvT;
      err = errT;
      lambda *= 0.1;
      if (math::Equal(lambda,RT(0.0))) lambda = 0.000001;
      break;
     }
     else
     {
      lambda *= 10.0;
     }
   }
   if (lambda>maxLambda) break;
  }

 return Sqrt(errNorm);
}

//------------------------------------------------------------------------------
/// An implimentation of the Levenberg Marquardt algorithm.
/// Given e = f(p) for error vector e and parameter vector p and a starting
/// p that is hopefully reasonably close to the actual answer (Prefably
/// calculated with a linear method.) this finds a p so as to minimise the 
/// 2 norm of the error vector. An iterative algorithm, based on
/// Newtons method. It can converge to a local minima if badly initialised. You can
/// choose to provide a method to calculate the Jacobian if you want, if not it
/// does so numerically. Providing such a method ushally only provides only major 
/// improvement to runtime, not the quality of the results, and is often not worth
/// the implimentation effort.
///
/// This version is for vectors of an unknown size, uses the heap for allocation so
/// has a slower startup time, but can handle reasonably big matrices.
///
/// Template parameters:
/// - OT - An auxilary type, passed into the functions, for if they require extra information.
///        Ushally a measurment vector.
/// - RT - The number type to use throughout.
///
/// \param evs The size of the error vector. The functions should all agree with this.
/// \param pv The parameter vector. On call this is the starting p, on return this
/// contains the final p, the answer it has converged to.
/// \param oi An arbitary data structure that is passed into the functions, incase
/// they need other information in addition to the parameter vector. Passed constant as
/// it does not make sense for this data to change. Will ushally take the form of
/// the measurment vector for the function to map to, so it can be subtracted from
/// the functions result before return.
/// \param F The function in question, takes a parameter vector and an error 
/// vector, writes out to the error vector as calculated from the parameter
/// vector. The entire point is to find a parameter vector which minimises the error
/// vectors 2 norm.
/// \param DF The differential of the function in question. As expected considering F,
/// produces a jacobian. If set to null, its default, the jacobian is calcualted
/// numerically.
/// \param CF This function is called on the parameters after every iteration, it can
/// apply a consistancy constraint to 'pull back' the result to something reasonable
/// when dealing with over-parameterisations. Set to null to do nothing. Except for 
/// the initialisation parameters this will have allways been called before either F
/// or DF has been, excluding for calculation of Jacobians.
/// \param maxIter The maximum number of iterations to do before giving up.
/// \returns The 2 norm length of the final error vector, an indication of how good
/// the answer is. (The residual.)
template <typename OT,typename RT>
inline RT LM(nat32 evs,Vector<RT> & pv,const OT & oi,
             void (*F)(const Vector<RT> & pv,Vector<RT> & err,const OT & oi),
             void (*DF)(const Vector<RT> & pv,Matrix<RT> & jac,const OT & oi) = 0,
             void (*CF)(Vector<RT> & pv,const OT & oi) = 0,
             nat32 maxIter = 1000)
{
 // Variables needed...
  Vector<RT> err(evs);
  RT errNorm; // Actually the 2 norm squared, we only care about comparisons of.
  Matrix<RT> jacobian(evs,pv.Size());
  RT lambda = 10e-3; // The value that varies depending on if things get better or not.  
  
  Vector<RT> pvT(pv.Size());
  Vector<RT> errT(evs);
  Matrix<RT> lhs(pv.Size(),pv.Size());
  PseudoInverseTemp<RT> lhsPIT(pv.Size(),pv.Size());
  Vector<RT> rhs(pv.Size());

 // Calculate the error vector and its norm squared... 
  F(pv,err,oi);
  errNorm = err.LengthSqr();
      
 // Iterate...
  static const RT maxLambda = 1e10;
  for (nat32 k=0;k<maxIter;k++)
  {
   // Calculate the jacobian...
    if (DF) DF(pv,jacobian,oi);
    else
    {
     // Calculate numerically...
      for (nat32 c=0;c<jacobian.Cols();c++)
      {
       pvT = pv;             
       RT delta = Max(Abs(10e-4 * pv[c]),10e-6);             
       pvT[c] += delta;
       F(pvT,errT,oi);
       
       delta = 1.0/delta;
       for (nat32 r=0;r<jacobian.Rows();r++)
       {
        jacobian[r][c] = (errT[r] - err[r])*delta;
       }
      }
    }
   
   while (lambda<=maxLambda)
   {
    // Calculate the lhs of the normal equation...
     TransMult(jacobian,jacobian,lhs);
     for (nat32 i=0;i<lhs.Rows();i++) lhs[i][i] *= 1.0 + lambda;
     
    // Pseudo inverse it...
     PseudoInverse(lhs,lhsPIT);
    
    // Calculate the negated update vector and inturn the new parameter vector...
     TransMultVect(jacobian,err,rhs);
     MultVect(lhs,rhs,pvT);
     for (nat32 i=0;i<pvT.Size();i++) pvT[i] = pv[i] - pvT[i];
     if (CF) CF(pvT,oi);
    
    // Test if the update vector improves things, if so store and
    // start another main iteration otherwise start another minor
    // iteration, adjust lambda as required...
     F(pvT,errT,oi);
     RT errNormT = errT.LengthSqr();  
     if (errNormT<errNorm)
     {
      errNorm = errNormT;
      pv = pvT;
      err = errT;
      lambda *= 0.1;
      if (math::Equal(lambda,RT(0.0))) lambda = 0.000001;
      break;
     }
     else
     {
      lambda *= 10.0;
     }
   }
   if (lambda>maxLambda) break;
  }

 return Sqrt(errNorm);
}

//------------------------------------------------------------------------------
/// Suport function for the LM users. This calculates the Jacobian of
/// the parameters for a given parameter vector and function, in the same style
/// as the LM implimentations using the same method of calculation.
/// (For the numerical case.)
/// Note that LM approximates (From Gauss-Newton iterations.) the Hessian as 
/// J^T J, so for consistancy if you need it then that calculation should 
/// probably be used.
/// This version works with fixed sized vectors.
template <nat32 MS,nat32 PS,typename OT,typename RT>
inline void JacobianLM(const Vect<PS,RT> & pv,const OT & oi,
             void (*F)(const Vect<PS,RT> & pv,Vect<MS,RT> & err,const OT & oi),
             const Mat<MS,PS,RT> & jacobian)
{
 // Calculate the error vector... 
  Vect<MS,RT> err;
  F(pv,err,oi);
      
 // Calculate the jacobian, numerically...
  Vect<PS,RT> pvT;  
  Vect<MS,RT> errT; 

  for (nat32 c=0;c<jacobian.Cols();c++)
  {
   pvT = pv;
   RT delta = Max(Abs(10e-4 * pv[c]),10e-6);             
   pvT[c] += delta;
   F(pvT,errT,oi);
     
   delta = 1.0/delta;
   for (nat32 r=0;r<jacobian.Rows();r++)
   {
    jacobian[r][c] = (errT[r] - err[r])*delta;
   }
  }
}

//------------------------------------------------------------------------------
/// Suport function for the LM users. This calculates the Jacobian of
/// the parameters for a given parameter vector and function, in the same style
/// as the LM implimentations using the same method of calculation.
/// (For the numerical case.)
/// Note that LM approximates (From Gauss-Newton iterations.) the Hessian as 
/// J^T J, so for consistancy if you need it then that calculation should 
/// probably be used.
/// This version works with variable sized vectors.
/// The provided jacobian matrix must be the correct size, i.e. evs X pv.Size().
template <typename OT,typename RT>
inline void JacobianLM(nat32 evs,const Vector<RT> & pv,const OT & oi,
                       void (*F)(const Vector<RT> & pv,Vector<RT> & err,const OT & oi),
                       Matrix<RT> & jacobian)
{
 log::Assert((jacobian.Rows()==evs)&&(jacobian.Cols()==pv.Size()));

 // Calculate the error vector... 
  Vector<RT> err(evs);
  F(pv,err,oi);
      
 // Calculate the jacobian, numerically...
  Vector<RT> pvT(pv.Size());
  Vector<RT> errT(evs);
  
  for (nat32 c=0;c<jacobian.Cols();c++)
  {
   pvT = pv;             
   RT delta = Max(Abs(10e-4 * pv[c]),10e-6);             
   pvT[c] += delta;
   F(pvT,errT,oi);
       
   delta = 1.0/delta;
   for (nat32 r=0;r<jacobian.Rows();r++)
   {
    jacobian[r][c] = (errT[r] - err[r])*delta;
   }
  }
}
    
//------------------------------------------------------------------------------
/// A sparse Levenberg Marquardt implimentation, suitable for solving certain 
/// systems, otherwise known as bundle adjustment. Implimented as a setup then
/// run class, due to the shear number of inputs this can be expected to take.
/// Expected to solve otherwise intractable problems, where the matrices involved
/// would be larger than the RAM in the computer used. For smaller problems just
/// use a basic LM implimentation, as this algorithm spends a lot of time on setup
/// to make the inner loop code fast.
///
/// This solves a non-linear problem iterativly which is in the following format:
/// - You have a set of parameter vectors that get mapped to a set of error vectors. The task is to minimise the error vectors. This transformation is provided to the class by a set of function pointers.
/// - The parameter vector is divided into 2 seperate lists of vectors. The error vector is divided into a set of elements for each pair made from an entry from each list.
/// - Each element of the error vector is only dependent on its two associated entrys, assuming it exists at all.
///
/// This means that the error function passed in is given two parameter vectors as well
/// as its own measurement vector. This class does not accept jacobian matrice functions
/// due to the complexity of specification, it does however accept covariance matrices.
/// From an optimisation point of view the first list is presumed greatly smaller than
/// the second, best to stick to this pattern.
///
/// This is a use and delete class, you can not use it twice. The memory consumption is
/// heavy also, so deleting it straight after use is recomended unless you want to start
/// paging, or even exceding memory address space on a 32 bit computer.
class EOS_CLASS SparseLM
{
 public:
  /// &nbsp;
   SparseLM();

  /// &nbsp;
   ~SparseLM();


  /// Before calling any other methods this sets the sizes associated with everything.
  /// These are the sizes of parameter vectors in the first and second lists and the
  /// size of each error vector. You must stick to these when calling the other methods.
   void SetSizes(nat32 sizeA,nat32 sizeB,nat32 sizeErr);

  /// This adds a parameter vector to the first list, returns its index.
  /// Once you have called AddError for the first time you must not call
  /// this again.
   nat32 AddParaA(const Vector<real64> & in);

  /// This adds a parameter vector to the second list, returns its index.
  /// Once you have called AddError for the first time you must not call
  /// this again.
   nat32 AddParaB(const Vector<real64> & in);

  /// This adds an error vector function, you tell it which two parameter
  /// vectors it eats and what extra data as a vector to provide the function
  /// with. You only need to add these as relevent, you can omit combinations
  /// for which you have no data. Note that you must make all relevent calls
  /// to the AddPara methods before calling this, as the first time this is 
  /// called it fundamentally changes the internal data structure. Obviously,
  /// you must call this at least once before calling Run, and you must *never*
  /// call this multiple times for the same (a,b) key.
   void AddError(nat32 a,nat32 b,const Vector<real64> & m,
                 void (*F)(const Vector<real64> & a,const Vector<real64> & b,const Vector<real64> & m,Vector<real64> & err));

  /// This sets a constraint function for a parameter vector, called after the
  /// vector changes so it can correct any inconsistancy in a representation with
  /// more degrees of freedom than its represente. This will allways be called 
  /// before the error function except for with the initialisation parameters and
  /// when creating the jacobian.
  /// Optional, must be called after the AddError state change.
   void AddConsA(nat32 a,void (*C)(Vector<real64> & a));

  /// This sets a constraint function for a parameter vector, called after the
  /// vector changes so it can correct any inconsistancy in a representation with
  /// more degrees of freedom than its represente. This will allways be called 
  /// before the error function except for with the initialisation parameters.
  /// Optional, must be called after the AddError state change.
   void AddConsB(nat32 b,void (*C)(Vector<real64> & b));

  /// This adds the covariance matrix for a specified error vector, must be a square
  /// matrix that is the size of the error vector of an allready passed in error
  /// function. The covariance matrix for any given error vector is assumed identity
  /// if you don't call this. The data structure is designed under the assumption that
  /// if this is going to be called it will be done so straight after the call to AddError
  /// for the same (a,b), it will still work otherwise, just at greatly reduced speed.
   void AddCovar(nat32 a,nat32 b,const Matrix<real64> & covar);


  /// This does the calculation. Whilst it does take a progress object it dosn't use it
  /// properly as it never knows when the main loop is goig to end till it does, so it
  /// just keeps track of how many loops it has actually done.
  /// Returns the final residual obtained, as a measure of how well it did.
   real64 Run(time::Progress * prog = null<time::Progress*>());


  /// Extracts the parameter vector from the first list for the given index.
   void GetParaA(nat32 ind,Vector<real64> & out);

  /// Extracts the parameter vector from the second list for the given index.
   void GetParaB(nat32 ind,Vector<real64> & out);


  /// &nbsp;
   inline cstrconst TypeString() const {return "eos::math::SparseLM";}


 private:
  // Internal constant variables...
   static const real64 maxLambda = 1e100;
   static const nat32 maxIter = 1000;

  // Sizes used throughout...
   nat32 sizeA;
   nat32 sizeB;
   nat32 sizeErr;

  // The pre-AddError data structure, just a pair of linked lists of parameters...
   bit preStruct; // true when the below stuff is actually in use.
   ds::List< Vector<real64>*,mem::KillDel< Vector<real64> > > paraListA;
   ds::List< Vector<real64>*,mem::KillDel< Vector<real64> > > paraListB;
   
  // The post-AddError data structure, augmented with stuff for use when running...
   // We augment the parameters with the e value. We also provided here 
   // storage for revised parameters before it is swaped in, assuming it passes
   // the test.
    struct ParaNode
    {
      ParaNode()
      :C(0),para(null<Vector<real64>*>()),paraNew(null<Vector<real64>*>()),
      uv(null<Matrix<real64>*>()),e(null<Vector<real64>*>())
      {}

     ~ParaNode() {delete para; delete paraNew; delete uv; delete e;}
     
     void (*C)(Vector<real64> & a);
     
     Vector<real64> * para;
     Vector<real64> * paraNew;
     Matrix<real64> * uv; // Either the u or v matrix.
     Vector<real64> * e;
    };

    ds::Array< ParaNode*,mem::MakeNull< ParaNode* >,mem::KillDel< ParaNode > > paraA;
    ds::Array< ParaNode*,mem::MakeNull< ParaNode* >,mem::KillDel< ParaNode > > paraB;

   // A structure and Array2D which stores some data for every pair taken from 
   // paraA and paraB. Several linked lists are also created over this structure
   // for efficient access of relevent data sets...
    struct PairNode
    {
      PairNode()
      :m(null< Vector<real64>* >()),covarInv(null< Matrix<real64>* >()),
      err(null< Vector<real64>* >()),errNew(null< Vector<real64>* >()),
      aJacob(null< Matrix<real64>* >()),bJacob(null< Matrix<real64>* >()),
      w(null< Matrix<real64>* >()),y(null< Matrix<real64>* >())
      {}
      
     ~PairNode()
     {delete m; delete covarInv; delete err; delete errNew; delete aJacob; delete bJacob; delete w; delete y;}
     
  
     // Linked list stuff...
      PairNode * next; // Next node in a linked list of all of 'em.
      PairNode * nextA; // Next node in a linked list by index in A.
      PairNode * nextB; // Next node in a linked list by index in B.            

      nat32 a; // Its location in the grid, so we know it when following the list.
      nat32 b; // "
          
     // Parameters passed in with regards to this coordinate...
      Vector<real64> * m;
      void (*F)(const Vector<real64> & a,const Vector<real64> & b,const Vector<real64> & m,Vector<real64> & err);
      Matrix<real64> * covarInv; // Store the inverse, as we don't need the not-inverse.

     // Intermediate storage used during algorithm run time...
      Vector<real64> * err;
      Vector<real64> * errNew;
      Matrix<real64> * aJacob;
      Matrix<real64> * bJacob;
      
      Matrix<real64> * w;
      Matrix<real64> * y; 
    };
    
    // Linked lists of pair nodes...     
     PairNode * list; // A linked list of all PairNode's.
     ds::Array<PairNode*,mem::MakeNull< PairNode* > > listA;
     ds::Array<PairNode*,mem::MakeNull< PairNode* > > listB;
     
     
  // The runtime data structures, to save passing parameters between methods...
   Vector<real64> * tempParaA;
   Vector<real64> * tempParaB;   
   Vector<real64> * tempErr;  
   
   Matrix<real64> * tempAerr; // sizeA by sizeErr matrix.
   Matrix<real64> * tempBerr; // sizeB by sizeErr matrix.
   
   Matrix<real64> * tempU; // sizeA by sizeA matrix.
   Matrix<real64> * tempV; // sizeB by sizeB matrix.
   Matrix<real64> * tempV2; // sizeB by sizeB matrix.
   
   PseudoInverseTemp<real64> * pimt;
   Matrix<real64> * s;
   Vector<real64> * es;
  
  // Internal methods, these do all the real work...
   void MakeJacobians();
   void NonLambdaWork();
   void MakePara(real64 lambda);
};

//------------------------------------------------------------------------------
 };
};
#endif
