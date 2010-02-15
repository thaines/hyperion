#ifndef CYCLOPS_ANAGLYPH_H
#define CYCLOPS_ANAGLYPH_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Creates Anaglyphs, not exactly complicated.
class Anaglyph
{
 public:
   Anaglyph(Cyclops & cyclops);
  ~Anaglyph();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  gui::EditBox * offset;


  svt::Var * leftVar;
  svt::Field<bs::ColourRGB> leftImage;

  svt::Var * rightVar;
  svt::Field<bs::ColourRGB> rightImage;

  svt::Var * var;
  svt::Field<bs::ColRGB> image;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);

  void LoadLeft(gui::Base * obj,gui::Event * event);
  void LoadRight(gui::Base * obj,gui::Event * event);
  void OffsetChange(gui::Base * obj,gui::Event * event);
  void SaveAnaglyph(gui::Base * obj,gui::Event * event);

  void Update();
};

//------------------------------------------------------------------------------
#endif
