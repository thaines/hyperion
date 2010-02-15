#ifndef CYCLOPS_RECTIFICATION_H
#define CYCLOPS_RECTIFICATION_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Allows a user to rectify two images.
class Rectification
{
 public:
   Rectification(Cyclops & cyc);
  ~Rectification();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * left;
  gui::Canvas * right;

  svt::Var * leftVar;
  svt::Field<bs::ColRGB> leftImage;
  svt::Var * rightVar;
  svt::Field<bs::ColRGB> rightImage;

  cam::CameraPair pair;

  svt::Var * leftResult;
  svt::Var * rightResult;
  
  
  void Quit(gui::Base * obj,gui::Event * event);

  void ResizeLeft(gui::Base * obj,gui::Event * event);
  void ResizeRight(gui::Base * obj,gui::Event * event);
  
  void LoadLeft(gui::Base * obj,gui::Event * event);
  void LoadRight(gui::Base * obj,gui::Event * event);  
  void LoadConfig(gui::Base * obj,gui::Event * event);
  
  void Rectify(gui::Base * obj,gui::Event * event);

  void SaveLeft(gui::Base * obj,gui::Event * event);
  void SaveRight(gui::Base * obj,gui::Event * event);
  void SaveConfig(gui::Base * obj,gui::Event * event);
};

//------------------------------------------------------------------------------
#endif
