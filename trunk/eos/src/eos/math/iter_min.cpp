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

#include "eos/math/iter_min.h"

namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
SparseLM::SparseLM()
:sizeA(0),sizeB(0),sizeErr(0),preStruct(true),list(null<PairNode*>()),
tempParaA(null<Vector<real64>*>()),tempParaB(null<Vector<real64>*>()),tempErr(null<Vector<real64>*>()),
tempAerr(null<Matrix<real64>*>()),tempBerr(null<Matrix<real64>*>()),
tempU(null<Matrix<real64>*>()),tempV(null<Matrix<real64>*>()),tempV2(null<Matrix<real64>*>()),
pimt(null<PseudoInverseTemp<real64>*>()),s(null<Matrix<real64>*>()),es(null<Vector<real64>*>())
{}

SparseLM::~SparseLM()
{
 while (list)
 {
  PairNode * victim = list;
  list = list->next;
  delete victim;
 }

 delete tempParaA;
 delete tempParaB;
 delete tempErr;
 
 delete tempAerr;
 delete tempBerr; 
 
 delete tempU;
 delete tempV;
 delete tempV2;
 
 delete pimt;
 delete s;
 delete es;
}

void SparseLM::SetSizes(nat32 sA,nat32 sB,nat32 sE)
{
 sizeA = sA;
 sizeB = sB;
 sizeErr = sE;
}

nat32 SparseLM::AddParaA(const Vector<real64> & in)
{
 nat32 ret = paraListA.Size();
 paraListA.AddBack(new Vector<real64>(in)); 
 return ret;
}

nat32 SparseLM::AddParaB(const Vector<real64> & in)
{
 nat32 ret = paraListB.Size();
 paraListB.AddBack(new Vector<real64>(in));
 return ret;
}

void SparseLM::AddError(nat32 a,nat32 b,const Vector<real64> & m,
                        void (*F)(const Vector<real64> & a,const Vector<real64> & b,const Vector<real64> & m,Vector<real64> & err))
{
 // If we are not allready in postStruct mode, enter it...
  if (preStruct)
  {
   preStruct = false;
   
   // Resize things... 
    paraA.Size(paraListA.Size());
    paraB.Size(paraListB.Size());
    listA.Size(paraListA.Size());
    listB.Size(paraListB.Size());
    
   // Transfer things to the new shiny data structure...
    nat32 i = 0;
    while (paraListA.Size()!=0)
    {
     paraA[i] = new ParaNode();
     paraA[i]->para = paraListA.Front();
     paraListA.RemFront();
     paraA[i]->paraNew = new Vector<real64>(sizeA);
     paraA[i]->uv = new Matrix<real64>(sizeA,sizeA); 
     paraA[i]->e = new Vector<real64>(sizeA);     
     ++i;
    }
    
    i = 0;
    while (paraListB.Size()!=0)
    {
     paraB[i] = new ParaNode();
     paraB[i]->para = paraListB.Front();
     paraListB.RemFront();
     paraB[i]->paraNew = new Vector<real64>(sizeB);
     paraB[i]->uv = new Matrix<real64>(sizeB,sizeB); 
     paraB[i]->e = new Vector<real64>(sizeB);
     ++i;
    }    
  }
 
 // Now add the error function, rather easy in comparison to a mode change...
  PairNode * npn = new PairNode();
   npn->next = list; list = npn;
   npn->nextA = listA[a]; listA[a] = npn;
   npn->nextB = listB[b]; listB[b] = npn;
   npn->a = a;
   npn->b = b;
   npn->m = new Vector<real64>(m);
   npn->F = F;
   npn->err = new Vector<real64>(sizeErr);
   npn->errNew = new Vector<real64>(sizeErr);
   npn->aJacob = new Matrix<real64>(sizeErr,sizeA);
   npn->bJacob = new Matrix<real64>(sizeErr,sizeB);   
   npn->w = new Matrix<real64>(sizeA,sizeB);
   npn->y = new Matrix<real64>(sizeA,sizeB);
}

void SparseLM::AddConsA(nat32 a,void (*C)(Vector<real64> & a))
{
 paraA[a]->C = C;
}

void SparseLM::AddConsB(nat32 b,void (*C)(Vector<real64> & b))
{
 paraB[b]->C = C;
}

void SparseLM::AddCovar(nat32 a,nat32 b,const Matrix<real64> & covar)
{
 Matrix<real64> * ci = new Matrix<real64>(covar);
 Matrix<real64> * cit = new Matrix<real64>(covar);
 Inverse(*ci,*cit); 
 delete cit;
  
 PairNode * targ = list;
 while (targ)
 {
  if ((targ->a==a)&&(targ->b==b))
  {
   targ->covarInv = ci;
   break;
  }
  targ = targ->next;
 }
}

