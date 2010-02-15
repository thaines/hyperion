//------------------------------------------------------------------------------
// Copyright 2008 Tom Haines

#include "cyclops/ambient_est.h"

//------------------------------------------------------------------------------
AmbientEst::AmbientEst(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
irrVar(null<svt::Var*>()),segVar(null<svt::Var*>()),dispVar(null<svt::Var*>()),
imageVar(null<svt::Var*>()),imgVar(null<svt::Var*>())
{
 // Create default images maps...
  bs::ColourRGB colourIni(0.0,0.0,0.0);
  nat32 segIni = 0;
  math::Fisher fishIni;

  irrVar = new svt::Var(cyclops.Core());
  irrVar->Setup2D(320,240);
  irrVar->Add("rgb",colourIni);
  irrVar->Commit();
  irrVar->ByName("rgb",irr);

  segVar = new svt::Var(cyclops.Core());
  segVar->Setup2D(320,240);
  segVar->Add("seg",segIni);
  segVar->Commit();
  segVar->ByName("seg",seg);

  dispVar = new svt::Var(cyclops.Core());
  dispVar->Setup2D(320,240);
  dispVar->Add("fish",fishIni);
  dispVar->Commit();
  dispVar->ByName("fish",fish);


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

 // Make default camera response function flat...
  crf.SetMult();


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Ambient Estimation");
  cyclops.App().Attach(win);
  win->SetSize(image.Size(0)+16,image.Size(1)+96);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);

   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);
   gui::Horizontal * horiz2 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz2,false);

   results = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   results2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   results->Set("Not yet run");
   vert1->AttachBottom(results,false);
   vert1->AttachBottom(results2,false);

   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but5 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but6 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab6 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   
   but1->SetChild(lab1); lab1->Set("Load Irradiance...");
   but2->SetChild(lab2); lab2->Set("Load Camera Response...");
   but3->SetChild(lab3); lab3->Set("Load Segmentation...");
   but4->SetChild(lab4); lab4->Set("Load Disparity...");
   but5->SetChild(lab5); lab5->Set("Run");
   but6->SetChild(lab6); lab6->Set("Save View...");   
   
   lightDirection = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   gui::Label * lab15 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   
   lightDirection->Set("(0.0,0.0,1.0)");
   lab15->Set(" Light Dir:");


   viewSelect = static_cast<gui::ComboBox*>(cyclops.Fact().Make("ComboBox"));
   viewSelect->Append("Irradiance");
   viewSelect->Append("Corrected Irradiance");
   viewSelect->Append("Segmentation");
   viewSelect->Append("Fisher Direction");
   viewSelect->Append("Fisher Concentration (log10)");
   viewSelect->Append("Linear Albedo");
   viewSelect->Append("Corrected Image");
   viewSelect->Append("Fully Corrected Image");
   viewSelect->Set(0);

   algSelect = static_cast<gui::ComboBox*>(cyclops.Fact().Make("ComboBox"));
   algSelect->Append("Haines & Wilson");
   algSelect->Set(0);


   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(but3,false);
   horiz1->AttachRight(but4,false);
   horiz1->AttachRight(lab15,false);
   horiz1->AttachRight(lightDirection,false);
   horiz2->AttachRight(viewSelect,false);
   horiz2->AttachRight(algSelect,false);
   horiz2->AttachRight(but5,false);
   horiz2->AttachRight(but6,false);



   brutalFish = static_cast<gui::Expander*>(cyclops.Fact().Make("Expander"));
   vert1->AttachBottom(brutalFish,false);
   brutalFish->Visible(true);
   brutalFish->Set("Haines & Wilson Parameters");
   brutalFish->Expand(false);

   gui::Vertical * vert2 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   gui::Horizontal * horiz3 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   gui::Horizontal * horiz4 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   brutalFish->SetChild(vert2);
   vert2->AttachBottom(horiz3,false);
   vert2->AttachBottom(horiz4,false);

   gui::Label * lab7 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab8 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab9 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab10 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab11 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab12 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab13 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab14 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   bfMinAmb = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   bfMaxAmb = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   bfMinAlb = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   bfMaxAlb = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   bfAmbRecursion = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));   
   bfAlbRecursion = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   bfPruneThresh = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   bfIrrErr = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   lab7->Set("  Minimum Ambient");
   lab8->Set(" Maximum Ambient");
   lab9->Set("  Minimum Albedo");
   lab10->Set(" Maximum Albedo");
   lab11->Set(" Ambient Recursion");
   lab12->Set(" Albedo Recursion");
   lab13->Set(" Pruning Threshold");
   lab14->Set(" Irradiance Error sd");
   

   bfMinAmb->Set("0.001");
   bfMaxAmb->Set("0.75");
   bfMinAlb->Set("0.001");
   bfMaxAlb->Set("1.5");
   bfAmbRecursion->Set("7");
   bfAlbRecursion->Set("7");
   bfPruneThresh->Set("0.2");
   bfIrrErr->Set("0.0078");

   bfMinAmb->SetSize(48,24);
   bfMaxAmb->SetSize(48,24);
   bfMinAlb->SetSize(48,24);
   bfMaxAlb->SetSize(48,24);
   bfAmbRecursion->SetSize(48,24);
   bfAlbRecursion->SetSize(48,24);
   bfPruneThresh->SetSize(48,24);
   bfIrrErr->SetSize(64,24);


   horiz3->AttachRight(lab7,false);
   horiz3->AttachRight(bfMinAmb,false);
   horiz3->AttachRight(lab8,false);
   horiz3->AttachRight(bfMaxAmb,false);
   horiz3->AttachRight(lab11,false);
   horiz3->AttachRight(bfAmbRecursion,false);
   horiz3->AttachRight(lab13,false);
   horiz3->AttachRight(bfPruneThresh,false);

   horiz4->AttachRight(lab9,false);
   horiz4->AttachRight(bfMinAlb,false);
   horiz4->AttachRight(lab10,false);
   horiz4->AttachRight(bfMaxAlb,false);
   horiz4->AttachRight(lab12,false);
   horiz4->AttachRight(bfAlbRecursion,false);
   horiz4->AttachRight(lab14,false);
   horiz4->AttachRight(bfIrrErr,false);



   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(image.Size(0),image.Size(1));


 // Event handlers...
  win->OnDeath(MakeCB(this,&AmbientEst::Quit));
  canvas->OnResize(MakeCB(this,&AmbientEst::Resize));
  but1->OnClick(MakeCB(this,&AmbientEst::LoadIrr));
  but2->OnClick(MakeCB(this,&AmbientEst::LoadCRF));
  but3->OnClick(MakeCB(this,&AmbientEst::LoadSeg));
  but4->OnClick(MakeCB(this,&AmbientEst::LoadDisp));
  but5->OnClick(MakeCB(this,&AmbientEst::Run));
  but6->OnClick(MakeCB(this,&AmbientEst::SaveView));
  viewSelect->OnChange(MakeCB(this,&AmbientEst::ChangeView));
  algSelect->OnChange(MakeCB(this,&AmbientEst::ChangeAlg));
}

