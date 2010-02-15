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


#include "cyclops/crop_pair.h"

//------------------------------------------------------------------------------
CropPair::CropPair(Cyclops & cyc)
:cyclops(cyc),win(null<gui::Window*>()),
leftVar(null<svt::Var*>()),rightVar(null<svt::Var*>())
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

  lMin[0] = 100; lMin[1] = 80;
  lMax[0] = 220; lMax[1] = 160;
  rMin = lMin;
  rMax = lMax;


 // Invoke gui...
  win = static_cast<gui::Window*>(cyclops.Fact().Make("Window"));
  win->SetTitle("Rectified Pair Crop");
  cyclops.App().Attach(win);
  win->SetSize(leftImage.Size(0)+rightImage.Size(0)+24,math::Max(leftImage.Size(1),rightImage.Size(1))+96);

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

   gui::Vertical * vert2 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   horiz1->AttachRight(vert2,false);
   vert2->AttachBottom(but1);
   vert2->AttachBottom(but2);


   gui::Button * but3 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Button * but4 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab3 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   gui::Label * lab4 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));

   but3->SetChild(lab3); lab3->Set("Save Left Crop...");
   but4->SetChild(lab4); lab4->Set("Save Right Crop...");

   gui::Vertical * vert3 = static_cast<gui::Vertical*>(cyclops.Fact().Make("Vertical"));
   horiz1->AttachRight(vert3,false);
   vert3->AttachBottom(but3);
   vert3->AttachBottom(but4);


   gui::Button * but5 = static_cast<gui::Button*>(cyclops.Fact().Make("Button"));
   gui::Label * lab5 = static_cast<gui::Label*>(cyclops.Fact().Make("Label"));
   but5->SetChild(lab5); lab5->Set("Update Pair Config...");
   horiz1->AttachRight(but5,false);


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
  win->OnDeath(MakeCB(this,&CropPair::Quit));

  left->OnResize(MakeCB(this,&CropPair::ResizeLeft));
  left->OnClick(MakeCB(this,&CropPair::ClickLeft));
  left->OnMove(MakeCB(this,&CropPair::MoveLeft));

  right->OnResize(MakeCB(this,&CropPair::ResizeRight));
  right->OnClick(MakeCB(this,&CropPair::ClickRight));
  right->OnMove(MakeCB(this,&CropPair::MoveRight));

  but1->OnClick(MakeCB(this,&CropPair::LoadLeft));
  but2->OnClick(MakeCB(this,&CropPair::LoadRight));
  but3->OnClick(MakeCB(this,&CropPair::SaveLeft));
  but4->OnClick(MakeCB(this,&CropPair::SaveRight));
  but5->OnClick(MakeCB(this,&CropPair::UpdatePair));
}

CropPair::~CropPair()
{
 delete win;
 delete leftVar;
 delete rightVar;
}

void CropPair::Quit(gui::Base * obj,gui::Event * event)
{
 gui::DeathEvent * e = static_cast<gui::DeathEvent*>(event);
 e->doDeath = false;
 delete this;
}

void CropPair::ResizeLeft(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  left->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(left->P().Width(),left->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (left->P().Width() - leftImage.Size(0))/2;
  nat32 sy = (left->P().Height() - leftImage.Size(1))/2;
  left->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(leftImage.Size(0),leftImage.Size(1))),bs::Pos(sx,sy),leftImage);

 // Render the selected region box...
  math::Vect<2,real64> tr;
   tr[0] = lMax[0];
   tr[1] = lMin[1];
  math::Vect<2,real64> bl;
   bl[0] = lMin[0];
   bl[1] = lMax[1];

  RenderLeftLine(lMin,tr,bs::ColourRGB(1.0,0.0,1.0));
  RenderLeftLine(lMin,bl,bs::ColourRGB(1.0,0.0,1.0));
  RenderLeftLine(tr,lMax,bs::ColourRGB(1.0,0.0,1.0));
  RenderLeftLine(bl,lMax,bs::ColourRGB(1.0,0.0,1.0));

 // Update...
  left->Update();
}

