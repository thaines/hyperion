//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

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


#include "mscr/main.h"

//------------------------------------------------------------------------------
int main(int argc,char ** argv)
{
 using namespace eos;
 os::Con conObj;
 os::Conversation & con = *conObj.StartConversation();

 if (argc<2)
 {
  con << "Usage:\nmscr [image]\n";
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


 // Add extra fields to image as required...
  bs::ColourRGB rgbIni(0.0,0.0,0.0);
  image->Add("mscr-e",rgbIni);
  image->Commit();


 // Get field objects setup...
  svt::Field<bs::ColourRGB> rgb(image,"rgb");
  svt::Field<bs::ColourRGB> rgbE(image,"mscr-e");


 // Run the MSCR algorithm to get a set of key locations...
  filter::MSCR mscr;
  mscr.Input(rgb);
  
  mscr.Run(&con.BeginProg());
  con.EndProg();
  con << "Found " << mscr.Size() << " keypoints\n";


 // Visualise the stable regions found as ellipsoids with crosses marking the centres...
  for (nat32 y=0;y<rgbE.Size(1);y++)
  {
   for (nat32 x=0;x<rgbE.Size(0);x++) rgbE.Get(x,y) = rgb.Get(x,y);
  }
  
  for (nat32 i=0;i<mscr.Size();i++)
  {
   // Calculate an affine frame...
    math::Mat<2,2> affine;
    {
     math::Mat<2> temp;
     math::Mat<2> affineInv = mscr[i].covar;
     math::Inverse(affineInv,temp);
     math::Sqrt22(affineInv);
     affine = affineInv;
     math::Inverse(affine,temp);
    }
  
   // Output coordinates...
    con << i << ": Coords = " << mscr[i].mean << "; Area = " << mscr[i].area << "\n";

   // Draw an ellipsoid...
    static const nat32 steps = 16;
    for (nat32 j=0;j<=steps;j++)
    {
     real32 angA = math::pi*2.0*real32(j)/real32(steps);
     real32 angB = math::pi*2.0*real32(j+1)/real32(steps);
     
     bs::Pnt a,b;
     a[0] = math::Cos(angA);
     a[1] = math::Sin(angA);
     b[0] = math::Cos(angB);
     b[1] = math::Sin(angB);
    
     bs::Pnt a2,b2;
     math::MultVect(affine,a,a2);
     math::MultVect(affine,b,b2);
     
     a2 += mscr[i].mean;
     b2 += mscr[i].mean;
         
     rend::Line(rgbE,a2,b2,bs::ColourRGB(0.0,0.0,1.0));
    }
   
   
   // Draw a cross over the centre...
   {
    bs::Pnt a,b,c,d;
    a[0] =  1.0; a[1] =  0.0;
    b[0] =  0.0; b[1] =  1.0;
    c[0] = -1.0; c[1] =  0.0;
    d[0] =  0.0; d[1] = -1.0;
    
    bs::Pnt a2,b2,c2,d2;
    math::MultVect(affine,a,a2);
    math::MultVect(affine,b,b2);
    math::MultVect(affine,c,c2);
    math::MultVect(affine,d,d2);
    
    a2 += mscr[i].mean;
    b2 += mscr[i].mean;
    c2 += mscr[i].mean;
    d2 += mscr[i].mean;
    
    rend::Line(rgbE,a2,c2,bs::ColourRGB(1.0,1.0,0.0));
    rend::Line(rgbE,b2,d2,bs::ColourRGB(1.0,1.0,0.0));
   }
  }
  
  for (nat32 i=0;i<mscr.Size();i++)
  {
   // Put a dot in the centre...
    rend::SetPixel(rgbE,mscr[i].mean,bs::ColourRGB(1.0,0.0,0.0));
  }


 // Save output...
  if (!filter::SaveImage(rgbE,"mscr_ellipsoids.bmp",true))
  {
   con << "Error saving ellipsoid image.\n";
  }


 // Clean up...
  con << "Done\n";
  delete image;

 return 0;
}

//------------------------------------------------------------------------------
