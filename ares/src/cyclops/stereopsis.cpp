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


#include "cyclops/stereopsis.h"

//------------------------------------------------------------------------------
Stereopsis::Stereopsis(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
leftVar(null<svt::Var*>()),rightVar(null<svt::Var*>()),
leftImg(null<svt::Var*>()),rightImg(null<svt::Var*>()),existing(null<svt::Var*>()),
result(null<svt::Var*>()),segmentation(null<svt::Var*>())
{
 // Create default images...
  leftVar = new svt::Var(cyclops.Core());
  leftVar->Setup2D(320,240);
  bs::ColRGB colIni(0,0,0);
  leftVar->Add("rgb",colIni);
  leftVar->Commit();
  leftVar->ByName("rgb",leftImage);

  rightVar = new svt::Var(cyclops.Core());
  rightVar->Setup2D(320,240);
  rightVar->Add("rgb",colIni);
  rightVar->Commit();
  rightVar->ByName("rgb",rightImage);


 // Invoke gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Stereopsis");
  cyclops.App().Attach(win);
  win->SetSize(leftImage.Size(0)+rightImage.Size(0)+48,math::Max(leftImage.Size(1),rightImage.Size(1))+128);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);

   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);


   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but1->SetChild(lab1); lab1->Set("Load Left Image...");
   but2->SetChild(lab2); lab2->Set("Load Right Image...");

   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but2,false);
   
   
   but7 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * labBut7 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   
   but7->SetChild(labBut7); labBut7->Set("Load Existing...");
   horiz1->AttachRight(but7,false);
   but7->Visible(false);


   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   but3->SetChild(lab3); lab3->Set("Run");
   horiz1->AttachRight(but3,false);


   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   but4->SetChild(lab4); lab4->Set("Save Disparity...");
   horiz1->AttachRight(but4,false);

   
   gui::Horizontal * horiz6 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz6,false);
   
   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lab5->Set(" Alg:");
   horiz6->AttachRight(lab5,false);

   whichAlg = static_cast<gui::ComboBox*>(cyclops.Fact().Make("ComboBox"));
   whichAlg->Append("Existing");
   whichAlg->Append("Hierarchical DP");
   whichAlg->Append("Hierarchical BP");
   whichAlg->Append("Diffusion Correlation");
   whichAlg->Set(2);
   horiz6->AttachRight(whichAlg,false);

   gui::Label * lab5b = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lab5b->Set(" Post:");
   horiz6->AttachRight(lab5b,false);

   whichPost = static_cast<gui::ComboBox*>(cyclops.Fact().Make("ComboBox"));
   whichPost->Append("None");
   whichPost->Append("Smoothing");
   whichPost->Append("Seg & Plane Fit");
   whichPost->Append("Poly Diffusion");
   whichPost->Set(1);
   horiz6->AttachRight(whichPost,false);
   
   gui::Label * lab5c = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lab5c->Set(" Aug Gaussian:");
   horiz6->AttachRight(lab5c,false);
   
   augGaussian = static_cast<gui::TickBox*>(cyclops.Fact().Make("TickBox"));
   horiz6->AttachRight(augGaussian,false);

   gui::Label * lab5d = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lab5d->Set(" Aug Fisher:");
   horiz6->AttachRight(lab5d,false);
   
   augFisher = static_cast<gui::TickBox*>(cyclops.Fact().Make("TickBox"));
   horiz6->AttachRight(augFisher,false);


   alg5 = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(alg5,false);
   alg5->Visible(false);
   gui::Vertical * vert2 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   alg5->SetChild(vert2);
   alg5->Set("Hierarchical DP Parameters");
   alg5->Expand(false);

   gui::Horizontal * horiz1b = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz1c = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert2->AttachBottom(horiz1b,false);
   vert2->AttachBottom(horiz1c,false);

   gui::Label * lab6 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab7 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab8 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab9 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab9b = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   lab6->Set("    Occlusion Cost:");
   lab7->Set("    Vertical Cost:");
   lab8->Set("Vertical Multiplier:");
   lab9->Set("Match Limit:");
   lab9b->Set("Diff Cap:");

   occCost = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   vertCost = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   vertMult = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   errLim = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   matchLim = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   horiz1b->AttachRight(lab6,false);
   horiz1b->AttachRight(occCost,false);
   horiz1b->AttachRight(lab9,false);
   horiz1b->AttachRight(errLim,false);
   horiz1b->AttachRight(lab9b,false);
   horiz1b->AttachRight(matchLim,false);
   horiz1c->AttachRight(lab7,false);
   horiz1c->AttachRight(vertCost,false);
   horiz1c->AttachRight(lab8,false);
   horiz1c->AttachRight(vertMult,false);

   occCost->Set("5.0"); occCost->SetSize(48,24);
   vertCost->Set("0.25"); vertCost->SetSize(48,24);
   vertMult->Set("0.75"); vertMult->SetSize(48,24);
   errLim->Set("1"); errLim->SetSize(48,24);
   matchLim->Set("10.0"); matchLim->SetSize(48,24);



   alg6 = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(alg6,false);
   //alg6->Visible(false);
   gui::Vertical * vert6 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   alg6->SetChild(vert6);
   alg6->Set("Hierarchical BP Parameters");
   alg6->Expand(false);

   gui::Horizontal * horiz5a = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz5b = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert6->AttachBottom(horiz5a,false);
   vert6->AttachBottom(horiz5b,false);

   gui::Label * lab24 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab24b = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab25 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab26 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab26b = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab27 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab27b = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   lab24->Set("    High Occlusion Cost:");
   lab24b->Set("Low Occlusion Cost:");
   lab25->Set("Occlusion Limit Multiplier:");
   lab26->Set("    Match Diff Cap:");
   lab26b->Set("Occlusion Diff Cap:");
   lab27->Set("Iters:");
   lab27b->Set("Output Cap:");

   bpOccCostHigh = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   bpOccCostLow = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   bpOccLimMult = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   bpMatchLim = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   bpOccLim = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   bpIters = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   bpOutput = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   horiz5a->AttachRight(lab24,false);
   horiz5a->AttachRight(bpOccCostHigh,false);
   horiz5a->AttachRight(lab24b,false);
   horiz5a->AttachRight(bpOccCostLow,false);
   horiz5a->AttachRight(lab25,false);
   horiz5a->AttachRight(bpOccLimMult,false);
   horiz5b->AttachRight(lab26,false);
   horiz5b->AttachRight(bpMatchLim,false);
   horiz5b->AttachRight(lab26b,false);
   horiz5b->AttachRight(bpOccLim,false);
   horiz5b->AttachRight(lab27,false);
   horiz5b->AttachRight(bpIters,false);
   horiz5b->AttachRight(lab27b,false);
   horiz5b->AttachRight(bpOutput,false);

   bpOccCostHigh->Set("16.0"); bpOccCostHigh->SetSize(48,24);
   bpOccCostLow->Set("8.0"); bpOccCostLow->SetSize(48,24);
   bpOccLimMult->Set("2.0"); bpOccLimMult->SetSize(48,24);
   bpMatchLim->Set("36.0"); bpMatchLim->SetSize(48,24);
   bpOccLim->Set("6.0"); bpOccLim->SetSize(48,24);
   bpIters->Set("6"); bpIters->SetSize(48,24);
   bpOutput->Set("1"); bpOutput->SetSize(48,24);
   
   
   
   alg7 = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(alg7,false);
   alg7->Visible(false);
   gui::Vertical * vert7 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   alg7->SetChild(vert7);
   alg7->Set("Diffusion Correlation");
   alg7->Expand(false);
   
   gui::Horizontal * horiz9a = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz9b = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz9c = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz9d = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert7->AttachBottom(horiz9a,false);
   vert7->AttachBottom(horiz9b,false);
   vert7->AttachBottom(horiz9c,false);
   vert7->AttachBottom(horiz9d,false);
   
   gui::Label * lab40 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab41 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab42 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab43 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab44 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab45 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab46 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab47 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab48 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab49 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab55 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab51 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab52 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   
   lab40->Set(" Use Half X:");
   lab41->Set(" Use Half Y:");
   lab42->Set(" Use Corners:");
   lab43->Set(" Half Height:");
   lab44->Set(" Distance Mult:");
   lab45->Set(" Diffusion Steps:");
   lab46->Set(" Minima Cap:");
   lab47->Set(" Base Distance Cap:");
   lab48->Set(" Cap Mult:");
   lab49->Set(" Cap Threshold:");
   lab55->Set(" Disparity Range:");
   lab51->Set(" Left-Right Check:");
   lab52->Set(" Distance Difference Cap:");

   dcUseHalfX = static_cast<gui::TickBox*>(cyclops.Fact().Make("TickBox"));
   dcUseHalfY = static_cast<gui::TickBox*>(cyclops.Fact().Make("TickBox"));
   dcUseCorners = static_cast<gui::TickBox*>(cyclops.Fact().Make("TickBox"));
   dcHalfHeight = static_cast<gui::TickBox*>(cyclops.Fact().Make("TickBox"));
   dcDistMult = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   dcDiffSteps = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   dcMinimaLimit = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   dcBaseDistCap = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   dcDistCapMult = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   dcDistCapThreshold = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   dcDispRange = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   dcDoLR = static_cast<gui::TickBox*>(cyclops.Fact().Make("TickBox"));
   dcDistCapDifference = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   
   horiz9a->AttachRight(lab40,false);
   horiz9a->AttachRight(dcUseHalfX,false);
   horiz9a->AttachRight(lab41,false);
   horiz9a->AttachRight(dcUseHalfY,false);
   horiz9a->AttachRight(lab42,false);
   horiz9a->AttachRight(dcUseCorners,false);
   horiz9a->AttachRight(lab43,false);
   horiz9a->AttachRight(dcHalfHeight,false);
   horiz9b->AttachRight(lab44,false);
   horiz9b->AttachRight(dcDistMult,false);
   horiz9b->AttachRight(lab45,false);
   horiz9b->AttachRight(dcDiffSteps,false);
   horiz9b->AttachRight(lab46,false);
   horiz9b->AttachRight(dcMinimaLimit,false);
   horiz9c->AttachRight(lab47,false);
   horiz9c->AttachRight(dcBaseDistCap,false);
   horiz9c->AttachRight(lab48,false);
   horiz9c->AttachRight(dcDistCapMult,false);
   horiz9c->AttachRight(lab49,false);
   horiz9c->AttachRight(dcDistCapThreshold,false);
   horiz9c->AttachRight(lab55,false);
   horiz9c->AttachRight(dcDispRange,false);
   horiz9d->AttachRight(lab51,false);
   horiz9d->AttachRight(dcDoLR,false);
   horiz9d->AttachRight(lab52,false);
   horiz9d->AttachRight(dcDistCapDifference,false);  
   
   dcUseHalfX->SetState(true);
   dcUseHalfY->SetState(true);
   dcUseCorners->SetState(true);
   dcHalfHeight->SetState(true);
   dcDistMult->Set("0.01"); dcDistMult->SetSize(48,24);
   dcDiffSteps->Set("5"); dcDiffSteps->SetSize(48,24);
   dcMinimaLimit->Set("2"); dcMinimaLimit->SetSize(48,24);
   dcBaseDistCap->Set("64.0"); dcBaseDistCap->SetSize(48,24);
   dcDistCapMult->Set("1.414"); dcDistCapMult->SetSize(48,24);
   dcDistCapThreshold->Set("0.4"); dcDistCapThreshold->SetSize(48,24);
   dcDispRange->Set("1"); dcDispRange->SetSize(48,24);
   dcDoLR ->SetState(true);
   dcDistCapDifference->Set("0.2"); dcDistCapDifference->SetSize(48,24);



   post2 = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(post2,false);
   gui::Vertical * vert3 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   post2->SetChild(vert3);
   post2->Set("Smoothing Parameters");
   post2->Expand(false);

   gui::Horizontal * horiz1d = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert3->AttachBottom(horiz1d,false);

   gui::Label * lab10 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab11 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab12 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab13 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   lab10->Set("    Strength:");
   lab11->Set(" Cutoff:");
   lab12->Set(" Half-life:");
   lab13->Set(" Iterations:");

   smoothStrength = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   smoothCutoff = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   smoothWidth = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   smoothIters = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   horiz1d->AttachRight(lab10,false);
   horiz1d->AttachRight(smoothStrength,false);
   horiz1d->AttachRight(lab12,false);
   horiz1d->AttachRight(smoothWidth,false);
   horiz1d->AttachRight(lab11,false);
   horiz1d->AttachRight(smoothCutoff,false);
   horiz1d->AttachRight(lab13,false);
   horiz1d->AttachRight(smoothIters,false);

   smoothStrength->Set("16.0"); smoothStrength->SetSize(48,24);
   smoothCutoff->Set("16.0"); smoothCutoff->SetSize(48,24);
   smoothWidth->Set("2.0"); smoothWidth->SetSize(48,24);
   smoothIters->Set("256"); smoothIters->SetSize(48,24);



   post3a = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(post3a,false);
   post3a->Visible(false);
   gui::Vertical * vert4 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   post3a->SetChild(vert4);
   post3a->Set("Seg & Plane Fit Parameters");
   post3a->Expand(false);

   gui::Horizontal * horiz3a = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz3b = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert4->AttachBottom(horiz3a,false);
   vert4->AttachBottom(horiz3b,false);

   gui::Label * lab14 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab15 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab16 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab17 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   lab14->Set("    Radius:");
   lab15->Set("Bail Out:");
   lab16->Set("    Occ Cost:");
   lab17->Set("Disc Cost:");

   planeRadius = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   planeBailOut = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   planeOcc = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   planeDisc = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   horiz3a->AttachRight(lab14,false);
   horiz3a->AttachRight(planeRadius,false);
   horiz3a->AttachRight(lab15,false);
   horiz3a->AttachRight(planeBailOut,false);
   horiz3b->AttachRight(lab16,false);
   horiz3b->AttachRight(planeOcc,false);
   horiz3b->AttachRight(lab17,false);
   horiz3b->AttachRight(planeDisc,false);

   planeRadius->Set("0.6"); planeRadius->SetSize(48,24);
   planeBailOut->Set("12"); planeBailOut->SetSize(48,24);
   planeOcc->Set("0.078"); planeOcc->SetSize(48,24);
   planeDisc->Set("0.01"); planeDisc->SetSize(48,24);



   post3b = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(post3b,false);
   post3b->Visible(false);
   gui::Vertical * vert5 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   post3b->SetChild(vert5);
   post3b->Set("Segmentation Parameters");
   post3b->Expand(false);

   gui::Button * but6 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab50 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   but6->SetChild(lab50); lab50->Set("Load Segmentation Instead...");
   vert5->AttachBottom(but6,false);
   
   gui::Horizontal * horiz4a = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz4b = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert5->AttachBottom(horiz4a,false);
   vert5->AttachBottom(horiz4b,false);

   gui::Label * lab18 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab19 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab20 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab21 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab22 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab23 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   lab18->Set("    Spatial Range:");
   lab19->Set("Colour Range:");
   lab20->Set("Min Size:");
   lab21->Set("    Radius:");
   lab22->Set("Mix:");
   lab23->Set("Cutoff:");

   segSpatial = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   segRange = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   segMin = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   segRad = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   segMix = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   segEdge = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   horiz4a->AttachRight(lab18,false);
   horiz4a->AttachRight(segSpatial,false);
   horiz4a->AttachRight(lab19,false);
   horiz4a->AttachRight(segRange,false);
   horiz4a->AttachRight(lab20,false);
   horiz4a->AttachRight(segMin,false);
   horiz4b->AttachRight(lab21,false);
   horiz4b->AttachRight(segRad,false);
   horiz4b->AttachRight(lab22,false);
   horiz4b->AttachRight(segMix,false);
   horiz4b->AttachRight(lab23,false);
   horiz4b->AttachRight(segEdge,false);

   segSpatial->Set("7.0"); segSpatial->SetSize(48,24);
   segRange->Set("4.5"); segRange->SetSize(48,24);
   segMin->Set("20"); segMin->SetSize(48,24);
   segRad->Set("2"); segRad->SetSize(48,24);
   segMix->Set("0.3"); segMix->SetSize(48,24);
   segEdge->Set("0.9"); segEdge->SetSize(48,24);
   
   
   
   post4 = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(post4,false);
   post4->Visible(false);
   gui::Vertical * vert10 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   post4->SetChild(vert10);
   post4->Set("Polynomial Refinement with Diffusion");
   post4->Expand(false);

   gui::Horizontal * horiz10 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz11 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert10->AttachBottom(horiz10,false);
   vert10->AttachBottom(horiz11,false);

   gui::Label * lab60 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab61 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab62 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab63 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab64 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab65 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab66 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   
   lab60->Set(" Use Half X:");
   lab61->Set(" Use Half Y:");
   lab62->Set(" Use Corners:");
   lab63->Set(" Dist Mult:");
   lab64->Set(" Diffusion Steps:");
   lab65->Set(" Dist Cap:");
   lab66->Set(" Prune:");
   
   polyUseHalfX = static_cast<gui::TickBox*>(cyclops.Fact().Make("TickBox"));
   polyUseHalfY = static_cast<gui::TickBox*>(cyclops.Fact().Make("TickBox"));
   polyUseCorners = static_cast<gui::TickBox*>(cyclops.Fact().Make("TickBox"));
   polyDistMult = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   polyDiffSteps = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   polyDistCap = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   polyPrune = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   
   horiz10->AttachRight(lab60,false);
   horiz10->AttachRight(polyUseHalfX,false);
   horiz10->AttachRight(lab61,false);
   horiz10->AttachRight(polyUseHalfY,false);
   horiz10->AttachRight(lab62,false);
   horiz10->AttachRight(polyUseCorners,false);
   horiz11->AttachRight(lab63,false);
   horiz11->AttachRight(polyDistMult,false);
   horiz11->AttachRight(lab64,false);
   horiz11->AttachRight(polyDiffSteps,false);
   horiz11->AttachRight(lab65,false);
   horiz11->AttachRight(polyDistCap,false);
   horiz11->AttachRight(lab66,false);
   horiz11->AttachRight(polyPrune,false);
   
   polyUseHalfX->SetState(true);
   polyUseHalfY->SetState(true);
   polyUseCorners->SetState(true);
   polyDistMult->Set("0.01"); polyDistMult->SetSize(48,24);
   polyDiffSteps->Set("7"); polyDiffSteps->SetSize(48,24);
   polyDistCap->Set("64.0"); polyDistCap->SetSize(48,24);
   polyPrune->Set("0.25"); polyPrune->SetSize(48,24);



   augG = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(augG,false);
   augG->Visible(true);
   gui::Vertical * vertG = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   augG->SetChild(vertG);
   augG->Set("Gaussian Fitting Parameters");
   augG->Expand(false);

   gui::Horizontal * horiz7 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz7b = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vertG->AttachBottom(horiz7,false);
   vertG->AttachBottom(horiz7b,false);

   gui::Label * lab70 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lab70->Set(" Radius:");
   gaussianRadius = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   gaussianRadius->Set("3");
   gaussianRadius->SetSize(48,24);

   gui::Label * lab71 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lab71->Set(" Falloff:");
   gaussianFalloff = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   gaussianFalloff->Set("0.25");
   gaussianFalloff->SetSize(48,24);
   
   gui::Label * lab28 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lab28->Set(" Range:");
   gaussianRange = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   gaussianRange->Set("64");
   gaussianRange->SetSize(48,24);

   gui::Label * lab53 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lab53->Set(" Mult:");
   gaussianMult = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   gaussianMult->Set("0.4");
   gaussianMult->SetSize(48,24);

   gui::Label * lab34 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lab34->Set(" Sd Mult:");
   gaussianSdMult = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   gaussianSdMult->Set("2.0");
   gaussianSdMult->SetSize(48,24);

   gui::Label * lab35 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lab35->Set(" Min Sd:");
   gaussianMin = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   gaussianMin->Set("0.1");
   gaussianMin->SetSize(48,24);
   
   gui::Label * lab36 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lab36->Set(" Max Sd:");
   gaussianMax = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   gaussianMax->Set("16.0");
   gaussianMax->SetSize(48,24);      
   
   gui::Label * lab38 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lab38->Set(" Min K:");
   gaussianMinK = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   gaussianMinK->Set("2.0");
   gaussianMinK->SetSize(48,24);
   
   gui::Label * lab39 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lab39->Set(" Max K:");
   gaussianMaxK = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   gaussianMaxK->Set("32.0");
   gaussianMaxK->SetSize(48,24);  
   
   gui::Label * lab37 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lab37->Set(" Max Iters:");
   gaussianIters = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   gaussianIters->Set("1000");
   gaussianIters->SetSize(48,24);
      
   horiz7->AttachRight(lab70,false);
   horiz7->AttachRight(gaussianRadius,false);
   horiz7->AttachRight(lab71,false);
   horiz7->AttachRight(gaussianFalloff,false);
   horiz7->AttachRight(lab28,false);
   horiz7->AttachRight(gaussianRange,false);
   horiz7->AttachRight(lab53,false);
   horiz7->AttachRight(gaussianMult,false);
   horiz7b->AttachRight(lab34,false);
   horiz7b->AttachRight(gaussianSdMult,false);
   horiz7b->AttachRight(lab35,false);
   horiz7b->AttachRight(gaussianMin,false);
   horiz7b->AttachRight(lab36,false);
   horiz7b->AttachRight(gaussianMax,false);
   horiz7b->AttachRight(lab38,false);
   horiz7b->AttachRight(gaussianMinK,false);
   horiz7b->AttachRight(lab39,false);
   horiz7b->AttachRight(gaussianMaxK,false);
   horiz7b->AttachRight(lab37,false);
   horiz7b->AttachRight(gaussianIters,false);            
   

   augF = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(augF,false);
   augF->Visible(false);
   gui::Horizontal * horiz8 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   augF->SetChild(horiz8);
   augF->Set("Fisher Augmentation Parameters");
   augF->Expand(true);
      
   gui::Button * but5 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab30 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   but5->SetChild(lab30); lab30->Set("Load Calibration...");
   horiz8->AttachRight(but5,false);

   gui::Label * lab29 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lab29->Set(" Prob:");
   fisherProb = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   fisherProb->Set("0.75");
   fisherProb->SetSize(48,24);

   //gui::Label * lab29 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   //lab29->Set(" Range:");
   //fisherRange = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   //fisherRange->Set("4");
   //fisherRange->SetSize(48,24);
   
   gui::Label * lab32 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lab32->Set(" Min K:");
   fisherMin = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   fisherMin->Set("0.0");
   fisherMin->SetSize(48,24);
   
   gui::Label * lab31 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lab31->Set(" Max K:");
   fisherMax = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   fisherMax->Set("16.0");
   fisherMax->SetSize(48,24);

   //gui::Label * lab33 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   //lab33->Set(" Bias:");
   //fisherBias = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   //fisherBias->Set("4.0");
   //fisherBias->SetSize(48,24);
      
   //horiz8->AttachRight(lab29,false);
   //horiz8->AttachRight(fisherRange,false);
   horiz8->AttachRight(lab29,false);
   horiz8->AttachRight(fisherProb,false);
   horiz8->AttachRight(lab32,false);
   horiz8->AttachRight(fisherMin,false);
   horiz8->AttachRight(lab31,false);
   horiz8->AttachRight(fisherMax,false);
   //horiz8->AttachRight(lab33,false);
   //horiz8->AttachRight(fisherBias,false);
   

   gui::Horizontal * horiz2 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz2);

   gui::Panel * panel1 = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   gui::Panel * panel2 = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   horiz2->AttachRight(panel1);
   horiz2->AttachRight(panel2);

   left = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   right = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel1->SetChild(left);
   panel2->SetChild(right);

   left->SetSize(leftImage.Size(0),leftImage.Size(1));
   right->SetSize(rightImage.Size(0),rightImage.Size(1));


 // Handlers...
  win->OnDeath(MakeCB(this,&Stereopsis::Quit));

  left->OnResize(MakeCB(this,&Stereopsis::ResizeLeft));
  right->OnResize(MakeCB(this,&Stereopsis::ResizeRight));

  whichAlg->OnChange(MakeCB(this,&Stereopsis::ChangeAlg));
  whichPost->OnChange(MakeCB(this,&Stereopsis::ChangePost));
  augGaussian->OnChange(MakeCB(this,&Stereopsis::SwitchGaussian));
  augFisher->OnChange(MakeCB(this,&Stereopsis::SwitchFisher));

  but1->OnClick(MakeCB(this,&Stereopsis::LoadLeft));
  but2->OnClick(MakeCB(this,&Stereopsis::LoadRight));
  but3->OnClick(MakeCB(this,&Stereopsis::Run));
  but4->OnClick(MakeCB(this,&Stereopsis::SaveSVT));
  but5->OnClick(MakeCB(this,&Stereopsis::LoadCalibration));
  but6->OnClick(MakeCB(this,&Stereopsis::LoadSeg));
  but7->OnClick(MakeCB(this,&Stereopsis::LoadExisting));
}

