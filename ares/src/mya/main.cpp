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

#include "mya/main.h"

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
 if (argc<6)
 {
  std::cout << "Usage:\nmya [left] [right] [lx] [ly] lz] <min_disp> <max_disp> <scale> <dMult>\n";
  std::cout << " [left] = left image of stereo pair, for which details disparity is calculated.\n";
  std::cout << " [right] = right image of stereo pair.\n";
  std::cout << " [lx] = X component of un-normalised vector pointing at light source.\n";
  std::cout << " [ly] = Y component of un-normalised vector pointing at light source.\n";
  std::cout << " [lz] = Z component of un-normalised vector pointing at light source.\n";    
  std::cout << " <min_disp> = minimum dispairity to consider during stereo post-step. Defaults to -30.\n";
  std::cout << " <max_disp> = maximum dispairity to consider during stereo post-step. Defaults to 30.\n";  
  std::cout << " <scale> = Scale to multiply dispairty by when writting out disparity images, defaults to 16.\n";
  std::cout << " <dMult> = dMult/disparity is taken to be depth. Defaults to 1.0.\n";
  std::cout << "Writes out several files, overwritting previous edittions.\n";
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
 
 bs::Normal toLight;
  toLight[0] = str::ToReal32(argv[3]);
  toLight[1] = str::ToReal32(argv[4]);
  toLight[2] = str::ToReal32(argv[5]);
 toLight.Normalise();   
 
 int32 minDisp = -30;
 if (argc>6) minDisp = str::ToInt32(argv[6]);
 
 int32 maxDisp = 30;
 if (argc>7) maxDisp = str::ToInt32(argv[7]); 
 
 int32 scale = 16;
 if (argc>8) scale = str::ToInt32(argv[8]); 
 
 real32 depthMult = 1.0;
 if (argc>9) depthMult = str::ToReal32(argv[9]);

 MyProg prog;
  prog.Report(0,5); 


 // Update the left image with extra fields that we want...
   bs::ColourL lIni(0.0);
   left->Add("l",lIni);
   bs::ColourLuv luvIni(0.0,0.0,0.0);
   left->Add("luv",luvIni);
   nat32 segsIni = 0;
   left->Add("segs",segsIni);
   left->Add("layers",segsIni);
   left->Add("surfaces",segsIni);      
   real32 dispIni = 0;
   left->Add("disp",dispIni);
   left->Add("disp-ss",dispIni);
   bit validIni = false;
   left->Add("valid-ss",validIni);
   bs::ColourRGB rgbIni(0.0,0.0,0.0);
   left->Add("dispRend",rgbIni);   
   left->Add("dispSSRend",rgbIni);
   left->Add("segs-rgb",rgbIni);
   left->Add("layers-rgb",rgbIni);
   left->Add("surfaces-rgb",rgbIni);
   left->Add("warp",rgbIni);
   left->Add("albedo",lIni);
   bs::Normal normalIni(0.0,0.0,1.0);
   left->Add("needle",normalIni);
   left->Add("needle_mask",validIni);   
  left->Commit();



 // Segment the left image...
  filter::RGBtoL(left);
  filter::RGBtoLuv(left);
  svt::Field<bs::ColourL> l(left,"l");  
  svt::Field<bs::ColourLuv> luv(left,"luv");

  filter::Synergism syn;
  syn.SetImage(l,luv);

  syn.Run(&prog);

  std::cout << "Obtained " << syn.Segments() << " segments.\n";
  svt::Field<nat32> segs; left->ByName("segs",segs);
  syn.GetSegments(segs);
  prog.Next();   



 // Run the sad stereo algorithm...
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
   sss.SetMinCluster(50);

   sss.Run(&prog);

  svt::Field<real32> disp_ss(left,"disp-ss");
  svt::Field<bit> valid_ss(left,"valid-ss");
  sss.GetDisparity(disp_ss);
  sss.GetMask(valid_ss);
  prog.Next(); 



 // Determine an albedo map for the left image - for the moment
 // just the maximum value found in the entire image...
  svt::Field<real32> albedo(left,"albedo");
  svt::Field<real32> lr(left,"l");

  real32 maxAlbedo = 0.0;
  for (nat32 y=0;y<l.Size(1);y++)
  {
   for (nat32 x=0;x<l.Size(0);x++) maxAlbedo = math::Max(maxAlbedo,lr.Get(x,y));
  }
  
  for (nat32 y=0;y<albedo.Size(1);y++)
  {
   for (nat32 x=0;x<albedo.Size(0);x++) albedo.Get(x,y) = maxAlbedo;
  }
  std::cout << "Maximum brightness = " << maxAlbedo << "\n";



 // Run shape from shading algorithm on the left image alone...
  svt::Field<bs::Normal> needle(left,"needle");
  svt::Field<bit> needle_mask(left,"needle_mask");

  mya::Sfs sfs;
   sfs.SetImage(lr);
   sfs.SetAlbedo(albedo);   
   sfs.SetLight(toLight);   
  sfs.Run(&prog);
  sfs.GetNeedle(needle);
  sfs.GetMask(needle_mask);
  prog.Next();



 // Build a parametric model fitting setup with relevent modules, and run it...
  data::Random random;
  
  mya::Disparity disparity(depthMult,disp_ss,valid_ss);
  mya::Needles ned(needle,needle_mask);
  
  mya::PlaneSurfaceType planeType;
  mya::SphereSurfaceType sphereType;  
  
  mya::Layers layers;
   layers.SetSegs(syn.Segments(),segs);
   //layers.SetFitMethod(mya::Layers::Ransac,&random);

   layers.SetOutlierDist(layers.AddIed(&disparity),1.0);
   layers.SetOutlierDist(layers.AddIed(&ned),1.0);

   layers.SetSurfaceWeight(layers.AddSurfaceType(&planeType),planeType.Degrees());
   layers.SetSurfaceWeight(layers.AddSurfaceType(&sphereType),sphereType.Degrees());
   
   layers.Commit(&prog);
   prog.Next();
   
  //mya::OutlierScore outlierScore(layers);
  //mya::WarpScore warpScore(layers,syn.Segments(),segs,leftRGB,rightRGB);
  
  //mya::LayerMerge layerMerge(segs,layers,warpScore);
  // layerMerge.SetBias(-5000.0);
  // layerMerge(&prog);

 std::cout << "Segments = " << layers.SegmentCount() << "; Layers = " << layers.LayerCount() << ";" << std::endl;



 // Cleanup and render the disparity generated...
  svt::Field<real32> disp(left,"disp");
  layers.GetDispMap(1.0,disp);
  
  svt::Field<bs::ColourL> cl(left,"l");
  stereo::RenderDisp(disp,cl,real32(scale)/256.0);

  svt::Field<bs::ColourRGB> dispRend(left,"dispRend");
  filter::LtoRGB(cl,dispRend);
  
  
  stereo::RenderDisp(disp_ss,cl,real32(scale)/256.0);
  svt::Field<bs::ColourRGB> dispSSRend(left,"dispSSRend");
  filter::LtoRGB(cl,dispSSRend);
  


 // Extract support/debugging information, and render...
  svt::Field<nat32> laySegs(left,"layers");
  svt::Field<nat32> surSegs(left,"surfaces");
   layers.GetLayerMap(laySegs);
   layers.GetSurfaceMap(surSegs);
  
  svt::Field<bs::ColourRGB> segsRend(left,"segs-rgb");
  svt::Field<bs::ColourRGB> layersRend(left,"layers-rgb");
  svt::Field<bs::ColourRGB> surfacesRend(left,"surfaces-rgb");
   filter::RenderSegsColour(segs,segsRend);
   filter::RenderSegsColour(laySegs,layersRend);
   filter::RenderSegsColour(surSegs,surfacesRend);

  svt::Field<bs::ColourRGB> warp(left,"warp");   
   stereo::ForwardWarp(leftRGB,disp,warp);



 // Output results for checking...
  if (filter::SaveImage(dispRend,"disp.bmp",true)==false)
  {
   std::cout << "Failed to save disp.bmp\n";
   delete left;
   delete right;
   return 1;
  }
  
  if (filter::SaveImage(dispSSRend,"disp_sad_seg.bmp",true)==false)
  {
   std::cout << "Failed to save disp_sad_seg.bmp\n";
   delete left;
   delete right;
   return 1;
  }
  


 // Output support/debugging information...
  if (filter::SaveImage(segsRend,"1-segs.bmp",true)==false)
  {
   std::cout << "Failed to save 1-segs.bmp\n";
   delete left;
   delete right;
   return 1;
  }  

  if (filter::SaveImage(layersRend,"2-layers.bmp",true)==false)
  {
   std::cout << "Failed to save 2-layers.bmp\n";
   delete left;
   delete right;
   return 1;
  } 
  
  if (filter::SaveImage(surfacesRend,"3-surfaces.bmp",true)==false)
  {
   std::cout << "Failed to save 3-surfaces.bmp\n";
   delete left;
   delete right;
   return 1;
  } 

  if (filter::SaveImage(warp,"warp.bmp",true)==false)
  {
   std::cout << "Failed to save warp.bmp\n";
   delete left;
   delete right;
   return 1;
  }
  
 // Generate a number of sfs based renders as though a light sweeps across the view...
  for (int32 i=0;i<5;i++)
  {
   bs::Normal ld(i-2,0.0,1.0);
   ld.Normalise();
   rend::LambertianNeedleRender(albedo,needle,ld,lr);
   filter::LtoRGB(left);
   
   cstrchar buf[64];
   str::Copy(buf,"rend");
   str::ToStr(i,str::End(buf));
   str::Append(buf,".bmp");
   filter::SaveImageRGB(left,buf,true);  
  }

  
 // Clean up...
  delete left;
  delete right;

 return 0;
}

//------------------------------------------------------------------------------
