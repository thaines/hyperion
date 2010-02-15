#ifndef CYCLOPS_CALIBRATION_H
#define CYCLOPS_CALIBRATION_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Allows you to calculate a camera projection matrix and radial distortion
// parameters from a single picture of a 3d target...
class Calibration
{
 public:
   Calibration(Cyclops & cyclops);
  ~Calibration(); 


 private:
  Cyclops & cyclops;


  gui::Window * win;
  gui::Canvas * canvas;
  gui::ComboBox * quality;
  gui::EditBox * worldX;
  gui::EditBox * worldY;
  gui::EditBox * worldZ;
  gui::Label * detail;

  
  bit resValid;
  cam::Camera camera;
  cam::Radial radial;

  svt::Var * var;
  svt::Field<bs::ColRGB> image;
  
  
  bs::Pnt pos;
  struct Match
  {
   bs::Vert world;
   bs::Pnt image;
  };
  ds::List<Match> data;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);
  void Click(gui::Base * obj,gui::Event * event);
  void Move(gui::Base * obj,gui::Event * event);

  void LoadImage(gui::Base * obj,gui::Event * event);
  void Add(gui::Base * obj,gui::Event * event);
  void Rem(gui::Base * obj,gui::Event * event);
  void Calculate(gui::Base * obj,gui::Event * event);
  void SaveCamera(gui::Base * obj,gui::Event * event);
  void SaveIntrinsic(gui::Base * obj,gui::Event * event);
  void SaveState(gui::Base * obj,gui::Event * event);
  void AppendState(gui::Base * obj,gui::Event * event);
  void ClearState(gui::Base * obj,gui::Event * event);

  
  void RenderPoint(const bs::Pnt & p,const bs::ColourRGB & col);
};

//------------------------------------------------------------------------------
#endif
