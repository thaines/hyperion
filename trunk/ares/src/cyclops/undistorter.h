#ifndef CYCLOPS_UNDISTORTER_H
#define CYCLOPS_UNDISTORTER_H
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
// Allows you to remove the lens distortion from a photo if you have it
// calibrated...
class Undistorter
{
 public:
   Undistorter(Cyclops & cyclops);
  ~Undistorter();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;

  svt::Var * floatVar;
  svt::Field<bs::ColourRGB> floatImage;

  svt::Var * var;
  svt::Field<bs::ColRGB> image;

  bit ccValid;
  bit imageValid;
  cam::CameraCalibration cc;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);

  void LoadImage(gui::Base * obj,gui::Event * event);
  void LoadIntrinsic(gui::Base * obj,gui::Event * event);
  void Undistort(gui::Base * obj,gui::Event * event);
  void SaveImage(gui::Base * obj,gui::Event * event);
  void SaveIntrinsic(gui::Base * obj,gui::Event * event);


  void RenderLine(const math::Vect<2,real64> & a,const math::Vect<2,real64> & b,const bs::ColourRGB & col);
};

//------------------------------------------------------------------------------
#endif
