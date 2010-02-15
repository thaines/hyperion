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
