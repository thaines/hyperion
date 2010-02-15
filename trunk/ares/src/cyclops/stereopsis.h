#ifndef CYCLOPS_STEREOPSIS_H
#define CYCLOPS_STEREOPSIS_H
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
// Allows a user to apply various stereo algorithms to an image pair.
class Stereopsis
{
 public:
   Stereopsis(Cyclops & cyc);
  ~Stereopsis();


 private:
  Cyclops & cyclops;

  gui::Window * win;
  gui::Canvas * left;
  gui::Canvas * right;


  gui::ComboBox * whichAlg;
  gui::ComboBox * whichPost;
  gui::TickBox * augGaussian;
  gui::TickBox * altAugG;
  gui::TickBox * augFisher;
  
  gui::Button * but7;

  gui::Expander * alg5;
  gui::Expander * alg6;
  gui::Expander * alg7;
  gui::Expander * post2;
  gui::Expander * post3a;
  gui::Expander * post3b;
  gui::Expander * post4;
  gui::Expander * augG;
  gui::Expander * augAltG;
  gui::Expander * augF;

  gui::EditBox * occCost;
  gui::EditBox * vertCost;
  gui::EditBox * vertMult;
  gui::EditBox * errLim;
  gui::EditBox * matchLim;
  
  gui::EditBox * bpOccCostHigh;
  gui::EditBox * bpOccCostLow;
  gui::EditBox * bpOccLimMult;
  gui::EditBox * bpMatchLim;
  gui::EditBox * bpOccLim;
  gui::EditBox * bpIters;
  gui::EditBox * bpOutput;
  
  gui::TickBox * dcUseHalfX;
  gui::TickBox * dcUseHalfY;
  gui::TickBox * dcUseCorners;
  gui::TickBox * dcHalfHeight;
  gui::EditBox * dcDistMult;
  gui::EditBox * dcDiffSteps;
  gui::EditBox * dcMinimaLimit;
  gui::EditBox * dcBaseDistCap;
  gui::EditBox * dcDistCapMult;
  gui::EditBox * dcDistCapThreshold;
  gui::EditBox * dcDispRange;
  gui::TickBox * dcDoLR;
  gui::EditBox * dcDistCapDifference;

  gui::EditBox * smoothStrength;
  gui::EditBox * smoothCutoff;
  gui::EditBox * smoothWidth;
  gui::EditBox * smoothIters;
  
  gui::EditBox * planeRadius;
  gui::EditBox * planeBailOut;
  gui::EditBox * planeOcc;
  gui::EditBox * planeDisc;

  gui::EditBox * segSpatial;
  gui::EditBox * segRange;
  gui::EditBox * segMin;
  gui::EditBox * segRad;
  gui::EditBox * segMix;
  gui::EditBox * segEdge;
  
  gui::TickBox * polyUseHalfX;
  gui::TickBox * polyUseHalfY;
  gui::TickBox * polyUseCorners;
  gui::EditBox * polyDistMult;
  gui::EditBox * polyDiffSteps;
  gui::EditBox * polyDistCap;
  gui::EditBox * polyPrune;
  
  gui::EditBox * gaussianRadius;
  gui::EditBox * gaussianFalloff;
  gui::EditBox * gaussianRange;
  gui::EditBox * gaussianMult;
  gui::EditBox * gaussianSdMult;
  gui::EditBox * gaussianMin;
  gui::EditBox * gaussianMax;
  gui::EditBox * gaussianMinK;
  gui::EditBox * gaussianMaxK;
  gui::EditBox * gaussianIters;

  gui::EditBox * agSd;
  gui::EditBox * agCostMult;
  gui::EditBox * agMin;
  gui::EditBox * agMax;
  gui::EditBox * agSdMult;

  //gui::EditBox * fisherRange;
  gui::EditBox * fisherProb;
  gui::EditBox * fisherMult;
  gui::EditBox * fisherMin;
  gui::EditBox * fisherMax;
  //gui::EditBox * fisherBias;
  cam::CameraPair pair;


  svt::Var * leftVar;
  svt::Field<bs::ColRGB> leftImage;
  svt::Var * rightVar;
  svt::Field<bs::ColRGB> rightImage;

  svt::Var * leftImg;
  svt::Var * rightImg;
  svt::Var * existing;
  svt::Var * result;
  
  svt::Var * segmentation;

  void Quit(gui::Base * obj,gui::Event * event);

  void ResizeLeft(gui::Base * obj,gui::Event * event);
  void ResizeRight(gui::Base * obj,gui::Event * event);

  void LoadLeft(gui::Base * obj,gui::Event * event);
  void LoadRight(gui::Base * obj,gui::Event * event);

  void LoadCalibration(gui::Base * obj,gui::Event * event);
  void LoadSeg(gui::Base * obj,gui::Event * event);
  void LoadExisting(gui::Base * obj,gui::Event * event);
  
  void ChangeAlg(gui::Base * obj,gui::Event * event);
  void ChangePost(gui::Base * obj,gui::Event * event);
  void SwitchGaussian(gui::Base * obj,gui::Event * event);
  void SwitchAltGaussian(gui::Base * obj,gui::Event * event);
  void SwitchFisher(gui::Base * obj,gui::Event * event);

  void Run(gui::Base * obj,gui::Event * event);

  void SaveSVT(gui::Base * obj,gui::Event * event);
};

//------------------------------------------------------------------------------
#endif
