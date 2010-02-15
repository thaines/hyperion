//------------------------------------------------------------------------------
// Copyright 2009 Tom Haines

#include "cyclops/albedo_est.h"

//------------------------------------------------------------------------------
AlbedoEst::AlbedoEst(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
irrVar(null<svt::Var*>()),segVar(null<svt::Var*>()),dispVar(null<svt::Var*>()),
imageVar(null<svt::Var*>()),imgVar(null<svt::Var*>())
{
 // Create default images maps...
  bs::ColourRGB colourIni(0.0,0.0,0.0);
  bs::ColRGB colIni(0,0,0);
  nat32 segIni = 0;
  real32 dispIni = 0.0;

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
  dispVar->Add("disp",dispIni);
  dispVar->Commit();
  dispVar->ByName("disp",disp);


  imageVar = new svt::Var(cyclops.Core());
  imageVar->Setup2D(320,240);
  imageVar->Add("rgb",colourIni);
  imageVar->Commit();
  imageVar->ByName("rgb",image);

  imgVar = new svt::Var(cyclops.Core());
  imgVar->Setup2D(320,240);
  imgVar->Add("rgb",colIni);
  imgVar->Commit();
  imgVar->ByName("rgb",img);


 // Make default camera setup...
  crf.SetMult();
  pair.SetDefault(320.0,240.0);


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Albedo Estimation");
  cyclops.App().Attach(win);
  win->SetSize(image.Size(0)+16,image.Size(1)+96);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);

   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);
   gui::Horizontal * horiz2 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz2,false);

   results = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   results->Set("Not yet run");
   vert1->AttachBottom(results,false);

   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but5 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but6 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but7 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab6 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab7 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   
   but1->SetChild(lab1); lab1->Set("Load Irradiance...");
   but2->SetChild(lab2); lab2->Set("Load Camera Response...");
   but3->SetChild(lab3); lab3->Set("Load Segmentation...");
   but4->SetChild(lab4); lab4->Set("Load Disparity...");
   but5->SetChild(lab5); lab5->Set("Load Camera Geometry...");
   but6->SetChild(lab6); lab6->Set("Run");
   but7->SetChild(lab7); lab7->Set("Save View...");
   
   lightDirection = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   albCap = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));
   gui::Label * lab8 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab9 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   
   lightDirection->Set("(0.0,0.0,1.0)");
   lightDirection->SetSize(96,24);
   albCap->Set("1.0");
   albCap->SetSize(32,24);
   lab8->Set(" Light Dir:");
   lab9->Set(" Albedo Cap:");


   viewSelect = static_cast<gui::ComboBox*>(cyclops.Fact().Make("ComboBox"));
   viewSelect->Append("Irradiance");
   viewSelect->Append("Corrected Irradiance");
   viewSelect->Append("Segmentation");
   viewSelect->Append("Disparity");
   viewSelect->Append("Linear Albedo");
   viewSelect->Append("Corrected Image");
   viewSelect->Append("Linear Corrected Image");
   viewSelect->Set(0);


   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(but3,false);
   horiz1->AttachRight(but4,false);
   horiz1->AttachRight(but5,false);
   
   horiz2->AttachRight(lab8,false);
   horiz2->AttachRight(lightDirection,false);
   horiz2->AttachRight(lab9,false);
   horiz2->AttachRight(albCap,false);
   horiz2->AttachRight(viewSelect,false);
   horiz2->AttachRight(but6,false);
   horiz2->AttachRight(but7,false);


   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(image.Size(0),image.Size(1));


 // Event handlers...
  win->OnDeath(MakeCB(this,&AlbedoEst::Quit));
  canvas->OnResize(MakeCB(this,&AlbedoEst::Resize));
  but1->OnClick(MakeCB(this,&AlbedoEst::LoadIrr));
  but2->OnClick(MakeCB(this,&AlbedoEst::LoadCRF));
  but3->OnClick(MakeCB(this,&AlbedoEst::LoadSeg));
  but4->OnClick(MakeCB(this,&AlbedoEst::LoadDisp));
  but5->OnClick(MakeCB(this,&AlbedoEst::LoadPair));
  but6->OnClick(MakeCB(this,&AlbedoEst::Run));
  but7->OnClick(MakeCB(this,&AlbedoEst::SaveView));
  viewSelect->OnChange(MakeCB(this,&AlbedoEst::ChangeView));
}

