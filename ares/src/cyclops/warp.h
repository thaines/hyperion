#ifndef CYCLOPS_WARP_H
#define CYCLOPS_WARP_H
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
// Allows you to forward warp an image, to verify a disparity map.
class Warp
{
 public:
   Warp(Cyclops & cyclops);
  ~Warp(); 


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  gui::EditBox * amount;


  svt::Var * imageVar;
  svt::Field<bs::ColourRGB> image;

  svt::Var * disparityVar;
  svt::Field<real32> disparity;
  svt::Field<bit> mask;

  svt::Var * viewVar;
  svt::Field<bs::ColRGB> view;


  void Quit(gui::Base * obj,gui::Event * event);
  void Resize(gui::Base * obj,gui::Event * event);

  void LoadImage(gui::Base * obj,gui::Event * event);
  void LoadObf(gui::Base * obj,gui::Event * event);
  void SaveImage(gui::Base * obj,gui::Event * event);
  void AmountChange(gui::Base * obj,gui::Event * event);
  
  void Update(); // Called to re-create view using the relvant inputs.
};

//------------------------------------------------------------------------------
#endif
