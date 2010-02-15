//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "cyclops/light_est.h"

//------------------------------------------------------------------------------
LightEst::LightEst(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
inputVar(null<svt::Var*>()),imageVar(null<svt::Var*>()),imgVar(null<svt::Var*>())
{
 // Create default images maps...
  bs::ColourRGB colourIni(0.0,0.0,0.0);

  inputVar = new svt::Var(cyclops.Core());
  inputVar->Setup2D(320,240);
  inputVar->Add("rgb",colourIni);
  inputVar->Commit();
  inputVar->ByName("rgb",inputImage);
  segmentation.SetInvalid();

  imageVar = new svt::Var(cyclops.Core());
  imageVar->Setup2D(320,240);
  imageVar->Add("rgb",colourIni);
  imageVar->Commit();
  imageVar->ByName("rgb",image);
  
  imgVar = new svt::Var(cyclops.Core());
  imgVar->Setup2D(320,240);
  bs::ColRGB colIni(0,0,0);
  imgVar->Add("rgb",colIni);
  imgVar->Commit();
  imgVar->ByName("rgb",img);
  


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Segmenter");
  cyclops.App().Attach(win);
  win->SetSize(image.Size(0)+16,image.Size(1)+96);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);

   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);

   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but1->SetChild(lab1); lab1->Set("Load Image...");
   but2->SetChild(lab2); lab2->Set("Run");
   but3->SetChild(lab3); lab3->Set("Save Segmentation...");
   but4->SetChild(lab4); lab4->Set("Save View...");
   
   
   viewSelect = static_cast<gui::ComboBox*>(cyclops.Fact().Make("ComboBox"));
   viewSelect->Append("Input Image");
   viewSelect->Append("Random Colours");
   viewSelect->Append("Average Colours");
   viewSelect->Append("Outlines");
   viewSelect->Append("Random with Outlines");
   viewSelect->Append("Average with Outlines");
   viewSelect->Set(2);
   
   algSelect = static_cast<gui::ComboBox*>(cyclops.Fact().Make("ComboBox"));
   algSelect->Append("K Means Grid");
   algSelect->Append("Mean Shift");
   algSelect->Set(1);


   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(viewSelect,false);
   horiz1->AttachRight(algSelect,false);
   horiz1->AttachRight(but3,false);
   horiz1->AttachRight(but4,false);
   
   
   
   kMeanGrid = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(kMeanGrid,false);
   kMeanGrid->Visible(false);
   kMeanGrid->Set("K Means Grid Parameters");
   kMeanGrid->Expand(false);
   
   gui::Vertical * vert2 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));         
   gui::Horizontal * horiz2 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz3 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   kMeanGrid->SetChild(vert2);
   vert2->AttachBottom(horiz2,false);
   vert2->AttachBottom(horiz3,false);
   
   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab6 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab7 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab8 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab9 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   
   kmgDim = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   kmgMinSeg = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   kmgColMult = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   kmgSpatialMult = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   kmgMaxIters = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   lab5->Set("    Grid Size");
   lab6->Set(" Minimum Segment Size");
   lab7->Set("    Colour Mult");
   lab8->Set(" Spatial Mult");
   lab9->Set(" Max Iters");
   
   kmgDim->Set("12");
   kmgMinSeg->Set("32");
   kmgColMult->Set("2.0");
   kmgSpatialMult->Set("1.0");
   kmgMaxIters->Set("1000");
   
   kmgDim->SetSize(48,24);
   kmgMinSeg->SetSize(48,24);
   kmgColMult->SetSize(48,24);
   kmgSpatialMult->SetSize(48,24);
   kmgMaxIters->SetSize(64,24);
   
   horiz2->AttachRight(lab5,false);
   horiz2->AttachRight(kmgDim,false);
   horiz2->AttachRight(lab6,false);
   horiz2->AttachRight(kmgMinSeg,false);
   
   horiz3->AttachRight(lab7,false);
   horiz3->AttachRight(kmgColMult,false);
   horiz3->AttachRight(lab8,false);
   horiz3->AttachRight(kmgSpatialMult,false);
   horiz3->AttachRight(lab9,false);
   horiz3->AttachRight(kmgMaxIters,false);
   
   
   meanShift = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(meanShift,false);
   meanShift->Visible(true);
   meanShift->Set("Mean Shift Parameters");
   meanShift->Expand(false);
   
   gui::Vertical * vert3 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));         
   gui::Horizontal * horiz4 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz5 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz6 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   meanShift->SetChild(vert3);
   vert3->AttachBottom(horiz4,false);
   vert3->AttachBottom(horiz5,false);
   vert3->AttachBottom(horiz6,false);
   
   gui::Label * lab10 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab11 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab12 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab13 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab14 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab15 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab16 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab17 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   
   msEdgeWindow = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   msEdgeMix = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   msSpatialSize = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   msColourSize = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   msMergeCutoff = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   msMinSeg = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   msAvgSteps = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   msMergeMax = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   
   lab10->Set("    Edge Window");
   lab11->Set(" Edge Mix");
   lab12->Set("    Spatial Size");
   lab13->Set(" Colour Size");
   lab14->Set(" Merge Cutoff");
   lab15->Set("    Minimum Segment Size");
   lab16->Set(" Averaging Steps");
   lab17->Set(" Border Merge Max");
    
   msEdgeWindow->Set("2");
   msEdgeMix->Set("0.3");
   msSpatialSize->Set("7.0");
   msColourSize->Set("4.5");
   msMergeCutoff->Set("2.25");
   msMinSeg->Set("20");
   msAvgSteps->Set("2");
   msMergeMax->Set("0.9");
   
   msEdgeWindow->SetSize(48,24);
   msEdgeMix->SetSize(48,24);
   msSpatialSize->SetSize(48,24);
   msColourSize->SetSize(48,24);
   msMergeCutoff->SetSize(48,24);
   msMinSeg->SetSize(48,24);
   msAvgSteps->SetSize(48,24);
   msMergeMax->SetSize(48,24);
   
   horiz4->AttachRight(lab10,false);
   horiz4->AttachRight(msEdgeWindow,false);
   horiz4->AttachRight(lab11,false);
   horiz4->AttachRight(msEdgeMix,false);
   horiz4->AttachRight(lab17,false);
   horiz4->AttachRight(msMergeMax,false);
   
   horiz5->AttachRight(lab12,false);
   horiz5->AttachRight(msSpatialSize,false);
   horiz5->AttachRight(lab13,false);
   horiz5->AttachRight(msColourSize,false);
   horiz5->AttachRight(lab14,false);
   horiz5->AttachRight(msMergeCutoff,false);
   
   horiz6->AttachRight(lab15,false);
   horiz6->AttachRight(msMinSeg,false);
   horiz6->AttachRight(lab16,false);
   horiz6->AttachRight(msAvgSteps,false);
   
   
   
   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(image.Size(0),image.Size(1));


 // Event handlers...
  win->OnDeath(MakeCB(this,&LightEst::Quit));
  canvas->OnResize(MakeCB(this,&LightEst::Resize));
  but1->OnClick(MakeCB(this,&LightEst::LoadImage));
  but2->OnClick(MakeCB(this,&LightEst::Run));
  but3->OnClick(MakeCB(this,&LightEst::SaveSeg));
  but4->OnClick(MakeCB(this,&LightEst::SaveView));
  viewSelect->OnChange(MakeCB(this,&LightEst::ChangeView));
  algSelect->OnChange(MakeCB(this,&LightEst::ChangeAlg));
}

