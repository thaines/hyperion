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


#include "sfs_bp/main.h"

//------------------------------------------------------------------------------
int main(int argc,char ** argv)
{
 using namespace eos;

 // Create basic objects...
  os::Con conObj;
  os::Conversation & con = *conObj.StartConversation();

  str::TokenTable tt;
  svt::Core core(tt);



 // Check we have enough parameters.
  if (argc!=4)
  {
   con << "Usage: sfs_bp [image.bmp] [camera.crf] [config.xml]\n";
   con << "[image.bmp] is the image to apply shape from shading to.\n";
   con << "[camera.crf] is the camera response function of the image.\n";
   con << "[config.xml] is an xml file that contains all the parameters.\n";
   return 1;
  }



 // Load the image...
  svt::Var * image = filter::LoadImageRGB(core,argv[1]);
  if (image==null<svt::Var*>())
  {
   con << "Unable to load image.\n";
   return 1;
  }

 // Create stack-based root node for automatic deletion...
  svt::Node root(core);
  image->AttachParent(&root);



 // Load the camera response function...
  math::Func crf;
  if (crf.Load(argv[2])==false)
  {
   con << "Unable to load camera response function.\n";
   return 1;
  }



 // Load the configuration file...
  mem::StackPtr<bs::Element> paras = file::LoadXML(tt,argv[3]);
  if (paras.IsNull())
  {
   con << "Unable to load algortihm configuration.\n";
   return 1;
  }
  
 // Extract the necessary parameters...
  real32 coneK = paras->GrabReal(":cone.k",16.0);
  real32 coneFade = paras->GrabReal(":cone.fade",0.0);
  real32 similarityK = paras->GrabReal(":similarity.k",4.5);
  real32 borderK = paras->GrabReal(":border.k",0.0);
  real32 centreK = paras->GrabReal(":centre.k",0.0);
  
  real32 gradEV = paras->GrabReal(":grad.ev",4.0);
  real32 gradBias = paras->GrabReal(":grad.dBias",4.0);
  
  nat32 gradCalcSteps = paras->GrabInt(":gradCalc.steps",8);
  real32 gradCalcExp = paras->GrabReal(":gradCalc.exp",4.0);
  real32 gradCalcStop = paras->GrabReal(":gradCalc.stop",0.0);
  //real32 gradSpatialSd = paras->GrabReal(":gradCalc.spatial",4.0);
  //real32 gradDomainSd = paras->GrabReal(":gradCalc.domain",4.0);

  bs::Normal light;
  light[0] = paras->GrabReal(":light.x",0.0);
  light[1] = paras->GrabReal(":light.y",0.0);
  light[2] = paras->GrabReal(":light.z",1.0);
  light.Normalise();
  
  real32 albedo = paras->GrabReal(":albedo.value",1.0);
  real32 zMult = paras->GrabReal(":mesh.zMult",1.0);
  
  nat32 iters = paras->GrabInt(":paras.iters",8);
  nat32 levels = paras->GrabInt(":paras.levels",32);



 // Augment the image with lots of fields...
  bs::ColourL lIni(0.0);
  bs::ColourLuv luvIni(0.0,0.0,0.0);
  bs::Normal normalIni(0.0,0.0,1.0);
  real32 probIni = 0.0;

  image->Add("l",lIni);
  image->Add("lb",lIni);
  image->Add("luv",luvIni);
  image->Add("gradX",probIni);
  image->Add("gradY",probIni);
  image->Add("needle",normalIni);
  image->Add("prob",probIni);
  image->Commit();


 // Obtain field objects to represent all of the above...
  svt::Field<bs::ColourRGB> imgRGB(image,"rgb");
  svt::Field<bs::ColourL> imgL(image,"l");
  svt::Field<bs::ColourL> imgLB(image,"lb");
  svt::Field<bs::ColourLuv> imgLuv(image,"luv");
  svt::Field<bs::Normal> imgNeedle(image,"needle");
  svt::Field<real32> imgProb(image,"prob");
  svt::Field<real32> imgGradX(image,"gradX");
  svt::Field<real32> imgGradY(image,"gradY");



 // Smooth the original image, using an algorithm designed to deal with
 // quantization artifacts...
 /*{
  filter::RGBtoLuv(image);

  svt::Field<real32> r,g,b;
  imgRGB.SubField(sizeof(real32)*0,r);
  imgRGB.SubField(sizeof(real32)*1,g);
  imgRGB.SubField(sizeof(real32)*2,b);
  
  time::Progress & prog = con.BeginProg();  
  prog.Report(0,3); filter::QuantSmooth(r,r,imgLuv);
  prog.Report(1,3); filter::QuantSmooth(g,g,imgLuv);
  prog.Report(2,3); filter::QuantSmooth(b,b,imgLuv);
  con.EndProg();
 }*/
 /*{
  filter::SimpleQuantSmooth(imgRGB,0.1,&con.BeginProg());
  con.EndProg();
 }*/
 
 
 // Convert the loaded rgb image into l and luv format...
  filter::RGBtoL(image);
  filter::RGBtoLuv(image);
  
 // Blur lb...
 {
  filter::KernelVect kernel(2);
  kernel.MakeGaussian(math::Sqrt(2.0));
  
  svt::Field<real32> in(image,"l");
  svt::Field<real32> out(image,"lb");  
  
  kernel.Apply(in,out);
 }


 // Prepare the solver...
  sfs::SfS_BP sfsbp;
  sfsbp.SetSize(imgL.Size(0),imgL.Size(1));
  
  for (nat32 y=0;y<imgL.Size(1);y++)
  {
   for (nat32 x=0;x<imgL.Size(0);x++)
   {
    // Horizontal smoothing term...
    if ((x+1)<imgL.Size(0))
    {
     sfsbp.SetHoriz(x,y,similarityK);
    }
     
    // Vertical smoothing term...
    if ((y+1)<imgL.Size(1))
    {
     sfsbp.SetVert(x,y,similarityK);
    }
     
    // Cone term...
     real32 corL = math::Min(imgL.Get(x,y).l/albedo,real32(1.0));
     sfsbp.SetCone(x,y,imgL.Get(x,y).l,albedo,light,coneK*(1.0 - corL*coneFade));
   }
  }



 // Calculate the gradiants...
  svt::Field<real32> imgReal(image,"lb");
  filter::GradWalkPerfect(imgReal,imgGradX,imgGradY,gradCalcSteps,gradCalcExp,gradCalcStop,&con.BeginProg());
  con.EndProg();
  //filter::GradBilatGauss(imgLuv,imgGradX,imgGradY,gradSpatialSd,gradDomainSd,3.0,&con.BeginProg());
  //con.EndProg();
  
  file::SaveGradient(imgGradX,imgGradY,"grad");



 // Add gradient following information, so it prefers directions that point
 // along gradients...
  for (int32 y=0;y<int32(imgL.Size(1));y++)
  {
   for (int32 x=0;x<int32(imgL.Size(0));x++)
   {
    // Generate the distribution, multiply it in...
     math::Vect<3,real32> dir;
     dir[0] = imgGradY.Get(x,y);
     dir[1] = -imgGradX.Get(x,y);
     dir[2] = 0.0;
     real32 len = dir.Length();
     if (!math::IsZero(len))
     {
      dir.Normalise();
      // The disc component...
       math::FisherBingham fb(len*gradEV,dir);
       sfsbp.MultDist(x,y,fb);
      
      // The bias component...
      // Behaviour changes depending on sign of gradBias...
      // (Have a rotate to cone step.)
       // Get vector in relevant direction...
        math::Swap(dir[0],dir[1]);
        dir[0] *= -1.0;
        if (gradBias>0.0) dir *= -1.0;
       
       // Rotate to nearest point on cone...     
        bs::Normal grad;
        CrossProduct(light,dir,grad);

        real32 coneAng = math::InvCos(math::Min(imgL.Get(x,y).l/albedo,real32(1.0)));
       
        math::Mat<3,3> rotMat;
        AngAxisToRotMat(grad,coneAng,rotMat);
        MultVect(rotMat,light,dir);        
       
       // Generate and multiply in the fisher distribution...
        math::Fisher f(dir,math::Abs(len*gradBias));
        sfsbp.MultDist(x,y,f);
     }
   }
  }



 // Add boundary constraint, as shape from shading and smoothing alone are not enough...
  for (int32 y=0;y<int32(imgL.Size(1));y++)
  {
   for (int32 x=0;x<int32(imgL.Size(0));x++)
   {
    if (!math::IsZero(imgL.Get(x,y).l))
    {
     bit boundary = false;
     boundary |= (x!=0) && (math::IsZero(imgL.Get(x-1,y).l));
     boundary |= (x+1<int32(imgL.Size(0))) && (math::IsZero(imgL.Get(x+1,y).l));
     boundary |= (y!=0) && (math::IsZero(imgL.Get(x,y-1).l));
     boundary |= (y+1<int32(imgL.Size(1))) && (math::IsZero(imgL.Get(x,y+1).l));
    
     if (boundary)
     {
      math::Fisher fish;
      fish[0] = -imgGradX.Get(x,y); fish[1] = -imgGradY.Get(x,y); fish[2] = 0.0;
      fish.Normalise();
      fish *= borderK;
      sfsbp.MultDist(x,y,fish);
     }
    }
   }
  }



 // Add a centre vector - this forces it to aim the vectors towards the viewer...
 {
  math::Fisher fish;
  fish[0] = 0.0; fish[1] = 0.0; fish[2] = centreK;
  sfsbp.MultDist(imgL.Size(0)/2,imgL.Size(1)/2,fish);
 }



 // Run...
  sfsbp.SetIters(iters,levels);
  sfsbp.Run(&con.BeginProg());
  con.EndProg();
  sfsbp.GetDir(imgNeedle);


 
 // Extract probabilities for each maximum, so we can see how reliable things are, relativly speaking...
  time::Progress & prog = con.BeginProg();
  for (nat32 y=0;y<imgProb.Size(1);y++)
  {
   prog.Report(y,imgProb.Size(1));
   for (nat32 x=0;x<imgProb.Size(0);x++)
   {
    math::FisherBingham fb;
    sfsbp.GetDist(x,y,fb);
    real32 normDiv = fb.NormDiv();
    imgProb.Get(x,y) = math::Ln(fb.UnnormProb(imgNeedle.Get(x,y))/normDiv);
   }
  }
  con.EndProg();



 // Generate a 3D model by integrating the needle map...
  inf::IntegrateBP ibp(imgNeedle.Size(0),imgNeedle.Size(1));
  ibp.SetVal(imgNeedle.Size(0)/2,imgNeedle.Size(1)/2,0.0,100.0);
  ibp.SetIters(imgNeedle.Size(0)+imgNeedle.Size(1));
  real32 lim = 10.0;
  for (nat32 y=0;y<imgNeedle.Size(1);y++)
  {
   for (nat32 x=0;x<imgNeedle.Size(0);x++)
   {
    bs::Normal n = imgNeedle.Get(x,y);
    if ((!math::IsZero(n[2]))&&(n[2]>0.0))
    {
     n /= n[2];
     ibp.SetRel(x,y,0,1.0,math::Clamp( n[0]*zMult,real32(-lim),real32(lim)),1.0);
     ibp.SetRel(x,y,1,1.0,math::Clamp( n[1]*zMult,real32(-lim),real32(lim)),1.0);
     ibp.SetRel(x,y,2,1.0,math::Clamp(-n[0]*zMult,real32(-lim),real32(lim)),1.0);
     ibp.SetRel(x,y,3,1.0,math::Clamp(-n[1]*zMult,real32(-lim),real32(lim)),1.0);
    }
   }
  }
  
  ibp.Run(&con.BeginProg());
  con.EndProg();
  
  file::Ply ply;
  // Vertices...
  ds::Array2D<nat32> ind(imgNeedle.Size(0),imgNeedle.Size(1));
  real32 scaler = 50.0;
  for (nat32 y=0;y<imgNeedle.Size(1);y++)
  {
   for (nat32 x=0;x<imgNeedle.Size(0);x++)
   {
    bs::Vert v;
    v[0] = x/scaler;
    v[1] = y/scaler;
    real32 z = ibp.Expectation(x,y);
    if (math::IsFinite(z)) v[2] = math::Clamp(z/scaler,real32(-10.0),real32(10.0));
                      else v[2] = -10.0;
    
    bs::Tex2D uv;
    uv[0] = x / real32(imgNeedle.Size(0));
    uv[1] = y / real32(imgNeedle.Size(1));
    ind.Get(x,y) = ply.Add(v,uv);
   }
  }
  
  // Faces...
   for (nat32 y=0;y<imgNeedle.Size(1)-1;y++)
   {
    for (nat32 x=0;x<imgNeedle.Size(0)-1;x++)
    {
     ply.Add(ind.Get(x,y));
     ply.Add(ind.Get(x+1,y));
     ply.Add(ind.Get(x+1,y+1));
     ply.Add(ind.Get(x,y+1));
     ply.Face();
    }
   }



 // Save the final needle map, the probability map, and the 3D model...
  file::SaveNeedle(imgNeedle,"needle");
  file::SaveReal(imgProb,"prob");
  ply.Save("mesh.ply",true,&con.BeginProg());
  con.EndProg();


 return 0;
}

//------------------------------------------------------------------------------
