//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "cyclops/disp_mask.h"

//------------------------------------------------------------------------------
DispMask::DispMask(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),var(null<svt::Var*>())
{
 // Create default image and disparity...
  var = new svt::Var(cyclops.Core());
  var->Setup2D(320,240);
  bs::ColRGB colIni(0,0,255);
  var->Add("rgb",colIni);
  var->Commit();
  var->ByName("rgb",image);

  dispVar = new svt::Var(cyclops.Core());
  dispVar->Setup2D(320,240);
  real32 dispIni = 0.0;
  bit maskIni = false;
  bit userMaskIni = true;
  dispVar->Add("disp",dispIni);
  dispVar->Add("mask",maskIni);
  dispVar->Add("user_mask",userMaskIni);
  dispVar->Commit();
  dispVar->ByName("disp",disp);
  dispVar->ByName("mask",mask);
  dispVar->ByName("user_mask",userMask);

  RenderDisp();


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Disparity Masking");
  cyclops.App().Attach(win);
  win->SetSize(image.Size(0)+8,image.Size(1)+48);

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

   but1->SetChild(lab1); lab1->Set("Load Disparity...");
   but2->SetChild(lab2); lab2->Set("Save Mask...");
   but3->SetChild(lab3); lab3->Set("Load Mask...");
   but4->SetChild(lab4); lab4->Set("Save Disparity...");

   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but2,false);
   horiz1->AttachRight(but3,false);
   horiz1->AttachRight(but4,false);


   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);

   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);

   canvas->SetSize(image.Size(0),image.Size(1));


 // Event handlers...
  win->OnDeath(MakeCB(this,&DispMask::Quit));
  canvas->OnResize(MakeCB(this,&DispMask::Resize));

  but1->OnClick(MakeCB(this,&DispMask::LoadDisp));
  but2->OnClick(MakeCB(this,&DispMask::SaveMask));
  but3->OnClick(MakeCB(this,&DispMask::LoadMask));
  but4->OnClick(MakeCB(this,&DispMask::SaveDisp));
}

DispMask::~DispMask()
{
 delete win;
 delete var;
}

void DispMask::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void DispMask::Resize(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  canvas->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(canvas->P().Width(),canvas->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (canvas->P().Width() - image.Size(0))/2;
  nat32 sy = (canvas->P().Height() - image.Size(1))/2;
  canvas->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(image.Size(0),image.Size(1))),bs::Pos(sx,sy),image);

 // Update...
  canvas->Update();
}

void DispMask::LoadDisp(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Disparity","*.obf,*.dis",fn))
 {
  cstr filename = fn.ToStr();
   svt::Node * newDisp = svt::Load(cyclops.Core(),filename);
  mem::Free(filename);

  if (newDisp==null<svt::Node*>())
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load disparity");
   return;
  }

  if (str::Compare(typestring(*newDisp),"eos::svt::Var")!=0)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"File is the wrong structure for a disparity map");
   delete newDisp;
   return;
  }
  svt::Var * newDispVar = static_cast<svt::Var*>(newDisp);

  nat32 dispInd;
  if ((newDispVar->Dims()!=2)||
      (newDispVar->GetIndex(cyclops.TT()("disp"),dispInd)==false)||
      (newDispVar->FieldType(dispInd)!=cyclops.TT()("eos::real32")))
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"File is the wrong format for a disparity map");
   delete newDispVar;
   return;
  }

  delete dispVar;
  dispVar = newDispVar;

  if ((!dispVar->Exists("mask"))||(!dispVar->Exists("user_mask")))
  {
   bit maskIni = true;
   if (!dispVar->Exists("mask")) dispVar->Add("mask",maskIni);
   if (!dispVar->Exists("user_mask")) dispVar->Add("user_mask",maskIni);
   dispVar->Commit();
  }

  dispVar->ByName("disp",disp);
  dispVar->ByName("mask",mask);
  dispVar->ByName("user_mask",userMask);

  RenderDisp();
  canvas->Redraw();
 }
}