LightEst::~LightEst()
{
 delete win;
 delete inputVar;
 delete imageVar;
 delete imgVar;
}

void LightEst::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void LightEst::Resize(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  canvas->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(canvas->P().Width(),canvas->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (canvas->P().Width() - img.Size(0))/2;
  nat32 sy = (canvas->P().Height() - img.Size(1))/2;
  canvas->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(img.Size(0),img.Size(1))),bs::Pos(sx,sy),img);

 // Update...
  canvas->Update();
}

void LightEst::LoadImage(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Image","*.bmp,*.jpg,*.png,*.tif",fn))
 {
  // Load image into memory...
   cstr filename = fn.ToStr();
   svt::Var * tempVar = filter::LoadImageRGB(cyclops.Core(),filename);
   mem::Free(filename);
   if (tempVar==null<svt::Var*>())
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load image");
    return;
   }

  // Stick in right variables...
   delete inputVar;
   inputVar = tempVar;
   inputVar->ByName("rgb",inputImage);
   segmentation.SetInvalid();

  // Update the display...
   Update();
 }
}

void LightEst::Run(gui::Base * obj,gui::Event * event)
{
 // If needed add segmentation info to the image...
  if (!segmentation.Valid())
  {
   nat32 segIni = 0;
   inputVar->Add("seg",segIni);
   inputVar->Commit();
   inputVar->ByName("rgb",inputImage);
   inputVar->ByName("seg",segmentation);
  }


 // Run the selected algorithm...
 switch (algSelect->Get())
 {
  case 0: // K Means Grid
  {
   // Get parameters...
    nat32 dim = kmgDim->GetInt(12);
    nat32 minSeg = kmgMinSeg->GetInt(32);
    real32 colMult = kmgColMult->GetReal(2.0);
    real32 spatialMult = kmgSpatialMult->GetReal(1.0);
    nat32 maxIters = kmgMaxIters->GetInt(1000);
   
   // Get image in right colour space...
    svt::Var tempVar(inputImage);
    bs::ColourLuv luvIni(0.0,0.0,0.0);
    tempVar.Add("luv",luvIni);
    tempVar.Commit();
    svt::Field<bs::ColourLuv> luvImage(&tempVar,"luv");
    
    filter::RGBtoLuv(inputImage,luvImage);
   
   // Create and fill in the algorithm object...
    filter::MeanGridSeg mgs;
    mgs.SetImage(luvImage);
    mgs.SetSize(dim);
    mgs.SetMinSeg(minSeg);
    mgs.SetDist(colMult,spatialMult);
    mgs.SetMaxIters(maxIters);
  
   // Run algorithm... 
    mgs.Run(cyclops.BeginProg());
    cyclops.EndProg();
  
   // Extract result...
    mgs.GetSegments(segmentation);
  }
  break;
  case 1: // Mean Shift
  {
   // Get parameters...
    nat32 edgeWindow = msEdgeWindow->GetInt(2);
    real32 edgeMix = msEdgeMix->GetReal(0.3);
    real32 spatialSize = msSpatialSize->GetReal(7.0);
    real32 colourSize = msColourSize->GetReal(4.5);
    real32 mergeCutoff = msMergeCutoff->GetReal(2.25);
    nat32 minSeg = msMinSeg->GetInt(20);
    nat32 avgSteps = msAvgSteps->GetInt(2);
    real32 mergeMax = msMergeMax->GetReal(0.9);
  
   // Get image in right colour space...
    svt::Var tempVar(inputImage);
    bs::ColourLuv luvIni(0.0,0.0,0.0);
    bs::ColourL lIni(0.0);
    tempVar.Add("luv",luvIni);
    tempVar.Add("l",lIni);
    tempVar.Commit();
    svt::Field<bs::ColourLuv> luvImage(&tempVar,"luv");
    svt::Field<bs::ColourL> lImage(&tempVar,"l");
    
    filter::RGBtoLuv(inputImage,luvImage);
    filter::RGBtoL(inputImage,lImage);
    
   // Create and fill in the algorithm object...
    filter::Synergism ms;
    ms.SetImage(lImage,luvImage);
    ms.DiffWindow(edgeWindow);
    ms.SetMix(edgeMix);
    ms.SetSpatial(spatialSize);
    ms.SetRange(colourSize);
    ms.SetCutoff(mergeCutoff);
    ms.SetSegmentMin(minSeg);
    ms.SetAverageSteps(avgSteps);
    ms.SetMergeMax(mergeMax);
   
   // Run algorithm... 
    ms.Run(cyclops.BeginProg());
    cyclops.EndProg();
  
   // Extract result...
    ms.GetSegments(segmentation);
  }
  break;
 }
 
 
 // Update the visualisation...
  Update();
}

