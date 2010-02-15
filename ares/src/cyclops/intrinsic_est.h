#ifndef CYCLOPS_INTRINSIC_EST_H
#define CYCLOPS_INTRINSIC_EST_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Given basic parameters for a camera produces an intrinsic matrix - obviously
// an estimate that will be wrong, but ok in certain situations.
class IntrinsicEst
{
 public:
   IntrinsicEst(Cyclops & cyclops);
  ~IntrinsicEst();


 private:
  Cyclops & cyclops;

  gui::Window * win;

  gui::EditBox * sizeX;  
  gui::EditBox * sizeY;
  
  bit xChange;
  gui::EditBox * mmX;
  gui::EditBox * angX;

  bit yChange;
  gui::EditBox * mmY;
  gui::EditBox * angY;


  void Quit(gui::Base * obj,gui::Event * event);

  void SaveICD(gui::Base * obj,gui::Event * event);

  void mmXChange(gui::Base * obj,gui::Event * event);
  void angXChange(gui::Base * obj,gui::Event * event);
  
  void mmYChange(gui::Base * obj,gui::Event * event);
  void angYChange(gui::Base * obj,gui::Event * event);

  void fromX(gui::Base * obj,gui::Event * event);
  void fromY(gui::Base * obj,gui::Event * event);
};

//------------------------------------------------------------------------------
#endif
