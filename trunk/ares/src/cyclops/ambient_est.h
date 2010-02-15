#ifndef CYCLOPS_AMBIENT_EST_H
#define CYCLOPS_AMBIENT_EST_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Does ambient estimation given a light source direction, plus albedo estimation for fun.
class AmbientEst
{
 public:
   AmbientEst(Cyclops & cyclops);
  ~AmbientEst(); 


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  
  gui::ComboBox * viewSelect;
  gui::ComboBox * algSelect;
  gui::EditBox * lightDirection;
  
  gui::Expander * brutalFish;
  gui::EditBox * bfMinAmb;
  gui::EditBox * bfMaxAmb;
  gui::EditBox * bfMinAlb;
  gui::EditBox * bfMaxAlb;
  gui::EditBox * bfAmbRecursion;
  gui::EditBox * bfAlbRecursion;
  gui::EditBox * bfPruneThresh;
  gui::EditBox * bfIrrErr;

  gui::Label * results;
  gui::Label * results2;

  ds::Array<real32> albedo;
  real32 ambient;
  
  
  math::Func crf;

  svt::Var * irrVar;
  svt::Field<bs::ColourRGB> irr;
  svt::Var * segVar;
  svt::Field<nat32> seg;
  svt::Var * dispVar;
  svt::Field<math::Fisher> fish;

  svt::Var * imageVar;
  svt::Field<bs::ColourRGB> image;

  svt::Var * imgVar;
  svt::Field<bs::ColRGB> img;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);

  void LoadIrr(gui::Base * obj,gui::Event * event);
  void LoadCRF(gui::Base * obj,gui::Event * event);
  void LoadSeg(gui::Base * obj,gui::Event * event);
  void LoadDisp(gui::Base * obj,gui::Event * event);
  void Run(gui::Base * obj,gui::Event * event);
  void ChangeView(gui::Base * obj,gui::Event * event);
  void ChangeAlg(gui::Base * obj,gui::Event * event);
  void SaveView(gui::Base * obj,gui::Event * event);

  void Update();
};

//------------------------------------------------------------------------------
#endif
