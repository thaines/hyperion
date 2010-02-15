#ifndef CYCLOPS_SFS_STEREO_H
#define CYCLOPS_SFS_STEREO_H
//------------------------------------------------------------------------------
// Copyright 2009 Tom Haines

#include "cyclops/main.h"

//------------------------------------------------------------------------------
// Combines a disparity map with a needle map to produce a refined disparity map.
class StereoSfS
{
 public:
   StereoSfS(Cyclops & cyclops);
  ~StereoSfS(); 


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  
  gui::ComboBox * viewSelect;
  gui::EditBox * needleWeight;
  gui::EditBox * iterations;
  gui::EditBox * deltaCap;


  cam::CameraPair pair;

  svt::Var * dispVar;
  svt::Field<real32> disp;
  svt::Field<real32> dispNew; // Created and used to store the results of running the algorithm. - 'disp_new'
  svt::Field<bit> mask;
  svt::Field<real32> dispSd; // Standard deviation of Gaussian fitted to disparity value.
  
  svt::Var * needleVar;
  svt::Field<math::Fisher> needle; // When loading an image all entries set to a concentration of 1.
  bit noNeedleFish; //  true if a needle map image was loaded, false if Fisher distributions were loaded.


  svt::Var * imageVar;
  svt::Field<bs::ColourRGB> image;

  svt::Var * imgVar;
  svt::Field<bs::ColRGB> img;


  void Quit(gui::Base * obj,gui::Event * event);
  void Resize(gui::Base * obj,gui::Event * event);

  void LoadDisp(gui::Base * obj,gui::Event * event);
  void LoadNeedle(gui::Base * obj,gui::Event * event);
  void LoadCalib(gui::Base * obj,gui::Event * event);
  void Run(gui::Base * obj,gui::Event * event);
  void ChangeView(gui::Base * obj,gui::Event * event);
  void SaveView(gui::Base * obj,gui::Event * event);
  void SaveDisp(gui::Base * obj,gui::Event * event);

  void Update();
};

//------------------------------------------------------------------------------
#endif
