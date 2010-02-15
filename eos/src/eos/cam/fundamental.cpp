//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/cam/fundamental.h"

#include "eos/math/iter_min.h"
#include "eos/math/eigen.h"
#include "eos/data/randoms.h"
#include "eos/file/csv.h"
#include "eos/mem/safety.h"

namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------
bit SevenPointFun(FunMatch * match[7],Fundamental & out)
{
 LogTime("eos::cam::SevenPointFun");
 // First construct the same matrix we would for the 8 point algorithm, but
 // with less content...
  math::Mat<9,9,real64> a;
  for (nat32 i=0;i<7;i++)
  {
   a[i][0] = match[i]->right[0]*match[i]->left[0];
   a[i][1] = match[i]->right[0]*match[i]->left[1];
   a[i][2] = match[i]->right[0];
   a[i][3] = match[i]->right[1]*match[i]->left[0];
   a[i][4] = match[i]->right[1]*match[i]->left[1];
   a[i][5] = match[i]->right[1];
   a[i][6] = match[i]->left[0];
   a[i][7] = match[i]->left[1];
   a[i][8] = 1.0;
  }
  for (nat32 i=0;i<9;i++) {a[7][i] = 0.0; a[8][i] = 0.0;}


 // Apply SVD, get u,d and v...
  math::Mat<9,9,real64> u = a;
  math::Vect<9,real64> d;
  math::Mat<9,9,real64> v;
  math::Vect<9,real64> temp;

  if (math::SVD(u,d,v,temp)==false) return false;


 // Extract the two matrices such that the fundamental matrix is the sum of
 // them multiplied by arbitary values where det(out)==0...
  Fundamental f1,f2;

  f1[0][0] = v[0][7]; f1[0][1] = v[1][7]; f1[0][2] = v[2][7];
  f1[1][0] = v[3][7]; f1[1][1] = v[4][7]; f1[1][2] = v[5][7];
  f1[2][0] = v[6][7]; f1[2][1] = v[7][7]; f1[2][2] = v[8][7];

  f2[0][0] = v[0][8]; f2[0][1] = v[1][8]; f2[0][2] = v[2][8];
  f2[1][0] = v[3][8]; f2[1][1] = v[4][8]; f2[1][2] = v[5][8];
  f2[2][0] = v[6][8]; f2[2][1] = v[7][8]; f2[2][2] = v[8][8];

  f1 /= math::FrobNorm(f1);
  f2 /= math::FrobNorm(f2);


 // Find real paramters to multiple the two matrices such that the determinant
 // is 0, multiple and add to create the output...
 // Because scale is irrelevant only the ratio matters, so we can define the two
 // values as one value, val for the first multiplier and 1-val as the second.
 // This is a cubic equation, we can get multiple answers, including complex
 // once, simply take one of the real answers.
  // Calculate the vector of multipliers for the cubic root, all 4 of them...
   math::Vect<4,real64> p(0.0);
   struct F
   {
    static inline void Func(real64 fr,real64 fs,real64 ft,real64 gr,real64 gs,real64 gt,math::Vect<4,real64> & out)
    {
     real64 dr = fr-gr;
     real64 ds = fs-gs;
     real64 dt = ft-gt;

     out[0] += dr*ds*dt;
     out[1] += dr*ds*gt + dr*gs*dt + gr*ds*dt;
     out[2] += dr*gs*gt + gr*ds*gt + gr*gs*dt;
     out[3] += gr*gs*gt;
    }
   };

   F::Func(f1[0][0],f1[1][2],f1[2][1],f2[0][0],f2[1][2],f2[2][1],p);
   F::Func(f1[0][1],f1[1][0],f1[2][2],f2[0][1],f2[1][0],f2[2][2],p);
   F::Func(f1[0][2],f1[1][1],f1[2][0],f2[0][2],f2[1][1],f2[2][1],p);
   p *= -1.0;
   F::Func(f1[0][0],f1[1][1],f1[2][2],f2[0][0],f2[1][1],f2[2][2],p);
   F::Func(f1[0][1],f1[1][2],f1[2][0],f2[0][1],f2[1][2],f2[2][0],p);
   F::Func(f1[0][2],f1[1][0],f1[2][1],f2[0][2],f2[1][0],f2[2][1],p);

   p.Normalise();

  // Find the roots...
   math::Vect<4,math::Complex<real64> > val;
   math::Mat<4,4,real64> polyTemp;
   nat32 rc = math::RobustPolyRoot(p,val,polyTemp);

  // Iterate the array of roots, ignore complex once, select one that looks good...
   real64 bestFrob = math::Infinity<real64>();
   bit foundFun = false;
   for (nat32 i=0;i<rc;i++)
   {
    if (!math::IsZero(val[i].y)) continue;

    Fundamental temp[2];
    temp[0] = f1; temp[0] *= val[i].x;
    temp[1] = f2; temp[1] *= 1.0-val[i].x;
    temp[0] += temp[1];
    real64 frob = math::Abs(math::FrobNorm(temp[0])-1.0);
    if ((foundFun==false)||(frob<bestFrob))
    {
     bestFrob = frob;
     out = temp[0];
     foundFun = true;
    }
   }
   if (foundFun==false) return false;


 // By construction it should be rank 2, but numerical error means it might not
 // be precise, so enforce this constraint...
 {
  math::Vect<3,real64> d;
  math::Mat<3,3,real64> v;
  math::Vect<3,real64> temp;

  if (math::SVD(out,d,v,temp)==false) return false;
  d[2] = 0.0;

  Fundamental funTemp;
  math::MultDiag(out,d,funTemp);
  math::MultTrans(funTemp,v,out);
 }


 // Set Frob norm==1 before we return...
  out /= math::FrobNorm(out);

 return true;
}

