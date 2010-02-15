#ifndef CYCLOPS_CROP_PAIR_H
#define CYCLOPS_CROP_PAIR_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Crops two images simultaneously under the assumption they are rectified and
// that you want the resulting image to be rectified...
class CropPair
{
 public:
   CropPair(Cyclops & cyclops);
  ~CropPair();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * left;
  gui::Canvas * right;

  svt::Var * leftVar;
  svt::Field<bs::ColRGB> leftImage;
  svt::Var * rightVar;
  svt::Field<bs::ColRGB> rightImage;


  math::Vect<2,real64> lMin;
  math::Vect<2,real64> lMax;

  math::Vect<2,real64> rMin;
  math::Vect<2,real64> rMax;


  void Quit(gui::Base * obj,gui::Event * event);

  void ResizeLeft(gui::Base * obj,gui::Event * event);
  void ClickLeft(gui::Base * obj,gui::Event * event);
  void MoveLeft(gui::Base * obj,gui::Event * event);

  void ResizeRight(gui::Base * obj,gui::Event * event);
  void ClickRight(gui::Base * obj,gui::Event * event);
  void MoveRight(gui::Base * obj,gui::Event * event);

  void LoadLeft(gui::Base * obj,gui::Event * event);
  void LoadRight(gui::Base * obj,gui::Event * event);
  void SaveLeft(gui::Base * obj,gui::Event * event);
  void SaveRight(gui::Base * obj,gui::Event * event);
  void UpdatePair(gui::Base * obj,gui::Event * event);

  void RenderLeftLine(const math::Vect<2,real64> & a,const math::Vect<2,real64> & b,const bs::ColourRGB & col);
  void RenderRightLine(const math::Vect<2,real64> & a,const math::Vect<2,real64> & b,const bs::ColourRGB & col);
};

//------------------------------------------------------------------------------
#endif
