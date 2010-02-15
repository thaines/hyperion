#ifndef CYCLOPS_HOMOGRAPHY_H
#define CYCLOPS_HOMOGRAPHY_H
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
// Allows the distortion of an image by selecting a 4 sided polygon to be made
// square - good for extracting textures from images.
class Homography
{
 public:
   Homography(Cyclops & cyclops);
  ~Homography();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * left;
  gui::Canvas * right;
  
  gui::EditBox * widthEdit;
  gui::EditBox * heightEdit;
  gui::EditBox * xLowEdit;
  gui::EditBox * xHighEdit;
  gui::EditBox * yLowEdit;
  gui::EditBox * yHighEdit;
 

  svt::Var * imageFullVar;
  svt::Field<bs::ColourRGB> imageFull;
  
  svt::Var * imageVar;
  svt::Field<bs::ColRGB> image;


  svt::Var * transFullVar;
  svt::Field<bs::ColourRGB> transFull;

  svt::Var * transVar;
  svt::Field<bs::ColRGB> trans;


  int32 width,height;
  math::Vect<2,real64> corner[4];
  real64 xLow,xHigh;
  real64 yLow,yHigh;
  
  nat32 nextSnap;



  void Quit(gui::Base * obj,gui::Event * event);

  void ResizeLeft(gui::Base * obj,gui::Event * event);
  void ClickLeft(gui::Base * obj,gui::Event * event);
  void MoveLeft(gui::Base * obj,gui::Event * event);
  void ResizeRight(gui::Base * obj,gui::Event * event);
  
  void OnEditChange(gui::Base * obj,gui::Event * event);

  void LoadImage(gui::Base * obj,gui::Event * event);
  void Calculate(gui::Base * obj,gui::Event * event);
  void SaveImage(gui::Base * obj,gui::Event * event);


  void RenderLeftPoint(const math::Vect<2,real64> & p,const bs::ColourRGB & col);
  void RenderLeftLine(const math::Vect<2,real64> & a,const math::Vect<2,real64> & b,const bs::ColourRGB & col);
};

//------------------------------------------------------------------------------
#endif
