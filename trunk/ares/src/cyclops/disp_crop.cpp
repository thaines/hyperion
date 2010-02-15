//------------------------------------------------------------------------------
// Copyright 2007 Tom Haines

#include "cyclops/disp_crop.h"

//------------------------------------------------------------------------------
DispCrop::DispCrop(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
imgVar(null<svt::Var*>()),dispVar(null<svt::Var*>())
{
 // Create default image and crop...
  bs::ColRGB colIni(0,0,255);
  real32 dispIni = 0.0;
  bit maskIni = true;

  imgVar = new svt::Var(cyclops.Core());
  imgVar->Setup2D(320,240);
  imgVar->Add("rgb",colIni);
  imgVar->Commit();
  imgVar->ByName("rgb",img);
  
  dispVar = new svt::Var(cyclops.Core());
  dispVar->Setup2D(320,240);
  dispVar->Add("disp",dispIni);
  dispVar->Add("mask",maskIni);
  dispVar->Commit();
  dispVar->ByName("disp",disp);
  dispVar->ByName("mask",mask);
  
  pMin[0] = 100.0;
  pMin[1] = 50.0;
  pMax[0] = 220.0;
  pMax[1] = 190.0;


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Crop");
  cyclops.App().Attach(win);
  win->SetSize(disp.Size(0)+8,disp.Size(1)+48);
   
   gui::Vertical * vert1 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   win->SetChild(vert1);
   
   gui::Horizontal * horiz1 = static_cast<gui::Horizontal*>(cyclops.Fact().Make("Horizontal"));
   vert1->AttachBottom(horiz1,false);
   
   gui::Button * but1 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but1b = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but2 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab1 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab1b = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab2 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
  
   but1->SetChild(lab1); lab1->Set("Load Disparity...");
   but1b->SetChild(lab1b); lab1b->Set("Save Cropped Disparity...");
   but2->SetChild(lab2); lab2->Set("Update Pair...");

   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but1b,false);
   horiz1->AttachRight(but2,false);

   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);
   
   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);
   
   canvas->SetSize(disp.Size(0),disp.Size(1));

 // Event handlers...
  win->OnDeath(MakeCB(this,&DispCrop::Quit));
  canvas->OnResize(MakeCB(this,&DispCrop::Resize));
  canvas->OnClick(MakeCB(this,&DispCrop::Click));
  canvas->OnMove(MakeCB(this,&DispCrop::Move));
  but1->OnClick(MakeCB(this,&DispCrop::LoadDisp));
  but1b->OnClick(MakeCB(this,&DispCrop::SaveDisp));
  but2->OnClick(MakeCB(this,&DispCrop::UpdatePair));
}

DispCrop::~DispCrop()
{
 delete win;
 delete imgVar;
 delete dispVar;
}

void DispCrop::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void DispCrop::Resize(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  canvas->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(canvas->P().Width(),canvas->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (canvas->P().Width() - img.Size(0))/2;
  nat32 sy = (canvas->P().Height() - img.Size(1))/2; 
  canvas->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(img.Size(0),img.Size(1))),bs::Pos(sx,sy),img);

 // Render the selected region box...
  math::Vect<2,real32> tr;
   tr[0] = pMax[0];
   tr[1] = pMin[1];
  math::Vect<2,real32> bl;
   bl[0] = pMin[0];
   bl[1] = pMax[1];

  RenderLine(pMin,tr,bs::ColourRGB(1.0,0.0,1.0));
  RenderLine(pMin,bl,bs::ColourRGB(1.0,0.0,1.0));
  RenderLine(tr,pMax,bs::ColourRGB(1.0,0.0,1.0));
  RenderLine(bl,pMax,bs::ColourRGB(1.0,0.0,1.0));

 // Update...
  canvas->Update();
}

void DispCrop::Click(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseButtonEvent * mbe = static_cast<gui::MouseButtonEvent*>(event);
  math::Vect<2,real32> mp;
  
  nat32 sx = (canvas->P().Width() - img.Size(0))/2;
  nat32 sy = (canvas->P().Height() - img.Size(1))/2; 

  mp[0] = mbe->x - sx;
  mp[1] = mbe->y - sy;
  
  if (mbe->button==gui::MouseButtonEvent::LMB)
  {
   // Move the nearest corner point, including the 2 virtual corners...
    if (mp[0]<(0.5*(pMin[0]+pMax[0]))) pMin[0] = mp[0];
                                  else pMax[0] = mp[0];

    if (mp[1]<(0.5*(pMin[1]+pMax[1]))) pMin[1] = mp[1];
                                  else pMax[1] = mp[1];  
  }
  else
  {
   pMin[0] = mp[0];
   pMax[0] = mp[0]+1;
   pMin[1] = mp[1];
   pMax[1] = mp[1]+1;
  }


 // Redraw the display... 
  canvas->Redraw();
}

void DispCrop::Move(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseMoveEvent * mme = static_cast<gui::MouseMoveEvent*>(event);
  if (!mme->lmb) return;
  math::Vect<2,real32> mp;
  
  nat32 sx = (canvas->P().Width() - img.Size(0))/2;
  nat32 sy = (canvas->P().Height() - img.Size(1))/2; 
  
  mp[0] = mme->x - sx;
  mp[1] = mme->y - sy;
  
 // Move the nearest end point...
  if (mp[0]<(0.5*(pMin[0]+pMax[0]))) pMin[0] = mp[0];
                                else pMax[0] = mp[0];

  if (mp[1]<(0.5*(pMin[1]+pMax[1]))) pMin[1] = mp[1];
                                else pMax[1] = mp[1];
 
 // Redraw the display... 
  canvas->Redraw();
}

