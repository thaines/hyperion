#ifndef CYCLOPS_TRIANGULATOR_H
#define CYCLOPS_TRIANGULATOR_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Little toy, allows the user to select any two points, it then assumes they
// are the same point and triangulates there location, giving camera depth as
// well as 3D coordinates in some arbitary coordinate system with some arbitary
// scale.
class Triangulator
{
 public:
   Triangulator(Cyclops & cyclops);
  ~Triangulator();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Label * inst1;
  gui::Label * inst2;
  gui::Canvas * left;
  gui::Canvas * right;

  svt::Var * leftVar;
  svt::Field<bs::ColRGB> leftImage;
  svt::Var * rightVar;
  svt::Field<bs::ColRGB> rightImage;

  cam::CameraPair pair;
  ds::Delaunay2D< math::Vect<2,real64> > tri;

  math::Vect<2,real64> leftP;
  math::Vect<2,real64> rightP;


  struct Stored
  {
   math::Vect<2,real64> leftP;
   math::Vect<2,real64> rightP;
   math::Vect<3,real64> pos;
  };
  ds::List<Stored> store;
  Stored * selected;



  void Quit(gui::Base * obj,gui::Event * event);

  void ResizeLeft(gui::Base * obj,gui::Event * event);
  void ClickLeft(gui::Base * obj,gui::Event * event);
  void MoveLeft(gui::Base * obj,gui::Event * event);

  void ResizeRight(gui::Base * obj,gui::Event * event);
  void ClickRight(gui::Base * obj,gui::Event * event);
  void MoveRight(gui::Base * obj,gui::Event * event);

  void LoadLeft(gui::Base * obj,gui::Event * event);
  void LoadRight(gui::Base * obj,gui::Event * event);
  void LoadPair(gui::Base * obj,gui::Event * event);

  void Add(gui::Base * obj,gui::Event * event);
  void Delete(gui::Base * obj,gui::Event * event);
  void SaveModel(gui::Base * obj,gui::Event * event);
  void ReloadModel(gui::Base * obj,gui::Event * event);


  void RenderLeftPoint(const math::Vect<2,real64> & p,const bs::ColourRGB & col);
  void RenderRightPoint(const math::Vect<2,real64> & p,const bs::ColourRGB & col);

  void RenderLeftLine(const math::Vect<2,real64> & a,const math::Vect<2,real64> & b,const bs::ColourRGB & col);
  void RenderRightLine(const math::Vect<2,real64> & a,const math::Vect<2,real64> & b,const bs::ColourRGB & col);


  void CalcPos();
};

//------------------------------------------------------------------------------
#endif
