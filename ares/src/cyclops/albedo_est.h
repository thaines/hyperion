#ifndef CYCLOPS_ALBEDO_EST_H
#define CYCLOPS_ALBEDO_EST_H
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
// Does albedo estimation assuming a given disparity map provides reasonable
// surface orientation and that segments are constant albedo. As each pixel
// provides an estimate it takes the median of each segment, for a degree of
// robustness.
class AlbedoEst
{
 public:
   AlbedoEst(Cyclops & cyclops);
  ~AlbedoEst(); 


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  
  gui::ComboBox * viewSelect;
  gui::EditBox * lightDirection;
  gui::EditBox * albCap;

  gui::Label * results;

  ds::Array<real32> albedo;
  
  struct WeightEst
  {
   real32 estimate;
   real32 weight;
   
   bit operator < (const WeightEst & rhs) const
   {
    return estimate < rhs.estimate;
   }
  };


  math::Func crf;
  cam::CameraPair pair;

  svt::Var * irrVar;
  svt::Field<bs::ColourRGB> irr;
  
  svt::Var * segVar;
  svt::Field<nat32> seg;
  
  svt::Var * dispVar;
  svt::Field<real32> disp;
  svt::Field<bit> dispMask; // Optional.


  svt::Var * imageVar;
  svt::Field<bs::ColourRGB> image;

  svt::Var * imgVar;
  svt::Field<bs::ColRGB> img;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);

  void LoadIrr(gui::Base * obj,gui::Event * event);
  void LoadCRF(gui::Base * obj,gui::Event * event);
  void LoadSeg(gui::Base * obj,gui::Event * event);
  void LoadDisp(gui::Base * obj,gui::Event * event);
  void LoadPair(gui::Base * obj,gui::Event * event);
  void Run(gui::Base * obj,gui::Event * event);
  void ChangeView(gui::Base * obj,gui::Event * event);
  void SaveView(gui::Base * obj,gui::Event * event);

  void Update();
};

//------------------------------------------------------------------------------
#endif