void DispMask::SaveMask(gui::Base * obj,gui::Event * event)
{
 // Ask the user for a filename...
  str::String fn(".bmp");
  if (cyclops.App().SaveFileDialog("Save Mask",fn))
  {
   // Create tempory image...
    svt::Var civ(cyclops.Core());
     civ.Setup2D(disp.Size(0),disp.Size(1));
     bs::ColourRGB iniRGB(0.0,0.0,0.0);
     civ.Add("rgb",iniRGB);
    civ.Commit(false);

    svt::Field<bs::ColourRGB> ci(&civ,"rgb");

   // Fill data structure...
    for (nat32 y=0;y<disp.Size(1);y++)
    {
     for (nat32 x=0;x<disp.Size(0);x++)
     {
      if (mask.Get(x,y)&&userMask.Get(x,y)) ci.Get(x,y) = bs::ColourRGB(1.0,1.0,1.0);
                                       else ci.Get(x,y) = bs::ColourRGB(0.0,0.0,0.0);
     }
    }

   // Save it...
    if (fn.EndsWith(".bmp")==false) fn << ".bmp";
    cstr ts = fn.ToStr();
    if (!filter::SaveImage(ci,ts,true))
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving mask.");
    }
    mem::Free(ts);
  }
}

void DispMask::LoadMask(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Load Mask","*.bmp,*.jpg,*.png,*.tif",fn))
 {
  // Load image into memory...
   cstr filename = fn.ToStr();
   svt::Var * maskImage = filter::LoadImageRGB(cyclops.Core(),filename);
   mem::Free(filename);
   if (maskImage==null<svt::Var*>())
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load image");
    return;
   }

   if ((maskImage->Size(0)!=disp.Size(0))||(maskImage->Size(1)!=disp.Size(1)))
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Mask dimensions do not match disparity map dimensions.");
    delete maskImage;
    return;
   }


  // Update the user mask...
   svt::Field<bs::ColourRGB> mi(maskImage,"rgb");
   for (nat32 y=0;y<userMask.Size(1);y++)
   {
    for (nat32 x=0;x<userMask.Size(0);x++)
    {
     userMask.Get(x,y) = (mi.Get(x,y).r + mi.Get(x,y).g + mi.Get(x,y).b) > 1.5;
    }
   }
   delete maskImage;


  // Redraw...
   RenderDisp();
   canvas->Redraw();
 }
}

void DispMask::SaveDisp(gui::Base * obj,gui::Event * event)
{
 // Create the result svt...
  real32 dispIni = 0.0;
  bit maskIni = false;

  svt::Var * result = new svt::Var(cyclops.Core());
  result->Setup2D(disp.Size(0),disp.Size(1));
  result->Add("disp",dispIni);
  result->Add("mask",maskIni);
  result->Commit();


 // Fill the result svt...
  svt::Field<real32> dispTemp(result,"disp");
  svt::Field<bit> maskTemp(result,"mask");

  dispTemp.CopyFrom(disp);
  for (nat32 y=0;y<maskTemp.Size(1);y++)
  {
   for (nat32 x=0;x<maskTemp.Size(0);x++) maskTemp.Get(x,y) = mask.Get(x,y) && userMask.Get(x,y);
  }


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


 // Clean up...
  delete result;
}

void DispMask::RenderDisp()
{
 if ((image.Size(0)!=disp.Size(0))||(image.Size(1)!=disp.Size(1)))
 {
  var->Setup2D(disp.Size(0),disp.Size(1));
  var->Commit();
  var->ByName("rgb",image);

  canvas->SetSize(image.Size(0),image.Size(1));
 }

 real32 minDisp = 0.0;
 real32 maxDisp = 0.0;
 for (nat32 y=0;y<image.Size(1);y++)
 {
  for (nat32 x=0;x<image.Size(0);x++)
  {
   if (mask.Get(x,y))
   {
    minDisp = math::Min(minDisp,disp.Get(x,y));
    maxDisp = math::Max(maxDisp,disp.Get(x,y));
   }
  }
 }

 for (nat32 y=0;y<image.Size(1);y++)
 {
  for (nat32 x=0;x<image.Size(0);x++)
  {
   if (mask.Get(x,y))
   {
    if (userMask.Get(x,y))
    {
     nat8 val = nat8(math::Clamp<real32>(255.0*(disp.Get(x,y)-minDisp)/(maxDisp-minDisp),0,255));
     image.Get(x,y) = bs::ColRGB(val,val,val);
    }
    else image.Get(x,y) = bs::ColRGB(255,0,255);
   }
   else image.Get(x,y) = bs::ColRGB(0,0,255);
  }
 }
}

//------------------------------------------------------------------------------
