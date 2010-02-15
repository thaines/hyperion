#ifndef CYCLOPS_DISP_CROP_H
#define CYCLOPS_DISP_CROP_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Crops a disparity map...
class DispCrop
{
 public:
   DispCrop(Cyclops & cyclops);
  ~DispCrop();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;

  svt::Var * imgVar;
  svt::Field<bs::ColRGB> img;

  svt::Var * dispVar;
  svt::Field<real32> disp;
  svt::Field<bit> mask;


  math::Vect<2,real32> pMin;
  math::Vect<2,real32> pMax;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);
  void Click(gui::Base * obj,gui::Event * event);
  void Move(gui::Base * obj,gui::Event * event);

  void LoadDisp(gui::Base * obj,gui::Event * event);
  void SaveDisp(gui::Base * obj,gui::Event * event);
  void UpdatePair(gui::Base * obj,gui::Event * event);

  
  void RenderLine(const math::Vect<2,real32> & a,const math::Vect<2,real32> & b,const bs::ColourRGB & col);
  void RenderDisp();
};

//------------------------------------------------------------------------------
#endif
