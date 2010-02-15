#ifndef CYCLOPS_TO_MESH_H
#define CYCLOPS_TO_MESH_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Allows you to load a disparity map, either as an image or a .svt, and a .pcc
// file from which it then generates a wavefront mesh, with correct UV's.
class ToMesh
{
 public:
   ToMesh(Cyclops & cyclops);
  ~ToMesh();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  gui::EditBox * conLimit;
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

  void Blur(gui::Base * obj,gui::Event * event);
  void Quant(gui::Base * obj,gui::Event * event);

  void SaveModel(gui::Base * obj,gui::Event * event);
};

//------------------------------------------------------------------------------
#endif
