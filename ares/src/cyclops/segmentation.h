#ifndef CYCLOPS_SEGMENTATION_H
#define CYCLOPS_SEGMENTATION_H
//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

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
// Allows you to segment images.
class Segmentation
{
 public:
   Segmentation(Cyclops & cyclops);
  ~Segmentation(); 


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;
  
  gui::ComboBox * viewSelect;
  gui::ComboBox * algSelect;
  
  gui::Expander * kMeanGrid;
  gui::EditBox * kmgDim;
  gui::EditBox * kmgMinSeg;
  gui::EditBox * kmgColMult;
  gui::EditBox * kmgSpatialMult;
  gui::EditBox * kmgMaxIters;
  
  gui::Expander * meanShift;
  gui::EditBox * msEdgeWindow;
  gui::EditBox * msEdgeMix;
  gui::EditBox * msSpatialSize;
  gui::EditBox * msColourSize;
  gui::EditBox * msMergeCutoff;
  gui::EditBox * msMinSeg;
  gui::EditBox * msAvgSteps;
  gui::EditBox * msMergeMax;
  
  
  svt::Var * inputVar;
  svt::Field<bs::ColourRGB> inputImage;
  svt::Field<nat32> segmentation;


  svt::Var * imageVar;
  svt::Field<bs::ColourRGB> image;

  svt::Var * imgVar;
  svt::Field<bs::ColRGB> img;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);

  void LoadImage(gui::Base * obj,gui::Event * event);
  void Run(gui::Base * obj,gui::Event * event);
  void ChangeView(gui::Base * obj,gui::Event * event);
  void ChangeAlg(gui::Base * obj,gui::Event * event);
  void SaveView(gui::Base * obj,gui::Event * event);
  void SaveSeg(gui::Base * obj,gui::Event * event);
  
  void Update();
};

//------------------------------------------------------------------------------
#endif
