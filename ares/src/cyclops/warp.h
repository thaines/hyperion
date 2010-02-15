#ifndef CYCLOPS_WARP_H
#define CYCLOPS_WARP_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Allows you to forward warp an image, to verify a disparity map.
class Warp
{
 public:
   Warp(Cyclops & cyclops);
  ~Warp(); 


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  gui::EditBox * amount;


  svt::Var * imageVar;
  svt::Field<bs::ColourRGB> image;

  svt::Var * disparityVar;
  svt::Field<real32> disparity;
  svt::Field<bit> mask;

  svt::Var * viewVar;
  svt::Field<bs::ColRGB> view;


  void Quit(gui::Base * obj,gui::Event * event);
  void Resize(gui::Base * obj,gui::Event * event);

  void LoadImage(gui::Base * obj,gui::Event * event);
  void LoadObf(gui::Base * obj,gui::Event * event);
  void SaveImage(gui::Base * obj,gui::Event * event);
  void AmountChange(gui::Base * obj,gui::Event * event);
  
  void Update(); // Called to re-create view using the relvant inputs.
};

//------------------------------------------------------------------------------
#endif