void LightEst::ChangeView(gui::Base * obj,gui::Event * event)
{
 Update();
}

void LightEst::ChangeAlg(gui::Base * obj,gui::Event * event)
{
 switch (algSelect->Get())
 {
  case 0:
   kMeanGrid->Visible(true);
   meanShift->Visible(false);
  break;
  case 1:
   kMeanGrid->Visible(false);
   meanShift->Visible(true);
  break;
 }
}

void LightEst::SaveView(gui::Base * obj,gui::Event * event)
{
 str::String fn("");
 if (cyclops.App().SaveFileDialog("Save Image",fn))
 {
  if (!(fn.EndsWith(".bmp")||fn.EndsWith(".jpg")||fn.EndsWith(".tga")||fn.EndsWith(".png"))) fn << ".bmp";
  cstr ts = fn.ToStr();
  if (!filter::SaveImage(image,ts,true))
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving image file.");
  }
  mem::Free(ts); 
 }
}

void LightEst::SaveSeg(gui::Base * obj,gui::Event * event)
{
 if (segmentation.Valid())
 {
  // Get the filename...
   str::String fn("");
   if (cyclops.App().SaveFileDialog("Save Segmentation",fn))
   {
    svt::Var tempVar(segmentation);
    nat32 segIni = 0;
    tempVar.Add("seg",segIni);
    tempVar.Commit();
    svt::Field<nat32> seg(&tempVar,"seg");
    
    for (nat32 y=0;y<seg.Size(1);y++)
    {
     for (nat32 x=0;x<seg.Size(0);x++) seg.Get(x,y) = segmentation.Get(x,y);
    }
   
    if (!fn.EndsWith(".seg")) fn << ".seg";
    cstr ts = fn.ToStr();
    if (!svt::Save(ts,&tempVar,true))
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving .seg file.");
    }
    mem::Free(ts);
   }
 }
 else
 {
  cyclops.App().MessageDialog(gui::App::MsgErr,"You need to do segmentation first!");
 }
}

