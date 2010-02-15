#ifndef CYCLOPS_MODEL_DISP_H
#define CYCLOPS_MODEL_DISP_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Allows you to render 3d objects to create a disparity map.
class ModelDisp
{
 public:
   ModelDisp(Cyclops & cyclops);
  ~ModelDisp(); 


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;

  cam::CameraPair pair;
  cam::MakeDisp makeDisp;

  svt::Var * var;
  svt::Field<bs::ColRGB> image;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);

  void LoadPair(gui::Base * obj,gui::Event * event);
  void AddModel(gui::Base * obj,gui::Event * event);
  void SaveDisp(gui::Base * obj,gui::Event * event);


  void RenderDisp();
};

//------------------------------------------------------------------------------
#endif
