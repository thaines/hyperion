#ifndef CYCLOPS_COMP_NEEDLE_H
#define CYCLOPS_COMP_NEEDLE_H
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
// Allows you to compare needle maps.
class CompNeedle
{
 public:
   CompNeedle(Cyclops & cyclops);
  ~CompNeedle(); 


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  
  gui::EditBox * cap;
  real32 capVal;
  gui::Label * resLab;
  gui::Label * resLab2;
  gui::Label * resLab3;


  svt::Var * truthVar;
  svt::Field<bs::ColourRGB> truthImage;
  
  svt::Var * guessVar;
  svt::Field<bs::ColourRGB> guessImage;

  svt::Var * imageVar;
  svt::Field<bs::ColRGB> image;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);

  void LoadTruth(gui::Base * obj,gui::Event * event);
  void LoadGuess(gui::Base * obj,gui::Event * event);
  void OnCapChange(gui::Base * obj,gui::Event * event);
  
  void Update();
};

//------------------------------------------------------------------------------
#endif