//------------------------------------------------------------------------------
bit ManyPointMirth(nat32 matchSize,FunMatch ** match,Fundamental & out)
{
 LogTime("eos::cam::ManyPointMirth");
 // Count how many matches we actualy have (Array can contain nulls.)...
  nat32 matchCount = 0;
  for (nat32 i=0;i<matchSize;i++)
  {
   if (match[i]) matchCount += 1;
  }


 // Handle the matchCount being less than 8...
  if (matchCount<8)
  {
   if (matchCount==7)
   {
    FunMatch * sml[7];
    nat32 j = 0;
    for (nat32 i=0;i<matchSize;i++)
    {
     if (match[i])
     {
      sml[j] = match[i];
      j += 1;
     }
    }
    return SevenPointFun(sml,out);
   }
   else
   {
    return false;
   }
  }


 // Build the big matrix to give to SVD...
  math::Matrix<real64> a(math::Max<nat32>(9,matchCount),9);
  for (nat32 i=0,j=0;i<matchSize;i++)
  {
   if (match[i])
   {
    a[j][0] = match[i]->right[0]*match[i]->left[0];
    a[j][1] = match[i]->right[0]*match[i]->left[1];
    a[j][2] = match[i]->right[0];
    a[j][3] = match[i]->right[1]*match[i]->left[0];
    a[j][4] = match[i]->right[1]*match[i]->left[1];
    a[j][5] = match[i]->right[1];
    a[j][6] = match[i]->left[0];
    a[j][7] = match[i]->left[1];
    a[j][8] = 1.0;

    j += 1;
   }
  }
  for (nat32 i=matchCount;i<9;i++)
  {
   for (nat32 j=0;j<9;j++) a[i][j] = 0.0;
  }


 // Calculate a full-rank fundamental matrix...
  math::Vect<9,real64> funV;
  if (math::RightNullSpace(a,funV)==false) return false;

  out[0][0] = funV[0]; out[0][1] = funV[1]; out[0][2] = funV[2];
  out[1][0] = funV[3]; out[1][1] = funV[4]; out[1][2] = funV[5];
  out[2][0] = funV[6]; out[2][1] = funV[7]; out[2][2] = funV[8];


 // Degrade to rank 2...
  math::Vect<3,real64> d;
  math::Mat<3,3,real64> v;
  math::Vect<3,real64> temp;

  if (math::SVD(out,d,v,temp)==false) return false;
  d[2] = 0.0;

  Fundamental funTemp;
  math::MultDiag(out,d,funTemp);
  math::MultTrans(funTemp,v,out);

 return true;
}

