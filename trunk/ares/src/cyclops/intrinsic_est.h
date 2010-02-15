#ifndef CYCLOPS_INTRINSIC_EST_H
#define CYCLOPS_INTRINSIC_EST_H
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
// Given basic parameters for a camera produces an intrinsic matrix - obviously
// an estimate that will be wrong, but ok in certain situations.
class IntrinsicEst
{
 public:
   IntrinsicEst(Cyclops & cyclops);
  ~IntrinsicEst();


 private:
  Cyclops & cyclops;

  gui::Window * win;

  gui::EditBox * sizeX;  
  gui::EditBox * sizeY;
  
  bit xChange;
  gui::EditBox * mmX;
  gui::EditBox * angX;

  bit yChange;
  gui::EditBox * mmY;
  gui::EditBox * angY;


  void Quit(gui::Base * obj,gui::Event * event);

  void SaveICD(gui::Base * obj,gui::Event * event);

  void mmXChange(gui::Base * obj,gui::Event * event);
  void angXChange(gui::Base * obj,gui::Event * event);
  
  void mmYChange(gui::Base * obj,gui::Event * event);
  void angYChange(gui::Base * obj,gui::Event * event);

  void fromX(gui::Base * obj,gui::Event * event);
  void fromY(gui::Base * obj,gui::Event * event);
};

//------------------------------------------------------------------------------
#endif
