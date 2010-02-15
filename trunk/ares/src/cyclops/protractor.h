#ifndef CYCLOPS_PROTRACTOR_H
#define CYCLOPS_PROTRACTOR_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Allows you to open an image and its intrinsic calibration, it then turns the 
// camera into a 2D protractor...
class Protractor
{
 public:
   Protractor(Cyclops & cyclops);
  ~Protractor(); 


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  gui::Label * angle;

  cam::CameraCalibration calib;

  svt::Var * var;
  svt::Field<bs::ColRGB> image;
  
  math::Vect<2,real32> pa;
  math::Vect<2,real32> pb;
  bit nor;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);
  void Click(gui::Base * obj,gui::Event * event);
  void Move(gui::Base * obj,gui::Event * event);

  void LoadImage(gui::Base * obj,gui::Event * event);
  void LoadIntrinsic(gui::Base * obj,gui::Event * event);


  void CalcAngle();
  
  void RenderLine(const math::Vect<2,real32> & a,const math::Vect<2,real32> & b,const bs::ColourRGB & col);
};

//------------------------------------------------------------------------------
#endif