void CropPair::ClickLeft(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseButtonEvent * mbe = static_cast<gui::MouseButtonEvent*>(event);
  math::Vect<2,real32> mp;

  nat32 sx = (left->P().Width() - leftImage.Size(0))/2;
  nat32 sy = (left->P().Height() - leftImage.Size(1))/2;

  mp[0] = mbe->x - sx;
  mp[1] = mbe->y - sy;

  if (mbe->button==gui::MouseButtonEvent::LMB)
  {
   // Move the nearest corner point, including the 2 virtual corners...
    if (mp[0]<(0.5*(lMin[0]+lMax[0]))) lMin[0] = mp[0];
                                  else lMax[0] = mp[0];

    if (mp[1]<(0.5*(lMin[1]+lMax[1]))) {lMin[1] = mp[1]; rMin[1] = mp[1];}
                                  else {lMax[1] = mp[1]; rMax[1] = mp[1];}
  }
  else
  {
   lMin[0] = mp[0];
   lMax[0] = mp[0]+1;
   lMin[1] = mp[1];   rMin[1] = mp[1];
   lMax[1] = mp[1]+1; rMax[1] = mp[1]+1;
  }

 // Redraw the display...
  left->Redraw();
  right->Redraw();
}

void CropPair::MoveLeft(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseMoveEvent * mme = static_cast<gui::MouseMoveEvent*>(event);
  if (!mme->lmb) return;
  math::Vect<2,real32> mp;

  nat32 sx = (left->P().Width() - leftImage.Size(0))/2;
  nat32 sy = (left->P().Height() - leftImage.Size(1))/2;

  mp[0] = mme->x - sx;
  mp[1] = mme->y - sy;

 // Move the nearest corner point, including the 2 virtual corners...
  if (mp[0]<(0.5*(lMin[0]+lMax[0]))) lMin[0] = mp[0];
                                else lMax[0] = mp[0];

  if (mp[1]<(0.5*(lMin[1]+lMax[1]))) {lMin[1] = mp[1]; rMin[1] = mp[1];}
                                else {lMax[1] = mp[1]; rMax[1] = mp[1];}

 // Redraw the display...
  left->Redraw();
}

void CropPair::ResizeRight(gui::Base * obj,gui::Event * event)
{
 // Clear the canvas to a nice shade of grey...
  right->P().Rectangle(bs::Rect(bs::Pos(0,0),bs::Pos(right->P().Width(),right->P().Height())),bs::ColourRGB(0.5,0.5,0.5));

 // Render the image...
  nat32 sx = (right->P().Width() - rightImage.Size(0))/2;
  nat32 sy = (right->P().Height() - rightImage.Size(1))/2;
  right->P().Image(bs::Rect(bs::Pos(0,0),bs::Pos(rightImage.Size(0),rightImage.Size(1))),bs::Pos(sx,sy),rightImage);

 // Render the selected region box...
  math::Vect<2,real64> tr;
   tr[0] = rMax[0];
   tr[1] = rMin[1];
  math::Vect<2,real64> bl;
   bl[0] = rMin[0];
   bl[1] = rMax[1];

  RenderRightLine(rMin,tr,bs::ColourRGB(1.0,0.0,1.0));
  RenderRightLine(rMin,bl,bs::ColourRGB(1.0,0.0,1.0));
  RenderRightLine(tr,rMax,bs::ColourRGB(1.0,0.0,1.0));
  RenderRightLine(bl,rMax,bs::ColourRGB(1.0,0.0,1.0));

 // Update...
  right->Update();
}

void CropPair::ClickRight(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseButtonEvent * mbe = static_cast<gui::MouseButtonEvent*>(event);
  math::Vect<2,real32> mp;

  nat32 sx = (right->P().Width() - rightImage.Size(0))/2;
  nat32 sy = (right->P().Height() - rightImage.Size(1))/2;

  mp[0] = mbe->x - sx;
  mp[1] = mbe->y - sy;

  if (mbe->button==gui::MouseButtonEvent::LMB)
  {
   // Move the nearest corner point, including the 2 virtual corners...
    if (mp[0]<(0.5*(rMin[0]+rMax[0]))) rMin[0] = mp[0];
                                  else rMax[0] = mp[0];

    if (mp[1]<(0.5*(rMin[1]+rMax[1]))) {lMin[1] = mp[1]; rMin[1] = mp[1];}
                                  else {lMax[1] = mp[1]; rMax[1] = mp[1];}
  }
  else
  {
   rMin[0] = mp[0];
   rMax[0] = mp[0]+1;
   lMin[1] = mp[1];   rMin[1] = mp[1];
   lMax[1] = mp[1]+1; rMax[1] = mp[1]+1;
  }

 // Redraw the display...
  left->Redraw();
  right->Redraw();
}

