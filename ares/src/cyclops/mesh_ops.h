#ifndef CYCLOPS_MESH_OPS_H
#define CYCLOPS_MESH_OPS_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

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
