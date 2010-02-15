#ifndef CYCLOPS_COMP_NEEDLE_H
#define CYCLOPS_COMP_NEEDLE_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Allows you to compare needle maps.
class CompNeedle
{
 public:
   CompNeedle(Cyclops & cyclops);
  ~CompNeedle(); 


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
  svt::Field<bs::ColourRGB> truthImage;
  
  svt::Var * guessVar;
  svt::Field<bs::ColourRGB> guessImage;

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
