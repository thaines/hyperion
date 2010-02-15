#ifndef CYCLOPS_CAPTURE_H
#define CYCLOPS_CAPTURE_H
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


#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Allows a user to drive two cameras connected to the computer to capture 
// stereo pairs.
class Capture
{
 public:
   Capture(Cyclops & cyc);
  ~Capture();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * left;
  gui::Canvas * right;

  svt::Var * leftVar;
  svt::Field<bs::ColRGB> leftImage;
  svt::Var * rightVar;
  svt::Field<bs::ColRGB> rightImage;


  os::CameraDetector cd;

  cstr leftName;
  cstr leftPort;
  bit leftPtp;

  cstr rightName;
  cstr rightPort;
  bit rightPtp;

  bs::Element * leftConfig;
  bs::Element * rightConfig;

  
  void Quit(gui::Base * obj,gui::Event * event);

  void ResizeLeft(gui::Base * obj,gui::Event * event);
  void ResizeRight(gui::Base * obj,gui::Event * event);
  
  void SelectLeft(gui::Base * obj,gui::Event * event);
  void SelectRight(gui::Base * obj,gui::Event * event);
  void ConfigLeft(gui::Base * obj,gui::Event * event);
  void ConfigRight(gui::Base * obj,gui::Event * event);

  void LoadConfig(gui::Base * obj,gui::Event * event);
  void SaveConfig(gui::Base * obj,gui::Event * event);
  
  void CaptureNow(gui::Base * obj,gui::Event * event);
  void CaptureLeft(gui::Base * obj,gui::Event * event);
  void CaptureRight(gui::Base * obj,gui::Event * event);
  
  void SaveLeft(gui::Base * obj,gui::Event * event); 
  void SaveRight(gui::Base * obj,gui::Event * event); 
};

//------------------------------------------------------------------------------
// Allows a user to select a camera from the avaliable cameras...
class SelectCamera
{
 public:
   SelectCamera(Cyclops & cyc,os::CameraDetector & cd,cstr & name,cstr & port,bit & ptp,bs::Element *& config);
  ~SelectCamera();


 private:
  Cyclops & cyclops;
  os::CameraDetector & camD;
  cstr & name;
  cstr & port;
  bit & ptp;
  bs::Element *& config;
  
  gui::Window * win;
  gui::ComboBox * camSel;
  gui::TickBox * ptpTick;

  void Quit(gui::Base * obj,gui::Event * event);
  void Store(gui::Base * obj,gui::Event * event);
};

//------------------------------------------------------------------------------
// Allows the user to edit a xml hierachy...
class EditXML
{
 public:
  EditXML(Cyclops & cyc,bs::Element *& data);
  ~EditXML();


 private:
  Cyclops & cyclops;
  bs::Element *& data;
  
  gui::Window * win;
  gui::Multiline * ml;
  
  void Quit(gui::Base * obj,gui::Event * event);
  void Store(gui::Base * obj,gui::Event * event); 
};

//------------------------------------------------------------------------------
// For capturing from multiple cameras simultaneously - a thread object that
// calls capture...
class EOS_CLASS CaptureThread : public mt::Thread
{
 public:
   CaptureThread(os::Camera & cam,svt::Var * out,svt::Field<bs::ColRGB> & field);
  ~CaptureThread();
  
  void 	Execute();
  
 private:
  os::Camera & cam;
  svt::Var * out;
  svt::Field<bs::ColRGB> & field;
};

//------------------------------------------------------------------------------
#endif
