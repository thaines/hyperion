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


#include <stdio.h>
#include <iostream>

#include "bp_stereo/main.h"

using namespace eos;

//------------------------------------------------------------------------------
class MyProg : public eos::time::Progress
{
 public:
  MyProg() {}

  void OnChange()
  {
   nat32 x,y;
   Part(Depth()-1,x,y);
   if (x%((y/100)+1)!=0) return;

   for (nat32 i=0;i<Depth();i++)
   {
    nat32 x,y;
    Part(i,x,y);
    std::cout << i << ": [" << x << " of " << y << "]\n";
   }
   std::cout << "\n";

  }
};

//------------------------------------------------------------------------------
int main(int argc,char ** argv)
{
 MyProg prog;

 if (argc<3)
 {
  std::cout << "Usage:\bp_stereo [left] [right] <minDisp> <maxDisp> <scale>\n";
  std::cout << "minDisp defaults to -30, maxDisp to 30, scale to 8.\n";
  return 1;
 }

 str::TokenTable tt;
 svt::Core core(tt);

 svt::Var * left = filter::LoadImageRGB(core,argv[1]);
 if (left==null<svt::Var*>())
 {
  std::cout << "Unable to load left image.\n";
  return 1;
 }

 svt::Var * right = filter::LoadImageRGB(core,argv[2]);
 if (right==null<svt::Var*>())
 {
  std::cout << "Unable to load right image.\n";
  return 1;
 }
 
 int32 minDisp = -30;
 if (argc>3) minDisp = str::ToInt32(argv[3]);
 
 int32 maxDisp = 30;
 if (argc>4) maxDisp = str::ToInt32(argv[4]); 
 
 int32 scale = 8;
 if (argc>5) scale = str::ToInt32(argv[5]);
 

 // Update the left image with extra fields that we want...
  bs::ColourL lIni(0.0);
  int32 dispNatIni = 0;
  real32 dispIni = 0;
   
  left->Add("l",lIni);
  left->Add("dispNat",dispNatIni);
  left->Add("disp",dispIni);   
  left->Commit();
  
 // Update the right image with extra fields...
  right->Add("l",lIni);
  right->Commit();
  
 // Convert both images to greyscale...
  filter::RGBtoL(left);
  filter::RGBtoL(right);
 

 // Run the stereo algorithm...
  svt::Field<real32> leftL(left,"l");
  svt::Field<real32> rightL(right,"l");
  
  filter::KernelVect kernel(2);
  kernel.MakeGaussian(0.7);
  kernel.Apply(leftL,leftL); 
  kernel.Apply(rightL,rightL);
   
  stereo::SimpleBP sa;
  sa.SetPair(leftL,rightL);
  sa.SetDisp(minDisp,maxDisp);
  
  sa.Run(&prog);


 // Render the disparity generated...
  svt::Field<int32> dispNat(left,"dispNat");
  sa.GetDisparity(dispNat);
  
  svt::Field<real32> disp(left,"disp");
  for (nat32 y=0;y<disp.Size(1);y++)
  {
   for (nat32 x=0;x<disp.Size(0);x++) disp.Get(x,y) = dispNat.Get(x,y);	  
  }
  
  svt::Field<bs::ColourL> cl(left,"l");
  stereo::RenderDisp(disp,cl,real32(scale)/255.0);
  filter::LtoRGB(left);
  
  if (filter::SaveImageRGB(left,"disp.bmp",true)==false)
  {
   std::cout << "Failed to save disp.bmp\n";
   delete left;
   delete right;
   return 1;
  }


 // Save a .obf file...
 {
  svt::Var tempVar(disp);
  real32 dispIni = 0.0;
  tempVar.Add("disp",dispIni);
  tempVar.Commit();
  
  svt::Field<real32> tempDisp(&tempVar,"disp");
  tempDisp.CopyFrom(disp);
  
  if (svt::Save("disp.obf",&tempVar,true)==false)
  {
   std::cout << "Failed to save disp.obf\n";
   delete left;
   delete right;
   return 1;
  }
 }


 // Clean up...
  delete left;
  delete right;

 return 0;
}

//------------------------------------------------------------------------------
