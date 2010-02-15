#ifndef CYCLOPS_COMP_DISP_H
#define CYCLOPS_COMP_DISP_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Allows you to compare disparity maps.
class CompDisp
{
 public:
   CompDisp(Cyclops & cyclops);
  ~CompDisp(); 


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  
  gui::EditBox * cap;
  real32 capVal;
  gui::Label * resLab;
  gui::Label * resLab2;
  gui::Label * resLab3;


  svt::Var * truthVar;
  svt::Field<real32> truthDisp;
  svt::Field<bit> truthMask;

  svt::Var * guessVar;
  svt::Field<real32> guessDisp;
  svt::Field<bit> guessMask;

  svt::Var * imageVar;
  svt::Field<bs::ColRGB> image;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);

  void LoadTruth(gui::Base * obj,gui::Event * event);
  void LoadGuess(gui::Base * obj,gui::Event * event);
  void OnCapChange(gui::Base * obj,gui::Event * event);
  
  void Update();
};

//------------------------------------------------------------------------------
#endif
