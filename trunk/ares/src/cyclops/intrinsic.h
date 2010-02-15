#ifndef CYCLOPS_INTRINSIC_H
#define CYCLOPS_INTRINSIC_H
//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Predeclerations...
class Shot;

//------------------------------------------------------------------------------
// Main window for intrinsic calibration.
class IntrinsicCalib
{
 public:
   IntrinsicCalib(Cyclops & cyclops);
  ~IntrinsicCalib();

  // Maintain a linked list of shots, so when calibrate is hit it can get all
  // the relevent details - these are called by the Shot objects in constructor/destructor.
   void Register(Shot * shot);
   void Unregister(Shot * shot);
   
  // Returns the first Shot object, or null if none exist...
   Shot * FirstShot() {if (shots.Size()==0) return null<Shot*>(); else return shots.Front();}


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::ComboBox * quality;
  gui::Multiline * ml;
  
  ds::List<Shot*> shots;
  
  cam::Zhang98::ResQuality resQ;
  nat32 imgWidth;
  nat32 imgHeight;

  real64 residual;
  cam::Intrinsic intrinsic;
  cam::Radial radial;
  
  
  void Done(gui::Base * obj,gui::Event * event);
  void Load(gui::Base * obj,gui::Event * event);
  void Calc(gui::Base * obj,gui::Event * event);
  void Zhang(gui::Base * obj,gui::Event * event);
  void Save(gui::Base * obj,gui::Event * event);
  void SaveState(gui::Base * obj,gui::Event * event);
};

//------------------------------------------------------------------------------
// Class to represent each open image that is going to contribute to the
// calibration, contains all the details in regards to a given shot.
class Shot
{
 public:
  // Constructor that assumed the object is constructed with new and
  // automatically registers itself, in other words it will commit suicide
  // if given a file it can't open.
   Shot(Cyclops & cyclops,IntrinsicCalib & intCalib,const str::String & fn);   
  ~Shot();


  static const nat32 gridSize = 16;
  math::Mat<gridSize,gridSize,math::Vect<2,real32> > pos; // User selected position of each point, pattern position is array position.

  bit DoingIni() const {return doingIni;}
  
  // Takes either index on either axis to a model in the same axis.
  static real32 ToModel(nat32 i)
  {
   return i + (i+1)/2;
  }


  const str::String & Filename() const {return filename;}
  nat32 Width() const {return image.Size(0);}
  nat32 Height() const {return image.Size(1);}


 private:	
  Cyclops & cyclops;
  IntrinsicCalib & intCalib;

  gui::Window * win;
  gui::Canvas * canvas;
  
  str::String filename;
  svt::Var * var;
  svt::Field<bs::ColRGB> image;
  
  bit doingIni; // If true the user still needs to select the initial 4 corners, otherwise its full editting mode.
  nat32 iniStep; // How far through ini the user is.
  
  str::String cgFn; // Contains the Store/Restore filename.
  filter::HarrisSnap * harrisSnap;
  
  void Quit(gui::Base * obj,gui::Event * event);
  void Resize(gui::Base * obj,gui::Event * event);
  void Click(gui::Base * obj,gui::Event * event);
  void Move(gui::Base * obj,gui::Event * event);
  
  void Reset(gui::Base * obj,gui::Event * event);
  void Restore(gui::Base * obj,gui::Event * event);
  void Store(gui::Base * obj,gui::Event * event);
  void Snap(gui::Base * obj,gui::Event * event);
  
  void RenderPoint(const math::Vect<2,real32> & p,const bs::ColourRGB & col);
  void RenderLine(const math::Vect<2,real32> & a,const math::Vect<2,real32> & b,const bs::ColourRGB & col);
};

//------------------------------------------------------------------------------
#endif
