#ifndef CYCLOPS_LIGHTING_H
#define CYCLOPS_LIGHTING_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Allows you to relight images, and calculate albedos.
class Lighting
{
 public:
   Lighting(Cyclops & cyclops);
  ~Lighting(); 


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  
  gui::ComboBox * viewSelect;
  gui::EditBox * origLight;
  gui::EditBox * newLight;
  gui::Label * albLab;
  
  math::Func crf;
  
  svt::Var * needleVar;
  svt::Field<bs::ColourRGB> needleImage;
  
  svt::Var * origVar;
  svt::Field<bs::ColourRGB> origImage;


  svt::Var * imageVar;
  svt::Field<bs::ColourRGB> image;

  svt::Var * imgVar;
  svt::Field<bs::ColRGB> img;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);

  void LoadNeedle(gui::Base * obj,gui::Event * event);
  void LoadOrig(gui::Base * obj,gui::Event * event);
  void LoadCRF(gui::Base * obj,gui::Event * event);
  void Render(gui::Base * obj,gui::Event * event);
  void SaveRender(gui::Base * obj,gui::Event * event);
  
  void Update();
};

//------------------------------------------------------------------------------
#endif
