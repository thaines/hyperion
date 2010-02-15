#ifndef CYCLOPS_CROP_H
#define CYCLOPS_CROP_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Allows you to open an image and its intrinsic calibration, it then turns the 
// camera into a 2D protractor...
class Crop
{
 public:
   Crop(Cyclops & cyclops);
  ~Crop();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;

  svt::Var * imgVar;
  svt::Field<bs::ColRGB> img;

  svt::Var * imageVar;
  svt::Field<bs::ColourRGB> image;


  math::Vect<2,real32> pMin;
  math::Vect<2,real32> pMax;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);
  void Click(gui::Base * obj,gui::Event * event);
  void Move(gui::Base * obj,gui::Event * event);

  void LoadImage(gui::Base * obj,gui::Event * event);

  void SaveImage(gui::Base * obj,gui::Event * event);
  void UpdateIntrinsic(gui::Base * obj,gui::Event * event);

  
  void RenderLine(const math::Vect<2,real32> & a,const math::Vect<2,real32> & b,const bs::ColourRGB & col);
};

//------------------------------------------------------------------------------
#endif