Stereopsis::~Stereopsis()
{
 delete win;
 delete leftVar;
 delete rightVar;
 delete leftImg;
 delete rightImg;
 delete existing;
 delete result;
 delete segmentation;
}

void Stereopsis::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void Stereopsis::ResizeLeft(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  left->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(left->P().Width(),left->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (left->P().Width() - leftImage.Size(0))/2;
  nat32 sy = (left->P().Height() - leftImage.Size(1))/2;
  left->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(leftImage.Size(0),leftImage.Size(1))),bs::Pos(sx,sy),leftImage);

 // Update...
  left->Update();
}

void Stereopsis::ResizeRight(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  right->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(right->P().Width(),right->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (right->P().Width() - rightImage.Size(0))/2;
  nat32 sy = (right->P().Height() - rightImage.Size(1))/2;
  right->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(rightImage.Size(0),rightImage.Size(1))),bs::Pos(sx,sy),rightImage);

 // Update...
  right->Update();
}

void Stereopsis::LoadLeft(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Left Image","*.bmp,*.jpg,*.png,*.tif",fn))
 {
  // Load image into memory...
   cstr filename = fn.ToStr();
   delete leftImg;
   leftImg = filter::LoadImageRGB(cyclops.Core(),filename);
   mem::Free(filename);
   if (leftImg==null<svt::Var*>())
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load image");
    return;
   }

  // Create a mask for the image...
   bit maskIni = true;
   leftImg->Add("mask",maskIni);
   leftImg->Commit();

   svt::Field<bs::ColourRGB> floatImage(leftImg,"rgb");
   svt::Field<bit> floatMask(leftImg,"mask");
   for (nat32 y=0;y<floatMask.Size(1);y++)
   {
    for (nat32 x=0;x<floatMask.Size(0);x++)
    {
     if (math::Equal(floatImage.Get(x,y).r,real32(1.0))&&
         math::Equal(floatImage.Get(x,y).g,real32(0.0))&&
         math::Equal(floatImage.Get(x,y).b,real32(1.0)))
     {
      floatMask.Get(x,y) = false;
     }
    }
   }


  // Image loaded, but its not in a format suitable for fast display - convert...
   leftVar->Setup2D(leftImg->Size(0),leftImg->Size(1));
   leftVar->Commit();
   leftVar->ByName("rgb",leftImage);

   for (nat32 y=0;y<leftImage.Size(1);y++)
   {
    for (nat32 x=0;x<leftImage.Size(0);x++)
    {
     leftImage.Get(x,y) = floatImage.Get(x,y);
    }
   }


  // Refresh the display...
   left->SetSize(leftImage.Size(0),leftImage.Size(1));
   left->Redraw();

  // Remove any previous disparity map...
   delete result;
   result = null<svt::Var*>();
 }
}