void CropPair::MoveRight(gui::Base * obj,gui::Event * event)
{
 // Determine image coordinates...
  gui::MouseMoveEvent * mme = static_cast<gui::MouseMoveEvent*>(event);
  if (!mme->lmb) return;
  math::Vect<2,real32> mp;

  nat32 sx = (right->P().Width() - rightImage.Size(0))/2;
  nat32 sy = (right->P().Height() - rightImage.Size(1))/2;

  mp[0] = mme->x - sx;
  mp[1] = mme->y - sy;

 // Move the nearest corner point, including the 2 virtual corners...
  if (mp[0]<(0.5*(rMin[0]+rMax[0]))) rMin[0] = mp[0];
                                else rMax[0] = mp[0];

  if (mp[1]<(0.5*(rMin[1]+rMax[1]))) {lMin[1] = mp[1]; rMin[1] = mp[1];}
                                else {lMax[1] = mp[1]; rMax[1] = mp[1];}

 // Redraw the display...
  right->Redraw();
}

void CropPair::LoadLeft(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Left Image","*.bmp,*.jpg,*.png,*.tif",fn))
 {
  // Load image into memory...
   cstr filename = fn.ToStr();
   svt::Var * floatVar = filter::LoadImageRGB(cyclops.Core(),filename);
   mem::Free(filename);
   if (floatVar==null<svt::Var*>())
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load image");
    return;
   }

  // Image loaded, but its not in a format suitable for fast display - convert...
   leftVar->Setup2D(floatVar->Size(0),floatVar->Size(1));
   leftVar->Commit();
   leftVar->ByName("rgb",leftImage);

   svt::Field<bs::ColourRGB> floatImage(floatVar,"rgb");
   for (nat32 y=0;y<leftImage.Size(1);y++)
   {
    for (nat32 x=0;x<leftImage.Size(0);x++)
    {
     leftImage.Get(x,y) = floatImage.Get(x,y);
    }
   }

   delete floatVar;

  // Refresh the display...
   left->SetSize(leftImage.Size(0),leftImage.Size(1));
   left->Redraw();
 }
}

void CropPair::LoadRight(gui::Base * obj,gui::Event * event)
{
 str::String fn;
 if (cyclops.App().LoadFileDialog("Select Right Image","*.bmp,*.jpg,*.png,*.tif",fn))
 {
  // Load image into memory...
   cstr filename = fn.ToStr();
   svt::Var * floatVar = filter::LoadImageRGB(cyclops.Core(),filename);
   mem::Free(filename);
   if (floatVar==null<svt::Var*>())
   {
    cyclops.App().MessageDialog(gui::App::MsgErr,"Failed to load image");
    return;
   }

  // Image loaded, but its not in a format suitable for fast display - convert...
   rightVar->Setup2D(floatVar->Size(0),floatVar->Size(1));
   rightVar->Commit();
   rightVar->ByName("rgb",rightImage);

   svt::Field<bs::ColourRGB> floatImage(floatVar,"rgb");
   for (nat32 y=0;y<rightImage.Size(1);y++)
   {
    for (nat32 x=0;x<rightImage.Size(0);x++)
    {
     rightImage.Get(x,y) = floatImage.Get(x,y);
    }
   }

   delete floatVar;

  // Refresh the display...
   right->SetSize(rightImage.Size(0),rightImage.Size(1));
   right->Redraw();
 }
}

