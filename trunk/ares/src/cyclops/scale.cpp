//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "cyclops/scale.h"

//------------------------------------------------------------------------------
Scale::Scale(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
imgVar(null<svt::Var*>()),imageVar(null<svt::Var*>())
{
 // Create default image...
  bs::ColRGB colIni(0,0,0);
  bs::ColourRGB colourIni(0.0,0.0,0.0);
  
  imgVar = new svt::Var(cyclops.Core());
  imgVar->Setup2D(320,240);
  imgVar->Add("rgb",colIni);
  imgVar->Commit();
  imgVar->ByName("rgb",img);

  imageVar = new svt::Var(cyclops.Core());
  imageVar->Setup2D(320,240);
  imageVar->Add("rgb",colourIni);
  imageVar->Commit();
  imageVar->ByName("rgb",image);


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Scale");
  cyclops.App().Attach(win);
  win->SetSize(image.Size(0)+8,image.Size(1)+48);

   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);

   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);

   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   scale = static_cast<gui::EditBox*>(cyclops.Fact().Make("EditBox"));

   but1->SetChild(lab1); lab1->Set("Load Image...");
   but2->SetChild(lab2); lab2->Set("Save Scaled Image...");
   but3->SetChild(lab3); lab3->Set("Update Intrinsic...");

   lab4->Set("New Height:");
   scale->SetSize(48,24);
   scale->Set("240");

   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(lab4,false);
   horiz1->AttachRight(scale,false);
   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(but3,false);


   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(image.Size(0),image.Size(1));


 // Event handlers...
  win->OnDeath(MakeCB(this,&Scale::Quit));
  canvas->OnResize(MakeCB(this,&Scale::Resize));

  but1->OnClick(MakeCB(this,&Scale::LoadImage));
  but2->OnClick(MakeCB(this,&Scale::SaveImage));
  but3->OnClick(MakeCB(this,&Scale::UpdateCalib));
}

Scale::~Scale()
{
 delete win;
 delete imgVar;
 delete imageVar;
}

void Scale::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void Scale::Resize(gui::Base * obj,gui::Event * event)
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

void Scale::LoadImage(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Image","*.bmp,*.jpg,*.png,*.tif",fn))
 {
  // Load image into memory...
   cstr filename = fn.ToStr();
   svt::Var * temp = filter::LoadImageRGB(cyclops.Core(),filename);
   mem::Free(filename);
   if (temp==null<svt::Var*>())
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load image");
    return;
   }
   delete imageVar;
   imageVar = temp;
   imageVar->ByName("rgb",image);
  
  // Image loaded, but its not in a format suitable for fast display - convert...
   imgVar->Setup2D(imageVar->Size(0),imageVar->Size(1));
   imgVar->Commit();
   imgVar->ByName("rgb",img);   
   
   for (nat32 y=0;y<img.Size(1);y++)
   {
    for (nat32 x=0;x<img.Size(0);x++)
    {
     img.Get(x,y) = image.Get(x,y);
    }
   }
   
  // Refresh the display...
   str::String ts;
   ts << image.Size(1);
   scale->Set(ts);
  
   canvas->SetSize(img.Size(0),img.Size(1));
   canvas->Redraw();
 }
}

void Scale::SaveImage(gui::Base * obj,gui::Event * event)
{
 // Ask the user for a filename...
  str::String fn("");
  if (cyclops.App().SaveFileDialog("Save Scaled Image",fn))
  {
   // Get the new dimensions...
    real32 factor = scale->Get().ToReal32();
    if (factor>1.0) factor /= real32(image.Size(1));

    int32 width  = int32(factor*image.Size(0));
    int32 height = int32(factor*image.Size(1));


   // Create an input mask, mask out pink regions...
    bit iniMask = true;
    
    svt::Var iMask(cyclops.Core());
    iMask.Setup2D(image.Size(0),image.Size(1));
    iMask.Add("mask",iniMask);
    iMask.Commit();
    svt::Field<bit> inMask(&iMask,"mask");
    
    for (nat32 y=0;y<image.Size(1);y++)
    {
     for (nat32 x=0;x<image.Size(0);x++)
     {
      inMask.Get(x,y) = !(math::Equal(image.Get(x,y).r,real32(1.0))&&
                          math::Equal(image.Get(x,y).g,real32(0.0))&&
                          math::Equal(image.Get(x,y).b,real32(1.0)));
     }
    }


   // Create output data structure, with mask...
    bs::ColourRGB iniRGB(0.0,0.0,0.0);
    
    svt::Var scaled(cyclops.Core());
    scaled.Setup2D(width,height);
    scaled.Add("rgb",iniRGB);
    scaled.Add("mask",iniMask);
    scaled.Commit();
    
    svt::Field<bs::ColourRGB> scaledCol(&scaled,"rgb");
    svt::Field<bit> scaledMask(&scaled,"mask");
   
   
   // Do the scaling, pink out un-masked areas...
    filter::ScaleExtend(image,inMask,scaledCol,scaledMask,factor);
    
    for (int32 y=0;y<height;y++)
    {
     for (int32 x=0;x<width;x++)
     {
      if (scaledMask.Get(x,y)==false) scaledCol.Get(x,y) = bs::ColourRGB(1.0,0.0,1.0);
     }
    }


   // Save it...
    if (!(fn.EndsWith(".bmp")||fn.EndsWith(".jpg")||fn.EndsWith(".tga")||fn.EndsWith(".png"))) fn << ".bmp";
    cstr ts = fn.ToStr();
    if (!filter::SaveImage(scaledCol,ts,true))
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving cropped image.");
    }
    mem::Free(ts);
  }
}

void Scale::UpdateCalib(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Intrinsic Calibration","*.icd",fn))
 {
  cam::CameraCalibration calib;
  if (calib.Load(fn)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load calibration file");
  }
  else
  {
   // Get the scaling factor adn new image size...
    real32 factor = scale->Get().ToReal32();
    if (factor>1.0) factor /= real32(image.Size(1));

    int32 width  = int32(factor*image.Size(0));
    int32 height = int32(factor*image.Size(1));

   // Update the camera calibration...
    calib.intrinsic.Scale(factor,factor);
    calib.radial.Scale(factor,factor);
    calib.dim[0] = width;
    calib.dim[1] = height;

   // Save back the file...
    if (cyclops.App().SaveFileDialog("Save Intrinsic Calibration...",fn))
    {
     if (!fn.EndsWith(".icd")) fn += ".icd";
     if (!calib.Save(fn,true))
     {
      cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving back the file.");
      return;	  
     }
    }
  }
 }
}

//------------------------------------------------------------------------------