void Stereopsis::LoadRight(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Right Image","*.bmp,*.jpg,*.png,*.tif",fn))
 {
  // Load image into memory...
   cstr filename = fn.ToStr();
   delete rightImg;
   rightImg = filter::LoadImageRGB(cyclops.Core(),filename);
   mem::Free(filename);
   if (rightImg==null<svt::Var*>())
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load image");
    return;
   }


  // Create a mask for the image...
   bit maskIni = true;
   rightImg->Add("mask",maskIni);
   rightImg->Commit();

   svt::Field<bs::ColourRGB> floatImage(rightImg,"rgb");
   svt::Field<bit> floatMask(rightImg,"mask");
   for (nat32 y=0;y<floatMask.Size(1);y++)
   {
    for (nat32 x=0;x<floatMask.Size(0);x++)
    {
     if (math::Equal(floatImage.Get(x,y).r,real32(1.0))&&
         math::Equal(floatImage.Get(x,y).g,real32(0.0))&&
         math::Equal(floatImage.Get(x,y).b,real32(1.0)))
     {
      floatMask.Get(x,y) = false;
     }
    }
   }


  // Image loaded, but its not in a format suitable for fast display - convert...
   rightVar->Setup2D(rightImg->Size(0),rightImg->Size(1));
   rightVar->Commit();
   rightVar->ByName("rgb",rightImage);

   for (nat32 y=0;y<rightImage.Size(1);y++)
   {
    for (nat32 x=0;x<rightImage.Size(0);x++)
    {
     rightImage.Get(x,y) = floatImage.Get(x,y);
    }
   }


  // Refresh the display...
   right->SetSize(rightImage.Size(0),rightImage.Size(1));
   right->Redraw();

  // Remove any previous disparity map...
   delete result;
   result = null<svt::Var*>();
 }
}

