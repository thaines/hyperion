//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "sur_stereo/main.h"

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
  if (argc!=5)
  {
   con << "Usage: sur_stereo [left.bmp] [right.bmp] [calibration.pair] [config.xml]\n";
   con << "[left.bmp] and [right.bmp] are two rectified images.\n";
   con << "[calibration.pair] is the calibration for the rectified pair.\n";
   con << "[config.xml] is an xml file that contains all the parameters.\n";
   return 1;
  }


 // Load the left and right images...
  svt::Var * left = filter::LoadImageRGB(core,argv[1]);
  if (left==null<svt::Var*>())
  {
   con << "Unable to load left image.\n";
   return 1;
  }

  svt::Var * right = filter::LoadImageRGB(core,argv[2]);
  if (right==null<svt::Var*>())
  {
   delete left;
   con << "Unable to load right image.\n";
   return 1;
  }
 
  svt::Node * root = new svt::Node(core);
  left->AttachParent(root);
  right->AttachParent(root);


 // Load the camera calibration, this gives us full capability to
 // triangulate etc coordinates from the rectified input...
  cam::CameraPair pair;
  if (pair.Load(argv[3])==false)
  {
   delete root;
   con << "Unable to load camera configuration.\n";
   return 1;
  }


 // Create the algorithm object, load the parameters file and fill them all in,
 // with defaults as needed...
  stereo::CloudStereo planeStereo;
  bs::Element * paras = file::LoadXML(tt,argv[4]);
  if (paras==null<bs::Element*>())
  {
   delete root;
   con << "Unable to load algortihm configuration.\n";
   return 1;   
  }

  planeStereo.Set(paras->GrabReal(":dsi.occCost",1.0),
                  paras->GrabReal(":dsi.vertCost",1.0),
                  paras->GrabReal(":dsi.vertMult",0.2),
                  paras->GrabReal(":dsi.errLim",0.1));
  planeStereo.Set(nat32(paras->GrabInt(":sel.radius",3)),
                  paras->GrabReal(":sel.mult",1.0),
                  nat32(paras->GrabInt(":sel.cap",3)));
  planeStereo.Set(paras->GrabReal(":ms.colour",4.5),
                  paras->GrabReal(":ms.spatial",7.0),
                  paras->GrabReal(":ms.disp",7.0),
                  paras->GrabInt(":ms.minMerge",20),
                  paras->GrabInt(":ms.minKill",0));
  planeStereo.Set(nat32(paras->GrabInt(":sur.radius",2)),
                  paras->GrabReal(":sur.damping",1.0),
                  paras->GrabReal(":sur.stepCost",0.5),
                  nat32(paras->GrabInt(":sur.iters",100)));


 // Load the light information...
  bs::Vertex lightPos;
  {
   str::String def("(0,0,0,1)");
   str::String s = paras->GrabString(":light.pos",def);
   str::String::Cursor targ = s.GetCursor();
   targ >> lightPos;
  }


 // Add luv, luminence, albedo, disparity, needle, confidence, dx and dy maps to both images...
  bs::ColourLuv luvIni(0.0,0.0,0.0);
  bs::ColourL lIni(0.0);
  real32 dispIni = 0.0;
  nat32 segIni = 0;
  bs::Normal needleIni(0.0,0.0,1.0);
  bit bitIni = false;

  left->Add("l",lIni);
  left->Add("luv",luvIni);
  left->Add("albedo",lIni);
  left->Add("disp",dispIni);
  left->Add("disp2",dispIni);
  left->Add("seg",segIni);
  left->Add("needle",needleIni);
  left->Add("needle_pre",needleIni);
  left->Add("needle_post",needleIni);
  left->Add("specMask",bitIni);
  left->Commit();

  right->Add("luv",luvIni);
  right->Commit();


 // Obtain field objects to represent all of the above...
  svt::Field<bs::ColourL> leftL(left,"l");
  svt::Field<bs::ColourRGB> leftRGB(left,"rgb");
  svt::Field<bs::ColourLuv> leftLuv(left,"luv");
  svt::Field<bs::ColourL> leftAlbedo(left,"albedo");
  svt::Field<real32> leftDisp(left,"disp");
  svt::Field<real32> leftDisp2(left,"disp2");
  svt::Field<nat32> leftSeg(left,"seg");
  svt::Field<bs::Normal> leftNeedle(left,"needle");
  svt::Field<bs::Normal> leftNeedlePre(left,"needle_pre");
  svt::Field<bs::Normal> leftNeedlePost(left,"needle_post");
  svt::Field<bit> leftSpecMask(left,"specMask");

  svt::Field<bs::ColourRGB> rightRGB(right,"rgb");   
  svt::Field<bs::ColourLuv> rightLuv(right,"luv");


 // Convert the loaded rgb images into luv...
  filter::RGBtoLuv(left);
  filter::RGBtoLuv(right);


 // Set the remaining values for the algorithm...
  planeStereo.Set(leftLuv,rightLuv);
  planeStereo.Set(pair);


 // Run...
  planeStereo.Run(&con.BeginProg());
  con.EndProg();
  planeStereo.GetDisp(leftDisp);
  planeStereo.GetNorm(leftNeedlePre);


 // Do specularity detection on the input...
  filter::SimpleSpecMask(leftRGB,leftSeg,leftSpecMask);


 // Extra step - calculate albedo, stick it into field...
  stereo::AlbedoEstimate albedo;
  albedo.SetLight(lightPos);
  albedo.SetImage(leftLuv);
  albedo.SetMask(leftSpecMask);
  albedo.SetSurSeg(planeStereo);
  //albedo.EnableDebug();
  
  albedo.Run(&con.BeginProg());
  con.EndProg();
  
  real32 mult = 1.0;
  for (nat32 i=0;i<planeStereo.Segments();i++)
  {
   LogDebug("albedo {seg,albedo}" << LogDiv() << i << LogDiv() << albedo.Albedo(i));
   mult = math::Max(mult,albedo.Albedo(i));
  }
  mult = 1.0/mult;
  
  for (nat32 y=0;y<leftAlbedo.Size(1);y++)
  {
   for (nat32 x=0;x<leftAlbedo.Size(0);x++)
   {
    leftAlbedo.Get(x,y) = albedo.Albedo(planeStereo.Seg(x,y))*mult;
   }
  }


 // Extra, extra step - sfs, stick into field...
  stereo::SurfaceSfS surfaceSfS;
  surfaceSfS.Set(leftLuv);
  surfaceSfS.Set(planeStereo);
  surfaceSfS.SetBG(planeStereo.GetBackground());
  surfaceSfS.Set(albedo);
  
  surfaceSfS.Run(&con.BeginProg());
  con.EndProg();
  
  for (nat32 y=0;y<leftNeedle.Size(1);y++)
  {
   for (nat32 x=0;x<leftNeedle.Size(0);x++)
   {
    leftNeedle.Get(x,y) = surfaceSfS.Orient(x,y);
   }
  }


 // Extra X 3 step - re-calculate, forcing the planes to orient with the needle 
 // map...
  planeStereo.ReRun(leftNeedle,&con.BeginProg());
  con.EndProg();
  planeStereo.GetDisp(leftDisp2);
  planeStereo.GetNorm(leftNeedlePost);



 // Output stuff...
  // Warp of left to right...
   stereo::ForwardWarp(leftRGB,leftDisp,rightRGB);
   if (filter::SaveImageRGB(right,"left_warp.bmp",true)==false)
   {
    con << "Failed to save left_warp.bmp\n";
   }
   
  // Warp of left to right, for disp2...
   stereo::ForwardWarp(leftRGB,leftDisp2,rightRGB);
   if (filter::SaveImageRGB(right,"left_warp2.bmp",true)==false)
   {
    con << "Failed to save left_warp2.bmp\n";
   }   

  // Albedo...
   filter::LtoRGB(leftAlbedo,leftRGB);  
   if (filter::SaveImageRGB(left,"left_albedo.bmp",true)==false)
   {
    con << "Failed to save left_albedo.bmp\n";
   }

  // Segmentation...
   planeStereo.GetSeg(leftSeg);
   filter::RenderSegsColour(leftSeg,leftRGB);
   if (filter::SaveImageRGB(left,"left_seg.bmp",true)==false)
   {
    con << "Failed to save left_seg.bmp\n";
   }
   
  // Specularity mask...
   filter::MaskToRGB(leftSpecMask,leftRGB);
   if (filter::SaveImageRGB(left,"left_spec_mask.bmp",true)==false)
   {
    con << "Failed to save left_spec_mask.bmp\n";
   }

  // Disparity...
   stereo::RenderDisp(leftDisp,leftL);
   filter::LtoRGB(left);
   if (filter::SaveImageRGB(left,"left_disp.bmp",true)==false)
   {
    con << "Failed to save left_disp.bmp\n";
   }
  
  // Disparity .odf file...
   {
    svt::Var leftD(leftDisp);
     real32 iniDisp = 0.0;
     leftD.Add("disp",iniDisp);
    leftD.Commit();
    
    svt::Field<real32> d(&leftD,"disp");
    for (nat32 y=0;y<d.Size(1);y++)
    {
     for (nat32 x=0;x<d.Size(0);x++)
     {
      d.Get(x,y) = leftDisp.Get(x,y);
     }
    }
    
    if (svt::Save("left_disp.obf",&leftD,true)==false)
    {
     con << "Failed to save left_disp.obf\n";
    }
   }

  // Disparity 2...
   stereo::RenderDisp(leftDisp2,leftL);
   filter::LtoRGB(left);
   if (filter::SaveImageRGB(left,"left_disp2.bmp",true)==false)
   {
    con << "Failed to save left_disp2.bmp\n";
   }
  
  // Disparity 2 .odf file...
   {
    svt::Var leftD(leftDisp);
     real32 iniDisp = 0.0;
     leftD.Add("disp",iniDisp);
    leftD.Commit();
    
    svt::Field<real32> d(&leftD,"disp");
    for (nat32 y=0;y<d.Size(1);y++)
    {
     for (nat32 x=0;x<d.Size(0);x++)
     {
      d.Get(x,y) = leftDisp2.Get(x,y);
     }
    }
    
    if (svt::Save("left_disp2.obf",&leftD,true)==false)
    {
     con << "Failed to save left_disp2.obf\n";
    }
   }

  // Needle map...
   rend::VisNeedleMap(leftNeedle,leftRGB);
   if (filter::SaveImageRGB(left,"left_needle.bmp",true)==false)
   {
    con << "Failed to save left_needle.bmp\n";
   }

   /*{
    file::Wavefront mod;
    rend::NeedleMapToModel(leftNeedle,mod);
    if (mod.Save("left_needle.obj",true)==false)
    {
     con << "Failed to save left_needle.obj\n";
    }
   }*/
   
  // Pre-sfs needle map...
   rend::VisNeedleMap(leftNeedlePre,leftRGB);
   if (filter::SaveImageRGB(left,"left_needle_pre.bmp",true)==false)
   {
    con << "Failed to save left_needle_pre.bmp\n";
   }

   /*{
    file::Wavefront mod;
    rend::NeedleMapToModel(leftNeedle,mod);
    if (mod.Save("left_needle_pre.obj",true)==false)
    {
     con << "Failed to save left_needle_pre.obj\n";
    }
   }*/

  // Post needle map...   
   rend::VisNeedleMap(leftNeedlePost,leftRGB);
   if (filter::SaveImageRGB(left,"left_needle_post.bmp",true)==false)
   {
    con << "Failed to save left_needle_post.bmp\n";
   }

   /*{
    file::Wavefront mod;
    rend::NeedleMapToModel(leftNeedle,mod);
    if (mod.Save("left_needle_post.obj",true)==false)
    {
     con << "Failed to save left_needle_post.obj\n";
    }
   }*/


 // Clean up...
  delete paras;
  delete root;

 return 0;
}

//------------------------------------------------------------------------------
