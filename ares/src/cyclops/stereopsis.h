#ifndef CYCLOPS_STEREOPSIS_H
#define CYCLOPS_STEREOPSIS_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Allows a user to apply various stereo algorithms to an image pair.
class Stereopsis
{
 public:
   Stereopsis(Cyclops & cyc);
  ~Stereopsis();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * left;
  gui::Canvas * right;


  gui::ComboBox * whichAlg;
  gui::ComboBox * whichPost;
  gui::TickBox * augGaussian;
  gui::TickBox * augFisher;

  gui::Expander * alg5;
  gui::Expander * alg6;
  gui::Expander * post2;
  gui::Expander * post3a;
  gui::Expander * post3b;
  gui::Expander * augG;
  gui::Expander * augF;

  gui::EditBox * occCost;
  gui::EditBox * vertCost;
  gui::EditBox * vertMult;
  gui::EditBox * errLim;
  gui::EditBox * matchLim;
  
  gui::EditBox * bpOccCostHigh;
  gui::EditBox * bpOccCostLow;
  gui::EditBox * bpOccLimMult;
  gui::EditBox * bpMatchLim;
  gui::EditBox * bpOccLim;
  gui::EditBox * bpIters;
  gui::EditBox * bpOutput;

  gui::EditBox * smoothStrength;
  gui::EditBox * smoothCutoff;
  gui::EditBox * smoothWidth;
  gui::EditBox * smoothIters;
  
  gui::EditBox * planeRadius;
  gui::EditBox * planeBailOut;
  gui::EditBox * planeOcc;
  gui::EditBox * planeDisc;

  gui::EditBox * segSpatial;
  gui::EditBox * segRange;
  gui::EditBox * segMin;
  gui::EditBox * segRad;
  gui::EditBox * segMix;
  gui::EditBox * segEdge;
  
  gui::EditBox * gaussianRange;
  gui::EditBox * fisherRange;
  gui::EditBox * fisherMin;
  gui::EditBox * fisherMax;
  gui::EditBox * fisherBias;
  cam::CameraPair pair;


  svt::Var * leftVar;
  svt::Field<bs::ColRGB> leftImage;
  svt::Var * rightVar;
  svt::Field<bs::ColRGB> rightImage;

  svt::Var * leftImg;
  svt::Var * rightImg;
  svt::Var * result;

  void Quit(gui::Base * obj,gui::Event * event);

  void ResizeLeft(gui::Base * obj,gui::Event * event);
  void ResizeRight(gui::Base * obj,gui::Event * event);

  void LoadLeft(gui::Base * obj,gui::Event * event);
  void LoadRight(gui::Base * obj,gui::Event * event);

  void LoadCalibration(gui::Base * obj,gui::Event * event);
  
  void ChangeAlg(gui::Base * obj,gui::Event * event);
  void ChangePost(gui::Base * obj,gui::Event * event);
  void SwitchGaussian(gui::Base * obj,gui::Event * event);
  void SwitchFisher(gui::Base * obj,gui::Event * event);

  void Run(gui::Base * obj,gui::Event * event);

  void SaveSVT(gui::Base * obj,gui::Event * event);
};

//------------------------------------------------------------------------------
#endif