AmbientEst::~AmbientEst()
{
 delete win;
 delete irrVar;
 delete segVar;
 delete dispVar;
 delete imageVar;
 delete imgVar;
}

void AmbientEst::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void AmbientEst::Resize(gui::Base * obj,gui::Event * event)
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

void AmbientEst::LoadIrr(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Irradiance Image","*.bmp,*.jpg,*.png,*.tif",fn))
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
   delete irrVar;
   irrVar = tempVar;
   irrVar->ByName("rgb",irr);

  // Update the display...
   Update();
 }
}

void AmbientEst::LoadCRF(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Camera Response Function","*.crf",fn))
 {
  // Load...
   if (crf.Load(fn)==false)
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load function");
   }

  // Update the display...
   Update();
 }
}

void AmbientEst::LoadSeg(gui::Base * obj,gui::Event * event)
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
   delete segVar;
   segVar = floatVar;
   segVar->ByName("seg",seg);

  // Update the display...
   Update();
 }
}
void AmbientEst::LoadDisp(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Disparity File","*.dis",fn))
 {
  // Load svt, verify it is a disparity map...
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

   nat32 fishInd;
   if ((floatVar->GetIndex(cyclops.TT()("fish"),fishInd)==false)||
       (floatVar->FieldType(fishInd)!=cyclops.TT()("eos::math::Fisher")))
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"File is not augmented with orientation information.");
    delete floatVar;
    return;
   }


  // Move the floatVar into the asReal array, deleting previous...
   delete dispVar;
   dispVar = floatVar;
   dispVar->ByName("fish",fish);


  // Refresh the display...
   Update();
 }
}

