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


#include "mser/main.h"

//------------------------------------------------------------------------------
int main(int argc,char ** argv)
{
 using namespace eos;
 os::Con conObj;
 os::Conversation & con = *conObj.StartConversation();

 if (argc<2)
 {
  con << "Usage:\nmser [image]\n";
  con << "Writes out many files, overwritting any previous versions.\n";
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


 // Update the image with extra field that we want...
  bs::ColourRGB rgbIni(0.0,0.0,0.0);
  bs::ColourL lIni(0.0);
  image->Add("rgb-region",rgbIni);
  image->Add("rgb-frame",rgbIni);
  image->Add("l",lIni);
  image->Commit();


 // Make a greyscale version for the algorithm...
  svt::Field<bs::ColourRGB> rgb(image,"rgb");
  svt::Field<bs::ColourRGB> rgb_region(image,"rgb-region");
  svt::Field<bs::ColourRGB> rgb_frame(image,"rgb-frame");
  svt::Field<bs::ColourL> ilr(image,"l");
  filter::RGBtoL(image);  


 // Run the mser algorithm...
  filter::MserKeys alg;
  alg.Set(ilr);
  alg.Set(rgb);
  alg.Run(&con.BeginProg());
  con.EndProg();
  con << "Found " << alg.Size() << " keypoints\n";


 // Render the MSER regions to an image for output...
  rgb_region.CopyFrom(rgb);
  alg.VisualiseRegions(rgb_region);
  if (!filter::SaveImage(rgb_region,"mser_regions.bmp",true))
  {
   con << "Failed to save mser_regions.bmp\n";
  }

 // Render the MSER regions to an image for output...
  rgb_frame.CopyFrom(rgb);
  alg.VisualiseFrames(rgb_frame);
  if (!filter::SaveImage(rgb_frame,"mser_frames.bmp",true))
  {
   con << "Failed to save mser_frames.bmp\n";
  }

 // Render the feature vectors as lots of little images to a new image...
 {
  svt::Var * keyImg = alg.KeyImage(core);
  svt::Field<bs::ColourRGB> keyIm(keyImg,"rgb");

  if (!filter::SaveImage(keyIm,"mser_keys.bmp",true))
  {
   con << "Failed to save mser_keys.bmp\n";
  }
  
  delete keyImg;	 
 }

 // Generate an xml file with the mser details in...
 {
  bs::Element * root = alg.AsXML(tt);
  root->MakeHuman();
  if (file::SaveXML(root,"mser.xml",true)==false)
  {
   con << "Failed to save mser.xml\n";
  }
  delete root;
 }


 // Clean up...
  con << "Done\n";
  delete image;

 return 0;
}

//------------------------------------------------------------------------------