AlbedoEst::~AlbedoEst()
{
 delete win;
 delete irrVar;
 delete segVar;
 delete dispVar;
 delete imageVar;
 delete imgVar;
}

void AlbedoEst::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void AlbedoEst::Resize(gui::Base * obj,gui::Event * event)
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

void AlbedoEst::LoadIrr(gui::Base * obj,gui::Event * event)
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

void AlbedoEst::LoadCRF(gui::Base * obj,gui::Event * event)
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

void AlbedoEst::LoadSeg(gui::Base * obj,gui::Event * event)
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

void AlbedoEst::LoadDisp(gui::Base * obj,gui::Event * event)
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


  // Move the floatVar into the asReal array, deleting previous...
   delete dispVar;
   dispVar = floatVar;
   dispVar->ByName("disp",disp);
   dispVar->ByName("mask",dispMask);


  // Refresh the display...
   Update();
 }
}

void AlbedoEst::LoadPair(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Load Camera Calibration","*.pcc",fn))
 {
  if (pair.Load(fn)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load pcc file");
  }
 }
 pair.LeftToDefault();
}

void AlbedoEst::Run(gui::Base * obj,gui::Event * event)
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
  toLight.Normalise();
  
  real32 cap = crf(albCap->GetReal(1.0));


 // Setup some storage...
  svt::Var tempVar(irr);
  real32 irrIni = 0.0;
  tempVar.Add("irr",irrIni);
  bit maskIni = true;
  tempVar.Add("mask",maskIni);
  bs::Normal needleIni(0.0,0.0,0.0);
  tempVar.Add("needle",needleIni);
  tempVar.Commit();
  svt::Field<real32> irradiance(&tempVar,"irr");
  svt::Field<bit> mask(&tempVar,"mask");
  svt::Field<bs::Normal> needle(&tempVar,"needle");
  
  ds::Array2D< math::Vect<3,real32> > loc(irr.Size(0),irr.Size(1));


 // Calculate the corrected irradiance...
  for (nat32 y=0;y<irr.Size(1);y++)
  {
   for (nat32 x=0;x<irr.Size(0);x++)
   {
    irradiance.Get(x,y) = crf((irr.Get(x,y).r+irr.Get(x,y).g+irr.Get(x,y).b)/3.0);
   }
  }


 // The below code needs the disparity positions as locations in 3D space...
  for (nat32 y=0;y<disp.Size(1);y++)
  {
   for (nat32 x=0;x<disp.Size(0);x++)
   {
    pair.Triangulate(real32(x),real32(y),disp.Get(x,y),loc.Get(x,y));
   }
  }


 // Get a mask of usable pixels, calculate surface orientations at same time...
  for (nat32 y=0;y<disp.Size(1);y++)
  {
   for (nat32 x=0;x<disp.Size(0);x++)
   {
    if ((dispMask.Valid())&&(!dispMask.Get(x,y))) mask.Get(x,y) = false;
    
    bit useXP = (x+1<disp.Size(0))&&((!dispMask.Valid())||dispMask.Get(x+1,y));
    bit useYP = (y+1<disp.Size(1))&&((!dispMask.Valid())||dispMask.Get(x,y+1));
    bit useXN = (x>0)&&((!dispMask.Valid())||dispMask.Get(x-1,y));
    bit useYN = (y>0)&&((!dispMask.Valid())||dispMask.Get(x,y-1));
    
    if ((useXP==false)&&(useXN==false)) mask.Get(x,y) = false;
    if ((useYP==false)&&(useYN==false)) mask.Get(x,y) = false;
    
    if (mask.Get(x,y))
    {
     bs::Normal xDir;
     if (useXP)
     {
      xDir = loc.Get(x+1,y);
      xDir -= loc.Get(x,y);
     }
     else
     {
      xDir = loc.Get(x,y);
      xDir -= loc.Get(x-1,y);
     }
     
     bs::Normal yDir;
     if (useYP)
     {
      yDir = loc.Get(x,y+1);
      yDir -= loc.Get(x,y);
     }
     else
     {
      yDir = loc.Get(x,y);
      yDir -= loc.Get(x,y-1);
     }
     
     math::CrossProduct(xDir,yDir,needle.Get(x,y));
     needle.Get(x,y).Normalise();
    }
   }
  }
  
  for (nat32 y=0;y<disp.Size(1);y++)
  {
   for (nat32 x=0;x<disp.Size(0);x++)
   {
    real32 dot = needle.Get(x,y) * toLight;
    if ((dot<0.0)||(math::IsZero(dot))) mask.Get(x,y) = false;
    if (math::IsZero(irradiance.Get(x,y))) mask.Get(x,y) = false;
   }
  }


 // Count the number of segments and the size of each...
  nat32 segCount = 0;
  for (nat32 y=0;y<seg.Size(1);y++)
  {
   for (nat32 x=0;x<seg.Size(0);x++) segCount = math::Max(segCount,seg.Get(x,y)+1);
  }
  
  ds::Array<nat32> segSize(segCount);
  for (nat32 i=0;i<segCount;i++) segSize[i] = 0;
  
  for (nat32 y=0;y<seg.Size(1);y++)
  {
   for (nat32 x=0;x<seg.Size(0);x++)
   {
    if (mask.Get(x,y)) segSize[seg.Get(x,y)] += 1;
   }
  }
 
 
 // Iterate each segment, collect all estimates together and calculate the mode...
  time::Progress * prog = cyclops.BeginProg();
  albedo.Size(segSize.Size());
  prog->Push();
  for (nat32 s=0;s<segSize.Size();s++)
  {
   prog->Report(s,segSize.Size());
   if (segSize[s]>0)
   {
    ds::Array<WeightEst> estimate(segSize[s]);
    nat32 pos = 0;
    for (nat32 y=0;y<irradiance.Size(1);y++)
    {
     for (nat32 x=0;x<irradiance.Size(0);x++)
     {
      if (mask.Get(x,y)&&(seg.Get(x,y)==s))
      {
       real32 dot = needle.Get(x,y) * toLight;
       estimate[pos].estimate = irradiance.Get(x,y) / dot;
       estimate[pos].weight = dot;
       
       if ((!math::IsFinite(estimate[pos].estimate))||(!math::IsFinite(estimate[pos].weight)))
       {
        estimate[pos].estimate = 0.0;
        estimate[pos].weight = 0.0;
       }
       
       pos += 1;
      }
     }
    }   
    eos::log::Assert(pos==segSize[s]);
   
    estimate.SortNorm();
    
    real32 weightSum = 0.0;
    for (nat32 i=0;i<estimate.Size();i++) weightSum += estimate[i].weight;
    weightSum /= 2.0;
    
    nat32 end = 0;
    real32 endWeight = estimate[0].weight;
    while (endWeight<weightSum)
    {
     end += 1;
     endWeight += estimate[end].weight;
    }
    
    if (end!=0)
    {
     real32 w = (weightSum - endWeight + estimate[end].weight) / estimate[end].weight;
     albedo[s] = (1.0-w)*estimate[end-1].estimate + w*estimate[end].estimate;
    }
    else albedo[s] = estimate[end].estimate;
    albedo[s] = math::Min(albedo[s],cap);
   }
   else
   {
    albedo[s] = 0.0;
   }
  }
  prog->Pop();
  cyclops.EndProg();


 // Find the maximum albedo for output...
  real32 maxA = 0.0;
  for (nat32 i=0;i<albedo.Size();i++) maxA = math::Max(maxA,albedo[i]);


 // Display the output string...
  str::String s;
  s << "Highest albedo = " << maxA << ", non-linear albedo = " << crf.Inverse(maxA);
  results->Set(s);


 // Update the view...
  Update();
}

