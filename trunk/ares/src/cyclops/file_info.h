#ifndef CYCLOPS_FILE_INFO_H
#define CYCLOPS_FILE_INFO_H
//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Simple class to show info about various file types, i.e. load a camera file 
// to get its position, orientation etc.
class FileInfo
{
 public:
   FileInfo(Cyclops & cyclops);
  ~FileInfo();

 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Multiline * ml;
  
  void Done(gui::Base * obj,gui::Event * event);
  void Load(gui::Base * obj,gui::Event * event);
};

//------------------------------------------------------------------------------
#endif
