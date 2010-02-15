//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "cyclops/main.h"
#include "cyclops/intrinsic.h"
#include "cyclops/protractor.h"
#include "cyclops/fundamental.h"
#include "cyclops/triangulator.h"
#include "cyclops/crop.h"
#include "cyclops/crop_pair.h"
#include "cyclops/rectification.h"
#include "cyclops/capture.h"
#include "cyclops/to_mesh.h"
#include "cyclops/stereopsis.h"
#include "cyclops/warp.h"
#include "cyclops/scale_pair.h"
#include "cyclops/homography.h"
#include "cyclops/calibration.h"
#include "cyclops/model_disp.h"
#include "cyclops/comp_disp.h"
#include "cyclops/file_info.h"
#include "cyclops/disp_mask.h"
#include "cyclops/colour_balance.h"
#include "cyclops/undistorter.h"
#include "cyclops/disp_scale.h"
#include "cyclops/to_orient.h"
#include "cyclops/model_render.h"
#include "cyclops/anaglyph.h"
#include "cyclops/disp_clean.h"
#include "cyclops/scale.h"
#include "cyclops/disp_crop.h"
#include "cyclops/mesh_ops.h"
#include "cyclops/comp_needle.h"
#include "cyclops/sfs.h"
#include "cyclops/integration.h"
#include "cyclops/lighting.h"
#include "cyclops/segmentation.h"
#include "cyclops/light_est.h"