void Stereopsis::LoadCalibration(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Load Camera Pair","*.pcc",fn))
 {
  if (pair.Load(fn)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load pcc file");
  }
  pair.LeftToDefault();
 }
}

void Stereopsis::LoadSeg(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Segmentation File","*.seg",fn))
 {
  // Load svt, verify it is a segmentation...
   cstr filename = fn.ToStr();
   svt::Node * floatNode = svt::Load(cyclops.Core(),filename);
   mem::Free(filename);
   if (floatNode==null<svt::Node*>())
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load file.");
    delete floatNode;
    return;
   }

   if (str::Compare(typestring(*floatNode),"eos::svt::Var")!=0)
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"File is the wrong structure for a segmentation");
    delete floatNode;
    return;
   }
   svt::Var * floatVar = static_cast<svt::Var*>(floatNode);

   nat32 segInd;
   if ((floatVar->Dims()!=2)||
       (floatVar->GetIndex(cyclops.TT()("seg"),segInd)==false)||
       (floatVar->FieldType(segInd)!=cyclops.TT()("eos::nat32")))
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"File is the wrong format for a segmentation");
    delete floatVar;
    return;
   }

  // Store it...
   delete segmentation;
   segmentation = floatVar;
 }
}

void Stereopsis::LoadExisting(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Disparity","*.obf,*.dis",fn))
 {
  cstr filename = fn.ToStr();
   svt::Node * floatNode = svt::Load(cyclops.Core(),filename);
  mem::Free(filename);
  
  if (floatNode==null<svt::Node*>())
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load disparity");
   return;
  }
  
  if (str::Compare(typestring(*floatNode),"eos::svt::Var")!=0)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"File is the wrong structure for a disparity map");
   delete floatNode;
   return;
  }
  svt::Var * floatVar = static_cast<svt::Var*>(floatNode);

  nat32 dispInd;
  if ((floatVar->Dims()!=2)||
      (floatVar->GetIndex(cyclops.TT()("disp"),dispInd)==false)||
      (floatVar->FieldType(dispInd)!=cyclops.TT()("eos::real32")))
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"File is the wrong format for a disparity map");
   delete floatVar;
   return;
  }
  
  delete existing;
  existing = floatVar;
 }
}

