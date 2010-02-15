#ifndef CYCLOPS_UNDISTORTER_H
#define CYCLOPS_UNDISTORTER_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Allows you to remove the lens distortion from a photo if you have it
// calibrated...
class Undistorter
{
 public:
   Undistorter(Cyclops & cyclops);
  ~Undistorter();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;

  svt::Var * floatVar;
  svt::Field<bs::ColourRGB> floatImage;

  svt::Var * var;
  svt::Field<bs::ColRGB> image;

  bit ccValid;
  bit imageValid;
  cam::CameraCalibration cc;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);

  void LoadImage(gui::Base * obj,gui::Event * event);
  void LoadIntrinsic(gui::Base * obj,gui::Event * event);
  void Undistort(gui::Base * obj,gui::Event * event);
  void SaveImage(gui::Base * obj,gui::Event * event);
  void SaveIntrinsic(gui::Base * obj,gui::Event * event);


  void RenderLine(const math::Vect<2,real64> & a,const math::Vect<2,real64> & b,const bs::ColourRGB & col);
};

//------------------------------------------------------------------------------
#endif