void AmbientEst::Run(gui::Base * obj,gui::Event * event)
{
 // Get the parameters...
  bs::Normal toLight;
  {
   str::String s = lightDirection->Get();
   str::String::Cursor cur = s.GetCursor();
   cur.ClearError();
   cur >> toLight;
   if (cur.Error()) toLight = bs::Normal(0.0,0.0,1.0);
  }
  
  real32 minAmb = bfMinAmb->GetReal(0.001);
  real32 maxAmb = bfMaxAmb->GetReal(0.75);
  real32 minAlb = bfMinAlb->GetReal(0.001);
  real32 maxAlb = bfMaxAlb->GetReal(1.5);
  nat32 ambRec = bfAmbRecursion->GetInt(7);
  nat32 albRec = bfAlbRecursion->GetInt(7);
  real32 irrErr = bfIrrErr->GetReal(0.0078);
  real32 pruneThresh = bfPruneThresh->GetReal(0.2);


 // Calculate the corrected irradiance...
  svt::Var tempVar(irr);
  real32 irrIni = 0.0;
  tempVar.Add("irr",irrIni);
  tempVar.Commit();
  svt::Field<real32> irradiance(&tempVar,"irr");

  for (nat32 y=0;y<irr.Size(1);y++)
  {
   for (nat32 x=0;x<irr.Size(0);x++)
   {
    irradiance.Get(x,y) = crf((irr.Get(x,y).r+irr.Get(x,y).g+irr.Get(x,y).b)/3.0);
   }
  }


 // Setup the algorithm object...
  fit::LightAmb la;
  la.SetData(seg,irradiance,fish);
  la.SetLightDir(toLight);
  la.SetAlbRange(minAlb,maxAlb);
  la.SetAmbRange(minAmb,maxAmb);
  la.SetIrrErr(irrErr);
  la.SetPruneThresh(pruneThresh);
  la.SetSubdivs(ambRec,albRec);
 

 // Run the algorithm...
  la.Run(cyclops.BeginProg());
  cyclops.EndProg();


 // Get the output...
  real32 bestAmb = la.BestAmb();


  albedo.Size(la.SegmentCount());
  real32 maxAlbedo = 0.0;
  for (nat32 i=0;i<albedo.Size();i++)
  {
   albedo[i] = la.SegmentAlbedo(i);
   maxAlbedo = math::Max(maxAlbedo,albedo[i]);
  }


 // Display the output strings...
  str::String s;
  s << "Best ambient = " << bestAmb;
  results->Set(s);
  
  str::String s2;
  s2 << "Highest albedo is " << maxAlbedo << ", which is " << crf.Inverse(maxAlbedo)
     << " in the uncorrected irradiance.";
  results2->Set(s2);


 // Update the view...
  Update();
}

void AmbientEst::ChangeView(gui::Base * obj,gui::Event * event)
{
 Update();
}

void AmbientEst::ChangeAlg(gui::Base * obj,gui::Event * event)
{
 // Noop.
}

void AmbientEst::SaveView(gui::Base * obj,gui::Event * event)
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