real64 SparseLM::Run(time::Progress * prog)
{
 prog->Push();
 // Here we do the looping over trying lambda values and checking for
 // improvment, everything else is pushed to other methods.
  // Create a bunch of large data structures...
   tempParaA = new Vector<real64>(sizeA);
   tempParaB = new Vector<real64>(sizeB);
   tempErr = new Vector<real64>(sizeErr);

   tempAerr = new Matrix<real64>(sizeA,sizeErr);
   tempBerr = new Matrix<real64>(sizeB,sizeErr);
   
   tempU = new Matrix<real64>(sizeA,sizeA);
   tempV = new Matrix<real64>(sizeB,sizeB);
   tempV2 = new Matrix<real64>(sizeB,sizeB);
   
   nat32 sSize = sizeA * paraA.Size();
   pimt = new PseudoInverseTemp<real64>(sSize,sSize);
   s = new Matrix<real64>(sSize,sSize);
   es = new Vector<real64>(sSize);


  // We require the listA linked lists to be sorted by b, a bit of a pain,
  // an inline bucket sort...
  {
   ds::Array<PairNode*> entry(paraB.Size());
   for (nat32 i=0;i<paraA.Size();i++)
   {
    for (nat32 j=0;j<entry.Size();j++) entry[j] = null<PairNode*>();
    
    PairNode * targ = listA[i];
    while (targ)
    {
     entry[targ->b] = targ; 
     targ = targ->nextA;
    }
    
    listA[i] = null<PairNode*>();
    for (int32 j=paraB.Size()-1;j>=0;j--)
    {
     if (entry[j])
     {
      entry[j]->nextA = listA[i];
      listA[i] = entry[j];
     }
    }
   }
  }


  // Before we start calculate the error vectors and the residual for all of 'em...
   real64 residual = 0.0; // We work squared, obviously.
   {
    PairNode * targ = list;
    while (targ)
    {
     (targ->F)(*paraA[targ->a]->para,*paraB[targ->b]->para,*targ->m,*targ->err);
     residual += targ->err->LengthSqr();
     targ = targ->next;
    }
   }


  // The primary loop, each time through we should reduce our residual...
   real64 lambda = 1e-3;   
   for (nat32 k=0;k<maxIter;k++)
   {
    prog->Report(k,k+1);
    // Fill out the jacobian matrices...
     MakeJacobians();
     
    // Calculate all cached values that are not dependent on lambda...
     NonLambdaWork();
        
    // The secondary loop, where we try out values of lambda till an improvment is found...
     while (lambda<=maxLambda)
     {
      // Calculate the new parameter vector...
       MakePara(lambda);

      // If its residual is an improvement swap it in, decrease lambda and break,
      // otherwise increase lambda and go arround again...
       real64 newResidual = 0.0;
       {
        PairNode * targ = list;
        while (targ)
        {
         (targ->F)(*paraA[targ->a]->paraNew,*paraB[targ->b]->paraNew,*targ->m,*targ->errNew);
         newResidual += targ->errNew->LengthSqr();
         targ = targ->next;
        }
       }
       if (newResidual<residual)
       {
        // It has improved, we are done doing secondry iterations, for now...
         residual = newResidual;
         for (nat32 i=0;i<paraA.Size();i++) mem::PtrSwap(paraA[i]->para,paraA[i]->paraNew);
         for (nat32 i=0;i<paraB.Size();i++) mem::PtrSwap(paraB[i]->para,paraB[i]->paraNew);
         
         PairNode * targ = list;
         while (targ)
         {
          mem::PtrSwap(targ->err,targ->errNew);
          targ = targ->next;
         }
         
         lambda *= 0.1;
         if (math::Equal(lambda,real64(0.0))) lambda = 0.000000001;
         break;
       }
       else
       {
        // It has not improved, fatten up the lambda...
         lambda *= 10.0;
       }
     }
     if (lambda>=maxLambda) break;
   }

 prog->Pop();
 return Sqrt(residual);
}

void SparseLM::GetParaA(nat32 ind,Vector<real64> & out)
{
 out = *(paraA[ind]->para);
}

void SparseLM::GetParaB(nat32 ind,Vector<real64> & out)
{
 out = *(paraB[ind]->para);
}

