#ifndef CYCLOPS_ANAGLYPH_H
#define CYCLOPS_ANAGLYPH_H
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
// Creates Anaglyphs, not exactly complicated.
class Anaglyph
{
 public:
   Anaglyph(Cyclops & cyclops);
  ~Anaglyph();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  gui::EditBox * offset;


  svt::Var * leftVar;
  svt::Field<bs::ColourRGB> leftImage;

  svt::Var * rightVar;
  svt::Field<bs::ColourRGB> rightImage;

  svt::Var * var;
  svt::Field<bs::ColRGB> image;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);

  void LoadLeft(gui::Base * obj,gui::Event * event);
  void LoadRight(gui::Base * obj,gui::Event * event);
  void OffsetChange(gui::Base * obj,gui::Event * event);
  void SaveAnaglyph(gui::Base * obj,gui::Event * event);

  void Update();
};

//------------------------------------------------------------------------------
#endif
