#ifndef CYCLOPS_TO_ORIENT_H
#define CYCLOPS_TO_ORIENT_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Allows you to load a disparity map, either as an image or a .svt, and a .pcc
// file from which it then generates a needle map.
class ToOrient
{
 public:
   ToOrient(Cyclops & cyclops);
  ~ToOrient();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  gui::EditBox * sample;


  cam::CameraPair pair;

  svt::Var * asReal;
  svt::Field<real32> disp;
  svt::Field<bit> dispMask; // true where its good, false where its bad.

  svt::Var * asImg; // Never null, it complains if so.
  svt::Field<bs::ColRGB> image;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);

  void LoadSvt(gui::Base * obj,gui::Event * event);
  void LoadPair(gui::Base * obj,gui::Event * event);

  void SaveNeedle(gui::Base * obj,gui::Event * event);
};

//------------------------------------------------------------------------------
#endif