void LightEst::Update()
{
 // Check sizes match, adjust if need be...
  if ((image.Size(0)!=inputImage.Size(0))||(image.Size(1)!=inputImage.Size(1)))
  {
   imageVar->Setup2D(inputImage.Size(0),inputImage.Size(1));
   imageVar->Commit();
   imageVar->ByName("rgb",image);
  }
  if ((img.Size(0)!=inputImage.Size(0))||(img.Size(1)!=inputImage.Size(1)))
  {
   imgVar->Setup2D(inputImage.Size(0),inputImage.Size(1));
   imgVar->Commit();
   imgVar->ByName("rgb",img);
  }

 
 // Switch on mode and render correct image...
  nat32 mode = viewSelect->Get();
  if (!segmentation.Valid()) mode = 0;
  switch(mode)
  {
   case 0: // Input image
   {
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++) image.Get(x,y) = inputImage.Get(x,y);
    }
   }
   break;
   case 1: // Random Colours
   {
    filter::RenderSegsColour(segmentation,image);
   }
   break;
   case 2: // Average Colours
   {
    filter::RenderSegsMean(segmentation,inputImage,image);
   }
   break;
   case 3: // Outlines
   {
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++) image.Get(x,y) = inputImage.Get(x,y);
    }
    
    filter::RenderSegsLines(segmentation,image,bs::ColourRGB(1.0,0.0,0.0));
   }
   break;
   case 4: // Random with Outlines
   {
    filter::RenderSegsColour(segmentation,image);
    
    filter::RenderSegsLines(segmentation,image,bs::ColourRGB(1.0,0.0,0.0));
   }
   break;
   case 5: // Average with Outlines
   {
    filter::RenderSegsMean(segmentation,inputImage,image);
    
    filter::RenderSegsLines(segmentation,image,bs::ColourRGB(1.0,0.0,0.0));
   }
   break;
  }


 // Copy from image to img...
  for (nat32 y=0;y<img.Size(1);y++)
  {
   for (nat32 x=0;x<img.Size(0);x++) img.Get(x,y) = image.Get(x,y);
  }


 // Redraw...
  canvas->SetSize(img.Size(0),img.Size(1));
  canvas->Redraw();
}

//------------------------------------------------------------------------------