void CropPair::SaveLeft(gui::Base * obj,gui::Event * event)
{
 // Ask the user for a filename...
  str::String fn("");
  if (cyclops.App().SaveFileDialog("Save Cropped Left Image",fn))
  {
   // Move to coordinates we can use...
    math::Vect<2,int32> cMin;
    math::Vect<2,int32> cMax;
     cMin[0] = int32(lMin[0]);
     cMin[1] = leftImage.Size(1)-1 - int32(lMax[1]);
     cMax[0] = int32(lMax[0]);
     cMax[1] = leftImage.Size(1)-1 - int32(lMin[1]);

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
      ci.Get(x-cMin[0],y-cMin[1]) = leftImage.Get(x,y);
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

void CropPair::SaveRight(gui::Base * obj,gui::Event * event)
{
 // Ask the user for a filename...
  str::String fn("");
  if (cyclops.App().SaveFileDialog("Save Cropped Right Image",fn))
  {
   // Move to coordinates we can use...
    math::Vect<2,int32> cMin;
    math::Vect<2,int32> cMax;
     cMin[0] = int32(rMin[0]);
     cMin[1] = rightImage.Size(1)-1 - int32(rMax[1]);
     cMax[0] = int32(rMax[0]);
     cMax[1] = rightImage.Size(1)-1 - int32(rMin[1]);

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
      ci.Get(x-cMin[0],y-cMin[1]) = rightImage.Get(x,y);
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

void CropPair::UpdatePair(gui::Base * obj,gui::Event * event)
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
   // Left...
   {
    // Move to coordinates we can use, include adjustment for image scaling...
     real64 xScale = pair.leftDim[0]/real64(leftImage.Size(0));
     real64 yScale = pair.leftDim[1]/real64(leftImage.Size(1));
     math::Vect<2,real64> cMin;
     math::Vect<2,real64> cMax;
      cMin[0] = lMin[0]*xScale;
      cMin[1] = (real64(leftImage.Size(1)-1) - lMax[1])*yScale;
      cMax[0] = lMax[0]*xScale;
      cMax[1] = (real64(leftImage.Size(1)-1) - lMin[1])*yScale;

    // Update the camera calibration...
     pair.CropLeft(cMin[0],cMax[0]-cMin[0]+1.0,cMin[1],cMax[1]-cMin[1]+1.0);
   }

   // Right...
   {
    // Move to coordinates we can use, include adjustment for image scaling...
     real64 xScale = pair.rightDim[0]/real64(rightImage.Size(0));
     real64 yScale = pair.rightDim[1]/real64(rightImage.Size(1));
     math::Vect<2,real64> cMin;
     math::Vect<2,real64> cMax;
      cMin[0] = rMin[0]*xScale;
      cMin[1] = (real64(rightImage.Size(1)-1) - rMax[1])*yScale;
      cMax[0] = rMax[0]*xScale;
      cMax[1] = (real64(rightImage.Size(1)-1) - rMin[1])*yScale;

    // Update the camera calibration...
     pair.CropRight(cMin[0],cMax[0]-cMin[0]+1.0,cMin[1],cMax[1]-cMin[1]+1.0);
   }

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

void CropPair::RenderLeftLine(const math::Vect<2,real64> & a,const math::Vect<2,real64> & b,const bs::ColourRGB & col)
{
 nat32 sx = (left->P().Width() - leftImage.Size(0))/2;
 nat32 sy = (left->P().Height() - leftImage.Size(1))/2;

 nat32 ax = sx + nat32(a[0]);
 nat32 ay = sy + nat32(a[1]);

 nat32 bx = sx + nat32(b[0]);
 nat32 by = sy + nat32(b[1]);

 left->P().Line(bs::Pos(ax,ay),bs::Pos(bx,by),col);
}

void CropPair::RenderRightLine(const math::Vect<2,real64> & a,const math::Vect<2,real64> & b,const bs::ColourRGB & col)
{
 nat32 sx = (right->P().Width() - rightImage.Size(0))/2;
 nat32 sy = (right->P().Height() - rightImage.Size(1))/2;

 nat32 ax = sx + nat32(a[0]);
 nat32 ay = sy + nat32(a[1]);

 nat32 bx = sx + nat32(b[0]);
 nat32 by = sy + nat32(b[1]);

 right->P().Line(bs::Pos(ax,ay),bs::Pos(bx,by),col);
}

//------------------------------------------------------------------------------
