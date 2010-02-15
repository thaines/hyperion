//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines
#include "eos/math/stats_dir.h"

#include "eos/math/bessel.h"
#include "eos/math/iter_min.h"
#include "eos/file/csv.h"
#include "eos/alg/solvers.h"
#include "eos/alg/nearest.h"


namespace eos
{
 namespace math
 {
//------------------------------------------------------------------------------
real32 Fisher::InvAk(real32 x,real32 accuracy,nat32 limit)
{
 LogTime("eos::math::Fisher::InvAk");
 if (IsZero(x)) return 0.0;
 
 // There is no analytical method that I know of - have to resort to Newton 
 // iterations.
 
 // Initialise with a sigmoid, as that matches the shape reasonably well...
 // Sigmoid is x=tanh(k/4)
  real32 ret = 4.0*InvTanh(x);

 // Iterate till convergance/limit reached...
  for (nat32 i=0;i<limit;i++)
  {
   real32 coth = Coth(ret);
   real32 invRet = 1.0/ret;
   real32 fun = coth - invRet - x;
   real32 dFun = 1.0 - math::Sqr(coth) + math::Sqr(invRet);
   real32 delta = -fun/dFun;
   
   ret += delta;
   if (Abs(delta)<accuracy) break;
  }
  
 return ret;
}

void Fisher::Sample(Vect<3,real32> & out,data::Random & rand) const
{
 LogTime("eos::math::Fisher::Sample");
 // Get concentration...
  real32 k = Length();
  
 // We work in spherical coordinates from the direction of the distribution,
 // the coordinates are independent, with the one moving away from the direction
 // being hard to sample and the one along contours with the same angular 
 // difference from the direction being easy.
 
 // To sample the angle from distribution direction we use the metropolis method...
  real32 theta = pi * rand.NormalInc();
  if (!math::IsZero(k))
  {
   real32 theta_prob = Exp(k*Cos(theta)) * Sin(theta);
   nat32 accepts = 48;
  
   while (accepts)
   {
    real32 theta_new = theta + rand.Gaussian(pi*0.25);
    real32 theta_new_prob = Exp(k * Cos(theta_new)) * Sin(theta_new);
    real32 ratio = theta_new_prob / theta_prob;
    
    if ((ratio>=1.0)||(ratio>rand.Normal()))
    {
     theta = theta_new;
     theta_prob = theta_new_prob;
     --accepts;
    }
   }
  }
  
 // To sample from the equal angle contours is easy...
  real32 phi = 2.0 * pi * rand.Normal();
  
  
 // Now we have our spherical coordinate we need to create a vector, this is
 // kinda fiddly...
  Vect<3,real32> self = *this;
  self.Normalise();
  
  Vect<3,real32> perp1,perp2;
  Perpendicular(self,perp1);
  CrossProduct(self,perp1,perp2);
  
  real32 p1m = Sin(theta) * Cos(phi);
  real32 p2m = Sin(theta) * Sin(phi);
  real32 sm  = Cos(theta);
  
  for (nat32 i=0;i<3;i++) out[i] = p1m*perp1[i] + p2m*perp2[i] + sm*self[i];
}

//------------------------------------------------------------------------------
void FisherBingham::Convolve(real32 k)
{
 LogTime("eos::math::FisherBingham::Convolve");

 // Convert to a sum of Fisher distributions...
  static const nat32 fish_count = 8; // Number of Fisher distributions to use.
  Fisher fish[fish_count];
  ToFisher(fish_count,fish);


 // Convolve the individual Fisher distributions...
 real32 fish_weight[fish_count];
 //LogDebug("Conv parameter" << LogDiv() << k);
 {
  //LogTime("eos::math::FisherBingham::Convolve convolve");
  for (nat32 i=0;i<fish_count;i++)
  {
   //LogDebug("fish pre conv" << LogDiv() << fish[i]);
   fish_weight[i] = fish[i].ConvolveRatio(k);
   //LogDebug("fish post conv" << LogDiv() << fish[i]);
   //LogDebug("fish post conv weight" << LogDiv() << fish_weight[i]);
  }
 }



 // Refit the FisherBingham distribution to the now convolved sum of Fisher distributions...
  // Find mean of Fisher distributions, subtract it from each of them...
   Vect<3,real32> mean(0.0);
   real32 weight = 0.0;
   {
    //LogTime("eos::math::FisherBingham::Convolve mean");   
    for (nat32 i=0;i<fish_count;i++)
    {
     for (nat32 j=0;j<3;j++) mean[j] += fish_weight[i]*fish[i][j];
     weight += fish_weight[i];
    }
    if (math::IsZero(weight))
    {
     // Whole thing will break down - zero it out as the result must by default be the uniform distribution...
      fisher = Fisher();
      bingham = Bingham();
     return;
    }
    mean /= weight;
   
    for (nat32 i=0;i<fish_count;i++) fish[i] -= mean;
   }


  // Do principal component analysis to find the rotation matrix...
   Mat<3,3,real32> rot;
   Vect<3,real32> diag;  
   {
    //LogTime("eos::math::FisherBingham::Convolve pca");
    Mat<3,3,real32> covar;
    Zero(covar);
    for (nat32 i=0;i<fish_count;i++)
    {
     for (nat32 r=0;r<3;r++)
     {
      for (nat32 c=0;c<3;c++)
      {
       covar[r][c] += fish_weight[i]*fish[i][r]*fish[i][c];
      }
     }
    }
    covar /= weight;
   

    SymEigen(covar,rot,diag);
   }


  // Rotate the zero-meaned sum of Fisher distributions into the rotational arrangment...
  {
   //LogTime("eos::math::FisherBingham::Convolve rotate");
   for (nat32 i=0;i<fish_count;i++)
   {
    Fisher temp;
    TransMultVect(rot,fish[i],temp);
    fish[i] = temp;
   }
  }


  // Calculate the Fisher distribution in the rotated coordinate system by
  // equating the axes of the distributions - a nice simple equation.
  // Subtract this new Fisher distribution from the sum of Fisher distributions,
  // so we have the effect of both means (One rotated.)...
   Vect<3,real32> rotFish;
   {
    //LogTime("eos::math::FisherBingham::Convolve fit:1");
    Vect<3,real32> dp(0.0);
    Vect<3,real32> dn(0.0);
    for (nat32 i=0;i<fish_count;i++)
    {
     for (nat32 j=0;j<3;j++)
     {
      dp[j] += fish_weight[i]*Exp(fish[i][j]);
      dn[j] += fish_weight[i]*Exp(-fish[i][j]);
     }
    }
    
    for (nat32 j=0;j<3;j++)
    {
     dp[j] = Ln(dp[j]/weight);
     dn[j] = Ln(dn[j]/weight);
     rotFish[j] = dp[j] - 0.5*(dp[j]+dn[j]);
    }
    
    for (nat32 i=0;i<fish_count;i++) fish[i] -= rotFish;
   }


  // Do the above again, but this time keep the Bingham distribution diagonal as
  // well as the Fisher...
   Vect<3,real32> rotBing;
   {
    //LogTime("eos::math::FisherBingham::Convolve fit:2");
    Vect<3,real32> dp(0.0);
    Vect<3,real32> dn(0.0);
    for (nat32 i=0;i<fish_count;i++)
    {
     for (nat32 j=0;j<3;j++)
     {
      dp[j] += fish_weight[i]*Exp(fish[i][j]);
      dn[j] += fish_weight[i]*Exp(-fish[i][j]);
     }
    }
    
    for (nat32 j=0;j<3;j++)
    {
     dp[j] = Ln(dp[j]/weight);
     dn[j] = Ln(dn[j]/weight);
     rotBing[j] = 0.5*(dp[j]+dn[j]);
     rotFish[j] += dp[j] - rotBing[j];
    }
   }
   
  // Zero mean the rotBing vector - adding/summing values has no effect, zero 
  // meaning reduces the scales involved and keeps the numbers sane...
  {
   real32 offset = 0.0;
   for (nat32 i=0;i<3;i++) offset += rotBing[i];
   offset /= 3.0;
   for (nat32 i=0;i<3;i++) rotBing[i] -= offset;
  }


  // Use the information gleaned to derive the final FisherBingham distribution...
  {
   //LogTime("eos::math::FisherBingham::Convolve extraction");
   MultVect(rot,rotFish,fisher);
   fisher += mean;
   
   Mat<3,3,real32> temp;
   MultDiag(rot,rotBing,temp);
   MultTrans(temp,rot,bingham);
  }
}

real32 FisherBingham::NormDiv() const
{
 LogTime("eos::math::FisherBingham::NormDiv");

 // Decleration of storage structure and of the K function and its 
 // first differential==1 solver...
  struct
  {
   math::Vect<3> fish;
   math::Vect<3> bing;
   
   real32 K0(real32 t) const
   {
    real32 ret = 0.0;
    for (nat32 i=0;i<3;i++)
    {
     ret -= 0.5*math::Ln(1.0 - t/bing[i]);
     ret += 0.25 * math::Sqr(fish[i])/(bing[i]-t);
     ret -= 0.25 * math::Sqr(fish[i])/bing[i];
    }
    return ret;
   }
   
   real32 K1(real32 t) const
   {
    real32 ret = 0.0;
    for (nat32 i=0;i<3;i++)
    {
     ret += 0.5/(bing[i]-t);
     ret += 0.25*math::Sqr(fish[i])/math::Sqr(bing[i]-t);
    }
    return ret;    
   }
   
   real32 K2(real32 t) const
   {
    real32 ret = 0.0;
    for (nat32 i=0;i<3;i++)
    {
     ret += 0.5/math::Sqr(bing[i]-t);
     ret += 0.5*math::Sqr(fish[i])/math::Cube(bing[i]-t);
    }
    return ret; 
   }
   
   real32 Kn(nat32 n,real32 t) const
   {
    real32 ret = 0.0;
    for (nat32 i=0;i<3;i++)
    {
     ret += (0.5*math::Factorial(n-1))/math::Pow(bing[i]-t,real32(n));
     ret += (math::Factorial(n)*math::Sqr(fish[i]))/(4.0*math::Pow(bing[i]-t,real32(n+1)));
    }
    return ret; 
   }
   
   real32 FindT() const
   {
    // Find the lower bound of the answer and use that as initialisation - this
    // makes sure we will always converge to the wanted location...
     real32 minBing = bing[0];
     real32 minBingFish = fish[0];
     for (nat32 i=1;i<3;i++)
     {
      if (bing[i]<minBing)
      {
       minBing = bing[i];
       minBingFish = fish[i];
      }
     }
     real32 maxFishSqr = math::Max(math::Sqr(fish[0]),math::Sqr(fish[1]),math::Sqr(fish[2]));
    
     real32 ret = minBing - 0.25*3.0 - 0.5*math::Sqrt(0.25*math::Sqr(3.0) + 3.0*maxFishSqr);


    // Find the upper bound, for safety...
     real32 upperBound = minBing - 0.25 - 0.5*math::Sqrt(0.25 + math::Sqr(minBingFish));


    // Do newton iterations...
     static const nat32 limit = 1000;
     static const real32 err = 1e-6;
     for (nat32 i=0;i<limit;i++)
     {
      real32 offset = (K1(ret) - 1.0)/K2(ret);
      ret -= offset;
      ret = math::Min(ret,upperBound);
      if (math::Abs(offset)<err) break;
     }
    
    return ret;
   }
  
  } store;



 // First rotate the distribution to make it diagonal, and arrange for the 
 // Bingham component to be positive definite...
 {
  Mat<3,3,real32> rot;
  Mat<3,3,real32> temp = static_cast< Mat<3,3,real32> >(bingham);
  temp *= -1.0; // Paper this is based on uses a different definition of the Fisher-Bingham dist.
  SymEigen(temp,rot,store.bing);
  
  TransMultVect(rot,fisher,store.fish);
 }


 // Calculate various needed parameters of the final calculation...
  real32 t = store.FindT();
  real32 k2 = store.K2(t);
  real32 T = store.Kn(4,t)/(8.0*math::Sqr(k2));
  T -= (5.0/24.0)*math::Sqr(store.Kn(3,t)/math::Pow(k2,real32(1.5)));
   
  real32 sum = 0.0;
  for (nat32 i=0;i<3;i++) sum += math::Sqr(store.fish[i])/(store.bing[i] - t);
  
      
 // Calculate the answer...
  real32 ans = math::Sqrt(2.0)*math::pi/math::Sqrt(k2);
  for (nat32 i=0;i<3;i++) ans /= math::Sqrt(store.bing[i] - t);
  ans *= math::Exp(T + 0.25*sum - t);


 return ans;
}

void FisherBingham::Maximum(Vect<3> & out,real32 err,nat32 limit) const
{
 LogTime("eos::math::FisherBingham::Maximum");

 // Geometrically inspired solution - transilate the problem to finding the 
 // closest point on an ellipsoid, which has an efficient solution...
  // Rotate so that we are dealing with a diagonal matrix...
   Vect<3,real32> bing;
   Mat<3,3,real32> rot;
   {
    Mat<3,3,real32> temp = static_cast< Mat<3,3,real32> >(bingham);
    SymEigen(temp,rot,bing);
   
    real32 maxBing = math::Max(bing[0],bing[1],bing[2]);
    bing -= maxBing + 0.5;
   }
  
   Vect<3,real32> fish;
   TransMultVect(rot,fisher,fish);


  // Solve... (Special case there being no Fisher distribution)
   if (math::IsZero(fish.LengthSqr()))
   {
    nat32 maxInd = 0;
    for (nat32 i=1;i<3;i++)
    {
     if (bing[i]>bing[maxInd]) maxInd = i;
    }

    for (nat32 i=0;i<3;i++) out[i] = 0.0;
    out[maxInd] = 1.0;
   }
   else
   {
    // Calculate the mean and diagonal covariance matrix of the equivalent conditioned normal distribution...
     //LogDebug("{fish,bing}" << LogDiv() << fish << LogDiv() << bing);
    
     Vect<3> covar;
     Vect<3> mean;
     for (nat32 i=0;i<3;i++)
     {
      covar[i] = -0.5/bing[i];
      mean[i] = covar[i] * fish[i];
     }
     //LogDebug("{covar,mean}" << LogDiv() << covar << LogDiv() << mean);
    
    // Calculate the parameters of the relevant ellipsoid and point for solving the nearest point problem...
     Vect<3> ellipsoid;
     Vect<3> point;
     for (nat32 i=0;i<3;i++)
     {
      ellipsoid[i] = math::Sqrt(1.0/covar[i]);
      point[i] = ellipsoid[i] * mean[i];
     }
     //LogDebug("{ellipsoid,point}" << LogDiv() << ellipsoid << LogDiv() << point);
    
    // Solve the nearest point problem...
     alg::PointEllipsoid(ellipsoid,point,out,err,limit);
     
     //LogDebug("{out}" << LogDiv() << out);
    
    // Translate back from ellipsoid coordinates to unit sphere coordinates, 
    // and normalise to handle any numerical error...
     for (nat32 i=0;i<3;i++) out[i] = out[i]/ellipsoid[i];
     out.Normalise();
     
     //LogDebug("{norm out}" << LogDiv() << out);
   }


  // Remove the rotation...
  {
   Vect<3,real32> temp = out;
   MultVect(rot,temp,out);
  }
}

nat32 FisherBingham::Critical(Vect<3> out[6],real32 err,nat32 limit) const
{
 LogTime("eos::math::FisherBingham::Critical");

 // Geometrically inspired solution - transilate the problem to finding the 
 // closest point on an ellipsoid, which has an efficient solution...
  // Rotate so that we are dealing with a diagonal matrix...
   Vect<3,real32> bing;
   Mat<3,3,real32> rot;
   {
    Mat<3,3,real32> temp = static_cast< Mat<3,3,real32> >(bingham);
    SymEigen(temp,rot,bing);
   
    real32 maxBing = math::Max(bing[0],bing[1],bing[2]);
    bing -= maxBing + 0.5;
   }
  
   Vect<3,real32> fish;
   TransMultVect(rot,fisher,fish);


  // Solve... (Special case there being no Fisher distribution)
   nat32 ret = 6;
   if (math::IsZero(fish.LengthSqr()))
   {
    out[0][0] =  1.0; out[0][1] =  0.0; out[0][2] =  0.0;
    out[1][0] = -1.0; out[1][1] =  0.0; out[1][2] =  0.0;
    out[2][0] =  0.0; out[2][1] =  1.0; out[2][2] =  0.0;
    out[3][0] =  0.0; out[3][1] = -1.0; out[3][2] =  0.0;
    out[4][0] =  0.0; out[4][1] =  0.0; out[4][2] =  1.0;
    out[5][0] =  0.0; out[5][1] =  0.0; out[5][2] = -1.0;
   }
   else
   {
    // Calculate the mean and diagonal covariance matrix of the equivalent conditioned normal distribution...
     Vect<3> covar;
     Vect<3> mean;
     for (nat32 i=0;i<3;i++)
     {
      covar[i] = -0.5/bing[i];
      mean[i] = covar[i] * fish[i];
     }
    
    // Calculate the parameters of the relevant ellipsoid and point for solving the nearest point problem...
     Vect<3> ellipsoid;
     Vect<3> point;
     for (nat32 i=0;i<3;i++)
     {
      ellipsoid[i] = math::Sqrt(1.0/covar[i]);
      point[i] = ellipsoid[i] * mean[i];
     }
    
    // Find all points where the ellipsoid normal points at the point...
     ret = alg::PointEllipsoidDir(ellipsoid,point,out,err,limit);
    
    // Translate back from ellipsoid coordinates to unit sphere coordinates, 
    // and normalise to handle any numerical error...
     for (nat32 j=0;j<ret;j++)
     {
      for (nat32 i=0;i<3;i++) out[j][i] = out[j][i]/ellipsoid[i];
      out[j].Normalise();
     }
   }


  // Remove the rotation...
   for (nat32 i=0;i<ret;i++)
   {
    Vect<3,real32> temp = out[i];
    MultVect(rot,temp,out[i]);
   }
  
 return ret;
}

nat32 FisherBingham::Maximums(Vect<3> out[2]) const
{
 LogTime("eos::math::FisherBingham::Maximums");

 // Decompose the distribution into a number of Fisher distributions - these 
 // make ideal starting directions for the iterations despite the complexity of
 // calculation...
  static const nat32 samples = 4;
  Fisher fish[samples];
  ToFisher(samples,fish);


 // Get the fisher and bingham as vectors after rotation, plus the rotation of course...
  Vect<6> vals;
  Mat<3,3,real32> rot;
  {
   Vect<3,real32> bing;
   Mat<3,3,real32> temp = static_cast< Mat<3,3,real32> >(bingham);
   SymEigen(temp,rot,bing);
  
   Vect<3,real32> fish;
   TransMultVect(rot,fisher,fish);
      
   for (nat32 i=0;i<3;i++)
   {
    vals[i]   = fish[i];
    vals[i+3] = bing[i];
   }
  }


 // Iterate the fisher distributions and use each as the starting point to 
 // iterate to the nearest maxima - this produces a set of maxima directions...
  Vect<3> dir[samples];
  for (nat32 i=0;i<samples;i++)
  {
   if (math::IsZero(fish[i].LengthSqr()))
   {
    dir[i][0] = 1.0;
    dir[i][1] = 0.0;
    dir[i][2] = 0.0;
   }
   else
   {
    TransMultVect(rot,fish[i],dir[i]);
    dir[i].Normalise();
    
    // Convert to spherical coordinates...
     math::Vect<2> spherical;
     spherical[0] = InvCos(dir[i][2]);
     spherical[1] = InvTan2(dir[i][1],dir[i][0]);
    
    // Optimise with LM...
     struct
     {
      static void Func(const math::Vect<2> & spherical,Vect<2> & err,const Vect<6> & vals)
      {
       // Fill error with the differentials of each direction...
        real32 sin0 = Sin(spherical[0]);
        real32 cos0 = Cos(spherical[0]);
        real32 sin1 = Sin(spherical[1]);
        real32 cos1 = Cos(spherical[1]);
       
        err[0] = (vals[0]*cos1 + vals[1]*sin1)*cos0 - vals[2]*sin0 +
                 2.0*(vals[3]*Sqr(cos1) + vals[4]*Sqr(sin1) - vals[5])*sin0*cos0;
        err[1] = vals[1]*sin0*cos1 - vals[0]*sin0*sin1 + 2.0*(vals[4]-vals[3])*Sqr(sin0)*sin1*cos1;
      }
     } func;
     
     LM(spherical,vals,func.Func);
    
    // Convert back to a direction vector...
     Vect<3> temp;
     temp[0] = Sin(spherical[0]) * Cos(spherical[1]);
     temp[1] = Sin(spherical[0]) * Sin(spherical[1]);
     temp[2] = Cos(spherical[0]);
     MultVect(rot,temp,dir[i]);
   }
  }


 // Go through the maximas found and merge when they are the same maxima, to
 // produce the two best maxima that arn't the same. Return the number found...
  real32 val[samples];
  for (nat32 i=0;i<samples;i++) val[i] = UnnormProb(dir[i]);
  
  // Find maximum...
   nat32 maxInd = 0;
   for (nat32 i=1;i<samples;i++)
   {
    if (val[maxInd]<val[i]) maxInd = i;
   }
   
  // Remove from equation those that are too close to the maximum...
  // (Too close being defined as less than 2 degrees.)
   for (nat32 i=0;i<6;i++)
   {
    if (math::InvCos(dir[i]*dir[maxInd])<(math::pi/90.0)) val[i] = -1.0;
   }

  // Find maximum, again...
   nat32 maxInd2 = 0;
   for (nat32 i=1;i<samples;i++)
   {
    if (val[maxInd2]<val[i]) maxInd2 = i;
   }
  
  // Return, checking 2nd maximum isn't negative which would indicate there being only 1 maxima...
   out[0] = dir[maxInd];
   out[1] = dir[maxInd2];
   if (val[maxInd2]<0.0) return 1;
                    else return 2;
}

void FisherBingham::ToFisher(nat32 num,Fisher * out) const
{
 LogTime("eos::math::FisherBingham::ToFisher");
 
 // Decompose the Bingham distribution, note the indices of the alpha, beta and minima...
  Mat<3,3,real32> rot;
  Vect<3,real32> diag;
  {
   Mat<3,3,real32> bing = bingham;
   SymEigen(bing,rot,diag);
  }
    
  nat32 min_ind = 0;
  for (nat32 i=1;i<3;i++) {if (diag[i]<diag[min_ind]) min_ind = i;}
  nat32 alpha_ind = (min_ind+1)%3;
  nat32 beta_ind  = (min_ind+2)%3;
    
  diag[alpha_ind] -= diag[min_ind];
  diag[beta_ind] -= diag[min_ind];
  diag[min_ind] = 0.0;


 // Calculate the two multipliers - this is the slow bit...
  real32 alpha_mult = InverseModBesselFirstOrder0(Exp(diag[alpha_ind]));
  real32 beta_mult = InverseModBesselFirstOrder0(Exp(diag[beta_ind]));
   
 // Iterate and fill in each Fisher distribution from the sum in turn...
  for (nat32 i=0;i<num;i++)
  {
   // Create it without the rotation...
    real32 ang = 2.0*pi * real32(i)/real32(num);
    out[i][alpha_ind] = alpha_mult * Cos(ang);
    out[i][beta_ind] = beta_mult * Sin(ang);
    out[i][min_ind] = 0.0;
 
   // Apply the rotation, sum in the Fisher component...
    out[i].Rotate(rot);
    out[i] *= fisher;
  }
}

//------------------------------------------------------------------------------
 };
};