//------------------------------------------------------------------------------
// Code for cyclops class...
Cyclops::Cyclops()
:tokTab(),core(tokTab),guiFact(tokTab),app(null<gui::App*>()),win(null<gui::Window*>())
{
 if (guiFact.Active()==false) return;

 // Build the gui control panel...
  app = static_cast<gui::App*>(guiFact.Make("App"));
  win = static_cast<gui::Window*>(guiFact.Make("Window"));

  win->SetTitle("Cyclops");
  win->SetSize(1,1);

  gui::Grid * grid = static_cast<gui::Grid*>(guiFact.Make("Grid"));
  win->SetChild(grid);
  grid->SetDims(6,3);


  prog = static_cast<gui::ProgressBar*>(guiFact.Make("ProgressBar"));
  prog->SetSize(256,24);
  grid->Attach(0,1,prog,6);

  gui::Vertical * vert0 = static_cast<gui::Vertical*>(guiFact.Make("Vertical"));
  gui::Vertical * vert1 = static_cast<gui::Vertical*>(guiFact.Make("Vertical"));
  gui::Vertical * vert2 = static_cast<gui::Vertical*>(guiFact.Make("Vertical"));
  gui::Vertical * vert3 = static_cast<gui::Vertical*>(guiFact.Make("Vertical"));
  gui::Vertical * vert4 = static_cast<gui::Vertical*>(guiFact.Make("Vertical"));
  gui::Vertical * vert5 = static_cast<gui::Vertical*>(guiFact.Make("Vertical"));
  grid->Attach(0,0,vert0);
  grid->Attach(1,0,vert1);
  grid->Attach(2,0,vert2);
  grid->Attach(3,0,vert3);
  grid->Attach(4,0,vert4);
  grid->Attach(5,0,vert5);

  gui::Vertical * vert6 = static_cast<gui::Vertical*>(guiFact.Make("Vertical"));
  gui::Vertical * vert7 = static_cast<gui::Vertical*>(guiFact.Make("Vertical"));
  gui::Vertical * vert8 = static_cast<gui::Vertical*>(guiFact.Make("Vertical"));
  gui::Vertical * vert9 = static_cast<gui::Vertical*>(guiFact.Make("Vertical"));
  gui::Vertical * vert10 = static_cast<gui::Vertical*>(guiFact.Make("Vertical"));
  gui::Vertical * vert11 = static_cast<gui::Vertical*>(guiFact.Make("Vertical"));
  grid->Attach(0,2,vert6);
  grid->Attach(1,2,vert7);
  grid->Attach(2,2,vert8);
  grid->Attach(3,2,vert9);
  grid->Attach(4,2,vert10);
  grid->Attach(5,2,vert11);


  gui::Button * but1 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but2 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but3 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but4 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but5 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but6 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but7 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but8 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but9 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but10 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but11 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but12 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but13 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but14 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but15 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but16 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but17 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but18 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but19 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but20 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but21 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but22 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but23 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but24 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but25 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but26 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but27 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but28 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but29 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but30 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but31 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but32 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but33 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but34 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but35 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but36 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but37 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but38 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but39 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but40 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but41 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but42 = static_cast<gui::Button*>(guiFact.Make("Button"));
  gui::Button * but43 = static_cast<gui::Button*>(guiFact.Make("Button"));

  gui::Label * lab1 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab2 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab3 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab4 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab5 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab6 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab7 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab8 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab9 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab10 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab11 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab12 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab13 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab14 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab15 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab16 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab17 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab18 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab19 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab20 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab21 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab22 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab23 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab24 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab25 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab26 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab27 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab28 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab29 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab30 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab31 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab32 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab33 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab34 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab35 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab36 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab37 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab38 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab39 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab40 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab41 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab42 = static_cast<gui::Label*>(guiFact.Make("Label"));
  gui::Label * lab43 = static_cast<gui::Label*>(guiFact.Make("Label"));

  but1->SetChild(lab1); lab1->Set("Intrinsic Calibration");
  but2->SetChild(lab2); lab2->Set("Protractor");
  but3->SetChild(lab3); lab3->Set("Fundamental Calibration");
  but4->SetChild(lab4); lab4->Set("Triangulator");
  but5->SetChild(lab5); lab5->Set("Rectification");
  but6->SetChild(lab6); lab6->Set("Capture");
  but7->SetChild(lab7); lab7->Set("Crop");
  but8->SetChild(lab8); lab8->Set("Crop Rectified Pair");
  but9->SetChild(lab9); lab9->Set("Swap Pair");
  but10->SetChild(lab10); lab10->Set("Disparity To Mesh");
  but11->SetChild(lab11); lab11->Set("Stereopsis");
  but12->SetChild(lab12); lab12->Set("Warp");
  but13->SetChild(lab13); lab13->Set("Scale Rectified Pair");
  but14->SetChild(lab14); lab14->Set("Homography");
  but15->SetChild(lab15); lab15->Set("Camera Calibration");
  but16->SetChild(lab16); lab16->Set("Cameras to Pair");
  but17->SetChild(lab17); lab17->Set("Mesh to Disparity");
  but18->SetChild(lab18); lab18->Set("Disparity Comparator");
  but19->SetChild(lab19); lab19->Set("Manual");
  but20->SetChild(lab20); lab20->Set("Calibration Target");
  but21->SetChild(lab21); lab21->Set("Tutorial Directory");
  but22->SetChild(lab22); lab22->Set("About");
  but23->SetChild(lab23); lab23->Set("File Info");
  but24->SetChild(lab24); lab24->Set("Disparity Masking");
  but25->SetChild(lab25); lab25->Set("Colour Matching");
  but26->SetChild(lab26); lab26->Set("Undistorter");
  but27->SetChild(lab27); lab27->Set("Scale Disparity");
  but28->SetChild(lab28); lab28->Set("Disparity To Needle Map");
  but29->SetChild(lab29); lab29->Set("Render Mesh");
  but30->SetChild(lab30); lab30->Set("Anaglyph");
  but31->SetChild(lab31); lab31->Set("Pair to Cameras");
  but32->SetChild(lab32); lab32->Set("Camera To Intrinsic");
  but33->SetChild(lab33); lab33->Set("Disparity Cleaner");
  but34->SetChild(lab34); lab34->Set("Scale");
  but35->SetChild(lab35); lab35->Set("Crop Disparity");
  but36->SetChild(lab36); lab36->Set("Mesh Wibbalizer");
  but37->SetChild(lab37); lab37->Set("Needle Comparator");
  but38->SetChild(lab38); lab38->Set("Shape from Shading");
  but39->SetChild(lab39); lab39->Set("Integration");
  but40->SetChild(lab40); lab40->Set("Re-lighter");
  but41->SetChild(lab41); lab41->Set("Segmentation");
  but42->SetChild(lab42); lab42->Set("Light Estimation");
  but43->SetChild(lab43); lab43->Set("Pair to Default");

  vert0->AttachBottom(but19,false);
  vert0->AttachBottom(but21,false);
  vert0->AttachBottom(but20,false);
  vert0->AttachBottom(but22,false);

  vert1->AttachBottom(but1,false);
  vert1->AttachBottom(but2,false);
  vert1->AttachBottom(but3,false);
  vert1->AttachBottom(but4,false);

  vert2->AttachBottom(but6,false);
  vert2->AttachBottom(but5,false);
  vert2->AttachBottom(but11,false);
  vert2->AttachBottom(but10,false);

  vert3->AttachBottom(but24,false);
  vert3->AttachBottom(but33,false);
  vert3->AttachBottom(but18,false);
  vert3->AttachBottom(but28,false);

  vert4->AttachBottom(but7,false);
  vert4->AttachBottom(but34,false);
  vert4->AttachBottom(but8,false);
  vert4->AttachBottom(but13,false);

  vert5->AttachBottom(but12,false);
  vert5->AttachBottom(but26,false);

  vert6->AttachBottom(but16,false);
  vert6->AttachBottom(but31,false);
  vert6->AttachBottom(but43,false);
  vert6->AttachBottom(but9,false);
  vert6->AttachBottom(but23,false);

  vert7->AttachBottom(but15,false);
  vert7->AttachBottom(but32,false);
  vert7->AttachBottom(but17,false);
  vert7->AttachBottom(but36,false);
  vert7->AttachBottom(but29,false);
  
  vert8->AttachBottom(but42,false);

  vert9->AttachBottom(but25,false);
  vert9->AttachBottom(but14,false);
  vert9->AttachBottom(but30,false);
  vert9->AttachBottom(but41,false);

  vert10->AttachBottom(but35,false);
  vert10->AttachBottom(but27,false);
  
  vert11->AttachBottom(but38,false);
  vert11->AttachBottom(but39,false);
  vert11->AttachBottom(but37,false);
  vert11->AttachBottom(but40,false);


  win->Resizable(false);
  app->Attach(win);


 // Add handlers to it all, and were done - the rest just takes care of itself...
  win->OnDeath(MakeCB(this,&Cyclops::Quit));
  but1->OnClick(MakeCB(this,&Cyclops::StartIntrinsic));
  but2->OnClick(MakeCB(this,&Cyclops::StartProtractor));
  but3->OnClick(MakeCB(this,&Cyclops::StartFundamental));
  but4->OnClick(MakeCB(this,&Cyclops::StartTriangulator));
  but5->OnClick(MakeCB(this,&Cyclops::StartRectification));
  but6->OnClick(MakeCB(this,&Cyclops::StartCapture));
  but7->OnClick(MakeCB(this,&Cyclops::StartCrop));
  but8->OnClick(MakeCB(this,&Cyclops::StartCropPair));
  but9->OnClick(MakeCB(this,&Cyclops::SwapPair));
  but10->OnClick(MakeCB(this,&Cyclops::StartToMesh));
  but11->OnClick(MakeCB(this,&Cyclops::StartStereopsis));
  but12->OnClick(MakeCB(this,&Cyclops::StartWarp));
  but13->OnClick(MakeCB(this,&Cyclops::StartScalePair));
  but14->OnClick(MakeCB(this,&Cyclops::StartHomography));
  but15->OnClick(MakeCB(this,&Cyclops::StartCalibration));
  but16->OnClick(MakeCB(this,&Cyclops::StartMakePair));
  but17->OnClick(MakeCB(this,&Cyclops::StartMakeDisp));
  but18->OnClick(MakeCB(this,&Cyclops::StartCompareDisp));
  but19->OnClick(MakeCB(this,&Cyclops::StartHelp));
  but20->OnClick(MakeCB(this,&Cyclops::StartTarget));
  but21->OnClick(MakeCB(this,&Cyclops::StartTutorial));
  but22->OnClick(MakeCB(this,&Cyclops::StartAbout));
  but23->OnClick(MakeCB(this,&Cyclops::StartFileInfo));
  but24->OnClick(MakeCB(this,&Cyclops::StartDispMask));
  but25->OnClick(MakeCB(this,&Cyclops::StartColourBalance));
  but26->OnClick(MakeCB(this,&Cyclops::StartUndistorter));
  but27->OnClick(MakeCB(this,&Cyclops::StartScaleDisparity));
  but28->OnClick(MakeCB(this,&Cyclops::StartToOrient));
  but29->OnClick(MakeCB(this,&Cyclops::StartModelRender));
  but30->OnClick(MakeCB(this,&Cyclops::StartAnaglyph));
  but31->OnClick(MakeCB(this,&Cyclops::StartPairToCameras));
  but32->OnClick(MakeCB(this,&Cyclops::StartCameraToIntrinsic));
  but33->OnClick(MakeCB(this,&Cyclops::StartDispClean));
  but34->OnClick(MakeCB(this,&Cyclops::StartScale));
  but35->OnClick(MakeCB(this,&Cyclops::StartDispCrop));
  but36->OnClick(MakeCB(this,&Cyclops::StartMeshOps));
  but37->OnClick(MakeCB(this,&Cyclops::StartCompNeedle));
  but38->OnClick(MakeCB(this,&Cyclops::StartSfS));
  but39->OnClick(MakeCB(this,&Cyclops::StartIntegration));
  but40->OnClick(MakeCB(this,&Cyclops::StartLighting));
  but41->OnClick(MakeCB(this,&Cyclops::StartSegmentation));
  but42->OnClick(MakeCB(this,&Cyclops::StartLightEst));
  but43->OnClick(MakeCB(this,&Cyclops::PairToDefault));

 // Enter the message pump...
  app->Go();
}

