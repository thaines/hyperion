//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

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


#include "fishbing/main.h"

//------------------------------------------------------------------------------
int main()
{
 using namespace eos;

 os::Con conObj;
 os::Conversation & con = *conObj.StartConversation();
 
 str::TokenTable tt;
 svt::Core core(tt);
  
  

 /*math::Vect<3> dir;
 
 dir[0] = 0.5;
 dir[1] = -1.0;
 dir[2] = 2.0;
 dir.Normalise();
 math::FisherBingham fb(dir,math::pi*0.2,30.0);
 //con << "1: fish = " << fb.fisher << "; bing = " << fb.bingham << "\n";

 dir[0] = 0.5;
 dir[1] = -0.5;
 dir[2] = 1.5;
 dir.Normalise();
 math::FisherBingham fb2(dir,math::pi*0.3,20.0);
 //con << "2: fish = " << fb2.fisher << "; bing = " << fb2.bingham << "\n";
 
 fb *= fb2;
 con << "m: fish = " << fb.fisher << "; bing = " << fb.bingham << "\n";*/
 
 // To provide profiler with sufficient info...
  /*for (nat32 i=1;i<1000;i++)
  {
   math::FisherBingham temp(fb);
   temp.Convolve(20.0);
  }*/

 //fb.Convolve(20.0);
 //con << "mc: fish = " << fb.fisher << "; bing = " << fb.bingham << "\n";

 
 // Test code for MaximiseFixedLenDualQuad...
 /*{
  data::Random rand;
  real32 a = rand.Signed();
  real32 b = rand.Signed();
  real32 c = rand.Signed();
  real32 d = rand.Signed();
  real32 n =  0.5+rand.Normal();
 
  con << "Finding maximum of " << a << "*x + " << b << "*y + " << c << "*x^2 + " << d << "*y^2 where x^2 + y^2 = " << n << "\n";
  real32 bx,by;
  alg::MaximiseFixedLenDualQuad(a,b,c,d,n,bx,by);
  con << "Maximum by function = " << (a*bx + b*by + c*math::Sqr(bx) + d*math::Sqr(by)) << "\n";
  con << "Using x = " << bx << ", y = " << by << " such that x^2+y^2 = " << (math::Sqr(bx)+math::Sqr(by)) << "\n";

  // Brute force to see if it can be bettered...
   real32 bfx = 0.0;
   real32 bfy = 0.0;
   real32 bfs = -math::Infinity<real32>();
   for (nat32 i=0;i<10000;i++)
   {
    real32 ang = math::pi*2.0*real32(i)/real32(10000);
    real32 x = math::Sqrt(n) * math::Sin(ang);
    real32 y = math::Sqrt(n) * math::Cos(ang);
    
    real32 score = a*x + b*y + c*math::Sqr(x) + d*math::Sqr(y);
    if (score>bfs)
    {
     bfx = x;
     bfy = y;
     bfs = score;
    }
   }
   
   con << "By brute force got maximum = " << bfs << "\n";
   con << "Using x = " << bfx << ", y = " << bfy << " such that x^2+y^2 = " << (math::Sqr(bfx)+math::Sqr(bfy)) << "\n";
 } */



 // Test belief propagation with Fisher-Bingham distributions, in a 1D case...
  // Create...
   /*const nat32 width = 11;
   const real32 adjConc = 10.0;
   const real32 strength = 1.0;

   sfs::SfS_BP sfsbp;
   sfsbp.SetSize(width,1);
   for (nat32 x=0;x<width;x++)
   {
    sfsbp.SetHoriz(x,0,adjConc);
    
    //math::Fisher fish;
    //real32 ang = 0.5*math::pi*real32(x)/real32(width-1);
    //fish[0] = math::Cos(ang)*strength;
    //fish[1] = math::Sin(ang)*strength;
    //sfsbp.MultDist(x,0,fish);
   }
   
   math::Fisher left; left[0] = strength;
   sfsbp.MultDist(0,0,left);

   math::Fisher right; right[1] = strength;
   sfsbp.MultDist(width-1,0,right);


  // Run...
   sfsbp.SetIters(width*4,1);
   sfsbp.Run(&con.BeginProg());
   con.EndProg();


  // Extract...
   bs::Normal out[width];
   for (nat32 x=0;x<width;x++)
   {
    sfsbp.GetDir(x,0,out[x]);
    con << x << " -> " << out[x] << "\n";
   }
   
   for (nat32 x=1;x<width;x++)
   {
    con << (x-1) << " to " << x << " is " << (math::InvCos(out[x-1]*out[x])*180.0/math::pi) << " degrees.\n";
   }*/
   


 // Test the nearest point on ellipsoid code...
 /* data::Random rand;
  nat32 bad = 0;
  for (nat32 j=0;j<1000;j++)
  {
   // Generate random parameters...
    math::Vect<3> ell;
    math::Vect<3> point;
    //for (nat32 i=0;i<3;i++)
    //{
    // ell[i] = rand.Normal() * 10.0;
    // point[i] = rand.Signed() * 20.0;
    //}
    ell[0] = 1.0;
    ell[1] = 1.0;
    ell[2] = 1.0+rand.Normal()*10.0;
    point[0] = 0.0;
    point[1] = 0.0;
    point[2] = rand.Signed()*20.0;
   
    con << "ell = " << ell << "\n";
    con << "point = " << point << "\n";

   // Use the solver to find the optimum...
    math::Vect<3> out;
    alg::PointEllipsoid(ell,point,out);
    real32 dist = point.DistanceTo(out);
    con << "out = " << out << "  (distance = " << dist << ")\n";
    real32 err = math::Sqr(out[0]/ell[0]) + math::Sqr(out[1]/ell[1]) + math::Sqr(out[2]/ell[2]) - 1.0;
    con << "err = " << err << "\n";
   
   // Randomly select points on the ellipsoid and see if they do better...
    real32 better = dist;
    math::Vect<3> best(0.0);
    for (nat32 i=0;i<1000;i++)
    {
     math::Vect<3> rout;
     rand.UnitVector(rout);
     for (nat32 i=0;i<3;i++) rout[i] *= ell[i];
     
     real32 rdist = point.DistanceTo(rout);
     if (rdist<better)
     {
      better = rdist;
      best = rout;
     }
    }
    if (better<dist)
    {
     con << "Found better - " << best << ", distance = " << better << "\n";
     bad++;
    }
   }
   con << "bad = " << bad << "\n";*/



 // Test finding maxima of fisher bingham distribution...
  /*nat32 total_bad = 0;
  for (nat32 i=0;i<1;i++)
  {
   data::Random rand;
   // Generate random distribution...
    math::FisherBingham fb;
    {
     rand.UnitVector(fb.fisher);
     fb.fisher *= rand.Normal()*20.0;
     fb.bingham[0][0] = rand.Signed()*10.0;
     fb.bingham[1][1] = rand.Signed()*10.0;
     fb.bingham[2][2] = rand.Signed()*10.0;
     fb.bingham[0][1] = rand.Signed()*10.0;
     fb.bingham[1][0] = fb.bingham[0][1];
     fb.bingham[0][2] = rand.Signed()*10.0;
     fb.bingham[2][0] = fb.bingham[0][2];
     fb.bingham[1][2] = rand.Signed()*10.0;
     fb.bingham[2][1] = fb.bingham[1][2];
    }
    {
    math::Vect<3> dir;
    rand.UnitVector(dir);
    real32 ang = rand.Normal()*math::pi*0.5;
    real32 k = rand.Normal()*20.0;
    fb = math::FisherBingham(dir,ang,k);
    }

   // Find maximum via method...
    math::Vect<3> dir;
    fb.Maximum(dir);
    real32 best = fb.UnnormProb(dir);


   // Try random directions to see if any do better...
    nat32 bad = 0;
    real32 better = best;
    math::Vect<3> dirB;
    for (nat32 i=0;i<10000;i++)
    {
     math::Vect<3> dir;
     rand.UnitVector(dir);
     real32 val = fb.UnnormProb(dir);
     if (val>best) bad++;
     if (val>better)
     {
      better = val;
      dirB = dir;
     }
    }
    
    if (bad!=0)
    {
     con << "Distribution " << fb << ":\n";
     con << "Efficient best is " << dir << " with a value of " << best << "\n";
     con << "Brute force best is " << dirB << " with a value of " << better << "\n";
     con << ((real32(bad)/10000.0)*100.0) << "% of samples were better.\n\n";
     total_bad++;
    }
  }
  con << "Total bad cases = " << total_bad << "\n";*/



 // Test calculation of Fisher-Bingham normalising constant...
 {
  data::Random rand;
  // Generate random distribution...
   math::FisherBingham fb;
   {
    rand.UnitVector(fb.fisher);
    fb.fisher *= rand.Normal()*20.0;
    fb.bingham[0][0] = rand.Signed()*10.0;
    fb.bingham[1][1] = rand.Signed()*10.0;
    fb.bingham[2][2] = rand.Signed()*10.0;
    fb.bingham[0][1] = rand.Signed()*10.0;
    fb.bingham[1][0] = fb.bingham[0][1];
    fb.bingham[0][2] = rand.Signed()*10.0;
    fb.bingham[2][0] = fb.bingham[0][2];
    fb.bingham[1][2] = rand.Signed()*10.0;
    fb.bingham[2][1] = fb.bingham[1][2];
   }
   con << "Selected distribution = " << fb << "\n";


  // Calculate normalising divisor the complicated efficient way...
   real32 quickWay = 0.0;
   for (nat32 i=0;i<1000;i++) quickWay = fb.NormDiv(); // For profiling.
   con << "Normalising divisor by complex method = " << quickWay << "\n";


  // Calculate the normalising divisor the inefficient easy way...
   real32 mean = 0.0;
   for (nat32 i=0;i<250000;i++)
   {
    math::Vect<3> dir;
    rand.UnitVector(dir);
    real32 val = fb.UnnormProb(dir);
    
    mean += (val-mean)/real32(i+1); // Incrimental mean calcualtion, for numerical robustness.
   }
   
   con << "Normalising divisor by easy method = " << (4.0*math::pi*mean) << "\n";
   
   con << "\n";
 }
 
 
 
 // Test method to find all the maximas of a FB8 distribution...
 /*{
  data::Random rand;
  // Generate random distribution...
   math::FisherBingham fb;
   {
    rand.UnitVector(fb.fisher);
    fb.fisher *= rand.Normal()*20.0;
    fb.bingham[0][0] = rand.Signed()*10.0;
    fb.bingham[1][1] = rand.Signed()*10.0;
    fb.bingham[2][2] = rand.Signed()*10.0;
    fb.bingham[0][1] = rand.Signed()*10.0;
    fb.bingham[1][0] = fb.bingham[0][1];
    fb.bingham[0][2] = rand.Signed()*10.0;
    fb.bingham[2][0] = fb.bingham[0][2];
    fb.bingham[1][2] = rand.Signed()*10.0;
    fb.bingham[2][1] = fb.bingham[1][2];
   }
   real32 fbDiv = fb.NormDiv();
   con << "Selected distribution = " << fb << "\n";
   
   
  // Extract all the maximas, and other stuff...
   math::Vect<3> maxes[2];
   real32 prob[2];
   nat32 num = fb.Maximums(maxes);
   con << num << " point(s) found.\n";
   for (nat32 i=0;i<num;i++)
   {
    prob[i] = fb.UnnormProb(maxes[i]);
    con << i << ": dir = " << maxes[i] << "; prob = " << (prob[i]/fbDiv) << ";\n";
   }
   
  // Check if randomly selected directions can beat 'em...
   if (num!=0)
   {
    for (nat32 i=0;i<1000;i++)
    {
     math::Vect<3> dir;
     rand.UnitVector(dir);
     
     nat32 closest = 0;
     for (nat32 i=1;i<num;i++)
     {
      if ((maxes[i]*dir)>(maxes[closest]*dir)) closest = i;
     }
     
     real32 pr = fb.UnnormProb(dir);
     if (pr>prob[closest])
     {
      // Problem...
       con << "Better dir = " << dir << "; prob = " << (pr/fbDiv) << ";\n";
     }
    }
   }
   
 
  con << "\n";
 }*/



 return 0;
}

//------------------------------------------------------------------------------

