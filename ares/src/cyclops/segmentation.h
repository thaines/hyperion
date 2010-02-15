#ifndef CYCLOPS_SEGMENTATION_H
#define CYCLOPS_SEGMENTATION_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Allows you to segment images.
class Segmentation
{
 public:
   Segmentation(Cyclops & cyclops);
  ~Segmentation(); 


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  
  gui::ComboBox * viewSelect;
  gui::ComboBox * algSelect;
  
  gui::Expander * kMeanGrid;
  gui::EditBox * kmgDim;
  gui::EditBox * kmgMinSeg;
  gui::EditBox * kmgColMult;
  gui::EditBox * kmgSpatialMult;
  gui::EditBox * kmgMaxIters;
  
  gui::Expander * meanShift;
  gui::EditBox * msEdgeWindow;
  gui::EditBox * msEdgeMix;
  gui::EditBox * msSpatialSize;
  gui::EditBox * msColourSize;
  gui::EditBox * msMergeCutoff;
  gui::EditBox * msMinSeg;
  gui::EditBox * msAvgSteps;
  gui::EditBox * msMergeMax;
  
  
  svt::Var * inputVar;
  svt::Field<bs::ColourRGB> inputImage;
  svt::Field<nat32> segmentation;


  svt::Var * imageVar;
  svt::Field<bs::ColourRGB> image;

  svt::Var * imgVar;
  svt::Field<bs::ColRGB> img;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);

  void LoadImage(gui::Base * obj,gui::Event * event);
  void Run(gui::Base * obj,gui::Event * event);
  void ChangeView(gui::Base * obj,gui::Event * event);
  void ChangeAlg(gui::Base * obj,gui::Event * event);
  void SaveView(gui::Base * obj,gui::Event * event);
  void SaveSeg(gui::Base * obj,gui::Event * event);
  
  void Update();
};

//------------------------------------------------------------------------------
#endif