void SparseLM::MakeJacobians()
{
 PairNode * targ = list;
 while (targ)
 {
  // Jacobian A...
   *tempParaA = paraA[targ->a]->para[0];
   for (nat32 c=0;c<targ->aJacob->Cols();c++)
   {
    real64 delta = Max(Abs(10e-4 * (*tempParaA)[c]),10e-6);
    (*tempParaA)[c] += delta;
      
    (targ->F)(*tempParaA,*paraB[targ->b]->para,*targ->m,*tempErr);
    delta = 1.0/delta;   
       
    for (nat32 r=0;r<targ->aJacob->Rows();r++)
    {
     (*targ->aJacob)[r][c] = ((*tempErr)[r] - (*targ->err)[r])*delta;
    }
    
    (*tempParaA)[c] = paraA[targ->a]->para[0][c];
   }

  // B Jacobian...
   *tempParaB = *paraB[targ->b]->para;
   for (nat32 c=0;c<targ->bJacob->Cols();c++)
   {
    real64 delta = Max(Abs(10e-4 * (*tempParaB)[c]),10e-6);
    (*tempParaB)[c] += delta;
    
    (targ->F)(*paraA[targ->a]->para,*tempParaB,*targ->m,*tempErr);       
    delta = 1.0/delta;        
    for (nat32 r=0;r<targ->bJacob->Rows();r++)
    {
     (*targ->bJacob)[r][c] = ((*tempErr)[r] - (*targ->err)[r])*delta;
    }
    
    (*tempParaB)[c] = (*paraB[targ->b]->para)[c];
   }
   
  targ = targ->next;
 }
}

void SparseLM::NonLambdaWork()
{
 // Calculate all the intermediate values that do not contain lambda, i.e. only have
 // to be done each primary loop instead of each secondry loop...
  PairNode * targ;
  // Pass through the a list...
   for (nat32 i=0;i<paraA.Size();i++)
   {
    Zero(*paraA[i]->uv);
    for (nat32 j=0;j<paraA[i]->e->Size();j++) (*paraA[i]->e)[j] = 0.0;
    
    targ = listA[i];
    while (targ)
    {
     if (targ->covarInv)
     {
      // Calculate common factor...
       TransMult(*targ->aJacob,*targ->covarInv,*tempAerr);
      
      // Calculate U...
       Mult(*tempAerr,*targ->aJacob,*tempU);
       (*paraA[i]->uv) += *tempU;
     
      // Calculate e_a...   
       MultVect(*tempAerr,*targ->err,*tempParaA);
       (*paraA[i]->e) += *tempParaA;
     }
     else
     {
      // Calculate U...
       TransMult(*targ->aJacob,*targ->aJacob,*tempU);
       (*paraA[i]->uv) += *tempU;
       
      // Calculate e_a...   
       TransMultVect(*targ->aJacob,*targ->err,*tempParaA);
       (*paraA[i]->e) += *tempParaA;
     } 
     targ = targ->nextA;
    }
   }
  
  // Pass through the b list...
   for (nat32 i=0;i<paraB.Size();i++)
   {
    Zero(*paraB[i]->uv);
    for (nat32 j=0;j<paraB[i]->e->Size();j++) (*paraB[i]->e)[j] = 0.0;
    
    targ = listB[i];
    while (targ)
    {
     if (targ->covarInv)
     {
      // Calculate common factor...
       TransMult(*targ->bJacob,*targ->covarInv,*tempBerr);
      
      // Calculate V...
       Mult(*tempBerr,*targ->bJacob,*tempV);
       (*paraB[i]->uv) += *tempV;
     
      // Calculate e_a...   
       MultVect(*tempBerr,*targ->err,*tempParaB);
       (*paraB[i]->e) += *tempParaB;
     }
     else
     {
      // Calculate V...
       TransMult(*targ->bJacob,*targ->bJacob,*tempV);
       (*paraB[i]->uv) += *tempV;
       
      // Calculate e_a...   
       TransMultVect(*targ->bJacob,*targ->err,*tempParaB);
       (*paraB[i]->e) += *tempParaB;            
     }
     targ = targ->nextB;
    }
   }
  
  // Pass through the error set...
   targ = list;
   while (targ)
   {
    if (targ->covarInv)
    {
     TransMult(*targ->aJacob,*targ->covarInv,*tempU);
     Mult(*tempU,*targ->bJacob,*targ->w);
    }
    else
    {
     TransMult(*targ->aJacob,*targ->bJacob,*targ->w);
    }    
    targ = targ->next;
   }
}

