#ifndef CYCLOPS_DISP_SCALE_H
#define CYCLOPS_DISP_SCALE_H
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
// Allows you to scale a disparity map, so less geometry is produced latter.
class DispScale
{
 public:
   DispScale(Cyclops & cyclops);
  ~DispScale();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  gui::EditBox * scale;

  svt::Var * var;
  svt::Field<bs::ColRGB> image;

  svt::Var * dispVar;
  svt::Field<real32> disp;
  svt::Field<bit> mask;


  void Quit(gui::Base * obj,gui::Event * event);
  void Resize(gui::Base * obj,gui::Event * event);

  void LoadDisp(gui::Base * obj,gui::Event * event);
  void SaveDisp(gui::Base * obj,gui::Event * event);
  void UpdatePair(gui::Base * obj,gui::Event * event);

  void RenderDisp();
};

//------------------------------------------------------------------------------
#endif
