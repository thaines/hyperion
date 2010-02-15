//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "sur_bp_stereo/main.h"

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
   con << "Usage: sur_bp_stereo [left.bmp] [right.bmp] [calibration.pair] [config.xml]\n";
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



 // Transform pair so left camera at origin, changes nothing but allows certain
 // assumptions to be made...
  math::Mat<4,4,real64> toCS;
  pair.LeftToDefault(&toCS);



 // Create the algorithm objects, load the parameters file and fill them all in,
 // with defaults as needed...
  stereo::SparseDSI2 dsi;
  stereo::SegmentDSI segy;
  stereo::SparsePos to3D;
  stereo::AlbedoEstimate3 albEst;
  stereo::SurfaceSfS sfs;
  stereo::RefineNorm bpRefine;

  bs::Element * paras = file::LoadXML(tt,argv[4]);
  if (paras==null<bs::Element*>())
  {
   delete root;
   con << "Unable to load algortihm configuration.\n";
   return 1;
  }

  dsi.Set(paras->GrabReal(":dsi.occCost",8.0),
          paras->GrabReal(":dsi.vertCost",0.4),
          paras->GrabReal(":dsi.vertMult",0.5),
          paras->GrabReal(":dsi.errLim",0.0));

  segy.Set(paras->GrabReal(":seg.colour",4.5),
                  paras->GrabReal(":seg.spatial",7.0),
                  paras->GrabReal(":seg.disp",7.0),
                  paras->GrabInt(":seg.minMerge",20),
                  paras->GrabInt(":seg.minKill",0));

  {
   bs::Vertex lightPos;
   str::String def("(0,0,0,1)");
   str::String s = paras->GrabString(":light.pos",def);
   str::String::Cursor targ = s.GetCursor();
   targ >> lightPos;

   bs::Vertex lightPos2;
   math::MultVect(toCS,lightPos,lightPos2);

   albEst.SetLight(lightPos2);
  }

  /*albEst.Set(paras->GrabReal(":alb.sur_sd",2.0),
             paras->GrabReal(":alb.dosa_sd",0.1),
             paras->GrabReal(":alb.soda_sd",0.25),
             paras->GrabReal(":alb.cutoff",5.0),
             paras->GrabInt(":alb.iters",100));*/
  albEst.Set(paras->GrabInt(":alb.radius",5));

  sfs.Set(paras->GrabInt(":sfs.iters",100));

  bpRefine.Set(paras->GrabInt(":refine.iters",100),
               paras->GrabReal(":refine.dsiSd",1.0),
               paras->GrabReal(":refine.costBase",2.0),
               paras->GrabReal(":refine.norm",0.4),
               paras->GrabReal(":refine.maxD",5.0));



 // Augment the images with lots and lots of fields...
  bs::ColourLuv luvIni(0.0,0.0,0.0);
  real32 albIni = 0.0;
  real32 dispIni = 0.0;
  nat32 segIni = 0;
  bs::Normal needleIni(0.0,0.0,1.0);
  bit maskIni;

  left->Add("luv",luvIni);
  left->Add("albedo",albIni);
  left->Add("disp_ini",dispIni);
  left->Add("disp_smooth",dispIni);
  left->Add("disp",dispIni);
  left->Add("seg",segIni);
  left->Add("seg-bg",segIni);
  left->Add("needle_ini",needleIni);
  left->Add("needle_smooth",needleIni);
  left->Add("needle_sfs",needleIni);
  left->Add("needle",needleIni);
  left->Add("mask",maskIni);
  left->Commit();

  right->Add("luv",luvIni);
  right->Commit();



 // Obtain field objects to represent all of the above...
  svt::Field<bs::ColourRGB> leftRGB(left,"rgb");
  svt::Field<bs::ColourLuv> leftLuv(left,"luv");
  svt::Field<real32> leftAlbedo(left,"albedo");
  svt::Field<real32> leftDispIni(left,"disp_ini");
  svt::Field<real32> leftDispSmooth(left,"disp_smooth");
  svt::Field<real32> leftDisp(left,"disp");
  svt::Field<nat32> leftSeg(left,"seg");
  svt::Field<nat32> leftSegBG(left,"seg-bg");
  svt::Field<bs::Normal> leftNeedleIni(left,"needle_ini");
  svt::Field<bs::Normal> leftNeedleSmooth(left,"needle_smooth");
  svt::Field<bs::Normal> leftNeedleSfs(left,"needle_sfs");
  svt::Field<bs::Normal> leftNeedle(left,"needle");
  svt::Field<bit> leftMask(left,"mask");

  svt::Field<bs::ColourRGB> rightRGB(right,"rgb");
  svt::Field<bs::ColourLuv> rightLuv(right,"luv");



 // Convert the loaded rgb images into luv...
  filter::RGBtoLuv(left);
  filter::RGBtoLuv(right);



 // Run...
  // DSI generation...
   stereo::BoundLuvDSC dsc(leftLuv,rightLuv);
   dsi.Set(&dsc);

   con << "Dsi generation...\n";
   dsi.Run(&con.BeginProg());
   con.EndProg();


  // Sort DSI by cost...
   con << "Dsi sorting...\n";
   dsi.SortByCost(&con.BeginProg());
   con.EndProg();

   for (nat32 y=0;y<leftDispIni.Size(1);y++)
   {
    for (nat32 x=0;x<leftDispIni.Size(0);x++) leftDispIni.Get(x,y) = dsi.Disp(x,y,0);
   }


  // Segmentation...
   segy.Set(leftLuv);
   segy.Set(dsi);

   con << "Segmentation...\n";
   segy.Run(&con.BeginProg());
   con.EndProg();

   nat32 segments = segy.Segments();
   segy.GetSeg(leftSeg);


  // Convert disparity to 3d positions...
   to3D.Set(1);

   con << "Disparity to 3D...\n";
   to3D.Set(dsi,pair,&con.BeginProg());
   con.EndProg();


  // Fit planes to generate initial normal map...
  {
   con << "Plane fitting segments...\n";
   time::Progress & prog = con.BeginProg();

   ds::ArrayDel<alg::LinePlaneFit> planeFit(segments);

   for (nat32 y=0;y<leftSeg.Size(1);y++)
   {
    for (nat32 x=0;x<leftSeg.Size(0);x++)
    {
     for (nat32 i=0;i<to3D.Size(x,y);i++) planeFit[leftSeg.Get(x,y)].Add(to3D.Start(x,y,i),to3D.End(x,y,i),1.0);
    }
   }

   for (nat32 i=0;i<segments;i++)
   {
    prog.Report(i,segments);
    planeFit[i].Run();
   }

   for (nat32 y=0;y<leftNeedleIni.Size(1);y++)
   {
    for (nat32 x=0;x<leftNeedleIni.Size(0);x++)
    {
     leftNeedleIni.Get(x,y) = planeFit[leftSeg.Get(x,y)].Plane().n;
     if (leftNeedleIni.Get(x,y)[2]<0.0) leftNeedleIni.Get(x,y) *= -1.0;
    }
   }

   con.EndProg();
  }


  // Some last minute configuration...
   bit eps = paras->GrabBit(":refine-ex.eps",false);
   stereo::LuvDSC luvDSC(leftLuv,rightLuv);
   if (eps)
   {
    bpRefine.Set(luvDSC,
                 paras->GrabReal(":refine-ex.threshold",1.0),
                 paras->GrabReal(":refine-ex.width",0.25));
   }


  // Run refine bp with plane normals to get initial surface...
   bpRefine.Set(dsi);
   bpRefine.Set(leftNeedleIni);
   if (!eps) bpRefine.Set(leftSeg);
   bpRefine.Set(pair);

   con << "Initial Surface Fit...\n";
   bpRefine.Run(&con.BeginProg());
   con.EndProg();

   bpRefine.Disp(leftDispSmooth);
   pair.Convert(leftDispSmooth,leftNeedleSmooth);


  // Estimate albedo...
   albEst.SetImage(leftLuv);
   stereo::SurSegBasic ssb;
   ssb.Set(leftDispSmooth,leftSeg,pair);
   albEst.SetSurSeg(ssb);

   con << "Albedo Estimation...\n";
   albEst.Run(&con.BeginProg());
   con.EndProg();

   albEst.GetAlbedo(leftAlbedo);
   for (nat32 y=0;y<leftAlbedo.Size(1);y++)
   {
    for (nat32 x=0;x<leftAlbedo.Size(0);x++)
    {
     if (leftAlbedo.Get(x,y)>400.0) leftAlbedo.Get(x,y) = 0.0;
    }
   }


  // 'Find' the background...
  // For each segment count its size and how many border pixels it has, if
  // border pixels multiplied by constant is greater than segment size its
  // background.
   nat32 background = 0;//ssb.GetBackground();
   {
    static const nat32 mult = 65;
    ds::Array<nat32> pixels(segy.Segments());
    ds::Array<nat32> bPixels(segy.Segments());
    for (nat32 i=0;i<pixels.Size();i++)
    {
     pixels[i] = 0;
     bPixels[i] = 0;
    }

    for (nat32 y=0;y<leftSeg.Size(1);y++)
    {
     for (nat32 x=0;x<leftSeg.Size(0);x++)
     {
      pixels[leftSeg.Get(x,y)] += 1;
      if ((x==0)||(y==0)||(x==leftSeg.Size(0)-1)||(y==leftSeg.Size(1)-1)) bPixels[leftSeg.Get(x,y)] += 1;
     }
    }

    ds::Array<bit> bg(segy.Segments());
    bg[0] = false;
    for (nat32 i=1;i<bg.Size();i++) bg[i] = pixels[i]<(bPixels[i]*mult);

    for (nat32 y=0;y<leftSeg.Size(1);y++)
    {
     for (nat32 x=0;x<leftSeg.Size(0);x++)
     {
      if (bg[leftSeg.Get(x,y)]) leftSeg.Get(x,y) = 0;
     }
    }
   }


  // Create the background segmentation if needed...
   if (eps)
   {
    for (nat32 y=0;y<leftSegBG.Size(1);y++)
    {
     for (nat32 x=0;x<leftSegBG.Size(0);x++)
     {
      leftSegBG.Get(x,y) = (leftSeg.Get(x,y)==background)?background:(background+1);
     }
    }
   }


  // Run sfs, initialise normals from initial surface...
   sfs.Set(leftLuv);
   sfs.Set(ssb);
   sfs.SetBG(background);
   sfs.Set(albEst.Light());
   sfs.Set(leftAlbedo);

   con << "SfS...\n";
   sfs.Run(&con.BeginProg());
   con.EndProg();

   for (nat32 y=0;y<leftNeedleSfs.Size(1);y++)
   {
    for (nat32 x=0;x<leftNeedleSfs.Size(0);x++) leftNeedleSfs.Get(x,y) = sfs.Orient(x,y);
   }


  // Run refine bp with sfs normals...
   bpRefine.Set(dsi);
   bpRefine.Set(leftNeedleSfs);
   if (eps) bpRefine.Set(leftSegBG);
       else bpRefine.Set(leftSeg);
   bpRefine.Set(pair);

   con << "Final Surface Fit...\n";
   bpRefine.Run(&con.BeginProg());
   con.EndProg();

   bpRefine.Disp(leftDisp);
   pair.Convert(leftDisp,leftNeedle);



  // Quickly create a validity mask...
   for (nat32 y=0;y<leftMask.Size(1);y++)
   {
    for (nat32 x=0;x<leftMask.Size(0);x++) leftMask.Get(x,y) = background!=leftSeg.Get(x,y);
   }

  // Write out all stuff to file...
   con << "Finished. Saving output...\n";
   {
    time::Progress & prog = con.BeginProg();

    prog.Report(0,12);  file::SaveReal(leftAlbedo,"left_albedo");
    prog.Report(1,12);  file::SaveDisparity(leftDispIni,"left_disp_ini",&leftMask);
    prog.Report(2,12);  file::SaveDisparity(leftDispSmooth,"left_disp_smooth",&leftMask);
    prog.Report(3,12);  file::SaveDisparity(leftDisp,"left_disp",&leftMask);
    prog.Report(4,12);  file::SaveSeg(leftSeg,"left_seg");
    prog.Report(5,12);  file::SaveNeedle(leftNeedleIni,"left_needle_ini");
    prog.Report(6,12);  file::SaveNeedle(leftNeedleSmooth,"left_needle_smooth");
    prog.Report(7,12);  file::SaveNeedle(leftNeedleSfs,"left_needle_sfs");
    prog.Report(8,12);  file::SaveNeedle(leftNeedle,"left_needle");
    prog.Report(9,12);  file::SaveWarp(leftRGB,rightRGB,leftDispIni,"left_warp_ini",&leftMask);
    prog.Report(10,11); file::SaveWarp(leftRGB,rightRGB,leftDispSmooth,"left_warp_smooth",&leftMask);
    prog.Report(11,12); file::SaveWarp(leftRGB,rightRGB,leftDisp,"left_warp",&leftMask);

    con.EndProg();
   }



 // Clean up...
  delete paras;
  delete root;

 return 0;
}

//------------------------------------------------------------------------------