Cyclops::~Cyclops()
{
 delete app;
}

void Cyclops::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 if (app->ChoiceDialog(gui::App::QuestYesNo,"Are you sure you want to quit?")) app->Die();
}

void Cyclops::StartIntrinsic(gui::Base * obj,gui::Event * event)
{
 new IntrinsicCalib(*this);
}

void Cyclops::StartProtractor(gui::Base * obj,gui::Event * event)
{
 new Protractor(*this);
}

void Cyclops::StartFundamental(gui::Base * obj,gui::Event * event)
{
 new Fundamental(*this);
}

void Cyclops::StartTriangulator(gui::Base * obj,gui::Event * event)
{
 new Triangulator(*this);
}

void Cyclops::StartRectification(gui::Base * obj,gui::Event * event)
{
 new Rectification(*this);
}

void Cyclops::StartCapture(gui::Base * obj,gui::Event * event)
{
 new Capture(*this);
}

void Cyclops::StartCrop(gui::Base * obj,gui::Event * event)
{
 new Crop(*this);
}

void Cyclops::StartCropPair(gui::Base * obj,gui::Event * event)
{
 new CropPair(*this);
}

void Cyclops::SwapPair(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (App().LoadFileDialog("Select Camera Pair File","*.pcc",fn))
 {
  cam::CameraPair pair;
  if (pair.Load(fn)==false)
  {
   App().MessageDialog(gui::App::MsgErr,"Failed to load camera pair file");
  }
  else
  {
   // Simply swap it...
    pair.Swap();

   // Save back the file...
    if (App().SaveFileDialog("Save Camera Pair File...",fn))
    {
     if (!fn.EndsWith(".pcc")) fn += ".pcc";
     if (!pair.Save(fn,true))
     {
      App().MessageDialog(gui::App::MsgErr,"Error saving back the file.");
      return;
     }
    }
  }
 }
}

