//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include <stdio.h>
#include <iostream>

#include "segs/main.h"

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

 if (argc!=2)
 {
  std::cout << "Usage:\nsegs [image]\n";
  std::cout << "Writes out several files, overwritting previous edittions.\n";
  return 1;
 }

 str::TokenTable tt;
 svt::Core core(tt);

 svt::Var * in = filter::LoadImageRGB(core,argv[1]);
 if (in==null<svt::Var*>())
 {
  std::cout << "Unable to load image.\n";
  delete in;
 }


 // Create the extra fields we want in our Var - I can't be doing with creating multiple once...
   bs::ColourL lIni(0.0);
   in->Add("l",lIni); 
   bs::ColourLuv luvIni(0.0,0.0,0.0);
   in->Add("luv",luvIni);
   nat32 segIni = 0;
   in->Add("segs",segIni);
   bs::ColourRGB greekIni(0.0,0.0,0.0);
   in->Add("weights",greekIni);
   bs::ColourRGB colIni(0.0,0.0,0.0);
   in->Add("render1",colIni);
   in->Add("render2",colIni);
  in->Commit(true);

 // Do the segmentation...
  filter::RGBtoL(in);
  filter::RGBtoLuv(in);
  svt::Field<bs::ColourL> l; in->ByName("l",l);
  svt::Field<bs::ColourLuv> luv; in->ByName("luv",luv);

  filter::Synergism syn;
   syn.SetImage(l,luv);

  MyProg prog;
  syn.Run(&prog);
  std::cout << "Obtained " << syn.Segments() << " segments.\n";

  svt::Field<nat32> segs; in->ByName("segs",segs);
  svt::Field<bs::ColourRGB> weights; in->ByName("weights",weights);
  
  svt::Field<real32> weight; weights.SubField(0,weight);
  svt::Field<real32> eta; weights.SubField(sizeof(real32),eta);
  svt::Field<real32> rho; weights.SubField(2*sizeof(real32),rho);

  syn.GetSegments(segs);
  syn.GetSmoothed(luv);
  syn.GetWeights(weight);
  syn.GetEta(eta);
  syn.GetRho(rho);


 // Render the segmentation... 
  svt::Field<bs::ColourRGB> render1; in->ByName("render1",render1);
  svt::Field<bs::ColourRGB> rgb; in->ByName("rgb",rgb);
  svt::Field<bs::ColourRGB> render2; in->ByName("render2",render2);

  filter::RenderSegsColour(segs,render1);
  filter::RenderSegsMean(segs,rgb,render2);
  filter::RenderSegsLines(segs,render2);


 // Save the mean shift smoothing...
  filter::LuvtoRGB(in);
  if (filter::SaveImageRGB(in,"smooth.bmp",true,"rgb")==false)
  {
   std::cout << "Failed to save smooth.bmp\n";
   delete in;
   return 1;
  } 


 // Save the visualisation...
  if (filter::SaveImageRGB(in,"segs1.bmp",true,"render1")==false)
  {
   std::cout << "Failed to save segs1.bmp\n";
   delete in;
   return 1;
  }
  if (filter::SaveImageRGB(in,"segs2.bmp",true,"render2")==false)
  {
   std::cout << "Failed to save segs2.bmp\n";
   delete in;
   return 1;
  }


 // Save out the weight stuff...
  if (filter::SaveImageRGB(in,"weights.bmp",true,"weights")==false)
  {
   std::cout << "Failed to save weights.bmp\n";
   delete in;
   return 1;
  }


 delete in;
 return 0;
}

//------------------------------------------------------------------------------
