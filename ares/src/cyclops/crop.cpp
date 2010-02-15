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


#include "cyclops/crop.h"

//------------------------------------------------------------------------------
Crop::Crop(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
imgVar(null<svt::Var*>()),imageVar(null<svt::Var*>())
{
 // Create default image and crop...
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
  
  pMin[0] = 100.0;
  pMin[1] = 50.0;
  pMax[0] = 220.0;
  pMax[1] = 190.0;


 // Build gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Crop");
  cyclops.App().Attach(win);
  win->SetSize(image.Size(0)+8,image.Size(1)+48);
   
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
  
   but1->SetChild(lab1); lab1->Set("Load Image...");
   but1b->SetChild(lab1b); lab1b->Set("Save Cropped Image...");
   but2->SetChild(lab2); lab2->Set("Update Intrinsic...");

   horiz1->AttachRight(but1,false);
   horiz1->AttachRight(but1b,false);
   horiz1->AttachRight(but2,false);

   gui::Panel * panel = static_cast<gui::Panel*>(cyclops.Fact().Make("Panel"));
   vert1->AttachBottom(panel);
   
   canvas = static_cast<gui::Canvas*>(cyclops.Fact().Make("Canvas"));
   panel->SetChild(canvas);
   
   canvas->SetSize(image.Size(0),image.Size(1));

 // Event handlers...
  win->OnDeath(MakeCB(this,&Crop::Quit));
  canvas->OnResize(MakeCB(this,&Crop::Resize));
  canvas->OnClick(MakeCB(this,&Crop::Click));
  canvas->OnMove(MakeCB(this,&Crop::Move));
  but1->OnClick(MakeCB(this,&Crop::LoadImage));
  but1b->OnClick(MakeCB(this,&Crop::SaveImage));
  but2->OnClick(MakeCB(this,&Crop::UpdateIntrinsic));
}

Crop::~Crop()
{
 delete win;
 delete imgVar;
 delete imageVar;
}

void Crop::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void Crop::Resize(gui::Base * obj,gui::Event * event)
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

void Crop::Click(gui::Base * obj,gui::Event * event)
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

void Crop::Move(gui::Base * obj,gui::Event * event)
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

void Crop::LoadImage(gui::Base * obj,gui::Event * event)
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
   imgVar->Setup2D(image.Size(0),image.Size(1));
   imgVar->Commit();
   imgVar->ByName("rgb",img);
   
   for (nat32 y=0;y<img.Size(1);y++)
   {
    for (nat32 x=0;x<img.Size(0);x++)
    {
     img.Get(x,y) = image.Get(x,y);
    }
   }
   
   pMin[0] = real32(img.Size(0))/3.0;
   pMin[1] = real32(img.Size(1))/3.0;
   pMax[0] = real32(img.Size(0))*2.0/3.0;
   pMax[1] = real32(img.Size(1))*2.0/3.0;
   
  // Refresh the display...
   canvas->SetSize(img.Size(0),img.Size(1));
   canvas->Redraw();
 }
}

void Crop::SaveImage(gui::Base * obj,gui::Event * event)
{
 // Ask the user for a filename...
  str::String fn("");
  if (cyclops.App().SaveFileDialog("Save Cropped Image",fn))
  {
   // Move to coordinates we can use...
    math::Vect<2,int32> cMin;
    math::Vect<2,int32> cMax;
     cMin[0] = int32(pMin[0]);
     cMin[1] = img.Size(1)-1 - int32(pMax[1]);
     cMax[0] = int32(pMax[0]);
     cMax[1] = img.Size(1)-1 - int32(pMin[1]);
  
   // Create a data structure to contain it...    
    svt::Var civ(cyclops.Core());
     civ.Setup2D(cMax[0]-cMin[0]+1,cMax[1]-cMin[1]+1);
     bs::ColourRGB iniRGB(0.0,0.0,0.0);
     civ.Add("rgb",iniRGB);
    civ.Commit(false);
    
    svt::Field<bs::ColourRGB> ci(&civ,"rgb");

   // Fill data structure...
    for (int32 y=cMin[1];y<=cMax[1];y++)
    {
     for (int32 x=cMin[0];x<=cMax[0];x++)
     {
      ci.Get(x-cMin[0],y-cMin[1]) = image.Get(x,y);
     }
    }
 
   // Save it...
    if (!(fn.EndsWith(".bmp")||fn.EndsWith(".jpg")||fn.EndsWith(".tga")||fn.EndsWith(".png"))) fn << ".bmp";
    cstr ts = fn.ToStr();
     if (!filter::SaveImage(ci,ts,true))
     {
      cyclops.App().MessageDialog(gui::App::MsgErr,"Error saving cropped image.");
     }
    mem::Free(ts);
  }
}

void Crop::UpdateIntrinsic(gui::Base * obj,gui::Event * event)
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
   // Move to coordinates we can use, include adjustment for image scaling...
    real64 xScale = calib.dim[0]/real64(img.Size(0));
    real64 yScale = calib.dim[1]/real64(img.Size(1));
    math::Vect<2,real64> cMin;
    math::Vect<2,real64> cMax;
     cMin[0] = pMin[0]*xScale;
     cMin[1] = (real64(img.Size(1)-1) - pMax[1])*yScale;
     cMax[0] = pMax[0]*xScale;
     cMax[1] = (real64(img.Size(1)-1) - pMin[1])*yScale;
  
   // Update the camera calibration...
    calib.Crop(cMin[0],cMax[0]-cMin[0]+1.0,cMin[1],cMax[1]-cMin[1]+1.0);

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

void Crop::RenderLine(const math::Vect<2,real32> & a,const math::Vect<2,real32> & b,const bs::ColourRGB & col)
{
 nat32 sx = (canvas->P().Width() - img.Size(0))/2;
 nat32 sy = (canvas->P().Height() - img.Size(1))/2;
 
 nat32 ax = sx + nat32(a[0]);
 nat32 ay = sy + nat32(a[1]);
 
 nat32 bx = sx + nat32(b[0]);
 nat32 by = sy + nat32(b[1]);
 
 canvas->P().Line(bs::Pos(ax,ay),bs::Pos(bx,by),col); 
}

//------------------------------------------------------------------------------