void SparseLM::MakePara(real64 lambda)
{
 // Calculate the only intermediate value dependent on lambda, Y...
 {
  PairNode * targ = list;
  while (targ)
  {
   *tempV = *paraB[targ->b]->uv;
   for (nat32 i=0;i<sizeB;i++) (*tempV)[i][i] *= 1.0 + lambda;
   Inverse(*tempV,*tempV2);
   Mult(*targ->w,*tempV,*targ->y);   
   targ = targ->next;
  }
 }


 // Construct S...
  Zero(*s);  
  // The main bit, with different code for the diagonals..
   for (nat32 br=0;br<paraA.Size();br++)
   {
    for (nat32 bc=0;bc<paraA.Size();bc++)
    {
     nat32 baseR = br*sizeA;
     nat32 baseC = bc*sizeA;

     if (bc==br)
     {
      // Diagonal...
       // Sum in augmented U...
        for (nat32 r=0;r<sizeA;r++)
        {
         for (nat32 c=0;c<sizeA;c++) (*s)[baseR+r][baseC+c] = (*paraA[bc]->uv)[r][c];
        }
        for (nat32 i=0;i<sizeA;i++) (*s)[baseR+i][baseC+i] *= 1.0 + lambda;
        
       // Subtract the combinations for each point...
        PairNode * targ = listA[br];
        while (targ)
        {
         MultTrans(*targ->y,*targ->w,*tempU);
         for (nat32 r=0;r<sizeA;r++)
         {
          for (nat32 c=0;c<sizeA;c++) (*s)[baseR+r][baseC+c] -= (*tempU)[r][c];
         }
         targ = targ->nextA;
        }
     }
     else
     {
      // Off-diagonal - subtract point combinations...
       PairNode * targY = listA[br];
       PairNode * targW = listA[bc];
       while (targY&&targW)
       {
        if (targY->b==targW->b)
        {
         // We have a match - multiply and subtract...
          MultTrans(*targY->y,*targW->w,*tempU);
          for (nat32 r=0;r<sizeA;r++)
          {
           for (nat32 c=0;c<sizeA;c++) (*s)[baseR+r][baseC+c] -= (*tempU)[r][c];
          }
         targY = targY->nextA;
         targW = targW->nextA;         
        }
        else
        {
         if (targY->b<targW->b) targY = targY->nextA;
                           else targW = targW->nextA;
        }
       }       
     }
    }
   }


 // Construct es...
  for (nat32 i=0;i<paraA.Size();i++)
  {
   nat32 base = i*sizeA;
   for (nat32 j=0;j<sizeA;j++) (*es)[base+j] = (*paraA[i]->e)[j];
   
   PairNode * targ = listA[i];
   while (targ)
   {
    MultVect(*targ->y,*paraB[targ->b]->e,*tempParaA);    
    for (nat32 j=0;j<sizeA;j++) (*es)[base+j] -= (*tempParaA)[j];
    targ = targ->nextA;
   }
  }


 // From S & es calculate the deltas for the first list...
 // (A pseudo inverse and inline multiply.)
  PseudoInverse(*s,*pimt);
  for (nat32 i=0;i<paraA.Size();i++)
  {
   nat32 base = i*sizeA;
   Vector<real64> & targ = *paraA[i]->paraNew;
   for (nat32 j=0;j<sizeA;j++)
   {
    targ[j] = 0.0;
    for (nat32 k=0;k<s->Cols();k++) targ[j] += (*s)[base+j][k] * (*es)[k];
   }
  }
 
 
 // Calculate and apply the second list deltas in one swoop...
  for (nat32 i=0;i<paraB.Size();i++)
  {
   // Fill a vector with the relevent starting vector...
    *tempParaB = *paraB[i]->e;
    
   // Subtract the W's by the a list deltas...
    PairNode * targ = listB[i];
    while (targ)
    {
     TransMultVect(*targ->w,*paraA[targ->a]->paraNew,*paraB[i]->paraNew);
     *tempParaB -= *paraB[i]->paraNew;    
     targ = targ->nextB;
    }
   
   // Multiply by V^*-1, to get the b list deltas...
    *tempV = *paraB[i]->uv;
    for (nat32 j=0;j<sizeB;j++) (*tempV)[j][j] *= 1.0 + lambda;    
    Inverse(*tempV,*tempV2);
    
    MultVect(*tempV,*tempParaB,*paraB[i]->paraNew);
    *paraB[i]->paraNew += *paraB[i]->para;
    if (paraB[i]->C) (*paraB[i]->C)(*paraB[i]->paraNew);
  }
 
 
 // Apply the first list deltas...
  for (nat32 i=0;i<paraA.Size();i++)
  {
   *paraA[i]->paraNew += *paraA[i]->para;   
   if (paraA[i]->C) (*paraA[i]->C)(*paraA[i]->paraNew);
  }
}

//------------------------------------------------------------------------------
 };
};
