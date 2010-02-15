//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include <stdio.h>
#include <iostream>

#include "sad_stereo/main.h"

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
  std::cout << "Usage:\nsad_stereo [left] [right] <min_disp> <max_disp> <scale> <cutoff>\n";
  std::cout << "Writes out several files, overwritting previous edittions.\n";
  std::cout << "minDisp defaults to -30, maxDisp to +30 and scale to 16.\n";
  std::cout << "cutoff is the minimum cluster size for the cluster to survive pruning, defaults to 32.\n";
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

 int32 cutoff = 32;
 if (argc>6) cutoff = str::ToInt32(argv[6]); 


 // Update the left image with extra fields that we want...
   bs::ColourL lIni(0.0);
   left->Add("l",lIni);
   bs::ColourLuv luvIni(0.0,0.0,0.0);
   left->Add("luv",luvIni);
   nat32 segsIni = 0;
   left->Add("segs",segsIni);
   real32 dispIni = 0;
   left->Add("disp",dispIni);
   bit validIni = 0;
   left->Add("valid",validIni);
  left->Commit();


 // Segment the left image...
  filter::RGBtoL(left);
  filter::RGBtoLuv(left);
  svt::Field<bs::ColourL> l; left->ByName("l",l);  
  svt::Field<bs::ColourLuv> luv; left->ByName("luv",luv);

  filter::Synergism syn;
  syn.SetImage(l,luv);

  syn.Run(&prog);

  std::cout << "Obtained " << syn.Segments() << " segments.\n";
  svt::Field<nat32> segs; left->ByName("segs",segs);
  syn.GetSegments(segs);
  

 // Run the stereo algorithm...
  stereo::SadSegStereo sss;
   sss.SetSegments(syn.Segments(),segs);
   
   svt::Field<bs::ColourRGB> leftRGB; left->ByName("rgb",leftRGB);
   svt::Field<bs::ColourRGB> rightRGB; right->ByName("rgb",rightRGB);

   svt::Field<real32> leftR; leftRGB.SubField(0,leftR);
   svt::Field<real32> leftG; leftRGB.SubField(sizeof(real32),leftG);
   svt::Field<real32> leftB; leftRGB.SubField(2*sizeof(real32),leftB);

   svt::Field<real32> rightR; rightRGB.SubField(0,rightR);
   svt::Field<real32> rightG; rightRGB.SubField(sizeof(real32),rightG);
   svt::Field<real32> rightB; rightRGB.SubField(2*sizeof(real32),rightB);

   sss.AddField(leftR,rightR);
   sss.AddField(leftG,rightG);
   sss.AddField(leftB,rightB);

   sss.SetRange(minDisp,maxDisp);
   sss.SetMinCluster(cutoff);
   sss.SetMaxRadius(3);


   sss.Run(&prog);


 // Cleanup and render the disparity generated...
  svt::Field<real32> disp; left->ByName("disp",disp);
  svt::Field<bit> valid; left->ByName("valid",valid);
  sss.GetDisparity(disp);
  sss.GetMask(valid);

  stereo::FillDispInf(disp,valid);
  
  svt::Field<bs::ColourL> cl; left->ByName("l",cl);
  stereo::RenderDisp(disp,cl,real32(scale)/256.0);

  filter::LtoRGB(left);

 // Output results for checking...
  if (filter::SaveImageRGB(left,"disp.bmp",true)==false)
  {
   std::cout << "Failed to save disp.bmp\n";
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
