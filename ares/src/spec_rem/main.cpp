//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "spec_rem/main.h"

//------------------------------------------------------------------------------
int main(int argc,char ** argv)
{
 using namespace eos;
 os::Con conObj;
 os::Conversation & con = *conObj.StartConversation();

 if (argc!=3)
 {
  con << "Usage:\nspec_rem [in] [out]\n";
  con << "Attempts to remove specular highlights from an image.\n";
  return 1;
 }

 str::TokenTable tt;
 svt::Core core(tt);


 // Load image...
  svt::Var * image = filter::LoadImageRGB(core,argv[1]);
  if (image==null<svt::Var*>())
  {
   con << "Unable to load image.\n";
   return 1;
  }


 // Determine light source colour...
  bs::ColourRGB light(1.0,1.0,1.0);
  // ***************************************


 // Run the specularity removal algorithm...
  svt::Field<bs::ColourRGB> rgb(image,"rgb");

  filter::SpecRemoval sr;
  sr.Set(rgb,rgb);
  sr.Set(light);

  sr.Run(&con.BeginProg());
  con.EndProg();


 // Do in-filling where the specularity has left no detail...
  // *********************************************


 // Save image...
  if (filter::SaveImageRGB(image,argv[2],true)==false)
   {
    con << "Unable to save image.\n";
   }


 // Clean up...
  con << "Done\n";
  delete image;

 return 0;
}

//------------------------------------------------------------------------------
