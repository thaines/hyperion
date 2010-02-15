#ifndef CYCLOPS_COLOUR_BALANCE_H
#define CYCLOPS_COLOUR_BALANCE_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

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
// Adjusts two images so they have similar colours, good for pre-processing
// stereo paris...
class ColourBalance
{
 public:
   ColourBalance(Cyclops & cyclops);
  ~ColourBalance();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * left;
  gui::Canvas * right;


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
  void Match(gui::Base * obj,gui::Event * event);
};

//------------------------------------------------------------------------------
#endif
