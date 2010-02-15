#ifndef CYCLOPS_MODEL_RENDER_H
#define CYCLOPS_MODEL_RENDER_H
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
// Allows you to render 3d objects to create normal maps.
class ModelRender
{
 public:
   ModelRender(Cyclops & cyclops);
  ~ModelRender(); 


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;

  cam::CameraFull camera;
  ds::Array2D<bs::Ray> camRays; // Optimisation data structure.
  
  svt::Var * floatVar;
  svt::Field<bs::ColourRGB> floatImage;
  ds::Array2D<real32> depth; // Depth map, used for rendering.

  svt::Var * var;
  svt::Field<bs::ColRGB> image;


  void Quit(gui::Base * obj,gui::Event * event);
  void Resize(gui::Base * obj,gui::Event * event);

  void LoadCam(gui::Base * obj,gui::Event * event);
  void AddModelNeedle(gui::Base * obj,gui::Event * event);
  void SaveImage(gui::Base * obj,gui::Event * event);

  void UpdateImage();
};

//------------------------------------------------------------------------------
#endif
