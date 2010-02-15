#ifndef CYCLOPS_DISP_MASK_H
#define CYCLOPS_DISP_MASK_H
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
// Allows you to mask a disparity map, i.e. remove areas you don't want.
class DispMask
{
 public:
   DispMask(Cyclops & cyclops);
  ~DispMask();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;

  svt::Var * var;
  svt::Field<bs::ColRGB> image;

  svt::Var * dispVar;
  svt::Field<real32> disp;
  svt::Field<bit> mask;
  svt::Field<bit> userMask;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);

  void LoadDisp(gui::Base * obj,gui::Event * event);
  void SaveMask(gui::Base * obj,gui::Event * event);
  void LoadMask(gui::Base * obj,gui::Event * event);
  void SaveDisp(gui::Base * obj,gui::Event * event);

  void RenderDisp();
};

//------------------------------------------------------------------------------
#endif
