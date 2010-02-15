//------------------------------------------------------------------------------
// Copyright 2006 Tom Haines

#include "cyclops/stereopsis.h"

//------------------------------------------------------------------------------
Stereopsis::Stereopsis(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
leftVar(null<svt::Var*>()),rightVar(null<svt::Var*>()),
leftImg(null<svt::Var*>()),rightImg(null<svt::Var*>()),
result(null<svt::Var*>())
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
   whichAlg->Append("Hierarchical DP");
   whichAlg->Append("Hierarchical BP");
   //whichAlg->Append("Hierarchical CBP");
   whichAlg->Set(1);
   horiz6->AttachRight(whichAlg,false);

   gui::Label * lab5b = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lab5b->Set(" Post:");
   horiz6->AttachRight(lab5b,false);

   whichPost = static_cast<gui::ComboBox*>(cyclops.Fact().Make("ComboBox"));
   whichPost->Append("None");
   whichPost->Append("Smoothing");
   whichPost->Append("Seg & Plane Fit");
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

   occCost->Set("5.0");
   vertCost->Set("0.25");
   vertMult->Set("0.75");
   errLim->Set("1");
   matchLim->Set("10.0");



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

   bpOccCostHigh->Set("16.0");
   bpOccCostLow->Set("8.0");
   bpOccLimMult->Set("2.0");
   bpMatchLim->Set("36.0");
   bpOccLim->Set("6.0");
   bpIters->Set("6");
   bpOutput->Set("1");



   post2 = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(post2,false);
   gui::Vertical * vert3 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   post2->SetChild(vert3);
   post2->Set("Smoothing Parameters");
   post2->Expand(false);

   gui::Horizontal * horiz1d = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz1e = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert3->AttachBottom(horiz1d,false);
   vert3->AttachBottom(horiz1e,false);

   gui::Label * lab10 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab11 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab12 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab13 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   lab10->Set("    Strength:");
   lab11->Set("    Cutoff:");
   lab12->Set("Half-life:");
   lab13->Set("Iterations:");

   smoothStrength = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   smoothCutoff = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   smoothWidth = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   smoothIters = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   horiz1d->AttachRight(lab10,false);
   horiz1d->AttachRight(smoothStrength,false);
   horiz1d->AttachRight(lab13,false);
   horiz1d->AttachRight(smoothIters,false);
   horiz1e->AttachRight(lab11,false);
   horiz1e->AttachRight(smoothCutoff,false);
   horiz1e->AttachRight(lab12,false);
   horiz1e->AttachRight(smoothWidth,false);

   smoothStrength->Set("16.0");
   smoothCutoff->Set("16.0");
   smoothWidth->Set("2.0");
   smoothIters->Set("100");



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

   planeRadius->Set("0.6");
   planeBailOut->Set("12");
   planeOcc->Set("0.078");
   planeDisc->Set("0.01");



   post3b = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(post3b,false);
   post3b->Visible(false);
   gui::Vertical * vert5 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   post3b->SetChild(vert5);
   post3b->Set("Segmentation Parameters");
   post3b->Expand(false);

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

   segSpatial->Set("7.0");
   segRange->Set("4.5");
   segMin->Set("20");
   segRad->Set("2");
   segMix->Set("0.3");
   segEdge->Set("0.9");



   augG = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(augG,false);
   augG->Visible(false);
   gui::Horizontal * horiz7 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   augG->SetChild(horiz7);
   augG->Set("Gaussian Augmentation Parameters");
   augG->Expand(false);
   
   gui::Label * lab28 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   lab28->Set(" Range:");
   gaussianRange = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   gaussianRange->Set("5");
   
   horiz7->AttachRight(lab28,false);
   horiz7->AttachRight(gaussianRange,false);
   
   

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
   lab29->Set(" Range:");
   fisherRange = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   fisherRange->Set("4");
   fisherRange->SetSize(48,24);
   
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
   
   horiz8->AttachRight(lab29,false);
   horiz8->AttachRight(fisherRange,false);
   horiz8->AttachRight(lab32,false);
   horiz8->AttachRight(fisherMin,false);
   horiz8->AttachRight(lab31,false);
   horiz8->AttachRight(fisherMax,false);


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
}

