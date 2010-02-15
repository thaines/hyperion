#ifndef CYCLOPS_LIGHT_EST_H
#define CYCLOPS_LIGHT_EST_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Does light source estimation, plus albedo estimation for fun.
class LightEst
{
 public:
   LightEst(Cyclops & cyclops);
  ~LightEst(); 


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  
  gui::ComboBox * viewSelect;
  gui::ComboBox * algSelect;
  gui::EditBox * ambient;
  
  gui::Expander * brutalFish;
  gui::EditBox * bfMinAlb;
  gui::EditBox * bfMaxAlb;
  gui::EditBox * bfMaxSegCost;
  gui::EditBox * bfPruneThresh;
  gui::EditBox * bfIrrErr;
  gui::EditBox * bfSampleSubdiv;
  gui::EditBox * bfFurtherSubdiv;
  gui::EditBox * bfAlbRecursion;
  gui::EditBox * bfIrrThresh;


  gui::Label * lightDir;
  gui::Label * lightDir2;
  bs::Normal lightD;
  
  struct Sample
  {
   real32 cost;
   bs::Normal dir;
  };
  ds::Array<Sample> samples;
  ds::Array<real32> albedo;


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