void Cyclops::StartToMesh(gui::Base * obj,gui::Event * event)
{
 new ToMesh(*this);
}

void Cyclops::StartStereopsis(gui::Base * obj,gui::Event * event)
{
 new Stereopsis(*this);
}

void Cyclops::StartWarp(gui::Base * obj,gui::Event * event)
{
 new Warp(*this);
}

void Cyclops::StartScalePair(gui::Base * obj,gui::Event * event)
{
 new ScalePair(*this);
}

void Cyclops::StartHomography(gui::Base * obj,gui::Event * event)
{
 new Homography(*this);
}

void Cyclops::StartCalibration(gui::Base * obj,gui::Event * event)
{
 new Calibration(*this);
}

void Cyclops::StartMakePair(gui::Base * obj,gui::Event * event)
{
 // Load the left .cam...
  cam::CameraFull left;
  {
   str::String fn;
   if (App().LoadFileDialog("Select Left Camera Projection","*.cam",fn)==false) return;
   if (left.Load(fn)==false)
   {
    App().MessageDialog(gui::App::MsgErr,"Failed to load calibration file");
    return;
   }
  }


 // Load the right .cam...
  cam::CameraFull right;
  {
   str::String fn;
   if (App().LoadFileDialog("Select Right Camera Projection","*.cam",fn)==false) return;
   if (right.Load(fn)==false)
   {
    App().MessageDialog(gui::App::MsgErr,"Failed to load calibration file");
    return;
   }
  }


 // Create the pair object...
  cam::CameraPair pair;
  pair.FromFull(left,right);


 // Save the .pcc...
 {
  str::String fn(".pcc");
  if (App().SaveFileDialog("Save Camera Pair Calibration...",fn))
  {
   if (!fn.EndsWith(".pcc")) fn += ".pcc";
   // Save the file...
    if (!pair.Save(fn,true))
    {
     App().MessageDialog(gui::App::MsgErr,"Error saving the file.");
     return;
    }
  }
 }
}

