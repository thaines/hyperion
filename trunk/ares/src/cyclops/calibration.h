#ifndef CYCLOPS_CALIBRATION_H
#define CYCLOPS_CALIBRATION_H
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
// Allows you to calculate a camera projection matrix and radial distortion
// parameters from a single picture of a 3d target...
class Calibration
{
 public:
   Calibration(Cyclops & cyclops);
  ~Calibration(); 


 private:
  Cyclops & cyclops;


  gui::Window * win;
  gui::Canvas * canvas;
  gui::ComboBox * quality;
  gui::EditBox * worldX;
  gui::EditBox * worldY;
  gui::EditBox * worldZ;
  gui::Label * detail;

  
  bit resValid;
  cam::Camera camera;
  cam::Radial radial;

  svt::Var * var;
  svt::Field<bs::ColRGB> image;
  
  
  bs::Pnt pos;
  struct Match
  {
   bs::Vert world;
   bs::Pnt image;
  };
  ds::List<Match> data;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);
  void Click(gui::Base * obj,gui::Event * event);
  void Move(gui::Base * obj,gui::Event * event);

  void LoadImage(gui::Base * obj,gui::Event * event);
  void Add(gui::Base * obj,gui::Event * event);
  void Rem(gui::Base * obj,gui::Event * event);
  void Calculate(gui::Base * obj,gui::Event * event);
  void SaveCamera(gui::Base * obj,gui::Event * event);
  void SaveIntrinsic(gui::Base * obj,gui::Event * event);
  void SaveState(gui::Base * obj,gui::Event * event);
  void AppendState(gui::Base * obj,gui::Event * event);
  void ClearState(gui::Base * obj,gui::Event * event);

  
  void RenderPoint(const bs::Pnt & p,const bs::ColourRGB & col);
};

//------------------------------------------------------------------------------
#endif
