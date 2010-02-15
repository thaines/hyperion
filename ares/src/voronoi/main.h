#ifndef VORONOI_MAIN_H
#define VORONOI_MAIN_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

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


#include "eos.h"

using namespace eos;

//------------------------------------------------------------------------------
// Class to represent the application as a whole - contains both the App and 
// main window...
class Voronoi
{
 public:
   Voronoi();
  ~Voronoi();
  
  svt::Core & Core() {return core;}
  gui::Factory & Fact() {return guiFact;}
  gui::App & App() {return *app;}


 private:
  str::TokenTable tokTab;
  svt::Core core;
  gui::GtkFactory guiFact;
  
  gui::App * app;
  gui::Window * win;
  gui::Canvas * canvas;
  
  ds::Delaunay2D<nat32> tri;
  
  void Resize(gui::Base * obj,gui::Event * event);
  void Click(gui::Base * obj,gui::Event * event);  
};

//------------------------------------------------------------------------------
#endif
