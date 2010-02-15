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

#include "sfs/main.h"

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
 
 if (argc<5)
 {
  std::cout << "Usage:\nsfs [image] [lx] [ly] [lz] <arrows> <albedo>\n";
  std::cout << " [image] = Image to apply sfs on.\n";
  std::cout << " [lx] = X component of light source normal.\n";
  std::cout << " [ly] = Y component of light source normal.\n";
  std::cout << " [lz] = Z component of light source normal.\n";
  std::cout << " <arrows> = Provide no parameter or 0 to do nothing or 1 to output an arrow map. Ushally you don't want to do this as arrow maps are very large and only useful for printing.\n";  
  std::cout << " <albedo> = Albedo map to use, if not provided it uses the highest value in the image throughout the image as the albedo.\n";
  std::cout << "The light normal is normalised by the program, and must point towards the light at infinity, outputs several images overwritting any previous versions.\n";
  return 1;
 }

 str::TokenTable tt;
 svt::Core core(tt);
 
 svt::Var * image = filter::LoadImageRGB(core,argv[1]);
 if (image==null<svt::Var*>())
 {
  std::cout << "Unable to load image.\n";
  return 1;
 }
 
 bs::Normal toLight;
  toLight[0] = str::ToReal32(argv[2]);
  toLight[1] = str::ToReal32(argv[3]);
  toLight[2] = str::ToReal32(argv[4]);    
 toLight.Normalise();
 
 svt::Var * albedo = null<svt::Var*>();
 if (argc>=7)
 {
  albedo = filter::LoadImageRGB(core,argv[6]);
  if (albedo==null<svt::Var*>())
  {
   std::cout << "Unable to load albedo image.\n";
   delete image;
   return 1;
  } 
 }


 // Obtain a greyscale version of the image...
  filter::RGBtoL(image);
  svt::Field<real32> img(image,"l");


 // If an albedo map has not been provided create the most basic one possible,
 // either way we want greyscale...
  svt::Field<real32> alb;
  if (albedo==null<svt::Var*>())
  {
   bs::ColourL colIni(0.0);
   for (nat32 y=0;y<img.Size(1);y++)
   {
    for (nat32 x=0;x<img.Size(0);x++)
    {
     colIni.l = math::Max(colIni.l,img.Get(x,y));
    }
   }
   
   std::cout << "Maximum brightness = " << colIni.l << "\n";
  
   albedo = new svt::Var(img);
    albedo->Add("l",colIni);
   albedo->Commit();
  }
  else filter::RGBtoL(albedo);  
  albedo->ByName("l",alb);


 // Apply the sfs...
  mya::Sfs sfs;
   sfs.SetImage(img);
   sfs.SetAlbedo(alb);   
   sfs.SetLight(toLight);
   
  sfs.Run(&prog); 


 // Extract the results...
  svt::Var * results = new svt::Var(img);
   bit bitIni = false;
   results->Add("mask",bitIni);
   bs::Normal normalIni(0.0,0.0,0.0);
   results->Add("needle",normalIni);
   bs::ColourL lIni(0.0);
   results->Add("l",lIni);   
   bs::ColourRGB rgbIni(0.0,0.0,0.0);
   results->Add("rgb",rgbIni);   
   real32 shapeIni = 0.0;
   results->Add("shape",shapeIni);      
  results->Commit();
  
  svt::Field<bit> mask(results,"mask");
  svt::Field<bs::Normal> needle(results,"needle");
  svt::Field<real32> render(results,"l");
  svt::Field<bs::ColourRGB> renderRGB(results,"rgb");
  svt::Field<real32> shapeIndex(results,"shape");  
  
  sfs.GetNeedle(needle);
  sfs.GetMask(mask);


 // Output several images created by adjusting the lightsource position...
  rend::LambertianNeedleRender(alb,needle,toLight,render);
  filter::LtoRGB(results);
  filter::SaveImage(renderRGB,"original.bmp",true);
  
  bs::Normal ld;

  ld = bs::Normal(-2.0,0.0,1.0);
  ld.Normalise();
  rend::LambertianNeedleRender(alb,needle,ld,render);
  filter::LtoRGB(results);
  filter::SaveImage(renderRGB,"rend0.bmp",true);  
  
  ld = bs::Normal(-1.0,0.0,1.0);
  ld.Normalise();
  rend::LambertianNeedleRender(alb,needle,ld,render);
  filter::LtoRGB(results);
  filter::SaveImage(renderRGB,"rend1.bmp",true);  
  
  ld = bs::Normal(0.0,0.0,1.0);
  ld.Normalise();
  rend::LambertianNeedleRender(alb,needle,ld,render);
  filter::LtoRGB(results);
  filter::SaveImage(renderRGB,"rend2.bmp",true);  
  
  ld = bs::Normal(1.0,0.0,1.0);
  ld.Normalise();
  rend::LambertianNeedleRender(alb,needle,ld,render);
  filter::LtoRGB(results);
  filter::SaveImage(renderRGB,"rend3.bmp",true);  
  
  ld = bs::Normal(2.0,0.0,1.0);
  ld.Normalise();
  rend::LambertianNeedleRender(alb,needle,ld,render);
  filter::LtoRGB(results);
  filter::SaveImage(renderRGB,"rend4.bmp",true);  


 // Output a colour map generted from the normals, bit psychodelic...
  sfs.GetNeedleColour(renderRGB);
  filter::SaveImage(renderRGB,"trippy.bmp",true);

 // Output a shape index map generated from the normals...
  filter::ShapeIndex(needle,mask,shapeIndex);
  filter::ColourShapeIndex(shapeIndex,renderRGB);
  filter::SaveImage(renderRGB,"shape_index.bmp",true);

 // Output a 3D model generated from the sfs data...
  if (mya::SaveNeedleModel(needle,mask,2,"model.obj",true)!=true)
  {
   std::cout << "Failed to save model.obj\n";
  }
  
 // If requested output the normals as a bit grid of arrows, 
 // for printouts as funky psychodelic diagrams are no use 
 // with typical quality printing...
  if ((argc>=6)&&(str::ToInt32(argv[5])!=0))
  {
   svt::Var * arrow = sfs.GetArrow();
   if (!filter::SaveImageRGB(arrow,"arrow.bmp",true))
   {
    std::cout << "Failed to save arrow.bmp\n";
   }
   delete arrow;
  }

  
 // Some test code - generate a well structured needle map then integrate to see if it comes out right...
 /* static const real32 rad = 120.0;
  for (nat32 y=0;y<needle.Size(1);y++)
  {
   for (nat32 x=0;x<needle.Size(0);x++)
   {
    real32 xPos = x - needle.Size(0)*0.5;
    real32 yPos = y - needle.Size(1)*0.5;
    real32 zPos = math::Sqr(rad) - math::Sqr(xPos) - math::Sqr(yPos);
    if (zPos>0.0)
    {
     zPos = math::Sqrt(zPos);
     needle.Get(x,y).X() = xPos;
     needle.Get(x,y).Y() = yPos;
     needle.Get(x,y).Z() = zPos;
     needle.Get(x,y).Normalise();
     mask.Get(x,y) = true;
    }
    else
    {
     needle.Get(x,y).X() = 0.0;
     needle.Get(x,y).Y() = 0.0;
     needle.Get(x,y).Z() = 0.0;    
     mask.Get(x,y) = false;
    }
   }
  }
  
  mya::SaveNeedleModel(needle,mask,2,"test.obj",true);*/

         
 // Clean up...
  delete image;
  delete albedo;

 return 0;
}

//------------------------------------------------------------------------------
