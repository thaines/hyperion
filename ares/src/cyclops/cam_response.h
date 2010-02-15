#ifndef CYCLOPS_CAM_RESPONSE_H
#define CYCLOPS_CAM_RESPONSE_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Calculates a camera response function given a set of images of a white 
// object where only the exposure time varies.
class CamResponse
{
 public:
   CamResponse(Cyclops & cyclops);
  ~CamResponse();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  
  gui::ComboBox * viewSelect;
  
  bit expRec;
  gui::EditBox * exposure;
  gui::EditBox * exposureInv;
  gui::EditBox * degrees;

  gui::Label * info;
  gui::Label * result;


  math::Vect<2,real32> pMin;
  math::Vect<2,real32> pMax;
  
  struct Sample
  {
   bs::ColourRGB colour;
   real32 expTime;
  };
  ds::ArrayResize<Sample> samples;


  svt::Var * irrVar;
  svt::Field<bs::ColourRGB> irr;

  math::Func crf;
  real32 expNorm; // Normalising multiplier for exposure, negative to indicate uncalculated.


  svt::Var * imageVar;
  svt::Field<bs::ColourRGB> image;

  svt::Var * imgVar;
  svt::Field<bs::ColRGB> img;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);
  void Click(gui::Base * obj,gui::Event * event);
  void Move(gui::Base * obj,gui::Event * event);
  
  void LoadIrr(gui::Base * obj,gui::Event * event);
  void Add(gui::Base * obj,gui::Event * event);
  void Run(gui::Base * obj,gui::Event * event);
  void ChangeView(gui::Base * obj,gui::Event * event);
  void SaveCRF(gui::Base * obj,gui::Event * event);
  
  void ExpChange(gui::Base * obj,gui::Event * event);
  void InvExpChange(gui::Base * obj,gui::Event * event);

  void Update();
  
  void RenderLine(const math::Vect<2,real32> & a,const math::Vect<2,real32> & b,const bs::ColourRGB & col);
};

//------------------------------------------------------------------------------
#endif