void AmbientEst::Update()
{
 /*
 // Find what size we need to be operating at...
  nat32 width=320, height=240;
  nat32 mode = viewSelect->Get();
  switch(mode)
  {
   case 0: // Irradiance
   case 1: // Corrected Irradiance
    width = irr.Size(0);
    height = irr.Size(1);
   break;
   case 5: // Light Source Sphere
   case 6: // Cost Sphere
    width = math::Max<nat32>(math::Min(irr.Size(0),irr.Size(1)),450);
    height = width;
   break;
   case 2: // Segmentation
   case 7: // Albedo
   case 8: // Per Pixel Solution Cost.
   case 9: // Segment cost.
    width = seg.Size(0);
    height = seg.Size(1);
   break;
   case 3: // Fisher Direction
   case 4: // Fisher Concentration
    width = fish.Size(0);
    height = fish.Size(1);
   break;
  }

 // Check sizes match, adjust if need be...
  if ((image.Size(0)!=width)||(image.Size(1)!=height))
  {
   imageVar->Setup2D(width,height);
   imageVar->Commit();
   imageVar->ByName("rgb",image);
  }
  if ((img.Size(0)!=width)||(img.Size(1)!=height))
  {
   imgVar->Setup2D(width,height);
   imgVar->Commit();
   imgVar->ByName("rgb",img);
  }


 // Switch on mode and render correct image...
  switch(mode)
  {
   case 0: // Irradiance
   {
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++)
     {
      real32 l = (irr.Get(x,y).r+irr.Get(x,y).g+irr.Get(x,y).b)/3.0;
      image.Get(x,y).r = l;
      image.Get(x,y).g = l;
      image.Get(x,y).b = l;
     }
    }
   }
   break;
   case 1: // Corrected Irradiance
   {
    real32 max = 0.01;
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++)
     {
      real32 l = crf((irr.Get(x,y).r+irr.Get(x,y).g+irr.Get(x,y).b)/3.0);
      max = math::Max(max,l);
      image.Get(x,y).r = l;
      image.Get(x,y).g = l;
      image.Get(x,y).b = l;
     }
    }

    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++) image.Get(x,y) /= max;
    }
   }
   break;
   case 2: // Segmentation
   {
    filter::RenderSegsMean(seg,irr,image);
    filter::RenderSegsLines(seg,image,bs::ColourRGB(1.0,0.0,0.0));
   }
   break;
   case 3: // Fisher Direction
   {
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++)
     {
      math::Fisher f = fish.Get(x,y);
      real32 len = f.Length();
      if (math::IsZero(len))
      {
       image.Get(x,y).r = 0.0;
       image.Get(x,y).g = 0.0;
       image.Get(x,y).b = 0.0;
      }
      else
      {
       f /= len;

       image.Get(x,y).r = (f[0]+1.0)*0.5;
       image.Get(x,y).g = (f[1]+1.0)*0.5;
       image.Get(x,y).b = f[2];
      }
     }
    }
   }
   break;
   case 4: // Fisher Concentration
   {
    real32 max = 0.001;
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++)
     {
      real32 k = math::Log10(1.0 + fish.Get(x,y).Length());
      max = math::Max(max,k);

      image.Get(x,y).r = k;
      image.Get(x,y).g = k;
      image.Get(x,y).b = k;
     }
    }

    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++) image.Get(x,y) /= max;
    }
   }
   break;
   case 5: // Light Source Sphere
   {
    real32 centX = image.Size(0)/2.0;
    real32 centY = image.Size(1)/2.0;
    real32 radius = centX*0.9;

    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++)
     {
      real32 dirX = (x-centX)/radius;
      real32 dirY = (y-centY)/radius;

      real32 l;
      if (math::Sqr(dirX)+math::Sqr(dirY)<1.0)
      {
       real32 dirZ = math::Sqrt(1.0 - math::Sqr(dirX) - math::Sqr(dirY));
       l = dirX*lightD[0] + dirY*lightD[1] + dirZ*lightD[2];
       l = math::Max<real32>(l,0.0);
      }
      else
      {
       l = 0.0;
      }

      image.Get(x,y).r = l;
      image.Get(x,y).g = l;
      image.Get(x,y).b = l;
     }
    }
   }
   break;
   case 6: // Cost Sphere
   {
    if (samples.Size()!=0)
    {
     // First generate a Delauney triangulation with all the samples in, with
     // costs as the stored data...
      ds::Delaunay2D<real32> doc;
      real32 minCost = math::Infinity<real32>();
      for (nat32 i=0;i<samples.Size();i++)
      {
       if ((samples[i].dir[2]>0.0)||(math::IsZero(samples[i].dir[2])))
       {
        doc.Add(samples[i].dir[0],samples[i].dir[1],samples[i].cost);
        minCost = math::Min(minCost,samples[i].cost);
       }
      }

     // Now iterate all the pixels, interpolating costs for those inside the
     // sphere and rendering the Log10 of the costs...
      real32 max = 0.001;
      real32 centX = image.Size(0)/2.0;
      real32 centY = image.Size(1)/2.0;
      real32 radius = centX*0.9;

      for (nat32 y=0;y<image.Size(1);y++)
      {
       for (nat32 x=0;x<image.Size(0);x++)
       {
        real32 dirX = (x-centX)/radius;
        real32 dirY = (y-centY)/radius;

        real32 l;
        if (math::Sqr(dirX)+math::Sqr(dirY)<1.0)
        {
         ds::Delaunay2D<real32>::Mid tri = doc.Triangle(dirX,dirY);
         nat32 pc = 0;
         real32 cost[3];
         real32 xp[3];
         real32 yp[3];
         for (nat32 i=0;i<3;i++)
         {
          if (!tri.Infinite(i))
          {
           ds::Delaunay2D<real32>::Pos pos = tri.GetPos(i);
           cost[pc] = *pos - minCost;
           xp[pc] = pos.X();
           yp[pc] = pos.Y();
           ++pc;
          }
         }

         switch(pc)
         {
          case 0:
           l = -1.0;
          break;
          case 1:
           l = cost[0];
          break;
          case 2:
          {
           real32 length = math::Sqrt(math::Sqr(xp[1]-xp[0]) + math::Sqr(yp[1]-yp[0]));
           real32 t = ((dirX-xp[0])*(xp[1]-xp[0]) + (dirY-yp[0])*(yp[1]-yp[0]))/math::Sqr(length);
           l = (1.0-t)*cost[0] + t*cost[1];
          }
          break;
          case 3:
          {
           // Below is using Barycentric Coordinates...
            real32 xC =  dirX-xp[2];
            real32 yC =  dirY-yp[2];
            real32 x1 = xp[0]-xp[2];
            real32 x2 = xp[1]-xp[2];
            real32 y1 = yp[0]-yp[2];
            real32 y2 = yp[1]-yp[2];

            real32 mult = 1.0/(x1*y2 - x2*y1);

            real32 lam1 = (y2*xC - x2*yC) * mult;
            real32 lam2 = (x1*yC - y1*xC) * mult;

            l = lam1*cost[0] + lam2*cost[1] + (1.0-lam1-lam2)*cost[2];
          }
          break;
         }
        }
        else
        {
         l = -1.0;
        }

        max = math::Max(max,l);
        image.Get(x,y).r = l;
        image.Get(x,y).g = l;
        image.Get(x,y).b = l;
       }
      }

     // Normalise...
      for (nat32 y=0;y<image.Size(1);y++)
      {
       for (nat32 x=0;x<image.Size(0);x++)
       {
        if (image.Get(x,y).r<0.0)
        {
         image.Get(x,y).r = 0.0;
         image.Get(x,y).g = 0.0;
         image.Get(x,y).b = 0.5;
        }
        else image.Get(x,y) /= max;
       }
      }
    }
   }
   break;
   case 7: // Albedo
   {
    real32 maxL = 0.001;
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++)
     {
      real32 l = -1.0;
      nat32 s = seg.Get(x,y);
      if (albedo.Size()>s) l = albedo[s];
      maxL = math::Max(maxL,l);

      image.Get(x,y).r = l;
      image.Get(x,y).g = l;
      image.Get(x,y).b = l;
     }
    }

    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++)
     {
      if (image.Get(x,y).r<0.0)
      {
       image.Get(x,y).r = 0.0;
       image.Get(x,y).g = 0.0;
       image.Get(x,y).b = 0.5;
      }
      else
      {
       image.Get(x,y) /= maxL;
      }
     }
    }
   }
   break;
   case 8: // Per Pixel Solution Cost
   {
    if (albedo.Size()!=0)
    {
     real32 maxL = 0.001;
     for (nat32 y=0;y<image.Size(1);y++)
     {
      for (nat32 x=0;x<image.Size(0);x++)
      {
       real32 i = crf((irr.Get(x,y).r+irr.Get(x,y).g+irr.Get(x,y).b)/3.0);
       real32 a = albedo[seg.Get(x,y)];

       real32 angI = math::InvCos(math::IsZero(a)?1.0:math::Min<real32>(i/a,1.0));
       real32 k = fish.Get(x,y).Length();
       real32 angS = math::IsZero(k)?angI:(math::InvCos(math::Min<real32>((lightD * fish.Get(x,y))/k,1.0)));

       real32 l = math::Log10(1.0 + -k*math::Cos(angI-angS) + k);

       image.Get(x,y).r = l;
       maxL = math::Max(maxL,l);
      }
     }

     for (nat32 y=0;y<image.Size(1);y++)
     {
      for (nat32 x=0;x<image.Size(0);x++)
      {
       real32 l = image.Get(x,y).r/maxL;

       image.Get(x,y).r = l;
       image.Get(x,y).g = l;
       image.Get(x,y).b = l;
      }
     }
    }
   }
   break;
   case 9: // Segment value
   {
    if ((seg.Size(0)==irr.Size(0))&&
        (seg.Size(1)==irr.Size(1))&&
        (seg.Size(0)==fish.Size(0))&&
        (seg.Size(1)==fish.Size(1)))
    {
     // First count the segments...
      nat32 segCount = 1;
      for (nat32 y=0;y<seg.Size(1);y++)
      {
       for (nat32 x=0;x<seg.Size(0);x++) segCount = math::Max(segCount,seg.Get(x,y)+1);
      }


     // Create array to hold expectation values for every segment...
      ds::Array<SegValue> segExp(segCount);
      for (nat32 s=0;s<segCount;s++)
      {
       segExp[s].div = 0.0;

       segExp[s].expI = 0.0;
       segExp[s].expX = 0.0;
       segExp[s].expY = 0.0;
       segExp[s].expZ = 0.0;

       segExp[s].expSqrI = 0.0;
       segExp[s].expSqrX = 0.0;
       segExp[s].expSqrY = 0.0;
       segExp[s].expSqrZ = 0.0;

       segExp[s].expIrrX = 0.0;
       segExp[s].expIrrY = 0.0;
       segExp[s].expIrrZ = 0.0;
      }


     // Now do a pass over the image and sum up the above for each segment...
      for (nat32 y=0;y<seg.Size(1);y++)
      {
       for (nat32 x=0;x<seg.Size(0);x++)
       {
        real32 l = fish.Get(x,y).Length();
        if (!math::IsZero(l))
        {
         real32 w = l;
         bs::Normal pos;
         for (nat32 i=0;i<3;i++) pos[i] = math::InvCos(fish.Get(x,y)[i]/l); // Makes 'em 0..pi
         nat32 s = seg.Get(x,y);
         real32 ir = crf((irr.Get(x,y).r+irr.Get(x,y).g+irr.Get(x,y).b)/3.0);


         segExp[s].div += w;

         segExp[s].expI += w*ir;
         segExp[s].expX += w*pos[0];
         segExp[s].expY += w*pos[1];
         segExp[s].expZ += w*pos[2];

         segExp[s].expSqrI += w*math::Sqr(ir);
         segExp[s].expSqrX += w*math::Sqr(pos[0]);
         segExp[s].expSqrY += w*math::Sqr(pos[1]);
         segExp[s].expSqrZ += w*math::Sqr(pos[2]);

         segExp[s].expIrrX += w*ir*pos[0];
         segExp[s].expIrrY += w*ir*pos[1];
         segExp[s].expIrrZ += w*ir*pos[2];
        }
       }
      }


     // Now calculate the 0..1 value for every segment...
      ds::Array<real32> segCost(segCount);
      for (nat32 s=0;s<segCount;s++)
      {
       if (!math::IsZero(segExp[s].div))
       {
        real32 sqrDivI = (segExp[s].expSqrI/segExp[s].div) - math::Sqr(segExp[s].expI/segExp[s].div);
        real32 sqrDivX = (segExp[s].expSqrX/segExp[s].div) - math::Sqr(segExp[s].expX/segExp[s].div);
        real32 sqrDivY = (segExp[s].expSqrY/segExp[s].div) - math::Sqr(segExp[s].expY/segExp[s].div);
        real32 sqrDivZ = (segExp[s].expSqrZ/segExp[s].div) - math::Sqr(segExp[s].expZ/segExp[s].div);

        if ((sqrDivI>0.0)&&(!math::IsZero(sqrDivI)))
        {
         real32 costX = 0.0,costY = 0.0,costZ = 0.0;

         if ((sqrDivX>0.0)&&(!math::IsZero(sqrDivX)))
         {
          costX = (segExp[s].expIrrX/segExp[s].div) - (segExp[s].expX*segExp[s].expI/math::Sqr(segExp[s].div));
          costX /= math::Sqrt(sqrDivX) * math::Sqrt(sqrDivI);
         }

         if ((sqrDivY>0.0)&&(!math::IsZero(sqrDivY)))
         {
          costY = (segExp[s].expIrrY/segExp[s].div) - (segExp[s].expY*segExp[s].expI/math::Sqr(segExp[s].div));
          costY /= math::Sqrt(sqrDivY) * math::Sqrt(sqrDivI);
         }

         if ((sqrDivZ>0.0)&&(!math::IsZero(sqrDivZ)))
         {
          costZ = (segExp[s].expIrrZ/segExp[s].div) - (segExp[s].expZ*segExp[s].expI/math::Sqr(segExp[s].div));
          costZ /= math::Sqrt(sqrDivZ) * math::Sqrt(sqrDivI);
         }

         segCost[s] = math::Sqrt(math::Sqr(costX) + math::Sqr(costY) + math::Sqr(costZ));
        }
        else segCost[s] = -1.0;
       }
       else segCost[s] = -1.0;
      }


     // And finally fill in the image with the values...
      real32 pruneThresh = bfPruneThresh->GetReal(0.2);
      for (nat32 y=0;y<image.Size(1);y++)
      {
       for (nat32 x=0;x<image.Size(0);x++)
       {
        image.Get(x,y).r = segCost[seg.Get(x,y)];
        if (image.Get(x,y).r<pruneThresh)
        {
         image.Get(x,y).r = 0.0;
         image.Get(x,y).g = 0.0;
         image.Get(x,y).b = 0.5;
        }
        else
        {
         image.Get(x,y).g = image.Get(x,y).r;
         image.Get(x,y).b = image.Get(x,y).r;
        }
       }
      }
    }
   }
   break;
  }


 // Copy from image to img...
  for (nat32 y=0;y<img.Size(1);y++)
  {
   for (nat32 x=0;x<img.Size(0);x++) img.Get(x,y) = image.Get(x,y);
  }
*/

 // Redraw...
  canvas->SetSize(img.Size(0),img.Size(1));
  canvas->Redraw();
}

//------------------------------------------------------------------------------
