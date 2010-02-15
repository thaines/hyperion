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

#include "sift/main.h"

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

 if (argc<2)
 {
  std::cout << "Usage:\nsift [image]\n";
  std::cout << "Writes out several files, overwritting any previous versions.\n";
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


 // Update the image with extra field that we want...
   bs::ColourL lIni(0.0);
   image->Add("l",lIni);
  image->Commit();
  
 // Make a greyscale version for the algorithm...
  svt::Field<bs::ColourRGB> rgb(image,"rgb");
  svt::Field<real32> ilr(image,"l");
  filter::RGBtoL(image);  

 // Run the sift algorithm...
  prog.Report(0,3);
  filter::SiftKeypoint siftK;
   siftK.Run(ilr,&prog);
   
  prog.Report(1,3);
  filter::SiftFeature siftF;
   siftF.Run(siftK,&prog);

   
 // Render the keypoints found to an image...
  prog.Report(2,3);
  std::cout << "Done, " << siftK.Keypoints() << " keypoints.\n";
  siftK.Render(rgb);

  
 // Output the gaussian pyramid for verification... 
 /*{
  const filter::Pyramid & p = siftK.GetPyramid();
  for (nat32 i=0;i<p.Octaves();i++)
  {
   svt::Var temp(core);
    temp.Setup2D(p.OctaveWidth(i),p.OctaveHeight(i));
    bs::ColourRGB nullRGB(0.0,0.0,0.0);
    temp.Add("rgb",nullRGB);
    temp.Commit();
    
    svt::Field<real32> grey;
    svt::Field<bs::ColourL> l;
    svt::Field<bs::ColourRGB> rgb(&temp,"rgb");
      
   for (nat32 j=0;j<p.Scales();j++)
   {
    p.Get(i,j,grey);
    grey.SubField(0,l);
    filter::LtoRGB(l,rgb);
    
    cstrchar str[128];
     str::Copy(str,"gauss-");
     str::ToStr(i,str+str::Length(str));
     str::Append(str,"-");
     str::ToStr(j,str+str::Length(str));
     str::Append(str,".bmp");
    
    filter::SaveImage(rgb,str,true);
   }
  }
 }*/
  
 // Output the keypoints that have been found...
  if (filter::SaveImageRGB(image,"keypoints.bmp",true)==false)
  {
   std::cout << "Failed to save keypoints.bmp\n";
   delete image;
   return 1;
  }
  
 // Save the sift features to a file, for examination...
  FILE * f = fopen("keypoints.txt","w");
  for (nat32 i=0;i<siftK.Keypoints();i++)
  {
   fprintf(f,"(%f,%f):[%i,%f]:%f: {",0.5*(siftK[i].x+siftK[i].xOff),0.5*(siftK[i].y+siftK[i].yOff),
                                     siftK[i].octave,siftK[i].scale+siftK[i].sOff,
                                     math::ToDeg(siftK[i].rot));
   for (nat32 j=0;j<siftF.fvSize-1;j++) fprintf(f,"%f,",siftF[i][j]);
   fprintf(f,"%f}\n",siftF[i][siftF.fvSize-1]);
  }
  fclose(f);
  
 // Clean up...
  delete image;

 return 0;
}

//------------------------------------------------------------------------------
