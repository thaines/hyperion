//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/alg/fitting.h"

#include "eos/math/mat_ops.h"
#include "eos/math/iter_min.h"

namespace eos
{
 namespace alg
 {
//------------------------------------------------------------------------------
PlaneFit::PlaneFit()
{Reset();}

PlaneFit::~PlaneFit()
{}

void PlaneFit::Reset()
{
 Zero(mat);
 vec[0] = 0.0;
 vec[1] = 0.0;
 vec[2] = 0.0;
}

void PlaneFit::Add(const math::Vect<3> & v)
{
 mat[0][0] += v[0]*v[0]; mat[0][1] += v[0]*v[1]; mat[0][2] += v[0];
 mat[1][0] += v[0]*v[1]; mat[1][1] += v[1]*v[1]; mat[1][2] += v[1];
 mat[2][0] += v[0];      mat[2][1] += v[1];      mat[2][2] += 1.0;

 vec[0] += v[0]*v[2];
 vec[1] += v[1]*v[2];
 vec[2] += v[2];
}

void PlaneFit::Add(real32 x,real32 y,real32 z)
{
 math::Vect<3> v;
  v[0] = x;
  v[1] = y;
  v[2] = z;
 Add(v);
}

bit PlaneFit::Valid()
{
 return !math::Equal(math::Determinant(mat),0.0);
}

void PlaneFit::Get(bs::PlaneABC & out)
{
 math::Mat<3,3,real64> a = mat;
 math::Vect<3,real64> b = vec;

 SolveLinear(a,b);

 out.a = b[0];
 out.b = b[1];
 out.c = b[2];
}

//------------------------------------------------------------------------------
LinePlaneFit::LinePlaneFit()
{}

LinePlaneFit::~LinePlaneFit()
{}

void LinePlaneFit::Add(const bs::Vertex & a,const bs::Vertex & b,real32 weight)
{
  bs::Vertex aa = a; aa /= aa[3];
  bs::Vertex bb = b; bb /= bb[3];

 Entry n;
  n.c[0] = 0.5*(aa[0] + bb[0]);
  n.c[1] = 0.5*(aa[1] + bb[1]);
  n.c[2] = 0.5*(aa[2] + bb[2]);
  
  bb -= aa;
  n.length = bb.Length();

  n.weight = weight;
 data.AddBack(n);
}

void LinePlaneFit::Run()
{
 // Fill in the data matrix, normalising each vector and calculating the 
 // mean of all the points...
  math::Matrix<real32> mat(data.Size(),4);
  bs::Vert mean(0.0,0.0,0.0);
  ds::List<Entry>::Cursor targ = data.FrontPtr();
  for (nat32 i=0;i<data.Size();i++)
  {
   bs::Vert pos = targ->c;

   // Stick it in the data matrix...    
    real32 scale = math::InvSqrt(pos.LengthSqr()+1.0);

    mat[i][0] = pos[0]*scale;
    mat[i][1] = pos[1]*scale;
    mat[i][2] = pos[2]*scale;
    mat[i][3] = scale;
    
   // Add to mean value...
    pos -= mean;
    pos /= real32(i+1);
    mean += pos;
    
   ++targ;
  }


 // Second pass - calculate the covariance matrix for the data...
  math::Mat<3> covar;
  math::Zero(covar);
  targ = data.FrontPtr();
  for (nat32 i=0;i<data.Size();i++)
  {
   real32 xo = targ->c[0] - mean[0];
   real32 yo = targ->c[1] - mean[1];
   real32 zo = targ->c[2] - mean[2];
   
   real32 div = 1.0/real32(i+1);
   
   covar[0][0] += ((xo*xo)-covar[0][0])*div;
   covar[0][1] += ((xo*yo)-covar[0][1])*div;
   covar[0][2] += ((xo*zo)-covar[0][2])*div;
   covar[1][1] += ((yo*yo)-covar[1][1])*div;
   covar[1][2] += ((yo*zo)-covar[1][2])*div;
   covar[2][2] += ((zo*zo)-covar[2][2])*div;
   
   ++targ;
  }
  
  covar[1][0] = covar[0][1];
  covar[2][0] = covar[0][2];
  covar[2][1] = covar[1][2];
  
  math::Mat<3> temp;
  math::Inverse(covar,temp);
  
  

 // Third pass, apply a weighting consisting of the expectation of the data point
 // multiplied by one over the line length multiplied by the user provided weighting...
  targ = data.FrontPtr();
  for (nat32 i=0;i<data.Size();i++)
  {
   real32 mult = targ->weight;
   mult *= math::Exp(-targ->length);
   
   bs::Vert pos = targ->c;
   pos -= mean;
   
   bs::Vert pos2;
   math::MultVect(covar,pos,pos2);
   
   mult *= math::Exp(-0.5 * (pos*pos2));
   
   mat[i][0] *= mult;
   mat[i][1] *= mult;
   mat[i][2] *= mult;
   mat[i][3] *= mult;
   
   ++targ;
  }


 // Get the right null space - this is our answer...
  math::Vect<4,real32> res;
  math::RightNullSpace(mat,res);


 // Store in the output...
  result.n[0] = res[0];
  result.n[1] = res[1];
  result.n[2] = res[2];
  result.d = res[3];

  result.Normalise();
}

const bs::Plane & LinePlaneFit::Plane() const
{
 return result;
}

//------------------------------------------------------------------------------
 };
};
