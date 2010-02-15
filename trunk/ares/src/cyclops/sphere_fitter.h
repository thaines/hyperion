#ifndef CYCLOPS_SPHERE_FITTER_H
#define CYCLOPS_SPHERE_FITTER_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Fits sphere to single images given there occluding boundary and size plus
// camera calibration...
class SphereFitter
{
 public:
   SphereFitter(Cyclops & cyclops);
  ~SphereFitter(); 


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  gui::EditBox * fitRad;
  gui::Label * sphereLab;
  gui::Label * pointLab;
  gui::Label * pointLab2;

  cam::CameraFull camera;
  bs::Vert camCentre;

  bit valid;
  bs::Vert centre;
  real32 radius;


  svt::Var * var;
  svt::Field<bs::ColRGB> image;
  
  bs::Pnt pos; // Selected coordinate for adding/ giving info about.
  ds::List<bs::Pnt> boundary; // Stored in rendering coordinates.


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);
  void Click(gui::Base * obj,gui::Event * event);
  void Move(gui::Base * obj,gui::Event * event);

  void LoadImage(gui::Base * obj,gui::Event * event);
  void LoadCam(gui::Base * obj,gui::Event * event);
  void Add(gui::Base * obj,gui::Event * event);
  void Del(gui::Base * obj,gui::Event * event);  
  void Fit(gui::Base * obj,gui::Event * event);


  void Update(); // Updates pointLab based on pos...
  void RenderPoint(const bs::Pnt & p,const bs::ColourRGB & col);
  void RenderLine(const bs::Pnt & a,const bs::Pnt & b,const bs::ColourRGB & col);
};

//------------------------------------------------------------------------------
#endif
