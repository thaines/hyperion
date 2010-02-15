#ifndef CYCLOPS_SFS_H
#define CYCLOPS_SFS_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Runs assorted shape from shading algorithms to produce needle maps.
class SfS
{
 public:
   SfS(Cyclops & cyclops);
  ~SfS();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;


  gui::ComboBox * whichAlg;
  gui::EditBox * albedo;
  gui::EditBox * lightDir;
  
  gui::Expander * alg1;
  gui::EditBox * zacMu;
  gui::EditBox * zacIters;
  gui::EditBox * zacDelta;
   
  gui::Expander * alg2;
  gui::EditBox * lakOuterIters;
  gui::EditBox * lakInnerIters;
  gui::EditBox * lakTolerance;
  gui::EditBox * lakInitRad;
  gui::EditBox * lakStartLambda;
  gui::EditBox * lakEndLambda;
  gui::EditBox * lakSpeed;
  
  gui::Expander * alg3;
  gui::EditBox * wahIters;
  
  gui::Expander * alg4;
  gui::EditBox * hawBlur;
  gui::EditBox * hawLength;
  gui::EditBox * hawExp;
  gui::EditBox * hawStopChance;
  gui::EditBox * hawSimK;
  gui::EditBox * hawConeK;
  gui::EditBox * hawFadeTo;
  gui::EditBox * hawIters;
  gui::EditBox * hawGradDisc;
  gui::EditBox * hawGradBias;
  gui::EditBox * hawBorderK;
  gui::TickBox * hawProject;
  
  gui::Expander * alg5;
  gui::EditBox * haw2SimK;
  gui::EditBox * haw2ConeK;
  gui::EditBox * haw2FadeTo;
  gui::EditBox * haw2Iters;
  gui::EditBox * haw2AngMult;
  gui::EditBox * haw2Momentum;
  gui::EditBox * haw2BoundK;
  gui::EditBox * haw2BoundLength;
  gui::EditBox * haw2BoundExp;
  gui::EditBox * haw2GradK;
  gui::EditBox * haw2GradLength;
  gui::EditBox * haw2GradExp;

  math::Func crf;

  svt::Var * dataVar;
  svt::Field<bs::ColourRGB> image;
  svt::Field<bs::Normal> needle;

  svt::Var * visVar;
  svt::Field<bs::ColRGB> visible;
  bit hasRun;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);

  void LoadImage(gui::Base * obj,gui::Event * event);
  void LoadCalib(gui::Base * obj,gui::Event * event);
  void Run(gui::Base * obj,gui::Event * event);
  void SaveNeedle(gui::Base * obj,gui::Event * event);

  void ChangeAlg(gui::Base * obj,gui::Event * event);

  void UpdateView();
};

//------------------------------------------------------------------------------
#endif