void Cyclops::StartMakeDisp(gui::Base * obj,gui::Event * event)
{
 new ModelDisp(*this);
}

void Cyclops::StartCompareDisp(gui::Base * obj,gui::Event * event)
{
 new CompDisp(*this);
}

void Cyclops::StartHelp(gui::Base * obj,gui::Event * event)
{
 os::DoSomething("doc/manual.pdf");
}

void Cyclops::StartTarget(gui::Base * obj,gui::Event * event)
{
 os::DoSomething("doc/calibration_target.pdf");
}

void Cyclops::StartTutorial(gui::Base * obj,gui::Event * event)
{
 os::DoSomething("tut");
}

void Cyclops::StartAbout(gui::Base * obj,gui::Event * event)
{
 app->MessageDialog(gui::App::MsgInfo,CYCLOPS_ABOUT);
}

void Cyclops::StartFileInfo(gui::Base * obj,gui::Event * event)
{
 new FileInfo(*this);
}

void Cyclops::StartDispMask(gui::Base * obj,gui::Event * event)
{
 new DispMask(*this);
}

void Cyclops::StartColourBalance(gui::Base * obj,gui::Event * event)
{
 new ColourBalance(*this);
}

void Cyclops::StartUndistorter(gui::Base * obj,gui::Event * event)
{
 new Undistorter(*this);
}

void Cyclops::StartScaleDisparity(gui::Base * obj,gui::Event * event)
{
 new DispScale(*this);
}

void Cyclops::StartToOrient(gui::Base * obj,gui::Event * event)
{
 new ToOrient(*this);
}

void Cyclops::StartModelRender(gui::Base * obj,gui::Event * event)
{
 new ModelRender(*this);
}

void Cyclops::StartAnaglyph(gui::Base * obj,gui::Event * event)
{
 new Anaglyph(*this);
}

void Cyclops::StartPairToCameras(gui::Base * obj,gui::Event * event)
{
 // Load the pair file...
  cam::CameraPair pair;
  {
   str::String fn;
   if (App().LoadFileDialog("Load the Pair File","*.pcc",fn)==false) return;
   if (pair.Load(fn)==false)
   {
    App().MessageDialog(gui::App::MsgErr,"Failed to load pair file");
    return;
   }
  }


 // Extract the left and right cameras...
  cam::CameraFull left;
  cam::CameraFull right;
  {
   math::Mat<3,3,real64> temp;
   math::Mat<3,3,real64> rectLeft  = pair.unRectLeft;
   math::Mat<3,3,real64> rectRight = pair.unRectRight;
   math::Inverse(rectLeft,temp);
   math::Inverse(rectRight,temp);

   math::Mult(rectLeft,pair.lp,left.camera);
   left.radial = pair.left.radial;
   left.dim = pair.leftDim;

   math::Mult(rectRight,pair.rp,right.camera);
   right.radial = pair.right.radial;
   right.dim = pair.rightDim;
  }



 // Save the left camera...
 {
  str::String fn(".cam");
  if (App().SaveFileDialog("Save Left Camera Calibration...",fn))
  {
   if (!fn.EndsWith(".cam")) fn += ".cam";
   // Save the file...
    if (!left.Save(fn,true))
    {
     App().MessageDialog(gui::App::MsgErr,"Error saving the file.");
     return;
    }
  }
 }


 // Save the right camera...
 {
  str::String fn(".cam");
  if (App().SaveFileDialog("Save Right Camera Calibration...",fn))
  {
   if (!fn.EndsWith(".cam")) fn += ".cam";
   // Save the file...
    if (!right.Save(fn,true))
    {
     App().MessageDialog(gui::App::MsgErr,"Error saving the file.");
     return;
    }
  }
 }
}