void DispCrop::LoadDisp(gui::Base * obj,gui::Event * event)
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

  if (!dispVar->Exists("mask"))
  {
   bit maskIni = true;
   if (!dispVar->Exists("mask")) dispVar->Add("mask",maskIni);
   dispVar->Commit();
  }

  dispVar->ByName("disp",disp);
  dispVar->ByName("mask",mask);

  pMin[0] = real32(disp.Size(0))/3.0;
  pMin[1] = real32(disp.Size(1))/3.0;
  pMax[0] = real32(disp.Size(0))*2.0/3.0;
  pMax[1] = real32(disp.Size(1))*2.0/3.0;

  RenderDisp();
  canvas->Redraw();
 }
}

void DispCrop::SaveDisp(gui::Base * obj,gui::Event * event)
{
 // Ask the user for a filename...
  str::String fn("");
  if (cyclops.App().SaveFileDialog("Save Cropped Disparity",fn))
  {
   // Move to coordinates we can use...
    math::Vect<2,int32> cMin;
    math::Vect<2,int32> cMax;
     cMin[0] = int32(pMin[0]);
     cMin[1] = img.Size(1)-1 - int32(pMax[1]);
     cMax[0] = int32(pMax[0]);
     cMax[1] = img.Size(1)-1 - int32(pMin[1]);
  
   // Create a data structure to contain it...    
    real32 dispIni = 0.0;
    bit maskIni = true;

    svt::Var cDispVar(cyclops.Core());
    cDispVar.Setup2D(cMax[0]-cMin[0]+1,cMax[1]-cMin[1]+1);
    cDispVar.Add("disp",dispIni);
    cDispVar.Add("mask",maskIni);
    cDispVar.Commit(false);
    
    svt::Field<real32> cDisp(&cDispVar,"disp");
    svt::Field<bit> cMask(&cDispVar,"mask");

   // Fill data structure...
    for (int32 y=cMin[1];y<=cMax[1];y++)
    {
     for (int32 x=cMin[0];x<=cMax[0];x++)
     {
      cDisp.Get(x-cMin[0],y-cMin[1]) = disp.Get(x,y) + real32(cMin[0]);
      cMask.Get(x-cMin[0],y-cMin[1]) = mask.Get(x,y);
     }
    }
 
   // Save it...
    if (!fn.EndsWith(".dis")) fn << ".dis";
    cstr ts = fn.ToStr();
    if (!svt::Save(ts,&cDispVar,true))
    {
     cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving svt file.");
    }
    mem::Free(ts);
  }
}

void DispCrop::UpdatePair(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Camera Pair File","*.pcc",fn))
 {
  cam::CameraPair pair;
  if (pair.Load(fn)==false)
  {
   cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load camera pair file");
  }
  else
  {
   // Apply the update...
    math::Vect<2,int32> cMin;
    math::Vect<2,int32> cMax;
     cMin[0] = int32(pMin[0]);
     cMin[1] = img.Size(1)-1 - int32(pMax[1]);
     cMax[0] = int32(pMax[0]);
     cMax[1] = img.Size(1)-1 - int32(pMin[1]);
    
    pair.CropLeft(cMin[0],cMax[0]-cMin[0]+1.0,cMin[1],cMax[1]-cMin[1]+1.0);
    pair.CropRight(0.0,pair.rightDim[0],cMin[1],cMax[1]-cMin[1]+1.0);
    

   // Save back the file...
    str::String fn2;
    if (cyclops.App().SaveFileDialog("Save Camera Pair File...",fn2))
    {
     if (!fn2.EndsWith(".pcc")) fn2 += ".pcc";
     if (!pair.Save(fn2,true))
     {
      cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving back the file.");
      return;
     }
    }
  }
 }
}

void DispCrop::RenderLine(const math::Vect<2,real32> & a,const math::Vect<2,real32> & b,const bs::ColourRGB & col)
{
 nat32 sx = (canvas->P().Width() - img.Size(0))/2;
 nat32 sy = (canvas->P().Height() - img.Size(1))/2;
 
 nat32 ax = sx + nat32(a[0]);
 nat32 ay = sy + nat32(a[1]);
 
 nat32 bx = sx + nat32(b[0]);
 nat32 by = sy + nat32(b[1]);
 
 canvas->P().Line(bs::Pos(ax,ay),bs::Pos(bx,by),col); 
}

void DispCrop::RenderDisp()
{
 if ((img.Size(0)!=disp.Size(0))||(img.Size(1)!=disp.Size(1)))
 {
  imgVar->Setup2D(disp.Size(0),disp.Size(1));
  imgVar->Commit();
  imgVar->ByName("rgb",img);

  canvas->SetSize(img.Size(0),img.Size(1));
 }

 real32 minDisp = 0.0;
 real32 maxDisp = 0.0;
 for (nat32 y=0;y<img.Size(1);y++)
 {
  for (nat32 x=0;x<img.Size(0);x++)
  {
   if (mask.Get(x,y))
   {
    minDisp = math::Min(minDisp,disp.Get(x,y));
    maxDisp = math::Max(maxDisp,disp.Get(x,y));
   }
  }
 }

 for (nat32 y=0;y<img.Size(1);y++)
 {
  for (nat32 x=0;x<img.Size(0);x++)
  {
   if (mask.Get(x,y))
   {
    nat8 val = nat8(math::Clamp<real32>(255.0*(disp.Get(x,y)-minDisp)/(maxDisp-minDisp),0,255));
    img.Get(x,y) = bs::ColRGB(val,val,val);
   }
   else img.Get(x,y) = bs::ColRGB(0,0,255);
  }
 }
}

//------------------------------------------------------------------------------

