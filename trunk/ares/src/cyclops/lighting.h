#ifndef CYCLOPS_LIGHTING_H
#define CYCLOPS_LIGHTING_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

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
// Allows you to relight images, and calculate albedos.
class Lighting
{
 public:
   Lighting(Cyclops & cyclops);
  ~Lighting(); 


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  
  gui::ComboBox * viewSelect;
  gui::EditBox * origLight;
  gui::EditBox * newLight;
  gui::Label * albLab;
  
  math::Func crf;
  
  svt::Var * needleVar;
  svt::Field<bs::ColourRGB> needleImage;
  
  svt::Var * origVar;
  svt::Field<bs::ColourRGB> origImage;


  svt::Var * imageVar;
  svt::Field<bs::ColourRGB> image;

  svt::Var * imgVar;
  svt::Field<bs::ColRGB> img;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);

  void LoadNeedle(gui::Base * obj,gui::Event * event);
  void LoadOrig(gui::Base * obj,gui::Event * event);
  void LoadCRF(gui::Base * obj,gui::Event * event);
  void Render(gui::Base * obj,gui::Event * event);
  void SaveRender(gui::Base * obj,gui::Event * event);
  
  void Update();
};

//------------------------------------------------------------------------------
#endif
