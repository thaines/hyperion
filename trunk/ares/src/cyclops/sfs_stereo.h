#ifndef CYCLOPS_SFS_STEREO_H
#define CYCLOPS_SFS_STEREO_H
//------------------------------------------------------------------------------
// Copyright 2009 Tom Haines

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
