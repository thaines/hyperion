#ifndef CYCLOPS_INTEGRATION_H
#define CYCLOPS_INTEGRATION_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Integrates needle maps to get 3D models.
class Integration
{
 public:
   Integration(Cyclops & cyclops);
  ~Integration();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;

  gui::EditBox * scale;
  gui::EditBox * zMult;
  gui::EditBox * limit;


  svt::Var * dataVar;
  svt::Field<bs::Normal> needle;

  svt::Var * visVar;
  svt::Field<bs::ColRGB> visible;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);

  void LoadNeedle(gui::Base * obj,gui::Event * event);
  void SaveMesh(gui::Base * obj,gui::Event * event);

  void UpdateView();
};

//------------------------------------------------------------------------------
#endif
