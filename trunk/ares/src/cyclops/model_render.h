#ifndef CYCLOPS_MODEL_RENDER_H
#define CYCLOPS_MODEL_RENDER_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Allows you to render 3d objects to create normal maps.
class ModelRender
{
 public:
   ModelRender(Cyclops & cyclops);
  ~ModelRender(); 


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;

  cam::CameraFull camera;
  ds::Array2D<bs::Ray> camRays; // Optimisation data structure.
  
  svt::Var * floatVar;
  svt::Field<bs::ColourRGB> floatImage;
  ds::Array2D<real32> depth; // Depth map, used for rendering.

  svt::Var * var;
  svt::Field<bs::ColRGB> image;


  void Quit(gui::Base * obj,gui::Event * event);
  void Resize(gui::Base * obj,gui::Event * event);

  void LoadCam(gui::Base * obj,gui::Event * event);
  void AddModelNeedle(gui::Base * obj,gui::Event * event);
  void SaveImage(gui::Base * obj,gui::Event * event);

  void UpdateImage();
};

//------------------------------------------------------------------------------
#endif
