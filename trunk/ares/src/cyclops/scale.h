#ifndef CYCLOPS_SCALE_H
#define CYCLOPS_SCALE_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Allows you to scale an image, adjusting the .icd accordingly...
class Scale
{
 public:
   Scale(Cyclops & cyclops);
  ~Scale();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  gui::EditBox * scale;

  svt::Var * imgVar;
  svt::Field<bs::ColRGB> img;

  svt::Var * imageVar;
  svt::Field<bs::ColourRGB> image;


  void Quit(gui::Base * obj,gui::Event * event);
  void Resize(gui::Base * obj,gui::Event * event);

  void LoadImage(gui::Base * obj,gui::Event * event);
  void SaveImage(gui::Base * obj,gui::Event * event);
  void UpdateCalib(gui::Base * obj,gui::Event * event);
};

//------------------------------------------------------------------------------
#endif
