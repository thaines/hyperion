//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines
#include "eos/cam/homography.h"

#include "eos/math/iter_min.h"

namespace eos
{
 namespace cam
 {
//------------------------------------------------------------------------------
Homography2D::Homography2D()
{}

Homography2D::~Homography2D()
{}

void Homography2D::Reset()
{
 data.Reset();
}

void Homography2D::Add(const math::Vect<2,real32> & f,const math::Vect<2,real32> & s)
{
 Pair<math::Vect<2,real64>,math::Vect<2,real64> > ne;
  ne.first[0] = f[0];
  ne.first[1] = f[1];
  ne.second[0] = s[0];
  ne.second[1] = s[1]; 
 data.AddBack(ne);	
}

void Homography2D::Add(const math::Vect<2,real64> & f,const math::Vect<2,real64> & s)
{
 Pair<math::Vect<2,real64>,math::Vect<2,real64> > ne;
  ne.first[0] = f[0];
  ne.first[1] = f[1];
  ne.second[0] = s[0];
  ne.second[1] = s[1]; 
 data.AddBack(ne);	
}

void Homography2D::Add(const math::Vect<3,real32> & f,const math::Vect<3,real32> & s)
{
 Pair<math::Vect<2,real64>,math::Vect<2,real64> > ne;
  ne.first[0] = f[0]/f[2];
  ne.first[1] = f[1]/f[2];
  ne.second[0] = s[0]/s[2];
  ne.second[1] = s[1]/s[2];
 data.AddBack(ne);	
}

void Homography2D::Add(const math::Vect<3,real64> & f,const math::Vect<3,real64> & s)
{
 Pair<math::Vect<2,real64>,math::Vect<2,real64> > ne;
  ne.first[0] = f[0]/f[2];
  ne.first[1] = f[1]/f[2];
  ne.second[0] = s[0]/s[2];
  ne.second[1] = s[1]/s[2];
 data.AddBack(ne);	
}

real64 Homography2D::Result(math::Mat<3,3,real32> & out)
{
 math::Mat<3,3,real64> temp;
 real64 ret = Result(temp);
 for (nat32 r=0;r<3;r++)
 {
  for (nat32 c=0;c<3;c++) out[r][c] = temp[r][c];
 }
 return ret;
}

real64 Homography2D::Result(math::Mat<3,3,real64> & out)
{
 LogTime("eos::cam::Homography2D::Result");

 // First we need to get a good approximation to the answer using a standard 
 // least-squares techneque with the right null vector as the answer, calcualted
 // using SVD...
  // Find the means of the 2 point sets...
   math::Vect<2,real64> fM(0.0);
   math::Vect<2,real64> sM(0.0);
   ds::List< Pair<math::Vect<2,real64>,math::Vect<2,real64> > >::Cursor targ = data.FrontPtr();
   nat32 n = 0;
   while (!targ.Bad())
   {
    n++;

    fM[0] += (targ->first[0]-fM[0])/real32(n);
    fM[1] += (targ->first[1]-fM[1])/real32(n);
    sM[0] += (targ->second[0]-sM[0])/real32(n);
    sM[1] += (targ->second[1]-sM[1])/real32(n);
    
    ++targ;
   }


  // Find the average distance of each point from the mean - the standard
  // deviation...
   math::Vect<2,real64> fS(0.0);
   math::Vect<2,real64> sS(0.0);
   targ = data.FrontPtr();
   n = 0;
   while (!targ.Bad())
   {
    n++;

    fS[0] += (math::Abs(targ->first[0]-fM[0])-fS[0])/real32(n);
    fS[1] += (math::Abs(targ->first[1]-fM[1])-fS[1])/real32(n);
    sS[0] += (math::Abs(targ->second[0]-sM[0])-sS[0])/real32(n);
    sS[1] += (math::Abs(targ->second[1]-sM[1])-sS[1])/real32(n);
    
    ++targ;
   }


  // Construct transforms to make each side zero-mean with an average distance of Sqrt(2) from (0,0)...
   math::Mat<3,3,real64> fTra; Zero(fTra);
    fTra[0][0] = 1.0/fS[0];
    fTra[1][1] = 1.0/fS[1];
    fTra[0][2] = -fM[0]/fS[0];
    fTra[1][2] = -fM[1]/fS[1];
    fTra[2][2] = 1.0;
    
   math::Mat<3,3,real64> sTra; Zero(sTra);
    sTra[0][0] = 1.0/sS[0];
    sTra[1][1] = 1.0/sS[1];
    sTra[0][2] = -sM[0]/sS[0];
    sTra[1][2] = -sM[1]/sS[1];
    sTra[2][2] = 1.0;


  // Build a matrix such that the answer is its right null vector...
   math::Matrix<real64> mat(math::Max(data.Size()*2,nat32(9)),9);
   targ = data.FrontPtr();
   for (nat32 i=0;i<data.Size();i++)
   {
    math::Vect<2,real64> f;  
    math::Vect<2,real64> s;
    math::MultVectEH(fTra,targ->first,f);
    math::MultVectEH(sTra,targ->second,s);	   
	   
    mat[i*2][0] = f[0];                 
    mat[i*2][1] = f[1];                 
    mat[i*2][2] = 1.0;                            
    mat[i*2][3] = 0.0;                            
    mat[i*2][4] = 0.0;                            
    mat[i*2][5] = 0.0;                            
    mat[i*2][6] = -s[0]*f[0];
    mat[i*2][7] = -s[0]*f[1];
    mat[i*2][8] = -s[0];               
    
    mat[i*2+1][0] = 0.0;                            
    mat[i*2+1][1] = 0.0;                            
    mat[i*2+1][2] = 0.0;                            
    mat[i*2+1][3] = f[0];                 
    mat[i*2+1][4] = f[1];                 
    mat[i*2+1][5] = 1.0;                            
    mat[i*2+1][6] = -s[1]*f[0];
    mat[i*2+1][7] = -s[1]*f[1];
    mat[i*2+1][8] = -s[1];               
    
    ++targ;
   }
   
   for (nat32 r=data.Size()*2;r<9;r++)
   {
    for (nat32 c=0;c<9;c++) mat[r][c] = 0.0;
   }

   
  // Get the right null space of our big fat matrix...
   math::Vect<9,real64> ansV;
   RightNullSpace(mat,ansV);
  
  // Extract and de-normalise the calculated matrix...
   math::Mat<3,3,real64> t1;
   math::Mat<3,3,real64> t2;
   
   t1[0][0] = ansV[0]; t1[0][1] = ansV[1]; t1[0][2] = ansV[2];
   t1[1][0] = ansV[3]; t1[1][1] = ansV[4]; t1[1][2] = ansV[5];
   t1[2][0] = ansV[6]; t1[2][1] = ansV[7]; t1[2][2] = ansV[8];
   
   Inverse(sTra,t2); // Yes, this is a major waste of cpu cycles. Too lazy.
   Mult(sTra,t1,t2);
   Mult(t2,fTra,out);


 // Now use Levenberg Marquardt to minimise stuff correctly, and get a good 
 // answer. We are minimising just the 9 parameters of the matrix, so as to
 // reduce the manhattan distance of all points after the transformation.
  math::Vector<real64> aV(9);
   aV[0] = out[0][0]; aV[1] = out[0][1]; aV[2] = out[0][2];
   aV[3] = out[1][0]; aV[4] = out[1][1]; aV[5] = out[1][2];
   aV[6] = out[2][0]; aV[7] = out[2][1]; aV[8] = out[2][2];
   
  real64 ret = LM(2*data.Size(),aV,*this,&LMfunc);


 // And one last bit of code to copy from the vector to the matrix...
  out[0][0] = aV[0]; out[0][1] = aV[1]; out[0][2] = aV[2];
  out[1][0] = aV[3]; out[1][1] = aV[4]; out[1][2] = aV[5];
  out[2][0] = aV[6]; out[2][1] = aV[7]; out[2][2] = aV[8];
 
 return ret;
}

void Homography2D::LMfunc(const math::Vector<real64> & pv,math::Vector<real64> & err,const Homography2D & self)
{
 ds::List< Pair<math::Vect<2,real64>,math::Vect<2,real64> > >::Cursor targ = self.data.FrontPtr();
 for (nat32 i=0;i<self.data.Size();i++)
 {
  // Transform the first point by the given 'matrix' to get something that
  // should be equal to the second point...
   real64 tx = pv[0]*targ->first[0] + pv[1]*targ->first[1] + pv[2];
   real64 ty = pv[3]*targ->first[0] + pv[4]*targ->first[1] + pv[5];
   real64 tw = pv[6]*targ->first[0] + pv[7]*targ->first[1] + pv[8];
   tx /= tw; ty /= tw;
  
  // The error vector contains the differences...
   err[i*2]   = tx - targ->second[0];
   err[i*2+1] = ty - targ->second[1];
  
  ++targ;	 
 }	
}

//------------------------------------------------------------------------------
 };
};
