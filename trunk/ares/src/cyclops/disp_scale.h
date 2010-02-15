#ifndef CYCLOPS_DISP_SCALE_H
#define CYCLOPS_DISP_SCALE_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Allows you to scale a disparity map, so less geometry is produced latter.
class DispScale
{
 public:
   DispScale(Cyclops & cyclops);
  ~DispScale();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  gui::EditBox * scale;

  svt::Var * var;
  svt::Field<bs::ColRGB> image;

  svt::Var * dispVar;
  svt::Field<real32> disp;
  svt::Field<bit> mask;


  void Quit(gui::Base * obj,gui::Event * event);
  void Resize(gui::Base * obj,gui::Event * event);

  void LoadDisp(gui::Base * obj,gui::Event * event);
  void SaveDisp(gui::Base * obj,gui::Event * event);
  void UpdatePair(gui::Base * obj,gui::Event * event);

  void RenderDisp();
};

//------------------------------------------------------------------------------
#endif