void Stereopsis::ChangeAlg(gui::Base * obj,gui::Event * event)
{
 switch (whichAlg->Get())
 {
  case 0:
   but7->Visible(true);
   alg5->Visible(false);
   alg6->Visible(false);
   alg7->Visible(false);
  break;
  case 1:
   but7->Visible(false);
   alg5->Visible(true);
   alg6->Visible(false);
   alg7->Visible(false);
  break;
  case 2:
   but7->Visible(false);
   alg5->Visible(false);
   alg6->Visible(true);
   alg7->Visible(false);
  break;
  case 3:
   but7->Visible(false);
   alg5->Visible(false);
   alg6->Visible(false);
   alg7->Visible(true);
  break;
 }
}

void Stereopsis::ChangePost(gui::Base * obj,gui::Event * event)
{
 switch (whichPost->Get())
 {
  case 0:
   post2->Visible(false);
   post3a->Visible(false);
   post3b->Visible(false);
   post4->Visible(false);
  break;
  case 1:
   post2->Visible(true);
   post3a->Visible(false);
   post3b->Visible(false);
   post4->Visible(false);
  break;
  case 2:
   post2->Visible(false);
   post3a->Visible(true);
   post3b->Visible(true);
   post4->Visible(false);
  break;
  case 3:
   post2->Visible(false);
   post3a->Visible(false);
   post3b->Visible(false);
   post4->Visible(true);  
  break;
 }
 
 augG->Visible(augGaussian->Ticked()||(whichPost->Get()==1));
}