Stereopsis::~Stereopsis()
{
 delete win;
 delete leftVar;
 delete rightVar;
 delete leftImg;
 delete rightImg;
 delete result;
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
 if (cyclops.App().LoadFileDialog("Load Camera Calibration","*.pcc",fn))
 {
  if (pair.Load(fn)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load pcc file");
  }
 }
}

void Stereopsis::ChangeAlg(gui::Base * obj,gui::Event * event)
{
 switch (whichAlg->Get())
 {
  case 0:
   alg5->Visible(true);
   alg6->Visible(false);
  break;
  case 1:
   alg5->Visible(false);
   alg6->Visible(true);
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
  break;
  case 1:
   post2->Visible(true);
   post3a->Visible(false);
   post3b->Visible(false);
  break;
  case 2:
   post2->Visible(false);
   post3a->Visible(true);
   post3b->Visible(true);
  break;
 }
}

void Stereopsis::SwitchGaussian(gui::Base * obj,gui::Event * event)
{
 augG->Visible(augGaussian->Ticked());
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
  
   delete result;
   result = new svt::Var(cyclops.Core());
   result->Setup2D(leftImg->Size(0),leftImg->Size(1));
   real32 dispIni = 0.0;
   bit maskIni = true;
   bs::Vert fishIni(0.0,0.0,0.0);
   result->Add("disp",dispIni);
   result->Add("mask",maskIni);
   if (aGaussian) result->Add("sd",dispIni);
   if (aFisher) result->Add("fish",fishIni);
   result->Commit(false);

   svt::Field<real32> disp(result,"disp");
   svt::Field<bit> mask(result,"mask");
   svt::Field<real32> sd(result,"sd");
   svt::Field<bs::Vert> fish(result,"fish");


  // Prep progress bar...
   time::Progress * prog = cyclops.BeginProg();
   nat32 step = 0;
   nat32 steps = 0;
   switch (whichAlg->Get())
   {
    case 0: steps += 1; break; // Dynamic Programming
    case 1: steps += 1; break; // Belief Propagation
   }
   switch (whichPost->Get())
   {
    case 1: steps += 1; break; // Smoothing.
    case 2: steps += 2; break; // Plane fit + Seg
   }
   
   if (aGaussian) steps += 1;
   if (aFisher) steps += 1;


  // Run the algorithm...
   prog->Report(step++,steps);
   stereo::DSC * dsc = null<stereo::DSC*>();
   stereo::DSI * dsi = null<stereo::DSI*>();

   svt::Field<bs::ColourLuv> leftLuv(leftImg,"luv");
   svt::Field<bs::ColourLuv> rightLuv(rightImg,"luv");
   svt::Field<bit> leftMask(leftImg,"mask");
   svt::Field<bit> rightMask(rightImg,"mask");

   switch (whichAlg->Get())
   {
    case 0: // Hierachical DP...
    {
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
    case 1: // Hierachical BP...
    {
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
     stereo::CleanDSI cdsi;
     cdsi.Set(leftLuv);
     cdsi.Set(*dsi);
     cdsi.Set(smoothStrength->GetReal(16.0),smoothCutoff->GetReal(16.0),
              smoothWidth->GetReal(2.0),smoothIters->GetInt(100));

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

     // Run...
      bleyer.Run(prog);

     // Extract the results...
      bleyer.GetDisparity(disp);
      mask.CopyFrom(leftMask);
    }
    break;
   }
   
   
  // If needed augment with standard deviations...
   if (aGaussian)
   {
    prog->Report(step++,steps);
    // ***************************************************************************
   }
  

  // If needed augment with Fisher distributions...
   if (aFisher)
   {
    prog->Report(step++,steps);
    
    stereo::LuvDSC luvDSC(leftLuv,rightLuv);
    
    fit::DispFish dispFish;
    dispFish.Set(disp,luvDSC,1.0);
    dispFish.SetMask(leftMask);
    dispFish.SetPair(pair);
    dispFish.SetRange(fisherRange->GetInt(4));
    dispFish.SetClamp(fisherMin->GetReal(0.0),fisherMax->GetReal(16.0));
   
    dispFish.Run(prog);
    
    dispFish.Get(fish);
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
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving svt file.");
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