void Cyclops::StartCameraToIntrinsic(gui::Base * obj,gui::Event * event)
{
 // Load the .cam...
  cam::CameraFull camera;
  {
   str::String fn;
   if (App().LoadFileDialog("Select Camera","*.cam",fn)==false) return;
   if (camera.Load(fn)==false)
   {
    App().MessageDialog(gui::App::MsgErr,"Failed to load calibration file");
    return;
   }
  }


 // Create the .icd...
  cam::CameraCalibration calib;
  math::Mat<3,3,real64> rot;

  camera.camera.Decompose(calib.intrinsic,rot);
  calib.radial = camera.radial;
  calib.dim = camera.dim;


 // Save the .icd...
 {
  str::String fn(".icd");
  if (App().SaveFileDialog("Save Camera Calibration...",fn))
  {
   if (!fn.EndsWith(".icd")) fn += ".icd";
   // Save the file...
    if (!calib.Save(fn,true))
    {
     App().MessageDialog(gui::App::MsgErr,"Error saving the file.");
     return;
    }
  }
 }
}

void Cyclops::StartDispClean(gui::Base * obj,gui::Event * event)
{
 new DispClean(*this);
}

void Cyclops::StartScale(gui::Base * obj,gui::Event * event)
{
 new Scale(*this);
}

void Cyclops::StartDispCrop(gui::Base * obj,gui::Event * event)
{
 new DispCrop(*this);
}

void Cyclops::StartMeshOps(gui::Base * obj,gui::Event * event)
{
 new MeshOps(*this);
}

void Cyclops::StartCompNeedle(gui::Base * obj,gui::Event * event)
{
 new CompNeedle(*this);
}

void Cyclops::StartSfS(gui::Base * obj,gui::Event * event)
{
 new SfS(*this);
}

void Cyclops::StartIntegration(gui::Base * obj,gui::Event * event)
{
 new Integration(*this);
}

void Cyclops::StartLighting(gui::Base * obj,gui::Event * event)
{
 new Lighting(*this);
}

void Cyclops::StartSegmentation(gui::Base * obj,gui::Event * event)
{
 new Segmentation(*this);
}

void Cyclops::StartLightEst(gui::Base * obj,gui::Event * event)
{
 new LightEst(*this);
}

void Cyclops::PairToDefault(gui::Base * obj,gui::Event * event)
{
 // Load the pair file...
  cam::CameraPair pair;
  {
   str::String fn;
   if (App().LoadFileDialog("Load the Pair File","*.pcc",fn)==false) return;
   if (pair.Load(fn)==false)
   {
    App().MessageDialog(gui::App::MsgErr,"Failed to load pair file");
    return;
   }
  }


 // Adjust so that the left camera is at (0,0,0) looking down the z axis in the
 // negative direction... 
  pair.LeftToDefault();


 // Save the pair file...
 {
  str::String fn(".pcc");
  if (App().SaveFileDialog("Save the Pair File...",fn))
  {
   if (!fn.EndsWith(".pcc")) fn += ".pcc";
   // Save the file...
    if (!pair.Save(fn,true))
    {
     App().MessageDialog(gui::App::MsgErr,"Error saving the file.");
     return;
    }
  }
 }
}

//------------------------------------------------------------------------------
// Program entry point...
EOS_APP_START
{
 Cyclops cyclops;
 return 0;
}

//------------------------------------------------------------------------------