void Stereopsis::SwitchGaussian(gui::Base * obj,gui::Event * event)
{
 augG->Visible(augGaussian->Ticked()||(whichPost->Get()==1));
}

void Stereopsis::SwitchFisher(gui::Base * obj,gui::Event * event)
{
 augF->Visible(augFisher->Ticked());
}

void Stereopsis::Run(gui::Base * obj,gui::Event * event)
{
 if ((leftImg==null<svt::Var*>())||(rightImg==null<svt::Var*>()))
 {
  cyclops.App().MessageDialog(gui::App::MsgErr,"You are a slug.");
 }
 else
 {
  // Create luv fields for both images, as needed...
   if (!leftImg->Exists("luv"))
   {
    bs::ColourLuv luvIni(0.0,0.0,0.0);
    leftImg->Add("luv",luvIni);
    leftImg->Commit();
    filter::RGBtoLuv(leftImg);
   }

   if (!rightImg->Exists("luv"))
   {
    bs::ColourLuv luvIni(0.0,0.0,0.0);
    rightImg->Add("luv",luvIni);
    rightImg->Commit();
    filter::RGBtoLuv(rightImg);
   }


  // Create the result to extract into...
   bit aGaussian = augGaussian->Ticked();
   bit aFisher = augFisher->Ticked();
   if (aFisher) aGaussian = true; // Need Gaussian as input - might as well force it as storage is cheap.
  
   delete result;
   result = new svt::Var(cyclops.Core());
   result->Setup2D(leftImg->Size(0),leftImg->Size(1));
   real32 dispIni = 0.0;
   bit maskIni = true;
   math::Fisher fishIni;
   result->Add("disp",dispIni);
   result->Add("mask",maskIni);
   if (aGaussian) result->Add("sd",dispIni);
   if (aFisher) result->Add("fish",fishIni);
   result->Commit(false);

   svt::Field<real32> disp(result,"disp");
   svt::Field<bit> mask(result,"mask");
   svt::Field<real32> sd(result,"sd");
   svt::Field<math::Fisher> fish(result,"fish");


  // Prep progress bar...
   time::Progress * prog = cyclops.BeginProg();
   nat32 step = 0;
   nat32 steps = 0;
   switch (whichAlg->Get())
   {
    case 0: break; // Existing
    case 1: steps += 1; break; // Dynamic Programming
    case 2: steps += 1; break; // Belief Propagation
    case 3: steps += 1; break; // Diffusion Correlation
   }
   switch (whichPost->Get())
   {
    case 1: steps += 2; break; // Smoothing. (Has sd fitting step.)
    case 2: steps += 1; break; // Plane fit + Seg
    case 3: steps += 1; break; // Poly fitting
   }
   
   if (aGaussian) steps += 1;
   if (aFisher) steps += 1;


  // Run the algorithm...
   stereo::DSC * dsc = null<stereo::DSC*>();
   stereo::DSI * dsi = null<stereo::DSI*>();

   svt::Field<bs::ColourLuv> leftLuv(leftImg,"luv");
   svt::Field<bs::ColourLuv> rightLuv(rightImg,"luv");
   svt::Field<bit> leftMask(leftImg,"mask");
   svt::Field<bit> rightMask(rightImg,"mask");

   switch (whichAlg->Get())
   {
    case 0: // Existing...
    {     
     // Create a dsi to expose the existing disparity map correctly...
      svt::Field<real32> dispEx(existing,"disp");
      svt::Field<bit> maskEx(existing,"mask");
     
      dsi = new stereo::DummyDSI(dispEx,&maskEx);
    }
    break;
    case 1: // Hierachical DP...
    {
     prog->Report(step++,steps);
     
     stereo::SparseDSI2 * sdsi = new stereo::SparseDSI2();
     dsi = sdsi;

     sdsi->Set(occCost->GetReal(1.0),vertCost->GetReal(0.0),
              vertMult->GetReal(0.33),errLim->GetInt(1));
     dsc = new stereo::HalfBoundLuvDSC(leftLuv,rightLuv,1.0,matchLim->GetReal(10.0));
     sdsi->Set(dsc);
     sdsi->Set(leftMask,rightMask);

     sdsi->Run(prog);
    }
    break;
    case 2: // Hierachical BP...
    {
     prog->Report(step++,steps);
     
     stereo::HEBP * sdsi = new stereo::HEBP();
     dsi = sdsi;

     real32 costCap = bpOccLim->GetReal(6.0);
     real32 occCostBase = bpOccCostHigh->GetReal(16.0);
     real32 occCostMult = (bpOccCostLow->GetReal(8.0)-occCostBase) / costCap;

     sdsi->Set(occCostBase,occCostMult,bpOccLimMult->GetReal(2.0),bpIters->GetInt(6),bpOutput->GetInt(1));
     dsc = new stereo::SqrBoundLuvDSC(leftLuv,rightLuv,1.0,bpMatchLim->GetReal(36.0));
     sdsi->Set(dsc);
     stereo::LuvDSC dscOcc(leftLuv,rightLuv,1.0,costCap);
     sdsi->SetOcc(&dscOcc);
     sdsi->Set(leftMask,rightMask);

     sdsi->Run(prog);
    }
    break;
    case 3: // Diffusion correlation
    {
     prog->Report(step++,steps);
     
     stereo::DiffCorrStereo * dcs = new stereo::DiffCorrStereo();
     dsi = dcs;
     
     dcs->SetImages(leftLuv,rightLuv);
     dcs->SetMasks(leftMask,rightMask);
     
     bit useHalfX = dcUseHalfX->Ticked();
     bit useHalfY = dcUseHalfY->Ticked();
     bit useCorners = dcUseCorners->Ticked();
     bit halfHeight = dcHalfHeight->Ticked();
     real32 distMult = dcDistMult->GetReal(0.1);
     nat32 diffSteps = dcDiffSteps->GetInt(5);
     nat32 minimaLimit = dcMinimaLimit->GetInt(8);
     real32 baseDistCap = dcBaseDistCap->GetReal(4.0);
     real32 distCapMult = dcDistCapMult->GetReal(2.0);
     real32 distCapThreshold = dcDistCapThreshold->GetReal(0.5);
     nat32 dispRange = dcDispRange->GetInt(2);
     bit doLR = dcDoLR->Ticked();
     real32 distCapDifference = dcDistCapDifference->GetReal(0.25);
     
     dcs->SetPyramid(useHalfX,useHalfY,useCorners,halfHeight);
     dcs->SetDiff(distMult,diffSteps);
     dcs->SetCorr(minimaLimit,baseDistCap,distCapMult,distCapThreshold,dispRange);
     dcs->SetRefine(doLR,distCapDifference);
     
     dcs->Run(prog);
    }
    break;
   }


  // Run the post-proccessor...
   switch (whichPost->Get())
   {
    case 0: // None - simply select the best...
    {
     for (nat32 y=0;y<disp.Size(1);y++)
     {
      for (nat32 x=0;x<disp.Size(0);x++)
      {
       if (dsi->Size(x,y)!=0)
       {
        real32 bestCost = dsi->Cost(x,y,0);
        disp.Get(x,y) = dsi->Disp(x,y,0);
        for (nat32 i=1;i<dsi->Size(x,y);i++)
        {
         if (dsi->Cost(x,y,i)<bestCost)
         {
          bestCost = dsi->Cost(x,y,i);
          disp.Get(x,y) = dsi->Disp(x,y,i);
         }
        }
        mask.Get(x,y) = true;
       }
       else
       {
        disp.Get(x,y) = 0.0;
        mask.Get(x,y) = false;
       }
      }
     }
    }
    break;
    case 1: // Smoothing...
    {
     prog->Report(step++,steps);
     
     // Extract a disparity map...
      for (nat32 y=0;y<disp.Size(1);y++)
      {
       for (nat32 x=0;x<disp.Size(0);x++)
       {
        if (dsi->Size(x,y)!=0)
        {
         real32 bestCost = dsi->Cost(x,y,0);
         disp.Get(x,y) = dsi->Disp(x,y,0);
         for (nat32 i=1;i<dsi->Size(x,y);i++)
         {
          if (dsi->Cost(x,y,i)<bestCost)
          {
           bestCost = dsi->Cost(x,y,i);
           disp.Get(x,y) = dsi->Disp(x,y,i);
          }
         }
         mask.Get(x,y) = true;
        }
        else
        {
         disp.Get(x,y) = 0.0;
         mask.Get(x,y) = false;
        }
       }
      }

     // Create tempory standard deviation storage...
      svt::Var sdTemp(leftLuv);
      {
       real32 realIni = 0.0;
       sdTemp.Add("sd",realIni);
       sdTemp.Commit();
      }
      svt::Field<real32> sdOR(&sdTemp,"sd");


     // Calculate standard deviations for the disparity values...
      stereo::LuvDSC luvDSC(leftLuv,rightLuv);
      stereo::RegionDSC regionDSC(&luvDSC,gaussianRadius->GetInt(4),gaussianFalloff->GetReal(0.5));

      fit::DispNorm dispNorm;
      dispNorm.Set(disp,regionDSC,gaussianMult->GetReal(1.0));
      dispNorm.SetMask(leftMask);
      dispNorm.SetRange(gaussianRange->GetInt(20),gaussianSdMult->GetReal(2.0));
      dispNorm.SetClampK(gaussianMinK->GetReal(2.5),gaussianMaxK->GetReal(10.0));
      dispNorm.SetClamp(gaussianMin->GetReal(0.1),gaussianMax->GetReal(10.0));
      dispNorm.SetMaxIters(gaussianIters->GetInt(1000));
     
      dispNorm.Run(prog);
    
      dispNorm.Get(sdOR);


     // Smooth it...
      prog->Report(step++,steps);
      stereo::CleanDSI cdsi;
      cdsi.Set(leftLuv);
      cdsi.Set(*dsi);
      cdsi.SetMask(leftMask);
      cdsi.SetSD(sdOR);
      cdsi.Set(smoothStrength->GetReal(16.0),smoothCutoff->GetReal(16.0),
               smoothWidth->GetReal(2.0),smoothIters->GetInt(256));

      cdsi.Run(prog);

      cdsi.GetMap(disp);
     
      for (nat32 y=0;y<disp.Size(1);y++)
      {
       for (nat32 x=0;x<disp.Size(0);x++)
       {
        mask.Get(x,y) = math::IsFinite(disp.Get(x,y))&&leftMask.Get(x,y);
       }
      }
    }
    break;
    case 2: // Plane fitting...
    {
     prog->Report(step++,steps);
     // First fill in the disparity map and mask from the result, so we can pass
     // them to the Bleyer04 algorithm as overrides...
      for (nat32 y=0;y<disp.Size(1);y++)
      {
       for (nat32 x=0;x<disp.Size(0);x++)
       {
        if (dsi->Size(x,y)!=0)
        {
         real32 bestCost = dsi->Cost(x,y,0);
         disp.Get(x,y) = dsi->Disp(x,y,0);
         for (nat32 i=1;i<dsi->Size(x,y);i++)
         {
          if (dsi->Cost(x,y,i)<bestCost)
          {
           bestCost = dsi->Cost(x,y,i);
           disp.Get(x,y) = dsi->Disp(x,y,i);
          }
         }
         mask.Get(x,y) = true;
        }
        else
        {
         disp.Get(x,y) = 0.0;
         mask.Get(x,y) = false;
        }
       }
      }

     // Create the Bleyers04 object, set parameters...
      svt::Field<bs::ColourRGB> leftRGB(leftImg,"rgb");
      svt::Field<bs::ColourRGB> rightRGB(rightImg,"rgb");

      stereo::Bleyer04 bleyer;
      bleyer.SetImages(leftRGB,rightRGB);
      bleyer.BootOverride(disp,mask);
      bleyer.SetMasks(leftMask,rightMask);
      bleyer.SetPlaneRadius(planeRadius->GetReal(0.6));
      bleyer.SetWarpCost(planeOcc->GetReal(0.078),planeDisc->GetReal(0.01));
      bleyer.SetBailOut(planeBailOut->GetInt(12));
      bleyer.SetSegment(segSpatial->GetReal(7.0),segRange->GetReal(4.5),segMin->GetInt(20));
      bleyer.SetSegmentExtra(segRad->GetInt(2),segMix->GetReal(0.3),segEdge->GetReal(0.9));
      
      if ((segmentation!=null<svt::Var*>())&&
          (segmentation->Size(0)==leftImg->Size(0))&&
          (segmentation->Size(0)==leftImg->Size(1)))
      {
       svt::Field<nat32> segs(segmentation,"seg");
       bleyer.SegmentOverride(segs);
      }

     // Run...
      bleyer.Run(prog);

     // Extract the results...
      bleyer.GetDisparity(disp);
      mask.CopyFrom(leftMask);
    }
    break;
    case 3: // Polynomial fitting...
    {
     prog->Report(step++,steps);

     // First run the selection of best code, but drop any where there is more than 1 match...
      for (nat32 y=0;y<disp.Size(1);y++)
      {
       for (nat32 x=0;x<disp.Size(0);x++)
       {
        if (dsi->Size(x,y)==1)
        {
         real32 bestCost = dsi->Cost(x,y,0);
         disp.Get(x,y) = dsi->Disp(x,y,0);
         for (nat32 i=1;i<dsi->Size(x,y);i++)
         {
          if (dsi->Cost(x,y,i)<bestCost)
          {
           bestCost = dsi->Cost(x,y,i);
           disp.Get(x,y) = dsi->Disp(x,y,i);
          }
         }
         mask.Get(x,y) = true;
        }
        else
        {
         disp.Get(x,y) = 0.0;
         mask.Get(x,y) = false;
        }
       }
      }
      
     // Now do the refinement...
      bit useHalfX = polyUseHalfX->Ticked();
      bit useHalfY = polyUseHalfY->Ticked();
      bit useCorners = polyUseCorners->Ticked();
      real32 distMult = polyDistMult->GetReal(0.01);
      nat32 diffSteps = polyDiffSteps->GetInt(7);
      real32 distCap = polyDistCap->GetReal(64.0);
      real32 prune = polyPrune->GetReal(0.25);
  
      stereo::DiffCorrRefine dcr;
      dcr.SetImages(leftLuv,rightLuv);
      dcr.SetMasks(leftMask,rightMask);
      dcr.SetDisparity(disp);
      dcr.SetDisparityMask(mask);
      dcr.SetFlags(useHalfX,useHalfY,useCorners);
      dcr.SetDiff(distMult,diffSteps);
      dcr.SetDist(distCap,prune);
      
      dcr.Run(prog);
      
      dcr.GetDisp(disp);
      dcr.GetMask(mask);
    }
    break;
   }
   
   
  // If needed augment with standard deviations...
   if (aGaussian)
   {
    prog->Report(step++,steps);
    
    stereo::LuvDSC luvDSC(leftLuv,rightLuv);
    stereo::RegionDSC regionDSC(&luvDSC,gaussianRadius->GetInt(4),gaussianFalloff->GetReal(0.5));
    //stereo::LuvRegionDSC regionDSC(leftLuv,rightLuv,&luvDSC,gaussianRadius->GetInt(4),0.1,gaussianFalloff->GetReal(0.5));
    
    fit::DispNorm dispNorm;
    dispNorm.Set(disp,regionDSC,gaussianMult->GetReal(1.0));
    dispNorm.SetMask(leftMask);
    dispNorm.SetRange(gaussianRange->GetInt(20),gaussianSdMult->GetReal(2.0));
    dispNorm.SetClampK(gaussianMinK->GetReal(2.5),gaussianMaxK->GetReal(10.0));
    dispNorm.SetClamp(gaussianMin->GetReal(0.1),gaussianMax->GetReal(10.0));
    dispNorm.SetMaxIters(gaussianIters->GetInt(1000));
    
    dispNorm.Run(prog);
    
    dispNorm.Get(sd);
   }
  

  // If needed augment with Fisher distributions...
   if (aFisher)
   {
    prog->Report(step++,steps);
    
    /*stereo::LuvDSC luvDSC(leftLuv,rightLuv);
    
    fit::DispFish dispFish;
    dispFish.Set(disp,luvDSC);
    dispFish.SetMask(leftMask);
    dispFish.SetPair(pair);
    dispFish.SetRange(fisherRange->GetInt(4));
    dispFish.SetBias(fisherBias->GetReal(0.0));
    dispFish.SetClamp(fisherMin->GetReal(0.0),fisherMax->GetReal(16.0));
   
    dispFish.Run(prog);
    
    dispFish.Get(fish);*/
    
    fit::DispNormFish dnf;
    dnf.Set(disp,sd);
    dnf.SetMask(leftMask);
    dnf.SetPair(pair);
    dnf.SetRegion(fisherProb->GetReal(0.1));
    dnf.SetRange(fisherMin->GetReal(0.0),fisherMax->GetReal(16.0));
    
    dnf.Run(prog);
    
    dnf.Get(fish);
   }


  // Clean up...
   delete dsc;
   delete dsi;
   cyclops.EndProg();


  // Update the visualisation of the left image, so as to represent the disparity map...
   real32 minDisp = 0.0;
   real32 maxDisp = 0.0;
   for (nat32 y=0;y<disp.Size(1);y++)
   {
    for (nat32 x=0;x<disp.Size(0);x++)
    {
     if (mask.Get(x,y))
     {
      minDisp = math::Min(minDisp,disp.Get(x,y));
      maxDisp = math::Max(maxDisp,disp.Get(x,y));
     }
    }
   }

   for (nat32 y=0;y<leftImage.Size(1);y++)
   {
    for (nat32 x=0;x<leftImage.Size(0);x++)
    {
     bs::ColRGB & targ = leftImage.Get(x,y);
     if (mask.Get(x,y))
     {
      real32 rxc = real32(x) + disp.Get(x,y);
      targ.r = byte(math::Clamp<real32>(255.0*(disp.Get(x,y)-minDisp)/(maxDisp-minDisp),0,255));
      if ((rxc>=0.0)&&(rxc<rightImage.Size(0)))
      {
       targ.g = targ.r;
       targ.b = targ.r;
      }
      else
      {
       targ.g = 127;
       targ.b = 0;
      }
     }
     else
     {
      targ.r = 0;
      targ.g = 0;
      targ.b = 255;
     }
    }
   }

   left->Redraw();
 }
}

void Stereopsis::SaveSVT(gui::Base * obj,gui::Event * event)
{
 if (result)
 {
  // Get the filename...
   str::String fn("");
   if (cyclops.App().SaveFileDialog("Save Disparity Map",fn))
   {
    if (!fn.EndsWith(".dis")) fn << ".dis";
    cstr ts = fn.ToStr();
    if (!svt::Save(ts,result,true))
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving .dis file.");
    }
    mem::Free(ts);
   }
 }
 else
 {
  cyclops.App().MessageDialog(gui::App::MsgErr,"You need to do stereopsis first!");
 }
}

//------------------------------------------------------------------------------
