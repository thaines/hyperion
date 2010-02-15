#ifndef CYCLOPS_FUNDAMENTAL_H
#define CYCLOPS_FUNDAMENTAL_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Calculates the fundamental matrix, displays two images and a control bar,
// allows the addition of reliable matches, the automatic addition of computer
// calculated matches and the editting and deletion of matches. (Editted once
// become reliable.)
// Once enough matches are found the fundamental matrix is calculated.
class Fundamental
{
 public:
   Fundamental(Cyclops & cyclops);
  ~Fundamental();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Label * inst;
  gui::Canvas * left;
  gui::Canvas * right;
  gui::Multiline * ml;
  gui::EditBox * gap;
  gui::ComboBox * autoAlg;

  svt::Var * leftVar;
  svt::Field<bs::ColRGB> leftImage;
  svt::Var * rightVar;
  svt::Field<bs::ColRGB> rightImage;

  struct Match
  {
   math::Vect<2,real64> leftP;
   math::Vect<2,real64> rightP;
   bit reliable;
  };
  ds::List<Match> matches;

  enum {Edit,AddFirst,AddLeft,AddRight} mode;
  Match * selected;

  cam::CameraPair pair;


  void Quit(gui::Base * obj,gui::Event * event);

  void ResizeLeft(gui::Base * obj,gui::Event * event);
  void ClickLeft(gui::Base * obj,gui::Event * event);
  void MoveLeft(gui::Base * obj,gui::Event * event);

  void ResizeRight(gui::Base * obj,gui::Event * event);
  void ClickRight(gui::Base * obj,gui::Event * event);
  void MoveRight(gui::Base * obj,gui::Event * event);

  void LoadLeft(gui::Base * obj,gui::Event * event);
  void LoadRight(gui::Base * obj,gui::Event * event);
  void LoadLeftC(gui::Base * obj,gui::Event * event);
  void LoadRightC(gui::Base * obj,gui::Event * event);

  void AddMatch(gui::Base * obj,gui::Event * event);
  void AutoMatch(gui::Base * obj,gui::Event * event);
  void DelMatch(gui::Base * obj,gui::Event * event);
  void Calculate(gui::Base * obj,gui::Event * event);
  void TrustNoMatch(gui::Base * obj,gui::Event * event);
  void AlreadyRectified(gui::Base * obj,gui::Event * event);

  void SavePair(gui::Base * obj,gui::Event * event);


  void RenderLeftPoint(const math::Vect<2,real64> & p,const bs::ColourRGB & col,bit special);
  void RenderRightPoint(const math::Vect<2,real64> & p,const bs::ColourRGB & col,bit special);
};

//------------------------------------------------------------------------------
#endif
