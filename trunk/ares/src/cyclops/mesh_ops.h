#ifndef CYCLOPS_MESH_OPS_H
#define CYCLOPS_MESH_OPS_H
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
// Provides the ability to load and then save a mesh,  applying various
// operations inbetween...
class MeshOps
{
 public:
   MeshOps(Cyclops & cyc);
  ~MeshOps();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  sur::Mesh * mesh;

  gui::EditBox * remDupRange;
  
  gui::EditBox * simplifyAmount;
  gui::EditBox * simplifyRange;
  gui::EditBox * simplifyEdge;
  
  gui::EditBox * scaleMult;
  
  gui::Label * stats;


  void Quit(gui::Base * obj,gui::Event * event);

  void LoadMesh(gui::Base * obj,gui::Event * event);
  void SaveMesh(gui::Base * obj,gui::Event * event);
  void RemDup(gui::Base * obj,gui::Event * event);
  void Simplify(gui::Base * obj,gui::Event * event);
  void Scale(gui::Base * obj,gui::Event * event);
  void Triangulate(gui::Base * obj,gui::Event * event);

  void UpdateStats();
};

//------------------------------------------------------------------------------
#endif