//------------------------------------------------------------------------------
// Helper for the below function...
struct ManyPointFunHelper
{
 // Structure to contain the data that is passed into LM to be passed through to
 // the error function...
  struct Cabbage
  {
   real64 cap;
   FunMatch ** match;
  };

 // The function that calculates the error vector...
  inline static void Func(const math::Vector<real64> & para,math::Vector<real64> & err,const Cabbage & cabbage)
  {
   // Convert the provided parameter vector back into a fundamental matrix...
    math::Mat<3,3,real64> m;
    math::Vect<3,real64> epiRight;
    m[0][0]     = para[0];
    m[0][1]     = para[1];
    m[0][2]     = para[2];
    m[1][0]     = para[3];
    m[1][1]     = para[4];
    m[1][2]     = para[5];
    m[2][0]     = para[6];
    m[2][1]     = para[7];
    m[2][2]     = para[8];
    epiRight[0] = para[9];
    epiRight[1] = para[10];
    epiRight[2] = para[11];

    math::Mat<3,3,real64> ss;
    math::SkewSymetric33(epiRight,ss);
    Fundamental fun;
    math::Mult(ss,m,fun);


   // Iterate every match and output the correct error for it...
    for (nat32 i=0;i<err.Size();i++)
    {
     if (cabbage.match[i])
     {
      err[i] = cabbage.match[i]->Dist(fun);
      if (cabbage.cap>0.0) err[i] = math::Min(err[i],cabbage.cap);
     }
     else
     {
      err[i] = 0.0;
     }
    }
  }

  inline static void NormFunc(math::Vector<real64> & pv, const Cabbage &)
  {
   real64 mLength = 0.0;
   for (nat32 i=0;i<9;i++) mLength += math::Sqr(pv[i]);
   mLength = math::InvSqrt(mLength);
   for (nat32 i=0;i<9;i++) pv[i] *= mLength;

   real64 eLength = 0.0;
   for (nat32 i=9;i<12;i++) eLength += math::Sqr(pv[i]);
   eLength = math::InvSqrt(eLength);
   for (nat32 i=9;i<12;i++) pv[i] *= eLength;
  }
};

//------------------------------------------------------------------------------
real64 ManyPointFun(nat32 matchSize,FunMatch ** match,Fundamental & fun,real64 tolerance)
{
 LogTime("eos::cam::ManyPointFun");
 // Minimises distance from the fundamental matrix. I represent it
 // over-specified but decomposed to enforce it being a singular matrix using 12 parameters.
 // 9 parameters for a 3x3 matrix.
 // 3 parameters for a vector of a skew symetric matrix.


 // Decompose the given fundamental matrix into the parameter vector...
  // Skew-symetric part (right epipole)...
   Fundamental funTra = fun;
   math::Transpose(funTra);
   math::Vect<3,real64> epiRight;
   math::RightNullSpace(funTra,epiRight);
   epiRight.Normalise();

  // Matrix that multiplied by the skew-symetric gets us the fundamental matrix...
   math::Mat<3,3,real64> m;
   math::SkewSymetric33(epiRight,funTra);
   math::Mult(funTra,fun,m);
   m /= math::FrobNorm(m);

  // Stick into a single vector...
   math::Vector<real64> para(12);
   para[0]  = m[0][0];
   para[1]  = m[0][1];
   para[2]  = m[0][2];
   para[3]  = m[1][0];
   para[4]  = m[1][1];
   para[5]  = m[1][2];
   para[6]  = m[2][0];
   para[7]  = m[2][1];
   para[8]  = m[2][2];
   para[9]  = epiRight[0];
   para[10] = epiRight[1];
   para[11] = epiRight[2];


 // Do the actual calculation...
  ManyPointFunHelper::Cabbage cabbage;
  cabbage.cap = tolerance;
  cabbage.match = match;

  real64 residual = math::LM(matchSize,para,cabbage,
                             &ManyPointFunHelper::Func,
                             static_cast<void(*)(const math::Vector<real64> &,math::Matrix<real64> &,const ManyPointFunHelper::Cabbage &)>(0),
                             &ManyPointFunHelper::NormFunc);
  if (!math::IsFinite(residual)) {LogDebug("[eos::cam::ManyPointFun] Tits up error."); return -1.0;}


 // Unpack the parameter matrix back into a fundamental matrix...
  m[0][0]     = para[0];
  m[0][1]     = para[1];
  m[0][2]     = para[2];
  m[1][0]     = para[3];
  m[1][1]     = para[4];
  m[1][2]     = para[5];
  m[2][0]     = para[6];
  m[2][1]     = para[7];
  m[2][2]     = para[8];
  epiRight[0] = para[9];
  epiRight[1] = para[10];
  epiRight[2] = para[11];

  math::SkewSymetric33(epiRight,funTra);
  math::Mult(funTra,m,fun);
  fun /= math::FrobNorm(fun);

 return residual;
}

