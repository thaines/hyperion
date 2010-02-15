#ifndef CYCLOPS_MAIN_H
#define CYCLOPS_MAIN_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "eos.h"

using namespace eos;

#define CYCLOPS_ABOUT "Cyclops Beta; " __DATE__ "; Copyright 2004-2008 Tom SF Haines. I may be contacted via e-mail at tom@thaines.net, the website for this program is at www.thaines.net/cyclops. By using this software you are agreeing to the terms in license.txt which must be in the executables directory and avaliable for you to access."

//------------------------------------------------------------------------------
class Cyclops
{
 public:
   Cyclops();
  ~Cyclops();

  str::TokenTable & TT() {return tokTab;}
  svt::Core & Core() {return core;}
  gui::Factory & Fact() {return guiFact;}
  gui::App & App() {return *app;}

  time::Progress * BeginProg() {return prog->Begin();}
  void EndProg() {prog->End();}


 private:
  str::TokenTable tokTab;
  svt::Core core;
  gui::GtkFactory guiFact;

  gui::App * app;
  gui::Window * win;
  gui::ProgressBar * prog;

  void Quit(gui::Base * obj,gui::Event * event);
  void StartIntrinsic(gui::Base * obj,gui::Event * event);
  void StartProtractor(gui::Base * obj,gui::Event * event);
  void StartFundamental(gui::Base * obj,gui::Event * event);
  void StartTriangulator(gui::Base * obj,gui::Event * event);
  void StartRectification(gui::Base * obj,gui::Event * event);
  void StartCapture(gui::Base * obj,gui::Event * event);
  void StartCrop(gui::Base * obj,gui::Event * event);
  void StartCropPair(gui::Base * obj,gui::Event * event);
  void SwapPair(gui::Base * obj,gui::Event * event);
  void StartToMesh(gui::Base * obj,gui::Event * event);
  void StartStereopsis(gui::Base * obj,gui::Event * event);
  void StartWarp(gui::Base * obj,gui::Event * event);
  void StartScalePair(gui::Base * obj,gui::Event * event);
  void StartHomography(gui::Base * obj,gui::Event * event);
  void StartCalibration(gui::Base * obj,gui::Event * event);
  void StartMakePair(gui::Base * obj,gui::Event * event);
  void StartMakeDisp(gui::Base * obj,gui::Event * event);
  void StartCompareDisp(gui::Base * obj,gui::Event * event);
  void StartHelp(gui::Base * obj,gui::Event * event);
  void StartTarget(gui::Base * obj,gui::Event * event);
  void StartTutorial(gui::Base * obj,gui::Event * event);
  void StartAbout(gui::Base * obj,gui::Event * event);
  void StartFileInfo(gui::Base * obj,gui::Event * event);
  void StartDispMask(gui::Base * obj,gui::Event * event);
  void StartColourBalance(gui::Base * obj,gui::Event * event);
  void StartUndistorter(gui::Base * obj,gui::Event * event);
  void StartScaleDisparity(gui::Base * obj,gui::Event * event);
  void StartToOrient(gui::Base * obj,gui::Event * event);
  void StartModelRender(gui::Base * obj,gui::Event * event);
  void StartAnaglyph(gui::Base * obj,gui::Event * event);
  void StartPairToCameras(gui::Base * obj,gui::Event * event);
  void StartCameraToIntrinsic(gui::Base * obj,gui::Event * event);
  void StartDispClean(gui::Base * obj,gui::Event * event);
  void StartScale(gui::Base * obj,gui::Event * event);
  void StartDispCrop(gui::Base * obj,gui::Event * event);
  void StartMeshOps(gui::Base * obj,gui::Event * event);
  void StartCompNeedle(gui::Base * obj,gui::Event * event);
  void StartSfS(gui::Base * obj,gui::Event * event);
  void StartIntegration(gui::Base * obj,gui::Event * event);
  void StartLighting(gui::Base * obj,gui::Event * event);
  void StartSegmentation(gui::Base * obj,gui::Event * event);
  void StartLightEst(gui::Base * obj,gui::Event * event);
  void PairToDefault(gui::Base * obj,gui::Event * event);
  void StartCamResponse(gui::Base * obj,gui::Event * event);
  void StartIntrinsicToCamera(gui::Base * obj,gui::Event * event);
  void StartIntrinsicEst(gui::Base * obj,gui::Event * event);
  void StartAmbientEst(gui::Base * obj,gui::Event * event);
  void StartSphereFitter(gui::Base * obj,gui::Event * event);
};

//------------------------------------------------------------------------------
#endif
