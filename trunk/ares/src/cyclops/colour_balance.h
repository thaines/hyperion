#ifndef CYCLOPS_COLOUR_BALANCE_H
#define CYCLOPS_COLOUR_BALANCE_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Adjusts two images so they have similar colours, good for pre-processing
// stereo paris...
class ColourBalance
{
 public:
   ColourBalance(Cyclops & cyclops);
  ~ColourBalance();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * left;
  gui::Canvas * right;


  svt::Var * leftImgVar;
  svt::Field<bs::ColourRGB> leftImg;
  svt::Var * rightImgVar;
  svt::Field<bs::ColourRGB> rightImg;

  svt::Var * leftVar;
  svt::Field<bs::ColRGB> leftImage;
  svt::Var * rightVar;
  svt::Field<bs::ColRGB> rightImage;


  void Quit(gui::Base * obj,gui::Event * event);
  void ResizeLeft(gui::Base * obj,gui::Event * event);
  void ResizeRight(gui::Base * obj,gui::Event * event);

  void LoadLeft(gui::Base * obj,gui::Event * event);
  void LoadRight(gui::Base * obj,gui::Event * event);
  void SaveLeft(gui::Base * obj,gui::Event * event);
  void SaveRight(gui::Base * obj,gui::Event * event);
  void Match(gui::Base * obj,gui::Event * event);
};

//------------------------------------------------------------------------------
#endif
