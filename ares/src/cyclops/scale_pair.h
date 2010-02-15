#ifndef CYCLOPS_SCALE_PAIR_H
#define CYCLOPS_SCALE_PAIR_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Scales two images simultaneously under the assumption they are rectified and
// that you want the resulting pair to remain rectified...
class ScalePair
{
 public:
   ScalePair(Cyclops & cyclops);
  ~ScalePair();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * left;
  gui::Canvas * right;

  gui::EditBox * newHeight;
  gui::Label * newDim;

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
  void UpdatePair(gui::Base * obj,gui::Event * event);

  void EditHeight(gui::Base * obj,gui::Event * event);

  void UpdateDim();
};

//------------------------------------------------------------------------------
#endif