//------------------------------------------------------------------------------
FunCalc::FunCalc()
{
 math::Identity(fun);
 residual = -1.0;
}

FunCalc::~FunCalc()
{}

nat32 FunCalc::AddMatch(const bs::Pnt & left,const bs::Pnt & right,bit reliable)
{
 Match match;
  match.left[0] = left[0];
  match.left[1] = left[1];
  match.right[0] = right[0];
  match.right[1] = right[1];
  match.reliable = reliable;
 nat32 ret = data.Size();
 data.AddBack(match);
 return ret;
}

nat32 FunCalc::AddMatch(const math::Vect<2,real64> & left,const math::Vect<2,real64> & right,bit reliable)
{
 Match match;
  match.left[0] = left[0];
  match.left[1] = left[1];
  match.right[0] = right[0];
  match.right[1] = right[1];
  match.reliable = reliable;
 nat32 ret = data.Size();
 data.AddBack(match);
 return ret;
}

nat32 FunCalc::Matches() const
{
 return data.Size();
}

bit FunCalc::Run(time::Progress * prog,real64 reliability,nat32 cap)
{
 LogBlock("eos::cam::FunCalc::Run","{reliability,cap}" << LogDiv() << reliability << LogDiv() << cap);

 // We need at least 7 matches to even think about doing something...
  residual = -1.0;
  if (data.Size()<7) return false;


 prog->Push();
 prog->Report(0,4);
 prog->Push();
 prog->Report(0,5);


 // Calculate the means of the matches...
  math::Vect<2,real64> leftMean(0.0);
  math::Vect<2,real64> rightMean(0.0);
  nat32 matchCount = 0;
  ds::List<Match>::Cursor targ = data.FrontPtr();
  while (!targ.Bad())
  {
   ++matchCount;

   leftMean[0]  += (targ->left[0] -leftMean[0])/real64(matchCount);
   leftMean[1]  += (targ->left[1] -leftMean[1])/real64(matchCount);
   rightMean[0] += (targ->right[0]-rightMean[0])/real64(matchCount);
   rightMean[1] += (targ->right[1]-rightMean[1])/real64(matchCount);

   ++targ;
  }


 // Calculate the standard deviation...
  prog->Report(1,5);
  math::Vect<2,real64> leftSd(0.0);
  math::Vect<2,real64> rightSd(0.0);
  matchCount = 0;
  targ = data.FrontPtr();
  while (!targ.Bad())
  {
   ++matchCount;

   leftSd[0]  += (math::Abs(targ->left[0]-leftMean[0])  -leftSd[0])/real64(matchCount);
   leftSd[1]  += (math::Abs(targ->left[1]-leftMean[1])  -leftSd[1])/real64(matchCount);
   rightSd[0] += (math::Abs(targ->right[0]-rightMean[0])-rightSd[0])/real64(matchCount);
   rightSd[1] += (math::Abs(targ->right[1]-rightMean[1])-rightSd[1])/real64(matchCount);

   ++targ;
  }


 // Set the standard deviations to there averages, so we don't change aspect ratio...
  leftSd[0] = 0.5*(leftSd[0]+leftSd[1]);
  leftSd[1] = leftSd[0];
  rightSd[0] = 0.5*(rightSd[0]+rightSd[1]);
  rightSd[1] = rightSd[0];


 // Calculate a pair of normalising matrices...
  prog->Report(2,5);
  math::Mat<3,3,real64> normLeft;
  math::Identity(normLeft);
  normLeft[0][0] = 1.0/leftSd[0]; normLeft[0][2] = -leftMean[0]/leftSd[0];
  normLeft[1][1] = 1.0/leftSd[1]; normLeft[1][2] = -leftMean[1]/leftSd[1];

  math::Mat<3,3,real64> normRight;
  math::Identity(normRight);
  normRight[0][0] = 1.0/rightSd[0]; normRight[0][2] = -rightMean[0]/rightSd[0];
  normRight[1][1] = 1.0/rightSd[1]; normRight[1][2] = -rightMean[1]/rightSd[1];


 // Fill in the normalised version of each match...
  prog->Report(3,5);
  targ = data.FrontPtr();
  while (!targ.Bad())
  {
   math::MultVectEH(normLeft,targ->left,targ->norm.left);
   math::MultVectEH(normRight,targ->right,targ->norm.right);
   targ->used = false; // For neatness, untidy to not.
   ++targ;
  }


 // Count how many reliable matches we have...
  prog->Report(4,5);
  nat32 rCount = 0;
  targ = data.FrontPtr();
  while (!targ.Bad())
  {
   if (targ->reliable) ++rCount;
   ++targ;
  }


 // Create an array of pointers to FunMatch that covers the entire match range,
 // used in multiple places below...
  mem::StackPtr<FunMatch*,mem::KillDelArray<FunMatch*> > fmp = new FunMatch*[data.Size()];

 // Adjust the tolerance to the normalised space...
  real64 adjTol = ransacTol * 0.25 * (normLeft[0][0] + normLeft[1][1] + normRight[0][0] + normRight[1][1]);

  prog->Pop();


 // Initialise, if we have less than 7 reliable matches use RANSAC,
 // otherwise the next step will cope...
  prog->Report(1,4);
  prog->Push();
  if (rCount<7)
  {
   // Ransac...
    // Create an array of matches, so we can select points efficiently,
    // fill in the relible matches into our sampling array...
     prog->Report(0,2);
     nat32 rCount = 0;
     FunMatch * smp[7];
     targ = data.FrontPtr();
     for (nat32 i=0,j=0;i<data.Size();i++)
     {
      if (targ->reliable)
      {
       smp[rCount] = &(targ->norm);
       ++rCount;
      }
      else
      {
       fmp[j] = &(targ->norm);
       ++j;
      }
      ++targ;
     }
     for (nat32 i=0;i<rCount;i++)
     {
      fmp[data.Size()-1-i] = smp[i];
     }


    // Now iterate trying random sets of matches, till we reach conditions
    // that mean its time to stop this monkey business...
     prog->Report(1,2);
     prog->Push();
     data::Random rand;
     nat32 mostInliers = 1;
     for (nat32 i=0;i<cap;i++)
     {
      // Fill in remaining match array with randomly selected matches...
      // (Without duplication.)
       for (nat32 j=rCount;j<7;j++)
       {
        nat32 ri = rand.Int(0,data.Size()-j-1);
        smp[j] = fmp[ri];
        math::Swap(fmp[ri],fmp[data.Size()-j-1]);
       }

      // Calculate the result...
       Fundamental funTemp;
       if (SevenPointFun(smp,funTemp)==false) continue;

      // Count how many matches are within tolerance...
       nat32 inlierCount = 0;
       for (nat32 j=0;j<data.Size();j++)
       {
        if (fmp[j]->Dist(funTemp)<adjTol) ++inlierCount;
       }

      // If best so far, store it...
       if (inlierCount>mostInliers)
       {
	mostInliers = inlierCount;
	fun = funTemp;
       }

      // Check if we can break due to having done enough samples...
       real64 sr = math::Ln(1.0-reliability)/math::Ln(1.0-math::Pow(real64(mostInliers)/real64(data.Size()),7.0));
       if (math::IsFinite(sr))
       {
	if (sr<=real64(i)) break;
	prog->Report(i,nat32(sr));
       }
     }
     prog->Pop();

     LogDebug("[fun.calculate] RANSAC result {fun,mostInliers}" << LogDiv() << fun << LogDiv() << mostInliers);
  }
  prog->Pop();


 // Refine...
  prog->Report(2,4);
  prog->Push();
   // Null the array of valid matches...
    targ = data.FrontPtr();
    nat32 goodMatches = 0;
    for (nat32 i=0;i<data.Size();i++)
    {
     if (targ->reliable) {fmp[i] = &(targ->norm); ++goodMatches;}
                    else fmp[i] = null<FunMatch*>();
     ++targ;
    }

   // Iterate, refining...
    bit firstTime = true;
    while (true)
    {
     prog->Report(goodMatches,data.Size());
     // Add to the valid matches array all matches valid with the current matrix...
      bit noneAdded = true;
      targ = data.FrontPtr();
      for (nat32 i=0;i<data.Size();i++)
      {
       if (fmp[i]==null<FunMatch*>())
       {
        if (targ->norm.Dist(fun)<adjTol)
        {
         fmp[i] = &(targ->norm);
	 noneAdded = false;
	 ++goodMatches;
        }
       }
       ++targ;
      }

     // If no matches were added we are done...
      if (noneAdded&&(firstTime==false)) break;
      firstTime = false;

     // Calculate the fundamental matrix from scratch...
      if (ManyPointMirth(data.Size(),&*fmp,fun)==false)
      {
       prog->Pop();
       prog->Pop();
       return false;
      }

     // Refine the matrix...
      real64 res = ManyPointFun(data.Size(),&*fmp,fun,-1.0);
      if ((!math::IsFinite(res))||(res<0.0))
      {
       prog->Pop();
       prog->Pop();
       return false;
      }

     LogDebug("[fun.calculate] Normalised estimate with " << goodMatches << " matches = " << fun);
    }
  prog->Pop();


 // Un-normalise, so we can revert to measurements in pixels for the final minimisation step...
  prog->Report(3,4);
  prog->Push();
  prog->Report(0,4);
  Fundamental funTemp;
  math::TransMult(normRight,fun,funTemp);
  math::Mult(funTemp,normLeft,fun);
  fun /= math::FrobNorm(fun);

  LogDebug("[fun.calculate] Un-normalised {fun}" << LogDiv() << fun);


 // Convert the fmp array to un-normalised matches...
  prog->Report(1,4);
  targ = data.FrontPtr();
  for (nat32 i=0;i<data.Size();i++)
  {
   if (fmp[i]) fmp[i] = &*targ;
   ++targ;
  }


 // One last refinement of the results the result using a LM minimisation...
  prog->Report(2,4);
  residual = ManyPointFun(data.Size(),&*fmp,fun,-1.0);
  if ((!math::IsFinite(residual))||(residual<0.0))
  {
   prog->Pop();
   prog->Pop();
   return false;
  }

  LogDebug("Final result {fun,residual}" << LogDiv() << fun << LogDiv() << residual);


 // Mark matches that were used...
  prog->Report(3,4);
  targ = data.FrontPtr();
  meanError = 0.0;
  usedCount = 0;
  for (nat32 j=0;j<data.Size();j++)
  {
   targ->used = fmp[j]!=null<FunMatch*>();
   if (targ->used)
   {
    ++usedCount;
    meanError += (targ->Dist(fun)-meanError)/real64(usedCount);
   }
   ++targ;
  }
  prog->Pop();


 prog->Pop();
 return true;
}

const Fundamental & FunCalc::Fun() const
{
 return fun;
}

real64 FunCalc::Residual() const
{
 return residual;
}

real64 FunCalc::MeanError() const
{
 return meanError;
}

nat32 FunCalc::UsedCount() const
{
 return usedCount;
}

void FunCalc::Used(ds::Array<bit> & out) const
{
 out.Size(data.Size());

 ds::List<Match>::Cursor targ = data.FrontPtr();
 for (nat32 i=0;i<data.Size();i++)
 {
  out[i] = targ->used;
  ++targ;
 }
}

//------------------------------------------------------------------------------
 };
};
