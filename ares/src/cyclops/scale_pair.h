#ifndef CYCLOPS_SCALE_PAIR_H
#define CYCLOPS_SCALE_PAIR_H
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
// Scales two images simultaneously under the assumption they are rectified and
// that you want the resulting pair to remain rectified...
class ScalePair
{
 public:
   ScalePair(Cyclops & cyclops);
  ~ScalePair();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * left;
  gui::Canvas * right;

  gui::EditBox * newHeight;
  gui::Label * newDim;

  svt::Var * leftImgVar;
  svt::Field<bs::ColourRGB> leftImg;
  svt::Var * rightImgVar;
  svt::Field<bs::ColourRGB> rightImg;

  svt::Var * leftVar;
  svt::Field<bs::ColRGB> leftImage;
  svt::Var * rightVar;
  svt::Field<bs::ColRGB> rightImage;

  void Quit(gui::Base * obj,gui::Event * event);
  void ResizeLeft(gui::Base * obj,gui::Event * event);
  void ResizeRight(gui::Base * obj,gui::Event * event);

  void LoadLeft(gui::Base * obj,gui::Event * event);
  void LoadRight(gui::Base * obj,gui::Event * event);
  void SaveLeft(gui::Base * obj,gui::Event * event);
  void SaveRight(gui::Base * obj,gui::Event * event);
  void UpdatePair(gui::Base * obj,gui::Event * event);

  void EditHeight(gui::Base * obj,gui::Event * event);

  void UpdateDim();
};

//------------------------------------------------------------------------------
#endif