void AlbedoEst::ChangeView(gui::Base * obj,gui::Event * event)
{
 Update();
}

void AlbedoEst::SaveView(gui::Base * obj,gui::Event * event)
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

void AlbedoEst::Update()
{

 // Find what size we need to be operating at...
  nat32 width=320, height=240;
  nat32 mode = viewSelect->Get();
  switch(mode)
  {
   case 0: // Irradiance
   case 1: // Corrected Irradiance
   case 5: // Corrected Image
   case 6: // Linear corrected image
    width = irr.Size(0);
    height = irr.Size(1);
   break;
   case 2: // Segmentation
   case 4: // Linear Albedo
    width = seg.Size(0);
    height = seg.Size(1);
   break;
   case 3: // Disparity
    width = disp.Size(0);
    height = disp.Size(1);
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
   case 3: // Disparity
   {
    real32 minD =  math::Infinity<real32>();
    real32 maxD = -math::Infinity<real32>();
    for (nat32 y=0;y<disp.Size(1);y++)
    {
     for (nat32 x=0;x<disp.Size(0);x++)
     {
      if ((!dispMask.Valid())||(dispMask.Get(x,y)))
      {
       real32 d = disp.Get(x,y);
       minD = math::Min(minD,d);
       maxD = math::Max(maxD,d);
      }
     }
    }
    
    for (nat32 y=0;y<disp.Size(1);y++)
    {
     for (nat32 x=0;x<disp.Size(0);x++)
     {
      if ((!dispMask.Valid())||(dispMask.Get(x,y)))
      {
       real32 d = disp.Get(x,y);
       real32 l = (d-minD)/(maxD-minD);
       
       image.Get(x,y).r = l;
       image.Get(x,y).g = l;
       image.Get(x,y).b = l;
      }
      else
      {
       image.Get(x,y).r = 0.0;
       image.Get(x,y).g = 0.0;
       image.Get(x,y).b = 1.0;
      }
     }
    }
   }
   break;   
   case 4: // Linear Albedo
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
   case 5: // Corrected Image
   {
    real32 maxCha = 0.01;
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++)
     {
      real32 l = (irr.Get(x,y).r+irr.Get(x,y).g+irr.Get(x,y).b)/3.0;
      real32 cl = crf(l);
      if (!math::IsZero(albedo[seg.Get(x,y)])) cl /= albedo[seg.Get(x,y)];
      real32 ul = crf.Inverse(cl);
      ul /= l;
      if (math::IsFinite(ul))
      {
       image.Get(x,y).r *= ul;
       image.Get(x,y).g *= ul;
       image.Get(x,y).b *= ul;
       maxCha = math::Max(maxCha,image.Get(x,y).r,image.Get(x,y).g,image.Get(x,y).b);
      }
      else
      {
       image.Get(x,y).r = 0.0;
       image.Get(x,y).g = 0.0;
       image.Get(x,y).b = 0.0;
      }
     }
    }
    
    if (maxCha>1.0)
    {
     for (nat32 y=0;y<image.Size(1);y++)
     {
      for (nat32 x=0;x<image.Size(0);x++)
      {
       image.Get(x,y) /= maxCha;
      }
     }
    }
   }
   break;
   case 6: // Linear corrected Image
   {
    real32 max = 0.001;
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++)
     {
      real32 l = (irr.Get(x,y).r+irr.Get(x,y).g+irr.Get(x,y).b)/3.0;
      real32 cl = crf(l);
      real32 a = albedo[seg.Get(x,y)];
      if (!math::IsZero(a)) cl /= a;
      max = math::Max(max,cl);

      image.Get(x,y).r = cl;
      image.Get(x,y).g = cl;
      image.Get(x,y).b = cl;
     }
    }
    
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++) image.Get(x,y) /= max;
    }
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
