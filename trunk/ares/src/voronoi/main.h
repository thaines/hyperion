#ifndef VORONOI_MAIN_H
#define VORONOI_MAIN_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

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
