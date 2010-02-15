#ifndef CYCLOPS_DISP_CLEAN_H
#define CYCLOPS_DISP_CLEAN_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Performs operations useful for automatically cleaning a disparity map.
class DispClean
{
 public:
   DispClean(Cyclops & cyclops);
  ~DispClean();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;

  gui::EditBox * srWindow;
  gui::EditBox * srRange;
  gui::EditBox * srTolerance;

  gui::EditBox * smoothSource;
  gui::EditBox * smoothSmooth;
  gui::EditBox * smoothIters;


  svt::Var * var;
  svt::Field<bs::ColRGB> image;

  svt::Var * dispVar;
  svt::Field<real32> disp;
  svt::Field<bit> mask;
  svt::Field<bit> userMask;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);

  void LoadDisp(gui::Base * obj,gui::Event * event);
  void RemoveSpikes(gui::Base * obj,gui::Event * event);
  void Smooth(gui::Base * obj,gui::Event * event);
  void SaveDisp(gui::Base * obj,gui::Event * event);

  void RenderDisp();
};

//------------------------------------------------------------------------------
#endif
