#ifndef CYCLOPS_SFS_H
#define CYCLOPS_SFS_H
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
// Runs assorted shape from shading algorithms to produce needle maps.
class SfS
{
 public:
   SfS(Cyclops & cyclops);
  ~SfS();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * canvas;


  gui::ComboBox * whichAlg;
  gui::EditBox * albedo;
  gui::EditBox * lightDir;
  
  gui::Expander * alg1;
  gui::EditBox * zacMu;
  gui::EditBox * zacIters;
  gui::EditBox * zacDelta;
   
  gui::Expander * alg2;
  gui::EditBox * lakOuterIters;
  gui::EditBox * lakInnerIters;
  gui::EditBox * lakTolerance;
  gui::EditBox * lakInitRad;
  gui::EditBox * lakStartLambda;
  gui::EditBox * lakEndLambda;
  gui::EditBox * lakSpeed;
  
  gui::Expander * alg3;
  gui::EditBox * wahIters;
  
  gui::Expander * alg4;
  gui::EditBox * hawBlur;
  gui::EditBox * hawLength;
  gui::EditBox * hawExp;
  gui::EditBox * hawStopChance;
  gui::EditBox * hawSimK;
  gui::EditBox * hawConeK;
  gui::EditBox * hawFadeTo;
  gui::EditBox * hawIters;
  gui::EditBox * hawGradDisc;
  gui::EditBox * hawGradBias;
  gui::EditBox * hawBorderK;
  gui::TickBox * hawProject;
  
  gui::Expander * alg5;
  gui::EditBox * haw2SimK;
  gui::EditBox * haw2ConeK;
  gui::EditBox * haw2FadeTo;
  gui::EditBox * haw2Iters;
  gui::EditBox * haw2AngMult;
  gui::EditBox * haw2Momentum;
  gui::EditBox * haw2BoundK;
  gui::EditBox * haw2BoundLength;
  gui::EditBox * haw2BoundExp;
  gui::EditBox * haw2GradK;
  gui::EditBox * haw2GradLength;
  gui::EditBox * haw2GradExp;
  
  gui::Expander * alg6;
  gui::EditBox * haw3SmoothChance;
  gui::EditBox * haw3SmoothBase;
  gui::EditBox * haw3SmoothMult;
  gui::EditBox * haw3SmoothMinK;
  gui::EditBox * haw3SmoothMaxK;
  gui::EditBox * haw3Cone0;
  gui::EditBox * haw3Cone45;
  gui::EditBox * haw3Cone90;
  gui::EditBox * haw3Iters;
  gui::EditBox * haw3BoundK;
  gui::EditBox * haw3BoundLength;
  gui::EditBox * haw3BoundExp;
  gui::EditBox * haw3GradK;
  gui::EditBox * haw3GradLength;
  gui::EditBox * haw3GradExp;
  gui::EditBox * haw3AngMult;
  gui::EditBox * haw3Momentum;

  math::Func crf;

  svt::Var * dataVar;
  svt::Field<bs::ColourRGB> image;
  svt::Field<bs::Normal> needle;
  
  svt::Var * albedoVar;
  svt::Field<bs::ColourRGB> albedoMap;

  svt::Var * iniNeedleVar;
  svt::Field<bs::ColourRGB> iniNeedle; // Used by w&h only.

  svt::Var * visVar;
  svt::Field<bs::ColRGB> visible;
  bit hasRun;


  void Quit(gui::Base * obj,gui::Event * event);

  void Resize(gui::Base * obj,gui::Event * event);

  void LoadImage(gui::Base * obj,gui::Event * event);
  void LoadAlbedo(gui::Base * obj,gui::Event * event);
  void LoadCalib(gui::Base * obj,gui::Event * event);
  void LoadInitialNeedle(gui::Base * obj,gui::Event * event);
  void Run(gui::Base * obj,gui::Event * event);
  void SaveNeedle(gui::Base * obj,gui::Event * event);

  void ChangeAlg(gui::Base * obj,gui::Event * event);

  void UpdateView();
};

//------------------------------------------------------------------------------
#endif
