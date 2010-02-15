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

#include "bleyer04/main.h"

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
  std::cout << "Usage:\bleyer04 [left] [right] <min_disp> <max_disp> <scale>\n";
  std::cout << "Writes out several files, overwritting previous edittions.\n";
  std::cout << "minDisp defaults to -30, maxDisp to +30 and scale to 16.\n";
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
 
 int32 scale = 16;
 if (argc>5) scale = str::ToInt32(argv[5]); 
 

 // Update the left image with extra fields that we want...
   bs::ColourL lIni(0.0);
   left->Add("l",lIni);
   real32 dispIni = 0;
   left->Add("disp",dispIni);
   nat32 segsIni = 0;
   left->Add("segs",segsIni);
   left->Add("layers",segsIni);   
  left->Commit();


 // Run the stereo algorithm...
  svt::Field<bs::ColourRGB> leftRGB; left->ByName("rgb",leftRGB);
  svt::Field<bs::ColourRGB> rightRGB; right->ByName("rgb",rightRGB);
   
  stereo::Bleyer04 st;
   st.SetImages(leftRGB,rightRGB);
   st.SetRange(minDisp,maxDisp);
   st.SetPlaneRadius(0.8);
   st.SetWarpCost(20.0/255.0,20.0/255.0);
   
   st.Run(&prog);
   

 // Cleanup and render the disparity generated...
  svt::Field<real32> disp; left->ByName("disp",disp);
  st.GetDisparity(disp);
  
  svt::Field<bs::ColourL> cl; left->ByName("l",cl);
  stereo::RenderDisp(disp,cl,real32(scale)/255.0);

  stereo::ForwardWarp(leftRGB,disp,rightRGB);
  filter::LtoRGB(left);
  
  std::cout << "Done, " << st.GetSegCount() << " segments, " << st.GetLayerCount() << " layers\n";

 // Output results for checking...
  if (filter::SaveImageRGB(left,"disp.bmp",true)==false)
  {
   std::cout << "Failed to save disp.bmp\n";
   delete left;
   delete right;
   return 1;
  }
  
  if (filter::SaveImageRGB(right,"right_warp.bmp",true)==false)
  {
   std::cout << "Failed to save right_warp.bmp\n";
   delete left;
   delete right;
   return 1;
  }
  
 // Plus some diagnostic results...
  svt::Field<nat32> segs; left->ByName("segs",segs);
  svt::Field<nat32> layers; left->ByName("layers",layers);  
  st.GetSegs(segs);
  st.GetLayers(layers);
  
  filter::RenderSegsColour(segs,leftRGB);
  filter::RenderSegsColour(layers,rightRGB);  
  
  if (filter::SaveImageRGB(left,"segs.bmp",true)==false)
  {
   std::cout << "Failed to save segs.bmp\n";
   delete left;
   delete right;
   return 1;
  }
  
  if (filter::SaveImageRGB(right,"layers.bmp",true)==false)
  {
   std::cout << "Failed to save layers.bmp\n";
   delete left;
   delete right;
   return 1;
  }  
  
 // Clean up...
  delete left;
  delete right;

 return 0;
}

//------------------------------------------------------------------------------
